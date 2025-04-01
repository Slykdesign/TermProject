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
    uint32_t s_mtime;
    uint32_t s_wtime;
    uint16_t s_mnt_count;
    uint16_t s_max_mnt_count;
    uint16_t s_magic;
    uint16_t s_state;
    uint16_t s_errors;
    uint16_t s_minor_rev_level;
    uint32_t s_lastcheck;
    uint32_t s_checkinterval;
    uint32_t s_creator_os;
    uint32_t s_rev_level;
    uint16_t s_def_resuid;
    uint16_t s_def_resgid;
    uint32_t s_first_ino;
    uint16_t s_inode_size;
    uint16_t s_block_group_nr;
    uint32_t s_feature_compat;
    uint32_t s_feature_incompat;
    uint32_t s_feature_ro_compat;
    uint8_t  s_uuid[16];
    char     s_volume_name[16];
    char     s_last_mounted[64];
    uint32_t s_algorithm_usage_bitmap;
    uint8_t  s_prealloc_blocks;
    uint8_t  s_prealloc_dir_blocks;
    uint16_t s_padding1;
    uint8_t  s_journal_uuid[16];
    uint32_t s_journal_inum;
    uint32_t s_journal_dev;
    uint32_t s_last_orphan;
    uint32_t s_hash_seed[4];
    uint8_t  s_def_hash_version;
    uint8_t  s_reserved_char_pad;
    uint16_t s_reserved_word_pad;
    uint32_t s_default_mount_opts;
    uint32_t s_first_meta_bg;
    uint32_t s_reserved[190];
} Ext2Superblock;

typedef struct {
    uint32_t bg_block_bitmap;
    uint32_t bg_inode_bitmap;
    uint32_t bg_inode_table;
    uint16_t bg_free_blocks_count;
    uint16_t bg_free_inodes_count;
    uint16_t bg_used_dirs_count;
    uint16_t bg_pad;
    uint32_t bg_reserved[3];
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
