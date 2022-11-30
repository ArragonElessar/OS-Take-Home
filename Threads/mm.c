#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

/**
 * 1. Matrix Multiplication
 * Ruparel
 */

// declare all the variables globally
int m = 3, n = 3, k = 3;
int A[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
int B[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
int C[3][3];
unsigned long thread_ids[4][4]; // array to store the id's of all the threads acc to their params
int tpause = 1;                 // used for locking (SIGSTOP, SIGCONT) etc

// making a tuple struct for ease of use
struct tuple
{
    int x;
    int y;
};

//
void *matrix_multiply(void *); // prototype for calling the thread which will create other threads.
void *row_col(void *);         // function for the sub_threads

int main()
{
    printf("Main thread: PID: %d\n", getpid()); // Just so we know

    pthread_t tid;                                      // declare the thread identifier
    pthread_attr_t attr;                                // Declare the attr struct
    pthread_attr_init(&attr);                           // Init the attr struct
    pthread_create(&tid, &attr, matrix_multiply, "25"); // Create the thread

    pthread_join(tid, NULL); // Wait for the thread to exit

    // print the result C matrix
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < k; j++)
        {
            printf("%d  ", C[i][j]);
            // printf("%lu ", thread_ids[i][j]);
        }
        printf("\n");
    }
}

void *matrix_multiply(void *param)
{
    // this thread will create multiple threads each for one square of the resulting matrix

    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < k; j++)
        {
            // this is the parameter that we will pass to each of the sub_threads
            struct tuple *t = malloc(sizeof(struct tuple));
            t->y = i;
            t->x = j;

            // create thread, and pass the parameter
            pthread_t sub_tid;
            pthread_attr_t sub_attr;
            pthread_attr_init(&sub_attr);
            pthread_create(&sub_tid, &sub_attr, row_col, t);

            // save the tid of this thread into the thread_ids array, used for joining later
            thread_ids[i][j] = sub_tid;
        }
    }
    // (unlock) Release all the processes
    tpause = 0;

    // wait for each thread to return and then join them
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < k; j++)
        {
            // collect the return value for each thread
            int *ret;
            pthread_join(thread_ids[i][j], (void *)&ret);
            C[i][j] = *ret;
        }
    }
    // exit the thread creator
    pthread_exit(0);
}

// the sub_threads will calculate the value of one square of the resulting matrix
void *row_col(void *param)
{
    // wait till all the sub-threads are created.
    while (tpause)
        ;

    // find which position of the matrix this thread will find the value of
    struct tuple *pos = (struct tuple *)param;
    int row = pos->y;
    int col = pos->x;

    // This type structure is important for returning the value in the right way
    int *sum = malloc(sizeof(int));
    sum[0] = 0;
    for (int i = 0; i < k; i++)
    {
        sum[0] += A[row][i] * B[i][col];
    }
    pthread_exit((void *)sum); // return to the main thread with the correct value
}
