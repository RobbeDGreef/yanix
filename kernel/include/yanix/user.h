#ifndef _USER_H
#define _USER_H

#include <sys/types.h>

#define USERFILE_LOCATION "/etc/passwd"
#define LOGIN_LEN         128

struct user
{
	char *name;
	char *gcos;
	uid_t uid;
	gid_t gid;
	char *home;
	char *shell;
	int   tty;
};

int          user_login(char *name, char *passwd);
int          init_user();
int          login();
struct user *get_current_user();

#endif /* _USER_H */