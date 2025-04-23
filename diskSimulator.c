#include "diskSimulator.h"

#include <stdint.h>
#include <stdio.h>

// first index is sector number, second index is byte in the block of bytes
static uint8_t disk[NUM_BLOCKS][BLOCK_SIZE];

int blockRead(uint8_t *buffer, uint8_t blockNum) {
    int i = 0;
    for (i = 0; i < 1024; i++) {
        *(buffer + i) = disk[(int)blockNum][(int)i];
    }
    return 0;
}

int blockWrite(uint8_t *buffer, uint8_t blockNum) {
    int i = 0;
    for (i = 0; i < 1024; i++) {
        disk[(int)blockNum][i] = *(buffer + i);
    }
    return 0;
}

void printBlock(uint8_t blockNum) {
    int i;
    
    fprintf(stdout, "\nDISK BLOCK %x:\n", blockNum);
    printf("      ");
    for (int i = 0; i < 16; i++){
        printf("%02x ", i);
    };
    printf("\n");
    for (i = 0; i < BLOCK_SIZE; i++) {
        if (i % 16 == 0) {
            fprintf(stdout, "%04x: ", i);
        }
        fprintf(stdout, "%02x ", disk[(int)blockNum][i]);
        if (i % 16 == 15) {
            fprintf(stdout, blockNum == 0 && i % 32 == 31 ? "\n\n" : "\n");
        }
    }
    fprintf(stdout, "\n");
}

size_t writeImage(char *fileName) {
    FILE *fp;
    size_t bytesWritten = -1;
    fp = fopen(fileName, "w");
    bytesWritten = fwrite(disk, BLOCK_SIZE, NUM_BLOCKS, fp);
    fclose(fp);
    return bytesWritten;
}

size_t readImage(char *fileName) {
    FILE *fp;
    size_t bytesRead = -1;
    fp = fopen(fileName, "r");
    // C uses row major order for multi dim arrays
    bytesRead = fread(disk, BLOCK_SIZE, NUM_BLOCKS, fp);
    fclose(fp);
    return bytesRead;
}
