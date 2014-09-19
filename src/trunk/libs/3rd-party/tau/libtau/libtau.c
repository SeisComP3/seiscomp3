#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "tau.h"

#define True	1
#define False	0
#define mod(i,j) (i-(int)((i)/(j))*(j))


static void	depcor(libtau *h, int);
static void make_tau(libtau *h, int nph, int mu, double umin, double dtol, double *tauus1, double *tauus2, double *xus1, double *xus2);
static void findtt(libtau *h, int jb, double *x0, int *pn, float *tt, float *dtdd, float *dtdh, float *dddp, float *ray_p, char **phnm);
static void pdecu(libtau *h, int i1, int i2, double x0, double x1, double xmin, int intt, int *len);
static void r4sort(int n, float *rkey, int *iptr);
static void fitspl(int i1, int i2, c1c2c3c4 *tau, double x1, double xn, double *c1, double *c2, double *c3, double *c4, double *c5);
static double umod(libtau *h, double zs, int *src, int nph);
static double zmod(libtau *h, double uend, int js, int nph);
static char *xindex(char *a, char *b);
static void spfit(libtau *h, int jb, int intt);
void tauint(double, double, double, double, double, double*, double*);
void tauspl(int, int, double*, double*, double*, double*, double*, double*);
int emdld(int*, float*, const char*, const char*);


int tabin(libtau *h, const char *model) {
	int i, j, k, l, ind, nasgr, nl, len2;
	char *modl;
	char phdif[6][10];
/*
fp10 = fopen("ttim1.lis", "wb");
*/

	/* Init handle */
	h->allocated = 0;
	h->ka = 4;
	h->deplim = 1.1;
	h->D = 0;
	h->fpin = NULL;

	if(model == NULL)
	{
		return(-3);
	}
	modl = (char *)malloc(strlen(model) + 5);

	for(i = 0; i < jbrn; i++) h->depths[0].phcd[i] = h->depths[0].phcd_buf+i*10;
	for(i = 0; i < jtsm; i++) h->depths[0].tauc[i] = 0.;
	for(i = 0; i < jxsm; i++) h->depths[0].xc[i] = 0.;
	h->depths[0].nph0 = -1;
	for(i = 0; i < jseg; i++) h->segmsk[i] = True;

	strcpy(&phdif[0][0], "P");
	strcpy(&phdif[1][0], "S");
	strcpy(&phdif[2][0], "pP");
	strcpy(&phdif[3][0], "sP");
	strcpy(&phdif[4][0], "pS");
	strcpy(&phdif[5][0], "sS");

	strcpy(modl, model);
	strcat(modl, ".hed");
	if((h->fpin = fopen(modl, "rb")) == NULL) /* iasp91.hed */
	{
		free(modl);
		return(-1);
	}
	
	fread(&nasgr, 4, 1, h->fpin); fread(&nl, 4, 1, h->fpin);
	fread(&len2, 4, 1, h->fpin); fread(&h->xn, 4, 1, h->fpin);
	fread(&h->pn, 4, 1, h->fpin); fread(&h->tn, 4, 1, h->fpin);
	fread(h->mt, 4, 2, h->fpin); fread(&h->nseg, 4, 1, h->fpin);
	fread(&h->nbrn, 4, 1, h->fpin); fread(h->ku, 4, 2, h->fpin);
	fread(h->km, 4, 2, h->fpin); fread(h->s, sizeof(struct s_struct), 3, h->fpin);
	fread(h->v[0].indx, 4, jseg, h->fpin); fread(h->v[1].indx, 4, jseg, h->fpin);
	fread(h->v[0].kndx, 4, jseg, h->fpin); fread(h->v[1].kndx, 4, jseg, h->fpin);
	fread(h->v[0].pm, 8, jsrc, h->fpin); fread(h->v[1].pm, 8, jsrc, h->fpin);
	fread(h->v[0].zm, 8, jsrc, h->fpin); fread(h->v[1].zm, 8, jsrc, h->fpin);
	fread(h->v[0].ndex, 4, jsrc, h->fpin); fread(h->v[1].ndex, 4, jsrc, h->fpin);
	fread(h->v[0].loc, 4, jsrc, h->fpin); fread(h->v[1].loc, 4, jsrc, h->fpin);
	fread(h->v[0].pu, 8, jtsm0, h->fpin); fread(h->v[1].pu, 8, jtsm0, h->fpin);
	fread(h->v[0].pux, 8, jbrn, h->fpin); fread(h->v[1].pux, 8, jbrn, h->fpin);

	for(i = 0; i < jbrn; i++)
	{
		fread(h->depths[0].phcd[i], 8, 1, h->fpin);
		/*strcpy(depths[1].phcd, depths[0].phcd[i]);*/
	}
	fread(h->v[0].px, 8, jbrn, h->fpin); fread(h->v[1].px, 8, jbrn, h->fpin);
	fread(h->v[0].xt, 8, jbrn, h->fpin); fread(h->v[1].xt, 8, jbrn, h->fpin);
	fread(h->depths[0].w[0].jndx, 4, jbrn, h->fpin);
	fread(h->depths[0].w[1].jndx, 4, jbrn, h->fpin);
	fread(h->depths[0].pt, 8, jout, h->fpin);
	fread(h->taut, 8, jout, h->fpin);
	fread(h->c1, 8, jout, h->fpin); fread(h->c2, 8, jout, h->fpin);
	fread(h->c3, 8, jout, h->fpin); fread(h->c4, 8, jout, h->fpin);
	fread(h->c5, 8, jout, h->fpin);
	fclose(h->fpin);

	strcpy(modl, model);
	strcat(modl, ".tbl");
	if((h->fpin = fopen(modl, "rb")) == NULL) /* iasp91.tbl */
	{
		free(modl);
		return(-2);
	}

	h->v[0].pu[h->ku[0]] = h->v[0].pm[0];
	h->v[1].pu[h->ku[1]] = h->v[1].pm[0];

/*
	fprintf(fp10, "nasgr nl len2 %d %d %d\n",nasgr,nl,len2);
	fprintf(fp10, "nseg nbrn mt ku km %d %d %d %d %d %d %d %d\n",
			nseg, nbrn, mt[0], mt[1], ku[0], ku[1], km[0], km[1]);
	fprintf(fp10, "xn pn tn %e %e %e\n\n", xn, pn, tn);
	for(i = 0; i < mt[1]; i++)
		fprintf(fp10, " %3d%7d%12.6f%12.6f%7d%12.6f%12.6f\n", i,
			v[0].ndex[i], (float)v[0].pm[i], (float)v[0].zm[i],
			v[1].ndex[i], (float)v[1].pm[i], (float)v[1].zm[i]);
	fprintf(fp10, "\n");
	for(i = 0; i < ku[1]+1; i++)
		fprintf(fp10, " %3d%12.6f%12.6f\n", i, (float)v[0].pu[i],
						       (float)v[1].pu[i]);
	fprintf(fp10, "\n");
	for(i = 0; i < km[1]; i++)
		fprintf(fp10, " %3d%12.6f%12.6f\n", i, (float)v[0].pux[i],
						       (float)v[1].pux[i]);
	fprintf(fp10, "\n");
	for(i = 0; i < nseg; i++)
		fprintf(fp10, " %3d%5d%5d%5d%5d%5d%5d%5d%5.0f%5.0f%5.0f\n",
			i, s[0].nafl[i], s[1].nafl[i], s[2].nafl[i],
			v[0].indx[i], v[1].indx[i], v[0].kndx[i], v[1].kndx[i],
			s[0].fcs[i], s[1].fcs[i], s[2].fcs[i]);
	dn = 180./M_PI;
	fprintf(fp10, "\n");
	for(i = 0; i < nbrn; i++)
		fprintf(fp10, " %3d%5d%5d%12.6f%12.6f%12.2f%12.2f  %s\n",
			i, depths[0].w[0].jndx[i], depths[0].w[1].jndx[i],
			(float)v[0].px[i], (float)v[1].px[i],
			(float)(dn*v[0].xt[i]), (float)(dn*v[1].xt[i]),
			depths[0].phcd[i]);
	fprintf(fp10, "\n");
	for(i = 0; i < jout; i++)
		fprintf(fp10,
			"%5d%12.6f%12.6f%10.2E%10.2E%10.2E%10.2E%10.2E\n", i,
			(float)depths[0].pt[i], (float)taut[i], c1[i], c2[i],
			c3[i], c4[i], c5[i]);
*/
	
	h->tn = 1./h->tn;
	h->dn =M_PI/(180.*h->pn*h->xn);
	h->depths[0].odep = -1.;
	h->depths[0].ki = -1;
	h->depths[0].msrc[0] = -1;
	h->depths[0].msrc[1] = -1;

	for(i = k = 0; i < h->nbrn; i++)
	{
		h->jidx[i] = h->depths[0].w[1].jndx[i];
		h->depths[0].w[0].dbrn[i] = -1.;
		h->depths[0].w[1].dbrn[i] = -1.;
		while(h->depths[0].w[1].jndx[i] > h->v[1].indx[k]) k++;
		if(h->s[1].nafl[k] == 0)
		{
			ind = h->s[0].nafl[k]-1;
			for(j = h->depths[0].w[0].jndx[i], l=0;
				j <= h->depths[0].w[1].jndx[i]; j++, l++)
					h->v[ind].tp[l] = h->depths[0].pt[j];
		}
		if(h->s[0].nafl[k] <= 0 || (h->depths[0].phcd[i][0]!='P' &&
			h->depths[0].phcd[i][0]!='S'))
		{
			for(j = 0; j < 6; j++)
				if(!strcmp(h->depths[0].phcd[i], &phdif[j][0]))
			{
 				h->depths[0].w[0].dbrn[i] = 1.0;
				phdif[j][0] = '\0';
				break;
			}

		}
	}

	/* depth[1] is used as backup when setting a new depth to be copied back
	   to depth[0] */
	memcpy(&h->depths[1], &h->depths[0], sizeof(struct Depth));

	free(modl);

	emdld(&h->np, h->rd, model, "");

	return(0);
}

int tabout(libtau *h) {
	int i;

	if ( !h->allocated ) {
		for ( i = 0; i < jseg; i++ ) free(h->phlst[i]);
		for ( i = 0; i < jbrn; i++ ) free(h->segcd[i]);
		h->allocated = 0;
	}

	if(h->fpin)
		return fclose(h->fpin);

	return 0;
}

void brnset(libtau *h, const char *branch)
{
/* Brnset takes character array pcntl[nn] as a list of nn tokens to be
 * used to select desired generic branches.  Prflg[3] is the old
 * prnt[2] debug print flags in the first two elements plus a new print
 * flag which controls a branch selected summary from brnset.  Note that
 * the original two flags controlled a list of all tau interpolations
 * and a branch range summary respectively.  The original summary output
 * still goes to logical unit 10 (ttim1.lis) while the new output goes
 * to the standard output (so the caller can see what happened].  Each
 * token of pcntl may be either a generic branch name [e.g., P, PcP,
 * PKP, etc.] or a keyword [defined in the data statement for cmdcd
 * below] which translates to more than one generic branch names.  Note
 * that generic branch names and keywords may be mixed.  The keywords
 * 'all' [for all branches] and 'query' [for an interactive token input
 * query mode] are also available.
 */

	int i, j, k, l, no, j1, j2, kseg;
	static const int ncmd = 4 /*, lcmd = 16 */;
	char fnd, all;
	int nsgpt[jbrn];
	static const char *cmdlst[] = {"P", "PKiKP", "PcP", "pP", "pPKiKP", "sP",
		"sPKiKP", "ScP", "SKP", "PKKP", "SKKP", "PP", "S", "ScS",
		"sS", "pS"};
	static const char *cmdcd[] = {"P", "P+", "basic", "S+"};
	/*
	 * The keywords do the following:
	 * P      gives P-up, P, Pdiff, PKP, and PKiKP
	 * P+     gives P-up, P, Pdiff, PKP, PKiKP, PcP, pP, pPdiff, pPKP,
	 * pPKiKP, sP, sPdiff, sPKP, and sPKiKP
	 * S+     gives S-up, S, Sdiff, SKS, sS, sSdiff, sSKS, pS, pSdiff,
	 * and pSKS
	 * basic  gives P+ and S+ as well as ScP, SKP, PKKP, SKKP, PP, and
	 * P'P'
	 * Note that generic S gives S-up, Sdiff, and SKS already and so
	 * doesn't require a keyword.
	 */
	static const int ncmpt1[] = {0, 0, 0, 12};
	static const int ncmpt2[] = {1, 6, 12, 15};

	if ( !h->allocated ) {
		for ( i = 0; i < jseg; i++ ) h->phlst[i] = (char *)malloc(10);
		for ( i = 0; i < jbrn; i++ ) h->segcd[i] = (char *)malloc(10);
		h->allocated = 1;
	}

	/* In query mode, get the tokens interactively into local storage.
	 */
/*
	printf("Enter desired branch control list at the prompts:\n*  ");
*/
/*
	for(no = 0; scanf("%s", phlst[no]) == 1; no++) printf("*  ");
*/
/*
	scanf("%s", phlst[0]); no = 1;
*/
	strncpy(h->phlst[0], branch, 8); no = 1;
	/* Terminate the list of tokens with a blank entry.
	 */
	h->phlst[no][0] = '\0';
	/* An 'all' keyword is easy as this is already the default.
	 */
	all = False;
	if(no == 1 && !strcmp(h->phlst[0], "all")) all = True;
	if(all) return;
	/*
	 * Make one or two generic branch names for each segment.  For example,
	 * the P segment will have the names P and PKP, the PcP segment will
	 * have the name PcP, etc.
	 * 
	 * Loop over the segments.
	 */
	for(i = j = kseg = 0; i < h->nseg; i++)
	{
		if(!all) h->segmsk[i] = False;
		/*
		 * For each segment, loop over associated branches.
		 */
		do
		{
			strcpy(h->phtmp, h->depths[h->D].phcd[j]);
			/*
			 * Turn the specific branch name into a generic name
			 * by stripping out the crustal branch and core phase
			 * branch identifiers.
			 */
			for(l = k = 0; h->phtmp[l] != '\0'; )
			{
				if(	!strcmp(h->phtmp+l, "ab") ||
					!strcmp(h->phtmp+l, "ac") ||
					!strcmp(h->phtmp+l, "df")	  )
				{
					l += 2;
				}
				else if(h->phtmp[l] != 'g' && h->phtmp[l] != 'b' &&
				   h->phtmp[l] != 'n')
				{
					h->phtmp[k++] = h->phtmp[l++];
				}
				else l++;
			}
			h->phtmp[k] = '\0';
/*printf("j phcd phtmp = %d %s %s\n",j, h->depths[h->D].phcd[j], phtmp); */
			/*
			 * Make sure generic names are unique within a segment.
			 */
			if(kseg == 0 || strcmp(h->phtmp, h->segcd[kseg-1]))
			{
				strcpy(h->segcd[kseg], h->phtmp);
				nsgpt[kseg] = i;
				kseg++;
			}
		} while(h->jidx[j++] < h->v[1].indx[i]);
	}

	if(!all)
	{
		/* Interpret the tokens in terms of the generic branch names.
		 */
		for(i = 0; i < no; i++)
		{
			/* Try for a keyword first.
			 */
			for(j = 0; j < ncmd; j++)
				if(!strcmp(h->phlst[i], cmdcd[j])) break;

			if(j == ncmd)
			{
				/* If the token isn't a keyword, see if it is
				 * a generic branch name.
				 */
				fnd = False;
				for(k = 0; k < kseg; k++)
					if(!strcmp(h->phlst[i], h->segcd[k]))
				{
					fnd = True;
					l = nsgpt[k];
					h->segmsk[l] = True;
				}
				if(!fnd)
				{
					/* If no matching entry is found, warn
					 * the caller.
					 */
					fprintf(stderr,"Brnset: %s not found\n",
						h->phlst[i]);
					continue;
				}
			}
			else
			{
				/* If the token is a keyword, find the
				 * matching generic branch names.
				 */
				j1 = ncmpt1[j];
				j2 = ncmpt2[j];
				for(j = j1; j <= j2; j++)
				{
					for(k = 0; k < h->nseg; k++)
					{
						if(!strcmp(cmdlst[j], h->segcd[k]))
							h->segmsk[nsgpt[k]] = True;
					}
				}
			}
		}
	}
	/* Make the caller a list of the generic branch names selected.
	 */
	fnd = False;
	j2 = -1;
	/* Loop over segments.
	 */
	for(i = 0; i < h->nseg; i++) if(h->segmsk[i])
	{
		/* If selected, find the associated generic branch names.
		 */
		j2++;
		for(j1 = j2; j1 < kseg; j1++) if(nsgpt[j1] == i) break;

		if(j1 == kseg)
		{
			fprintf(stderr, "Brnset: Segment %d missing?\n", i);
		}
		else
		{
			for(j2 = j1; j2 < kseg; j2++)
				if(nsgpt[j2] != i) break;
			/* Print the result.
			 */
			j2--;
			fnd = True;
		}
	}
}

static void depcor(libtau *h, int nph) {
	int i, j, k, l, k1, k2, ks, ms, mu, is, iph, lp; 
	char noend, noext, do_integral, shallow;
	float ztol;
	double *tup, tauus1[2], tauus2[2], xus1[2], xus2[2],
		ttau, tx, sgn, umin, u0, u1, z0, z1, du;
	static const double dtol = 1.e-6;
	static const float tol = .01;
	static const int lpower = 7;

/*
	fprintf(fp10, "depcor:  nph nph0 %d %d\n", nph, h->depths[h->D].nph0);
*/
	tup = h->depths[h->D].tauc;
	if(nph == h->depths[h->D].nph0)
	{
		mu++;
		make_tau(h, nph, mu, umin, dtol, tauus1, tauus2, xus1, xus2);
		return;
	}
	h->depths[h->D].nph0 = nph;
	h->depths[h->D].us[nph] = umod(h, h->depths[h->D].zs, h->depths[h->D].isrc, nph);
	/* If we are in a high slowness zone, find the slowness of
	 * the lid.
	 */
	umin = h->depths[h->D].us[nph];
	ks = h->depths[h->D].isrc[nph];
/*
	fprintf(fp10, "ks us %d %f\n", ks, (float)umin);
*/
	for(i = 0; i <= ks; i++)
	{
		if(h->v[nph].pm[i] <= umin) umin = h->v[nph].pm[i];
	}
	/* Find where the source slowness falls in the ray parameter
	 * array.
 	 */
	for(k2 = 1; k2 <= h->ku[nph]; k2++) if(h->v[nph].pu[k2] > umin) break;
	if(k2 > h->ku[nph])
	{
		if(h->v[nph].pu[h->ku[nph]] == umin)
		{
			k2 = h->ku[nph];
		}
		else
		{
			fprintf(stderr, "depcor: Source slowness too large.\n");
			exit(1);
		}
	}
/*
	fprintf(fp10, "k2 umin %d %e\n", k2, (float)umin);
*/
	/* Read in the appropriate depth correction values.
	 */
	noext = False;
	sgn = 1.;
	if(h->depths[h->D].msrc[nph] == -1) h->depths[h->D].msrc[nph] = 0;
	/* See if the source depth coincides with a model samples.
	 */
	ztol = h->xn*tol/(1. - h->xn*h->depths[h->D].odep);
	if(fabs(h->depths[h->D].zs - h->v[nph].zm[ks+1]) <= ztol ||
		fabs(h->depths[h->D].zs - h->v[nph].zm[ks]) <= ztol)
	{
		if(fabs(h->depths[h->D].zs - h->v[nph].zm[ks+1]) <= ztol) ks++;
		/*
		 * If so flag the fact and make sure that the right
		 * integrals are available.
		 */
		noext = True;
		if(h->depths[h->D].msrc[nph] != ks)
		{
/*
fprintf(fp10, "first read ks= %d\n", ks);
printf("loc = %d\n",v[nph].loc[ks]);
*/
			fseek(h->fpin, h->v[nph].loc[ks], 0);
			fread(tup, 8, h->ku[nph]+h->km[nph], h->fpin);
			/* 
			 * Move the depth correction values to a less
			 * temporary area.
			 */
			for(i = 0; i < h->ku[nph]; i++)
				h->depths[h->D].w[nph].tauu[i] = tup[i];
			for(i = 0, k = h->ku[nph]; i < h->km[nph]; i++, k++)
			{
				h->depths[h->D].xc[i] = tup[k];
				h->depths[h->D].w[nph].xu[i] = tup[k];
			}
/*
		 	fprintf(fp10, "bkin %d %e %e %e\n", ks, (float)sgn,
				(float)h->depths[h->D].w[nph].tauu[0],
				(float)h->depths[h->D].w[nph].xu[0]);
*/
		}
	}
	else
	{
		/* If it is necessary to interpolate, see if the
		 * appropriate integrals have already been read in.
		 */
		if(h->depths[h->D].msrc[nph] == ks+1)
		{
			ks++;
			sgn = -1.;
		}
		else if(h->depths[h->D].msrc[nph] != ks)
		{
			/* If not, read in integrals for the model
			 * depth nearest the source depth.
			 */
			if(fabs(h->v[nph].zm[ks]-h->depths[h->D].zs) >
				fabs(h->v[nph].zm[ks+1]-h->depths[h->D].zs))
			{
				ks++;
				sgn = -1.;
			}
/*
fprintf(fp10, "second read ks= %d\n", ks);
*/
			fseek(h->fpin, h->v[nph].loc[ks], 0);
			fread(tup, 8, h->ku[nph]+h->km[nph], h->fpin);
			/* 
			 * Move the depth correction values to a less
			 * temporary area.
			 */
			for(i = 0; i < h->ku[nph]; i++)
				h->depths[h->D].w[nph].tauu[i] = tup[i];
			for(i = 0, k = h->ku[nph]; i < h->km[nph]; i++, k++)
			{
				h->depths[h->D].xc[i] = tup[k];
				h->depths[h->D].w[nph].xu[i] = tup[k];
			}
/*
		 	fprintf(fp10, "bkin %d %e %e %e\n", ks, (float)sgn,
				(float)h->depths[h->D].w[nph].tauu[0],
				(float)h->depths[h->D].w[nph].xu[0]);
*/
		}
	}
	/* 
	 * Fiddle pointers.
	 */
	h->depths[h->D].msrc[nph] = ks;
/*
	fprintf(fp10, "msrc sgn %d %e\n", h->depths[h->D].msrc[nph], (float)sgn);
*/
	noend = False;
	if(fabs(umin-h->v[nph].pu[k2-1]) <= dtol*umin) k2--;
	if(fabs(umin-h->v[nph].pu[k2]) <= dtol*umin) noend = True;
	if(h->depths[h->D].msrc[nph] <= 0 && noext) h->depths[h->D].msrc[nph] = -1;
	k1 = k2 - 1;
	if(noend) k1 = k2;
/*
	fprintf(fp10, "noend noext k2 k1 %d %d %d %d\n",
			(int)noend, (int)noext, k2, k1);
*/
	if(noext == False)
	{
		/* Correct the integrals for the depth interval (zm[msrc],zs).
		 */
		ms = h->depths[h->D].msrc[nph];
		if(sgn >= 0)
		{
			u0 = h->v[nph].pm[ms];
			z0 = h->v[nph].zm[ms];
			u1 = h->depths[h->D].us[nph];
			z1 = h->depths[h->D].zs;
		}
		else
		{
			u0 = h->depths[h->D].us[nph];
			z0 = h->depths[h->D].zs;
			u1 = h->v[nph].pm[ms];
			z1 = h->v[nph].zm[ms];
		}
/*
		fprintf(fp10, "u0 z0 %e %e\n", (float)u0, (float)z0);
		fprintf(fp10, "u1 z1 %e %e\n", (float)u1, (float)z1);
fprintf(fp10, "k1 = %d\n", k1);
*/
		for(k = mu = 0; k <= k1; k++)
		{
			tauint(h->v[nph].pu[k], u0, u1, z0, z1, &ttau, &tx);
			h->depths[h->D].tauc[k] = h->depths[h->D].w[nph].tauu[k] + sgn*ttau;
			if(fabs(h->v[nph].pu[k]-h->v[nph].pux[mu]) <= dtol)
			{
				h->depths[h->D].xc[mu]=h->depths[h->D].w[nph].xu[mu]+sgn*tx;
/*
				fprintf(fp10, "up first x: k mu %d %d %e %e\n",
					k, mu, (float)h->depths[h->D].w[nph].xu[mu],
					(float)h->depths[h->D].xc[mu]);
*/
				mu++;
			}
		}
	}
	else
	{
		/* If there is no correction, copy the depth corrections to
		 * working storage.
		 */
		for(k = mu = 0; k <= k1; k++)
		{
			h->depths[h->D].tauc[k] = h->depths[h->D].w[nph].tauu[k];
			if(fabs(h->v[nph].pu[k]-h->v[nph].pux[mu]) <= dtol)
			{
				h->depths[h->D].xc[mu] = h->depths[h->D].w[nph].xu[mu];
/*
				fprintf(fp10, "up second x: k mu %d %d %e %e\n",
					k, mu, (float)h->depths[h->D].w[nph].xu[mu],
					(float)h->depths[h->D].xc[mu]);
*/
				mu++;
			}
		}
	}
	/* 
	 * Calculate integrals for the ray bottoming at the source depth.
	 */
	xus1[nph] = 0.;
	xus2[nph] = 0.;
	mu--;
	if(fabs(umin-h->depths[h->D].us[nph]) > dtol &&
		fabs(umin-h->v[nph].pux[mu]) <= dtol) mu--;
	/*
	 * This loop may be skipped only for surface focus as range is not
	 * available for all ray parameters.
	 */
	if(h->depths[h->D].msrc[nph] < 0)
	{
		mu++;
		make_tau(h, nph, mu, umin, dtol, tauus1, tauus2, xus1, xus2);
	}
	is = h->depths[h->D].isrc[nph];
	tauus2[nph] = 0.;
	if(fabs(h->v[nph].pux[mu]-umin) <= dtol &&
		fabs(h->depths[h->D].us[nph]-umin) <= dtol)
	{
		/* If we happen to be right at a discontinuity,
		 * range is available.
		 */
		tauus1[nph] = h->depths[h->D].tauc[k1];
		xus1[nph] = h->depths[h->D].xc[mu];
/*
		fprintf(fp10, "1: is ks tauus1 xus1 %d %d %e %e  *\n",
			is, ks, (float)tauus1[nph], (float)xus1[nph]);
*/
	}
	else
	{
		/* Integrate from the surface to the source.
		 */
		tauus1[nph] = 0.;
		for(i = 1; i <= is; i++)
		{
			tauint(umin, h->v[nph].pm[i-1], h->v[nph].pm[i],
				h->v[nph].zm[i-1],h->v[nph].zm[i],&ttau,&tx);
			tauus1[nph] += ttau;
			xus1[nph] += tx;
		}
/*
		if(is>=1)fprintf(fp10, "2: is ks tauus1 xus1 %d %d %e %e\n",
			is, ks, (float)tauus1[nph], (float)xus1[nph]);
*/
		if(fabs(h->v[nph].zm[is]-h->depths[h->D].zs) > dtol)
		{
			/* Unless the source is right on a sample
			 * slowness, one more partial integral is
			 * needed.
			 */
			tauint(umin, h->v[nph].pm[is], h->depths[h->D].us[nph],
				h->v[nph].zm[is], h->depths[h->D].zs, &ttau, &tx);
			tauus1[nph] += ttau;
			xus1[nph] += tx;
/*
			fprintf(fp10, "3: is ks tauus1 xus1 %d %d %e %e\n",
				is, ks, (float)tauus1[nph], (float)xus1[nph]);
*/
		}
	}
	if(h->v[nph].pm[is+1] >= umin)
	{
		/* If we are in a high slowness zone, we will also
		 * need to integrate down to the turning point of the
		 * shallowest down-going ray.
		 */
		u1 = h->depths[h->D].us[nph];
		z1 = h->depths[h->D].zs;
		for(i = is+1; i < h->mt[nph]; i++)
		{
			u0 = u1;
			z0 = z1;
			u1 = h->v[nph].pm[i];
			z1 = h->v[nph].zm[i];
			if(u1 < umin) break;
			tauint(umin, u0, u1, z0, z1, &ttau, &tx);
			tauus2[nph] += ttau;
			xus2[nph] += tx;
		}
/*
		fprintf(fp10, "is ks tauus2 xus2 %d %d %e %e  *\n",
			is, ks, (float)tauus2[nph], (float)xus2[nph]);
*/
		z1 = zmod(h, umin, i-1, nph);
		if(fabs(z0-z1) > dtol)
		{
			/* Unless the turning point is right on a
			 * sample slowness, one more partial integral
			 * is needed.
			 */
			tauint(umin, u0, umin, z0, z1, &ttau, &tx);
			tauus2[nph] += ttau;
			xus2[nph] += tx;
/*
			fprintf(fp10, "is ks tauus2 xus2 %d %d %e %e  *\n",
				is, ks, (float)tauus2[nph], (float)xus2[nph]);
*/
		}
	}
	/* Take care of converted phases.
	 */
	iph = mod(nph+1, 2);
	xus1[iph] = 0.;
	xus2[iph] = 0.;
	tauus1[iph] = 0.;
	tauus2[iph] = 0.;
	do_integral = False;
	if(nph == 1)
	{
		if(umin <= h->v[0].pu[h->ku[0]])
		{
			/* If we are doing an S-wave depth correction,
			 * we may need range and tau for the P-wave
			 * which turns at the S-wave source slowness.
			 * This would bd needed for sPg and SPg when
			 * the source is in the deep mantle.
			 */
			for(j = 0; j < h->nbrn; j++) if( h->v[1].px[j] > 0.
				 && (!strncmp(h->depths[h->D].phcd[j],"sP",2) ||
				     !strncmp(h->depths[h->D].phcd[j],"SP",2)) )
			{
/*
	fprintf(fp10, "Depcor: j h->depths[h->D].phcd px umin = %d %s %E %E %E\n",
		j, h->depths[h->D].phcd[j], v[0].px[j], v[1].px[j], umin);
*/
				if(umin >= h->v[0].px[j] && umin < h->v[1].px[j])
				{
					do_integral = True;
					break;
				}
			}
		}
	}
	else
	{
		/* If we are doing an P-wave depth correction, we may
		 * need range and tau for the S-wave which turns at
		 * the P-wave source slowness.  This would be needed
		 * for pS and PS.
		 */
		for(j = 0; j < h->nbrn; j++) if( h->v[1].px[j] > 0. &&
			(!strncmp(h->depths[h->D].phcd[j], "pS", 2) ||
			 !strncmp(h->depths[h->D].phcd[j], "PS", 2)) )
		{
/*
	fprintf(fp10, "Depcor: j phcd px umin = %d %s %E %E %E\n",
		j, h->depths[h->D].phcd[j], v[0].px[j], v[1].px[j], umin);
*/
			if(umin >= h->v[0].px[j] && umin < h->v[1].px[j])
			{
				do_integral = True;
				break;
			}
		}
	}
	if(do_integral)
	{
		/* Do the integral.
		 */
/*
		fprintf(fp10, "Depcor: do pS or sP integral - iph = %d\n",iph);
*/
		for(i = 1; i < h->mt[iph]; i++)
		{
			if(umin >= h->v[iph].pm[i]) break;
			tauint(umin, h->v[iph].pm[i-1], h->v[iph].pm[i],
				h->v[iph].zm[i-1],h->v[iph].zm[i],&ttau,&tx);
			tauus1[iph] += ttau;
			xus1[iph] += tx;
		}
		z1 = zmod(h, umin, i-1, iph);
		if(fabs(h->v[iph].zm[i-1]-z1) > dtol)
		{
			/* Unless the turning point is right on a
			 * sample slowness, one more partial integral
			 * is needed.
			 */
			tauint(umin, h->v[iph].pm[i-1], umin,
				h->v[iph].zm[i-1], z1, &ttau, &tx);
			tauus1[iph] += ttau;
			xus1[iph] += tx;
/*
			fprintf(fp10, "is ks tauusp xusp %d %d %e %e\n", i-1,
				ks, (float)tauus1[iph], (float)xus1[iph]);
*/
		}
	}
	h->depths[h->D].ua[0][nph] = -1.;
	shallow = False;
	if(h->depths[h->D].odep < h->deplim)
	{
		for(i = 0; i < h->nseg; i++) if(h->segmsk[i])
		{
			if(h->s[0].nafl[i] == nph+1 && h->s[1].nafl[i] == 0
				&& h->depths[h->D].iidx[i] < 0)
			{
				shallow = True;
				break;
			}
		}
	}
	if(shallow)
	{
		/* If the source is very shallow, we will need to
		 * insert some extra ray parameter samples into the
		 * up-going branches.
		 */
		du = 1.e-5 + (h->depths[h->D].odep-.4)*2.e-5;
		if(du > 1.e-5) du = 1.e-5;
/*
		fprintf(fp10, "Add: nph is ka odep du us = %d %d %d %e %e %e\n",
			nph, is, ka, h->depths[h->D].odep, (float)du,
			(float)h->depths[h->D].us[nph]);
*/
		lp = lpower;
		for(l = h->ka-1, k = 0; l >= 0; l--, k++)
		{
			h->depths[h->D].ua[k][nph] = h->depths[h->D].us[nph] - du *
					pow((double)(l+1), (double)lp);
			lp--;
			h->depths[h->D].taua[k][nph] = 0.;
			for(i = 1; i <= is; i++)
			{
				tauint(h->depths[h->D].ua[k][nph], h->v[nph].pm[i-1],
					h->v[nph].pm[i], h->v[nph].zm[i-1],
					h->v[nph].zm[i], &ttau, &tx);
				h->depths[h->D].taua[k][nph] += ttau;
			}
/*
			if(is >= 1) fprintf(fp10, "l k ua taua %d %d %e %e\n",
				l, k, (float)h->depths[h->D].ua[k][nph],
				(float)h->depths[h->D].taua[k][nph]);
*/
			if(fabs(h->v[nph].zm[is]-h->depths[h->D].zs) > dtol)
			{
				/* Unless the source is right on a
				 * sample slowness, one more partial
				 * integral is needed.
				 */
				tauint(h->depths[h->D].ua[k][nph], h->v[nph].pm[is],
					h->depths[h->D].us[nph], h->v[nph].zm[is],
					h->depths[h->D].zs,&ttau,&tx);
				h->depths[h->D].taua[k][nph] += ttau;
/*
			fprintf(fp10, "l k ua taua %d %d %e %e\n", l, k, 
				(float)h->depths[h->D].ua[k][nph],
				(float)h->depths[h->D].taua[k][nph]);
*/
			}
		}
	}

	make_tau(h, nph, mu, umin, dtol, tauus1, tauus2, xus1, xus2);
}

static void
make_tau(libtau *h, int nph, int mu, double umin, double dtol, double *tauus1, double *tauus2, double *xus1, double *xus2)
{
	int i, j, k, l, m, iph, kph, i1, i2;
	double sgn, fac;

	/* Construct tau for all branches.
	 */
/*
fprintf(fp10, "mu = %d\nkiller loop:\n", mu);
*/
    for(i = j = 0; i < h->nseg; i++)
    {
/*
if(segmsk[i]) fprintf(fp10,"i iidx nafl nph %d %d %d %d\n", i,
		h->depths[h->D].iidx[i], s[0].nafl[i], nph);
*/
	if(h->segmsk[i] && h->depths[h->D].iidx[i] < 0 &&
		abs(h->s[0].nafl[i])-1 == nph &&
		(h->depths[h->D].msrc[nph] > -1 || h->s[0].nafl[i] <= 0))
	{
		iph = h->s[1].nafl[i]-1;
		kph = h->s[2].nafl[i]-1;
		/* Handle up-going P and S.
		 */
		if(iph < 0) iph = nph;
		if(kph < 0) kph = nph;
		sgn = (h->s[0].nafl[i] >= 0) ? 1 : -1;
		i1 = h->v[0].indx[i];
		i2 = h->v[1].indx[i];
/*
fprintf(fp10, "i1 i2 sgn iph %d %d %e %d\n", i1, i2, (float)sgn, iph);
*/
		for(k = i1, m = 0; k <= i2; k++)
		{
			if(h->depths[h->D].pt[k] > umin) break;
			while(fabs(h->depths[h->D].pt[k]-h->v[nph].pu[m]) > dtol) m++;
			h->depths[h->D].tau[k].c1 = h->taut[k] + sgn*h->depths[h->D].tauc[m];
		}
		if(k > i2)
		{
			k = i2;
/*
fprintf(fp10, "k m %d %d\n", k, m);
*/
		}
		else
		{
/*
fprintf(fp10, "k m %d %d\n", k, m);
*/
			if(fabs(h->depths[h->D].pt[k-1]-umin) <= dtol) k--;
			h->depths[h->D].ki++;
			h->depths[h->D].kk[h->depths[h->D].ki] = k;
			h->depths[h->D].pk[h->depths[h->D].ki] = h->depths[h->D].pt[k];
			h->depths[h->D].pt[k] = umin;
			fac = h->s[0].fcs[i];
/*
fprintf(fp10, "ki fac %d %e\n", h->depths[h->D].ki, (float)fac);
*/
			h->depths[h->D].tau[k].c1 = fac*(tauus1[iph] + tauus2[iph] +
				tauus1[kph] + tauus2[kph])+ sgn*tauus1[nph];
/*
fprintf(fp10, "&&&&& nph iph kph tauus1 tauus2 tau = %d %d %d %e %e %e %e %e\n",
	nph, iph, kph, (float)tauus1[0], (float)tauus1[1], (float)tauus2[0],
	(float)tauus2[1], (float)h->depths[h->D].tau[k].c1);
*/
		}
		m = 0;
		while(h->depths[h->D].w[0].jndx[j] < h->v[0].indx[i]) j++;

/*		while(j < nbrn && h->depths[h->D].w[0].jndx[j] < h->depths[h->D].w[1].jndx[j])
		{
*/
		do
		{
			h->depths[h->D].w[1].jndx[j] = (h->jidx[j] < k) ? h->jidx[j] : k;
			if(h->depths[h->D].w[0].jndx[j] >= h->depths[h->D].w[1].jndx[j])
			{
				h->depths[h->D].w[1].jndx[j] = -1;
				break;
			}

/*
fprintf(fp10, "j jndx jidx %d %d %d %d %s\n", j, h->depths[h->D].w[0].jndx[j], w[1].jndx[j],
jidx[j], h->depths[h->D].phcd[j]);
*/
			for(l = 0; l < 2; l++)
			{
				for(; m <= mu; m++)
				{
					if(fabs(h->v[nph].pux[m]-h->v[l].px[j])
							<= dtol) break;
				}
				if(m <= mu)
				{
					h->depths[h->D].t[l].xbrn[j] = h->v[l].xt[j] +
							sgn*h->depths[h->D].xc[m];
/*
fprintf(fp10, "x up: j l m %d %d %d\n", j, l, m);
*/
				}
				else
				{
					h->depths[h->D].t[l].xbrn[j] = fac*(xus1[iph]
						+ xus2[iph] + xus1[kph]
						+ xus2[kph]) + sgn*xus1[nph];
/*
fprintf(fp10, "x up: j l end %d %d\n", j, l);
fprintf(fp10, " nph iph kph xusr1 xusr2 xbrn = %d %d %d %e %e %e %e %e\n",
nph, iph, kph, (float)xus1[0], (float)xus1[1], (float)xus2[0],
(float)xus2[1], (float)h->depths[h->D].t[l].xbrn[j]);
*/
				}
			}
			if(j+1 >= h->nbrn) break;
			j++;
		} while(h->depths[h->D].w[0].jndx[j] <= k);
	}
    }
}

#define amax1(a,b)  (((a) > (b)) ? a : b)
#define amin1(a,b)  (((a) < (b)) ? a : b)

void DepSet(libtau *h, float dep);

void depset(libtau *h, float dep)
{
	h->D = 0;
	DepSet(h, dep);
}

void DepSet(libtau *h, float dep)
{
	char dop, dos;
	int i, ind, j, k, intt;
	float rdep /*, xnl */;

	/* Reset all depth structures */
	memcpy(&h->depths[0], &h->depths[1], sizeof(struct Depth));

	if(amax1(dep, .011) == h->depths[h->D].odep)
	{
		dop = False;
		dos = False;
		for(i = 0; i < h->nseg; i++) if(h->segmsk[i] && h->depths[h->D].iidx[i] < 0)
		{
			if(abs(h->s[0].nafl[i]) <= 1) dop = True;
			else dos = True;
		}
		if(!dop && !dos) return;
	}
	else
	{
		h->depths[h->D].nph0 = -1;
		h->depths[h->D].int0[0] = 0;
		h->depths[h->D].int0[1] = 0;
		h->depths[h->D].mbr1 = h->nbrn+1;
		h->depths[h->D].mbr2 = 0;
		dop = False;
		dos = False;
		for(i = 0; i < h->nseg; i++) if(h->segmsk[i])
		{
			if(abs(h->s[0].nafl[i]) <= 1) dop = True;
			else dos = True;
		}
		for(i = 0; i < h->nseg; i++)
		{
			if(h->s[1].nafl[i] <= 0 && h->depths[h->D].odep >= 0.)
			{
				ind = h->s[0].nafl[i]-1;
				for(j = h->v[0].indx[i], k=0; j <= h->v[1].indx[i]; j++, k++)
				{
					h->depths[h->D].pt[j] = h->v[ind].tp[k];
				}
			}
			h->depths[h->D].iidx[i] = -1;
		}
		for(i = 0; i < h->nbrn; i++) h->depths[h->D].w[1].jndx[i] = -1;

		for(i = 0; i <= h->depths[h->D].ki; i++)
			h->depths[h->D].pt[h->depths[h->D].kk[i]] = h->depths[h->D].pk[i];

		h->depths[h->D].ki = -1;
		/* 
		 * Sample the model at the source depth.;
		 */
		h->depths[h->D].odep = amax1(dep, .011);
		rdep = dep;
		if(rdep < .011) rdep = 0.;
		h->depths[h->D].zs = 1. - rdep*h->xn;
		if(h->depths[h->D].zs < 1.e-30) h->depths[h->D].zs = 1.e-30;
		h->depths[h->D].zs = log(h->depths[h->D].zs);
		if(h->depths[h->D].zs > 0.) h->depths[h->D].zs = 0.;
		h->depths[h->D].hn = 1./(h->pn*(1. - rdep*h->xn));
	}
	if(h->depths[h->D].nph0 <= 0)
	{
		if(dop) depcor(h, 0);
		if(dos) depcor(h, 1);
	}
	else
	{
		if(dos) depcor(h, 1);
		if(dop) depcor(h, 0);
	}
	/*
	 * Interpolate all tau branches.
	 */
	for(i = j = 0; i < h->nseg; i++) if(h->segmsk[i] && h->depths[h->D].iidx[i] < 0 &&
		(h->depths[h->D].msrc[abs(h->s[0].nafl[i])-1] >= 0 || h->s[0].nafl[i] <= 0))
	{
		h->depths[h->D].iidx[i] = 1;
		if(h->s[1].nafl[i] <= 0) intt = h->s[0].nafl[i];
		else if(h->s[1].nafl[i] == abs(h->s[0].nafl[i]))
			intt = h->s[1].nafl[i] + 2;
		else intt = abs(h->s[0].nafl[i]) + 4;
		if(h->s[1].nafl[i] > 0 && h->s[1].nafl[i] != h->s[2].nafl[i])
			intt = h->s[1].nafl[i] + 6;

		while(h->depths[h->D].w[0].jndx[j] < h->v[0].indx[i]) j++;
		do
		{
			h->depths[h->D].t[2].idel[j] = h->s[0].nafl[i];
			spfit(h, j, intt);
			if(h->depths[h->D].mbr1 > j) h->depths[h->D].mbr1 = j;
			if(h->depths[h->D].mbr2 < j) h->depths[h->D].mbr2 = j;
		} while(++j < h->nbrn && h->jidx[j] <= h->v[1].indx[i] &&
				h->depths[h->D].w[1].jndx[j] >= 0);
	}
/*
fprintf(fp10, "mbr1 mbr2 %d %d\n", h->depths[h->D].mbr1, h->depths[h->D].mbr2);
fprintf(fp10, "msrc isrc odep zs us %d %d %d %d %e %e %e %e\n",
h->depths[h->D].msrc[0], h->depths[h->D].msrc[1], h->depths[h->D].isrc[0], h->depths[h->D].isrc[1],
h->depths[h->D].odep, (float)h->depths[h->D].zs, (float)h->depths[h->D].us[0],
(float)h->depths[h->D].us[1]);
fprintf(fp10, "\n          %5d\n", h->depths[h->D].ki);
for(i = 0; i < nseg; i++)
	fprintf(fp10, " %5d%5d%5d%12.6f\n", i, h->depths[h->D].iidx[i],
		h->depths[h->D].kk[i], (float)h->depths[h->D].pk[i]);
*/
}

static void
findtt(libtau *h, int jb, double *x0, int *pn, float *tt, float *dtdd, float *dtdh, float *dddp, float *ray_p, char **phnm)
{
	int i, j, n, nph, ij, ie;
	char *s;
	float hsgn, dsgn, dpn;
	double x, p0, p1, arg, dp, dps, dp0, delp, ps;
	static const double tol = 3.e-6, deps = 1.e-10;

	n = *pn;
	nph = abs(h->depths[h->D].t[2].idel[jb]) - 1;
	hsgn = (h->depths[h->D].t[2].idel[jb] >= 0) ? h->depths[h->D].hn : -h->depths[h->D].hn;
	dsgn = pow(-1., (double)h->depths[h->D].t[0].idel[jb]) * h->dn;
	dpn = -1./h->tn;
	for(ij = h->depths[h->D].t[0].idel[jb]; ij <= h->depths[h->D].t[1].idel[jb]; ij++)
	{
	    x = x0[ij-1];
	    dsgn = -dsgn;
	    if(x >= h->depths[h->D].t[0].xbrn[jb] && x <= h->depths[h->D].t[1].xbrn[jb])
	    {
		ie = h->depths[h->D].w[1].jndx[jb];
		for(i = h->depths[h->D].w[0].jndx[jb]+1; i <= ie; i++)
			if(x> h->depths[h->D].xlim1[i-1] && x <= h->depths[h->D].xlim2[i-1])
		{
		    j = i - 1;
		    p0 = h->depths[h->D].pt[ie] - h->depths[h->D].pt[j];
		    p1 = h->depths[h->D].pt[ie] - h->depths[h->D].pt[i];
		    delp = tol*(h->depths[h->D].pt[i] - h->depths[h->D].pt[j]);
		    if(delp < 1.e-3) delp = 1.e-3;
		    if(fabs(h->depths[h->D].tau[j].c3) <= 1e-30)
		    {
			dps=(x - h->depths[h->D].tau[j].c2)/(1.5*h->depths[h->D].tau[j].c4);
			dp = dps*dps;
			if(dps < 0.) dp = -dp;
			dp0 = dp;
			if(dp < p1-delp || dp > p0+delp)
			{
			    fprintf(stderr,
			     "findtt failed on: %s %8.1f%7.4f%7.4f%7.4f%7.4f\n",
				h->depths[h->D].phcd[jb], x, dp0, dp, p1, p0);
			    continue;
			}
			ps = h->depths[h->D].pt[ie] - dp;
			ray_p[n] = h->tn*ps;
			tt[n] = h->tn*(h->depths[h->D].tau[j].c1 +dp*(h->depths[h->D].tau[j].c2
					 + dps*h->depths[h->D].tau[j].c4) + ps*x);
			dtdd[n] = dsgn*ps;
			dtdh[n] = hsgn*sqrt(fabs(h->depths[h->D].us[nph]*
					h->depths[h->D].us[nph] - ps*ps));
			dddp[n] = dpn*.75*h->depths[h->D].tau[j].c4/
					amax1(fabs(dps),deps);
			strcpy(phnm[n], h->depths[h->D].phcd[jb]);
			if((s=xindex(phnm[n], "ab")) != NULL)
			{
			    if(ps <= h->depths[h->D].t[2].xbrn[jb]) strcpy(s, "bc");
			}
			n++;
		    }
		    else
		    {
			arg = 9.*h->depths[h->D].tau[j].c4*h->depths[h->D].tau[j].c4 +
				32.*h->depths[h->D].tau[j].c3*(x-h->depths[h->D].tau[j].c2);
			if(arg < 0.) fprintf(stderr, "findtt: bad sqrt arg.\n");
			dps = sqrt(fabs(arg));
			if(h->depths[h->D].tau[j].c4 < 0.) dps = -dps;
			dps = -(3.*h->depths[h->D].tau[j].c4 + dps)/
					(8.*h->depths[h->D].tau[j].c3);
			dp = (dps >= 0.) ? dps*dps : -dps*dps;
			dp0 = dp;
			if(dp >= p1-delp && dp <= p0+delp)
			{
			    ps = h->depths[h->D].pt[ie] - dp;
			    ray_p[n] = h->tn*ps;
			    tt[n] = h->tn*(h->depths[h->D].tau[j].c1 + dp*
					(h->depths[h->D].tau[j].c2 +
					 dp*h->depths[h->D].tau[j].c3 +
					 dps*h->depths[h->D].tau[j].c4) + ps*x);
			    dtdd[n] = dsgn*ps;
			    dtdh[n] = hsgn*sqrt(fabs(h->depths[h->D].us[nph]*
					h->depths[h->D].us[nph] -ps*ps));
			    dddp[n] = dpn*(2.*h->depths[h->D].tau[j].c3 +
					.75*h->depths[h->D].tau[j].c4/
					amax1(fabs(dps),deps));
			    strcpy(phnm[n], h->depths[h->D].phcd[jb]);
			    if((s=xindex(phnm[n], "ab")) != NULL)
			    {
				if(ps <= h->depths[h->D].t[2].xbrn[jb])strcpy(s,"bc");
			    }
			    n++;
			}
			dps = (h->depths[h->D].tau[j].c2-x)/
					(2.*h->depths[h->D].tau[j].c3*dps);
			dp = (dps >= 0.) ? dps*dps : -dps*dps;
			if(dp >= p1-delp && dp <= p0+delp)
			{
			    ps = h->depths[h->D].pt[ie] - dp;
			    ray_p[n] = h->tn*ps;
			    tt[n] = h->tn*(h->depths[h->D].tau[j].c1 +
					dp*(h->depths[h->D].tau[j].c2 +
					dp*h->depths[h->D].tau[j].c3 +
					dps*h->depths[h->D].tau[j].c4) + ps*x);
			    dtdd[n] = dsgn*ps;
			    dtdh[n] = hsgn*sqrt(fabs(h->depths[h->D].us[nph]*
					h->depths[h->D].us[nph] -ps*ps));
			    dddp[n] = dpn*(2.*h->depths[h->D].tau[j].c3 +
					.75*h->depths[h->D].tau[j].c4/
					amax1(fabs(dps),deps));
			    strcpy(phnm[n], h->depths[h->D].phcd[jb]);
			    if((s=xindex(phnm[n], "ab")) != NULL)
			    {
				if(ps <= h->depths[h->D].t[2].xbrn[jb])strcpy(s,"bc");
			    }
			    n++;
			}
		    }
		}
	    }
	    if(x >= h->depths[h->D].w[0].dbrn[jb] && x <= h->depths[h->D].w[1].dbrn[jb])
	    {
		    j = h->depths[h->D].w[0].jndx[jb];
		    i = h->depths[h->D].w[1].jndx[jb];
		    dp = h->depths[h->D].pt[i] - h->depths[h->D].pt[j];
		    dps = sqrt(fabs(dp));
		    ray_p[n] = h->tn*h->depths[h->D].pt[j];
		    tt[n] = h->tn*(h->depths[h->D].tau[j].c1 + dp*(h->depths[h->D].tau[j].c2
				+ dp*h->depths[h->D].tau[j].c3
				+ dps*h->depths[h->D].tau[j].c4) + h->depths[h->D].pt[j]*x);
		    dtdd[n] = dsgn*h->depths[h->D].pt[j];
		    dtdh[n] = hsgn*sqrt(fabs(h->depths[h->D].us[nph]*h->depths[h->D].us[nph]
				- h->depths[h->D].pt[j]*h->depths[h->D].pt[j]));
		    dddp[n] = dpn*(2.*h->depths[h->D].tau[j].c3 +
				.75*h->depths[h->D].tau[j].c4/amax1(dps,deps));
		    strcpy(phnm[n], h->depths[h->D].phcd[jb]);
		    strcat(phnm[n], "diff");
		    n++;
	    }
	}
	*pn = n;
}

static void
pdecu(libtau *h, int i1, int i2, double x0, double x1, double xmin, int intt, int *len)
{
	int i, j, k, is, ie, n, m;
	double dx, dx2, sgn, rnd, xm, axm, x, h1, h2, hh, xs;

/*
fprintf(fp10, "Pdecu: us = %e\n",(float)h->depths[h->D].ua[0][intt-1]);
*/
	if(h->depths[h->D].ua[0][intt-1] > 0.)
	{
/*
fprintf(fp10, "Pdecu: fill in new grid\n");
*/
		for(i = 0, k = i1+1; i < h->ka; i++, k++)
		{
			h->depths[h->D].pt[k] = h->depths[h->D].ua[i][intt-1];
			h->depths[h->D].tau[k].c1 = h->depths[h->D].taua[i][intt-1];
		}
		h->depths[h->D].pt[k] = h->depths[h->D].pt[i2];
		h->depths[h->D].tau[k].c1 = h->depths[h->D].tau[i2].c1;
		*len = k;
/*
fprintf(fp10, "\n");
for(i = i1; i <= *len; i++)
fprintf(fp10, " %5d %12.6f %15.4f\n",
i, (float)h->depths[h->D].pt[i], (float)h->depths[h->D].tau[i].c1);
*/
		return;
	}
	is = i1 + 1;
	ie = i2 - 1;
	xs = x1;
	for(i = ie; i >= i1; i--)
	{
		x = xs;
		if(i == i1)
		{
			xs = x0;
		}
		else
		{
			h1 = h->depths[h->D].pt[i-1] - h->depths[h->D].pt[i];
			h2 = h->depths[h->D].pt[i+1] - h->depths[h->D].pt[i];
			hh = h1*h2*(h1-h2);
			h1 = h1*h1;
			h2 = -h2*h2;
			xs = -(h2*h->depths[h->D].tau[i-1].c1
					- (h2+h1)*h->depths[h->D].tau[i].c1
					+ h1*h->depths[h->D].tau[i+1].c1)/hh;
		}
		if(fabs(x-xs) <= xmin) break;
	}
	if(i < i1)
	{
		*len = i2;
		return;
	}
	ie = i;
	if(fabs(x-xs) <= .75*xmin && ie != i2)
	{
		xs = x;
		ie++;
	}
	n = (int)(fabs(xs-x0)/xmin + .8);
	if(n < 1) n = 1;
	dx = (xs-x0)/n;
	dx2 = fabs(.5*dx);
	sgn = (dx >= 0.) ? 1. : -1.;
	rnd = 0.;
	if(sgn > 0.) rnd = 1.;
	xm = x0 + dx;
	k = i1;
	m = is;
	axm = 1.e+10;
	for(i = is; i <= ie; i++)
	{
		if(i == ie)
		{
			x = xs;
		}
		else
		{
			h1 = h->depths[h->D].pt[i-1] - h->depths[h->D].pt[i];
			h2 = h->depths[h->D].pt[i+1] - h->depths[h->D].pt[i];
			hh = h1*h2*(h1-h2);
			h1 = h1*h1;
			h2 = -h2*h2;
			x = -(h2*h->depths[h->D].tau[i-1].c1
					- (h2+h1)*h->depths[h->D].tau[i].c1
					+ h1*h->depths[h->D].tau[i+1].c1)/hh;
		}
		if(sgn*(x-xm) > dx2)
		{
			for(j = m; j <= k; j++) h->depths[h->D].pt[j] = -1.;
			m = k + 2;
			k = i-1;
			axm = 1.e+10;
			xm += dx*(int)((x - xm - dx2)/dx + rnd);
		}
		if(fabs(x-xm) < axm)
		{
			axm = fabs(x-xm);
			k = i-1;
		}
	}
	for(j = m; j <= k; j++) h->depths[h->D].pt[j] = -1.;
	for(i = is, k = i1; i <= i2; i++) if(h->depths[h->D].pt[i] >= 0.)
	{
		k++;
		h->depths[h->D].pt[k] = h->depths[h->D].pt[i];
		h->depths[h->D].tau[k].c1 = h->depths[h->D].tau[i].c1;
	}
	*len = k;
/*
fprintf(fp10, "\n");
for(i = i1; i <= *len; i++)
fprintf(fp10, " %5d %12.6f %15.4f\n",
i, (float)h->depths[h->D].pt[i], (float)h->depths[h->D].tau[i].c1);
*/
}

static void
r4sort(int n, float *rkey, int *iptr)
{
	/*
	 * R4sort sorts the n elements of array rkey so that rkey[i],
	 * i  =  0, 1, 2, ..., n-1 are in asending order.  R4sort is a trivial
	 * modification of ACM algorithm 347:  "An efficient algorithm for
	 * sorting with minimal storage" by R. C. Singleton.  Array rkey is
	 * sorted in place in order n*alog2(n) operations.  Coded on
	 * 8 March 1979 by R. Buland.  Modified to handle real*4 data on
	 * 27 September 1983 by R. Buland.
	 * Translated to C by I. Henson, Jan, 1991.
	 */
	int i, j, k, l, m, ij, it, kk, ib;
	int il[10], iu[10];
	float r, tmpkey;
	/*
	 * Note:  il and iu implement a stack containing the upper and
	 * lower limits of subsequences to be sorted independently.  A
	 * depth of k allows for n <= 2**[k+1]-1.
	 */
	if(n <= 0) return;
	for(i = 0; i < n; i++) iptr[i] = i;
	if(n <= 1) return;
	r = .375;
	m = 0;
	i = 0;
	j = n-1;

	for(;;)
	{
		if(i < j)
		{
			/*
			 * The first section interchanges low element i, middle
			 * element ij, and high element j so they are in order.
			 */
			k = i;
			/* Use a floating point modification, r, of Singleton's
			 * bisection strategy [suggested by R. Peto in his
			 * verification of the algorithm for the ACM].
			 */
			if(r <= .58984375) r += .0390625;
			else  r -= .21875;
			ij = i + (j-i)*r;
			if(rkey[iptr[i]] > rkey[iptr[ij]])
			{
				it = iptr[ij];
				iptr[ij] = iptr[i];
				iptr[i] = it;
			}
			l = j;
			if(rkey[iptr[j]] < rkey[iptr[ij]])
			{
				it = iptr[ij];
				iptr[ij] = iptr[j];
				iptr[j] = it;
				if(rkey[iptr[i]] > rkey[iptr[ij]])
				{
					it = iptr[ij];
					iptr[ij] = iptr[i];
					iptr[i] = it;
				}
			}
			tmpkey = rkey[iptr[ij]];

			/* The second section continues this process.  K counts
			 * up from i and l down from j.  Each time the k
			 * element is bigger than the ij and the l element is
			 * less than the ij, then interchange the k and l
			 * elements.  This continues until k and l meet.
			 */
			do
			{
				while(rkey[iptr[--l]] > tmpkey);
				while(rkey[iptr[++k]] < tmpkey);
				if(k <= l)
				{
					it = iptr[l];
					iptr[l] = iptr[k];
					iptr[k] = it;
				}
			} while(k <= l);
			/*
			 * The third section considers the intervals i to l and
			 * k to j.  The larger interval is saved on the stack
			 * [il and iu] and the smaller is remapped into i and j
			 * for another shot at section one.
			 */
			if(l-i > j-k)
			{
				il[m] = i;
				iu[m] = l;
				i = k;
				m++;
			}
			else
			{
				il[m] = k;
				iu[m] = j;
				j = l;
				m++;
			}
		}
		else
		{
			/*
			 * The fourth section pops elements off the stack
			 * [into i and j].  If necessary control is transfered
			 * back to section one for more interchange sorting.
			 * If not we fall through to section five.  Note that
			 * the algorighm exits when the stack is empty.
			 */
			if(m == 0) return;
			m--;
			i = il[m];
			j = iu[m];
		}
		if(i > 0 && j-i < 11)
		{
			i--;
			/*
			 * The fifth section is the end game.  Final sorting is
			 * accomplished (within each subsequence popped off the
			 * stack) by rippling out of order elements down to
			 * their proper positions.
			 */
			for(;;)
			{
				while(++i != j &&
					rkey[iptr[i]] <= rkey[iptr[i+1]]);
				if(i == j) break;
				k = i;
				kk = k + 1;
				ib = iptr[kk];
				do
				{
					iptr[kk] = iptr[k];
					kk = k;
					k--;
				} while(rkey[ib] < rkey[iptr[k]]);
				iptr[kk] = ib;
			}
		}
	}
}

static void
spfit(libtau *h, int jb, int intt)
{
	char disc[5];
	char newgrd, makgrd;
	int i, j, k, i1, i2, nn, mxcnt, mncnt;
	double pmn, dmn, dmx, hm, shm, thm, p0, p1, tau0, tau1, x0, x1, pe,
		pe0, spe0, scpe0, pe1, spe1, scpe1, dpe, dtau;
	static const double dbrnch = 2.5307274 /*, cn = 57.295779 */, x180 =M_PI,
		x360 = 6.283185, dtol = 1.e-6,
		ptol = 2.e-6;
	double a1[jbrna], a2[jbrna], a3[jbrna],a4[jbrna], a5[jbrna];
	double b1[jbrna], b2[jbrna], b3[jbrna],b4[jbrna], b5[jbrna];
	double xmin;

	i1 = h->depths[h->D].w[0].jndx[jb];
	i2 = h->depths[h->D].w[1].jndx[jb];
/*
fprintf(fp10, "Spfit: jb i1 i2 pt = %d %d %d %e %e\n", jb, i1, i2,
			(float)h->depths[h->D].pt[i1], (float)h->depths[h->D].pt[i2]);
*/
	if(i2 - i1 <= 1 && fabs(h->depths[h->D].pt[i2] - h->depths[h->D].pt[i1]) <= ptol)
	{
		h->depths[h->D].w[1].jndx[jb] = -1;
		return;
	}
	newgrd = False;
	makgrd = False;
	if(fabs(h->v[1].px[jb] - h->depths[h->D].pt[i2]) > dtol) newgrd = True;
/*
fprintf(fp10, "Spfit: px newgrd = %f %d\n", (float)v[1].px[jb], (int)newgrd);
*/
	if(newgrd)
	{
		k = mod(intt-1,2);
		if(intt != h->depths[h->D].int0[k]) makgrd = True;
/*
fprintf(fp10, "Spfit: int k int0 makgrd = %d %d %d %d\n",
intt, k, h->depths[h->D].int0[k],
(int)makgrd);
*/
		if(intt <= 2)
		{
			xmin = amax1(2.*h->depths[h->D].odep, 2.);
			xmin = (xmin < 25.) ? h->xn*xmin : h->xn*25.;
/*
fprintf(fp10, "Spfit: xmin = %e %e\n", (float)xmin, (float)(xmin/xn));
*/
			pdecu(h, i1, i2, h->depths[h->D].t[0].xbrn[jb],
			      h->depths[h->D].t[1].xbrn[jb], xmin, intt, &i2);
			h->depths[h->D].w[1].jndx[jb] = i2;
		}
		nn = i2 - i1;
		if(makgrd)
		{
			if(!k) tauspl(0, nn, h->depths[h->D].pt+i1, a1, a2, a3,a4,a5);
			else   tauspl(0, nn, h->depths[h->D].pt+i1, b1, b2, b3,b4,b5);
		}
/*
fprintf(fp10, " %3d%3d%3d%3d%2d%2d%12.8f%12.8f\n", jb, k, nn+1, intt,
(int)newgrd,(int)makgrd, (float)h->depths[h->D].t[0].xbrn[jb],
	(float)h->depths[h->D].t[1].xbrn[jb]);
for(i = 0; i <= nn; i++)
if(!k)fprintf(fp10, "%5d%12.8f%12.8f%10.2E%10.2E%10.2E%10.2E%10.2E\n", i,
	(float)h->depths[h->D].pt[i1+i], h->depths[h->D].tau[i1+i].c1,
	a1[i],a2[i],a3[i],a4[i],a5[i]);
else fprintf(fp10, "%5d%12.8f%12.8f%10.2E%10.2E%10.2E%10.2E%10.2E\n", i,
	(float)h->depths[h->D].pt[i1+i], h->depths[h->D].tau[i1+i].c1,
	b1[i],b2[i],b3[i],b4[i],b5[i]);
*/

		if(!k) fitspl(0, nn, h->depths[h->D].tau+i1, h->depths[h->D].t[0].xbrn[jb],
				h->depths[h->D].t[1].xbrn[jb], a1, a2, a3, a4, a5);
		else   fitspl(0, nn, h->depths[h->D].tau+i1, h->depths[h->D].t[0].xbrn[jb],
				h->depths[h->D].t[1].xbrn[jb], b1, b2, b3, b4, b5);
		h->depths[h->D].int0[k] = intt;
	}
	else
	{
		fitspl(i1, i2, h->depths[h->D].tau, h->depths[h->D].t[0].xbrn[jb],
				h->depths[h->D].t[1].xbrn[jb], h->c1, h->c2, h->c3, h->c4, h->c5);
	}
	pmn = h->depths[h->D].pt[i1];
	dmn = h->depths[h->D].t[0].xbrn[jb];
	dmx = dmn;
	mxcnt = 0;
	mncnt = 0;
	pe = h->depths[h->D].pt[i2];
	p1 = h->depths[h->D].pt[i1];
	tau1 = h->depths[h->D].tau[i1].c1;
	x1 = h->depths[h->D].tau[i1].c2;
	pe1 = pe - p1;
	spe1 = sqrt(fabs(pe1));
	scpe1 = pe1*spe1;
	for(i = i1+1; i <= i2; i++)
	{
		p0 = p1;
		p1 = h->depths[h->D].pt[i];
		tau0 = tau1;
		tau1 = h->depths[h->D].tau[i].c1;
		x0 = x1;
		x1 = h->depths[h->D].tau[i].c2;
		dpe = p0-p1;
		dtau = tau1-tau0;
		pe0 = pe1;
		pe1 = pe - p1;
		spe0 = spe1;
		spe1 = sqrt(fabs(pe1));
		scpe0 = scpe1;
		scpe1 = pe1*spe1;
		h->depths[h->D].tau[i-1].c4 = (2.*dtau - dpe*(x1+x0))/
			(.5*(scpe1-scpe0)-1.5*spe1*spe0*(spe1-spe0));
		h->depths[h->D].tau[i-1].c3 = (dtau - dpe*x0 - (scpe1 + .5*scpe0
				-1.5*pe1*spe0)*h->depths[h->D].tau[i-1].c4)/(dpe*dpe);
		h->depths[h->D].tau[i-1].c2 = (dtau -(pe1*pe1 - pe0*pe0)*
				h->depths[h->D].tau[i-1].c3
				- (scpe1-scpe0)*h->depths[h->D].tau[i-1].c4)/dpe;
		h->depths[h->D].tau[i-1].c1 = tau0 - scpe0*h->depths[h->D].tau[i-1].c4
				- pe0*(pe0*h->depths[h->D].tau[i-1].c3
				+ h->depths[h->D].tau[i-1].c2);
		h->depths[h->D].xlim1[i-1] = (x0 < x1) ? x0 : x1;
		h->depths[h->D].xlim2[i-1] = (x0 > x1) ? x0 : x1;
		if(h->depths[h->D].xlim1[i-1] < dmn)
		{
			dmn = h->depths[h->D].xlim1[i-1];
			pmn = h->depths[h->D].pt[i-1];
			if(x1 < x0) pmn = h->depths[h->D].pt[i];
		}
		disc[0] = '\0';
		if(fabs(h->depths[h->D].tau[i-1].c3) > 1.e-30)
		{
			shm = -.375*h->depths[h->D].tau[i-1].c4/h->depths[h->D].tau[i-1].c3;
			hm = shm*shm;
			if(shm > 0. && hm > pe1 && hm < pe0)
			{
				thm = h->depths[h->D].tau[i-1].c2
					+ shm*(2.*shm*h->depths[h->D].tau[i-1].c3
					+ 1.5*h->depths[h->D].tau[i-1].c4);
				if(h->depths[h->D].xlim1[i-1] > thm)
					h->depths[h->D].xlim1[i-1] = thm;
				if(h->depths[h->D].xlim2[i-1] < thm)
					h->depths[h->D].xlim2[i-1] = thm;
				if(thm < dmn)
				{
					dmn = thm;
					pmn = pe - hm;
				}
				if(h->depths[h->D].tau[i-1].c4 >= 0.)
				{
					strcpy(disc, "max");
					mxcnt++;
				}
				else
				{
					strcpy(disc, "min");
					mncnt++;
				}
			}
		}
		if(dmx < h->depths[h->D].xlim2[i-1]) dmx = h->depths[h->D].xlim2[i-1];
	}
	h->depths[h->D].t[0].xbrn[jb] = dmn;
	h->depths[h->D].t[1].xbrn[jb] = dmx;
	h->depths[h->D].t[2].xbrn[jb] = pmn;
	h->depths[h->D].t[0].idel[jb] = 1;
	h->depths[h->D].t[1].idel[jb] = 1;
	if(h->depths[h->D].t[0].xbrn[jb] > x180) h->depths[h->D].t[0].idel[jb] = 2;
	if(h->depths[h->D].t[1].xbrn[jb] > x180) h->depths[h->D].t[1].idel[jb] = 2;
	if(h->depths[h->D].t[0].xbrn[jb] > x360) h->depths[h->D].t[0].idel[jb] = 3;
	if(h->depths[h->D].t[1].xbrn[jb] > x360) h->depths[h->D].t[1].idel[jb] = 3;
	if(intt <= 2)
	{
		h->depths[h->D].phcd[jb][1] = '\0';
		i = jb;
		for(j = 0; j < h->nbrn; j++)
		{
			i = mod(i+1, h->nbrn);
			if(h->depths[h->D].phcd[i][0] == h->depths[h->D].phcd[jb][0]
				&& h->depths[h->D].phcd[i][1] != 'P'
				&& (pe >= h->v[0].px[i] && pe <= h->v[1].px[i]))
			{
				strcpy(h->depths[h->D].phcd[jb] ,h->depths[h->D].phcd[i]);
				if(fabs(h->depths[h->D].pt[i2] -
				  h->depths[h->D].pt[h->depths[h->D].w[0].jndx[i]]) <= dtol)
				{
					strcpy(h->depths[h->D].phcd[jb],
						h->depths[h->D].phcd[i-1]);
				}
				break;
			}
		}
	}
	if(h->depths[h->D].w[0].dbrn[jb] > 0.)
	{
		h->depths[h->D].w[0].dbrn[jb] = dmx;
		h->depths[h->D].w[1].dbrn[jb] = dbrnch;
	}
	if(mxcnt > mncnt || mncnt > mxcnt+1)
	{
		fprintf(stderr, "spfit: Bad interpolation on %s\n",
				h->depths[h->D].phcd[jb]);
/*
		fprintf(fp10, "spfit: Bad interpolation on %s\n",
				h->depths[h->D].phcd[jb]);
*/
	}
}

static void
fitspl(int i1, int i2,
	c1c2c3c4 *tau,
       double x1, double xn,
       double *c1, double *c2, double *c3, double *c4, double *c5)
{
/* Given ray parameter grid p;i (p sub i), i=1,2,...,n, corresponding
 * tau;i values, and x;1 and x;n (x;i = -dtau/dp|p;i); tauspl finds
 * interpolation I such that:  tau(p) = a;1,i + Dp * a;2,i + Dp**2 *
 * a;3,i + Dp**(3/2) * a;4,i where Dp = p;n - p and p;i <= p < p;i+1.
 * Interpolation I has the following properties:  1) x;1, x;n, and
 * tau;i, i=1,2,...,n are fit exactly, 2) the first and second
 * derivitives with respect to p are continuous everywhere, and
 * 3) because of the paramaterization d**2 tau/dp**2|p;n is infinite.
 * Thus, interpolation I models the asymptotic behavior of tau(p)
 * when tau(p;n) is a branch end due to a discontinuity in the
 * velocity model.  Note that array a must be dimensioned at least
 * a(4,n) though the interpolation coefficients will be returned in
 * the first n-1 columns.  The remaining column is used as scratch
 * space and returned as all zeros.  Programmed on 16 August 1982 by
 * R. Buland.  (Translated to C by I. Henson, Jan. 1991).
 */
	int i, j, n;
	double a1[200], a2[200], ap[3], b[200], alr, gn;

	if(i2 < i1) return;
	if(i2 == i1)
	{
		tau[i1].c2 = x1;
		return;
	}
	for(i = i1, n = 0; i <= i2; i++, n++)
	{
		b[n] = tau[i].c1;
 		a1[n] = c1[i];
 		a2[n] = c2[i];
	}
	ap[0]   = c3[i2];
	ap[1]   = c4[i2];
	ap[2]   = c5[i2];
	/*
	 * Arrays ap[*][0], a, and ap[*][1] comprise n+2 x n+2 penta-diagonal
	 * matrix A.  Let x1, tau, and xn comprise corresponding n+2 vector b.
	 * Then, A * g  =  b, may be solved for n+2 vector g such that
	 * interpolation I is given by I(p)  =  sum(i=0,n+1) g;i * G;i(p).
	 * 
	 * Eliminate the lower triangular portion of A to form A'.  A
	 * corresponding transformation applied to vector b is stored in
	 * a[3][*].
	 */
	alr = a1[0]/c3[i1];
	a1[0] = 1. - c4[i1]*alr;
	a2[0] = a2[0] - c5[i1]*alr;
	b[0] = b[0] - x1*alr;
	for(i = 1; i < n; i++)
	{
		alr = a1[i]/a1[i-1];
		a1[i] = 1. - a2[i-1]*alr;
		b[i] = b[i] - b[i-1]*alr;
	}
	alr = ap[0]/a1[n-2];
	ap[1] = ap[1] - a2[n-2]*alr;
	gn = xn - b[n-2]*alr;
	alr = ap[1]/a1[n-1];
	/*
	 * Back solve the upper triangular portion of A' for coefficients g;i.
	 * When finished, storage g[1], a[3][*], g[4] will comprise vector g.
	 */
	gn = (gn - b[n-1]*alr)/(ap[2] - a2[n-1]*alr);
	b[n-1] = (b[n-1] - gn*a2[n-1])/a1[n-1];

	for(i = n-2; i >= 0; i--) b[i] = (b[i] - b[i+1]*a2[i])/a1[i];

	tau[i1].c2 = x1;

	for(i = i1+1, j = 1; i < i2; i++, j++)
		tau[i].c2 = c3[i]*b[j-1] + c4[i]*b[j] + c5[i]*b[j+1];

	tau[i2].c2 = xn;
}

void Trtm(libtau *h, float delta, int *pn, float *tt, float *ray_p, float *dtdd, float *dtdh, float *dddp, char **phnm);

void trtm(libtau *h, float delta, int *pn, float *tt, float *ray_p, float *dtdd, float *dtdh, float *dddp, char **phnm)
{
	h->D = 0;
	Trtm(h, delta, pn, tt, ray_p, dtdd, dtdh, dddp, phnm);
}

void trtm1(libtau *h, float delta, int *pn, float *tt, float *ray_p, float *dtdd, float *dtdh, float *dddp, char **phnm)
{
	h->D = 0;
	Trtm(h, delta, pn, tt, ray_p, dtdd, dtdh, dddp, phnm);
}

void trtm2(libtau *h, float delta, int *pn, float *tt, float *ray_p, float *dtdd, float *dtdh, float *dddp, char **phnm)
{
	h->D = 1;
	Trtm(h, delta, pn, tt, ray_p, dtdd, dtdh, dddp, phnm);
}

void Trtm(libtau *h, float delta, int *pn, float *tt, float *ray_p, float *dtdd, float *dtdh, float *dddp, char **phnm)
{
	float tmp1[200], tmp2[200], tmp3[200], tmp4[200], tmp5[200];
	int i, j, k, n, iptr[200];
	char *ctmp[200], cbuf[2000];
	double x[3];
	static const float atol = .005;
	static const double cn = .017453292519943296, dtol = 1.e-6,
		pi = 3.1415926535897932, pi2 = 6.2831853071795865;

	for(i = 0; i < 200; i++) ctmp[i] = cbuf+i*10;
	*pn = n = 0;
	if(h->depths[h->D].mbr2 < 0) return;
	x[0] = mod(fabs(cn*delta), pi2);
	if(x[0] > pi) x[0] = pi2 - x[0];
	x[1] = pi2 - x[0];
	x[2] = x[0] + pi2;
	if(fabs(x[0]) <= dtol)
	{
		x[0] = dtol;
		x[2] = -10.;
	}
	if(fabs(x[0] - pi) <= dtol)
	{
		x[0] = pi - dtol;
		x[1] = -10.;
	}
	for(j = h->depths[h->D].mbr1; j <= h->depths[h->D].mbr2; j++)
		if(h->depths[h->D].w[1].jndx[j] >= 0)
	{
		findtt(h, j, x, &n, tmp1, tmp2, tmp3, tmp4, tmp5, ctmp);
	}
	if(n <= 0)
	{
		*pn = n;
		return;
	}
	else if(n == 1)
	{
		iptr[0] = 0;
	}
	else
	{
		r4sort(n, tmp1, iptr);
	}
	for(i = k = 0; i < n; i++)
	{
		j = iptr[i];
		if(k == 0 || strcmp(phnm[k-1], ctmp[j]) ||
					fabs(tt[k-1]-tmp1[j]) > atol)
		{
			tt[k]    = tmp1[j];
			dtdd[k]  = tmp2[j];
			dtdh[k]  = tmp3[j];
			dddp[k]  = tmp4[j];
			ray_p[k] = tmp5[j];
			strcpy(phnm[k], ctmp[j]);
			k++;
		}
	}
	*pn = k;
}

#define MAX_BRANCH 10

void
get_seg(libtau *h, char *phase, int *npts, float *tt, float *delta, float *ray_p, int *n_branch)
{
	char *ctmp[200], cbuf[2000];
	float del, tmp1[20], tmp2[20], tmp3[20], tmp4[20], tmp5[20];
	int iptr[20];
	struct 
	{
		int n;
		float t[400], d[400], p[400];
	} b[MAX_BRANCH];
	char ph[10];
	int i, j, k, n, nbr;
	int jndex[jbrn], nj;
	double x[3];
	static const double cn = .017453292519943296, dtol = 1.e-6,
		pi = 3.1415926535897932, pi2 = 6.2831853071795865;

	h->D = 0;
	for(i = 0; i < 200; i++) ctmp[i] = cbuf+i*10;
	*npts = 0;
	*n_branch = 0;

	if(h->depths[h->D].mbr2 < 0) return;

	strcpy(ph, phase);
	if((n=strlen(ph)) >= 2 && !strcmp(ph+n-2, "bc"))
	{
		strcat(ph+n-2, "ab");
	}
	else if(n > 4 && !strcmp(ph+n-4, "diff"))
	{
		ph[n-4] = '\0';
	}
		
	for(j = h->depths[h->D].mbr1, nj = 0; j <= h->depths[h->D].mbr2; j++)
		if(h->depths[h->D].w[1].jndx[j] >= 0 && !strcmp(ph,h->depths[h->D].phcd[j]))
	{
		jndex[nj++] = j;
	}

	for(i = 0; i < MAX_BRANCH; i++) b[i].n = 0;
	nbr = 0;
	for(i = 0; i <= 360; i++)
	{
		del = i*.5;
		x[0] = mod(fabs(cn*del), pi2);
		if(x[0] > pi) x[0] = pi2 - x[0];
		x[1] = pi2 - x[0];
		x[2] = x[0] + pi2;
		if(fabs(x[0]) <= dtol)
		{
			x[0] = dtol;
			x[2] = -10.;
		}
		if(fabs(x[0] - pi) <= dtol)
		{
			x[0] = pi - dtol;
			x[1] = -10.;
		}
		
		for(k = n = 0; k < nj; k++)
		{
			j = jndex[k];
			findtt(h, j, x, &n, tmp1, tmp2, tmp3, tmp4, tmp5, ctmp);
		}
		if(n > MAX_BRANCH)
		{
			fprintf(stderr, "get_branch: n = %d\n",n);
			exit(1);
		}
		r4sort(n, tmp1, iptr);
			
		for(j = k = 0; j < n; j++) if(!strcmp(phase, ctmp[j]))
		{
			b[k].t[b[k].n] = tmp1[iptr[j]];
			b[k].d[b[k].n] = del;
			b[k].p[b[k].n] = tmp5[iptr[j]];
			b[k].n++;
			k++;
			if(k > nbr) nbr = k;
		}
	}
	
	for(k = 0; k < nbr; k++)
	{
		if(*npts > 0 && b[k].n > 0)
		{
			tt[*npts] = tt[*npts-1];
			/*
			 * set delta = NaN to denote a break
			 * in the branch.
			 */
			/* set_fnan(delta[*npts]); */
			delta[*npts] = -1; /* XXX */
			*npts += 1;
		}
		for(i = 0; i < b[k].n; i++)
		{
			tt[*npts] = b[k].t[i];
			delta[*npts] = b[k].d[i];
			ray_p[*npts] = b[k].p[i];
			*npts += 1;
		}
	}
	*n_branch = nbr;
}

static double
umod(libtau *h, double zs, int *src, int nph)
{
	int i;
	static const double dtol = 1.e-6;
	float dep;

	for(i = 1; i < h->mt[nph]; i++) if(h->v[nph].zm[i] <= zs) break;
	if(i == h->mt[nph])
	{
		dep = (1.0 - exp(zs))/h->xn;
		fprintf(stderr, "Source depth: %6.1f is too deep.\n", dep);
		exit(1);
	}
	if(fabs(zs - h->v[nph].zm[i]) > dtol ||
		fabs(h->v[nph].zm[i] - h->v[nph].zm[i+1]) > dtol)
	{
		src[nph] = i-1;
		return( h->v[nph].pm[i-1] +
		    (h->v[nph].pm[i]-h->v[nph].pm[i-1])*(exp(zs-h->v[nph].zm[i-1])-1.)/
			(exp(h->v[nph].zm[i]-h->v[nph].zm[i-1]) - 1.) );
	}
	src[nph] = i;
	return(h->v[nph].pm[i+1]);
}

static double
zmod(libtau *h, double uend, int js, int nph)
{
	double d;

	d = (uend - h->v[nph].pm[js]) * (exp(h->v[nph].zm[js+1]-h->v[nph].zm[js]) - 1.)/
		(h->v[nph].pm[js+1]-h->v[nph].pm[js]) + 1.;
	if(d < 1.e-30) d = 1.e-30;

	return(h->v[nph].zm[js] + log(d));
}

static char *
xindex(char *a, char *b)
{
	int i, n, len;

	len = strlen(b);
	n = strlen(a) - len;

	for(i = 0; i <= n; i++)
	{
		if(!strncmp(a+i, b, len)) return(a+i);
	}
	return(NULL);
}
