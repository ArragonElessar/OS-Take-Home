#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <stdlib.h>

void sigint_handler(int sig)
{
    printf("SIGINT Recieved, but no fucks given.\n");
}

int main()
{

    struct sigaction sa;

    sa.sa_handler = SIG_DFL;  // declare the SIGCONT handler function
    sa.sa_flags = 0;          // set all the flags to zero
    sigemptyset(&sa.sa_mask); // clear the signal masks

    if (sigaction(SIGINT, &sa, NULL) == -1)
        perror("Error recieving SIGCONT.");

    for (int i = 0; i < 100; i++)
    {
        printf("i: %d\n", i);
        sleep(1);
    }
}