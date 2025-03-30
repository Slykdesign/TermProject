#include "ext2.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

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
    off_t offset = filesize->partition->start + 1024;  // Superblock is always at 1024-byte offset
    lseek(filesize->partition->vdi->fd, offset, SEEK_SET);
    ssize_t bytesWritten = write(filesize->partition->vdi->fd, superblock, 1024);

    if (bytesWritten != 1024) {
        printf("Error: Failed to write superblock (wrote %ld bytes)\n", bytesWritten);
        return false;
    }

    printf("Debug: Successfully wrote superblock at offset %ld\n", offset);
    return true;
}

bool fetchBGDT(Ext2File *filesize, uint32_t blockNum, void *bgdt) {
    uint32_t bgdtBlock = filesize->firstDataBlock + blockNum;
    return fetchBlock(filesize, bgdtBlock, bgdt);
}

bool writeBGDT(Ext2File *filesize, uint32_t blockNum, void *bgdt) {
    uint32_t bgdtBlock = filesize->firstDataBlock + blockNum;
    return writeBlock(filesize, bgdtBlock, bgdt);
}

void displaySuperblock(Ext2File *filesize, void *superblock) {
    printf("Superblock from block 0\n");
    printf("Superblock contents:\n");
    printf("Number of inodes: %u\n", *(uint32_t *)(superblock + 0));
    printf("Number of blocks: %u\n", *(uint32_t *)(superblock + 4));
    printf("Number of reserved blocks: %u\n", *(uint32_t *)(superblock + 8));
    printf("Number of free blocks: %u\n", *(uint32_t *)(superblock + 12));
    printf("Number of free inodes: %u\n", *(uint32_t *)(superblock + 16));
    printf("First data block: %u\n", *(uint32_t *)(superblock + 20));
    printf("Log block size: %u (%u)\n", *(uint32_t *)(superblock + 24), 1024 << *(uint32_t *)(superblock + 24));
    printf("Log fragment size: %u (%u)\n", *(uint32_t *)(superblock + 28), 1024 << *(uint32_t *)(superblock + 28));
    printf("Blocks per group: %u\n", *(uint32_t *)(superblock + 32));
    printf("Fragments per group: %u\n", *(uint32_t *)(superblock + 36));
    printf("Inodes per group: %u\n", *(uint32_t *)(superblock + 40));
    printf("Last mount time: %s", ctime((time_t *)(superblock + 44)));
    printf("Last write time: %s", ctime((time_t *)(superblock + 48)));
    printf("Mount count: %u\n", *(uint16_t *)(superblock + 52));
    printf("Max mount count: %u\n", *(uint16_t *)(superblock + 54));
    printf("Magic number: 0x%X\n", *(uint16_t *)(superblock + 56));
    printf("State: %u\n", *(uint16_t *)(superblock + 58));
    printf("Error processing: %u\n", *(uint16_t *)(superblock + 60));
    printf("Revision level: %u.%u\n", *(uint32_t *)(superblock + 62), *(uint32_t *)(superblock + 66));
    printf("Last system check: %s", ctime((time_t *)(superblock + 70)));
    printf("Check interval: %u\n", *(uint32_t *)(superblock + 74));
    printf("OS creator: %u\n", *(uint32_t *)(superblock + 78));
    printf("Default reserve UID: %u\n", *(uint16_t *)(superblock + 82));
    printf("Default reserve GID: %u\n", *(uint16_t *)(superblock + 84));
    printf("First inode number: %u\n", *(uint32_t *)(superblock + 88));
    printf("Inode size: %u\n", *(uint16_t *)(superblock + 90));
    printf("Block group number: %u\n", *(uint16_t *)(superblock + 92));
    printf("Feature compatibility bits: 0x%08X\n", *(uint32_t *)(superblock + 96));
    printf("Feature incompatibility bits: 0x%08X\n", *(uint32_t *)(superblock + 100));
    printf("Feature read/only compatibility bits: 0x%08X\n", *(uint32_t *)(superblock + 104));
    printf("UUID: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
        ((uint8_t *)superblock)[108], ((uint8_t *)superblock)[109], ((uint8_t *)superblock)[110], ((uint8_t *)superblock)[111],
        ((uint8_t *)superblock)[112], ((uint8_t *)superblock)[113], ((uint8_t *)superblock)[114], ((uint8_t *)superblock)[115],
        ((uint8_t *)superblock)[116], ((uint8_t *)superblock)[117], ((uint8_t *)superblock)[118], ((uint8_t *)superblock)[119],
        ((uint8_t *)superblock)[120], ((uint8_t *)superblock)[121], ((uint8_t *)superblock)[122], ((uint8_t *)superblock)[123]);
    printf("Volume name: [%.*s]\n", 16, (char *)(superblock + 124));
    printf("Last mount point: [%.*s]\n", 64, (char *)(superblock + 140));
    printf("Algorithm bitmap: 0x%08X\n", *(uint32_t *)(superblock + 204));
    printf("Number of blocks to preallocate: %u\n", *(uint8_t *)(superblock + 208));
    printf("Number of blocks to preallocate for directories: %u\n", *(uint8_t *)(superblock + 209));
    printf("Journal UUID: %02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
        ((uint8_t *)superblock)[212], ((uint8_t *)superblock)[213], ((uint8_t *)superblock)[214], ((uint8_t *)superblock)[215],
        ((uint8_t *)superblock)[216], ((uint8_t *)superblock)[217], ((uint8_t *)superblock)[218], ((uint8_t *)superblock)[219],
        ((uint8_t *)superblock)[220], ((uint8_t *)superblock)[221], ((uint8_t *)superblock)[222], ((uint8_t *)superblock)[223],
        ((uint8_t *)superblock)[224], ((uint8_t *)superblock)[225], ((uint8_t *)superblock)[226], ((uint8_t *)superblock)[227]);
    printf("Journal inode number: %u\n", *(uint32_t *)(superblock + 228));
    printf("Journal device number: %u\n", *(uint32_t *)(superblock + 232));
    printf("Journal last orphan inode number: %u\n", *(uint32_t *)(superblock + 236));
    printf("Default hash version: %u\n", *(uint8_t *)(superblock + 240));
    printf("Default mount option bitmap: 0x%08X\n", *(uint32_t *)(superblock + 241));
    printf("First meta block group: %u\n", *(uint32_t *)(superblock + 245));
}

void displayBGDT(Ext2File *filesize, void *bgdt) {
    printf("\nBlock group descriptor table:\n");
    printf("Block Block Inode Inode Free Free Used\n");
    printf("Number Bitmap Bitmap Table Blocks Inodes Dirs\n");

    for (uint32_t i = 0; i < filesize->numBlockGroups; i++) {
        uint32_t blockBitmap = *(uint32_t *)(bgdt + i * 32);
        uint32_t inodeBitmap = *(uint32_t *)(bgdt + i * 32 + 4);
        uint32_t inodeTable = *(uint32_t *)(bgdt + i * 32 + 8);
        uint16_t freeBlocks = *(uint16_t *)(bgdt + i * 32 + 12);
        uint16_t freeInodes = *(uint16_t *)(bgdt + i * 32 + 14);
        uint16_t usedDirs = *(uint16_t *)(bgdt + i * 32 + 16);

        printf("%6u %6u %6u %5u %6u %6u %4u\n",
            i,
            blockBitmap,
            inodeBitmap,
            inodeTable,
            freeBlocks,
            freeInodes,
            usedDirs);
    }
}

void ext2Close(Ext2File *filesize) {
    if (filesize) {
        mbrClose(filesize->partition);
        free(filesize);
    }
}
