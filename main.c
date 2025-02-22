#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#define BUFFER_SIZE 256

// Function prototypes
void displayBufferPage(uint8_t *buf, uint32_t count, uint32_t skip, uint64_t offset);
void displayBuffer(uint8_t *buf, uint32_t count, uint64_t offset);

int main() {
    const char *filename = "testfile.bin";  // Sample file to read/write
    int fd;
    uint8_t buffer[BUFFER_SIZE];

    // Step 1: Open the file
    fd = open(filename, O_RDWR | O_CREAT, 0644);
    if (fd == -1) {
        perror("Error opening file");
        return 1;
    }

    // Step 2: Write some sample data
    memset(buffer, 0, BUFFER_SIZE);
    strcpy((char *)buffer, "This is a sample file content for testing.");

    if (write(fd, buffer, BUFFER_SIZE) == -1) {
        perror("Error writing to file");
        close(fd);
        return 1;
    }

    // Step 3: Reset cursor to the beginning
    lseek(fd, 0, SEEK_SET);

    // Step 4: Read the file into buffer
    memset(buffer, 0, BUFFER_SIZE);
    if (read(fd, buffer, BUFFER_SIZE) == -1) {
        perror("Error reading file");
        close(fd);
        return 1;
    }

    // Step 5: Display the buffer contents
    displayBuffer(buffer, BUFFER_SIZE, 0);

    // Step 6: Close the file
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