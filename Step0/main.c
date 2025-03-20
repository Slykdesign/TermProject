#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#define BUFFER_SIZE 1024  // Adjust based on file size

// Function prototypes
void displayBufferPage(uint8_t *buf, uint32_t count, uint32_t skip, uint64_t offset);
void displayBuffer(uint8_t *buf, uint32_t count, uint64_t offset);

int main() {
    const char *vdi = "./good-fixed-1k.vdi";
    uint8_t buffer[BUFFER_SIZE];

    // Step 1: Open the file
    int fd = open(vdi, O_RDONLY);
    if (access(vdi, F_OK) == -1) {
        perror("File does not exist");
        return 1;
    }

    // Step 2: Read the file into buffer
    ssize_t bytesRead = read(fd, buffer, BUFFER_SIZE);
    if (bytesRead == -1) {
        perror("Error reading file");
        close(fd);
        return 1;
    }

    // Step 3: Display the buffer contents
    displayBuffer(buffer, bytesRead, 0);

    // Step 4: Close the file
    close(fd);

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
