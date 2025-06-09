#include <node_api.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>

#define MAX_PACKET_SIZE 1514

// Функция для приёма L2 пакета
napi_value RecvPacket(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);

    // Аргумент: имя интерфейса
    char iface[IFNAMSIZ];
    size_t iface_len;
    napi_get_value_string_utf8(env, args[0], iface, sizeof(iface), &iface_len);

    // Создание RAW сокета
    int sock = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sock < 0) {
        napi_throw_error(env, NULL, "Failed to create socket");
        return NULL;
    }

    // Получение индекса интерфейса
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, IFNAMSIZ, "%s", iface);
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        close(sock);
        napi_throw_error(env, NULL, "Failed to get interface index");
        return NULL;
    }

    // Привязка к интерфейсу
    struct sockaddr_ll addr = {
        .sll_family = AF_PACKET,
        .sll_ifindex = ifr.ifr_ifindex
    };
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        close(sock);
        napi_throw_error(env, NULL, "Failed to bind socket");
        return NULL;
    }

    // Приём пакета
    unsigned char buffer[MAX_PACKET_SIZE];
    ssize_t recv_len = recvfrom(sock, buffer, sizeof(buffer), 0, NULL, NULL);
    close(sock);

    if (recv_len < 0) {
        napi_throw_error(env, NULL, "Failed to receive packet");
        return NULL;
    }

    // Возврат как Buffer
    napi_value result;
    napi_create_buffer_copy(env, recv_len, buffer, NULL, &result);
    return result;
}

// Функция для отправки L2 пакета
napi_value SendPacket(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, NULL, NULL);
    
    // Аргумент 0: имя интерфейса (строка)
    char iface[IFNAMSIZ];
    size_t iface_len;
    napi_get_value_string_utf8(env, args[0], iface, sizeof(iface), &iface_len);
    
    // Аргумент 1: буфер с данными
    void* data;
    size_t len;
    napi_get_buffer_info(env, args[1], &data, &len);
    
    // Создание RAW сокета на уровне Ethernet
    int sock = socket(AF_PACKET, SOCK_RAW, 0);
    if (sock < 0) {
        napi_throw_error(env, NULL, "Failed to create socket");
        return NULL;
    }
    
    // Получение индекса интерфейса
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, IFNAMSIZ, "%s", iface);
    
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        close(sock);
        napi_throw_error(env, NULL, "Failed to get interface index");
        return NULL;
    }
    
    struct sockaddr_ll addr = {
        .sll_family = AF_PACKET,
        .sll_ifindex = ifr.ifr_ifindex
    };
    
    // Отправка пакета
    ssize_t sent = sendto(sock, data, len, 0, (struct sockaddr*)&addr, sizeof(addr));
    close(sock);
    
    if (sent < 0) {
        napi_throw_error(env, NULL, "Failed to send packet");
        return NULL;
    }
    
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

// Экспорт
napi_value Init(napi_env env, napi_value exports) {
    napi_value fn;
    napi_create_function(env, NULL, 0, RecvPacket, NULL, &fn);
    napi_set_named_property(env, exports, "recvPacket", fn);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)
