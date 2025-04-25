#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdint.h>
#include "../../diskSimulator.h"
#include "../../cpmfsys.h"

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

// Testing with a valid case with less than full length
void test_cpmRename_successNotFull() {

    int result = cpmRename("mytestf1.txt", "mytest2.tx");
    CU_ASSERT(result == 0);

    blockRead(diskBlock0, 0);
    int extentNum = findExtentWithName("mytest2.tx", diskBlock0);
    
    CU_ASSERT(strncmp((char*)(diskBlock0 + (extentNum * EXTENT_SIZE) + 1), "mytest2 tx ", 9 + 4 - 2) == 0);
}

// Testing with a valid case with full length
void test_cpmRename_successFull() {

    int result = cpmRename("mytest2.tx", "mytestf1.txt");
    CU_ASSERT(result == 0);

    blockRead(diskBlock0, 0);
    int extentNum = findExtentWithName("mytestf1.txt", diskBlock0);
    
    CU_ASSERT(strncmp((char*)(diskBlock0 + (extentNum * EXTENT_SIZE) + 1), "mytestf1txt", 9 + 4 - 2) == 0);
}

void test_cpmRename_invalid_filename() {

    int result = cpmRename("!invalid", "newname");
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
    CU_pSuite suite = CU_add_suite("cpmRename Suite", setup, cleanup);
    if (NULL == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Add tests to the suite
    if (
        (NULL == CU_add_test(suite, "test SuccessNotFull cpmRename", test_cpmRename_successNotFull)) ||
        (NULL == CU_add_test(suite, "test SuccessFull cpmRename", test_cpmRename_successFull)) ||
        (NULL == CU_add_test(suite, "test invalid cpmRename", test_cpmRename_invalid_filename))
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
