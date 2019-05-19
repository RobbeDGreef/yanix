filename = input("Filename > ")
filesize = input("Filesize (bytes) > "); filesize = int(filesize)
s = input("string: (if left blank none will be included)")

if s != "":
	size = filesize - len(s)
else:
	size = filesize

with open(filename, 'wb') as f:
	f.write(s.encode())
	f.write('\x00'.encode()*size)
	f.close()