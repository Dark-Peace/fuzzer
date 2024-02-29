#include <stdio.h>
#include <string.h>

#ifndef UTILS
#define UTILS

char archive[] = "archive.tar";
const char control_chars[] = { '\n', '\t', '\b', '\"', '\'', '\a',
                                '\v', '\r', '\f', '\\', '\0', '\x7F',
                                '\x1A', '\x1B', '\x1C', '\x1D', '\x1E',
                                '\x1F', ' '};

struct tar_t {                 /* byte offset */
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

const enum header_len {
    NAME_LEN = 100,
    MODE_LEN = 8,
    UID_LEN = 8,
    GID_LEN = 8,
    SIZE_LEN = 12,
    MTIME_LEN = 12,
    CHKSUM_LEN = 8,
    TYPE_LEN = 1,
    LINK_LEN = 100,
    MAGIC_LEN = 6,
    VERSION_LEN = 2,
    UNAME_LEN = 32,
    GNAME_LEN = 32,
    DEVMAJOR_LEN = 8,
    DEVMINOR_LEN = 8,
    PREFIX_LEN = 155,
    PADDING_LEN = 12
} lens; //otherwise a warning that this instance of it doesnt have a name

/* Bits used in the mode field, values in octal.  */
#define TSUID    04000          /* set UID on execution */
#define TSGID    02000          /* set GID on execution */
#define TSVTX    01000          /* reserved */
                                /* file permissions */
#define TUREAD   00400          /* read by owner */
#define TUWRITE  00200          /* write by owner */
#define TUEXEC   00100          /* execute/search by owner */
#define TGREAD   00040          /* read by group */
#define TGWRITE  00020          /* write by group */
#define TGEXEC   00010          /* execute/search by group */
#define TOREAD   00004          /* read by other */
#define TOWRITE  00002          /* write by other */
#define TOEXEC   00001          /* execute/search by other */

static const unsigned ALL_MODE[] = {TSUID,TSGID,TSVTX,TUREAD,
                                    TUWRITE,TUEXEC,TGREAD,TGWRITE,
                                    TGEXEC,TOREAD,TOWRITE,TOEXEC};

const int BLOCK_SIZE = 512;
const int PARAM_NUM = 17;

#endif

