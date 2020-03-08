#include <libk/string.h>
#include <mm/heap.h>
#include <stdarg.h>

/**
 * @todo  	Environment variables should not be a hardcoded amount  
 */
#define ENVIRONMENT_AMOUNT 256

size_t env_size(char **vars)
{
	size_t amount = 0;
	while (vars[amount++] != 0);
	return amount;
}

char *make_userstring(char *string)
{
	char *us = kmalloc_user(sizeof(string));
	memcpy(us, string, strlen(string));
	return us;
}

char **make_envvars()
{
	char **env = kmalloc_user(sizeof(char*)*ENVIRONMENT_AMOUNT);
	memset(env, 0, sizeof(char*) * ENVIRONMENT_AMOUNT);
	env[0] = make_userstring("USER=root");
	env[1] = make_userstring("PATH=/bin");
	env[2] = make_userstring("HOME=/root");
	return env;
}


/**
 * @brief      Makes userspace readable arguments.
 *
 * @param[in]  amount     The amount of arguments
 * @param[in]  ... 		  The arguments
 *
 * @return     Pointer to argument list
 */
char **make_args(int amount, ...)
{
	char **args = kmalloc_user(sizeof(char*) * (amount+1));
	memset(args, 0, sizeof(char*) * (amount+1));

	va_list valist;
	va_start(valist, amount);

	for (int i = 0; i < amount; i++)
	{
		args[i] = make_userstring(va_arg(valist, char*));
	}

	va_end(valist);

	return args;
}

char **combine_args_env(char **argv, char **env)
{
	/* @todo: This is crap please find a better solution for arguments and environment vars */
	char **new = kmalloc_user(sizeof(char*) * ENVIRONMENT_AMOUNT);
	memset(new, 0, sizeof(char*) * ENVIRONMENT_AMOUNT);
	memcpy(new, argv, env_size(argv) * sizeof(char*));
	memcpy(new + env_size(argv)-1, env, env_size(env) * sizeof(char*));

	return new;
}