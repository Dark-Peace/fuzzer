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
void run_test(bool has_content) {
    createTar(&header, archive, has_content);
    // execution: ./name extractor_x86_64
    if (test(extractor, archive)) {
    	crashCount += 1;
    	success[8] = crashCount;
    	createTar(&header, success, has_content);
    }
    // Reset for next test : Load a tar from a base file
    tar_to_struct(&header);
}

void header_field_test(int x) {
    // add archive termination
    memset(header.termination, 0, TERM_SIZE);
    run_test(true);
}

void single_basic_test(int x, char* field, int size, int value) {
    // for each test, the field is filled with the test value, then the test is called
    memset(field, value, size-1);
    field[size - 1] = 0;                // null termination
    header_field_test(0);
}

int print_test(char* field, char* test) {
    printf("Test: %s%s\n", field, test);
    return 0;
}

void basic_field_tests(char* field_name, char* field, int size) {
    //----- invalid inputs -----

    // field empty
    strncpy(field, "", size);
    // no null termination
    header_field_test(print_test(field_name, " is empty"));

    // non numeral value
    single_basic_test(print_test(field_name, " is not a number"), field, size, 'a');

    // non octal value
    single_basic_test(print_test(field_name, " is not octal"), field, size, '9');

    // non ascii value (any emoji or unicode character in general is valid)
    // you can get unicode char on https://symbl.cc/fr/
    single_basic_test(print_test(field_name, " is not ascii"), field, size, '♥');

    // control characters
    for (int i = 0; i < (int) sizeof(control_chars); i++) {
        single_basic_test(print_test(field_name, " has a control character"), field, size, control_chars[i]);
    }

    //----- other inputs -----

    // max value
    single_basic_test(print_test(field_name, " is max"), field, size, '7');

    // 0 everywhere
    single_basic_test(print_test(field_name, " is 0"), field, size, '0');

    //----- null termination -----

    // no null termination
    memset(field, '1', size);
    header_field_test(print_test(field_name, " has no null termination"));

    // null termination midway
    memset(field, 0, size);
    memset(field, '1', size/2);
    header_field_test(print_test(field_name, " has null termination midway"));

    // null termination everywhere!
    single_basic_test(print_test(field_name, " has only null termination"), field, size, 0);
}


void test_name() {
    printf("n");
    basic_field_tests("name", header.name, NAME_LEN);
}

void test_mode() {
    basic_field_tests("mode", header.mode, MODE_LEN);

    // test all modes
    for (int i = 0; i < 12; i++) {
        sprintf(header.mode, "%07o", ALL_MODE[i]);
        header_field_test(print_test("mode", " is invalid"));
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
    header_field_test(print_test(test_name, " "));
}

void test_size() {
    basic_field_tests("size", header.size, SIZE_LEN);

    single_test_size("negative size", -1);
    single_test_size("size = 0", 0);
    single_test_size("size = 1", 1);
    single_test_size("size = 512", BLOCK_SIZE);
    single_test_size("size too big", BLOCK_SIZE*5);
}

void single_test_mtime(char* test_name, int value) {
    sprintf(header.mtime, "%lo", value);
    header_field_test(print_test(test_name, " "));
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
            header_field_test(print_test("bad version", " "));
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
    header_field_test(print_test("negative typeflag", " "));

    // test non-ascii
    header.typeflag = "♥";
    header_field_test(print_test("non ascii typeflag", " "));

    for (int i = 0; i < (int) sizeof(control_chars); i++) {
        header.typeflag = control_chars[i];
        header_field_test(print_test("typeflag has a control character", " "));
    }

    // test every value
    // we can afford it because there's not many
    for (int i = 0; i < 256; i++) {
        header.typeflag = i;
        header_field_test(print_test("bad typeflag", " "));
    }

}

void test_files() {
    //@todo multiple files with same name
    //@todo directory with content like it was a file
    //@todo very big file
}

void test_archive_termination() {
    // tar files end by 1024 bytes of 0, we test what happens if that's not the case
    int term_amount[] = {0, 1, TERM_SIZE/2 , TERM_SIZE-1, TERM_SIZE+1, TERM_SIZE*2};

    for (unsigned i = 0; i < sizeof(term_amount)/sizeof(int); i++) {
        memset(header.termination, 0, term_amount[i]);
        printf("Crash: archive termination with size %d\n", term_amount[i]);
        run_test(true);
    }

    // test file termination without file content before the termination
    memset(header.termination, 0, TERM_SIZE);
    print_test("file termination", " without content");
    run_test(false);
}

void test_empty_header() {
    //@todo
}


void test_fields() {
    test_empty_header();
    test_name();
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
    test_typeflag();
    test_files();
    test_archive_termination();
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
