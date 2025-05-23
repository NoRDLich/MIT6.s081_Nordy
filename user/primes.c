// user/primes.c
#include "kernel/types.h"
#include "user/user.h"

void prime_process(int left_pipe_read_end) {
    int p;
    // Read the first number, which is a prime
    if (read(left_pipe_read_end, &p, sizeof(int)) != sizeof(int)) {
        // If read fails or pipe is empty, this process might be an unnecessary one or error.
        close(left_pipe_read_end);
        exit(0);
    }
    printf("prime %d\n", p);

    int right_pipe[2];
    if (pipe(right_pipe) < 0) {
        fprintf(2, "pipe failed in prime_process\n");
        close(left_pipe_read_end);
        exit(1);
    }

    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed in prime_process\n");
        close(left_pipe_read_end);
        close(right_pipe[0]);
        close(right_pipe[1]);
        exit(1);
    }

    if (pid == 0) { // Child process
        close(right_pipe[1]); // Child reads from right_pipe[0] (which is its left)
        close(left_pipe_read_end); // This was parent's left, child doesn't need it
        prime_process(right_pipe[0]); // Recursively call with the read end of the new pipe
    } else { // Current prime process (parent to the new fork)
        close(right_pipe[0]); // Parent writes to right_pipe[1]
        
        int n;
        while (read(left_pipe_read_end, &n, sizeof(int)) == sizeof(int)) {
            if (n % p != 0) { // If not divisible by current prime, pass to right neighbor
                if (write(right_pipe[1], &n, sizeof(int)) != sizeof(int)) {
                    fprintf(2, "write to right_pipe failed\n");
                    // Error, but cleanup is tricky here. Let child handle EOF.
                }
            }
        }
        // Close pipes and wait for child
        close(left_pipe_read_end);
        close(right_pipe[1]);
        wait(0); // [cite: 1919] 第一个进程应等待整个管道终止
        exit(0);
    }
}

int
main(int argc, char *argv[])
{
    int initial_pipe[2];

    if (pipe(initial_pipe) < 0) {
        fprintf(2, "initial pipe failed\n");
        exit(1);
    }

    int pid = fork();

    if (pid < 0) {
        fprintf(2, "initial fork failed\n");
        close(initial_pipe[0]);
        close(initial_pipe[1]);
        exit(1);
    }

    if (pid == 0) { // First child process (handles primes)
        close(initial_pipe[1]); // This process reads from initial_pipe[0]
        prime_process(initial_pipe[0]);
        exit(0);
    } else { // Main parent process (feeds numbers)
        close(initial_pipe[0]); // Parent writes to initial_pipe[1]

        for (int i = 2; i <= 35; i++) { // [cite: 1916] 第一个进程将数字2到35送入管道
            if (write(initial_pipe[1], &i, sizeof(int)) != sizeof(int)) {
                fprintf(2, "initial write failed for %d\n", i);
            }
        }
        close(initial_pipe[1]); // Close write end, signals EOF to first prime_process
        wait(0); // Wait for the first child process (which waits for its children, etc.)
        exit(0);
    }
}