// user/find.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h" // For DIRSIZ

// Function to format path correctly (similar to ls.c)
char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p)); // Pad with spaces for ls.c compatibility if needed, or just NUL terminate
  buf[strlen(p)] = 0; // NUL terminate for general C string usage
  // For find, we just need the name, not padding.
  // So, a simple strcpy is fine too if not reusing ls.c formatting.
  // For simplicity here, just ensure it's NUL terminated and not overly long.
  if(strlen(p) >= DIRSIZ) {
     // Handle very long names if necessary, though DIRSIZ is the limit in dirent
  }
  strcpy(buf, p);
  return buf;
}

void
find(char *path, const char *filename)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){ // [cite: 1926] 参考 user/ls.c 如何读取目录
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE: // It's a file, check if its name matches
    if (strcmp(fmtname(path), filename) == 0) { // [cite: 1930] 使用 strcmp 比较字符串
        printf("%s\n", path);
    }
    break;

  case T_DIR: // It's a directory, recurse
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/'; // Add / to the end of current path
    
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0) // Skip empty directory entries
        continue;
      if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) // [cite: 1927] 不要递归进入 "." 和 ".."
        continue;
      
      memmove(p, de.name, DIRSIZ); // DIRSIZ includes NUL, or just strlen(de.name)
      p[DIRSIZ] = 0; // NUL terminate, or ensure p points to NUL after de.name
      // More robustly:
      strcpy(p, de.name);

      // Recursive call [cite: 1926]
      find(buf, filename);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if (argc != 3) {
    fprintf(2, "Usage: find <directory> <filename>\n");
    exit(1);
  }
  find(argv[1], argv[2]);
  exit(0);
}