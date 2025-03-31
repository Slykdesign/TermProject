#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ext2.h"

// Reads an inode from disk
int readInode(struct Ext2File *f, uint32_t iNum, struct ext2_inode *inode) {
    uint32_t inodeTableBlock = f->sb.s_first_data_block + f->bg.bg_inode_table;
    uint32_t inodeIndex = iNum - 1;
    uint32_t blockOffset = (inodeIndex * sizeof(struct ext2_inode)) % BLOCK_SIZE;
    uint32_t blockNum = inodeTableBlock + ((inodeIndex * sizeof(struct ext2_inode)) / BLOCK_SIZE);

    uint8_t block[BLOCK_SIZE];
    if (pread(f->fd, block, BLOCK_SIZE, blockNum * BLOCK_SIZE) != BLOCK_SIZE) {
        perror("Error reading inode");
        return -1;
    }

    memcpy(inode, block + blockOffset, sizeof(struct ext2_inode));
    return 0;
}

// Searches a directory for a file
uint32_t searchDir(struct Ext2File *f, uint32_t iNum, char *target) {
    struct ext2_inode inode;
    if (readInode(f, iNum, &inode) != 0) {
        return 0;
    }

    if ((inode.i_mode & 0xF000) != 0x4000) {
        return 0;  // Not a directory
    }

    uint8_t block[BLOCK_SIZE];
    for (int i = 0; i < 12 && inode.i_block[i] != 0; i++) {
        if (pread(f->fd, block, BLOCK_SIZE, inode.i_block[i] * BLOCK_SIZE) != BLOCK_SIZE) {
            perror("Error reading directory block");
            return 0;
        }

        uint32_t offset = 0;
        while (offset < BLOCK_SIZE) {
            struct ext2_dir_entry *entry = (struct ext2_dir_entry *)(block + offset);
            if (entry->inode == 0) {
                break;
            }

            char name[256] = {0};
            strncpy(name, entry->name, entry->name_len);
            name[entry->name_len] = '\0';

            if (strcmp(name, target) == 0) {
                return entry->inode;
            }

            offset += entry->rec_len;
        }
    }
    return 0;
}

// Traverses a file path and returns the inode number
uint32_t traversePath(struct Ext2File *f, char *path) {
    if (path[0] != '/') {
        return 0;
    }

    uint32_t iNum = 2;
    char *token = strtok(path, "/");

    while (token != NULL) {
        iNum = searchDir(f, iNum, token);
        if (iNum == 0) {
            return 0;
        }
        token = strtok(NULL, "/");
    }

    return iNum;
}