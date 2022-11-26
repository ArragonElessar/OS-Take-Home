#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <signal.h>

/**
 * Right now, it is hardcoded for depth 2
 * Later I will implement the recursive version so that we can have whatever depth required
 * Adding extra comments for readability
 */

// this is the SIGCONT Handler function, [OPTIONAL]
void sigcont_handler(int sig)
{
    write(1, "SIGCONT received\n", 18);
}

// Define a tuple for use
struct tuple
{
    int y;
    int x;
};

// given an index in the shared memory, find out the (x, y) in our required process tree
struct tuple get_pos(int *arr, int size, int idx)
{

    struct tuple pos;

    if (idx == 0)
    {
        pos.x = 0, pos.y = 0;
        return pos;
    }

    int i, lo, hi;
    for (i = 0; i < size - 1; i++)
    {
        lo = arr[i];
        hi = arr[i + 1];
        if (idx >= lo && idx <= hi)
            break;
    }
    pos.x = idx - lo - 1;
    pos.y = i + 1;

    return pos;
}

int main(int argc, char *argv[])
{
    // check if we have the right number of arguments
    if (argc != 3)
    {
        printf("Invalid number of arguments.");
        return -1;
    }

    // now we have the right number of arguments, check if they are both integers
    for (int i = 1; i < argc; i++)
    {
        for (int j = 0; argv[i][j] != '\0'; j++)
        {
            if (!isdigit(argv[i][j]))
            {
                printf("Invalid argument.");
                return -1;
            }
        }
    }

    // now we have 2 integer arguements
    int n = atoi(argv[1]);
    int h = atoi(argv[2]);

    // max children
    int max_children = h * h * (h + 1) / 2;

    // jump idx contains the locations where our depth will increase by one in the tree
    int *jump_idx = (int *)malloc((h + 1) * sizeof(int));
    jump_idx[0] = 0;
    jump_idx[1] = 2;

    for (int i = 2; i <= h; i++)
    {
        jump_idx[i] = jump_idx[i - 1] + ((i) * (i + 1) / 2);
    }

    // setting up sigaction and handler
    void sigcont_handler(int sig);
    struct sigaction sa;

    sa.sa_handler = sigcont_handler; // declare the SIGCONT handler function
    sa.sa_flags = 0;                 // set all the flags to zero
    sigemptyset(&sa.sa_mask);        // clear the signal masks

    // set up the shared memory
    key_t key;
    int shmid;
    int *sh;

    key = ftok("warmup.c", '1');                 // get key for shared memory
    shmid = shmget(key, 1024, 0644 | IPC_CREAT); // get the pointer to shared memory
    sh = shmat(shmid, (void *)0, 0);             // attach a pointer to the first location of shared memory

    // catch any error with attaching to SM
    if (sh == (int *)(-1))
    {
        printf("Error Attaching to shared memory.\n");
        return -1;
    }

    // pid var
    pid_t pid;

    // Root process identifier
    printf("Root process id: %d\n", getpid());

    sh[0] = 1;        // process counter
    sh[1] = getpid(); // write the parent process' pid in the first slot

    // loop n times to make n children at the first lebel
    for (int i = 0; i < n; i++)
    {
        pid = fork(); // fork
        if (pid)
        {
            // parent executes here
            sh[0]++;         // increment the process counter
            sh[sh[0]] = pid; // store the pid of the newly created process here

            // make this child wait and catch errors if any
            if (kill(pid, SIGSTOP) == -1)
                perror("Couldn't send SIGSTOP.\n");
        }
        else
        {
            // child executes here
            if (sigaction(SIGCONT, &sa, NULL) == -1)
                perror("Error recieving SIGCONT.");

            // find the index of this child's pid in the SM
            int idx;
            for (int j = 1; j < max_children; j++)
            {
                if (sh[j] == getpid())
                {
                    // this is the entry of this child in the shared memory
                    idx = j;
                    break;
                }
            }

            // get (x, y) corresponding to the given idx in the SM
            struct tuple pos = get_pos(jump_idx, h + 1, idx - 1);
            printf("I am child: %d, at pos: (%d, %d), creating %d children\n", getpid(), pos.x, pos.y, pos.x + 1);

            pid_t cpid;                         // repeat of all the code above
            for (int j = 0; j < pos.x + 1; j++) // every child will create x + 1 more children
            {
                cpid = fork();
                if (cpid)
                {
                    // add this to the sh table
                    sh[0]++;
                    sh[sh[0]] = cpid;

                    // stop this child as well
                    kill(cpid, SIGSTOP);
                }
                else
                {
                    // child executes here
                    if (sigaction(SIGCONT, &sa, NULL) == -1)
                        perror("Error recieving SIGCONT.");

                    int new_idx;
                    for (int k = 1; k < max_children; k++)
                    {
                        if (sh[k] == getpid())
                        {
                            // this is the entry of this child in the shared memory
                            new_idx = k;
                            break;
                        }
                    }

                    struct tuple new_pos = get_pos(jump_idx, h + 1, new_idx - 1);
                    printf("I am child: %d, at new_pos: (%d, %d), creating %d children\n", getpid(), new_pos.x, new_pos.y, new_pos.x + 1);

                    if (new_idx + 1 < sh[0])
                        kill(sh[new_idx + 1], SIGCONT);
                    exit(1);
                }
            }

            kill(sh[idx + 1], SIGCONT); // after we have completed execution, signal the next process to continue
            while (wait(NULL) == -1)    // wait
                ;
            exit(1); // exit
        }
    }

    // send sigcont to all children right now
    kill(sh[2], SIGCONT);
    while (wait(NULL) == -1)
        ;

    for (int i = 0; i <= sh[0]; i++)
    {
        printf("%d --> %d\n", i, sh[i]);
    }

    // close the connection to shared memory
    if (shmdt(sh) == -1)
    {
        printf("Error detaching shared memory.");
        return -1;
    }

    // delete the shared memory
    shmctl(shmid, IPC_RMID, NULL);
}