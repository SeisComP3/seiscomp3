#pragma ident "$Id: version.h 165 2005-12-23 12:34:58Z andres $"
/* --------------------------------------------------------------------
 Program  : Any
 Task     : Archive Library API functions.
 File     : version.h
 Purpose  : da' version label.
 Host     : CC, GCC, Microsoft Visual C++ 5.x
 Target   : Solaris (Sparc and x86), Linux, Win32
 Author   : Robert Banfill (r.banfill@reftek.com)
 Company  : Refraction Technology, Inc.
            2626 Lombardy Lane, Suite 105
            Dallas, Texas  75220  USA
            (214) 353-0609 Voice, (214) 353-9659 Fax, info@reftek.com
 Copyright: (c) 1997-2005 Refraction Technology, Inc. - All Rights Reserved.
 Notes    :
 $Revision: 165 $
 $Logfile : R:/cpu68000/rt422/struct/version.h_v  $
 Revised  :
	2005-10-05 2.00 PLD promote version for easy recognition
	2005-09-26 1.17 PLD modifications to sample rate (float), support Steim2
	2004-12-16 1.15 PLD modifications to purge operations (purge.c)
	2002-01-16 1.15 DEC changed interpretation of unit ID from BCD to binary
	2000-03-22 1.13 RLB Fixed rate determination bug in archive API(read.c).
  	17 Aug 1998  ---- (RLB) First effort.

-------------------------------------------------------------------- */

#ifndef _VERSION_LABEL_
#define _VERSION_LABEL_

#define ARC_VERSION "RefTek Archive API 2.00"
#define ARC_COPYRIGHT "Copyright (c) 1998-2005, Refraction Technology, Inc. - All Rights Reserved."

#endif

/* Revision History
 *
 * $Log$
 * Revision 1.2  2005/12/23 12:34:57  andres
 * upgrade to new version with Steim2 support
 *
 * Revision 2.0  2005/10/07 21:30:20  pdavidson
 * Finish Steim2 support.

 * Bug fixes in 0.1 sps, aux data (stream 9) support.

 * Handle all trigger types in EH/ET decoding.

 * Promote archive API, modified programs to v2.0.

 * DOES NOT INCLUDE modifications to RTP log or client protocol.
 *
 * Revision 1.4  2004/12/16 21:55:52  pdavidson
 * New library version - work on purge.
 *
 * Revision 1.3  2002/01/18 17:53:22  nobody
 * changed interpretation of unit ID from BCD to binary
 *
 * Revision 1.2  2001/07/23 18:48:26  nobody
 * Added purge thread
 *
 * Revision 1.1.1.1  2000/06/22 19:13:09  nobody
 * Import existing sources into CVS
 *
 */
