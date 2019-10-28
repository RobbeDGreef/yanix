import os, sys

print("Runs from parent dir: ", os.getcwd())

HEADERTOFIX = sys.argv[1]
NEWHEADER = sys.argv[2]

for subdir, dirs, files in os.walk(os.getcwd()):
	for file in files:
		if file.endswith(".c") or file.endswith(".h"):
			print("file: ", os.path.join(subdir, file))
			with open(os.path.join(subdir, file), "r") as f:
				content = f.read().split("\n")
				for i, line in enumerate(content):
					if line.startswith("#include "):
						if line.endswith(HEADERTOFIX):
							content[i] = "#include " + NEWHEADER
				f.close()
			with open(os.path.join(subdir, file), "w+") as f:
				f.write("\n".join(content))
				f.close()
