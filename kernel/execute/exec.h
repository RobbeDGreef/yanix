#ifndef EXEC_H
#define EXEC_H

int execve_user(const char *filename, char *const argv[], char *const envp[]);
int execve(const char *filename, char *const argv[], char *const envp[]);

#endif