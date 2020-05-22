#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>
#include <string.h>

extern int errno;

char *cat_help = 
	"Usage: cat [OPTION]... [FILE]...\n"
	"Concatenate FILE(s) to standard output.\n\n"
	"With no FILE, or when FILE is -, read standard input.\n\n"
	"  -n, --number             number all output lines\n"
	"      --help     display this help and exit\n"
	"      --version  output version information and exit\n\n";

int readfile(char *file)
{
	FILE *fp = fopen(file, "r");
	if (!fp)
	{
		int err = errno;
		fprintf(stderr, "cat: %s: %s", file, strerror(err));
		exit(1);
	}

	int c;
	while ((c = fgetc(fp)) != EOF)
		putchar(c);

	return 0;
}


int main(int argc, char **argv)
{
	int f_printLine = 0;

	int optind = 0;
	static struct option long_options[] = 
	{
		{"help", no_argument, 0, 'h'},
		{"number", no_argument, 0, 'n'}
	};

	int c;
	while ((c = getopt_long(argc, argv, "nh", long_options, &optind)) != -1)
	{
		switch (c)
		{
		case 'n':
			f_printLine = 1;
			break;
		
		case 'h':
			printf(cat_help);
			exit(0);

		default:
			fprintf(stderr, "Try 'cat --help' for more information\n");
			exit(1);
		}
	}

	if (++optind < argc)
	{
		while (optind < argc)
			readfile(argv[optind++]);
	}



	return 0;
}