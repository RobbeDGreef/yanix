#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>

int main()
{
	printf("Lets try this\n");
	struct dirent *dir;
	DIR *dp = opendir("/");
	if (dp == 0)
	{
		printf("Could not open directory");
		exit(1);
	}
	while ((dir = readdir(dp)) != 0)
	{
		printf("%s\n", dir->d_name);
	}
	closedir(dp);
	return 0;
}
