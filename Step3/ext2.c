#include "ext2.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define EXT2_SUPERBLOCK_OFFSET 1024
#define EXT2_SUPERBLOCK_MAGIC 0xEF53

Ext2File *ext2Open(VDIFile *vdi) {
    MBRPartition *partition = NULL;

    // Open first Linux partition (0x83)
    for (int i = 0; i < 4; i++) {
        MBRPartition *tempPart = mbrOpen(vdi, i);
        if (tempPart) {
            uint8_t type;
            lseek(vdi->fd, 0x1BE + (i * 16) + 4, SEEK_SET);
            read(vdi->fd, &type, 1);
            if (type == 0x83) {
                partition = tempPart;
                break;
            }
            mbrClose(tempPart);
        }
    }

    if (!partition) {
        printf("No ext2 partition found.\n");
        return NULL;
    }

    Ext2File *filesize = malloc(sizeof(Ext2File));
    if (!filesize) return NULL;

    filesize->partition = partition;

    // Read superblock
    uint8_t superblock[1024];
    fetchSuperblock(filesize, 0, superblock);

    // Validate superblock
    uint16_t magic;
    memcpy(&magic, superblock + 56, sizeof(uint16_t)); // s_magic field
    if (magic != EXT2_SUPERBLOCK_MAGIC) {
        printf("Invalid ext2 superblock.\n");
        ext2Close(filesize);
        return NULL;
    }

    // Extract key filesystem parameters
    filesize->blockSize = 1024 << *(uint32_t *)(superblock + 24); // s_log_block_size
    filesize->numBlockGroups = (*(uint32_t *)(superblock + 4) + *(uint32_t *)(superblock + 32) - 1) / *(uint32_t *)(superblock + 32);
    filesize->firstDataBlock = *(uint32_t *)(superblock + 20);
    filesize->totalInodes = *(uint32_t *)(superblock + 0);
    filesize->totalBlocks = *(uint32_t *)(superblock + 4);

    return filesize;
}

void ext2Close(Ext2File *filesize) {
    if (filesize) {
        mbrClose(filesize->partition);
        free(filesize);
    }
}

bool fetchBlock(Ext2File *filesize, uint32_t blockNum, void *buf) {
    off_t offset = filesize->partition->start + (blockNum * filesize->blockSize);
    lseek(filesize->partition->vdi->fd, offset, SEEK_SET);
    return read(filesize->partition->vdi->fd, buf, filesize->blockSize) == filesize->blockSize;
}

bool writeBlock(Ext2File *filesize, uint32_t blockNum, void *buf) {
    off_t offset = filesize->partition->start + (blockNum * filesize->blockSize);
    lseek(filesize->partition->vdi->fd, offset, SEEK_SET);
    return write(filesize->partition->vdi->fd, buf, filesize->blockSize) == filesize->blockSize;
}

bool fetchSuperblock(Ext2File *filesize, uint32_t blockNum, void *superblock) {
    off_t offset = filesize->partition->start + 1024;  // Superblock is always at 1024-byte offset
    lseek(filesize->partition->vdi->fd, offset, SEEK_SET);
    ssize_t bytesRead = read(filesize->partition->vdi->fd, superblock, 1024);

    if (bytesRead != 1024) {
        printf("Error: Failed to read superblock (read %ld bytes)\n", bytesRead);
        return false;
    }

    printf("Debug: Successfully read superblock at offset %ld\n", offset);
    return true;
}

bool writeSuperblock(Ext2File *filesize, uint32_t blockNum, void *superblock) {
    return writeBlock(filesize, filesize->firstDataBlock + blockNum, superblock);
}

bool fetchBGDT(Ext2File *filesize, uint32_t blockNum, void *bgdt) {
    return fetchBlock(filesize, filesize->firstDataBlock + blockNum, bgdt);
}

bool writeBGDT(Ext2File *filesize, uint32_t blockNum, void *bgdt) {
    return writeBlock(filesize, filesize->firstDataBlock + blockNum, bgdt);
}

void displaySuperblock(Ext2File *filesize, void *superblock) {
    printf("Superblock Information:\n");
    printf("Total Inodes: %u\n", *(uint32_t *)(superblock + 0));
    printf("Total Blocks: %u\n", *(uint32_t *)(superblock + 4));
    printf("Block Size: %u\n", filesize->blockSize);
    printf("First Data Block: %u\n", *(uint32_t *)(superblock + 20));
    printf("Magic Number: 0x%X\n", *(uint16_t *)(superblock + 56));
}

void displayBGDT(Ext2File *filesize, void *bgdt) {
    printf("\nBlock Group Descriptor Table:\n");
    printf("Block Bitmap: %u\n", *(uint32_t *)(bgdt + 0));
    printf("Inode Bitmap: %u\n", *(uint32_t *)(bgdt + 4));
    printf("Inode Table: %u\n", *(uint32_t *)(bgdt + 8));
    printf("Free Blocks Count: %u\n", *(uint16_t *)(bgdt + 12));
    printf("Free Inodes Count: %u\n", *(uint16_t *)(bgdt + 14));
    printf("Used Directories Count: %u\n", *(uint16_t *)(bgdt + 16));
}
