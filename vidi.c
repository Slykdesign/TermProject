#include "vidi.h"
#include <string.h>
#include <fcntl.h>

VDIFile vdiOpen(const char *fn) {
    VDIFile *fn = malloc(sizeof(VDIFile));
    if (!fn) return NULL;

    fn->fd = open(fn, O_RDWR);
    if (fn->fd == -1) {
        perror("Error opening file!");
        free(fn);
        return NULL;
    }

    if (read(fn->fd, fn->header, 400) != 400) {
        perror("Error reading VIDI header!");
        close(fn->fd);
        free(fn);
        return NULL;
    }

    off_t mapOffset = *(off_t *)(fn->header + 16);

    lseek(fn->fd, mapOffset, SEEK_SET);
    fn->map = malloc(128 * sizeof(uint32_t));
    if (!fn->map) {
        perror("Error allocating map!");
        close(fn->fd);
        free(fn);
        return NULL;
    }
    fn->cursor = 0;
    read(fn->fd, fn->map, 128 * sizeof(uint32_t));
}

void vdiClose(struct VDIFile *f) {
    if (f) {
        close(f->fd);
        free(f->map);
        free(f);
    }
}

ssize_t vdiRead(struct VDIFile *f, void *buf, size_t count) {
    if (!f) return -1;

    lseek(f->fd, f->cursor, SEEK_SET);
    ssize_t bytesRead = read(f->fd, buf, count);

    if (bytesRead > 0) {
        f->cursor += bytesRead;
    }
    return bytesRead;
}

ssize_t vdiWrite(struct VDIFile *f, void *buf, size_t count) {
    if (!f) return -1;
    lseek(f->fd, f->cursor, SEEK_SET);
    ssize_t bytesWritten = read(f->fd, buf, count);

    if (bytesWritten > 0) {
        f->cursor += bytesWritten;
    }
    return bytesWritten;
}

off_t vdiSeek(VDIFile *f, off_t offset, int anchor) {
    if (!f) return -1;
}