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
	/* 13 */ 0,
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
	/* 33 */ 0,
	/* 34 */ 0,
	/* 35 */ 0,
	/* 36 */ 0,
	/* 37 */ 0,
	/* 38 */ 0,
	/* 39 */ &sys_getpid, /* DONE */
};

const void *syscalls_linux[] = {

};

int syscall_yanix_amount = sizeof syscalls_yanix / sizeof syscalls_yanix[0];
int syscall_linux_amount = sizeof syscalls_linux / sizeof syscalls_linux[0];