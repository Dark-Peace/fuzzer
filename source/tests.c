#include "utils.h"



static struct tar_t header;

// @todo reset the header to the default values after each test

void header_field_test() {
    // @todo
    //f write tar
    //f test extractor
}

void single_basic_test(const char* field_name, char* field, int size, int value) {
    // for each test, the field is filled with the test value, then the test is called
    memset(field, value, size-1);
    field[size - 1] = 0;                // null termination
    header_field_test();
}


void basic_field_tests(const char* field_name, char* field, int size) {
    //----- invalid inputs -----

    // field empty
    strncpy(field, "", size);
    // no null termination
    header_field_test();

    // non numeral value
    single_basic_test(field_name, field, size, "a");

    // non octal value
    single_basic_test(field_name, field, size, "9");

    // non ascii value (any emoji or unicode character in general is valid)
    // you can get unicode char on https://symbl.cc/fr/
    single_basic_test(field_name, field, size, "日");

    // control characters
    for (int i = 0; i < (int) sizeof(control_chars); i++) {
        single_basic_test(field_name, field, size, &control_chars[i]);
    }

    //----- other inputs -----

    // max value
    single_basic_test(field_name, field, size, "7");

    // 0 everywhere
    single_basic_test(field_name, field, size, "0");

    //----- null termination -----

    // no null termination
    memset(field, "1", size);
    header_field_test();

    // null termination midway
    memset(field, 0, size);
    memset(field, "1", size/2);
    header_field_test();

    // null termination everywhere!
    single_basic_test(field_name, field, size, 0);

}

void test_name() {
    basic_field_tests("name", header.name, NAME_LEN);
}

void test_mode() {
    basic_field_tests("mode", header.mode, MODE_LEN);

    // test all modes
    for (int i = 0; i < 12; i++) {
        sprintf(header.mode, "%07o", ALL_MODE[i]);
    }
}

void test_uid() {
    basic_field_tests("uid", header.uid, UID_LEN);
}

void test_gid() {
    basic_field_tests("gid", header.gid, GID_LEN);
}

void test_size() {
    basic_field_tests("size", header.size, SIZE_LEN);

    // @todo test sizes
}

void test_mtime() {
    basic_field_tests("mtime", header.mtime, MTIME_LEN);
}

void test_chksum() {
    basic_field_tests("chksum", header.chksum, CHKSUM_LEN);
}

void test_linkname() {
    basic_field_tests("linkname", header.linkname, LINK_LEN);
}

void test_magic() {
    basic_field_tests("magic", header.magic, MAGIC_LEN);
}

void test_version() {
    basic_field_tests("version", header.version, VERSION_LEN);
}

void test_uname() {
    basic_field_tests("uname", header.uname, UNAME_LEN);
}

void test_gname() {
    basic_field_tests("gname", header.gname, GNAME_LEN);
}
