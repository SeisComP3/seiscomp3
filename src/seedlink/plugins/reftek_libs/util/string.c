#pragma ident "$Id: string.c 165 2005-12-23 12:34:58Z andres $"
/*======================================================================
 *
 *  Misc. string operations.
 *
 *  util_strpad:
 *  Replace all characters from current end-of-string to specified
 *  length with constant character.  User is responsible for memory.
 *
 *  Pointer to begining of padded string is returned.
 *
 *----------------------------------------------------------------------
 *
 *  util_strtrm:
 *  Truncate a string, removing trailing blanks.  Truncated string
 *  is returned.
 *
 *----------------------------------------------------------------------
 *
 *  util_ucase:
 *  Convert a string to all upper case.  Pointer to string is returned.
 *
 *----------------------------------------------------------------------
 *
 *  util_lcase:
 *  Convert a string to all lower case.  Pointer to string is returned.
 *
 *====================================================================*/
#include "util.h"

CHAR *util_strpad(CHAR *input, UINT16 maxlen, CHAR padchar)
{
UINT16 i;

    if (input == NULL) {
        errno = EINVAL;
        return (CHAR *) NULL;
    }

    if (strlen(input) == maxlen) return input;
    for (i = strlen(input); i < maxlen-1; i++) input[i] = padchar;
    input[maxlen-1] = 0;

    return input;

}

CHAR *util_strtrm(CHAR *input)
{
UINT16 n;

    if (input == NULL) {
        errno = EINVAL;
        return (CHAR *) NULL;
    }

	if (strlen(input) <= 0) return input;

    n = strlen(input) - 1;
    while (n != 0 && input[n] == ' ') --n;
    input[++n] = 0;

    return input;
}

CHAR *util_ucase(CHAR *input)
{
UINT16 i;

    if (input == NULL) {
        errno = EINVAL;
        return (CHAR *) NULL;
    }

    for (i = 0; i < strlen(input); i++) {
        if (islower(input[i])) input[i] = toupper(input[i]);
    }

    return input;

}

CHAR *util_lcase(CHAR *input)
{
UINT16 i;

    if (input == NULL) {
        errno = EINVAL;
        return (CHAR *) NULL;
    }

    for (i = 0; i < strlen(input); i++) {
        if (isupper(input[i])) input[i] = tolower(input[i]);
    }

    return input;

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
