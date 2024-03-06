#include "utils.h"

// Convert a given tar file to a struct
int tar_to_struct(struct tar_t* entry) {
    FILE *fptr;
    fptr = fopen("base.tar", "r");
    if(!fptr)
    {
        printf("Base file missing.\n");
    	return 0;
	}

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
    return 1;
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
    for(int i = 0; i < BLOCK_SIZE; i++) {
        check += raw[i];
    }

    snprintf(entry->chksum, sizeof(entry->chksum), "%06o0", check);

    entry->chksum[6] = '\0';
    entry->chksum[7] = ' ';
    return check;
}

// Write the header part of the tar file.
int writeHeader(struct tar_t* entry, FILE* fptr)
{
    fwrite(entry->name, NAME_LEN, 1, fptr);
    fwrite(entry->mode, MODE_LEN, 1, fptr);
    fwrite(entry->uid, UID_LEN, 1, fptr);
    fwrite(entry->gid, GID_LEN, 1, fptr);
    fwrite(entry->size, SIZE_LEN, 1, fptr);
    fwrite(entry->mtime, MTIME_LEN, 1, fptr);
    fwrite(entry->chksum, CHKSUM_LEN, 1, fptr);
    fwrite(&(entry->typeflag), 1, 1, fptr);
    fwrite(entry->linkname, LINK_LEN, 1, fptr);
    fwrite(entry->magic, MAGIC_LEN, 1, fptr);
    fwrite(entry->version, VERSION_LEN, 1, fptr);
    fwrite(entry->uname, UNAME_LEN, 1, fptr);
    fwrite(entry->gname, GNAME_LEN, 1, fptr);
    fwrite(entry->devmajor, DEVMAJOR_LEN, 1, fptr);
    fwrite(entry->devminor, DEVMINOR_LEN, 1, fptr);
    fwrite(entry->prefix, PREFIX_LEN, 1, fptr);
    return 1;
}

// Create a valid tar from the given struct.
int createTar(struct tar_t* entry, char path[], bool has_content, bool write, bool final) {
    FILE *fptr;
    // If not write mode we write back.
    // @todo: delete the last one instead, and always wb
    if(write)
    	fptr = fopen(path, "w");
    else
    	fptr = fopen(path, "wb");
    if(!fptr) {
    	return 0;
        printf("Could not write archive file.\n");
    }

    // calculate the padding
    unsigned size_padding = 512 - ((int) entry->size % 512);
    memset(entry->padding, 0, size_padding);
	
    // last modif to the data
    calculate_checksum(entry);

    // write the result to a tar file
    writeHeader(entry, fptr);

    if(has_content) {
    	fwrite(entry->content, BLOCK_SIZE, 1, fptr);
		fwrite(entry->padding, size_padding, 1, fptr);
    }
    

	if(final || write)
		fwrite(entry->termination, TERM_SIZE, 1, fptr);

    fclose(fptr);
    return 1;
};

// Like the function before but write a lot of content.
int extraContent(struct tar_t* entry, char path[], bool checksum)
{
	FILE *fptr;
    fptr = fopen(path, "a");
    if(!fptr) {
    	return 0;
        printf("Could not write archive file.\n");
    }
    //strncpy(entry->size, BLOCK_SIZE * 5, SIZE_LEN);
    
    if(checksum)
    	calculate_checksum(entry);
    writeHeader(entry, fptr);
    fwrite("1", BLOCK_SIZE * 5, 1, fptr);

    fwrite(entry->termination, TERM_SIZE, 1, fptr);
    fclose(fptr);
    return 1;
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
    char cmd[51] = "./";
    strncat(cmd, exec, 23);
    cmd[26] = '\0';
    /*char truepath[26];
    truepath[0] = ' ';
    strncat(truepath, path, 25);*/
    strncat(cmd, " archive.tar", 25);
    printf("tar: %s\n%s\n", "", cmd);
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
        printf("Crash successful#############################################""\n");
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
