#include <libk/string.h>
#include <mm/heap.h>
#include <stdarg.h>
#include <yanix/env.h>
#include <errno.h>
#include <yanix/user.h>

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
	int argvsize = env_size(argv);
	if (argvsize > EXECVE_MAX_ARGS)
	{
		errno = E2BIG;
		return NULL;
	}

	int envsize = env_size(env);
	if (envsize > EXECVE_MAX_ENV)
	{
		errno = E2BIG;
		return NULL;
	}

	int paramsize = argvsize + envsize;
	char **new = kmalloc_user(paramsize * sizeof(char*));
	memset(new, 0, paramsize * sizeof(char*));

	int i;
	for (i = 0; i < argvsize; i++)
		new[i] = strdup_user(argv[i]);
	i--;
	
	for (int j = 0; j < envsize; j++)
		new[i++] = strdup_user(env[j]);

	return new;
}