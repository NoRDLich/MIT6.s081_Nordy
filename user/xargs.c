// user/xargs.c
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/param.h" // For MAXARG [cite: 1937]

#define MAX_LINE_LEN 512

int
main(int argc, char *argv[])
{
  char line[MAX_LINE_LEN];
  char *p, *last_char_in_line;
  char *cmd_argv[MAXARG];
  int cmd_argc;
  int i;

  if (argc < 2) {
    fprintf(2, "Usage: xargs <command> [initial-args...]\n");
    exit(1);
  }

  // Prepare the command and its initial arguments
  for (i = 1; i < argc && i < MAXARG -1; i++) {
      cmd_argv[i-1] = argv[i];
  }
  cmd_argc = i - 1; // Number of initial arguments for the command

  p = line;
  last_char_in_line = line + MAX_LINE_LEN -1;

  while(1) {
    // Read one line from standard input [cite: 1936]
    char *current_arg_start = p;
    int line_argc = 0;
    char *line_args[MAXARG]; // Arguments from the current input line

    while(read(0, p, 1) == 1) {
        if (*p == ' ' || *p == '\n') { // Argument delimiter or end of line
            *p = 0; // Null-terminate the argument
            if (p > current_arg_start) { // If it's not an empty string
                 if(line_argc < MAXARG -1 - cmd_argc) { // Check space for new args
                    line_args[line_argc++] = current_arg_start;
                 } else {
                    // Too many arguments from line, could error or truncate
                 }
            }
            current_arg_start = p + 1;
            if (*p == '\n') break; // End of line
        }
        p++;
        if (p >= last_char_in_line) { // Prevent buffer overflow
            fprintf(2, "xargs: input line too long\n");
            // Skip rest of line or error
            while(read(0, p, 1) == 1 && *p != '\n'); // consume rest of the line
            break; 
        }
    }
    
    if (p == current_arg_start && line_argc == 0 && (p >= last_char_in_line || *p != 0) ) { // EOF or error before any arg read on this line
        break;
    }
     *p = 0; // Null-terminate the last argument if line didn't end with newline or space
     if (p > current_arg_start && line_argc < MAXARG -1 - cmd_argc) {
         line_args[line_argc++] = current_arg_start;
     }


    if (line_argc > 0) {
        // Combine initial_args with line_args
        int total_cmd_argc = 0;
        for(int j=0; j < cmd_argc; j++) {
            cmd_argv[total_cmd_argc++] = argv[j+1]; // Original command and its args
        }
        for(int j=0; j < line_argc; j++) {
            if (total_cmd_argc < MAXARG -1) {
                 cmd_argv[total_cmd_argc++] = line_args[j]; // Args from stdin line
            }
        }
        cmd_argv[total_cmd_argc] = 0; // Null-terminate argv for exec


        if (fork() == 0) { // [cite: 1935] 使用 fork 和 exec
            exec(cmd_argv[0], cmd_argv);
            fprintf(2, "xargs: exec %s failed\n", cmd_argv[0]);
            exit(1);
        } else {
            wait(0); // [cite: 1935] 父进程中 wait
        }
    }
    
    if (*p == 0 && p == line) break; // EOF reached at start of line processing
    if (*p != 0 && read(0, p,0) == 0 && p == current_arg_start) break; // Proper EOF check
    if (p < last_char_in_line) {
         p = line; // Reset for next line
    } else {
        break; // Should not happen if line length check is correct
    }

  }
  exit(0);
}