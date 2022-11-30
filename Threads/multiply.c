#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

/**
 * 2. String Multiplication
 * Ruparel
 */

// globals and flag
int tpause = 1;
int n;
char *a;
char *b;

int num_digits(char *a, char *b)
{
    // if the lengths are equal, return the length
    if (strlen(a) == strlen(b))
        return strlen(a);

    // else return 0
    return 0;
}

// prototype for the function that threads will use.
void *multiply(void *);

int main(int argc, char *argv[])
{
    // check if we have enough arguements
    if (argc != 3)
    {
        printf("Not enough numbers.\n");
        return 0;
    }

    /** Uncomment to use integer multiplication
    int n1 = atoi(argv[1]);
    int n2 = atoi(argv[2]);
    printf("%d x %d = %d\n", n1, n2, n1 * n2);
    */

    a = argv[1]; // save the arguements, first number
    b = argv[2]; // second number

    // find number of digits and catch errors
    n = num_digits(a, b);
    if (n == 0)
    {
        printf("Can't multiply %s and %s.\n", a, b);
    }

    // array to store thread_ids
    unsigned long thread_ids[n];

    // main thread creation program
    for (int i = 0; i < 2 * n; i++)
    {
        // get a new int memory location for parameter passing to the thread
        int *s = malloc(sizeof(int));
        *s = 2 * n - i - 1; // the ith thread will handle (2 * n - i - 1)th thread

        // parameters to make a new thread
        pthread_t thread;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_create(&thread, &attr, multiply, (void *)s); // set the callback function and arguements

        // save this thread's id in our table (unsigned long)
        thread_ids[i] = thread;
    }

    // reset the flag variables so that threads can start exec (may not be needed)
    tpause = 0;

    // int array to save the outputs of each thread
    int *prod = malloc(2 * n * sizeof(int));

    for (int i = 0; i < 2 * n; i++)
    {
        // get return values from each thread
        int *ret;
        pthread_join(thread_ids[i], (void *)&ret);
        // printf("T%d: %d\n", i, *ret);

        // save them in the array
        prod[i] = (*ret);
    }

    // this is the actual computation
    for (int i = 1; i < 2 * n; i++)
    {
        prod[i] += prod[i - 1] / 10; // get carry value of the prev digit
        prod[i - 1] %= 10;           // assign the digit value to the prev digit
    }

    // buffer to store the result
    char buf[500];
    for (int i = 1; i < 2 * n; i++)
    {
        // using strings as array of char
        buf[2 * n - i] = prod[i] + '0'; // int to char conversion
    }
    // print the final answer
    printf("%s x %s = %s\n", a, b, buf + 1);
}

// actual multiply function
void *multiply(void *param)
{
    // wait till main thread does not reset flag
    while (tpause)
        ;

    // get the param value
    int *d = (int *)param;
    // printf("Digit %d thread.\n", *d);

    // digit 0 can be formed in how many ways ... 0 + 0
    // 1 --> (1 + 0), (0 + 1) and so on

    // variable to store the result
    int *sum = malloc(sizeof(int));
    sum[0] = 0;

    // main comp.
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            // consider all cases where multiplication alters the d'th digit of answer
            if (i + j == *d)
            {
                // this means we have to consider this value
                sum[0] += (a[i] - '0') * (b[j] - '0');
            }
        }
    }

    // exit with return value.
    pthread_exit((void *)sum);
}
