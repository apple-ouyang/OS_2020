#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char* getName(char *path){
    char *p;
    for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
    p++;
    return p;
}

void dfs(char *path, char *name, int fir){
    if(!fir && !strcmp(getName(path), name))
        printf("%s\n", path);
        
    
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if((fd = open(path, 0)) < 0){
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if(fstat(fd, &st) < 0){
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // printf("path: %s, fd: %d\n", path, fd);
    switch(st.type){
    case T_FILE:
        break;

    case T_DIR:
        if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
            printf("find: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf+strlen(buf);
        *p++ = '/';

        char dot[2] = ".", dott[3] = "..";
        while(read(fd, &de, sizeof(de)) == sizeof(de)){
            if(de.inum == 0)
                continue;
            if(strcmp(de.name, dot)==0 || strcmp(de.name, dott)==0)
                continue;
            // printf("de.name %s\n", de.name);
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if(stat(buf, &st) < 0){
                printf("find: cannot stat %s\n", buf);
                continue;
            }
            dfs(buf, name, 0);
        }
        break;
    }
    close(fd);
    return;
}


int
main(int argc, char *argv[])
{
  int i;

  if(argc < 2){
    printf("error! no file name input\n");
    exit();
  }
  for(i=1; i<argc; i++)
    dfs(".", getName(argv[i]), 1);
  exit();
}