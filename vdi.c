#include "vdi.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void *vdiOpen(const char *filename) {
    VDIFile *vdi = malloc(sizeof(VDIFile));
    if (!vdi) return NULL;

    vdi->fd = open(filename, O_RDWR);
    if (vdi->fd == -1) {
        perror("Error opening VDI file");
        free(vdi);
        return NULL;
    }

    // Read VDI header
    if (read(vdi->fd, vdi->header, 1000) != 1000) {
        perror("Error reading VDI header");
        close(vdi->fd);
        free(vdi);
        return NULL;
    }

    // Extract relevant data
    vdi->pageSize = *(uint32_t *)(vdi->header + 36); // Example offset for page size
    vdi->totalPages = *(uint32_t *)(vdi->header + 40); // Example offset for total pages

    // Read translation map
    off_t mapOffset = *(off_t *)(vdi->header + 16); // Example offset for map
    lseek(vdi->fd, mapOffset, SEEK_SET);
    vdi->map = malloc(vdi->totalPages * sizeof(uint32_t));
    read(vdi->fd, vdi->map, vdi->totalPages * sizeof(uint32_t));

    vdi->cursor = 0;
    return vdi;
}

void vdiClose(VDIFile *vdi) {
    if (vdi) {
        close(vdi->fd);
        free(vdi->map);
        free(vdi);
    }
}

off_t vdiTranslate(VDIFile *vdi, off_t logicalOffset) {
    uint32_t pageNum = logicalOffset / vdi->pageSize;
    uint32_t offsetInPage = logicalOffset % vdi->pageSize;

    if (pageNum >= vdi->totalPages) {
        return -1;  // Out of range
    }

    uint32_t physicalPage = vdi->map[pageNum];

    if (physicalPage == 0xFFFFFFFF) {
        return -1; // Page not allocated, return error or zeroed buffer
    }

    return (off_t)(physicalPage * vdi->pageSize + offsetInPage);
}

ssize_t vdiRead(VDIFile *vdi, void *buf, size_t count) {
    size_t bytesRead = 0;
    uint8_t *buffer = buf;

    while (count > 0) {
        off_t physicalOffset = vdiTranslate(vdi, vdi->cursor);
        if (physicalOffset == -1) {
            memset(buffer, 0, count);  // Unallocated pages return zeroes
            return bytesRead;
        }

        size_t pageRemaining = vdi->pageSize - (vdi->cursor % vdi->pageSize);
        size_t toRead = (count < pageRemaining) ? count : pageRemaining;

        lseek(vdi->fd, physicalOffset, SEEK_SET);
        ssize_t result = read(vdi->fd, buffer, toRead);
        if (result <= 0) break;

        bytesRead += result;
        buffer += result;
        count -= result;
        vdi->cursor += result;
    }
    return bytesRead;
}

ssize_t vdiWrite(VDIFile *vdi, void *buf, size_t count) {
    size_t bytesWritten = 0;
    uint8_t *buffer = (uint8_t *)buf;

    while (count > 0) {
        off_t physicalOffset = vdiTranslate(vdi, vdi->cursor);
        if (physicalOffset == -1) {
            return bytesWritten;  // Writing to unallocated pages is not supported yet
        }

        size_t pageRemaining = vdi->pageSize - (vdi->cursor % vdi->pageSize);
        size_t toWrite = (count < pageRemaining) ? count : pageRemaining;

        lseek(vdi->fd, physicalOffset, SEEK_SET);
        ssize_t result = write(vdi->fd, buffer, toWrite);
        if (result <= 0) break;

        bytesWritten += result;
        buffer += result;
        count -= result;
        vdi->cursor += result;
    }
    return bytesWritten;
}

off_t vdiSeek(VDIFile *vdi, off_t offset, int anchor) {
    if (!vdi) return -1;

    off_t newCursor;
    if (anchor == SEEK_SET) {
        newCursor = offset;
    } else if (anchor == SEEK_CUR) {
        newCursor = vdi->cursor + offset;
    } else if (anchor == SEEK_END) {
        newCursor = 134217728 + offset;  // Adjust disk size accordingly
    } else {
        return -1;
    }

    if (newCursor < 0 || newCursor > 134217728) return -1;

    vdi->cursor = newCursor;
    return vdi->cursor;
}

void displayVDIHeader(VDIFile *vdi) {
    printf("VDI Header Info:\n");
    printf("Signature: 0x%x\n", *(uint32_t *)(vdi->header));
    printf("Version: %d.%d\n", vdi->header[4], vdi->header[5]);
    printf("Header Size: %d bytes\n", *(uint32_t *)(vdi->header + 12));
    printf("Disk Size: %llu bytes\n", *(uint64_t *)(vdi->header + 16));
}
