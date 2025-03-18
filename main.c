#include <stdio.h>
#include <stdint.h>
#include <ctype.h>
#include "vdi.h"
#include "mbr.h"

#define BUFFER_SIZE 256

// Function prototypes
void displayBufferPage(uint8_t *buf, uint32_t count, uint32_t skip, uint64_t offset);
void displayBuffer(uint8_t *buf, uint32_t count, uint64_t offset);
void displayVDIHeader(VDIFile *vdi);

int main() {
    VDIFile *vdi = vdiOpen("./good-dynamic-2k.vdi");
    if (!vdi) return 1;

    displayVDIHeader(vdi);

    MBRPartition *partition = mbrOpen(vdi, 0);
    if (!partition) {
        printf("Failed to open MBR\n");
        vdiClose(vdi);
        return 1;
    }

    char buffer[512];
    vdiSeek(vdi, 0, SEEK_SET);
    vdiRead(vdi, buffer, 512);
    printf("Read first sector:\n%.*s\n", 512, buffer);

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
