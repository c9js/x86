from scapy.all import *
import time

iface = 'wlan0'
bssid = '00:11:22:33:44:55'
channel = 6  # Укажи нужный канал

ssid_list = [
    '11111',
    '22222',
    '33333',
    '44444'
]

def send_beacon(ssid):
    dot11 = Dot11(type=0, subtype=8, addr1='ff:ff:ff:ff:ff:ff', addr2=bssid, addr3=bssid)
    beacon = Dot11Beacon(cap='ESS+privacy')
    essid = Dot11Elt(ID='SSID', info=ssid, len=len(ssid))
    dsset = Dot11Elt(ID=3, info=chr(channel))
    frame = RadioTap()/dot11/beacon/essid/dsset
    sendp(frame, iface=iface, verbose=0)

print(f'Transmitting SSIDs on channel {channel} with fixed MAC...')
while True:
    for ssid in ssid_list:
        send_beacon(ssid)
        time.sleep(0.5)
