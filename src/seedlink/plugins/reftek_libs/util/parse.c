#pragma ident "$Id: parse.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 *  parse a string (new version with quote support)
 *
 *====================================================================*/
#include "util.h"

UINT16 util_parse(
    CHAR *input,      /* input string                */
    CHAR **argv,      /* output array to hold tokens */
    CHAR *delimiters, /* token delimiters            */
    UINT16  max_tokens, /* max number of tokens (#elements in argv) */
    CHAR quote        /* quote character for strings */
){
CHAR *ptr;
UINT16 i = 0, nquote = 0;

    if (max_tokens < 1) {
        errno = EINVAL;
        return -1;
    }

/* Save embedded blanks inside quoted strings */

    if (quote != 0) {
        for (ptr = input; *ptr != (char) 0; ptr++) {
            if (*ptr == quote) {
                if (++nquote == 2) nquote = 0;
            } else {
                if (nquote == 1 && *ptr == ' ') *ptr = (char) -1;
            }
        }
    }

/* Parse the string, restoring blanks if required */

    if ((argv[0] = strtok(input, delimiters)) == NULL) return 0;
    
    i = 1;
    do {
        if ((argv[i] = strtok(NULL, delimiters)) != NULL && quote != 0) {
            for (ptr = argv[i]; *ptr != (char) 0; ptr++) {
                if (*ptr == (char) -1) *ptr = ' ';
            }
        }
    } while (argv[i] != NULL && ++i < max_tokens);

/* Return the number of tokens */

    return i;
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.2  2002/01/18 17:51:45  nobody
 * replaced WORD, BYTE, LONG, etc macros with size specific equivalents
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
