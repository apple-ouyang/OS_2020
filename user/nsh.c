#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define CMDLEN 100
#define ARGNUM 10
#define ARGLEN 100

char argv[ARGNUM][ARGLEN];
char *args[ARGNUM], *argl[ARGNUM], *argr[ARGNUM];

void
panic(char *s)
{
  fprintf(2, "%s\n", s);
  exit(-1);
}

int
fork1(void)
{
  int pid;

  pid = fork();
  if(pid == -1)
    panic("fork");
  return pid;
}

char white[] = " \t\r\n\v";

void parse(char *cmd){

    int p = 0, l = 0, argc = 0, len = strlen(cmd);
    // fprintf(2, "parse:%s\n", cmd);
    while(p+l<=len){
        // fprintf(2, "p:%d, l:%d, cmd[p+l]:%c\n", p , l, cmd[p+l]);
        if(strchr(white, cmd[p+l])){
            // fprintf(2, "white\n");
            if(l==0) ++p;
            else{
                cmd[p+l] = '\0';
                // fprintf(2, "cmd+p:%s\n", cmd+p);
                strcpy(argv[argc++], cmd+p);
                // fprintf(2, "new:%s\n", new);
                p = p+l+1, l=0;
            }
            
        }else ++l;
    }
    strcpy(argv[argc], "\0");
    // fprintf(2, "parse result:\n");
    // for (int i = 0; i < argc; i++)
    // {
    //     fprintf(2, "%d %s\n", i, argv[i]);
    // }

    for (uint i = 0; i <= argc; i++)
    {
        args[i] = argv[i];
    }
    
}
int p[2];

void runcmd(char *cmd){
    fprintf(2, "runcmd:%s\n", cmd);
    char *ptr;
    if((ptr = strchr(cmd, '|'))){
        // fprintf(2, "pipe\n");
        *ptr = '\0';
        // fprintf(2, "left pipe:%s\n", cmd);
        // fprintf(2, "right pipe:%s\n", ptr+1);
        if(pipe(p) < 0)
            panic("pipe");
        if(fork1()==0){
            fprintf(2, "pipe write\n");
            close(1);
            dup(p[1]);
            close(p[0]);
            runcmd(cmd);
        }
        if(fork1() == 0){
            fprintf(2, "pipe read\n");
            close(0);
            dup(p[0]);
            close(p[0]);
            close(p[1]);
            runcmd(ptr+1);
        }
        close(p[0]);
        close(p[1]);
        wait(0);
        wait(0);

    }else if ((ptr = strchr(cmd, '<'))){
        fprintf(2, "redirection <\n");
        *ptr = '\0';
        close(0);
        open(ptr+1, O_RDONLY);
        runcmd(cmd);

    }else if((ptr = strchr(cmd, '>'))){
        fprintf(2, "redirection >\n");
        *ptr = '\0';
        close(1);
        parse(ptr+1);
        int fd = open(args[0], O_CREATE|O_WRONLY);
        if(fd != 1) fprintf(2, "fd:%s\n", fd);
        runcmd(cmd);
    }else{
        parse(cmd);
        fprintf(2, "exec %s\n", args[0]);
        exec(args[0], args);
        fprintf(2, "exec %s failed\n", args[0]);
    }
}

int main(){
    char cmd[CMDLEN];
    // while(1){
        fprintf(2, "@ ");
        gets(cmd, CMDLEN);
        // if(cmd[0]==0) break;
        // fprintf(2, "cmd:%s\n", cmd);
        runcmd(cmd);
    // }
    
    exit(0);
}