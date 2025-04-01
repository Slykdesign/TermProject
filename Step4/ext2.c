#include "ext2.h"
#include <time.h>

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
