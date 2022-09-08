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
#define MAXARGS 1024
typedef struct node{
  char cmd[MAXCMDLEN];
  float duration;
  TAILQ_ENTRY(node) nodes;
} node_t;
typedef TAILQ_HEAD(head,node) head_t;

static struct timeval timeout = {
  .tv_sec = 1,
  .tv_usec = 0
};
static char exec_cmd[MAXCMDLEN];
static char *exec_argv[MAXCMDLEN];
static char tmp[MAXCMDLEN];
int main(int argc, char *argv[], char *envp[])
{ 

  int pid,pipes[2];
  char *line,*ppath;
  char **pexec_arg,**parg;

  size_t maxlen;
  ssize_t nreads;
  FILE *in;

  assert(argc>=2);
  setbuf(stdout,NULL);
  setbuf(stderr,NULL);

  if(pipe(pipes)>0)
    assert(0);
  
  strcpy(tmp,getenv("PATH"));
  maxlen = 4096;
  exec_argv[0] = exec_cmd;
  exec_argv[1] = "-T";
  for(pexec_arg=exec_argv+2,parg=argv+1;*parg;pexec_arg++,parg++)
    *pexec_arg=*parg;

  if((pid = fork()) == 0){
    close(pipes[0]);
    dup2(pipes[1],STDERR_FILENO);
    ppath = strtok(tmp,":");
    while(ppath){
      sprintf(exec_argv[0],"%s/strace",ppath);
      execve(exec_argv[0],exec_argv,envp);
      ppath = strtok(NULL,":");
    }
    assert(0);
  }else{
    close(pipes[1]);
    in = fdopen(pipes[0],"r");
    line = NULL;
    while((nreads = getline(&line,&maxlen,in)) != -1){
      fprintf(stderr,"%s",line);

    }
  }
  // assert(argc>=2);
  // pipe(pipes);
  // pid = fork();
  

  // execve("/bin/strace",     exec_argv, exec_envp);
  // execve("/usr/bin/strace", exec_argv, exec_envp);
  // perror(exec_argv[0]);
  // exit(EXIT_FAILURE);
  return 0;
}
