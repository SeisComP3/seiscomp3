#pragma ident "$Id: si.c 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any needing SI units.
 Task     : Any
 File     : si.c
 Purpose  : SI unit utility functions.
 Host     : CC, GCC, Microsoft Visual C++ 5.x, MCC68K 3.1
 Target   : Solaris (Sparc and x86), Linux, DOS, Win32, and RTOS
 Author	  : Robert Banfill (r.banfill@reftek.com)
 Company  : Refraction Technology, Inc.
            2626 Lombardy Lane, Suite 105
            Dallas, Texas  75220  USA
            (214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
 Copyright: (c) 1997 Refraction Technology, Inc. - All Rights Reserved.
 Notes    :
 $Revision: 165 $
 $Logfile : R:/cpu68000/rt422/struct/version.h_v  $
 Revised  :
  17 Aug 1998  ---- (RLB) First effort.

-----------------------------------------------------------------------*/

#define _SI_C
#include <si.h>

/*---------------------------------------------------------------------*/
BOOL LookupBestSIUnit( REAL64 value, SI_UNIT *si ) {
    INT32 i;

    i = 0;
    while( _si_units[i].symbol[0] != '\0' ) {
        /* Find the largest factor < value */
        if( _si_units[i].factor <= value ) {
            memcpy( si, &_si_units[i], sizeof( SI_UNIT ) );
            return( TRUE );
        }
        i++;
    }

    return( FALSE );
}

/*---------------------------------------------------------------------*/
BOOL LookupBestSIBinaryUnit( INT64 value, SI_BIN_UNIT *si ) {
    INT32 i;

    i = 0;
    while( _si_bin_units[i].symbol[0] != '\0' ) {
        /* Find the largest factor < value */
        if( _si_bin_units[i].factor <= value ) {
            memcpy( si, &_si_bin_units[i], sizeof( SI_BIN_UNIT ) );
            return( TRUE );
        }
        i++;
    }

    return( FALSE );
}

/*---------------------------------------------------------------------*/
CHAR *FormatAsSIUnit( CHAR *string, REAL64 value, BOOL symbol ) {
    SI_UNIT si;

    if( value < 10.0 && value > 0.1 )
        sprintf( string, "%.3lf", value );
    else if( LookupBestSIUnit( value, &si ) ) 
        sprintf( string, "%.3lf %s", value / si.factor, (symbol ? si.symbol : si.prefix) );
    else
        sprintf( string, "%.3lE", value );

    return( string );
}

/*---------------------------------------------------------------------*/
CHAR *FormatAsSIBinaryUnit( CHAR *string, INT64 value, BOOL symbol ) {
    SI_BIN_UNIT si;

    if( LookupBestSIBinaryUnit( value, &si ) ) 
        sprintf( string, "%.2lf %s", (REAL64)value / (REAL64)si.factor, 
                 (symbol ? si.symbol : si.prefix) );
    else
#ifdef unix
        sprintf( string, "%lld", value );
#else
        sprintf( string, "%I64d", value );
#endif

    return( string );
}

/*---------------------------------------------------------------------*/
INT64 DecodeSIBinaryUnit( CHAR *string ) {
    CHAR *symbol;
    INT32 i;
    REAL64 value;

    /* Decode the value */
    sscanf( string, "%lf", &value );

    /* Find the symbol */
    symbol = string;
    while( *symbol && ! isalpha( *symbol ) )
        symbol++;

    if( symbol ) {
        /* Make sure its uppercase */
        if( ! isupper( *symbol ) )
            *symbol = (CHAR)toupper( *symbol );
        
        /* Find this symbol */
        i = 0;
        while( _si_bin_units[i].symbol[0] != '\0' ) {
            if( _si_bin_units[i].symbol[0] == *symbol ) {
                value *= (REAL64)_si_bin_units[i].factor;
                break;
            }
            i++;
        }
    }

    return( (INT64)(value + 0.5) );
}

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:58  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
