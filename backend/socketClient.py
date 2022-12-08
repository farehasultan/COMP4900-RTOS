import socket
import keyboard #Non-standard, requires pip download
import struct
import time

HOST = "192.168.56.101"  # Address assigned by host bridge for QNX VM
PORT = 6000  # The port used by the server
throttle = 0.0

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    while True:
        if keyboard.is_pressed('w'):
            throttle = 1 if throttle >= 0.9 else throttle + 0.1
            time.sleep(1)
        if keyboard.is_pressed('s'):
            throttle = 0 if throttle <= 0.1 else throttle - 0.1
            time.sleep(1)
        s.sendall(struct.pack("<1f", throttle))
        data = struct.unpack("<1d", s.recv(8))
        print(f"{data} RPM\n")
        
