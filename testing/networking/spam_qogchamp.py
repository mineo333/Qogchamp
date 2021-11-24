from scapy.all import *
import socket

#run using superuser

conf.L3socket=L3RawSocket
conf.use_pcap=True
while True:
    packet=IP(dst="localhost")/UDP(sport=10000,dport=42069)/Raw("Qogchamp")
    send(packet)