#ifndef EXT2_H
#define EXT2_H

#include "vdi.h"

// Define the Ext2 Superblock structure
typedef struct {
    uint32_t s_inodes_count;
    uint32_t s_blocks_count;
    uint32_t s_r_blocks_count;
    uint32_t s_free_blocks_count;
    uint32_t s_free_inodes_count;
    uint32_t s_first_data_block;
    uint32_t s_log_block_size;
    uint32_t s_log_frag_size;
    uint32_t s_blocks_per_group;
    uint32_t s_frags_per_group;
    uint32_t s_inodes_per_group;
    // ... other fields ...
    uint16_t s_magic;
} Ext2Superblock;

// Define the Ext2 Block Group Descriptor structure
typedef struct {
    uint32_t bg_block_bitmap;      // Blocks bitmap block
    uint32_t bg_inode_bitmap;      // Inodes bitmap block
    uint32_t bg_inode_table;       // Inodes table block
    uint16_t bg_free_blocks_count; // Free blocks count
    uint16_t bg_free_inodes_count; // Free inodes count
    uint16_t bg_used_dirs_count;   // Directories count
    // ... other fields ...
} Ext2BlockGroupDescriptor;

// Define the Ext2 Inode structure
typedef struct {
    uint16_t i_mode;
    uint16_t i_uid;
    uint32_t i_size;
    uint32_t i_atime;
    uint32_t i_ctime;
    uint32_t i_mtime;
    uint32_t i_dtime;
    uint16_t i_gid;
    uint16_t i_links_count;
    uint32_t i_blocks;
    uint32_t i_flags;
    uint32_t i_osd1;
    uint32_t i_block[15];
    uint32_t i_generation;
    uint32_t i_file_acl;
    uint32_t i_dir_acl;
    uint32_t i_faddr;
    uint8_t  i_osd2[12];
} Ext2Inode;

// Define the Ext2 File structure
typedef struct {
    VDIFile *vdi;                  // Pointer to the VDI file
    MBRPartition *partition;       // Pointer to the partition
    Ext2Superblock superblock;     // Superblock structure
    Ext2BlockGroupDescriptor *bgdt; // Block group descriptor table
    uint32_t block_size;           // Block size
    uint32_t num_block_groups;     // Number of block groups
} Ext2File;

// Function prototypes
Ext2File *openExt2(const char *filename);
void closeExt2(Ext2File *ext2);
bool fetchBlock(Ext2File *ext2, uint32_t blockNum, void *buf);
bool writeBlock(Ext2File *ext2, uint32_t blockNum, void *buf);
bool fetchSuperblock(Ext2File *ext2, uint32_t blockNum, Ext2Superblock *sb);
bool writeSuperblock(Ext2File *ext2, uint32_t blockNum, Ext2Superblock *sb);
bool fetchBGDT(Ext2File *ext2, uint32_t blockNum, Ext2BlockGroupDescriptor *bgdt);
bool writeBGDT(Ext2File *ext2, uint32_t blockNum, Ext2BlockGroupDescriptor *bgdt);
int32_t fetchInode(Ext2File *f, uint32_t iNum, Ext2Inode *buf);
int32_t writeInode(Ext2File *f, uint32_t iNum, Ext2Inode *buf);
int32_t inodeInUse(Ext2File *f, uint32_t iNum);
uint32_t allocateInode(Ext2File *f, int32_t group);
int32_t freeInode(Ext2File *f, uint32_t iNum);

#endif
