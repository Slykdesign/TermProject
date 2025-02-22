#ifndef VDI_H
#define VDI_H

#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    int fd;                // File descriptor
    uint8_t header[400];   // VDI header
    uint32_t *map;         // Translation map (allocated dynamically)
    size_t cursor;         // Cursor position for read/write
} VDIFile;

VDIFile vdiOpen(const char *fn);
void vdiClose(VDIFile *f);
ssize_t vdiRead(struct VDIFile *f,void *buf,size_t count);
ssize_t vdiWrite(struct VDIFile *f,void *buf,size_t count);
off_t vdiSeek(VDIFile *f,off_t offset,int anchor);

#endif