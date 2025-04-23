#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "cpmfsys.h" // Include CP/M file system definitions and utilities.
#include "diskSimulator.h" // Include disk simulation functions for block-level read/write and debugging.

// Function to print the contents of a buffer for debugging purposes.
void printBuffer(uint8_t buffer[], int size) {
    int i;
    fprintf(stdout, "\nBUFFER PRINT:\n");
    for (i = 0; i < size; i++) {
        if (i % 16 == 0) { // Print offset at the start of each line.
            fprintf(stdout, "%4x: ", i);
        }
        fprintf(stdout, "%2x ", buffer[i]); // Print buffer contents in hex format.
        if (i % 16 == 15) { // Break line after every 16 bytes.
            fprintf(stdout, "\n");
        }
    }
    fprintf(stdout, "\n");
}

int main(int argc, char* argv[]) {
    uint8_t buffer1[BLOCK_SIZE], buffer2[BLOCK_SIZE]; // Two buffers for disk operations.
    int i;

    // Read the disk image "image1.img" into memory.
    int blocksRead = (int) readImage("image1.img");

    assert(blocksRead == NUM_BLOCKS && "The number of blocks read don't match specification!\n");

    // Generate a free list of unused blocks in the CP/M file system.
    makeFreeList();

    // Print the directory contents of the CP/M file system.
    cpmDir();

    // Print the list of free blocks for debugging purposes.
    printFreeList();

    // Delete the file "shortf.ps" from the file system.
    cpmDelete("shortf.ps");

    // Display the directory contents again to reflect changes after deletion.
    cpmDir();

    // Rename the file "mytestf1.txt" to "mytest2.tx".
    cpmRename("mytestf1.txt", "mytest2.tx");

    // Test renaming another file and print the return code for the operation.
    fprintf(stdout,
            "cpmRename return code = %d,\n",
            cpmRename("mytestf.", "mytestv2.x"));

    // Display the directory contents again to reflect changes after renaming.
    cpmDir();

    // Print the updated list of free blocks for debugging purposes.
    printFreeList();

    int fp = cpmOpen("test.txt", 'w');

    assert(fp >= 0 && "File creation failed!\n");

    uint8_t *dataBuffer = malloc(16);
    for (int i = 0; i < 16; i++) dataBuffer[i] = i + 1;

    assert(cpmWrite(fp, dataBuffer, 16) >= 0 && "File write failed!\n");
    
    assert(cpmClose(fp) == 0 && "File closing failed!\n");

    cpmDir();
    printFreeList();
    cpmCopy("test.txt", "test2.txt");
    
    cpmDir();
    printFreeList();

    free(dataBuffer);

    return 0; // Return success code.
}
