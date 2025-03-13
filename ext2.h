#ifndef EXT2_H
#define EXT2_H

#include "mbr.h"

typedef struct {
    MBRPartition *partition;  // Opened partition containing ext2 filesystem
    uint32_t blockSize;       // File system block size
    uint32_t numBlockGroups;  // Number of block groups
    uint32_t firstDataBlock;  // First data block in the filesystem
    uint32_t totalInodes;     // Total inodes in filesystem
    uint32_t totalBlocks;     // Total blocks in filesystem
} Ext2File;

Ext2File *ext2Open(VDIFile *vdi);
void ext2Close(Ext2File *fs);
bool fetchBlock(Ext2File *fs, uint32_t blockNum, void *buf);
bool writeBlock(Ext2File *fs, uint32_t blockNum, void *buf);
bool fetchSuperblock(Ext2File *fs, uint32_t blockNum, void *sb);
bool writeSuperblock(Ext2File *fs, uint32_t blockNum, void *sb);
bool fetchBGDT(Ext2File *fs, uint32_t blockNum, void *bgdt);
bool writeBGDT(Ext2File *fs, uint32_t blockNum, void *bgdt);
void displaySuperblock(Ext2File *fs, void *sb);
void displayBGDT(Ext2File *fs, void *bgdt);

#endif