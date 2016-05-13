#pragma ident "$Id: util.h 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 * Defines, templates, and prototypes for general utilities library
 *
 *====================================================================*/
#ifndef util_include_defined
#define util_include_defined

/* platform specific stuff */

#include "platform.h"
#include "mstime.h"
#include "timer.h"

/* misc constants */

#define UTIL_MAXTIMESTRLEN 32

/* For the findfirst, etc functions */

#ifndef _MAX_FNAME
#    define MAX_FNAME_LEN 255
#else
#    define MAX_FNAME_LEN _MAX_FNAME
#endif

#if defined (unix) || defined (MACOSX)
    struct _finddata_t {
        unsigned  attrib;
        time_t    time_create;
        time_t    time_access;
        time_t    time_write;
        size_t    size;
        char      name[MAX_FNAME_LEN+1];
    };

#   define _A_NORMAL 0x00    /* Normal file - No read/write restrictions */
#   define _A_RDONLY 0x01    /* Read only file */
#   define _A_HIDDEN 0x02    /* Hidden file */
#   define _A_SYSTEM 0x04    /* System file */
#   define _A_SUBDIR 0x10    /* Subdirectory */
#   define _A_ARCH   0x20    /* Archive file */
#endif

/* function prototypes */

UINT32 utilBcdToUint32(UINT8 *input, UINT16 numDigits, UINT16 nibble);
UINT16 util_getline(FILE *fp, CHAR *buffer, INT32 buflen, CHAR comment, INT32 *lineno);
UINT16 util_parse(CHAR *input, CHAR **argv, CHAR *delimiters, UINT16  max_tokens, CHAR quote);
CHAR *util_strpad(CHAR *input, UINT16 maxlen, CHAR padchar);
CHAR *util_strtrm(CHAR *input);
CHAR *util_ucase(CHAR *input);
CHAR *util_lcase(CHAR *input);
VOID util_lswap(UINT32 *input, INT32 number);
VOID util_sswap(UINT16 *input, INT32 number);
VOID util_iftovf(UINT32 *input, INT32 number);
VOID util_vftoif(UINT32 *input, INT32 number);
REAL64 util_attodt(CHAR *string);
CHAR *util_dttostr(REAL64 dtime, UINT16 code, CHAR *buf);
CHAR *util_lttostr(INT32 ltime, UINT16 code, CHAR *buf);
VOID util_tsplit(REAL64 dtime, UINT16 *yr, UINT16 *da, UINT16 *hr, UINT16 *mn, UINT16 *sc, UINT16 *ms);
REAL64 util_ydhmsmtod(UINT16 yr, UINT16 da, UINT16 hr, UINT16 mn, UINT16 sc, UINT16 ms);
VOID util_jdtomd(UINT16 year, UINT16 day, UINT16 *m_no, UINT16 *d_no);
INT32 util_ymdtojd(UINT16 year, UINT16 mo, UINT16 da);
INT32 util_today(VOID);
VOID util_bindmp(FILE *fp, UINT8 *ptr, INT32 count, INT32 off, CHAR obase);
VOID util_hexdmp(FILE *fp, UINT8 *ptr, INT32 count, INT32 off, CHAR obase);
VOID util_octdmp(FILE *fp, UINT8 *ptr, INT32 count, INT32 off, CHAR obase);

#ifndef WINNT
    INT32 util_findclose(INT32 handle);
    INT32 util_findnext(INT32 handle, struct _finddata_t *fileinfo);
    INT32 util_findfirst(CHAR *filespec, struct _finddata_t *fileinfo);
#   define _findclose(a) util_findclose(a)
#   define _findnext(a, b) util_findnext(a, b)
#   define _findfirst(a, b) util_findfirst(a, b)
#else
#   define util_findclose(a) _findclose(a)
#   define util_findnext(a, b) _findnext(a, b)
#   define util_findfirst(a, b) _findfirst(a, b)
#endif

#endif /* util_include_defined */

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:49:01  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
