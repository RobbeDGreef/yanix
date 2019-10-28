#include <unistd.h>

int main(int argc, char const *argv[])
{
	char *msg = "Yeah it ran";
	write(1, msg, 12);
	return 0;
}
