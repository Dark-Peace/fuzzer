#include "utils.h"
#include "help.c"


// For proper naming of the successful tars.
int crashCount = 0;
// Names of the successful tars.
char success[] = "success_x.tar";
static struct tar_t header;
char* extractor;

// Create a tar, test it and save it if it crashed the program.
// Resets the tar struct after.
void header_field_test(char* test_name) {
    createTar(&header, archive);
    // execution: ./name extractor_x86_64
    if (test(extractor, archive)) {
    	crashCount += 1;
    	success[8] = crashCount;
        printf("Crash: %s\n", test_name);
    	createTar(&header, success);
    }
    // Load a tar from a base file
    tar_to_struct(&header);
}

void single_basic_test(char* test_name, char* field, int size, int value) {
    // for each test, the field is filled with the test value, then the test is called
    memset(field, value, size-1);
    field[size - 1] = 0;                // null termination
    header_field_test(test_name);
}


void basic_field_tests(char* field_name, char* field, int size) {
    //----- invalid inputs -----

    // field empty
    strncpy(field, "", size);
    // no null termination
    header_field_test(strcat(field_name, "is empty"));

    // non numeral value
    single_basic_test(strcat(field_name, " is not a number"), field, size, 'a');

    // non octal value
    single_basic_test(strcat(field_name, " is not octal"), field, size, '9');

    // non ascii value (any emoji or unicode character in general is valid)
    // you can get unicode char on https://symbl.cc/fr/
    single_basic_test(strcat(field_name, " is not ascii"), field, size, '♥');

    // control characters
    for (int i = 0; i < (int) sizeof(control_chars); i++) {
        single_basic_test(strcat(field_name, " has a control character"), field, size, control_chars[i]);
    }

    //----- other inputs -----

    // max value
    single_basic_test(strcat(field_name, " is max"), field, size, '7');

    // 0 everywhere
    single_basic_test(strcat(field_name, " is 0"), field, size, '0');

    //----- null termination -----

    // no null termination
    memset(field, '1', size);
    header_field_test(strcat(field_name, " has no null termination"));

    // null termination midway
    memset(field, 0, size);
    memset(field, '1', size/2);
    header_field_test(strcat(field_name, " has null termination midway"));

    // null termination everywhere!
    single_basic_test(strcat(field_name, " has only null termination"), field, size, 0);

}


void test_name() {
    basic_field_tests("name", header.name, NAME_LEN);
}

void test_mode() {
    basic_field_tests("mode", header.mode, MODE_LEN);

    // test all modes
    for (int i = 0; i < 12; i++) {
        sprintf(header.mode, "%07o", ALL_MODE[i]);
        header_field_test("bad mode");
    }
}

void test_uid() {
    basic_field_tests("uid", header.uid, UID_LEN);
}

void test_gid() {
    basic_field_tests("gid", header.gid, GID_LEN);
}

void single_test_size(char* test_name, int value) {
    sprintf(header.size, "%0*lo", value);
    header_field_test(test_name);
}

void test_size() {
    basic_field_tests("size", header.size, SIZE_LEN);

    single_test_size("negative size", -1);
    single_test_size("size = 0", 0);
    single_test_size("size = 1", 1);
    single_test_size("size = 512", BLOCK_SIZE);
    single_test_size("size too big", BLOCK_SIZE*10);
}

void single_test_mtime(char* test_name, int value) {
    sprintf(header.mtime, "%lo", value);
    header_field_test(test_name);
}

void test_mtime() {
    basic_field_tests("mtime", header.mtime, MTIME_LEN);

    time_t now = time(NULL);
    single_test_mtime("negative mtime", -1);
    single_test_mtime("null mtime", 0);
    single_test_mtime("mtime = now", now);
    single_test_mtime("mtime = 1y ago", now - (3600*24*365));
    single_test_mtime("mtime = 1y later", now + (3600*24*365));
    single_test_mtime("mtime = max INT", 2147483647);
    single_test_mtime("mtime = min INT", -2147483648);
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

    // test every value
    // we can afford it because there's not many
    char version[3] = "00\0";
    for (int i = 0; i < 8; i++) {
        version[0] = "0"+i;
        for (int j = 0; j < 8; j++) {
            version[1] = "0"+j;
            strncpy(header.version, version, VERSION_LEN);
        }
    }
}

void test_uname() {
    basic_field_tests("uname", header.uname, UNAME_LEN);
}

void test_gname() {
    basic_field_tests("gname", header.gname, GNAME_LEN);
}

void test_typeflag() {
    // test negative
    header.typeflag = -1;
    header_field_test("negative typeflag");

    // test non-ascii
    header.typeflag = "♥";
    header_field_test("non ascii typeflag");

    for (int i = 0; i < (int) sizeof(control_chars); i++) {
        header.typeflag = control_chars[i];
        header_field_test("typeflag has a control character");
    }

    // test every value
    // we can afford it because there's not many
    for (int i = 0; i < 256; i++) {
        header.typeflag = i;
        header_field_test("bad typeflag");
    }

}


void test_fields() {
	test_mode();
	test_uid();
	test_gid();
	test_size();
	test_mtime();
	test_chksum();
	test_linkname();
	test_magic();
	test_version();
	test_uname();
	test_gname();
}

/**
 * Creates tars and tests if they crash the program provided as argument.
 * If one does, copy it and name it success_X.
 * The methods used for crashing are: incomplete/inappropriate header.
 * Archive shenanigans
 */
int main(int argc, char* argv[]) {
	// should print what the error is about
	if(argc < 2)
		return 0;
    // Load a tar from a base file.
    if(!tar_to_struct(&header))
    	return 0;
    extractor = argv[1];
    test_fields();
    return crashCount;
};
