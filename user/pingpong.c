// user/pingpong.c
#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  int p_to_c[2]; // Pipe: Parent to Child
  int c_to_p[2]; // Pipe: Child to Parent
  char buf[8]; // Buffer for the byte

  if (pipe(p_to_c) < 0) { // [cite: 1907] 使用 pipe 创建管道
    fprintf(2, "pipe failed\n");
    exit(1);
  }
  if (pipe(c_to_p) < 0) {
    fprintf(2, "pipe failed\n");
    exit(1);
  }

  int pid = fork(); // [cite: 1907] 使用 fork 创建子进程

  if (pid < 0) {
    fprintf(2, "fork failed\n");
    exit(1);
  }

  if (pid == 0) { // Child process
    close(p_to_c[1]); // Close write end of parent-to-child pipe
    close(c_to_p[0]); // Close read end of child-to-parent pipe

    if (read(p_to_c[0], buf, 1) != 1) { // [cite: 1908] 使用 read 从管道读取
      fprintf(2, "child: failed to read from parent\n");
      exit(1);
    }
    close(p_to_c[0]);

    printf("%d: received ping\n", getpid()); // [cite: 1909] getpid 获取进程ID, [cite: 1904] 子进程打印 "received ping"

    if (write(c_to_p[1], buf, 1) != 1) { // [cite: 1908] 使用 write 写入管道
      fprintf(2, "child: failed to write to parent\n");
      exit(1);
    }
    close(c_to_p[1]);
    exit(0);

  } else { // Parent process
    close(p_to_c[0]); // Close read end of parent-to-child pipe
    close(c_to_p[1]); // Close write end of child-to-parent pipe

    buf[0] = 'B'; // Any byte
    if (write(p_to_c[1], buf, 1) != 1) {
      fprintf(2, "parent: failed to write to child\n");
      exit(1);
    }
    close(p_to_c[1]);

    // Wait for child to finish its part
    // wait(0); // Optional: can also just block on read

    if (read(c_to_p[0], buf, 1) != 1) {
      fprintf(2, "parent: failed to read from child\n");
      exit(1);
    }
    close(c_to_p[0]);
    
    printf("%d: received pong\n", getpid()); // [cite: 1905] 父进程打印 "received pong"
    wait(0); // Ensure child has exited and resources are cleaned up
    exit(0);
  }
}