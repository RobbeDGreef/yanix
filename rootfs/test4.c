#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

int main()
{
	printf("Lets try this\n");
	struct dirent *dir;
	DIR *dp = opendir("/.");
	if (dp == 0)
	{
		printf("Could not open directory");
		exit(1);
	}

	printf("DIR location: %x\n",dp);

	while ((dir = readdir(dp)) != 0)
	{
		printf("inode %i name %s\n", dir->d_ino, dir->d_name);
	}
	closedir(dp);
	return 0;
}
