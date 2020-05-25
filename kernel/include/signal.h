#ifndef SIGNAL_H
#define SIGNAL_H

#include <sys/types.h>

// WARNING: the descriptions don't match

#define SIGHUP    1  // Hangup (POSIX)
#define SIGINT    2  // Terminal interrupt (ANSI)
#define SIGQUIT   3  // Terminal quit (POSIX)
#define SIGILL    4  // Illegal instruction (ANSI)
#define SIGTRAP   5  // Trace trap (POSIX)
#define SIGABRT   6  // IOT Trap (4.2 BSD)
#define SIGIOT    6  // BUS error (4.2 BSD)
#define SIGUNUSED 7  // Floating point exception (ANSI)
#define SIGFPE    8  // Kill(can't be caught or ignored) (POSIX)
#define SIGKILL   9  // User defined signal 1 (POSIX)
#define SIGUSR1   10 // Invalid memory segment access (ANSI)
#define SIGSEGV   11 // User defined signal 2 (POSIX)
#define SIGUSR2   12 // Write on a pipe with no reader, Broken pipe (POSIX)
#define SIGPIPE   13 // Alarm clock (POSIX)
#define SIGALRM   14 // Termination (ANSI)
#define SIGTERM   15 // Stack fault
#define SIGSTKFLT 16 // Child process has stopped or exited, changed (POSIX)
#define SIGCHLD   17 // Continue executing, if stopped (POSIX)
#define SIGCONT   18 // Stop executing(can't be caught or ignored) (POSIX)
#define SIGSTOP   19 // Terminal stop signal (POSIX)
#define SIGTSTP   20 // Background process trying to read, from TTY (POSIX)
#define SIGTTIN   21 // Background process trying to write, to TTY (POSIX)

#endif