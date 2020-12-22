//
// Created by ouyang on 2020/12/20.
//

#ifndef FILE_SYSTEM_C99_IO_H
#define FILE_SYSTEM_C99_IO_H

#include "util.h"

void Open();
void Close();
void Read(unsigned int block_num, char* buff);
void Write(unsigned int block_num, char* buff);

sp_block read_super();
void write_super(sp_block new_sp);

//id是inode数组的编号，读写时覆盖了buft，需要确保buft没有数据
Inode read_inode(int id);
void write_inode(int id, Inode new_inode);

#endif //FILE_SYSTEM_C99_IO_H
