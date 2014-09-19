#pragma ident "$Id: si.h 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any needing SI units.
 Task     : Any
 File     : si.h
 Purpose  : Define the SI units, symbols, and names.
 Host     : CC, GCC, Microsoft Visual C++ 5.x
 Target   : Solaris (Sparc and x86), Linux, Win32
 Author   : Robert Banfill (r.banfill@reftek.com)
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

-------------------------------------------------------------------- */

#ifndef _SI_H_INCLUDED_
#define _SI_H_INCLUDED_

/* Includes -----------------------------------------------------------*/
#include <platform.h>

/* Constants ----------------------------------------------------------*/

/* The 20 standard SI prefix unit factors */
#define SI_YOCTO    1.0e-24
#define SI_ZEPTO    1.0e-21
#define SI_ATTO     1.0e-18
#define SI_FEMTO    1.0e-15
#define SI_PICO     1.0e-12
#define SI_NANO     1.0e-9 
#define SI_MICRO    1.0e-6 
#define SI_MILLI    1.0e-3 
#define SI_CENTI    1.0e-2 
#define SI_DECI     1.0e-1 
#define SI_DEKA     1.0e+1 
#define SI_HECTO    1.0e+2 
#define SI_KILO     1.0e+3 
#define SI_MEGA     1.0e+6 
#define SI_GIGA     1.0e+9 
#define SI_TERA     1.0e+12
#define SI_PETA     1.0e+15
#define SI_EXA      1.0e+18
#define SI_ZETTA    1.0e+21
#define SI_YOTTA    1.0e+24

/* SI binary unit factors (IEEE-ISO-IEC proposed prefixes for IT) */
#define SI_KIBI     1024                /* 2^10 */
#define SI_MEBI     1048576             /* 2^20 */
#define SI_GIBI     1073741824          /* 2^30 */
#define SI_TEBI     1099511627776       /* 2^40 */
#define SI_PEBI     1125899906842624    /* 2^50 */
#define SI_EXBI     1152921504606846976 /* 2^60 */

/* Types --------------------------------------------------------------*/
typedef struct _SI_UNIT {
    REAL64 factor;
    CHAR *symbol;
    CHAR *prefix;
} SI_UNIT;

typedef struct _SI_BIN_UNIT {
    INT64 factor;
    CHAR *symbol;
    CHAR *prefix;
} SI_BIN_UNIT;

/* Globals ------------------------------------------------------------*/
#ifdef _SI_C
SI_UNIT _si_units[] = {
    { SI_YOTTA, "Y",  "yotta" },
    { SI_ZETTA, "Z",  "zetta" },
    { SI_EXA,   "E",  "exa"   },
    { SI_PETA,  "P",  "peta"  },
    { SI_TERA,  "T",  "tera"  },
    { SI_GIGA,  "G",  "giga"  },
    { SI_MEGA,  "M",  "mega"  },
    { SI_KILO,  "k",  "kilo"  },
    { SI_HECTO, "h",  "hecto" },
    { SI_DEKA,  "da", "deka"  },
    { SI_DECI,  "d",  "deci"  },
    { SI_CENTI, "c",  "centi" },
    { SI_MILLI, "m",  "milli" },
    { SI_MICRO, "u",  "micro" },
    { SI_NANO,  "n",  "nano"  },
    { SI_PICO,  "p",  "pico"  },
    { SI_FEMTO, "f",  "femto" },
    { SI_ATTO,  "a",  "atto"  },
    { SI_ZEPTO, "z",  "zepto" },
    { SI_YOCTO, "y",  "yocto" },
    { 0.0,      "",   "" }
};
SI_BIN_UNIT _si_bin_units[] = {
    { SI_EXBI, "Ei",  "exbi"  },
    { SI_PEBI, "Pi",  "pebi"  },
    { SI_TEBI, "Ti",  "tebi"  },
    { SI_GIBI, "Gi",  "gibi"  },
    { SI_MEBI, "Mi",  "mebi"  },
    { SI_KIBI, "Ki",  "kibi"  },
    { 0,       "",   "" }
};
#else
extern SI_UNIT _si_units[];
extern SI_BIN_UNIT _si_bin_units[];
#endif

/* Prototypes ---------------------------------------------------------*/
BOOL LookupBestSIUnit( REAL64 value, SI_UNIT *si );
BOOL LookupBestSIBinaryUnit( INT64 value, SI_BIN_UNIT *si );

/* Note: string should be at least 32 bytes, symbol specifies symbol or name appended */
CHAR *FormatAsSIUnit( CHAR *string, REAL64 value, BOOL symbol );
CHAR *FormatAsSIBinaryUnit( CHAR *string, INT64 value, BOOL symbol );
INT64 DecodeSIBinaryUnit( CHAR *string );

#endif  /*_SI_H_INCLUDED_*/

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
