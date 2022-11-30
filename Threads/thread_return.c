#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

void *multiply(void *);

int main(int argc, char *argv[])
{
    printf("Multiply Program, PID: %d\n", getpid());

    int val = 2345;

    int *ret;
    pthread_t thread;
    pthread_create(&thread, NULL, multiply, &val);
    pthread_join(thread, (void *)&ret);

    printf("Return value: %d\n", *ret);
}

void *multiply(void *param)
{
    int *val = (int *)param;
    printf("Arg Passed: %d\n", *(val));
    (*val) = -(*val);
    pthread_exit((void *)val);
}