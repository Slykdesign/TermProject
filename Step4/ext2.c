#include "ext2.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

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

// Function to read a block from the Ext2 file system
bool fetchBlock(Ext2File *ext2, uint32_t blockNum, void *buf) {
    off_t offset = ext2->partition->start + blockNum * ext2->block_size;
    if (vdiSeek(ext2->vdi, offset, SEEK_SET) == (off_t)-1) return false;
    return vdiRead(ext2->vdi, buf, ext2->block_size) == ext2->block_size;
}

// Function to write a block to the Ext2 file system
bool writeBlock(Ext2File *ext2, uint32_t blockNum, void *buf) {
    off_t offset = ext2->partition->start + blockNum * ext2->block_size;
    if (vdiSeek(ext2->vdi, offset, SEEK_SET) == (off_t)-1) return false;
    return vdiWrite(ext2->vdi, buf, ext2->block_size) == ext2->block_size;
}

// Function to read the superblock from the Ext2 file system
bool fetchSuperblock(Ext2File *ext2, uint32_t blockNum, Ext2Superblock *sb) {
    uint8_t buf[1024];
    if (!fetchBlock(ext2, blockNum, buf)) return false;
    memcpy(sb, buf, sizeof(Ext2Superblock));
    return sb->s_magic == 0xef53;
}

// Function to write the superblock to the Ext2 file system
bool writeSuperblock(Ext2File *ext2, uint32_t blockNum, Ext2Superblock *sb) {
    uint8_t buf[1024] = {0};
    memcpy(buf, sb, sizeof(Ext2Superblock));
    return writeBlock(ext2, blockNum, buf);
}

// Function to read the block group descriptor table from the Ext2 file system
bool fetchBGDT(Ext2File *ext2, uint32_t blockNum, Ext2BlockGroupDescriptor *bgdt) {
    uint8_t *buf = malloc(ext2->block_size);
    if (!buf) return false;
    if (!fetchBlock(ext2, blockNum, buf)) {
        free(buf);
        return false;
    }
    memcpy(bgdt, buf, ext2->num_block_groups * sizeof(Ext2BlockGroupDescriptor));
    free(buf);
    return true;
}

// Function to write the block group descriptor table to the Ext2 file system
bool writeBGDT(Ext2File *ext2, uint32_t blockNum, Ext2BlockGroupDescriptor *bgdt) {
    uint8_t *buf = malloc(ext2->block_size);
    if (!buf) return false;
    memcpy(buf, bgdt, ext2->num_block_groups * sizeof(Ext2BlockGroupDescriptor));
    bool result = writeBlock(ext2, blockNum, buf);
    free(buf);
    return result;
}
