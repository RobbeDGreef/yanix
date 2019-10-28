import sys
import socket

print("creating a socket")
s = socket.socket()
s.connect((sys.argv[1], int(sys.argv[2])))
s.send(b"Hello world!")
