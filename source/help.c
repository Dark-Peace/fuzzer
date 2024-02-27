#include <stdio.h>
#include <string.h>

struct tar_t
{                              /* byte offset */
    char name[100];               /*   0 */
    char mode[8];                 /* 100 */
    char uid[8];                  /* 108 */
    char gid[8];                  /* 116 */
    char size[12];                /* 124 */
    char mtime[12];               /* 136 */
    char chksum[8];               /* 148 */
    char typeflag;                /* 156 */
    char linkname[100];           /* 157 */
    char magic[6];                /* 257 */
    char version[2];              /* 263 */
    char uname[32];               /* 265 */
    char gname[32];               /* 297 */
    char devmajor[8];             /* 329 */
    char devminor[8];             /* 337 */
    char prefix[155];             /* 345 */
    char padding[12];             /* 500 */
    char content[512];             /* 512 */
};

const int BLOCK_SIZE = 512;
const int PARAM_NUM = 17;






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

int introduce_errors(struct tar_t* entry) {
    // introduce errors in the tar file

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

int createTar(struct tar_t* entry) {
    FILE *fptr;
    fptr = fopen("archive.tar", "w");

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

int test(int argc, char* argv[])
{
    int rv = 0;
    char cmd[51];
    strncpy(cmd, argv[1], 25);
    cmd[26] = '\0';
    strncat(cmd, " archive.tar", 25);
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
 * Launches another executable given as argument,
 * parses its output and check whether or not it matches "*** The program has crashed ***".
 * @param the path to the executable
 * @return -1 if the executable cannot be launched,
 *          0 if it is launched but does not print "*** The program has crashed ***",
 *          1 if it is launched and prints "*** The program has crashed ***".
 *
 * BONUS (for fun, no additional marks) without modifying this code,
 * compile it and use the executable to restart our computer.
 */
int main(int argc, char* argv[]) {
    struct tar_t header;
    printf("ok\n");
    introduce_errors(&header);
    printf("ok\n");
    createTar(&header);
    printf("ok\n");
    // execution: ./name extractor_x86_64
    test(argc, argv);
    return 0;
};
