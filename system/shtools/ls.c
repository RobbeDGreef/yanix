#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
extern int errno;

void lsdir(char *path)
{
	DIR *          d = opendir(path);
	struct dirent *dir;

	if (!d)
	{
		printf("ls: cannot access '%s': %s\n", path, strerror(errno));
		exit(1);
	}

	while ((dir = readdir(d)) != NULL)
	{
		printf("    %s\n", dir->d_name);
	}
}

int main(int argc, char **argv)
{
	if (argc > 1)
		lsdir(argv[1]);
	else
	{
		char *buf = malloc(BUFSIZ);

		if (!getcwd(buf, BUFSIZ))
		{
			free(buf);
			return -1;
		}

		lsdir(buf);
		free(buf);
	}

	return 0;
}