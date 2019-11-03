import os

lines = 0
filecount = 0
v = False
extensions = [".h", ".c", ".asm", ".py", ".ld", ".md"]

noreaddirs = ["examples", ".git", "sysroot", "sysroot2", "sysroot3", ".testing"]

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
				filecount += 1
				if v:
					print("    reading file: ", file)
				with open(os.path.join(subdir, file), "r") as f:
					lines += len(f.read().split("\n"))

print("total lines: ", lines)
print("total files: ", filecount)
try:
	size = os.path.getsize(os.path.join(os.getcwd(), "os-image.bin"))
	print("os image size: ", str(size) + "B", "(" + str(round(size/1024)) + "kB)")
except os.error:
	print("project not build yet, cannot return os-image size")
