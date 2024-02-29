#include "tests.c"



// Convert a given tar file to a struct
char* tar_to_struct(struct tar_t* entry) {
    FILE *fptr;
    fptr = fopen("base.tar", "r");

    for (int i = 0; i < 100; i++) { entry->name[i] = fgetc(fptr); }
    for (int i = 0; i < 8; i++) { entry->mode[i] = fgetc(fptr); }
    for (int i = 0; i < 8; i++) { entry->uid[i] = fgetc(fptr); }
    for (int i = 0; i < 8; i++) { entry->gid[i] = fgetc(fptr); }
    for (int i = 0; i < 12; i++) { entry->size[i] = fgetc(fptr); }
    for (int i = 0; i < 12; i++) { entry->mtime[i] = fgetc(fptr); }
    for (int i = 0; i < 8; i++) { entry->chksum[i] = fgetc(fptr); }
    entry->typeflag = fgetc(fptr);
    for (int i = 0; i < 100; i++) { entry->linkname[i] = fgetc(fptr); }
    for (int i = 0; i < 6; i++) { entry->magic[i] = fgetc(fptr); }
    for (int i = 0; i < 2; i++) { entry->version[i] = fgetc(fptr); }
    for (int i = 0; i < 32; i++) { entry->uname[i] = fgetc(fptr); }
    for (int i = 0; i < 32; i++) { entry->gname[i] = fgetc(fptr); }
    for (int i = 0; i < 8; i++) { entry->devmajor[i] = fgetc(fptr); }
    for (int i = 0; i < 8; i++) { entry->devminor[i] = fgetc(fptr); }
    for (int i = 0; i < 155; i++) { entry->prefix[i] = fgetc(fptr); }
    for (int i = 0; i < 12; i++) { entry->padding[i] = fgetc(fptr); }
    for (int i = 0; i < BLOCK_SIZE; i++) { entry->content[i] = fgetc(fptr); }

    fclose(fptr);
    return 0;
}


// introduce errors in the tar file
int introduce_errors(struct tar_t* entry) {

    return 0;
}


/**
 * Computes the checksum for a tar header and encode it on the header
 * @param entry: The tar header
 * @return the value of the checksum
 */
unsigned int calculate_checksum(struct tar_t* entry){
    // use spaces for the checksum bytes while calculating the checksum
    memset(entry->chksum, ' ', 8);

    // sum of entire metadata
    unsigned int check = 0;
    unsigned char* raw = (unsigned char*) entry;
    for(int i = 0; i < BLOCK_SIZE; i++){
        check += raw[i];
    }

    snprintf(entry->chksum, sizeof(entry->chksum), "%06o0", check);

    entry->chksum[6] = '\0';
    entry->chksum[7] = ' ';
    return check;
}


// Create a valid tar from the given struct.
int createTar(struct tar_t* entry, char path[]) {
    FILE *fptr;
    fptr = fopen(path, "w");

    // last modif to the data
    calculate_checksum(entry);

    // write the result to a tar file
    fwrite(entry->name, 100, 1, fptr);
    fwrite(entry->mode, 8, 1, fptr);
    fwrite(entry->uid, 8, 1, fptr);
    fwrite(entry->gid, 8, 1, fptr);
    fwrite(entry->size, 12, 1, fptr);
    fwrite(entry->mtime, 12, 1, fptr);
    fwrite(entry->chksum, 8, 1, fptr);
    fwrite(&(entry->typeflag), 1, 1, fptr);
    fwrite(entry->linkname, 100, 1, fptr);
    fwrite(entry->magic, 6, 1, fptr);
    fwrite(entry->version, 2, 1, fptr);
    fwrite(entry->uname, 32, 1, fptr);
    fwrite(entry->gname, 32, 1, fptr);
    fwrite(entry->devmajor, 8, 1, fptr);
    fwrite(entry->devminor, 8, 1, fptr);
    fwrite(entry->prefix, 155, 1, fptr);
    fwrite(entry->padding, 12, 1, fptr);

    fwrite(entry->content, BLOCK_SIZE, 1, fptr);

    fclose(fptr);
    return 0;
};

/**
 * Launches another executable given as argument,
 * parses its output and check whether or not it matches "*** The program has crashed ***".
 * @param the path to the executable
 * @return -1 if the executable cannot be launched,
 *          0 if it is launched but does not print "*** The program has crashed ***",
 *          1 if it is launched and prints "*** The program has crashed ***".
 */
int test(char exec[], char path[]) {
    int rv = 0;
    char cmd[52];
    strncpy(cmd, exec, 25);
    cmd[26] = '\0';
    cmd[27] = ' ';
    strncat(cmd, path, 25);
    char buf[33];
    FILE *fp;

    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Error opening pipe!\n");
        return -1;
    }

    if(fgets(buf, 33, fp) == NULL) {
        printf("No output\n");
        goto finally;
    }
    if(strncmp(buf, "*** The program has crashed ***\n", 33)) {
        printf("Not the crash message\n");
        goto finally;
    } else {
        printf("Crash message\n");
        rv = 1;
        goto finally;
    }
    finally:
    if(pclose(fp) == -1) {
        printf("Command not found\n");
        rv = -1;
    }
    return rv;
}


 
/**
 * Creates tars and tests if they crash the program provided as argument. If one does, copy it and name it success_X.
 * The methods used for crashing are: incomplete/inappropriate header.
 * Archive shinanigens
 */
int main(int argc, char* argv[]) {
	// For proper naming of the successful tars.
	int crashCount = 0;
    // Names of the successful tars.
    char success[] = "success_x.tar";

    // Load a tar from a base file.
    struct tar_t header;
    tar_to_struct(&header);
    introduce_errors(&header);
    
    char archive[] = "archive.tar";
    createTar(&header, archive);
    // execution: ./name extractor_x86_64
    if test(argv[1], archive)
    {
    	crashCount += 1;
    	success[9] = crashCount;
    	createTar(&header, success);
    }
    return 0;
};
