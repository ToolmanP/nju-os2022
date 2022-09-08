#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <malloc.h>
#include <regex.h>
#include <sys/queue.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>

#ifdef LOCAL_MACHINE
#define TODO()\
do {\
printf("This section is waiting you to finish\n");\
assert(0);\
}while(0)
#define FAILURE() assert(0);
#else
#define TODO()
#define FAILURE()
#endif
#define MAXCMDLEN 4096

typedef struct node{
  char cmd[MAXCMDLEN];
  float duration;
  TAILQ_ENTRY(node) nodes;
} node_t;

typedef TAILQ_HEAD(head,node) head_t;

static int flides[2];
static char *PATH = NULL;
static char cmd[MAXCMDLEN];

int main(int argc, char *argv[], char *envp[])
{

  int pid,fildes[2];
  char *PATH,*token;

  assert(argc>=2);

  if(pipe(fildes)>0)
    assert(0);
  
  PATH = getenv("PATH");
  pid = fork();

  if(pid == 0){
    // dup2(fildes[1],STDERR_FILENO);
    token = strtok(PATH,":");
    while(token){
      sprintf(cmd,"%s/%s",token,argv[1]);
      execve(cmd,argv+1,envp);
      token = strtok(NULL,":");
    }
    assert(0);
  }else{
    wait(NULL);
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
