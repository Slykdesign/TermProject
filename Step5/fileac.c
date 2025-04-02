#include "file_access.h"
#include "ext2.h"
#include <stdlib.h>
#include <string.h>

// Helper function to fetch a block from a file
int32_t fetchBlockFromFile(Ext2Inode *inode, uint32_t bNum, void *buf) {
    uint32_t k = 1024 / 4; // Assuming block size is 1024 bytes
    uint32_t *blockList;
    uint8_t tempBuf[1024];

    if (bNum < 12) {
        // Direct block
        blockList = inode->i_block;
        goto direct;
    } else if (bNum < 12 + k) {
        // Single indirect block
        if (inode->i_block[12] == 0) return -1;
        fetchBlock(inode->i_block[12], tempBuf);
        blockList = (uint32_t *)tempBuf;
        bNum -= 12;
        goto direct;
    } else if (bNum < 12 + k + k * k) {
        // Double indirect block
        if (inode->i_block[13] == 0) return -1;
        fetchBlock(inode->i_block[13], tempBuf);
        blockList = (uint32_t *)tempBuf;
        bNum -= 12 + k;
        goto single;
    } else {
        // Triple indirect block
        if (inode->i_block[14] == 0) return -1;
        fetchBlock(inode->i_block[14], tempBuf);
        blockList = (uint32_t *)tempBuf;
        bNum -= 12 + k + k * k;
    }

    // Triple indirect block handling
    while (bNum >= k * k) {
        if (blockList[bNum / (k * k)] == 0) return -1;
        fetchBlock(blockList[bNum / (k * k)], tempBuf);
        blockList = (uint32_t *)tempBuf;
        bNum %= k * k;
    }

single:
    // Single indirect block handling
    if (blockList[bNum / k] == 0) return -1;
    fetchBlock(blockList[bNum / k], tempBuf);
    blockList = (uint32_t *)tempBuf;
    bNum %= k;

direct:
    // Direct block handling
    if (blockList[bNum] == 0) return -1;
    return fetchBlock(blockList[bNum], buf);
}

// Helper function to write a block to a file
int32_t writeBlockToFile(Ext2Inode *inode, uint32_t bNum, void *buf) {
    uint32_t k = 1024 / 4; // Assuming block size is 1024 bytes
    uint32_t *blockList;
    uint8_t tempBuf[1024];
    uint8_t tempBuf2[1024];

    if (bNum < 12) {
        // Direct block
        blockList = inode->i_block;
        goto direct;
    } else if (bNum < 12 + k) {
        // Single indirect block
        if (inode->i_block[12] == 0) {
            inode->i_block[12] = allocateBlock();
            memset(tempBuf, 0, sizeof(tempBuf));
            writeBlock(inode->i_block[12], tempBuf);
        }
        fetchBlock(inode->i_block[12], tempBuf);
        blockList = (uint32_t *)tempBuf;
        bNum -= 12;
        goto direct;
    } else if (bNum < 12 + k + k * k) {
        // Double indirect block
        if (inode->i_block[13] == 0) {
            inode->i_block[13] = allocateBlock();
            memset(tempBuf, 0, sizeof(tempBuf));
            writeBlock(inode->i_block[13], tempBuf);
        }
        fetchBlock(inode->i_block[13], tempBuf);
        blockList = (uint32_t *)tempBuf;
        bNum -= 12 + k;
        goto single;
    } else {
        // Triple indirect block
        if (inode->i_block[14] == 0) {
            inode->i_block[14] = allocateBlock();
            memset(tempBuf, 0, sizeof(tempBuf));
            writeBlock(inode->i_block[14], tempBuf);
        }
        fetchBlock(inode->i_block[14], tempBuf);
        blockList = (uint32_t *)tempBuf;
        bNum -= 12 + k + k * k;
    }

    // Triple indirect block handling
    while (bNum >= k * k) {
        if (blockList[bNum / (k * k)] == 0) {
            blockList[bNum / (k * k)] = allocateBlock();
            memset(tempBuf2, 0, sizeof(tempBuf2));
            writeBlock(blockList[bNum / (k * k)], tempBuf2);
        }
        fetchBlock(blockList[bNum / (k * k)], tempBuf);
        blockList = (uint32_t *)tempBuf;
        bNum %= k * k;
    }

single:
    // Single indirect block handling
    if (blockList[bNum / k] == 0) {
        blockList[bNum / k] = allocateBlock();
        memset(tempBuf2, 0, sizeof(tempBuf2));
        writeBlock(blockList[bNum / k], tempBuf2);
    }
    fetchBlock(blockList[bNum / k], tempBuf);
    blockList = (uint32_t *)tempBuf;
    bNum %= k;

direct:
    // Direct block handling
    if (blockList[bNum] == 0) {
        blockList[bNum] = allocateBlock();
    }
    return writeBlock(blockList[bNum], buf);
}
