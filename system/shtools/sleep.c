#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc == 1)
	{
		printf("Sleep missing opperant\n");
	}

	sleep(atoi(argv[1]));
}