#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <getopt.h>
#include <malloc.h> 
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <memory.h>

#define MAXNODES 256
#define MAXPROCESSLEN 256
#define MAXDIRLEN 256

#define panic() do{ \
  printf("No implemented.\n");\
  assert(0);\
}while(0)\

#define THROW(string) do{\
    printf("%s\n",string);\
    assert(0);\
}while(0)\

#define FMT_PRINT(string,depth,pid) do{\
  for(i=0;i<depth;i++){\
    printf("\t");\
  }\
  if(showpid){\
    printf("(%d)",pid);\
  }\
  printf("%s\n",string);\
}while(0)\

const struct option table[]={
  {"show-pids",no_argument,NULL,'p'},
  {"numeric-sort",no_argument,NULL,'n'},
  {"version",no_argument,NULL,'V'}
};

int32_t numeric,showpid;

enum{
  PROC_STAT,
  PROC_CHILDREN,
  PROC_THREADS
};

static inline void print_version(){
  printf("PTree by ToolmanP v114514\n");
}

static inline char *get_proc_dir(int pid,int mode){
  static char buf[MAXDIRLEN*2];
  switch(mode){
    case PROC_STAT:
      sprintf(buf,"/proc/%d/stat",pid);
      break;
    case PROC_THREADS:
      sprintf(buf,"/proc/%d/task",pid);
      break;
    case PROC_CHILDREN:
      sprintf(buf,"/proc/%d/task/%d/children",pid,pid);
      break;
    default:
      return NULL;
  }
  return buf;
}


// pid 1 always refers to the init-system's pid 
// so we just start with pid = 1


static void pstree(int pid,unsigned depth){
  char *ps,name[MAXPROCESSLEN];
  int next_pid,i;
  FILE *fd;
  DIR *dp;
  struct dirent *dir;
  
  ps = get_proc_dir(pid,PROC_STAT);

  if((fd = fopen(ps,"r")) == NULL)
    THROW(ps);
  
  fscanf(fd,"%*d %s",name);
  fclose(fd);

  ps = get_proc_dir(pid,PROC_THREADS);

  if((dp = opendir(ps)) == NULL)
    THROW(ps);
  
  FMT_PRINT(name,depth,pid);

  while((dir = readdir(dp)) != NULL){

    if(strcmp(dir->d_name,".") != 0 && strcmp(dir->d_name,"..") != 0 && dir->d_type == DT_DIR){
      next_pid = strtol(dir->d_name,NULL,10);
      if(next_pid == pid){

        ps = get_proc_dir(pid,PROC_CHILDREN);

        if((fd = fopen(ps,"r")) == NULL){
          THROW(ps);
        }

        while(fscanf(fd,"%d",&next_pid) != EOF){
          pstree(next_pid,depth+1);
        }

        fclose(fd);
      }else{
        FMT_PRINT(name,depth+1,next_pid);
      }
    }
  }
  closedir(dp);
}

int main(int argc, char *argv[]) {
  int o;
  while((o = getopt_long(argc,argv,"pnV",table,NULL))!=-1){
    switch(o){
      case 'n':
        numeric = 1;
        break;
      case 'p':
        showpid = 1;
        break;
      case 'V':
        print_version();
        exit(0);
      default:
        assert(0);
    }
  }

  pstree(1,0);
  
  return 0;
}
