#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

extern int errno;

/**
 * This is a terrible shell written in a couple of minutes
 * just to have something without porting too much stuff
*/

#define MAXARGS 16
#define SIZE(x) (sizeof x / sizeof x[0])

int 	g_running = 1;
char 	g_buffer[BUFSIZ];
char 	*g_args[MAXARGS];

int builtin_exit(char *const *args)
{
	printf("Goodbye\n");
	exit(0);
}

typedef int (*builtin_fpointer) (char *const *args);
struct builtin_lookup
{
	char *name;
	builtin_fpointer func;
};

const struct builtin_lookup builtins[] = 
{
	{"exit", &builtin_exit}
};

int builtin(char *const *args)
{
	for (int i = 0; i < SIZE(builtins); i++)
	{
		if (!strcmp(builtins[i].name, args[0]))
			return builtins[i].func(args);
	}

	return -1;
}

void execute_command(char* const *args)
{
	int pid;
	int status;

	if (builtin(args) >= 0)
		return;

	if ((pid = fork()) == 0)
	{
		int err = execvp(args[0], args);
		printf("Error: %s\n", strerror(errno));
		exit(errno);
	}
	else
		wait(&status);
}

char *create_string(char *str, int len)
{
	if (str[len-1] == '\n')
		len--;
	char *newstr = malloc(len+1);
	memcpy(newstr, str, len);
	newstr[len] = '\0';
	return newstr;
}

char **split_command(char *cmd)
{
	char *lastptr = cmd;
	int argcnt = 0;
	int len = 0;

	while (1)
	{
		if (*cmd == ' ' || *cmd == '\0')
		{
			if (argcnt == MAXARGS)
			{
				printf("Error, too many arguments\n");
				return NULL;
			}
			g_args[argcnt++] = create_string(lastptr, len);

			if (*cmd == '\0')
			{
				g_args[argcnt] = 0;
				return g_args;
			}

			len = 0;
			lastptr = ++cmd;
			continue;
		}
		cmd++;
		len++;
	}

	printf("Error\n");

	return NULL;
}

void mainloop()
{
	char **args = NULL;
	while (1)
	{
		printf("yash > ");
		fflush(stdout);
		fgets(g_buffer, BUFSIZ, stdin);
		args = split_command(g_buffer);
		if (!args)
			continue;
		execute_command(args);
	}
}

int main()
{
	printf("Hi :)\n");
	mainloop();
	return 0;
}