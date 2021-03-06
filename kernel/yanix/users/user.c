#include <debug.h>
#include <errno.h>
#include <fcntl.h>
#include <fs/vfs.h>
#include <libk/stdio.h>
#include <libk/stdlib.h>
#include <libk/string.h>
#include <mm/heap.h>
#include <kernel/user.h>

struct user *g_current_user;

int evalpasswd(char *given, char *correct)
{
	if (correct[strlen(given)] != ':')
		return 1;

	return memcmp(given, correct, strlen(given) - 1);
}

int user_login(char *name, char *passwd)
{
	char *filebuf = kmalloc(BUFSIZ);
	char *line    = kmalloc(LOGIN_LEN);
	int   namelen = strlen(name);

	struct file *fp = vfs_open(USERFILE_LOCATION, 0, 0);
	if (!fp)
	{
		kfree(filebuf);
		kfree(line);
		return -1;
	}

	vfs_read_fd(fp->fd, filebuf, BUFSIZ);

	int seek = 0;
	int ret  = 0;
	while ((ret = readline(filebuf + seek, BUFSIZ - seek, line, LOGIN_LEN))
	       != EOF)
	{
		char *buf = line;
		if (!memcmp(buf, name, namelen) && buf[namelen] == ':')
		{
			if (evalpasswd(passwd, buf + namelen + 1))
			{
				errno = EACCES;
				kfree(filebuf);
				kfree(line);
				vfs_close(fp);
				return -1;
			}

			/* Cutting of newline */
			buf[strlen(buf) - 1] = '\0';

			int end = strlen(name) + strlen(passwd) + 2;
			buf += end;
			g_current_user->uid   = atoi(buf);
			buf                   = strchr(buf, ':') + 1;
			g_current_user->gid   = atoi(buf);
			buf                   = strchr(buf, ':') + 1;
			g_current_user->name  = strdup(name);
			g_current_user->gcos  = strdup_s(buf, strchr(buf, ':') - buf);
			buf                   = strchr(buf, ':') + 1;
			g_current_user->home  = strdup_s(buf, strchr(buf, ':') - buf);
			buf                   = strchr(buf, ':') + 1;
			g_current_user->shell = strdup(buf);

			kfree(filebuf);
			kfree(line);
			vfs_close(fp);
			return 0;
		}

		seek += ret;
	}

	kfree(filebuf);
	kfree(line);
	vfs_close(fp);
	return -1;
}

int init_user()
{
	g_current_user = kmalloc(sizeof(struct user));
	return !g_current_user;
}

struct user *get_current_user()
{
	return g_current_user;
}