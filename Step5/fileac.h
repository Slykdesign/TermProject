#ifndef FILE_ACCESS_H
#define FILE_ACCESS_H

#include "ext2.h"

typedef struct {
} FileAccess;

// Function prototypes for file access
int32_t fetchBlockFromFile(Ext2Inode *inode, uint32_t bNum, void *buf);
int32_t writeBlockToFile(Ext2Inode *inode, uint32_t bNum, void *buf);

#endif // FILE_ACCESS_H
