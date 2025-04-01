#include "ext2.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

// Function to open the Ext2 file system
Ext2File *openExt2(const char *filename) {
    VDIFile *vdi = vdiOpen(filename);
    if (!vdi) {
        printf("Failed to open VDI file.\n");
        return NULL;
    }

    printf("VDI file opened successfully.\n");

    // Find the first partition with type 0x83 (Linux filesystem)
    MBRPartition *partition = NULL;
    for (int i = 0; i < 4; i++) {
        partition = mbrOpen(vdi, i);
        if (partition) {
            printf("Partition %d, Type: 0x%02x, Start LBA: %u, Sector Count: %u\n", i, partition->type, partition->start / 512, partition->size / 512);
            if (partition->type == 0x83) {
                printf("Found ext2 partition at index %d.\n", i);
                break;
            }
            mbrClose(partition);
        }
    }

    if (!partition || partition->type != 0x83) {
        printf("No ext2 partition found.\n");
        vdiClose(vdi);
        return NULL;
    }

    Ext2File *ext2 = malloc(sizeof(Ext2File));
    if (!ext2) {
        printf("Failed to allocate memory for Ext2File.\n");
        mbrClose(partition);
        vdiClose(vdi);
        return NULL;
    }

    ext2->vdi = vdi;
    ext2->partition = partition;

    // Read the superblock
    if (!fetchSuperblock(ext2, 1, &ext2->superblock)) {
        printf("Failed to read superblock.\n");
        free(ext2);
        mbrClose(partition);
        vdiClose(vdi);
        return NULL;
    }

    printf("Superblock read successfully.\n");
    printf("Superblock magic number: 0x%x\n", ext2->superblock.s_magic);

    // Calculate block size and number of block groups
    ext2->block_size = 1024 << ext2->superblock.s_log_block_size;
    ext2->num_block_groups = (ext2->superblock.s_blocks_count + ext2->superblock.s_blocks_per_group - 1) / ext2->superblock.s_blocks_per_group;

    // Allocate memory for block group descriptor table
    ext2->bgdt = malloc(ext2->num_block_groups * sizeof(Ext2BlockGroupDescriptor));
    if (!ext2->bgdt) {
        printf("Failed to allocate memory for block group descriptor table.\n");
        free(ext2);
        mbrClose(partition);
        vdiClose(vdi);
        return NULL;
    }

    // Fetch the block group descriptor table
    if (!fetchBGDT(ext2, 2, ext2->bgdt)) {
        printf("Failed to read block group descriptor table.\n");
        free(ext2->bgdt);
        free(ext2);
        mbrClose(partition);
        vdiClose(vdi);
        return NULL;
    }

    printf("Block group descriptor table read successfully.\n");

    return ext2;
}

// Function to close the Ext2 file system
void closeExt2(Ext2File *ext2) {
    if (ext2) {
        free(ext2->bgdt);
        mbrClose(ext2->partition);
        vdiClose(ext2->vdi);
        free(ext2);
    }
}

int32_t fetchInode(Ext2File *f, uint32_t iNum, Ext2Inode *buf) {
    if (!f || !buf || iNum == 0) return -1;
    uint32_t inodeIndex = iNum - 1;
    uint32_t group = inodeIndex / f->superblock.s_inodes_per_group;
    uint32_t indexInGroup = inodeIndex % f->superblock.s_inodes_per_group;
    uint32_t blockContainingInode = f->bgdt[group].bg_inode_table + (indexInGroup * f->superblock.s_inode_size / f->block_size);
    uint32_t offsetInBlock = (indexInGroup * f->superblock.s_inode_size) % f->block_size;

    uint8_t blockBuffer[f->block_size];
    if (!fetchBlock(f, blockContainingInode, blockBuffer)) return -1;
    memcpy(buf, blockBuffer + offsetInBlock, sizeof(Ext2Inode));
    return 0;
}

int32_t writeInode(Ext2File *f, uint32_t iNum, Ext2Inode *buf) {
    if (!f || !buf || iNum == 0) return -1;
    uint32_t inodeIndex = iNum - 1;
    uint32_t group = inodeIndex / f->superblock.s_inodes_per_group;
    uint32_t indexInGroup = inodeIndex % f->superblock.s_inodes_per_group;
    uint32_t blockContainingInode = f->bgdt[group].bg_inode_table + (indexInGroup * f->superblock.s_inode_size / f->block_size);
    uint32_t offsetInBlock = (indexInGroup * f->superblock.s_inode_size) % f->block_size;

    uint8_t blockBuffer[f->block_size];
    if (!fetchBlock(f, blockContainingInode, blockBuffer)) return -1;
    memcpy(blockBuffer + offsetInBlock, buf, sizeof(Ext2Inode));
    if (!writeBlock(f, blockContainingInode, blockBuffer)) return -1;
    return 0;
}

int32_t inodeInUse(Ext2File *f, uint32_t iNum) {
    if (!f || iNum == 0) return -1;
    uint32_t inodeIndex = iNum - 1;
    uint32_t group = inodeIndex / f->superblock.s_inodes_per_group;
    uint32_t indexInGroup = inodeIndex % f->superblock.s_inodes_per_group;
    uint32_t bitmapBlock = f->bgdt[group].bg_inode_bitmap;
    uint8_t bitmapBuffer[f->block_size];
    if (!fetchBlock(f, bitmapBlock, bitmapBuffer)) return -1;
    uint32_t byteIndex = indexInGroup / 8;
    uint32_t bitIndex = indexInGroup % 8;
    return (bitmapBuffer[byteIndex] & (1 << bitIndex)) != 0;
}

uint32_t allocateInode(Ext2File *f, int32_t group) {
    if (!f) return 0;
    for (uint32_t g = (group == -1 ? 0 : group); g < f->num_block_groups; g++) {
        uint32_t bitmapBlock = f->bgdt[g].bg_inode_bitmap;
        uint8_t bitmapBuffer[f->block_size];
        if (!fetchBlock(f, bitmapBlock, bitmapBuffer)) continue;
        for (uint32_t i = 0; i < f->superblock.s_inodes_per_group; i++) {
            uint32_t byteIndex = i / 8;
            uint32_t bitIndex = i % 8;
            if ((bitmapBuffer[byteIndex] & (1 << bitIndex)) == 0) {
                bitmapBuffer[byteIndex] |= (1 << bitIndex);
                if (!writeBlock(f, bitmapBlock, bitmapBuffer)) return 0;
                f->bgdt[g].bg_free_inodes_count--;
                writeBGDT(f, g, &f->bgdt[g]);
                return g * f->superblock.s_inodes_per_group + i + 1;
            }
        }
        if (group != -1) break;
    }
    return 0;
}

int32_t freeInode(Ext2File *f, uint32_t iNum) {
    if (!f || iNum == 0) return -1;
    uint32_t inodeIndex = iNum - 1;
    uint32_t group = inodeIndex / f->superblock.s_inodes_per_group;
    uint32_t indexInGroup = inodeIndex % f->superblock.s_inodes_per_group;
    uint32_t bitmapBlock = f->bgdt[group].bg_inode_bitmap;
    uint8_t bitmapBuffer[f->block_size];
    if (!fetchBlock(f, bitmapBlock, bitmapBuffer)) return -1;
    uint32_t byteIndex = indexInGroup / 8;
    uint32_t bitIndex = indexInGroup % 8;
    bitmapBuffer[byteIndex] &= ~(1 << bitIndex);
    if (!writeBlock(f, bitmapBlock, bitmapBuffer)) return -1;
    f->bgdt[group].bg_free_inodes_count++;
    writeBGDT(f, group, &f->bgdt[group]);
    return 0;
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

bool fetchBlock(Ext2File *ext2, uint32_t blockNum, void *buf) {
    if (!ext2 || !buf) return false;
    off_t offset = blockNum * ext2->block_size;
    if (vdiSeek(ext2->vdi, offset, SEEK_SET) != offset) return false;
    if (vdiRead(ext2->vdi, buf, ext2->block_size) != ext2->block_size) return false;
    return true;
}

bool writeBlock(Ext2File *ext2, uint32_t blockNum, void *buf) {
    if (!ext2 || !buf) return false;
    off_t offset = blockNum * ext2->block_size;
    if (vdiSeek(ext2->vdi, offset, SEEK_SET) != offset) return false;
    if (vdiWrite(ext2->vdi, buf, ext2->block_size) != ext2->block_size) return false;
    return true;
}

bool fetchSuperblock(Ext2File *ext2, uint32_t blockNum, Ext2Superblock *sb) {
    uint8_t buf[ext2->block_size];
    if (!fetchBlock(ext2, blockNum, buf)) return false;
    memcpy(sb, buf + 1024 % ext2->block_size, sizeof(Ext2Superblock));
    return true;
}

bool writeSuperblock(Ext2File *ext2, uint32_t blockNum, Ext2Superblock *sb) {
    uint8_t buf[ext2->block_size];
    if (!fetchBlock(ext2, blockNum, buf)) return false;
    memcpy(buf + 1024 % ext2->block_size, sb, sizeof(Ext2Superblock));
    return writeBlock(ext2, blockNum, buf);
}

bool fetchBGDT(Ext2File *ext2, uint32_t blockNum, Ext2BlockGroupDescriptor *bgdt) {
    uint8_t buf[ext2->block_size];
    if (!fetchBlock(ext2, blockNum, buf)) return false;
    memcpy(bgdt, buf, ext2->num_block_groups * sizeof(Ext2BlockGroupDescriptor));
    return true;
}

bool writeBGDT(Ext2File *ext2, uint32_t blockNum, Ext2BlockGroupDescriptor *bgdt) {
    uint8_t buf[ext2->block_size];
    if (!fetchBlock(ext2, blockNum, buf)) return false;
    memcpy(buf, bgdt, ext2->num_block_groups * sizeof(Ext2BlockGroupDescriptor));
    return writeBlock(ext2, blockNum, buf);
}
