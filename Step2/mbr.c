#include "mbr.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#define SECTOR_SIZE 512
#define MBR_OFFSET  0x1BE  // Start of partition table

typedef struct {
    uint8_t status;
    uint8_t chsFirst[3];
    uint8_t type;
    uint8_t chsLast[3];
    uint32_t lbaFirst;
    uint32_t lbaCount;
} __attribute__((packed)) PartitionEntry;

void displayPartitionTable(VDIFile *vdi) {
    PartitionEntry partitions[4];
    lseek(vdi->fd, MBR_OFFSET, SEEK_SET);
    read(vdi->fd, partitions, sizeof(partitions));

    printf("Partition Table:\n");
    for (int i = 0; i < 4; i++) {
        printf("Partition table entry %d:\n", i);
        printf("  Status: %s\n", (partitions[i].status == 0x80) ? "Active" : "Inactive");
        printf("  First sector CHS: %d-%d-%d\n", partitions[i].chsFirst[0], partitions[i].chsFirst[1], partitions[i].chsFirst[2]);
        printf("  Last sector CHS: %d-%d-%d\n", partitions[i].chsLast[0], partitions[i].chsLast[1], partitions[i].chsLast[2]);
        printf("  Partition type: %02x %s\n", partitions[i].type, (partitions[i].type == 0x83) ? "linux native" : "empty");
        printf("  First LBA sector: %u\n", partitions[i].lbaFirst);
        printf("  LBA sector count: %u\n", partitions[i].lbaCount);
    }
}

MBRPartition *mbrOpen(VDIFile *vdi, int part) {
    if (part < 0 || part > 3) return NULL;

    PartitionEntry partitions[4];
    vdiSeek(vdi->fd, MBR_OFFSET, SEEK_SET);
    vdiRead(vdi->fd, partitions, sizeof(partitions));

    printf("lba: %lu\n",&partitions[part].lbaCount);
    // Make your own mbr file
    // use vdiSeek and vdiRead instead of lseek and read in MBRPartition *mbrOpen

    if (partitions[part].lbaCount == 0) {
        printf("Partition %d is empty.\n", part);
        return NULL;
    }

    MBRPartition *mbr = malloc(sizeof(MBRPartition));
    if (!mbr) return NULL;

    mbr->vdi = vdi;
    mbr->start = partitions[part].lbaFirst * SECTOR_SIZE;
    mbr->size = partitions[part].lbaCount * SECTOR_SIZE;
    mbr->cursor = 0;

    return mbr;
}

void mbrClose(MBRPartition *mbr) {
    if (mbr) free(mbr);
}

ssize_t mbrRead(MBRPartition *mbr, void *buf, size_t count) {
    if (!mbr) return -1;

    if (mbr->cursor + count > mbr->size) {
        count = mbr->size - mbr->cursor; // Restrict to partition size
    }

    return vdiRead(mbr->vdi, buf, count);
}

ssize_t mbrWrite(MBRPartition *mbr, void *buf, size_t count) {
    if (!mbr) return -1;

    if (mbr->cursor + count > mbr->size) {
        count = mbr->size - mbr->cursor;
    }

    return vdiWrite(mbr->vdi, buf, count);
}

off_t mbrSeek(MBRPartition *mbr, off_t offset, int whence) {
    if (!mbr) return -1;

    off_t newCursor;
    if (whence == SEEK_SET) {
        newCursor = offset;
    } else if (whence == SEEK_CUR) {
        newCursor = mbr->cursor + offset;
    } else if (whence == SEEK_END) {
        newCursor = mbr->size + offset;
    } else {
        return -1;
    }

    if (newCursor < 0 || newCursor > mbr->size) return -1;

    mbr->cursor = newCursor;
    return newCursor;
}
