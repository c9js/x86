#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <sys/resource.h>
#include <time.h>
#include <fcntl.h>

#define BUFFER_SIZE 1514
#define HEX_SIZE (BUFFER_SIZE * 2)
#define MAX_ERRORS 100
#define ERROR_WINDOW 60
#define DEFAULT_DELAY 0.1f
#define MIN_DELAY 0.001f
#define MAX_DELAY 10.0f

static const char hex_table[16] = "0123456789ABCDEF";

static volatile sig_atomic_t keep_running = 1;
static char interface[IFNAMSIZ] = "eth0";
static float delay_seconds = DEFAULT_DELAY;

void handle_signal(int sig) {
    (void)sig;
    keep_running = 0;
}

struct error_counter {
    time_t last_reset;
    int count;
};

int should_continue_after_error(struct error_counter *counter) {
    time_t now = time(NULL);
    
    if (difftime(now, counter->last_reset) > ERROR_WINDOW) {
        counter->count = 0;
        counter->last_reset = now;
    }
    
    if (++counter->count > MAX_ERRORS) {
        fprintf(stderr, "Critical: Too many errors (%d) in %d seconds. Exiting.\n", 
                MAX_ERRORS, ERROR_WINDOW);
        return 0;
    }
    
    if (counter->count > 1) {
        unsigned delay = 1 << (counter->count < 10 ? counter->count : 10);
        sleep(delay);
    }
    
    return 1;
}

int setup_socket(const char *iface) {
    int sock = socket(AF_PACKET, SOCK_RAW | SOCK_CLOEXEC, htons(ETH_P_ALL));
    if (sock < 0) {
        perror("socket");
        return -1;
    }

    int rcvbuf_size = 1024 * 1024 * 8;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &rcvbuf_size, sizeof(rcvbuf_size)) < 0) {
        perror("setsockopt(SO_RCVBUF)");
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    ifr.ifr_name[IFNAMSIZ - 1] = '\0';
    
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        fprintf(stderr, "Error getting index for interface %s: ", iface);
        perror("ioctl(SIOCGIFINDEX)");
        close(sock);
        return -1;
    }

    struct sockaddr_ll addr;
    memset(&addr, 0, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htons(ETH_P_ALL);
    addr.sll_ifindex = ifr.ifr_ifindex;
    addr.sll_pkttype = PACKET_HOST;
    
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "Bind failed for interface %s: ", iface);
        perror("bind");
        close(sock);
        return -1;
    }

    int flags = fcntl(sock, F_GETFL, 0);
    if (flags < 0) {
        perror("fcntl(F_GETFL)");
        close(sock);
        return -1;
    }
    if (fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl(F_SETFL)");
        close(sock);
        return -1;
    }

    return sock;
}

void print_usage(const char *prog_name) {
    fprintf(stderr, "Usage: %s <interface> [delay_seconds]\n", prog_name);
    fprintf(stderr, "Example: %s eth0\n", prog_name);
    fprintf(stderr, "Example: %s eth0 0.1\n", prog_name);
    fprintf(stderr, "Example: %s eth0 1.5\n", prog_name);
    fprintf(stderr, "Delay must be between %.3f and %.1f seconds\n", MIN_DELAY, MAX_DELAY);
    fprintf(stderr, "Default delay: %.1f seconds\n", DEFAULT_DELAY);
}

int main(int argc, char *argv[]) {
    setvbuf(stdout, NULL, _IONBF, 0);

    if (argc < 2 || argc > 3) {
        print_usage(argv[0]);
        return 1;
    }
    
    size_t iface_len = strnlen(argv[1], IFNAMSIZ);
    if (iface_len == 0 || iface_len >= IFNAMSIZ) {
        fprintf(stderr, "Invalid interface name length\n");
        return 1;
    }
    memcpy(interface, argv[1], iface_len);
    interface[iface_len] = '\0';

    if (argc == 3) {
        char *endptr;
        delay_seconds = strtof(argv[2], &endptr);
        
        if (endptr == argv[2] || *endptr != '\0') {
            fprintf(stderr, "Invalid delay value: %s\n", argv[2]);
            print_usage(argv[0]);
            return 1;
        }
        
        if (delay_seconds < MIN_DELAY || delay_seconds > MAX_DELAY) {
            fprintf(stderr, "Error: Delay must be between %.3f and %.1f seconds\n", 
                    MIN_DELAY, MAX_DELAY);
            print_usage(argv[0]);
            return 1;
        }
    }

    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    signal(SIGPIPE, SIG_IGN);

    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
        rl.rlim_cur = rl.rlim_max;
        setrlimit(RLIMIT_NOFILE, &rl);
    }

    static unsigned char packet[BUFFER_SIZE];
    static char hex_output[HEX_SIZE];
    
    struct error_counter err_counter = {.last_reset = time(NULL), .count = 0};
    
    // Упрощённое вычисление задержки (обрезание до 3 знаков)
    long delay_ms = (long)(delay_seconds * 1000.0f);
    if (delay_ms < 1) delay_ms = 1;
    else if (delay_ms > 10000) delay_ms = 10000;
    
    struct timespec delay_ts = {
        .tv_sec = delay_ms / 1000,
        .tv_nsec = (delay_ms % 1000) * 1000000L  // Добавлен L для 32-бит
    };
    
    fprintf(stderr, "Interface: %s\n", interface);
    fprintf(stderr, "Interval: %ld.%03ld seconds\n", 
            (long)delay_ts.tv_sec, delay_ms % 1000);  // Явное приведение типов
    fprintf(stderr, "Starting packet capture...\n");
    
    while (keep_running) {
        int sock = setup_socket(interface);
        if (sock < 0) {
            if (!should_continue_after_error(&err_counter)) break;
            continue;
        }
        
        while (keep_running) {
            struct timespec req = delay_ts;
            while (nanosleep(&req, &req) == -1 && errno == EINTR && keep_running) {
                // Повторяем при прерывании сигналом
            }

            ssize_t last_len = 0;
            int read_error = 0;
            
            while (1) {
                ssize_t len = recv(sock, packet, BUFFER_SIZE, 0);
                
                if (len > 0) {
                    last_len = len;
                }
                else if (len == 0) {
                    break;
                }
                else {
                    if (errno == EAGAIN || errno == EWOULDBLOCK) {
                        break;
                    }
                    else if (errno == EINTR) {
                        continue;
                    }
                    else {
                        perror("recv");
                        
                        if (errno == ENETDOWN || errno == EBADF || errno == ENOTSOCK) {
                            read_error = 1;
                        }
                        
                        if (!should_continue_after_error(&err_counter)) {
                            keep_running = 0;
                        }
                        break;
                    }
                }
            }
            
            if (read_error) {
                break;
            }
            
            if (last_len > 0) {
                err_counter.count = 0;
                
                char *out_ptr = hex_output;
                for (ssize_t i = 0; i < last_len; i++) {
                    *out_ptr++ = hex_table[packet[i] >> 4];
                    *out_ptr++ = hex_table[packet[i] & 0x0F];
                }
                
                if (fwrite(hex_output, 1, last_len * 2, stdout) != (size_t)(last_len * 2)) {
                    if (ferror(stdout)) {
                        clearerr(stdout);
                        fprintf(stderr, "stdout write error\n");
                    }
                }
            }
        }
        
        if (sock >= 0) close(sock);
    }

    fprintf(stderr, "\nStopping packet capture on interface: %s\n", interface);
    return 0;
}