#include <stddef.h>
#include <stdint.h>

/** @brief Size of a single disk block in bytes. */
#define BLOCK_SIZE 1024

/** @brief Total number of blocks on the simulated disk. */
#define NUM_BLOCKS 256

/**
 * @brief Reads data from a specified disk block into the provided buffer.
 *
 * @param buffer Pointer to the buffer where data from the block will be stored.
 * @param blockNum Block number to read from. Must be between 0 and (NUM_BLOCKS - 1).
 * @return 0 for success, 1 for error.
 */
int blockRead(uint8_t *buffer, uint8_t blockNum);

/**
 * @brief Writes data from the provided buffer to a specified disk block.
 *
 * @param buffer Pointer to the buffer containing data to write to the block.
 * @param blockNum Block number to write to. Must be between 0 and (NUM_BLOCKS - 1).
 * @return 0 for success, 1 for error.
 */
int blockWrite(uint8_t *buffer, uint8_t blockNum);

/**
 * @brief Prints the contents of a specified disk block in a formatted view for debugging purposes.
 *
 * @param blockNum Block number to print. Must be between 0 and (NUM_BLOCKS - 1).
 */
void printBlock(uint8_t blockNum);

/**
 * @brief Writes the entire simulated disk image to a specified file, byte for byte from a Unix file.
 *
 * @param fileName Name of the file where the disk image will be saved.
 * @return The number of bytes written, or (size_t)-1 if an error occurs.
 */
size_t writeImage(char *fileName);

/**
 * @brief Reads the entire simulated disk image from a specified file.
 *
 * @param fileName Name of the file to read the disk image from.
 * @return The number of bytes read, or (size_t)-1 if an error occurs.
 */
size_t readImage(char *fileName);

// Error codes for disk operations
/** Operation completed successfully. */
#define SUCCESS 0

/** Error occurred during disk operation. */
#define ERROR 1
