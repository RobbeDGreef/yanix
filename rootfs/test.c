#include <stdio.h>
#include <stdlib.h>

int main()
{
	FILE *fp = fopen("/test.c", "r");
	if (fp == 0)
	{
		perror("Error while opening the file");
		exit(1);
	}
	printf("File contents:\n");

	char ch;
	while ((ch = fgetc(fp)) != '\0')
	{
		printf("%c", ch);
	}
	fclose(fp);
	return 0;
}
