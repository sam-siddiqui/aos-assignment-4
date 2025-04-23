#include "cpmfsys.h"
#include <ctype.h>
#include <assert.h>

#define NAME_SIZE 9
#define EXT_SIZE 4 
#define REGION_SIZE 128

bool freeList[NUM_BLOCKS];
typedef enum FileMode{w = 10, r = 20, a = 30} fileMode;
typedef struct FCB {
    DirStructType *dirStruct; 
    fileMode mode;
    int readWriteIndex;
    int currBlockIndex;
    int dirExtentIndex;
    uint8_t *blockBuffer;
} FileControlBlock;

uint8_t *block0;

FileControlBlock** openFileTable = NULL;
int fileTableSize = 0;
int fileTableCapacity = 0;

void resizeOpenFileTable(int newCapacity) {
    if (!openFileTable) {
        openFileTable = calloc(newCapacity, sizeof(FileControlBlock*));
    } else {
        if(newCapacity <= fileTableCapacity) {
            perror("Cannot reduce openFileTable capacity!\n");
            return;
        }
        openFileTable = realloc(openFileTable, newCapacity);
    }
    fileTableCapacity = newCapacity;
}

void initOpenFileTable() {
    int initialCapacity = 10;
    fileTableSize = 0;
    resizeOpenFileTable(initialCapacity);
}

int fetchGlobalBlock0() {
    if(openFileTable == NULL) initOpenFileTable();
    if(block0 == NULL) block0 = malloc(BLOCK_SIZE);

    if (blockRead(block0, 0) != 0) {
        free(block0);
        return -4; // Error reading disk block
    }

    return 0;
}

int updateGlobalBlock0() {
   if(block0 == NULL) return -1;
   return blockWrite(block0, 0);
}

void addFileTableEntry(FileControlBlock *newEntry) {
    if (!openFileTable) initOpenFileTable();

    if(fileTableSize == fileTableCapacity) resizeOpenFileTable(fileTableCapacity * 2);

    openFileTable[fileTableSize++] = newEntry;
}

FileControlBlock* getFileEntry(size_t index) {
    if (index < fileTableSize) {
        return openFileTable[index];
    }
    return NULL;  // Invalid index
}

void updateFileEntry(size_t index, FileControlBlock* updatedEntry) {
    if (index > fileTableSize) return;
    free(openFileTable[index]->blockBuffer);
    free(openFileTable[index]);  // Free the memory allocated for this entry

    openFileTable[index] = updatedEntry;
}

void removeFileEntry(size_t index) {
    if (index > fileTableSize) return;
    free(openFileTable[index]->blockBuffer);
    free(openFileTable[index]);  // Free the memory allocated for this entry
    memmove(
        &openFileTable[index], 
        &openFileTable[index + 1],          // Move from i + 1, to i
        (fileTableSize - index - 1) * sizeof(FileControlBlock*)
    );                                      // TODO this still leaves a gap
    fileTableSize--;
}

int numFreeFileBlocks() {
    int numFreeFileBlocks = 0;
    for (int i = 0; i < sizeof(freeList); i++) {
        if (freeList[i]) numFreeFileBlocks++;
    }
    return numFreeFileBlocks;    
}

void updateFileBlockStatus(int index, bool value) {
    if(index < 0 && index > fileTableSize) return;

    freeList[index] = value;
}

void cleanupOpenFileTable() {
    for (size_t i = 0; i < fileTableSize; i++) {
        free(openFileTable[i]);  // Free memory for each FileControlBlock
    }
    free(openFileTable);  // Free the array itself
    openFileTable = NULL;
    fileTableSize = 0;
    fileTableCapacity = 0;
}

int firstNonZeroFileBlock(int extent) {
    fetchGlobalBlock0();
    
    int startIndex = extent * EXTENT_SIZE;
    for (int i = (startIndex + BLOCKS_PER_EXTENT); i < (startIndex + EXTENT_SIZE); i++) {
        if(block0[i] != 0) return block0[i];
    }
    return -1;
}

int firstFreeExtent() {
    fetchGlobalBlock0();
    for(int extentIndex=0; extentIndex < (int) (BLOCK_SIZE / EXTENT_SIZE); extentIndex++)
	{
		if(block0[extentIndex*EXTENT_SIZE] == 0xe5) {
			return extentIndex;
		}
	}
	return -1;
}

int* firstNFreeFileBlocks(int n) {
    int *arr = malloc(sizeof(int) * n);
    for (int i = 1, j = 0; i < NUM_BLOCKS; i++) {
        if(freeList[i]) {
            arr[j++] = i; 
            n--;
        }
        if (n < 1) break;
    }
    return arr;
}

int* extractNonZeroBlocksD(DirStructType *d, int count) {
    int *nonZeroBlocksArr = malloc(sizeof(int) * count);

    for (int i = 0, j = 0; i < BLOCKS_PER_EXTENT; i++) {
        if(d->blocks[i] > 0 && d->blocks[i] < NUM_BLOCKS) {
            nonZeroBlocksArr[j++] = d->blocks[i];
        }
    }

    return nonZeroBlocksArr;
}

int* extractNonZeroBlocksE(uint8_t *e, int count) {
    int *nonZeroBlocksArr = malloc(sizeof(int) * count);

    for (int i = (EXTENT_SIZE / 2), j = 0; i < BLOCKS_PER_EXTENT; i++) {
        if(e[i] > 0 && e[i] < NUM_BLOCKS) {
            nonZeroBlocksArr[j++] = e[i];
        }
    }

    return nonZeroBlocksArr;
}

int closestNextFreeFileBlock(int index) {
    for(int i=0; i < NUM_BLOCKS; i++) {
		if(freeList[i])	{
			updateFileBlockStatus(i, false);
			return i;
		}
	}
    return -1;
}

int findCharIndex(char* s, char what, int maxLen) {
    for (int i = 0; i < maxLen; i++) {
        if (s[i] == what) {
            return i;     // Return the index 
        }
    }
    return -1;  // Return -1 if the character was not found
}

int replaceChar(char* s, char what, char with, int maxLen) {
    int index = findCharIndex(s, what, maxLen);
    if (index > -1) s[index] = with;
    return index;
}

int cutOnChar(char* s, char c, int maxLen) {
    int index = replaceChar(s, c, '\0', maxLen);  // Use replaceChar to replace the character with '\0'
    return (index != -1 && index < maxLen) ? index : maxLen - 1;        // If the char was found and it was cut, return newLen, else orignalLen
}

int numBlocksUsedInExtent(Extent extent) {
    int numBlocks = 0;
    for (int i = (EXTENT_SIZE / 2); i < EXTENT_SIZE; i++) {
        if (extent[i] != 0 && extent[i] > 0 && extent[i] < NUM_BLOCKS) numBlocks++;
    }
    return numBlocks;
}

int numBlocksUsedInDir(DirStructType* d) {
    int numBlocks = 0;
    for (int i = 0; i < (EXTENT_SIZE / 2); i++) {
        if (d->blocks[i] != 0 && d->blocks[i] > 0 && d->blocks[i] < NUM_BLOCKS) numBlocks++;
    }
    return numBlocks;
}

int numFileBlocksUsed(int extentIndex) {
    fetchGlobalBlock0();
    int numBlocks = 0;
    
    for (int i = (EXTENT_SIZE / 2); i < EXTENT_SIZE; i++) {
        if (block0[(extentIndex * EXTENT_SIZE) + i] != 0 
        && block0[(extentIndex * EXTENT_SIZE) + i] > 0 
        && block0[(extentIndex * EXTENT_SIZE) + i] < NUM_BLOCKS) numBlocks++;
    }
    return numBlocks;
}

void populateDirStruct(DirStructType *d, uint8_t index, uint8_t* e) {
    if (index < 0 || index >= (BLOCK_SIZE/EXTENT_SIZE) || !e || !d) return;

    // Calculate starting position of the extent
    int startingIndex = index * EXTENT_SIZE;
    uint8_t *ptr = e + startingIndex;

    d->status = *ptr++;                               // dirStructPtr = 0 + 1 = 1
    strncpy(d->name, (char*)ptr, NAME_SIZE - 1);
    d->name[NAME_SIZE - 1] = '\0';                    // Ensure null termination
    cutOnChar(d->name, ' ', NAME_SIZE);
    ptr += NAME_SIZE - 1;                                       // dirStructPtr = 8 + 1 = 9
    strncpy(d->extension, (char*)ptr, EXT_SIZE - 1);
    d->extension[EXT_SIZE - 1] = '\0';                // Ensure null termination
    cutOnChar(d->extension, ' ', EXT_SIZE);
    ptr += EXT_SIZE - 1;                                        // dirStructPtr = 9 + 3 = 12
    
    d->XL = *ptr++;
    d->BC = *ptr++;
    d->XH = *ptr++;
    d->RC = *ptr++;
    //dirStructPtr = 13 + 4 = 17

    // Copy block indices
    memcpy(d->blocks, ptr, BLOCKS_PER_EXTENT);

}

DirStructType* initDirStruct(char* fileName, uint8_t index, uint8_t *e) {
    if (index < 0 || index >= (BLOCK_SIZE/EXTENT_SIZE) || !e) return NULL;

    if (e[EXTENT_SIZE * index] != 0xe5) return NULL;
    
    DirStructType* d = malloc(sizeof(DirStructType));
    
    char *dotPointer = strchr(fileName, '.');
    int dotIndex = (int) (dotPointer - fileName);

    d->BC = 0;
    d->RC = 0;
    d->XH = (index >> 5) & 0x3F;
    d->XL = index & 0x1F;
    d->status = 0x01;

    for (int i = EXTENT_SIZE * index, j = 0; i < EXTENT_SIZE; i++) {
        e[i] = 0;
        d->blocks[j++] = 0;
    }
    
    if (dotPointer) {
        int nameLength = strlen(fileName);
        fileName += dotIndex + 1;
        memset(d->extension, ' ', EXT_SIZE - 1);
        strncpy(d->extension, fileName, nameLength - dotIndex - 1);
        fileName -= dotIndex + 1;
    }

    memset(d->name, ' ', NAME_SIZE - 1);
    strncpy(d->name, fileName, dotIndex);

    return d;
}

DirStructType* mkDirStruct(int index, uint8_t* e) {
    if (index < 0 || index >= (BLOCK_SIZE/EXTENT_SIZE) || !e) return NULL;
    
    DirStructType* fileInfoPtr = malloc(sizeof(DirStructType));
    if (!fileInfoPtr) return NULL; // Handle memory allocation failure

    populateDirStruct(fileInfoPtr, index, e);

    return fileInfoPtr;
} 

void writeDirStruct(DirStructType* d, uint8_t index, uint8_t* e) {
    if (index < 0 || index >= (BLOCK_SIZE/EXTENT_SIZE) || !e || !d) return;
    int startingIndex = index * EXTENT_SIZE;

    // Calculate starting position of the extent
    uint8_t *ptr = e + startingIndex;

    *ptr++ = d->status;                                         // dirStructPtr = 0 + 1 = 1

    memset(ptr, ' ', NAME_SIZE - 1);                            // Fill with blanks first
    strncpy(ptr, d->name, strlen(d->name));
    ptr += NAME_SIZE - 1;                                       // dirStructPtr = 8 + 1 = 9
    
    memset(ptr, ' ', EXT_SIZE - 1);                             // Fill with blanks first
    strncpy(ptr, d->extension, strlen(d->extension));
    ptr += EXT_SIZE - 1;                                        // dirStructPtr = 9 + 3 = 12

    *ptr++ = d->XL;
    *ptr++ = d->BC;
    *ptr++ = d->XH;
    *ptr++ = d->RC;
    //dirStructPtr = 13 + 4 = 17

    // Copy block indices
    memcpy(ptr, d->blocks, BLOCKS_PER_EXTENT);
}

void makeFreeList() {
    fetchGlobalBlock0();
    DirStructType* d;
    for (int i = 0; i < NUM_BLOCKS; i++) {
        freeList[i] = true;
    }
    for(int i = 0; i < (BLOCK_SIZE / EXTENT_SIZE); i++) {
        d = mkDirStruct(i, block0);
        if(d->status != 0xe5) {
            for(int j = 0; j < 16; j++) {
                freeList[(int) d->blocks[j]] = false;
            }
        }
    }

    freeList[0] = false;
}

void printFreeList() {
    printf("FREE BLOCK LIST: (* means in-use) \n");
    int rowSize = EXTENT_SIZE / 2;
    for (int i = 0; i < rowSize; i++) {
        printf("%3x: ", i * rowSize);
        for (int j = 0; j < rowSize; j++) printf(freeList[i*16+j] ? ". " : "* ");
        printf("\n");
    }
}

int findExtentWithName(char* name, uint8_t* block0) {
    if (!checkLegalName(name)) return -1;

    int extentNum = 0;
    uint8_t* ptr;
    char fileName[NAME_SIZE], fileExt[EXT_SIZE], completeName[NAME_SIZE + EXT_SIZE + 1];
    while(extentNum < (BLOCK_SIZE/EXTENT_SIZE)) {
        int startingIndex = extentNum * EXTENT_SIZE;
        if (block0[startingIndex] == 0xe5) {
            extentNum++;
            continue;                                               // Skip unused extents
        }

        // Read file name (8 bytes, padded with spaces)
        ptr = block0 + startingIndex + 1;                           // Starting index = Extent_Start + 1
        
        memset(fileName, 0, NAME_SIZE);                             // Initialize buffer
        strncpy(fileName, (char*)ptr, NAME_SIZE - 1);
        fileName[NAME_SIZE - 1] = '\0';
        cutOnChar(fileName, ' ', NAME_SIZE); 
        ptr += NAME_SIZE - 1;

        memset(fileExt, 0, EXT_SIZE);
        strncpy(fileExt, (char*)ptr, EXT_SIZE - 1);
        fileExt[EXT_SIZE - 1] = '\0';

        cutOnChar(fileExt, ' ', EXT_SIZE);

        snprintf(completeName, sizeof(completeName), "%s.%s", fileName, fileExt);

        if(strcmp(completeName, name) == 0) 
            return extentNum;

        extentNum++;
    }

    return -1;
}

bool checkLegalName(char* name) { 
    // Validate input
    if (!name || strlen(name) == 0) return false;

    // Split the filename and extension
    char *dotPtr = strchr(name, '.');
    
    // Pre-check for filename length
    if (!dotPtr) {
        // No extension present; entire name is the filename
        if (strlen(name) > NAME_SIZE - 1) return false;
    } else {
        // Dot exists; check filename length before the dot
        if ((int)(dotPtr - name) > NAME_SIZE - 1) return false;

        // Pre-check for extension length
        if (strlen(name) - (int)(dotPtr - name) - 1 > EXT_SIZE - 1) return false;
    }

    // Split the filename and extension
    char fileName[NAME_SIZE] = {0};       // Buffer for filename
    char fileExtension[EXT_SIZE] = {0};  // Buffer for extension

    if (dotPtr) {
        // Extract filename (up to NAME_SIZE) and extension (up to EXT_SIZE)
        strncpy(fileName, name, dotPtr - name);
        strncpy(fileExtension, dotPtr + 1, EXT_SIZE);
    } else {
        // No extension, entire name is the filename
        strncpy(fileName, name, NAME_SIZE);
    }

    #define validCharCheck(char) ((char >= '0' && char <= '9') || \
                                 (char >= 'A' && char <= 'Z') || \
                                 (char >= 'a' && char <= 'z')) 

    // Ensure filename contains only valid characters
    for (int i = 0; i < strlen(fileName); i++) {
        if (!validCharCheck(fileName[i])) 
          return false; // Only A-Z, a-z, 0-9 allowed
    }

    // Ensure extension contains only valid characters
    for (int i = 0; i < strlen(fileExtension); i++) {
        if (!validCharCheck(fileExtension[i])) 
          return false; // Only A-Z, a-z, 0-9 allowed
    }

    return true; // All checks passed
}

void printColorized(uint8_t *extent, int index) {
    #define COLOR_RED     "\033[31m"
    #define COLOR_BLUE    "\033[34m"
    #define COLOR_YELLOW  "\033[33m"
    #define COLOR_ORANGE  "\033[38;5;208m" // Using 256-color orange
    #define COLOR_GREEN   "\033[32m"
    #define COLOR_WHITE   "\033[37m"
    #define COLOR_RESET   "\033[0m"
    if (extent[0] == 0xe5) {
        printf(COLOR_RED);
        for (int i = 0; i < EXTENT_SIZE; i++) {
            printf("%02x ", extent[i]);
            if (i == 15) {printf("\n"); printf(COLOR_RESET "%04x: " COLOR_RED, index + 16);}
        }
        printf(COLOR_RESET "\n");
    } else {
        printf(COLOR_BLUE "%02x " COLOR_RESET, extent[0]); // First byte
        printf(COLOR_YELLOW);
        for (int i = 1; i <= 8; i++) { // Next 8 bytes
            printf("%02x ", extent[i]);
        }
        printf(COLOR_RESET COLOR_ORANGE);
        for (int i = 9; i <= 11; i++) { // Next 3 bytes
            printf("%02x ", extent[i]);
        }
        printf(COLOR_RESET COLOR_GREEN);
        for (int i = 12; i <= 15; i++) { // Next 4 bytes
            printf("%02x ", extent[i]);
        }
        printf("\n"); printf(COLOR_RESET "%04x: " COLOR_WHITE, index + 16);
        printf(COLOR_RESET COLOR_WHITE);
        for (int i = 16; i < EXTENT_SIZE; i++) { // Last 16 bytes
            printf("%02x ", extent[i]);
        }
        printf(COLOR_RESET "\n");
    }
}

void cpmDir() {
    fetchGlobalBlock0();
    const int raw = 1;
    const int rowSize = EXTENT_SIZE / 2;
    
    if (raw == 0) {
        printf("      ");
        for (int i = 0; i < rowSize; i++){
            printf("%02x ", i);
        };
        printf("\n      ");
        for (int i = 0; i < rowSize; i++){
            printf("-- ");
        };
        printf("\n");
        for (int i = 0; i < BLOCK_SIZE; i += EXTENT_SIZE) {
            if (i % rowSize == 0) {
                fprintf(stdout, "%04x: ", i);
            }
            printColorized(&block0[i], i);
            if (i % rowSize == rowSize - 1) {
                printf(i % EXTENT_SIZE == EXTENT_SIZE - 1 ? "\n\n" : "\n");
            }
        }
        fprintf(stdout, "\n");
    } else {
        int filesRead = 0;
        int numBlocks, fileSize;
        printf("DIRECTORY LISTING\n");
        for (int i = 0; i < BLOCK_SIZE; i+=EXTENT_SIZE) {
            DirStructType* fileInfoPtr = mkDirStruct(filesRead, block0);
            if(fileInfoPtr->status != 0xe5) {
                numBlocks = 0;
                for (int i = 0; i < BLOCKS_PER_EXTENT; i++ ) if(fileInfoPtr->blocks[i] != 0) numBlocks++;
                fileSize = ((numBlocks - 1) * BLOCK_SIZE) + (fileInfoPtr->RC * REGION_SIZE) + fileInfoPtr->BC;
                assert(fileInfoPtr->BC <= REGION_SIZE && "BC should at max 128");
                assert(fileInfoPtr->RC <= (int) (BLOCK_SIZE / REGION_SIZE) && "RC should be at max 8");
                assert((fileInfoPtr->RC != 0 || fileInfoPtr->BC != 0) && "Both BC and RC can't be 0!");
                printf("%s.%s %d\n", fileInfoPtr->name, fileInfoPtr->extension, fileSize);
            }
            filesRead++;
        }
    }
}

int cpmRename(char* oldName, char* newName) { 
    // Validate names
    if (!checkLegalName(newName) || !checkLegalName(oldName)) 
        return -2;

    // Read directory block
    if (fetchGlobalBlock0() != 0) return -4; // Error reading disk block

    // Check if newName already exists
    if (findExtentWithName(newName, block0) != -1) return -3; // Destination filename exists

    // Find the extent with oldName
    int extentNum = findExtentWithName(oldName, block0);
    if (extentNum == -1) return -1; // Source file not found
    

    // Locate extent and verify oldName
    uint8_t *ptr = block0 + (extentNum * EXTENT_SIZE) + 1; // Name starts at +1
    // Check to see if we got the correct Extent with the oldName TODO
    // if (strncmp((char*)ptr, oldName, splitAt) != 0) 
    //     return -1;
    
    memset(ptr, ' ', NAME_SIZE + EXT_SIZE - 2); // Fill with spaces first
    
    // Update name with newName (space-padded to NAME_SIZE)
    int splitAt = findCharIndex(newName, '.', NAME_SIZE + EXT_SIZE - 1);
    strncpy((char*)ptr, newName, splitAt);
    ptr += NAME_SIZE - 1;
    
    newName += splitAt + 1;

    strncpy((char*)ptr, newName, findCharIndex(newName, '\0', EXT_SIZE));

    // Write updated block back to disk
    if (updateGlobalBlock0()) return -4;

    return 0;
}

int cpmDelete(char* name) { 
    // Validate names
    if (!checkLegalName(name)) 
        return -2;

    // Find the extent with (file)name
    int extentNum = findExtentWithName(name, block0);
    if (extentNum == -1) 
        return -1; // Source file not found

#ifdef MORE_SECURE
    uint8_t dummyBlock[BLOCK_SIZE];
    memset(dummyBlock, 0, BLOCK_SIZE);
#endif
    uint8_t *blocksPtr = block0 + (extentNum * EXTENT_SIZE) + (EXTENT_SIZE - BLOCKS_PER_EXTENT);
    int blockNum;
    for (int i = 0; i < BLOCKS_PER_EXTENT; i++) {
        if (*blocksPtr != 0) {
            blockNum = atoi((char*)blocksPtr);
            updateFileBlockStatus(blockNum, true);          // Mark as free
            #ifdef MORE_SECURE
                blockWrite(dummyBlock, blockNum);
            #endif
        }
        blocksPtr++;
    }


    // Mark extent as unused
    uint8_t *extentPtr = block0 + (extentNum * EXTENT_SIZE) + 0;
    *extentPtr = 0xe5;
    extentPtr++;

    //Clear out the entire extent
    for(int i = 1; i < EXTENT_SIZE; i++) *extentPtr++ = (uint8_t) 0 ;

    if (updateGlobalBlock0() != 0) return -4;

    return 0;
}

int cpmCopy(char* oldName, char* newName) { 
    if(!checkLegalName(oldName) || !checkLegalName(newName)) return -1;

    fetchGlobalBlock0();

    int oldFileExtentNum = findExtentWithName(oldName, block0);
    if (oldFileExtentNum == -1) return -1;

    if(findExtentWithName(newName, block0) != -1) return -3;

    int numFileBlocksNeeded = numFileBlocksUsed(oldFileExtentNum);
    if(numFileBlocksNeeded > numFreeFileBlocks()) return -4;

    int firstFreeExtentIndex = firstFreeExtent();
    if (firstFreeExtentIndex == -1) return -5;

    DirStructType *newDirStruct = malloc(sizeof(DirStructType));
    populateDirStruct(newDirStruct, oldFileExtentNum, block0);
    // Update the extent bits
    newDirStruct->XH = (firstFreeExtentIndex >> 5) & 0x3F;
    newDirStruct->XL = firstFreeExtentIndex & 0x1F;

    uint8_t *tempTransferBlock = malloc(BLOCK_SIZE);

    int *newFreeBlocks = firstNFreeFileBlocks(numFileBlocksNeeded);
    for (int i = 0, j = 0; i < BLOCKS_PER_EXTENT; i++) {
        
        if (i < numFileBlocksNeeded) {
            if (newDirStruct->blocks[i] != 0) {
                updateFileBlockStatus(newFreeBlocks[i], false);
                blockRead(tempTransferBlock, newDirStruct->blocks[i]);
                newDirStruct->blocks[i] = newFreeBlocks[i];
                blockWrite(tempTransferBlock, newDirStruct->blocks[i]);
                continue;
            }
        }

        // Zero out the rest of the file blocks if any
        newDirStruct->blocks[i] = 0;
    }
    char *dotPointer = strchr(newName, '.');
    int dotIndex = (int) (dotPointer - newName);
    if (dotPointer) {
        int nameLength = strlen(newName);
        newName += dotIndex + 1;
        memset(newDirStruct->extension, ' ', EXT_SIZE - 1);
        strncpy(newDirStruct->extension, newName, nameLength - dotIndex - 1);
        newName -= dotIndex + 1;
    }
    memset(newDirStruct->name, ' ', NAME_SIZE - 1);
    strncpy(newDirStruct->name, newName, dotIndex);
    writeDirStruct(newDirStruct, firstFreeExtentIndex, block0);
    // Write updated block back to disk
    if (updateGlobalBlock0() != 0) return -4;
    free(newDirStruct);
    free(tempTransferBlock);
    return 0;
}

int cpmOpen(char* fileName, char mode) {
    if(!checkLegalName(fileName)) return -2;

    if(mode != 'r' && mode != 'w') return -6;

    fetchGlobalBlock0();

    int fileExtentIndex = findExtentWithName(fileName, block0);

    if((mode == 'r') && (fileExtentIndex == -1)) return -1;

    FileControlBlock *entry;
    for (int i = 0; i < fileTableSize; i++) {
        entry = getFileEntry(i);
        if(entry != NULL && entry->dirExtentIndex == fileExtentIndex) return -7;    // It's already open!
    }
    
    if (mode == 'w' && numFreeFileBlocks() == 0) return -5;                             // No free blocks

    for (int i = 0; i < fileTableCapacity; i++) {
        entry = getFileEntry(i);
        if (entry != NULL) continue;

        entry = malloc(sizeof(FileControlBlock));
        if(!entry) return -13;

        int firstNonZeroBlock = firstNonZeroFileBlock(fileExtentIndex);
        entry->blockBuffer = malloc(BLOCK_SIZE);

        if(mode == 'r' && blockRead(entry->blockBuffer, firstNonZeroBlock) != 0) 
            return -6;
        else {
            if(blockWrite(entry->blockBuffer, firstNonZeroBlock) != 0) 
                return -6;
        }

        if (fileExtentIndex != -1) {
            entry->currBlockIndex = firstNonZeroBlock;
            entry->dirExtentIndex = fileExtentIndex;
            entry->mode = mode;
            entry->dirStruct = mkDirStruct(fileExtentIndex, block0);
            if(!entry->dirStruct) return -6;
            entry->readWriteIndex = 0;
            addFileTableEntry(entry);
            return i;

        } else if(fileExtentIndex == -1) {

            int toAssignExtent = firstFreeExtent();

            if (toAssignExtent == -1) return -8;

            fetchGlobalBlock0();

            DirStructType *newDirStruct = initDirStruct(fileName, toAssignExtent, block0);
            if (!newDirStruct) return -6;
            block0[toAssignExtent * EXTENT_SIZE] = 0x01;

            entry->dirExtentIndex = toAssignExtent;
            entry->readWriteIndex = 0;
            entry->mode = mode;
            entry->dirStruct = newDirStruct;
            writeDirStruct(newDirStruct, toAssignExtent, block0);
            entry->currBlockIndex = firstNonZeroFileBlock(toAssignExtent);
            addFileTableEntry(entry);
            return i;
        }
    }

    // No slot in openFileTable was empty
    return -13;
}

int cpmClose(int filePointer) { 
    if (filePointer < 0 || filePointer >= fileTableSize) return -9;

    if (openFileTable[filePointer] != NULL) removeFileEntry(filePointer);

    return 0; 
}

int cpmRead(int pointer, uint8_t* buffer, int size) { 
    FileControlBlock *fcb = getFileEntry(pointer);

    if(fcb == NULL) return -8;
    if(fcb != NULL && fcb->mode != 'r') return -6;                  // Wrong mode
    if(size > BLOCK_SIZE) return -11;                                     // Buffer to big to read

    int nonZeroFileBlocksCount = numFileBlocksUsed(fcb->dirExtentIndex);

    int fileSize = (nonZeroFileBlocksCount - 1) * BLOCK_SIZE;
    fileSize += (fcb->dirStruct->BC) * REGION_SIZE + (fcb->dirStruct->RC);

    if(size > (fileSize - fcb->readWriteIndex)) return -9;

    uint8_t *fileBlockBuffer = malloc(BLOCK_SIZE);
    int *nonZeroBlocksArr = extractNonZeroBlocksD(fcb->dirStruct, nonZeroFileBlocksCount);
    int currReadWriteBlockNum = (int) fcb->readWriteIndex / BLOCK_SIZE;
    if(blockRead(fileBlockBuffer, nonZeroBlocksArr[currReadWriteBlockNum]) != 0) return -1;

    int readIndex = 0;
    int currReadWriteIndex = (int) fcb->readWriteIndex % BLOCK_SIZE;
    while(readIndex < size) {
        if(currReadWriteIndex == BLOCK_SIZE) {
            // Reading crossed file blocks, reset the counter from beginning of new block
            currReadWriteBlockNum++;
            if(blockRead(fileBlockBuffer, nonZeroBlocksArr[currReadWriteBlockNum]) != 0) return -1;     // Reset the toRead buffer
            currReadWriteIndex = 0;                                                                     // Reset the count
            continue;                                                                                   // Restart again
        } 
        
        
        buffer[readIndex++]=fileBlockBuffer[currReadWriteIndex++];                  // We're filing up the buffer variable
        fcb->readWriteIndex++;
        
    }

    free(fileBlockBuffer);
    free(nonZeroBlocksArr);
    return 0;
}

int cpmWrite(int pointer, uint8_t* buffer, int size) { 
    FileControlBlock *fcb = getFileEntry(pointer);

    if(fcb == NULL) return -8;
    if(fcb != NULL && fcb->mode != 'w') return -6;          // Wrong Mode

    int nonZeroFileBlocksCount = numFileBlocksUsed(fcb->dirExtentIndex);
    int fileSize = (nonZeroFileBlocksCount - 1) * BLOCK_SIZE;
    fileSize += (fcb->dirStruct->BC) * REGION_SIZE + (fcb->dirStruct->RC);

    if (size > ((BLOCKS_PER_EXTENT * BLOCK_SIZE) - fileSize)) return -10;

    int availableDiskSpace = numFreeFileBlocks() * BLOCK_SIZE;

    if (size > availableDiskSpace) return -4;

    if(size > BLOCK_SIZE) return -11;                                     // Buffer to big to read

    int firstEmptyBlockIndex;
    for (firstEmptyBlockIndex = -1; firstEmptyBlockIndex < BLOCKS_PER_EXTENT;) 
        if (fcb->dirStruct->blocks[++firstEmptyBlockIndex] == 0) break;

    uint8_t *tempWriteBuffer = malloc(BLOCK_SIZE);
    if(!tempWriteBuffer) return -1;

    int currFileBlockNum;
    if(firstEmptyBlockIndex == 0 && fcb->dirStruct->blocks[firstEmptyBlockIndex] == 0) {
        int *freeBlocksArr;
        freeBlocksArr = firstNFreeFileBlocks(1);
        if(!freeBlocksArr) return -1;
        currFileBlockNum = freeBlocksArr[0];
        fcb->dirStruct->blocks[0] = currFileBlockNum;
        if(blockWrite(tempWriteBuffer, currFileBlockNum) != 0) return -1;
    } else {
        currFileBlockNum = fcb->dirStruct->blocks[firstEmptyBlockIndex];
        if(blockWrite(tempWriteBuffer, currFileBlockNum) != 0) return -1;
    }

    int blockWriteBufferIndex = 0;
    int writeIndex = 0;
    int currReadWriteIndex = (int) fileSize % BLOCK_SIZE;
    fcb->currBlockIndex = currFileBlockNum;

    while(writeIndex < size && currReadWriteIndex < BLOCK_SIZE) {
        if(currReadWriteIndex == BLOCK_SIZE) {
            if(blockWrite(tempWriteBuffer, currFileBlockNum) != 0) return -1;

            currReadWriteIndex = 0;                                                             // Reset the curr Write Index

            int nextClosestFreeFileBlockNum = closestNextFreeFileBlock(firstEmptyBlockIndex);   // Get next closest file block which is free
            currFileBlockNum = nextClosestFreeFileBlockNum;                                     // We're at the new empty File Block
            fcb->dirStruct->blocks[++firstEmptyBlockIndex] = nextClosestFreeFileBlockNum;       // Save that to the list of blocks   
            fcb->currBlockIndex = nextClosestFreeFileBlockNum;                                  // Save it to the FCB as well
            updateFileBlockStatus(nextClosestFreeFileBlockNum, false);                          // Update the freelist too!
            
            if(blockRead(tempWriteBuffer, currFileBlockNum) != 0) return -1;
        }
        
        
        tempWriteBuffer[currReadWriteIndex++] = buffer[blockWriteBufferIndex++];
        writeIndex++;
        fcb->readWriteIndex++;
    }
    
    // One final write of the last block
    fcb->dirStruct->RC += (int) size / REGION_SIZE;                            // Update the Region Count
    fcb->dirStruct->BC += (int) size % REGION_SIZE;                            // Update the Byte Count in the Last region
    updateFileBlockStatus(currFileBlockNum, false);
    if(blockRead(tempWriteBuffer, currFileBlockNum) != 0) return -1;

    // Update the Global Block 0
    fetchGlobalBlock0();
    writeDirStruct(fcb->dirStruct, fcb->dirExtentIndex, block0);
    // Write updated block back to disk
    if (updateGlobalBlock0() != 0) return -4;
    free(tempWriteBuffer);
    return 0;
}
