#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
Replaces the routine with the same name and its entry, emdld, in the
IASPEI91 package.  This version allows introduction of other models
to prodouce tables.  REMODL calls emdld first.  Here it opens a file
containing the model (ascii).  The first two lines are comment lines
for the P and S models respectively.  The remaining lines are z, vp,
vs, density per line in free format starting from the surface down.
(Density is not used and could be left out.)  First order discontinuities
are included by repeated z values.  At this stage the user is prompted
for the model name and the filespec.  The calls to emdlv use linear
interpolation to get the vp and vs for the desired r.  R0=6371.0 is
assumed.  Limit is 200 layers and a maximum of 30 discontinuities.
*/


#define NMAX  200
#define NPMAX 30

#define True  1
#define False 0


static float zin[NMAX], vpin[NMAX], vsin[NMAX], rd[NPMAX];
static int nz = 0;


int emdlv(float r, float *vp, float *vs) {
	float depth = 6371.0 - r;
	int ldep = False;
	int i = 0;

	if ( !nz ) return 1;

	if ( depth < 0 ) depth = 0;

	while ( !ldep && i < nz ) {
		if ( zin[i] <= depth ) {
			if ( zin[i] == depth) {
				*vp = vpin[i];
				*vs = vsin[i];
				return 0;
			}

			++i;
		}
		else
			ldep = True;
	}

	if ( ldep ) {
		*vp = vpin[i-1]+(vpin[i]-vpin[i-1])*(depth-zin[i-1])/(zin[i]-zin[i-1]);
		*vs = vsin[i-1]+(vsin[i]-vsin[i-1])*(depth-zin[i-1])/(zin[i]-zin[i-1]);
	}
	else {
		*vp = vpin[nz-1];
		*vs = vsin[nz-1];
	}

	return 0;
}


int emdld(int *n, float *cpr, const char *name, const char *path) {
	FILE *fp;
	char *line = NULL;
	size_t len;
	int lc = 0;
	int np, i;
	char filespec[1200];

	strcpy(filespec, path);
	if ( strlen(filespec) > 0 ) {
		if (filespec[strlen(filespec)-1] != '/' )
			strcat(filespec, "/");
	}

	strcat(filespec, name);
	strcat(filespec, ".tvel");

	fp = fopen(filespec, "r");
	if ( fp == NULL ) {
		*n = 0;
		return 1;
	}

	/* Skip the first two lines */
	getline(&line, &len, fp);
	getline(&line, &len, fp);

	while ( getline(&line, &len, fp) > 0 ) {
		if ( sscanf(line, "%f %f %f", zin+lc, vpin+lc, vsin+lc) == 3 ) {
			++lc;
			nz = lc;
		}
	}

	fclose(fp);
	if ( line != NULL ) free(line);

	/* now for the discontinuities */
	np = 0;
	i = nz-2;
	while ( i >= 1 ) {
		if ( zin[i] == zin[i+1] ) {
			rd[np] = 6371.0 - zin[i];
			++np;
		}
		--i;
	}

	rd[np] = 6371.0;
	++np;

	*n = np;

	for ( i = 0; i < np; ++i )
		cpr[i] = rd[i];

	return 0;
}
