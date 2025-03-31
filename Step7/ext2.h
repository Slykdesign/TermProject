#ifndef EXT2_H
#define EXT2_H

#include <stdint.h>

#define BLOCK_SIZE 1024  // Standard ext2 block size

// Ext2 structures (simplified)
struct ext2_super_block {
    uint32_t s_first_data_block;
};

struct ext2_group_desc {
    uint32_t bg_inode_table;
};

struct ext2_inode {
    uint16_t i_mode;
    uint32_t i_size;
    uint32_t i_block[15];
};

struct ext2_dir_entry {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    char name[255];  // Max name length
};

// Struct representing an opened Ext2 file
struct Ext2File {
    int fd;  // File descriptor of the VDI file
    struct ext2_super_block sb;
    struct ext2_group_desc bg;
};

// Function declarations
int readInode(struct Ext2File *f, uint32_t iNum, struct ext2_inode *inode);
uint32_t searchDir(struct Ext2File *f, uint32_t iNum, char *target);
uint32_t traversePath(struct Ext2File *f, char *path);

#endif
