#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <getopt.h>


const struct option table[]={
  {"show-pids",no_argument,NULL,'p'},
  {"numeric-sort",no_argument,NULL,'n'},
  {"version",no_argument,NULL,'V'}
};

static inline void print_version(){
  printf("PTree by ToolmanP v114514\n");
}

static inline void pstree(int numeric,int showpid){
  printf("Not implemented.\n");
  assert(0);
}

int main(int argc, char *argv[]) {
  int o;
  int numeric = 0, showpid = 0;
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
  pstree(numeric,showpid);
  // assert(!argv[argc]);
  
  return 0;
}
