//
// Created by ouyang on 2020/12/20.
//

#ifndef FILE_SYSTEM_C99_UTIL_H
#define FILE_SYSTEM_C99_UTIL_H

#include <stdint.h>
#include "disk.h"

#define BLOCK_SIZE      (2*DEVICE_BLOCK_SIZE)
#define PATH_MAX        1000
#define NAME_MAX        121
#define INODE_NUM       1024
#define DISK_MAGIC      0x0ec0de
#define INODE_SIZE      32
#define BLOCK_MAP       128
#define INODE_MAP       32
#define INODE_NUM_PER_BLOCK (BLOCK_SIZE/INODE_SIZE) //一个block有多少个inode
#define SUPER_NUM       1                   //super_block从1号开始，0号留给boot启动文件
#define INODE_START_NUM 2                   //inode从2号开始
#define INODE_BLOCK_PTR 6
#define BLOCK_START_NUM (INODE_START_NUM+32)  //inode一共有1024*32byte，也就是一共占有32块
#define BLOCK_NUM       (4096 - BLOCK_START_NUM - 1)
#define TYPE_FILE       1
#define TYPE_DIR        2

typedef struct super_block {
    int32_t magic_num;                  // 幻数
    int32_t free_block_count;           // 空闲数据块数
    int32_t free_inode_count;           // 空闲inode数
    int32_t dir_inode_count;            // 目录inode数
    uint32_t block_map[BLOCK_MAP];            // 数据块占用位图
    uint32_t inode_map[INODE_MAP];             // inode占用位图
} sp_block;


typedef struct inode {
    uint32_t size;              // 文件大小
    uint16_t file_type;         // 文件类型（文件/文件夹）
    uint16_t link;              // 连接数
    uint32_t block_point[6];    // 数据块指针
} Inode;

typedef struct dir_item {               // 目录项一个更常见的叫法是 dirent(directory entry)
    uint32_t inode_id;          // 当前目录项表示的文件/目录的对应inode
    uint16_t valid;             // 当前目录项是否有效
    uint8_t type;               // 当前目录项类型（文件/目录）
    char name[121];             // 目录项表示的文件/目录的文件名/目录名
} Dir;

extern char buf[BLOCK_SIZE], buf2[BLOCK_SIZE], buft[BLOCK_SIZE];
extern char current_path[PATH_MAX];

void panic(char*);
void map_inode(int id, int* i, int* j);
int  get_inode(int i, int j);
void map_block(int num, int* i, int* j);
int  get_block(int i, int j);

//分配一个inode
int alloc_inode_id();
int alloc_block();

#endif //FILE_SYSTEM_C99_UTIL_H
