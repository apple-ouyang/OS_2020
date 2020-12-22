//
// Created by ouyang on 2020/12/20.
//

#include "shell.h"
#include "util.h"
#include "IO.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


#define LIST    1
#define MKDIR   2
#define TOUCH   3
#define COPY    4
#define SHUTDOWN    5
#define CLEAN   6
#define EMPTY   7

//  寻找文件路径的inode编号，以及parent的inode编号
//  如果找到文件则返回1，否则返回0
static int look_up_all(const char *path, int* in_id, int* inp_id, int only_find_parent){
    int path_len = strlen(path);

    int i = 0;

    //删除前面和后面的'/'，不是真的删除，而是移动i和path_len
    if(path[i] == '/') ++i;
    if(path[path_len-1] == '/') --path_len;

    *in_id = *inp_id = 0; //in_id现在指向了root
    for (int j = i+1; j <= path_len; i=j+1, j=j+1) {
        while (path[j]!='/' && j < path_len) ++j;   //[i, j)指向文件名下标

        //如果只用找到上一级，到最后一个文件名字的时候就退出
        if(only_find_parent && j == path_len)
            return 1;

        Inode in = read_inode(*in_id);
        
        if(in.file_type!=TYPE_DIR)
            panic("look_up_all: inode类型不是DIR");
        
        for (int k = 0; k < INODE_BLOCK_PTR; ++k) {
            if(in.block_point[k]){
                Read(in.block_point[k], buf);
                for (Dir* dir = (Dir *)buf; dir < buf+BLOCK_SIZE; ++dir) {
                    if(dir->valid && strncmp(dir->name, path+i, j-i) == 0){
                        *inp_id = *in_id;
                        *in_id = dir->inode_id;
                        goto next;
                    }
                }
            }
        }
        return 0;
//        panic("look_up_all:没有找到对应目录下的文件，请检查拼写");
        next:;
    }
    return 1;
}

//查找路径的上一层
int look_up_parent(const char path[], int* in_id){
    int inp_id;
    return look_up_all(path, in_id, &inp_id, 1);
}

//查找路径,1找到，0没有找到
int look_up(const char path[], int* in_id){
    int inp_id;
    return look_up_all(path, in_id, &inp_id, 0);
}


void list(const char path[]){
    int in_id;
    if(look_up(path, &in_id) == 0){
        panic("没有找到这个路径\n");
        return;
    }
    Inode in = read_inode(in_id);
    for (int i = 0; i < INODE_BLOCK_PTR; ++i) {
        if(in.block_point[i]){
            Read(in.block_point[i], buf);
            for (Dir* dir = (Dir *)buf; dir < buf+BLOCK_SIZE; ++dir) {
                if(dir->valid)
                    printf("%s\n", dir->name);
            }
        }
    }
}

static int creat_file(const char path[], uint16_t type){
    int id, idp;
    if(look_up(path, &id)){
        printf("该文件/目录已经存在了\n");
        return -1;
    }
    look_up_parent(path, &idp);
    Inode inp = read_inode(idp);
    for (int i = 0; i < INODE_BLOCK_PTR; ++i) {
        if(inp.block_point[i] == 0){
            inp.block_point[i] = alloc_block();
            write_inode(idp, inp);
        }

        Read(inp.block_point[i], buf);
        for (Dir* dirent = (Dir *)buf; dirent < buf + BLOCK_SIZE; ++dirent) {
            if(!dirent->valid){
                int p = strlen(path)-1;     //最后一个路径的起始位置
                while (p-1>=0 && path[p-1]!='/') --p;

                int new_id = alloc_inode_id();
                dirent->inode_id = new_id;
                dirent->valid = 1;
                dirent->type = type;
                strcpy(dirent->name, path + p);
                Write(inp.block_point[i], buf);

                Inode new_in = read_inode(new_id);
                new_in.link = 1;
                new_in.size = 0;
                new_in.file_type = type;
                memset(new_in.block_point, 0, sizeof(new_in.block_point));
                write_inode(new_id, new_in);
                return new_id;
            }
        }
    }
    panic("creat_file: inode没有足够的block ptr了");
    return 0;
}

void mkdir(const char path[]){
    creat_file(path, TYPE_DIR);
}
void touch(const char path[]){
    creat_file(path, TYPE_FILE);
}
void copy(const char src_path[], const char dst_path[]){
    int src_in_id;
    if(look_up(src_path, &src_in_id) == 0){
        panic("没有找到该路径下需要复制的源文件");
        return;
    }
    Inode src_in = read_inode(src_in_id);

    if(src_in.file_type == TYPE_FILE)
        creat_file(dst_path, TYPE_FILE);
    else if(src_in.file_type == TYPE_DIR)
        ;//TODO
}
void shutdown(){
    Close();
    exit(0);
}


int parse_cmd(const char input[PATH_MAX], char arg[3][PATH_MAX]){
    int input_len = strlen(input);
    if(input_len == 0)
        return EMPTY;
    int ret = 0, cnt = 0;
    for (int p = 0; p < input_len; ++p) {
        int i = 0;
        if(!isspace(input[p])) {
            if(cnt >= 3)
            panic("你输入了太多参数了！");
            while (p < input_len && !isspace(input[p])) {
                arg[cnt][i++] = input[p++];
            }
            ++cnt;
        }
    }
    if(strcmp(arg[0], "ls")==0) ret = LIST;
    else if(strcmp(arg[0], "mkdir")==0) ret = MKDIR;
    else if(strcmp(arg[0], "touch")==0) ret = TOUCH;
    else if(strcmp(arg[0], "copy") ==0) ret = COPY;
    else if(strcmp(arg[0], "shutdown")==0)  ret = SHUTDOWN;
    else if(strcmp(arg[0], "clean")==0)  ret = CLEAN;
    return ret;
}

void clean(){

    while (1){
        printf("确认删除磁盘上所有文件吗？(y/Y)\n");
        char ch = getchar(); getchar();
        if(ch == 'n' || ch == 'N')
            return;
        else if(ch == 'y' || ch == 'Y'){
            memset(buf, 0, sizeof(buf));
            for (int i = 0; i < BLOCK_SIZE; ++i) {
                Write(i, buf);
            }
            return;
        }
        printf("请重新输入！\n");
    }
}

void shell(){
    while (1){
        sh:
        printf("==> ");
        char input[PATH_MAX], arg[3][PATH_MAX];
        memset(input, 0, sizeof(input));
        memset(arg, 0, sizeof(arg));
        fgets(input, PATH_MAX, stdin);
        input[strlen(input)-1] = 0; //去掉换行符
        int cmd = parse_cmd(input, arg);
        switch (cmd) {
            case EMPTY:
                break;
            case LIST:
                list(arg[1]);
                break;
            case MKDIR:
                mkdir(arg[1]);
                break;
            case TOUCH:
                touch(arg[1]);
                break;
            case COPY:
                copy(arg[1], arg[2]);
                break;
            case SHUTDOWN:
                shutdown();
                exit(0);
            case CLEAN:
                clean();
                break;
            default:
                printf("请输入正确的命令\n");
                break;
        }
    }
}