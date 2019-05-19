#! /usr/bin/python3
import sys, struct

size = 80*1024	# 80 KB

def test():
	print(size)
	with open("ramdisk.iso", 'wb') as f:
		# now padding
		f.write(b"\x00"*(size))
		f.close()

def main():
	files = sys.argv[1:]
	print("creating ramdisk with files:", ", ".join(files))

	# filesystem layout:
	"""
	alright so let's do this
	we will follow the traditional unix filesystem way 
	or we'll try that because i do not really now how they did it but thats not importand 
	the importand part being we are creating our own os so we do stuff how we want it 
	first part the inode struct:

	inode:
		
		
	"""
	for file in files:
		content = open(file, 'r').read()
		print(content)


def padding():
	size = int(sys.argv[1])
	a = ["a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l",
		 "m", "n", "o", "p", "q", "r", "s", "t", "u", "v" "w", "x",
		 "y", "z"]
	with open("padding", "wb") as f:
		i = 0
		for x in range(int(size/0x200)):
			f.write((a[i]*0x200).encode())
			i+=1
			if i == len(a):
				i = 0
#test()
padding()
#main()