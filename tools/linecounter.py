import os

lines = 0
v = True
extensions = [".h", ".c", ".asm", ".py"]

noreaddirs = ["examples"]

def splitpath(path):
	p = []
	b = ""
	for c in path:
		if c == "/":
			p.append(b)
			b = ""
		else:
			b += c
	return p

def checkpath(path):
	for n in noreaddirs:
		if n in splitpath(path):
			return True
	return False

for subdir, dirs, files in os.walk(os.getcwd()):

	if checkpath(subdir):
		if v:
			print("skipping: ", subdir)
		continue	

	if v:
		print("reading: ", subdir)

	for file in files:
		for ext in extensions:
			if file.endswith(ext):
				# how many lines
				if v:
					print("    reading file: ", file)
				with open(os.path.join(subdir, file), "r") as f:
					lines += len(f.read().split("\n"))

print("total lines: ", lines)
try:
	size = os.path.getsize(os.path.join(os.getcwd(), "os-image.bin"))
	print("os image size: ", str(size) + "B", "(" + str(round(size/1024)) + "kB)")
except os.error:
	print("project not build yet, cannot return os-image size")
