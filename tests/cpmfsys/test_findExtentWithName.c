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
     * '\x01', 'm', 'y', 't', 'e', 's', 't', ' ', ' ', ' ', ' ', ' ', 
     * '\x01', '\x02', '\x03', '\x04',
     * '\xFF', '\x0E', '\xFD', '\x0C', '\xFB', '\x0A', '\xF9', '\x08', 
     * '\xF7', '\x06', '\xF5', '\x04', '\xF3', '\x02', '\xF1', ' '
     */
    0x01, 0x6D, 0x79, 0x74, 0x65, 0x73, 0x74, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x02, 0x03, 0x04,
    0xFF, 0x0E, 0xFD, 0x0C, 0xFB, 0x0A, 0xF9, 0x08, 0xF7, 0x06, 0xF5, 0x04, 0xF3, 0x02, 0xF1, 0x20,
    
    /**
     * '\x01', 's', 'h', 'o', 'r', 't', 'f', ' ', ' ', 'p', 's', ' ', '\x01', '\x08', '\x03', '\x00',
     * '0', '1', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
     * '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'
     */
    0x01, 0x73, 0x68, 0x6F, 0x72, 0x74, 0x66, 0x20, 0x20, 0x70, 0x73, 0x20, 0x01, 0x08, 0x03, 0x00,
    0x30, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,

    /**
     * '\xe5', 's', 'h', 'o', 'r', 't', ' ', ' ', ' ', 'p', 's', ' ', 
     * '\x01', '\x08', '\x03', '\x00', '0', '1', 
     * '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', '\x00', 
     * '\x00', '\x00', '\x00', '\x00', '\x00', '\x00'
     */
    0xe5, 0x73, 0x68, 0x6F, 0x72, 0x74, 0x20, 0x20, 0x20, 0x70, 0x73, 0x20, 0x01, 0x08, 0x03, 0x00,
    0x30, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Test case for valid findExtentWithName()
void test_findExtentWithName_valid(void) {
    int result = findExtentWithName("mytestf1.txt", diskBlock0);
    CU_ASSERT(result == 0);
}

// Test case for short names wit blank spaces
void test_findExtentWithName_validShort(void) {
    int result = findExtentWithName("shortf.ps", diskBlock0);
    CU_ASSERT(result == 2);
}

// Test case for short names wit blank spaces
void test_findExtentWithName_validNoExt(void) {
    int result = findExtentWithName("mytest.", diskBlock0);
    CU_ASSERT(result == 1);
}

// Test case for skipping unused extent in findExtentWithName()
// Even if data exists in the extent
void test_findExtentWithName_unused(void) {
    int result = findExtentWithName("short.ps", diskBlock0);
    CU_ASSERT(result == -1);
}

// Test case for skipping unused extent in findExtentWithName()
// Even if data exists in the extent
void test_findExtentWithName_invalid(void) {
    int result = findExtentWithName("sho@rt.ps", diskBlock0);
    CU_ASSERT(result == -1);
}

// Test case for skipping unused extent in findExtentWithName()
// Even if data exists in the extent
void test_findExtentWithName_nonExistant(void) {
    int result = findExtentWithName("long.ps", diskBlock0);
    CU_ASSERT(result == -1);
}

int main() {
    // Initialize CUnit test registry
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    // Add a suite for mkDirStruct
    CU_pSuite suite = CU_add_suite("findExtentWithName Suite", NULL, NULL);
    if (NULL == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Add tests to the suite
    if (
        (NULL == CU_add_test(suite, "test valid findExtentWithName", test_findExtentWithName_valid)) ||
        (NULL == CU_add_test(suite, "test validShort findExtentWithName", test_findExtentWithName_validShort)) ||
        (NULL == CU_add_test(suite, "test validNoExt findExtentWithName", test_findExtentWithName_validNoExt)) ||
        (NULL == CU_add_test(suite, "test nonExistant findExtentWithName", test_findExtentWithName_nonExistant)) ||
        (NULL == CU_add_test(suite, "test Unused findExtentWithName", test_findExtentWithName_unused)) ||
        (NULL == CU_add_test(suite, "test invalid findExtentWithName", test_findExtentWithName_invalid))
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
