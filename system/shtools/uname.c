#include <getopt.h>
#include <sys/utsname.h>
#include <stdio.h>
#include <stdlib.h>

#define SYSNAME 1
#define RELEASE 2
#define VERSION 3
#define MACHINE 4

char *help_text = 
"Usage: uname [OPTION]...\n"
"Print certain system information.  With no OPTION, same as -s.\n"
"\n"
"  -a, --all                print all information, in the following order,\n"
"                           except omit -p and -i if unknown:\n"
"  -s, --kernel-name        print the kernel name\n"
"  -r, --kernel-release     print the kernel release\n"
"  -v, --kernel-version     print the kernel version\n"
"  -m, --machine            print the machine hardware name\n"
"  -o, --operating-system   print the operating system\n"
"      --help     display this help and exit\n"
"      --version  output version information and exit\n"
"\n"
"yanix shell utilities\n";

int print_help()
{
	printf(help_text);
	return 0;
}

int print_sysinfo(int infotype)
{
	struct utsname info;
	uname(&info);

	switch (infotype)
	{
	case SYSNAME:
		printf("%s", info.sysname);
		break;

	case RELEASE:
		printf("%s", info.release);
		break;

	case VERSION:
		printf("%s", info.version);
		break;

	case MACHINE:
		printf("%s", info.machine);
		break;
	}

	return 0;
}

int print_all()
{
	print_sysinfo(SYSNAME);
	printf(" ");
	print_sysinfo(VERSION);
	printf(" ");
	print_sysinfo(RELEASE);
	printf(" ");
	print_sysinfo(MACHINE);
	printf("\n");
}

int print_version()
{
	printf("uname (yanix shell tools) 0.0.1\n");
	return 0;
}

struct option options[] = 
{
	{"all",				 no_argument, 0, 'a'},
	{"kernel-name",		 no_argument, 0, 's'},
	{"kernel-release",	 no_argument, 0, 'r'},
	{"kernel-version", 	 no_argument, 0, 'v'},
	{"machine", 		 no_argument, 0, 'm'},
	{"operating-system", no_argument, 0, 'o'},
	{"help",			 no_argument, 0, 'h'},
	{"version", 		 no_argument, 0, 0}
};


int main(int argc, char **argv)
{
	if (argc == 1)
	{
		print_sysinfo(SYSNAME);
		printf("\n");
		exit(0);
	}

	int c;
	int optindex;
	while ((c = getopt_long(argc, argv, "asrvmoh", options, &optindex)) != -1)
	{
		switch (c)
		{
		case 'a':
			print_all();
			exit(0);
			break;

		case 's':
			print_sysinfo(SYSNAME);
			break;

		case 'r':
			print_sysinfo(VERSION);
			break;

		case 'v':
			print_sysinfo(RELEASE);
			break;

		case 'm':
			print_sysinfo(MACHINE);
			break;

		case 'o':
			print_sysinfo(SYSNAME);
			break;

		case 'h':
			print_help();
			exit(0);

		case 0:
			if (optindex == 7)
				exit(print_version());

		case '?':
			printf("Try 'uname --help' for more information.\n");
			exit(1);
		}
		printf(" ");
	}
	printf("\n");

	return 0;
}