#include <proc/syscalls/syscalls.h>

const void *syscalls_yanix[] = {
	/* 0 */ 0,
	/* 1 */ &sys_exit,    /* DONE */
	/* 2 */ &sys_fork,    /* DONE */
	/* 3 */ &sys_read,    /* DONE */
	/* 4 */ &sys_write,   /* DONE */
	/* 5 */ &sys_open,    /* DONE */
	/* 6 */ &sys_close,   /* DONE */
	/* 7 */ &sys_wait,    /* NOT DONE */
	/* 8 */ &sys_creat,   /* DONE */
	/* 9 */ &sys_link,    /* NOT DONE */
	/* 10 */ &sys_unlink, /* NOT DONE */
	/* 11 */ &sys_execve, /* DONE */
	/* 12 */ &sys_kill,   /* DONE */
	/* 13 */ &sys_lstat,
	/* 14 */ &sys_lseek,    /* DONE */
	/* 15 */ &sys_sbrk,     /* DONE */
	/* 16 */ &sys_times,    /* NOT DONE */
	/* 17 */ &sys_isatty,   /* NOT DONE */
	/* 18 */ &sys_stat,     /* DONE */
	/* 19 */ &sys_signal,   /* NOT DONE */
	/* 20 */ &sys_readdir,  /* ???? */
	/* 21 */ &sys_getdents, /* DONE */
	/* 22 */ &sys_chdir,    /* NOT DONE */
	/* 23 */ &sys_getcwd,   /* NOT DONE */
	/* 24 */ &sys_pipe,     /* NOT DONE */
	/* 25 */ &sys_mkdir,    /* NOT DONE */
	/* 26 */ &sys_fcntl,    /* NOT DONE */
	/* 27 */ &sys_getwd,    /* NOT DONE */
	/* 28 */ &sys_fstat,    /* DONE */
	/* 29 */ &sys_mmap,     /* NOT DONE */
	/* 30 */ &sys_munmap,   /* NOT DONE */
	/* 31 */ &sys_chown,    /* NOT DONE */
	/* 32 */ &sys_sysinfo,  /* NOT DONE */
	/* 33 */ &sys_getuid,
	/* 34 */ &sys_geteuid,
	/* 35 */ &sys_dup2,
	/* 36 */ &sys_dup,
	/* 37 */ &sys_sigaction,
	/* 38 */ &sys_umask,
	/* 39 */ &sys_getpid, /* DONE */
	/* 40 */ &sys_getppid,
	/* 41 */ &sys_pipe2,
	/* 42 */ &sys_getgid,
	/* 43 */ &sys_ioctl,
	/* 44 */ &sys_setuid,
	/* 45 */ &sys_seteuid,
	/* 46 */ &sys_getgroups,
	/* 47 */ &sys_setgroups,
	/* 48 */ &sys_sigprocmask,
	/* 49 */ &sys_getegid,
	/* 50 */ &sys_sigsuspend,
	/* 51 */ &sys_wait3,
	/* 52 */ &sys_gethostname,
	/* 53 */ &sys_sethostname,
	/* 54 */ &sys_alarm,
	/* 55 */ &sys_nanosleep,
	/* 56 */ &sys_fchmod,
	/* 57 */ &sys_setgid,
	/* 58 */ &sys_setregid,
	/* 59 */ &sys_setreuid,
	/* 60 */ &sys_access,
	/* 61 */ &sys_time,
	/* 62 */ &sys_requesterrno,
	/* 63 */ &sys_mknod,
	/* 64 */ &sys_getgrent,
	/* 65 */ &sys_setgrent,
	/* 66 */ &sys_endgrent,
	/* 67 */ &sys_select,
	/* 68 */ &sys_uname,
	/* 69 */ &sys_gettimeofday,

};

const void *syscalls_linux[] = {

};

int syscall_yanix_amount = sizeof syscalls_yanix / sizeof syscalls_yanix[0];
int syscall_linux_amount = sizeof syscalls_linux / sizeof syscalls_linux[0];