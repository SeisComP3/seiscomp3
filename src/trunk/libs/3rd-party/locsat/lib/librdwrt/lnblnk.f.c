/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"
#define ftnlen int

/* file lnblnk.f */
/*      ======== */

/* version 1, 17-Nov-98 */

/* missing function lnblnk */
/* K. Stammler, 17-Nov-98 */
/* ----------------------------------------------------------------------------- */
integer lnblnk_(lin, lin_len)
char *lin;
ftnlen lin_len;
{
    /* System generated locals */
    int ret_val;

    /* Builtin functions */
    int i_len();

    /* Local variables */
    static int i__;

/*     returns length of string not counting trailing blanks */
/*     parameters of routine */
/*     local variables */
/*     functions */
/* input string */
/*     executable code */
/* counter */
    i__ = i_len(lin, lin_len);
L1234:
    if (*(unsigned char *)&lin[i__ - 1] != ' ') {
	goto L1235;
    }
    --i__;
    if (i__ == 0) {
	goto L1235;
    }
    goto L1234;
L1235:
    ret_val = i__;
    return ret_val;
} /* lnblnk_ */

