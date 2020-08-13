/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/



#ifndef __SEISCOMP3_SEISMOLOGY_LOCSAT_TYPES_H__
#define __SEISCOMP3_SEISMOLOGY_LOCSAT_TYPES_H__

//#include "params.h"
#define MAXSTA 	 9999	/* Max. number of stations allowed		      */
#define MAXWAV 	 40	/* Max. number of waves per station allowed	      */
#define MAXTYP 	 3	/* Max. number of different station correction types  */
#define MAXDATA	 9999	/* Max. number of data allowed in a single location   */
#define MAXTBD 	 181	/* Max. number of distance samples in T-T tables      */
#define MAXTBZ 	 20	/* Max. number of depth samples in travel-time tables */
#define luout 	 17	/* Verbose output unit number			      */

//#include "aesir.h"
#define AESIR_VERSION		"3.2"	/* string for printing */

#ifdef Bool
#undef Bool
#endif
typedef int	Bool;

#ifndef NULL
#define NULL	0
#endif

#ifndef EOF
#define EOF	-1
#endif

#ifndef FALSE
#define FALSE		0
#define TRUE		1
#endif

#ifndef False
#define False		0
#define True		1
#endif

#define REG		register
#define EOS		'\0'		/* end of string */
#define EOL		'\n'		/* end of line */
#define EOP		'\14'		/* end of page (form feed) */

#define FILENAMELEN 1024		/* longest file name */
#define	MAXHOSTNAME	256
#define MAXPATH		FILENAMELEN	/* longest file name */
#define NBITS		8		/* number of bits in a byte */
#define BUFLENGTH	256		/* convenient size for getting input */
#define	ID_SIZE		32
#define	PPOID_SIZE	32
#define LPOID_SIZE	PPOID_SIZE
#define MAX_MESSSIZE	1024
#define AESIR_main	"AESIR_main"
#define	AESIR_dispatch	"AESIR_dispatch"
#define LONGTIME	(60*60*24*7)	/* seven days */

/* aesir library routines */
extern char	*gethost ();
extern int	d_open ();
extern char	*d_send ();
extern char	*d_listen ();
extern int	d_getfd ();

/* a sub-second sleep */
extern void	nap();

#define UALLOC(type, count)	(type *) malloc ((unsigned) (count) * (sizeof (type)))
#define UALLOCA(type, count)	(type *) alloca (count * sizeof (type))
#define UREALLOC(ptr,type,count) (type *) REALLOC ((char *)ptr, (unsigned) sizeof (type) * count)
#define STRALLOC(string)	strcpy (UALLOC (char, strlen (string)+1), string)
#define STRALLOCA(string)	strcpy (UALLOCA (char, strlen (string)+1), string)
#define UFREE(ptr)		if (!(ptr));\
				else {\
					(void) free ((char *) (ptr));\
					(ptr) = 0;\
				}

/* Return the number of elements in an array. */
#define DIM(ar)		(sizeof (ar) / sizeof (*(ar)))
/* Are two strings equal? */
#define STREQ(a,b)		(strcmp ((a), (b)) == 0)


//#include "loc_params.h"
struct Locator_params {
	Locator_params() {
		outfile_name = prefix = NULL;
	}

	~Locator_params() {
		if ( outfile_name )
			delete [] outfile_name;
		if ( prefix )
			delete [] prefix;
	}

	/* DEFAULT - DESCRIPTION                     */
	int     num_dof;         /* 9999    - number of degrees of freedom    */
	float   est_std_error;   /* 1.0     - estimate of data std error      */
	float   conf_level;      /* 0.9     - confidence level    	     */
	float   damp;            /* -1.0    - damping (-1.0 means no damping) */
	int     max_iterations;  /* 20      - limit iterations to convergence */
	char    fix_depth;       /* true    - use fixed depth ?               */
	float   fixing_depth;    /* 0.0     - fixing depth value              */
	float   lat_init;        /* modifiable - initial latitude             */
	float   lon_init;        /* modifiable - initial longitude            */
	float   depth_init;      /* modifiable - initial depth                */
	int     use_location;    /* true    - use current origin data ?       */
	char    verbose;         /* true    - verbose output of data ?        */
	int     cor_level;       /* 0       - correction table level          */
	char   *outfile_name;    /* NULL    - name of file to print data      */
	char   *prefix;          /* NULL    - dir name & prefix of tt tables  */
};


struct Locator_errors {
	int arid;
	int time;
	int az;
	int slow;
};


#include "db3/db_arrival.h"
#include "db3/db_assoc.h"
#include "db3/db_origerr.h"
#include "db3/db_origin.h"
#include "db3/db_site.h"

#include "css/csstime.h"


extern "C" {
	void mdtodate(struct date_time*);
	void htoe(struct date_time*);
	void etoh(struct date_time*);
	int locate_event(char* _newnet,
	                 Site* _sites,
	                 int _num_sta,
	                 Arrival* _arrival,
	                 Assoc* _assoc,
	                 Origin* _origin,
	                 Origerr* _origerr,
	                 Locator_params* _locator_params,
	                 Locator_errors* _locator_errors,
	                 int _num_obs);
}

struct Loc {
	Arrival           *arrival;
	Assoc             *assoc;
	Origerr           *origerr;
	Origin            *origin;
	Site              *sites;
	Locator_params    *locator_params;
	Locator_errors    *locator_errors;
	struct date_time  *dt;
	int                siteCount;    // num_sta
	int                arrivalCount; // num_obs
	int                assocCount;   // = siteCount
	char              *newnet;
};


#endif
