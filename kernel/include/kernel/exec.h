#ifndef EXEC_H
#define EXEC_H

int execve_user(const char *filename, const char **argv, char const **envp);
int execve(const char *filename, const char **argv, char const **envp);

#endif