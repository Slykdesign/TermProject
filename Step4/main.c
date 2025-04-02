#include "ext2.h"
#include "inode.h" // Include the new inode header
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>

// Function prototypes
void displayBufferPage(uint8_t *buf, uint32_t count, uint32_t skip, uint64_t offset);
void displayBuffer(uint8_t *buf, uint32_t count, uint64_t offset);
void displaySuperblock(Ext2Superblock *sb);
void printUUID(uint8_t *uuid);
void displayBGDT(Ext2BlockGroupDescriptor *bgdt, uint32_t num_groups);
void displayInode(Ext2Inode *inode);

int main() {
    Ext2File *ext2 = openExt2("./good-fixed-1k.vdi");
    if (!ext2) {
        printf("Failed to open ext2 file system.\n");
        return 1;
    }

    printf("Superblock from block 0\n");
    printf("Superblock contents:\n");
    displaySuperblock(&ext2->superblock);

    printf("Block group descriptor table:\n");
    displayBGDT(ext2->bgdt, ext2->num_block_groups);

    // Fetch and display inode 2 (root inode)
    Ext2Inode inode;
    if (fetchInode(ext2, 2, &inode) == 0) {
        printf("Inode 2 (Root):\n");
        displayInode(&inode);
    } else {
        fprintf(stderr, "Failed to fetch inode 2.\n");
    }

    // Fetch and display inode 11 (lost+found)
    if (fetchInode(ext2, 11, &inode) == 0) {
        printf("Inode 11 (lost+found):\n");
        displayInode(&inode);
    } else {
        fprintf(stderr, "Failed to fetch inode 11.\n");
    }

    closeExt2(ext2);
    return 0;
}

// Function Definitions
void displayBufferPage(uint8_t *buf, uint32_t count, uint32_t skip, uint64_t offset) {
    printf("Offset: 0x%lx\n", offset);
    for (uint32_t i = skip; i < count; i += 16) {
        printf("%04x | ", i);
        for (uint32_t j = 0; j < 16 && i + j < count; j++) {
            printf("%02x ", buf[i + j]);
        }
        printf("| ");
        for (uint32_t j = 0; j < 16 && i + j < count; j++) {
            printf("%c", isprint(buf[i + j]) ? buf[i + j] : '.');
        }
        printf("\n");
    }
}

void displayBuffer(uint8_t *buf, uint32_t count, uint64_t offset) {
    uint32_t step = 256;
    for (uint32_t i = 0; i < count; i += step) {
        displayBufferPage(buf, count, i, offset + i);
    }
}

void displaySuperblock(Ext2Superblock *sb) {
    printf("Number of inodes: %u\n", sb->s_inodes_count);
    printf("Number of blocks: %u\n", sb->s_blocks_count);
    printf("Number of reserved blocks: %u\n", sb->s_r_blocks_count);
    printf("Number of free blocks: %u\n", sb->s_free_blocks_count);
    printf("Number of free inodes: %u\n", sb->s_free_inodes_count);
    printf("First data block: %u\n", sb->s_first_data_block);
    printf("Log block size: %u (%u)\n", sb->s_log_block_size, 1024 << sb->s_log_block_size);
    printf("Log fragment size: %u (%u)\n", sb->s_log_frag_size, 1024 << sb->s_log_frag_size);
    printf("Blocks per group: %u\n", sb->s_blocks_per_group);
    printf("Fragments per group: %u\n", sb->s_frags_per_group);
    printf("Inodes per group: %u\n", sb->s_inodes_per_group);
    printf("Last mount time: %s", ctime((time_t*)&sb->s_mtime));
    printf("Last write time: %s", ctime((time_t*)&sb->s_wtime));
    printf("Mount count: %u\n", sb->s_mnt_count);
    printf("Max mount count: %u\n", sb->s_max_mnt_count);
    printf("Magic number: 0x%x\n", sb->s_magic);
    printf("State: %u\n", sb->s_state);
    printf("Error processing: %u\n", sb->s_errors);
    printf("Revision level: %u.%u\n", sb->s_rev_level, sb->s_minor_rev_level);
    printf("Last system check: %s", ctime((time_t*)&sb->s_lastcheck));
    printf("Check interval: %u\n", sb->s_checkinterval);
    printf("OS creator: %u\n", sb->s_creator_os);
    printf("Default reserve UID: %u\n", sb->s_def_resuid);
    printf("Default reserve GID: %u\n", sb->s_def_resgid);
    printf("First inode number: %u\n", sb->s_first_ino);
    printf("Inode size: %u\n", sb->s_inode_size);
    printf("Block group number: %u\n", sb->s_block_group_nr);
    printf("Feature compatibility bits: 0x%08x\n", sb->s_feature_compat);
    printf("Feature incompatibility bits: 0x%08x\n", sb->s_feature_incompat);
    printf("Feature read/only compatibility bits: 0x%08x\n", sb->s_feature_ro_compat);
    printf("UUID: ");
    printUUID(sb->s_uuid);
    printf("\n");
    printf("Volume name: [%.*s]\n", (int)sizeof(sb->s_volume_name), sb->s_volume_name);
    printf("Last mount point: [%.*s]\n", (int)sizeof(sb->s_last_mounted), sb->s_last_mounted);
    printf("Algorithm bitmap: 0x%08x\n", sb->s_algorithm_usage_bitmap);
    printf("Number of blocks to preallocate: %u\n", sb->s_prealloc_blocks);
    printf("Number of blocks to preallocate for directories: %u\n", sb->s_prealloc_dir_blocks);
    printf("Journal UUID: ");
    printUUID(sb->s_journal_uuid);
    printf("\n");
    printf("Journal inode number: %u\n", sb->s_journal_inum);
    printf("Journal device number: %u\n", sb->s_journal_dev);
    printf("Journal last orphan inode number: %u\n", sb->s_last_orphan);
    printf("Default hash version: %u\n", sb->s_def_hash_version);
    printf("Default mount option bitmap: 0x%08x\n", sb->s_default_mount_opts);
    printf("First meta block group: %u\n", sb->s_first_meta_bg);
}

void printUUID(uint8_t *uuid) {
    for (int i = 0; i < 16; i++) {
        printf("%02x", uuid[i]);
        if (i == 3 || i == 5 || i == 7 || i == 9) printf("-");
    }
}

void displayBGDT(Ext2BlockGroupDescriptor *bgdt, uint32_t num_groups) {
    printf("Block Block Inode Inode Free Free Used\n");
    printf("Number Bitmap Bitmap Table Blocks Inodes Dirs\n");
    printf("------ ------ ------ ----- ------ ------ ----\n");
    for (uint32_t i = 0; i < num_groups; i++) {
        printf("%6u %6u %6u %5u %6u %6u %4u\n",
               i,
               bgdt[i].bg_block_bitmap,
               bgdt[i].bg_inode_bitmap,
               bgdt[i].bg_inode_table,
               bgdt[i].bg_free_blocks_count,
               bgdt[i].bg_free_inodes_count,
               bgdt[i].bg_used_dirs_count);
    }
}

void displayInode(Ext2Inode *inode) {
    printf("Mode: %o\n", inode->i_mode);
    printf("Size: %u\n", inode->i_size);
    printf("Blocks: %u\n", inode->i_blocks);
    printf("UID / GID: %u / %u\n", inode->i_uid, inode->i_gid);
    printf("Links: %u\n", inode->i_links_count);
    printf("Created: %s", ctime((time_t *)&inode->i_ctime));
    printf("Last access: %s", ctime((time_t *)&inode->i_atime));
    printf("Last modification: %s", ctime((time_t *)&inode->i_mtime));
    printf("Deleted: %s", ctime((time_t *)&inode->i_dtime));
    printf("Flags: %08x\n", inode->i_flags);
    printf("File version: %u\n", inode->i_generation);
    printf("ACL block: %u\n", inode->i_file_acl);
    printf("Direct blocks: ");
    for (int i = 0; i < 12; i++) {
        printf("%u ", inode->i_block[i]);
    }
    printf("\nSingle indirect block: %u\n", inode->i_block[12]);
    printf("Double indirect block: %u\n", inode->i_block[13]);
    printf("Triple indirect block: %u\n", inode->i_block[14]);
}
