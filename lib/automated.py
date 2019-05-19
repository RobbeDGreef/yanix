


"""
#ifndef _ERRNO_H
#define _ERRNO_H

/**
 * This file contains all the linux error codes 
 */

#define EPERM	1	// Operation not permitted
#define ENOENT 	2 	// No such file or directory
#define ESRCH	3	// No such process
#define EINTR	4	// Interrupted system call
#define EIO 	5 	// I/O error
#define ENXIO	6 	// No such device or address
#define E2BIG 	7 	// Argument list too long
#define ENOEXEC 8 	// Exec format error
#define EBADF 	9 	// Bad file number
#define ECHILD 	10	// No child processes

// @todo: finish this list (automize this from this website: https://mariadb.com/kb/en/library/operating-system-error-codes/)

#endif
"""
with open("errno.h", "w") as f1:
	with open("linux_errcode_list.txt", "r") as f2:
		for line in f2:
			num = ""
			i = 0
			for c in line:
				if c != '\t':
					num += c
					i+=1
				else:
					i+=1
					break
			
			errcodename = ""
			for c in line[i:]:
				if c != '\t':
					errcodename += c
					i+=1
				else:
					i+=1
					break
	
			errcodeuitleg = ""
			for c in line[i:]:
				if c != '\n':
					errcodeuitleg += c
				else:
					break

			f1.write("#define " + errcodename + " "*(20 - len(num) - len(errcodename)) + num + " "*5 + "//" + errcodeuitleg + "\n");
