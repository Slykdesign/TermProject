#ifndef INODE_H
#define INODE_H

#include "ext2.h"

int32_t fetchInode(Ext2File *f, uint32_t iNum, Ext2Inode *buf);
int32_t writeInode(Ext2File *f, uint32_t iNum, Ext2Inode *buf);
int32_t inodeInUse(Ext2File *f, uint32_t iNum);
uint32_t allocateInode(Ext2File *f, int32_t group);
int32_t freeInode(Ext2File *f, uint32_t iNum);
void displayInode(Ext2Inode *inode);

#endif
