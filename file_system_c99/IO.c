//
// Created by ouyang on 2020/12/20.
//

#include "IO.h"
#include "disk.h"
#include "util.h"
#include "stdio.h"

void Open(){
    printf("正在创建虚拟磁盘\n");
    if(open_disk()<0)
        panic("error open disk");
}

void Close(){
    printf("青春版文件系统关闭\n");
    if(close_disk()<0)
        panic("error close disk");
}

void Read(unsigned int block_num, char* buff){
    if(disk_read_block(2*block_num, buff) < 0){\
        panic("read 1 error");
    }
    if(disk_read_block(2*block_num+1, buff+DEVICE_BLOCK_SIZE) < 0){\
        panic("read 2 error");
    }
}

void Write(unsigned int block_num, char* buff){
    if(disk_write_block(2*block_num, buff) < 0){
        panic("write 1 error");
    }
    if(disk_write_block(2*block_num+1, buff+DEVICE_BLOCK_SIZE) < 0){
        panic("write 2 error");
    }
}
sp_block read_super(){
    Read(SUPER_NUM, buft);
    return *((sp_block*)buft);
}

void write_super(sp_block new_sp){
    Read(SUPER_NUM, buft);
    *((sp_block*)buft) = new_sp;
    Write(SUPER_NUM, buft);
}

Inode read_inode(int id){
    int i = id / INODE_NUM_PER_BLOCK, j = id % INODE_NUM_PER_BLOCK;
    Read(i+INODE_START_NUM, buft);
    Inode* inode_array = (Inode*) buft;
    return inode_array[j];
}

void write_inode(int id, Inode new_inode){
    int i = id / INODE_NUM_PER_BLOCK, j = id % INODE_NUM_PER_BLOCK;
    Read(i+INODE_START_NUM, buft);
    Inode* inode_array = (Inode*) buft;
    inode_array[j] = new_inode;
    Write(i+INODE_START_NUM, buft);
}
