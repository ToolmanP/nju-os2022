#include <stdio.h>
#include <stdlib.h>
#define _GNU_SOURCE
int main()
{   
    char *line = NULL;
    size_t len = 1000;
    int result = getline(&line,&len,stdin);
    printf("%d\n",result);
    printf("%s\n",line);
    return 0;
}