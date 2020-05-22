#include <unistd.h>
#include <string.h>

void main()
{
	const char *clear_escape = "\e[1;1H\e[J";
	write(STDOUT_FILENO, clear_escape, strlen(clear_escape));
}
