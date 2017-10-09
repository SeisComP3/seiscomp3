/*  -- translated by f2c (version 20000121).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)
*/

#include "f2c.h"
#define ftnlen int

/* Common Block Declarations */

struct sccsclitok_1_ {
    char sccsid[80];
};

#define sccsclitok_1 (*(struct sccsclitok_1_ *) &sccsclitok_)

/* Initialized data */

struct {
    char e_1[80];
    } sccsclitok_ = { {'@', '(', '#', ')', 'c', 'l', 'i', 't', 'o', 'k', '.', 
	    'f', '\t', '4', '1', '.', '1', '\t', '1', '2', '/', '2', '1', '/',
	     '9', '0', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 
	    ' ', ' ', ' ', ' '} };


/* Subroutine */ int clitok_(list, itok, tok, ilist, list_len, tok_len)
char *list;
int *itok;
char *tok;
int *ilist;
ftnlen list_len;
ftnlen tok_len;
{
    /* Builtin functions */
    int i_len();
    /* Subroutine */ int s_copy();
    int i_indx();

    /* Local variables */
    static int lenl, i__, jl, kl, it;


/* TOK = ITOK'th token in LIST. */
/* ILIST = position in LIST of the last charater in TOK. */

/* If there are fewer than ITOK tokens in LIST, then the values */
/*  returned are TOK = ' ' and ILIST = 0. */
/* If necessary, the ITOK'th token in LIST is truncated to fit in TOK. */


/* Initialize. */

    lenl = i_len(list, list_len);
    it = 0;
    jl = 0;

/* Find start of next token (skip over blanks). */
/* Return if there is none, otherwise count it. */

L10:
    ++jl;
    if (jl > lenl) {
	s_copy(tok, " ", tok_len, (ftnlen)1);
	*ilist = 0;
	return 0;
    } else if (*(unsigned char *)&list[jl - 1] == ' ') {
	goto L10;
    }
    ++it;

/* Find end of token (search for blank). */

    i__ = i_indx(list + (jl - 1), " ", list_len - (jl - 1), (ftnlen)1);
    if (i__ == 0) {
	kl = lenl;
    } else {
	kl = jl + i__ - 1;
    }

/* Okay, it'th token of list is in list(jl:kl). */
/* Either grab it or go back for another. */

    if (it == *itok) {
	s_copy(tok, list + (jl - 1), tok_len, kl - (jl - 1));
	*ilist = kl;
	return 0;
    } else {
	jl = kl;
	goto L10;
    }

} /* clitok_ */

