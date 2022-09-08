#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <unistd.h>
#include <assert.h>

#ifdef LOCAL_MACHINE
#define TODO()\
do {\
printf("This section is waiting you to finish\n");\
assert(0);\
}while(0)
#else
#define TODO()
#endif
#define MAXCMDLEN 4096

static int flides[2];
static char *PATH = NULL;
static char cmd[MAXCMDLEN];

int main(int argc, char *argv[]) {


  PATH = getenv("PATH");
  char *token = strtok(PATH,":");
  while(token){
    printf("%s\n",token);
    token = strtok(NULL,":");
  }
  // assert(argc>=2);
  // pipe(flides);
  // pid = fork();
  

  // execve("/bin/strace",     exec_argv, exec_envp);
  // execve("/usr/bin/strace", exec_argv, exec_envp);
  // perror(exec_argv[0]);
  // exit(EXIT_FAILURE);
  return 0;
}
