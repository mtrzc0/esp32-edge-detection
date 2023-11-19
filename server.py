# write function which will listen udp segment on port 8765 and print the data
# received on the terminal
# Use python3
# Hint: use socket library

import socket

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(('192.168.43.170' , 8765))
while True:
    data, addr = s.recvfrom(1024*20)
    print("received message: %s" % data)
    print("from: %s" % addr)
