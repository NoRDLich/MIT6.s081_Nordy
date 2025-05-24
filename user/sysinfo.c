#include "kernel/types.h"
#include "kernel/sysinfo.h"
#include "user/user.h"

int
main(void)
{
  struct sysinfo info;
  if(sysinfo(&info) < 0){
    printf("sysinfo failed\n");
    exit(1);
  }
  printf("free memory: %d bytes\n", info.freemem);
  printf("processes: %d\n", info.nproc);
  exit(0);
}