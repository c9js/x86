from scapy.all import *

def handle(pkt):
    if pkt.haslayer(Dot11Beacon):
        ssid = pkt[Dot11Elt].info
        bssid = pkt[Dot11].addr2
        print(f"{bssid} -> SSID: {ssid} ({len(ssid)} bytes)")

sniff(iface='wlan0', prn=handle, store=0, monitor=True)