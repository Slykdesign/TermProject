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

    Ext2File *fs = malloc(sizeof(Ext2File));
    if (!fs) return NULL;

    fs->partition = partition;

    // Read superblock
    uint8_t sb[1024];
    fetchSuperblock(fs, 0, sb);

    // Validate superblock
    uint16_t magic;
    memcpy(&magic, sb + 56, sizeof(uint16_t)); // s_magic field
    if (magic != EXT2_SUPERBLOCK_MAGIC) {
        printf("Invalid ext2 superblock.\n");
        ext2Close(fs);
        return NULL;
    }

    // Extract key filesystem parameters
    fs->blockSize = 1024 << *(uint32_t *)(sb + 24); // s_log_block_size
    fs->numBlockGroups = (*(uint32_t *)(sb + 4) + *(uint32_t *)(sb + 32) - 1) / *(uint32_t *)(sb + 32);
    fs->firstDataBlock = *(uint32_t *)(sb + 20);
    fs->totalInodes = *(uint32_t *)(sb + 0);
    fs->totalBlocks = *(uint32_t *)(sb + 4);

    return fs;
}

void ext2Close(Ext2File *fs) {
    if (fs) {
        mbrClose(fs->partition);
        free(fs);
    }
}