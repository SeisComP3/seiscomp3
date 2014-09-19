
/* NAME
 *	rdtttab -- Read travel-time tables
 
 * FILE
 *	rdtttab.c

 * SYNOPSIS
 *	Read travel-time and amplitude tables for individual phase-types

 * DESCRIPTION
 *	Function.  Establish file structures and read travel-time and 
 *	amplitude tables applicable for the given input phase-types. 
 *	Read travel-time tables from files into memory.  The filenames to
 *	be read have the form root.phase_type_ptr[k], where root is the 
 *	non-blank part of froot, and where phase_type_ptr[k] is a wave 
 *	identifier for k = 1, nwav; e.g., 'Pg', 'Pn', 'Sn', 'Lg'.  The 
 *	data read is put into the appropriate arrays as indicated by the 
 *	filename suffix.
 *	Example: If froot = 'tab' and phase_type_ptr = 'Pg', 'Lg', then this
 *		 routine reads files 'tab.Pg' and 'tab.Lg'.

 *	---- Indexing ----
 *	i = 1, ntbd[k];	j = 1, ntbz[k];	k = 1, nwav;
  
 *	---- On entry ----
 *	froot:		Root name of file to be read
 *	nwav:		Number of wave types to be used
 *	maxtbd:		Maximum number of distance samples in tables
 *	maxtbz:		Maximum number of depth samples in tables
 *	phase_type_ptr[k]: Character identifier of k'th wave
 
 *	---- On return ----
 *	ntbd[k]:	Number of distance samples in tables
 *	ntbz[k]:	Number of depth samples in tables
 *	tbd[i][k]:	Angular distance of the [i][j]'th sample (deg)
 *	tbz[j][k]:	Depth of the [i][j]'th sample (km)
 *	tbtt[i][j][k]:	Travel-time of the [i][j][k]'th sample (sec, sec/deg)
 
 * DIAGNOSTICS
 *	Will specify when any (or all) input files cannot be opened.

 * NOTE
 *	If file, filnam will not open, then arrays, ntbd[][] and ntbz[][] 
 *	are returned as zero.
 
 * AUTHOR
 *	Walter Nagy, January 1991
 */


#include <stdio.h>
#include <string.h>

#ifdef SCCSID
static	char	SccsId[] = "@(#)rdtttab.c	44.1	9/20/91";
#endif

/* Error report */
#define READ_E1(a)	{ fprintf (stderr, "\nERROR reading %s in File: %s\n", \
				(a), filnam); \
			}


rdtttab (froot, phase_type_ptr, nwav, maxtbd, maxtbz, ntbd, ntbz,
	 tbd, tbz, tbtt, ierr)

int	*ierr,				/* Error flag 			      */
	maxtbd, maxtbz, nwav,		/* Array lengths 		      */
	*ntbd, *ntbz;			/* Size [nwav]  		      */
float	*tbd,				/* Size [nwav][maxtbd] 		      */
	*tbz,				/* Size [nwav][maxtbz]	 	      */
	*tbtt;				/* Size [nwav][maxtbz][maxtbd]	      */
char	*froot,				/* Size [ca. 1024] 	 	 	      */
	**phase_type_ptr;		/* Size [nwav][8] 		      */

{
	int	i, j, k,		/* Loop counters		      */
		err,			/* Local error flag		      */
		ntbzx,			/* Number of depth samples to read    */
		ntbdx;			/* Number of distance samples to read */
	int	cnt, num_files;		/* File counters		      */
	float	tempval,		/* Temporary read before assign       */
		*temp_tbz,
		*temp_tbd,
		*temp_tt,
		*tflt;
	char	filnam[1024],	/* ["root" + "." + "postname" + NULL] */
		**s2ptr;		/* Temporary array pointer	      */
	char	readtemp[128];
	FILE	*opf, *fopen();


	/* Initialize */

	temp_tbz = (float *)tbz;
	temp_tbd = (float *)tbd;
	temp_tt  = (float *)tbtt;

	cnt = 0; num_files = 0;		/* Counters */


	/* Read the files */

	for (err = 0, s2ptr = phase_type_ptr, k = 0; k < nwav && !err; ++k)
	{
		strcpy (filnam, froot);
		strcat (filnam, ".");
		
		strcat (filnam, *s2ptr);
		
		/** HACK to avoid problems with filesystems not able to handle upper/lower case filenames 
		**  e.g. Microsofts NTFS or HFS+ on Mac's
		*/
		if (strcmp(*s2ptr, "pP") == 0)
			strcat (filnam, "_");

		s2ptr++;

		/* Open files */
 
		if ((opf = fopen (filnam, "r")) == NULL)
		{
			fprintf (stderr, "\nFile %s will not open!\n", filnam);
			ntbd[k] = 0; ntbz[k] = 0;
			temp_tbz += maxtbz;
			temp_tbd += maxtbd;
			temp_tt += maxtbd * maxtbz;
			continue;
		}
		cnt++;			/* Successful open counter */


		/* Begin reading info -- Start with depth sampling */
 
		if (fscanf (opf, "%*[^\n]\n%d%*[^\n]", &ntbzx) != 1)
		{
			READ_E1("number of depth samples");
			err = 2;
		}


		if (!err && (ntbz[k]=ntbzx) > maxtbz)
		{
			fprintf (stderr, "%s %s, %d used, %d skipped\n",
				 "\nToo many depth samples in file",
				 filnam, maxtbz, ntbzx-maxtbz);
			ntbz[k] = maxtbz;
		}

		for (tflt = temp_tbz, i = 0; i < ntbzx && !err; ++i)
		{
			if (fscanf (opf, "%f", &tempval) != 1)
			{
				READ_E1("depth sample value");
				err = 2;
			}

			/* Skip, if necessary */
			if (i < ntbz[k] ) *tflt++ = tempval;
		}
		temp_tbz += maxtbz;

 
		/* Read distance sampling */
 
		if (fscanf (opf, "%d%*[^\n]", &ntbdx) != 1)
		{
			READ_E1("number of distance samples");
			err = 2;
		}

		if (!err && (ntbd[k]=ntbdx) > maxtbd)
		{
			fprintf (stderr, "%s %s, %d used, %d skipped\n",
				 "\nToo many distance samples in file",
				 filnam, maxtbd, ntbdx-maxtbd);
			ntbd[k] = maxtbd;
		}

		for (tflt = temp_tbd, i = 0; i < ntbdx && !err; ++i)
		{
			if (fscanf (opf, "%f", &tempval) != 1)
			{
				READ_E1("distance sample value");
				err = 2;
			}

			/* Skip, if necessary */
			if (i < ntbd[k]) *tflt++ = tempval;
		}
		temp_tbd += maxtbd;


		/* Read travel-time tables */
 
		for (j = 0 ; j < ntbzx && !err; j++ )
		{
			/* skip the comment line */
			while (getc(opf) != '#');
			while (getc(opf) != '\n');

			for (tflt = temp_tt, i = 0; i < ntbdx & !err; ++i)
			{
				fgets(readtemp, 127, opf);
				if (sscanf (readtemp, "%f", &tempval) != 1)
				{
					READ_E1("travel-time value");
					err = 2;
				}

				/* Skip, if necessary */
				if (j < ntbd[k] ) *tflt++ = tempval;
			}
			temp_tt += maxtbd;
		}

		if (j < maxtbz ) temp_tt += ((maxtbz - j) * maxtbd);
 
		num_files++;
		fclose(opf);
	}


	if (num_files == 0 && cnt == 0)	/* Error: No tables found */
	{
		fprintf (stderr, "\nrdtttab: No tables can be read\n");
		err = 1;
	}

	*ierr = err;	
} 
