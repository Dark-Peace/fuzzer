#include "utils.h"
#include "help.c"


// For proper naming of the successful tars.
int crashCount = 0;
// Names of the successful tars.
// By default assume it's in the same folder as the program is launched.
char* successFile = "success_";
// To avoid having to pass it around everywhere one header is declared here.
static struct tar_t header;

char* extractor;


// Used when a successful tar is found, to copy it.
void saveSuccess()
{
	char success_string[25];
	strcpy(success_string, successFile);
	
	char *success_nbr;
	asprintf(&success_nbr, "%i", crashCount);
	strcat(success_string, success_nbr);

	strcat(success_string, ".tar");
	// save it to path/success_X.tar
	create_tar(&header, success_string, true);
	crashCount += 1;
}

// Used to return to a blank state.
void reset() {
	// Delete the previous archive file so we can always append.
	remove(archive);
    // Reset for next test : Load a tar from a base file
    tar_to_struct(&header);
}

// Create a tar, test it and save it if it crashed the program.
// Resets the tar struct after, and return if this was a success, so we dont test variations of the same attempt afterwards.
int run_test(bool has_content) {
    create_tar(&header, archive, has_content);
    int rv = 0;
    // execution: ./name ./extractor_x86_64
    if (test(extractor, archive))
    {
    	rv = 1;
    	saveSuccess();
    }
	
	reset();
    return rv;
}

// Same as the function above, but will add <amount> multiple files to the same archive, optionaly with all having the same name.
void run_test_append(int amount, bool sameName) {
    for (int i = 0; i < amount - 1; i++)
    {
    	if(!sameName)
    	{
			char *nbr;
			asprintf(&nbr, "%i", i);
    		strncpy(header.name, nbr, NAME_LEN);
    	}
    	create_tar(&header, archive, true);
    }
    
    if (test(extractor, archive))
    	saveSuccess(true);

	reset();
}

int header_field_test(int x) {
    // add archive termination
    memset(header.termination, 0, TERM_SIZE);
    return run_test(true);
}

int single_basic_test(int x, char* field, int size, int value) {
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
    single_basic_test(print_test(field_name, " is not ascii"), field, size, '♥');

    // control characters
    for (int i = 0; i < (int) sizeof(control_chars); i++) {
    	// make sure we only count it once by breaking once one of them works
        if(single_basic_test(print_test(field_name, " has a control character"), field, size, control_chars[i]))
        	break;
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
    sprintf(header.size, "%i", value); // was "%0*lo"
    header_field_test(print_test(test_name, " "));
}

void test_size() {
    basic_field_tests("size", header.size, SIZE_LEN);

    single_test_size("negative size", -1);
    single_test_size("size = 0", 0);
    single_test_size("size = 1", 1);
    single_test_size("size = 512", BLOCK_SIZE);
    single_test_size("size too big", BLOCK_SIZE*3); // 5
}

void single_test_mtime(char* test_name, int value) {
    sprintf(header.mtime, "%i", value);
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
        version[0] = '0'+i;
        for (int j = 0; j < 8; j++) {
            version[1] = '0'+j;
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
    header.typeflag = '♥';
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


// Essentially a custom run_test to force extra content on the file.
void test_big_files() {
	print_test("big", " file");
    extra_content(&header, archive, true);
    if (test(extractor, archive)) {
		saveSuccess(true);
    }
    reset();
    
    print_test("big", " file without checksum");
    extra_content(&header, archive, false);
    if (test(extractor, archive)) {
		saveSuccess(true);
    }
    reset();
}

// Tests with big files and archives.
void test_files() {
	// multiple files with the same name
	print_test("5 files in archive", " with the same name");
	run_test_append(5, true);
	
    test_big_files();
    
    // Change the name to be a directory, but keep the content as a tar file.
    strncpy(header.name, "archive/", NAME_LEN);
    print_test("directory", " as a file");
    run_test(true);
    
    // Make it extract a big amount of files from a single archive.
	print_test("Too big directory", "");
	run_test_append(1000, false);
}


// Test improper archive terminations sizes.
void test_archive_termination() {
    // tar files end by 1024 bytes of 0, we test what happens if that's not the case
    int term_amount[] = {0, 1, TERM_SIZE/2 , TERM_SIZE-1};//, TERM_SIZE+1};//, TERM_SIZE*2}; // gives archive.tar not found, then segfault? disabled for safety

    for (unsigned i = 0; i < sizeof(term_amount)/sizeof(int); i++) {
        memset(header.termination, 0, term_amount[i]);
        printf("Test: archive termination with size %d\n", term_amount[i]);
        run_test(true);
    }

    // test file termination without file content before the termination
    memset(header.termination, 0, TERM_SIZE);
    print_test("file termination", " without content");
    run_test(false);
}

void test_empty_header() {
    strncpy(header.name, "", NAME_LEN);
    strncpy(header.mode, "", MODE_LEN);
    strncpy(header.uid, "", UID_LEN);
    strncpy(header.gid, "", GID_LEN);
    strncpy(header.size, "", SIZE_LEN);
    strncpy(header.mtime, "", MTIME_LEN);
    strncpy(header.chksum, "", CHKSUM_LEN);
    strncpy(&(header.typeflag), "", 1);
    strncpy(header.linkname, "", LINK_LEN);
    strncpy(header.magic, "",  MAGIC_LEN);
    strncpy(header.version, "", VERSION_LEN);
    strncpy(header.uname, "", UNAME_LEN);
    strncpy(header.gname, "", GNAME_LEN);
    strncpy(header.devmajor, "", DEVMAJOR_LEN);
    strncpy(header.devminor, "", DEVMINOR_LEN);
    strncpy(header.prefix, "", PREFIX_LEN);
    strncpy(header.padding, "", PADDING_LEN);
    print_test("header", "empty");
    run_test(false);
}



// Start all the tests.
void run_all_tests() {
    test_empty_header();
    // Note: if the extractor does not crash this will cause it to create files with weird names.
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
 * If one does, copy it and name it path/success_X.tar
 * The methods used for crashing are: incomplete/inappropriate header and archive shenanigans.
 * All the relevant functions are present above.
 */
int main(int argc, char* argv[]) {
	// Assumed execution: "./fuzzer ./<path/>extractor"
	if(argc < 2)
	{
		printf("Missing the link to the executable. (in the form \"./exec_name\")");
		return 0;
    }
    // Load a default tar file.
    tar_to_struct(&header);
	
	// Recover the path to the executable and the folder we're in, if any.
    extractor = argv[1];
    
    // The position of the last '/'.
	int fileI = 0;
    for (int i = 0; i < strlen(extractor) - 1; i++)
    	if(extractor[i] == '/' || extractor[i] == '\\')
    		fileI = i;
    
	char path[21];
    // To make sure it isn't just './'
	if(fileI > 2)
	{
		// Do not copy the initial './', but keep the '/' at the end of the path.
		for (int i = 0; i < fileI - 1; i++)
			path[i] = extractor[i + 2];
		strncat(path, "success_", 8);
	}
	else
		strcpy(path, "success_");
	successFile = path;
	
    // See if we can create a tar file in that folder.
	if(!create_tar(&header, successFile, true))
	{
		printf("Failed to create a tar file, exiting.");
		printf(successFile);
		return 0;
	}
	remove(successFile);
	
	// Test all crashes.
	run_all_tests();
    
    return 1;
};
