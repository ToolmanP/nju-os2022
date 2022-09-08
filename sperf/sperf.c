#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <malloc.h>
#include <regex.h>
#include <signal.h>
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
static char exec_cmd[MAXCMDLEN];
static struct timeval timeout = {
  .tv_sec = 1,
  .tv_usec = 0
};

int main(int argc, char *argv[], char *envp[])
{
  int pid,fildes[2];
  char *PATH,*token,*line;
  size_t maxlen;
  ssize_t nreads;
  FILE *out;

  assert(argc>=2);

  if(pipe(fildes)>0)
    assert(0);
  
  PATH = getenv("PATH");
  maxlen = 4096;
  argv[1] = exec_cmd;
  pid = fork();
  if(pid == 0){
    dup2(fildes[1],STDERR_FILENO);
    token = strtok(PATH,":");
    while(token){
      sprintf(exec_cmd,"%s/%s",token,argv[1]);
      execve(argv[1],argv+1,envp);
      token = strtok(NULL,":");
    }
    assert(0);
  }else{
    wait(NULL);
    out = fdopen(fildes[0],"r");
    while((nreads = getline(&line,&maxlen,out)) != -1)
      printf("%s",line);
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
