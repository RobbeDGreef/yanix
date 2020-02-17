#include <stdio.h>
#include <dirent.h>

extern int getdents(int fd, struct dirent* d, int count);


int main()
{
	printf("I'm doing the best i can okay\n\n");
	getdents(0,0,0);

	return 0;
}
