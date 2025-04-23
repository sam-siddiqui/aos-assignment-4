#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdint.h>
#include "../../diskSimulator.h"
#include "../../cpmfsys.h"

// Example buffer simulating Block 0 of the disk
uint8_t diskBlock0[EXTENT_SIZE * 2] = {
    0x01, 0x6D, 0x79, 0x74, 0x65, 0x73, 0x74, 0x66, 0x31, 0x74, 0x78, 0x74, 0x01, 0x02, 0x03, 0x04,
    0xFF, 0x0E, 0xFD, 0x0C, 0xFB, 0x0A, 0xF9, 0x08, 0xF7, 0x06, 0xF5, 0x04, 0xF3, 0x02, 0xF1, 0x20,

    0x01, 0x73, 0x68, 0x6F, 0x72, 0x74, 0x66, 0x20, 0x70, 0x73, 0x20, 0x01, 0x08, 0x03, 0x00, 0x30,
    0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Test case for valid mkDirStruct()
void test_mkDirStruct_valid(void) {
    DirStructType *dir = mkDirStruct(0, diskBlock0);
    CU_ASSERT_PTR_NOT_NULL(dir); // Ensure the structure was created
    CU_ASSERT(dir->status == 0x01);
    CU_ASSERT_STRING_EQUAL(dir->name, "mytestf1");
    CU_ASSERT_STRING_EQUAL(dir->extension, "txt");
    free(dir); // Clean up
}

void test_mkDirStruct_valid2(void) {
    DirStructType *dir = mkDirStruct(1, diskBlock0);
    CU_ASSERT_PTR_NOT_NULL(dir); // Ensure the structure was created
    CU_ASSERT(dir->status == 0x01);
    CU_ASSERT_STRING_EQUAL(dir->name, "mytestf1");
    CU_ASSERT_STRING_EQUAL(dir->extension, "txt");
    free(dir); // Clean up
}

// Test case for invalid index in mkDirStruct()
void test_mkDirStruct_invalid_index(void) {
    DirStructType *dir = mkDirStruct(32, diskBlock0); // Invalid index
    CU_ASSERT_PTR_NULL(dir); // mkDirStruct should return NULL for invalid index
}

int main() {
    // Initialize CUnit test registry
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    // Add a suite for mkDirStruct
    CU_pSuite suite = CU_add_suite("mkDirStruct Suite", NULL, NULL);
    if (NULL == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Add tests to the suite
    if (
        (NULL == CU_add_test(suite, "test valid mkDirStruct", test_mkDirStruct_valid)) ||
        (NULL == CU_add_test(suite, "test invalid mkDirStruct", test_mkDirStruct_invalid_index))
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
