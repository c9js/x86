#┌────────────────┐
#│ Добавляем цвет │
#└────────────────┘
function color_my_prompt {
# Список цветов
    local BLUE='\[\e[1;34m\]'        # Синий (тёмный)
    local GREEN_LIGHT='\[\e[1;32m\]' # Зеленый (светлый)
    local CLEAR='\[\e[0m\]'          # Сброс цвета
    
# Обновляем основной вид приглашения командной строки
    PS1="$GREEN_LIGHT\u$CLEAR:$BLUE\w$CLEAR$ "
}
export PROMPT_COMMAND=color_my_prompt

#┌──────────────────────────────┐
#│ Отключаем сохранение истории │
#└──────────────────────────────┘
export HISTFILE=/dev/null

#┌────────────────┐
#│ Список алиасов │
#└────────────────┘
alias c='reset'
alias l='c && ls -AFhl --group-directories-first'

#┌─────────────┐
#│ Приветствие │
#└─────────────┘
c
echo 'Добро пожаловать на сервер!'

#┌─────────────────────────────┐
#│ Переходим в рабочий каталог │
#└─────────────────────────────┘
cd "$PATH_WORKSPACE"


# apk add firmware-b43 wireless-tools wpa_supplicant iw dhclient -X http://dl-cdn.alpinelinux.org/alpine/v3.21/community/ --no-cache
# apk add dhcpcd





# apk add py3-pip -X http://dl-cdn.alpinelinux.org/alpine/v3.21/community/ --no-cache
# apk add python3
# pip install --break-system-packages scapy

# ip link set wlan0 down
# iw dev wlan0 set type monitor
# iw dev wlan0 set monitor control
# ip link set wlan0 up

c
# iw dev wlan0 set channel 6
# python3 send.py
# python3 scan.py

# airodump-ng wlan0
# airmon-ng start wlan0

# apk add pciutils

# cd 'mdk4/src'

# apk update
# Драйвера
# apk add wireless-tools wpa_supplicant iw linux-firmware usbutils

# apk add iw wireless-tools tcpdump

# apk add build-base libpcap-dev libnl3-dev linux-headers iw
# make clean
# make

# apk add aircrack-ng -X http://dl-cdn.alpinelinux.org/alpine/v3.21/community/ --no-cache
# iw dev wlan0 scan | grep SSID

# apk add macchanger -X http://dl-cdn.alpinelinux.org/alpine/v3.21/community/ --no-cache
# ip link set wlan0 down
# macchanger -m 00:11:22:33:44:55 wlan0
# ip link set wlan0 up

# ip link set dev wlan0 address 00:11:22:33:44:55