#include "util.h"
#include "IO.h"
#include "shell.h"
#include <stdio.h>
#include "string.h"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wwritable-strings"

int if_has_init(){
    FILE* fp;
    if((fp = fopen("disk_config", "w+")) == NULL)
        panic("error read from disk_config");
    int if_init;
    if_init = fgetc(fp);
    fclose(fp);
    return if_init == 1;
}

void set_has_init(){
    FILE* fp;
    if((fp = fopen("disk_config", "w+")) == NULL)
        panic("error read from disk_config");
    fputc(1, fp);
    fclose(fp);
}

void init(){
    Open();
    sp_block sp = read_super();
    if(!if_has_init()){
        printf("初始化超级块\n");
        sp.magic_num = DISK_MAGIC;
        sp.free_block_count = BLOCK_NUM;
        sp.free_inode_count = INODE_NUM;
        memset(sp.block_map, 0, sizeof(sp.block_map));
        memset(sp.inode_map, 0, sizeof(sp.inode_map));
        sp.dir_inode_count = 0;
        set_has_init();
    }else if(sp.magic_num!=DISK_MAGIC)
        panic("无法读取超级块");
}


void make_root(){
    sp_block sp = read_super();
    if(sp.dir_inode_count){
        printf("root路径已经初始化\n");
        return;
    }
    printf("正在初始化root路径\n");
    sp.dir_inode_count++;
    sp.free_inode_count--;
    sp.free_block_count--;
    int i = 0, j = 0;
    sp.inode_map[i] |= 1<<j;

    struct inode root_i = read_inode(0);
    root_i.file_type = TYPE_DIR;
    root_i.link = 1;
    write_inode(0, root_i);

    write_super(sp);
}

int main() {
    printf("青春版文件系统正在启动\n");
    init();
    make_root();
    printf("初始化成功！\n");
    shell();
    return 0;
}

#pragma clang diagnostic pop