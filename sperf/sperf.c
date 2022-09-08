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
#define MAXCMDLEN 1024
#define MAXARGS 128
#define MAXSYSCALLNAME 64
#define MAXGROUPS 3

typedef struct node{
  char syscall[MAXSYSCALLNAME];
  double duration;
  SLIST_ENTRY(node) field;
} node_t;

typedef SLIST_HEAD(head,node) head_t;

static struct timeval timeout = {
  .tv_sec = 1,
  .tv_usec = 0
};

static char exec_cmd[MAXCMDLEN];
static char *exec_argv[MAXCMDLEN];
static char tmp[MAXCMDLEN];
static char buf[MAXCMDLEN];

static inline char *regex_extract(char *str,regmatch_t *regMatch){
  static char buf[MAXSYSCALLNAME] = {0};
  char tmp = *(str+regMatch->rm_eo);
  *(str+regMatch->rm_eo) = 0;
  strcpy(buf,(str+regMatch->rm_so));
  *(str+regMatch->rm_eo) = tmp;
  return buf;
}

static inline void syscall_list_insert(head_t *hd,char *csys, double dur){
  node_t *elm;
  SLIST_FOREACH(elm,hd,field){
    if(strcmp(elm->syscall,csys)==0){
      elm->duration+=dur;
      return;
    }
  }
  elm = malloc(sizeof(node_t));
  elm->duration = dur;
  strcpy(elm->syscall,csys);
  SLIST_INSERT_HEAD(hd,elm,field);
}

int main(int argc, char *argv[], char *envp[])
{ 

  int pid,pipes[2],reti,i;
  char *line,*ppath,*rtmp;
  char **pexec_arg,**parg;
  char tmp[MAXCMDLEN];
  double duration;
  
  head_t *hd;
  size_t maxlen;
  ssize_t nreads;
  regex_t regexCompiled;
  regmatch_t matchGroups[MAXGROUPS];
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
  in = fdopen(pipes[0],"r");
  line = NULL;
  for(pexec_arg=exec_argv+2,parg=argv+1;*parg;pexec_arg++,parg++)
    *pexec_arg=*parg;
  *pexec_arg = "2>/dev/null";
  if(reti = regcomp(&regexCompiled,"([^(]*)\\(.*\\)\\s*=\\s-??[0-9a-fx]*\\s[^<]*<([.0-9]*)>",REG_EXTENDED)){
    printf("Regex Compilaton Error\n");
    exit(EXIT_FAILURE);
  }

  hd = malloc(sizeof(head_t));
  SLIST_INIT(hd);

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

    while((nreads = getline(&line,&maxlen,in)) != -1){
      if(regexec(&regexCompiled,line,MAXGROUPS,matchGroups,0) == 0){
        rtmp = regex_extract(line,&matchGroups[2]);
        duration = atof(rtmp);
        rtmp = regex_extract(line,&matchGroups[1]);
        syscall_list_insert(hd,rtmp,duration);
      }
    }
  }
  
  return 0;
}
