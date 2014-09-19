#pragma ident "$Id: parse.c,v 1.11 2006/02/14 17:05:30 dechavez Exp $"
/*======================================================================
 *
 *  parse strings
 *
 *====================================================================*/
#include "util.h"
#define SAVED_BLANK ((char) -1)

/* Save embedded blanks inside quoted strings */

static VOID SaveBlanks(char *input, char quote)
{
char *ptr;
int nquote = 0;

    if (quote != 0) {
        for (ptr = input; *ptr != (char) 0; ptr++) {
            if (*ptr == quote) {
                if (++nquote == 2) nquote = 0;
            } else {
                if (nquote == 1 && *ptr == ' ') *ptr = SAVED_BLANK;
            }
        }
    }
}

/* Restore blanks which were saved with SaveBlanks */

static VOID RestoreBlanks(char *input)
{
char *ptr;

    for (ptr = input; *ptr != (char) 0; ptr++) if (*ptr == SAVED_BLANK) *ptr = ' ';
}

/* Parse a string into a linked list of tokens, leaving input untouched */

LNKLST *utilStringTokenList(char *input, char *delimiters, char quote)
{
LNKLST *list;
char *copy, *token, *s1, *lasts;

    if ((list = listCreate()) == NULL) return NULL;

    if ((copy = strdup(input)) == NULL) {
        free(list);
        return NULL;
    }
    SaveBlanks(copy, quote);
    
    s1 = copy;
    while ((token = (char *) strtok_r(s1, delimiters, &lasts)) != NULL) {
        s1 = NULL;
        RestoreBlanks(token);
        if (!listAppend(list, token, strlen(token)+1)) {
            free(copy);
            listDestroy(list);
            return FALSE;
        }
    }

    free(copy);
    return list;
}

/* Parse a string into a user supplied array */

int utilParse(
    char *input,      /* input string                */
    char **argv,      /* output array to hold tokens */
    char *delimiters, /* token delimiters            */
    int  max_tokens,  /* max number of tokens (#elements in argv) */
    char quote        /* quote character for strings */
){
char *ptr, *lasts;
int i = 0, nquote = 0;

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

    if ((argv[0] = (char *) strtok_r(input, delimiters, &lasts)) == NULL) return 0;
    
    i = 1;
    do {
        if ((argv[i] = (char *) strtok_r(NULL, delimiters, &lasts)) != NULL && quote != 0) {
            for (ptr = argv[i]; *ptr != (char) 0; ptr++) {
                if (*ptr == (char) -1) *ptr = ' ';
            }
        }
    } while (argv[i] != NULL && ++i < max_tokens);

/* Return the number of tokens */

    return i;
}

int util_parse( char *input, char **argv, char *delimiters, int  max_tokens, char quote)
{
    return utilParse(input, argv, delimiters, max_tokens, quote);
}

int util_sparse( char *input, char *argv[], char *delimiters, int  max_tokens)
{
int i = 0;
char *lasts;

    if (max_tokens < 1) {
        fprintf(stderr,"sparse: illegal 'max_tokens'\n");
        return -1;
    }

    i = 0;
    if ((argv[i] = (char *) strtok_r(input, delimiters, &lasts)) == NULL) return 0;
    for (i = 1; i < max_tokens; i++) {
        if ((argv[i] = (char *) strtok_r(NULL, delimiters, &lasts)) == NULL) return i;
    }

    return i;
}

/* Revision History
 *
 * $Log: parse.c,v $
 * Revision 1.11  2006/02/14 17:05:30  dechavez
 * Change LIST to LNKLIST to avoid name clash with third party code
 *
 * Revision 1.10  2004/06/24 17:09:23  dechavez
 * removed unneccesary declaration (aap)
 *
 * Revision 1.9  2004/06/04 22:49:46  dechavez
 * various AAP windows portability modifications
 *
 * Revision 1.8  2003/11/19 23:29:46  dechavez
 * added explicit strtok_r declaration
 *
 * Revision 1.7  2003/11/13 19:53:59  dechavez
 * cast strtok_r to remove compiler complaints
 *
 * Revision 1.6  2003/11/13 19:37:55  dechavez
 * removed dead code, recast util_parse into call to utilParse()
 *
 * Revision 1.5  2003/10/16 15:45:10  dechavez
 * added utilStringTokenList(), switched to strtok_r to make other functions reentrant
 *
 * Revision 1.4  2003/06/10 00:37:26  dechavez
 * include util_sparse in the build
 *
 * Revision 1.3  2003/06/10 00:00:09  dechavez
 * added utilParse()
 *
 * Revision 1.2  2001/05/07 22:40:13  dec
 * ANSI function declarations
 *
 * Revision 1.1.1.1  2000/02/08 20:20:41  dec
 * import existing IDA/NRTS sources
 *
 */
