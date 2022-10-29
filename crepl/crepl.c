#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <assert.h>

#include <sys/wait.h>

#define MAXLEN 8192
#define MAXPATH 256
#define MAXSYM 128

static inline char * call_compiler(const char *expr)
{
  char source[] = "/tmp/creplXXXXXX";
  char object[MAXPATH];
  static char shared[MAXPATH];

  int fd = mkstemp(source);
  write(fd,expr,strlen(expr));
  close(fd);

  sprintf(object,"%s.o",source);
  sprintf(shared,"%s.so",source);

  char *argv_compile[] = {
    "gcc",
    "-x",
    "c",
    "-c",
    source,
    "-o",
    object,
    "-fPIC",
    "-Wno-implicit-function-declaration",
    NULL
  };

  char *argv_link[] = {
    "gcc",
    "-shared",
    object,
    "-o",
    shared,
    NULL
  };

  if(fork() == 0){
    execvp(argv_compile[0],argv_compile);
    perror("execvp");
  }else{
    wait(NULL);
  }
  if(fork() == 0){
    execvp(argv_link[0], argv_link);
    perror("execvp");
  }else{
    wait(NULL);
  }

  return shared;
}

static inline char *wrap_expression(const char *sym,const char *def)
{
  static char buf[MAXLEN] = {};
  sprintf(buf,"int %s(){return (%s);}",sym,def);
  return buf;
}


static inline int exec_expression(const char *line,int *success)
{
  static int expr_total = 0;
  static char sym[MAXSYM] = {};
  int (*func)();
  sprintf(sym,"expr_wrapper_%d",expr_total++);
  char *expr = wrap_expression(sym,line);
  char *shared = call_compiler(expr);

  void *handle = dlopen(shared, RTLD_NOW | RTLD_GLOBAL);
  char *error = dlerror();

  if(error != NULL){
    printf("expr:%s\n",error);
    *success = -1;
    expr_total--;
    return 0;
  }

  *(void **)(&func) = dlsym(handle,sym);
  
  if((error = dlerror()) != NULL){
    printf("expr:%s\n",error);
    *success = 1;
    expr_total--;
    return -1;
  }

  return func();
}

static inline void compile_function(const char *line)
{
  char *shared = call_compiler(line);
  void *handle = dlopen(shared, RTLD_NOW | RTLD_GLOBAL);
  char *error = dlerror();
  (void)handle;
  if(error != NULL){
    printf("compile: %s\n",error);
  }else{
    printf("compile: success\n");
  }
  
  return;
}

static inline void parse_line(char *line)
{

  line[strcspn(line,"\n")] = '\0';

  const char *p = line;
  char *q;
  int result,success=0;
  while(*p == ' ' || *p == '\t')
    p++;
  q = strstr(p,"int ");
  
  if(p==q){
    compile_function(line);
  }else{
    result = exec_expression(line,&success);
    if(success == 0){
      printf("(%s) == %d\n",line,result);
    }
  }
}


int main(int argc, char *argv[]) {
  static char line[4096];

  while (1) {
    printf("crepl> ");
    fflush(stdout);
    if (!fgets(line, sizeof(line), stdin)) {
      break;
    }
    parse_line(line);
    // printf("Got %zu chars.\n", strlen(line)); // ??
  }
  return EXIT_FAILURE; // never reach here.
}
