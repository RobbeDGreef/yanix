#ifndef _ENV_H
#define _ENV_H

char **make_envvars();
char **make_args(int amount, ...);
char **combine_args_env(char **argv, char **env);

#endif /* _ENV_H */