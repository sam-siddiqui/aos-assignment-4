#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdint.h>
#include "../../diskSimulator.h"
#include "../../cpmfsys.h"

bool testFreeList[NUM_BLOCKS];
// Example buffer simulating Block 0 of the disk
uint8_t diskBlock0[BLOCK_SIZE] = {
    /**
     * '\x01', 'm', 'y', 't', 'e', 's', 't', 'f', '1', 't', 'x', 't', 
     * '\x01', '\x02', '\x03', '\x04',
     * '\xFF', '\x0E', '\xFD', '\x0C', '\xFB', '\x0A', '\xF9', '\x08', 
     * '\xF7', '\x06', '\xF5', '\x04', '\xF3', '\x02', '\xF1', ' '
     */
    0x01, 0x6D, 0x79, 0x74, 0x65, 0x73, 0x74, 0x66, 0x31, 0x74, 0x78, 0x74, 0x01, 0x02, 0x03, 0x04,
    0xFF, 0x0E, 0xFD, 0x0C, 0xFB, 0x0A, 0xF9, 0x08, 0xF7, 0x06, 0xF5, 0x04, 0xF3, 0x02, 0xF1, 0x20,

    /**
     * '\x01', 's', 'h', 'o', 'r', 't', 'f', ' ', ' ', 'p', 's', ' ', 
     * '\x01', '\x08', '\x03', '\x00', '0', '1', 
     * '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
     * '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'
     */
    0x01, 0x73, 0x68, 0x6F, 0x72, 0x74, 0x66, 0x20, 0x20, 0x70, 0x73, 0x20, 0x01, 0x08, 0x03, 0x00, 
    0x30, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//
void test_cpmDelete_valid_deletion() {

    // Attempt to delete the file
    blockRead(block0, 0);
    int extentNum = findExtentWithName("shortf.ps", block0);
    int result = cpmDelete("shortf.ps");
    CU_ASSERT(result == 0);
    
    // Verify extent 0 is marked as unused
    blockRead(block0, 0);
    CU_ASSERT(block0[extentNum * EXTENT_SIZE] == 0xe5);

    // Verify blocks 1, 2, 3, and 4 are marked as free
    // CU_ASSERT(testFreeList[1] == true);
}

void test_cpmDelete_invalid_filename() {
    // Attempt to delete a non-existent file
    int result = cpmDelete("nonexistent.");
    CU_ASSERT(result == -2);
}

int setup() {  
    blockWrite(diskBlock0, 0);
    if(refreshFileSystem() == 0) return 0;
    else return 1;
}

int cleanup() {
    cleanUpFileSystem(false);
    return 0;
}

int main() {
    // Initialize CUnit test registry
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    // Add a suite for mkDirStruct
    CU_pSuite suite = CU_add_suite("cpmDelete Suite", setup, cleanup);
    if (NULL == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Add tests to the suite
    if (
        (NULL == CU_add_test(suite, "test Success cpmDelete", test_cpmDelete_valid_deletion)) ||
        (NULL == CU_add_test(suite, "test invalid cpmDelete", test_cpmDelete_invalid_filename))
    ) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Run tests using the basic interface
    CU_basic_set_mode(CU_BRM_VERBOSE); // Verbose mode
    CU_basic_run_tests();
    CU_cleanup_registry();
    return CU_get_error();
}
