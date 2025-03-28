#ifndef MBR_H
#define MBR_H

#include "vdi.h"

typedef struct {
    VDIFile *vdi;         // Pointer to the VDI file
    uint32_t start;       // Start of the partition (in bytes)
    uint32_t size;        // Size of the partition (in bytes)
    size_t cursor;        // Current cursor position
} MBRPartition;

MBRPartition *mbrOpen(VDIFile *vdi, int part);
void mbrClose(MBRPartition *mbr);
ssize_t mbrRead(MBRPartition *mbr, void *buf, size_t count);
ssize_t mbrWrite(MBRPartition *mbr, void *buf, size_t count);
off_t mbrSeek(MBRPartition *mbr, off_t offset, int whence);
void displayPartitionTable(VDIFile *vdi);

#endif