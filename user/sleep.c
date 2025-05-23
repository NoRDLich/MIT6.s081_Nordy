#include "kernel/types.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
  if (argc != 2) { // [cite: 1893] 如果用户忘记传递参数，sleep 应该打印错误消息
    fprintf(2, "Usage: sleep <ticks>\n");
    exit(1);
  }

  int ticks = atoi(argv[1]); // [cite: 1894] 命令行参数是字符串，用 atoi 转换为整数
  if (ticks <= 0) {
    fprintf(2, "Error: ticks must be a positive integer\n");
    exit(1);
  }

  sleep(ticks); // [cite: 1895] 使用 sleep 系统调用

  exit(0); // [cite: 1896] 确保 main 调用 exit()
}