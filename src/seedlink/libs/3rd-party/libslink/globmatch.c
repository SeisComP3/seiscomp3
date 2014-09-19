/*
 * robust glob pattern matcher
 * ozan s. yigit/dec 1994
 * public domain
 *
 * glob patterns:
 *	*	matches zero or more characters
 *	?	matches any single character
 *	[set]	matches any character in the set
 *	[^set]	matches any character NOT in the set
 *		where a set is a group of characters or ranges. a range
 *		is written as two characters seperated with a hyphen: a-z denotes
 *		all characters between a to z inclusive.
 *	[-set]	set matches a literal hypen and any character in the set
 *	[]set]	matches a literal close bracket and any character in the set
 *
 *	char	matches itself except where char is '*' or '?' or '['
 *	\char	matches char, including any pattern character
 *
 * examples:
 *	a*c		ac abc abbc ...
 *	a?c		acc abc aXc ...
 *	a[a-z]c		aac abc acc ...
 *	a[-a-z]c	a-c aac abc ...
 *
 * $Log: globmatch.c,v $
 * Revision 1.1  2007/10/09 00:47:17  chad
 * add extended reply message handling
 *
 * Revision 1.4sl  2007/06/18  12:47:00  ct
 * Rename globmatch to sl_globmatch for integration into libslink.
 *
 * Revision 1.4  2004/12/26  12:38:00  ct
 * Changed function name (amatch -> globmatch), variables and
 * formatting for clarity.  Also add matching header globmatch.h.
 *
 * Revision 1.3  1995/09/14  23:24:23  oz
 * removed boring test/main code.
 *
 * Revision 1.2  94/12/11  10:38:15  oz
 * charset code fixed. it is now robust and interprets all
 * variations of charset [i think] correctly, including [z-a] etc.
 * 
 * Revision 1.1  94/12/08  12:45:23  oz
 * Initial revision
 */


#include "globmatch.h"

#define SL_GLOBMATCH_TRUE    1
#define SL_GLOBMATCH_FALSE   0


/***********************************************************************
 * sl_globmatch:
 *
 * Check if a string matches a globbing pattern.
 *
 * Return 0 if string does not match pattern and non-zero otherwise.
 **********************************************************************/
int
sl_globmatch (char *string, char *pattern)
{
  int negate;
  int match;
  int c;
  
  while ( *pattern )
    {
      if ( !*string && *pattern != '*' )
	return SL_GLOBMATCH_FALSE;
      
      switch ( c = *pattern++ )
	{
	  
	case '*':
	  while ( *pattern == '*' )
	    pattern++;
	  
	  if ( !*pattern )
	    return SL_GLOBMATCH_TRUE;
	  
	  if ( *pattern != '?' && *pattern != '[' && *pattern != '\\' )
	    while ( *string && *pattern != *string )
	      string++;
	  
	  while ( *string )
	    {
	      if ( sl_globmatch(string, pattern) )
		return SL_GLOBMATCH_TRUE;
	      string++;
	    }
	  return SL_GLOBMATCH_FALSE;
	  
	case '?':
	  if ( *string )
	    break;
	  return SL_GLOBMATCH_FALSE;
	  
	  /* set specification is inclusive, that is [a-z] is a, z and
	   * everything in between. this means [z-a] may be interpreted
	   * as a set that contains z, a and nothing in between.
	   */
	case '[':
	  if ( *pattern != SL_GLOBMATCH_NEGATE )
	    negate = SL_GLOBMATCH_FALSE;
	  else
	    {
	      negate = SL_GLOBMATCH_TRUE;
	      pattern++;
	    }
	  
	  match = SL_GLOBMATCH_FALSE;
	  
	  while ( !match && (c = *pattern++) )
	    {
	      if ( !*pattern )
		return SL_GLOBMATCH_FALSE;
	      
	      if ( *pattern == '-' ) 	/* c-c */
		{
		  if ( !*++pattern )
		    return SL_GLOBMATCH_FALSE;
		  if ( *pattern != ']' )
		    {
		      if ( *string == c || *string == *pattern ||
			   ( *string > c && *string < *pattern ) )
			match = SL_GLOBMATCH_TRUE;
		    }
		  else
		    {		/* c-] */
		      if ( *string >= c )
			match = SL_GLOBMATCH_TRUE;
		      break;
		    }
		}
	      else			/* cc or c] */
		{
		  if ( c == *string )
		    match = SL_GLOBMATCH_TRUE;
		  if ( *pattern != ']' )
		    {
		      if ( *pattern == *string )
			match = SL_GLOBMATCH_TRUE;
		    }
		  else
		    break;
		}
	    } 
	  
	  if ( negate == match )
	    return SL_GLOBMATCH_FALSE;
	  
	  /*
	   * if there is a match, skip past the charset and continue on
	   */
	  while ( *pattern && *pattern != ']' )
	    pattern++;
	  if ( !*pattern++ )	/* oops! */
	    return SL_GLOBMATCH_FALSE;
	  break;
	  
	case '\\':
	  if ( *pattern )
	    c = *pattern++;
	default:
	  if ( c != *string )
	    return SL_GLOBMATCH_FALSE;
	  break;
	}
      
      string++;
    }
  
  return !*string;
}
