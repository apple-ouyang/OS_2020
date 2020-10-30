#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAXLEN 100

int main(int argc, char *argv[]){

    char *args[30];
    for (int i = 0; i < argc; i++)
    {
        args[i] = argv[i];
    }
    // for (int i = 0; i < argc; i++)
    // {
    //     printf("args%d:%s\n", i, args[i]);
    // }
    char line[MAXLEN] = "no line gets in here";
    while(gets(line, 100)){
        if(line[0]==0) break;
        int p = 0, l = 0, len = strlen(line);
        line[len-1] = 0, --len;
        // printf("line:%s\n", line);
        while(p+l<=len){
            if(line[p+l]==' ' || line[p+l]==0){
                line[p+l] = 0;
                // printf("line+p:%s\n", line+p);
                char *new = (char*)malloc(MAXLEN);
                strcpy(new, line+p);
                // printf("new:%s\n", new);
                args[argc++] = new;
                p = p+l+1, l=0;
            }else ++l;
        }
    }
    // for (int i = 0; i < argc; i++)
    // {
    //     printf("args%d:%s, %p\n", i, args[i], args[i]);
    // }
    exec(args[1], args+1);
    exit();
}