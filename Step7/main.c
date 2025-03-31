#include "vdi.h"
#include "mbr.h"
#include "ext2.h"
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char *argv[]) {
    VDIFile *vdi = vdiOpen("./good-fixed-1k.vdi");
    if (argc != 3) {
        printf("Usage: %s <VDI file> <file path>\n", argv[0]);
        return 1;
    }

    struct Ext2File fs;
    fs.fd = open(argv[1], O_RDONLY);
    if (fs.fd < 0) {
        perror("Error opening VDI file");
        return 1;
    }

    // Read superblock
    pread(fs.fd, &fs.sb, sizeof(fs.sb), 1024);
    pread(fs.fd, &fs.bg, sizeof(fs.bg), 2048);

    // Copy path since strtok modifies it
    char pathCopy[256];
    strncpy(pathCopy, argv[2], 255);
    pathCopy[255] = '\0';

    uint32_t inode = traversePath(&fs, pathCopy);
    if (inode == 0) {
        printf("File not found: %s\n", argv[2]);
    } else {
        printf("File inode: %u\n", inode);
    }
    close(fs.fd);
    return 0;
}
