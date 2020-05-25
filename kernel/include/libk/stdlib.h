#ifndef _STDLIB_H
#define _STDLIB_H 1

//#include <sys/cdefs.h>
#include <limits.h>

// standard library

#ifndef NULL
	#define NULL 0
#endif
#define EXIT_FAILURE 0
#define EXIT_SUCCESS 1
#define RAND_MAX     UINT_MAX // this is defined in limits.h

void abort(void)
	__attribute__((__noreturn__)); // causes an abnormal program termination
void exit(int status)
	__attribute__((__noreturn__)); // causes a program to terminate normally
int system(const char *);          // passes string to command processor

int      abs(int); // returns absolute value
long int labs(long int);

int  rand(void); // returns pseudo random number from 0 to RAND_MAX
void srand(
	unsigned int seed); // this function seeds the random number generator
int atoi(const char *str);

//@todo: a lot of other functions should be implemented

// extra functions:

#endif