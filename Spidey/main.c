#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <signal.h>

int n = 4;
int grid[4][4] = {
    {0, 0, 1, 1},
    {1, 0, 0, 1},
    {1, 0, 1, 0},
    {1, 0, 0, 0}};

int cdi(int y, int x)
{
    return n * y + x;
}

void traverse(int y, int x)
{
    // base case --> we are at (n-1, n-1)
    if (y == n - 1 && x == n - 1)
    {
        printf("%d, %d\n", x, y);
        exit(1);
    }

    int s1 = 0, s2 = 0;
    if (y + 1 < n && grid[y + 1][x] == 0)
    {
        pid_t pid = fork();
        if (pid)
        {
            waitpid(pid, &s1, 0);
            s1 /= 256;
        }
        else
        {
            traverse(y + 1, x);
        }
    }
    else if (x + 1 < n && grid[y][x + 1] == 0)
    {
        pid_t pid = fork();
        if (pid)
        {
            waitpid(pid, &s2, 0);
            s2 /= 256;
        }
        else
        {
            traverse(y, x + 1);
        }
    }
    if (s1 || s2)
    {
        printf("%d, %d\n", x, y);
        exit(1);
    }
    exit(0);
}

int main()
{

    /*
     * In this program, we will use two seperate shared memory locations
     * One for global variables etc
     * One for the actual grid
     */

    // printf("Enter N: ");
    // scanf("%d", &n);

    // Take in the grid values
    // printf("Enter grid values one by one: \n");
    // for (int i = 0; i < n * n; i++)
    //     scanf("%d", &grid[i]);

    // perform some mandatory checks (0,0) == 0, (n-1, n-1) == 0
    if (grid[0][0] == 1 || grid[0][0] == 1)
    {
        printf("Spidey cannot enter/exit this grid.\n");
        return -1;
    }

    traverse(0, 0);
}