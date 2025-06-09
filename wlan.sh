#!/bin/sh
# Скрипт для установки прошивки Broadcom b43 и подключения к Wi-Fi
# Использование: ./wlan.sh "SSID_сети" "пароль"

set -e

if [ $# -ne 2 ]; then
  echo "Использование: $0 \"SSID\" \"PASSWORD\""
  exit 1
fi

SSID="$1"
PASSWORD="$2"

echo 'nameserver 8.8.8.8' > /etc/resolv.conf
echo 'nameserver 1.1.1.1' >> /etc/resolv.conf

echo 'Обновляем apk и ставим необходимые пакеты...'

apk update
apk add b43-fwcutter wireless-tools iw wpa_supplicant dhcpcd wget tar -X http://dl-cdn.alpinelinux.org/alpine/edge/community/ --no-cache

FIRMWARE_URL='http://www.lwfinger.com/b43-firmware/broadcom-wl-5.100.138.tar.bz2'
FIRMWARE_ARCHIVE='broadcom-wl-5.100.138.tar.bz2'
FIRMWARE_DIR='broadcom-wl-5.100.138'
FIRMWARE_O="$FIRMWARE_DIR/linux/wl_apsta.o"

# Скачиваем прошивку, если нет
if [ ! -f "$FIRMWARE_ARCHIVE" ]; then
  echo 'Скачиваем прошивку Broadcom...'
  wget "$FIRMWARE_URL"
fi

# Распаковываем, если нет папки
if [ ! -d "$FIRMWARE_DIR" ]; then
  echo 'Распаковываем прошивку...'
  tar xjf "$FIRMWARE_ARCHIVE"
fi

echo 'Извлекаем прошивку в /lib/firmware...'
b43-fwcutter -w /lib/firmware "$FIRMWARE_O"

echo 'Перезагружаем модуль b43...'
modprobe -r b43 || true
modprobe b43

echo 'Включаем интерфейс wlan0...'
ip link set wlan0 down || true
ip link set wlan0 up

echo 'Создаём конфиг /etc/wpa_supplicant.conf с вашими данными...'
cat > /etc/wpa_supplicant.conf <<EOF
ctrl_interface=/var/run/wpa_supplicant
ctrl_interface_group=wheel
update_config=1
network={
    ssid="$SSID"
    psk="$PASSWORD"
}
EOF

echo 'Запускаем wpa_supplicant...'
killall wpa_supplicant || true
wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant.conf

echo 'Запускаем dhcpcd для получения IP...'
dhcpcd -k wlan0 || true
dhcpcd -t 10000 wlan0

echo 'Готово! Проверьте подключение командой: ping -c 4 8.8.8.8'

reset
echo 'IP eth0:'
ip -4 addr show eth0 | awk '/inet/ {print $2}' | cut -d'/' -f1
echo 'IP wlan0:'
ip -4 addr show wlan0 | awk '/inet/ {print $2}' | cut -d'/' -f1

ip addr flush dev eth0
# iface eth0 inet manual
