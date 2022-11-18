import socket

HOST = "192.168.56.101"  # Address assigned by host bridge for QNX VM
PORT = 6000  # The port used by the server

with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    s.sendall(b"test")
    data = s.recv(1024)

print(f"Received {data!r}")
