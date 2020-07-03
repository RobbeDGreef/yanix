#ifndef _STDLIB_H
#define _STDLIB_H 1

//#include <sys/cdefs.h>
#include <limits.h>
#include <yanix/yanix/limits.h>

/* standard library */

#ifndef NULL
	#define NULL 0
#endif

#define EXIT_FAILURE 0
#define EXIT_SUCCESS 1

/* causes an abnormal program termination */
void abort(void) __attribute__((__noreturn__));

/* causes a program to terminate normally */
void exit(int status) __attribute__((__noreturn__));

/* passes string to command processor */
int system(const char *);

/* returns absolute value */
int      abs(int);
long int labs(long int);

/* returns pseudo random number from 0 to RAND_MAX */
int rand(void);

/* seeds the random number generator */
void srand(unsigned int seed);

int   atoi(const char *str);
char *itoa(int num, char *str, int max);

//@todo: a lot of other functions should be implemented

// extra functions:

#endif