#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>
#include <stdint.h>
#include "../../diskSimulator.h"
#include "../../cpmfsys.h"

// Test case for valid mkDirStruct()
void test_checkLegalName_valid_with_extension() {
    CU_ASSERT(checkLegalName("myfile.txt") == true);  // Valid 8.3 filename
}

void test_checkLegalName_valid_no_extension() {
    CU_ASSERT(checkLegalName("myfile") == true);  // Valid filename with no extension
}

void test_checkLegalName_filename_too_long() {
    CU_ASSERT(checkLegalName("thisfileiswaytoolong.txt") == false);  // Invalid filename
}

void test_checkLegalName_illegal_characters() {
    CU_ASSERT(checkLegalName("myfile@.txt") == false);  // Invalid '@' in filename
    CU_ASSERT(checkLegalName("myfile.doc!") == false); // Invalid '!' in extension
}


int main() {
    // Initialize CUnit test registry
    if (CUE_SUCCESS != CU_initialize_registry())
        return CU_get_error();

    // Add a suite for mkDirStruct
    CU_pSuite suite = CU_add_suite("checkLegalName Suite", NULL, NULL);
    if (NULL == suite) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    // Add tests to the suite
    if (
        (NULL == CU_add_test(suite, "test validExtension checkLegalName", test_checkLegalName_valid_with_extension)) ||
        (NULL == CU_add_test(suite, "test validNoExtension checkLegalName", test_checkLegalName_valid_no_extension)) ||
        (NULL == CU_add_test(suite, "test illegalChar checkLegalName", test_checkLegalName_illegal_characters)) ||
        (NULL == CU_add_test(suite, "test tooLong checkLegalName", test_checkLegalName_filename_too_long)) 
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
