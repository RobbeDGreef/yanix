#include <stdio.h>
#include <unistd.h>

int main()
{
        printf("Going to fork\n");
        if (fork() == 0)
                printf("Child running\n");
        else
                printf("Parent running\n");

        printf("Leaving now\n");
}
