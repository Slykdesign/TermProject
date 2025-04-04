#include "ext2.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <ctype.h>

#define BUFFER_SIZE 1024  // Adjust based on file size

// Function prototypes
void displayBufferPage(uint8_t *buf, uint32_t count, uint32_t skip, uint64_t offset);
void displayBuffer(uint8_t *buf, uint32_t count, uint64_t offset);

int main() {
    VDIFile *vdi = vdiOpen("./good-fixed-1k.vdi");
    if (!vdi) return 1;

    Ext2File *filesize = ext2Open(vdi);
    if (!filesize) {
        vdiClose(vdi);
        return 1;
    }

    uint8_t superblock[1024];
    fetchSuperblock(filesize, 0, superblock);
    displaySuperblock(filesize, superblock);

    uint8_t bgdt[32];
    fetchBGDT(filesize, 1, bgdt);
    displayBGDT(filesize, bgdt);

    ext2Close(filesize);
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
