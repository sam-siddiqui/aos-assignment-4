#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdint.h>
#include "../../diskSimulator.h"
#include "../../cpmfsys.h"

// Example buffer simulating Block 0 of the disk
uint8_t diskBlock0[EXTENT_SIZE * 2] = {
    0x01, 0x6D, 0x79, 0x74, 0x65, 0x73, 0x74, 0x66, 0x31, 0x74, 0x78, 0x74, 0x01, 0x02, 0x03, 0x04,
    0xFF, 0x0E, 0xFD, 0x0C, 0xFB, 0x0A, 0xF9, 0x08, 0xF7, 0x06, 0xF5, 0x04, 0xF3, 0x02, 0xF1, 0x20,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

DirStructType entry = {
    .status = 1,
    .name = "myfile",    // Shorter name
    .extension = "txt",  // Valid extension
    .XL = 0x01,
    .BC = 0x02,
    .XH = 0x03,
    .RC = 0x04,
    .blocks = {1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Test case for valid writeDirStruct()
void test_writeDirStruct_valid(void) {
    writeDirStruct(&entry, 0, diskBlock0); // Invalid index
    CU_ASSERT(diskBlock0[0] == 0x01);
}

// Test case for invalid index in writeDirStruct()
void test_writeDirStruct_invalid_index(void) {
    writeDirStruct(&entry, 32, diskBlock0); // Invalid index
    CU_ASSERT(diskBlock0[32] == 0x00);
}

int main() {
    // Initialize CUnit test registry
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    // Add a suite for writeDirStruct
    CU_pSuite suite = CU_add_suite("writeDirStruct Suite", NULL, NULL);
    if (NULL == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Add tests to the suite
    if (
        (NULL == CU_add_test(suite, "test valid writeDirStruct", test_writeDirStruct_valid)) ||
        (NULL == CU_add_test(suite, "test invalid writeDirStruct", test_writeDirStruct_invalid_index))
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
