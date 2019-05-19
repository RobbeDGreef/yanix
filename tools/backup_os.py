#! /usr/bin/python3
import os

print("Backuping os...")
print("Note that the name will start with OS_BACKUP_DAY_MONTH")
name = input("Name: ")
print("seperate fixes with a ','")
fixes = input("Fixtext: ")
with open("./fixes", "w+") as f:
	for line in fixes.split(","):
		f.write(line+"\n")
os.system("zip -r ../backups/`date +'os_backup_%d_%m_" + name + "'` ./")
