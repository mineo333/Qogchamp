from scapy.all import *
import socket

#run using superuser


while True:
    conf.L3socket=L3RawSocket
    packet=IP(dst="localhost")/UDP(sport=10000,dport=42069)/Raw("Qogchamp")
    send(packet)