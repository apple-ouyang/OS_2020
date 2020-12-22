//
// Created by ouyang on 2020/12/20.
//

#include "util.h"
#include "IO.h"
#include <stdio.h>
#include <stdlib.h>

char buf[BLOCK_SIZE], buf2[BLOCK_SIZE], buft[BLOCK_SIZE];
char current_path[PATH_MAX];

void panic(char* s){
    printf("错误：%s\n", s);
    printf("按回车继续");
    getchar();
}

//返回从0开始的一个inode编号
int alloc_inode_id(){
    sp_block sp = read_super();
    if(sp.free_inode_count == 0)
        panic("无法分配inode：sp说没有足够inode了");
    for (int id = 0; id < INODE_NUM; ++id) {
        int i = id / INODE_MAP, j = id % INODE_MAP;
        if((sp.inode_map[i] & (1<<j)) == 0){
            sp.inode_map[i] |= 1<<j;
            write_super(sp);
            return id;
        }

    }
    panic("alloc_inode_id: 没有找到空闲inode");
    return -1;
}

//返回文件系统中空闲的数据块编号，从BLOCK_START_NUM开始
int alloc_block(){
    sp_block sp = read_super();
    if(sp.free_block_count == 0)
        panic("alloc_block: sp说没有足够block了");
    for (int id = 0; id < BLOCK_SIZE; ++id) {
        int i = id/BLOCK_MAP, j = id%BLOCK_MAP;
        if((sp.block_map[i] & (1<<j)) == 0){
            sp.block_map[i] |= 1<<j;
            write_super(sp);
            return id + BLOCK_START_NUM;
        }
    }
    panic("alloc_block: 没有找到空闲block");
    return -1;
}