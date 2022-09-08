#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>

#ifdef LOCAL_MACHINE
#define TODO()\
do {\
printf("This section is waiting you to finish\n");\
assert(0);\
}while(0)
#else
#define TODO()
#endif
#define MAXCMDLEN 

static int flides[2];

int main(int argc, char *argv[],char *envp[]) {
  
  // char **exec_argv = argv+1;
  
  
  // execve("/bin/strace",     exec_argv, exec_envp);
  // execve("/usr/bin/strace", exec_argv, exec_envp);
  
  printf("%s\n",getenv("PATH"));
  // perror(exec_argv[0]);
  // exit(EXIT_FAILURE);
  return 0;
}
