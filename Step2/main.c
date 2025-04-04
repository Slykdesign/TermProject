#include "vdi.h"
#include "mbr.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <ctype.h>

#define BUFFER_SIZE 1024  // Adjust based on file size

// Function prototypes
void displayBufferPage(uint8_t *buf, uint32_t count, uint32_t skip, uint64_t offset);
void displayBuffer(uint8_t *buf, uint32_t count, uint64_t offset);
void displaySuperblock(MBRPartition *partition);

int main() {
    VDIFile *vdi = vdiOpen("./good-fixed-1k.vdi");
    if (!vdi) return 1;

    printf("Output from good-fixed-1k.vdi:\n");
    displayPartitionTable(vdi);

    MBRPartition *partition = mbrOpen(vdi, 0);
    if (!partition) {
        printf("Failed to open partition.\n");
        vdiClose(vdi);
        return 1;
    }

    char buffer[512];
    mbrSeek(partition, 0, SEEK_SET);
    mbrRead(partition, buffer, 512);
    printf("Read first sector of partition:\n%.*s\n", 512, buffer);

    displaySuperblock(partition);

    mbrClose(partition);
    vdiClose(vdi);
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

void displaySuperblock(MBRPartition *partition) {
    char buffer[BUFFER_SIZE];
    mbrSeek(partition, 1024, SEEK_SET);  // Seek to superblock offset
    mbrRead(partition, buffer, BUFFER_SIZE);
    printf("Superblock:\n");
    displayBuffer((uint8_t *)buffer, BUFFER_SIZE, 0);  // Display buffer with proper offset
}
