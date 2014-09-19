#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <libtau/limits.h>
/*
 * program setbrn
 */

extern void tauspl();

static void pdecx(int n1, int nph, float fac);
static void collct(int i1, int i2, double *x, float xmn);
static float varn(double *x, int k0, int k1, int k2, int kt, float xmn,
     double var, int m, int *mn);
static void layout();
static void mkdbr(int l1, int l2, int isgn, int lyr, int nph, int kph, double *fac);
static void mkubr(int l1, int isgn);
static void mkrbr(int l1, int isgn, int lyr, int nph, int kph, double *fac);
static void mkcbr(int l1, int l2, int isgn, int lyr, int nph, int kph, double *fac);
static void pdect(int i1, int i2, int j1, int nph, float fac);
static void kseq();
static void mseq();

static struct
{
	char    *code[nbr1];
	int 	ndx2[nsr0], loc[nsr0], oloc[nsr0];
	int     ndex[nsr0], lbrk[nbr2], lvz[nlvz0], indx[jseg], kndx[jseg],
		jndx[jbrn], mndx[jbrn], kuse[nsl1], midx[jbrn];
	double  pu[nsl1], px[jbrn], xt[jbrn], pux[jbrn], taul[nlvz0], xl[nlvz0];
	double  pm[nsr0], zm[nsr0], tmp[nsl1], xm[nsl1];
	struct 
	{
		double taup[nsl1], xp[nsl1];
	} t[3];
} v[2];

struct s_struct
{
	int nafl[jseg];
	float fcs[jseg];
} s[3];

static char *phcd[jbrn];
static char modnam[25], hedfile[32], tblfile[32];
static double pb[nsl1], pt[jout], taut[jout], xa[jout];
static double c1[jout], c2[jout], c3[jout], c4[jout], c5[jout];
static double deg, dtol = 1.e-6, zmax, zoc, zic, z0;
static int mt[2], lcb[2], lbb[2], kb[2], ku[2], lt[2], km[2] = {0, 0};
static int nseg, nbrn, nl;
static float xmin = 200.;

static FILE *fpin, *fp10, *fpout;

int main(int argc, char *argv[])
{
	char filespec[100];
	int i, j, k, l, m, n, nph, ndasr, nasgr, nrec, k1, m1, n1, ind,
	    len1, len0, len2;
	float xn, tn, pn;
	double d, cn;

	for(i = 0; i < nbr1; i++)
	{
		v[0].code[i] = (char *)malloc(10);
		v[1].code[i] = (char *)malloc(10);
	}
	for(i = 0; i < jbrn; i++) phcd[i] = (char *)malloc(10);

	deg = 180./3.1415927;

	if ( argc > 1 )
		snprintf(filespec, sizeof(filespec)-1, "remodl_%s.hed", argv[1]);
	else
		strcpy(filespec, "remodl.hed");

	fpin = fopen(filespec, "rb");
	if ( !fpin ) {
		fprintf(stderr, "could not open %s\n", filespec);
		exit(-1);
	}

	fread(&ndasr, 4, 1, fpin);
	fread(modnam, 1, 20, fpin);
	fread(&zmax, 8, 1, fpin);
	fread(&zoc, 8, 1, fpin);
	fread(&zic, 8, 1, fpin);
	fread(kb, 4, 2, fpin);
	fread(pb, 8, kb[1], fpin);
	fread(mt, 4, 2, fpin);
	fread(lt, 4, 2, fpin);
	fread(lbb, 4, 2, fpin);
	fread(lcb, 4, 2, fpin);
	fread(&xn, 4, 1, fpin);
	fread(&pn, 4, 1, fpin);
	fread(&tn, 4, 1, fpin);
	for(nph = 0; nph < 2; nph++)
	{
		fread(v[nph].lbrk, 4, lbb[nph], fpin);
		for(i = 0; i < lcb[nph]; i++)
			fread(v[nph].code[i], 8, 1, fpin);
		fread(v[nph].zm,   8, mt[nph], fpin);
		fread(v[nph].pm,   8, mt[nph], fpin);
		fread(v[nph].ndex, 4, mt[nph], fpin);
		fread(v[nph].loc,  4, mt[nph], fpin);
		fread(v[nph].lvz,  4, lt[nph], fpin);
		fread(v[nph].taul, 8, lt[nph], fpin);
		fread(v[nph].xl,   8, lt[nph], fpin);
	}
	fclose(fpin);

	printf("ndasr = %d  modnam = %s\n", ndasr, modnam);

	if ( argc > 1 )
		snprintf(filespec, sizeof(filespec)-1, "remodl_%s.tbl", argv[1]);
	else
		strcpy(filespec, "remodl.tbl");

	fpin = fopen(filespec, "rb");
	if(!fpin)
	{
		fprintf(stderr, "could not open %s\n", filespec);
		exit(-1);
	}

	nrec = 0;
	for(nph = 0; nph < 2; nph++)
	{
		n1 = kb[nph];
		ind = 0;
		for(k = 0; k < n1; k++) v[nph].xm[k] = 0.;
		do {
			for(;;)
			{
				nrec++;
				fread(&z0, 8, 1, fpin);
				fread(&n, 4, 1, fpin);
				fread(v[0].tmp, 8, n, fpin);
				fread(v[1].tmp, 8, n, fpin);
				if(ind > 0 || fabs(z0-zoc) <= dtol) break;
				for(k = 1; k < n; k++)
				{
					d = fabs(v[1].tmp[k-1] - v[1].tmp[k]);
					if(d > v[nph].xm[k]) v[nph].xm[k] = d;
				}
				if(n+1 == n1) v[nph].xm[n1-1] = v[1].tmp[n-1];
			}
			ind++;
			for(k = 0; k < n; k++)
			{
				v[nph].t[ind-1].taup[k] = v[0].tmp[k];
				v[nph].t[ind-1].xp[k]   = v[1].tmp[k];
			}
		} while(ind < 3);
	}
	xmin *= xn;

	fp10 = fopen("setbrn1.lis", "w");
	fprintf(fp10,"kb mt lt lbb lcb %d %d %d %d %d %d %d %d %d %d\n",kb[0],
		kb[1],mt[0],mt[1],lt[0],lt[1],lbb[0],lbb[1],lcb[0],lcb[1]);
	fprintf(fp10, "xn pn tn xmin %E %E %E %E\n", xn,pn,tn,xmin);
	cn = 1./xn;
	for(i = 0; i < lbb[0]; i++)
	{
		fprintf(fp10, "%5d%5d  %s %5d %s\n",i, v[0].lbrk[i],
			v[0].code[i], v[1].lbrk[i], v[1].code[i]);
	}
	for(i = lbb[0]; i < lbb[1]; i++)
	{
		fprintf(fp10, "%5d                %5d  %s\n", i, v[1].lbrk[i],
			v[1].code[i]);
	}
	for(i = 0; i < mt[0]; i++)
	{
		fprintf(fp10, " %5d%12.6f%12.6f%5d  %12.6f%12.6f%5d\n", i,
			v[0].zm[i], v[0].pm[i], v[0].ndex[i],
			v[1].zm[i], v[1].pm[i], v[1].ndex[i]);
	}
	for(i = mt[0]; i < mt[1]; i++)
	{
	   fprintf(fp10, " %5d                               %12.6f%12.6f%5d\n",
			i, v[1].zm[i], v[1].pm[i], v[1].ndex[i]);
	}
	for(i = 0; i < lt[0]; i++)
		fprintf(fp10, " %5d%5d%5d%12.6f%12.2f\n",
			0, i, v[0].lvz[i], v[0].taul[i], deg*v[0].xl[i]);
	for(i = 0; i < lt[1]; i++)
		fprintf(fp10, " %5d%5d%5d%12.6f%12.2f\n",
			1, i, v[1].lvz[i], v[1].taul[i], deg*v[1].xl[i]);
	for(i = 0; i < kb[0]; i++)
		fprintf(fp10, "     %5d%12.6f%12.2f%12.2f\n", i, (float)pb[i],
			(float)(cn*v[0].xm[i]), (float)(cn*v[1].xm[i]));
	for(i = kb[0]; i < kb[1]; i++)
		fprintf(fp10, "     %5d%12.6f            %12.2f\n",
			i, (float)pb[i], (float)(cn*v[1].xm[i]));
	fclose(fp10);

	fp10 = fopen("setbrn2.lis", "w");

	for(nph = 0; nph < 2; nph++)
	{
		n1 = kb[nph];
		for(i = 1; i < n1; i++)
		{
			v[nph].xm[i] += v[nph].xm[i-1];
			v[nph].pu[i] = pb[i];
			v[nph].kuse[i] = -1;
		}
		for(i = 0; i < n1; i++)
		{
			v[nph].t[2].taup[i] -= v[nph].t[1].taup[i];
			v[nph].t[2].xp[i]   -= v[nph].t[1].xp[i];
			v[nph].t[1].taup[i] -= v[nph].t[0].taup[i];
			v[nph].t[1].xp[i]   -= v[nph].t[0].xp[i];
		}
	}
	pdecx(kb[0], 0, 2.);
	pdecx(kb[1], 1, 2.);

	fprintf(fp10, "ku %d %d\n", ku[0], ku[1]);
	for(i = 0; i < ku[0]; i++)
		fprintf(fp10, "%5d%12.6f%12.2f%12.2f%12.6f%12.2f%12.2f\n", i,
		(float)v[0].pu[i], (float)(cn*v[0].xm[i]),
		(float)(cn*(v[0].xm[i+1]-v[0].xm[i])),
		(float)v[1].pu[i], (float)(cn*v[1].xm[i]),
		(float)(cn*(v[1].xm[i+1]-v[1].xm[i])));
	for(i = ku[0]; i < ku[1]; i++)
		fprintf(fp10, "%5d   %12.6f%12.2f%12.2f\n",i,(float)v[1].pu[i],
		(float)(cn*v[1].xm[i]),(float)(cn*(v[1].xm[i+1] - v[1].xm[i])));
	for(nph = 0; nph < 2; nph++)
	{
	   for(i = 0; i < kb[nph]; i++)
	      fprintf(fp10, "%5d%10.6f%10.6f%10.6f%10.6f%10.2f%10.2f%10.2f\n",
		i, (float)pb[i], (float)v[nph].t[0].taup[i],
		(float)v[nph].t[1].taup[i], (float)v[nph].t[2].taup[i],
		(float)(deg*v[nph].t[0].xp[i]), (float)(deg*v[nph].t[1].xp[i]),
		(float)(deg*v[nph].t[2].xp[i]));
	}
	
	layout();
	for(nph = 0; nph < 2; nph++)
	{
		for(i = k = 0; i < kb[nph]; i++)
		{
			if(v[nph].kuse[i] >= 0)
				v[nph].pu[k++] = pb[i];
		}
		ku[nph] = k;
	}
	kseq();
	mseq();

	for(i = 0; i < ku[0]; i++)
		fprintf(fp10, "%5d%12.6f%12.6f\n", i, (float)v[0].pu[i],
			(float)v[1].pu[i]);
	for(i = ku[0]; i < ku[1]; i++)
		fprintf(fp10, "%5d     %12.6f\n", i, (float)v[1].pu[i]);
	for(i = 0; i < nseg; i++)
	   fprintf(fp10, " %6d%6d%6d%6d%6d%6d%6d%6d%6.1f%6.1f%6.1f\n", i,
		s[0].nafl[i],s[1].nafl[i],s[2].nafl[i],v[0].indx[i],
		v[1].indx[i],v[0].kndx[i],v[1].kndx[i],s[0].fcs[i],
		s[1].fcs[i],s[2].fcs[i]);

	for(i = 0; i < nbrn; i++)
		fprintf(fp10," %3d%5d%5d%5d%5d%12.6f%12.6f%10.2f%10.2f  %s\n",
		i,v[0].jndx[i],v[1].jndx[i],v[0].mndx[i],v[1].mndx[i],
		(float)v[0].px[i], (float)v[1].px[i], (float)(deg*v[0].xt[i]),
		(float)(deg*v[1].xt[i]), phcd[i]);
	j = (km[0] > km[1]) ? km[0] : km[1];
	for(i = 0; i < j; i++) fprintf(fp10, " %3d%5d%5d%12.6f%12.6f\n",i,
	   v[0].midx[i],v[1].midx[i],(float)v[0].pux[i], (float)v[1].pux[i]);
	for(i = 0; i < nl; i++)
		fprintf(fp10, " %4d %E %E %E %E %E %E %E %E %E\n",
			i, pt[i], taut[i], deg*xa[i], cn*(xa[i]-xa[i+1]),
			c1[i], c2[i], c3[i], c4[i], c5[i]);
	fclose(fp10);

	fp10 = fopen("setbrn3.lis", "w");

	for(nph = 0; nph < 2; nph++)
	{
		mt[nph] -= 3;
		ku[nph] -= 1;
		km[nph] -= 1;
	}
	/* icor = 33  -  originally 32 records used as header in setbrn
	 * and 2 records used as header in remodl.
	 */
	for(i = 1; i < mt[0]; i++) v[0].ndx2[i] = v[0].ndex[i];
	for(i = 1; i < mt[1]; i++) v[1].ndx2[i] = v[1].ndex[i] - 3;
	len1 = ku[1] + km[1];
	len0 = 8*len1;
	len2 = 5*nl;
	fprintf(fp10,
		"nseg nbrn mt ku km len len1 %d %d %d %d %d %d %d %d %d %d\n",
		nseg, nbrn, mt[0], mt[1], ku[0], ku[1], km[0], km[1],len0,len1);
	nasgr = len0;
	/*printf("reclength for direct access %d\n", nasgr);*/

	strcpy(hedfile, modnam);
	strcat(hedfile, ".hed");
	strcpy(tblfile, modnam);
	strcat(tblfile, ".tbl");
	printf("header file : %s\n", hedfile);
	printf("table file : %s\n", tblfile);
	
	fpout = fopen(tblfile, "wb");

	nrec = 0;
	for(nph = 0; nph < 2; nph++)
	{
		m1 = mt[nph];
		n1 = ku[nph];
		k1 = km[nph];
		fprintf(fp10, "nph m1 n1 k1 %d %d %d %d\n", nph, m1, n1, k1);

		v[nph].oloc[0] = 0;
		for(m = 1; m < m1; m++)
		{
			if(v[nph].ndex[m] != v[nph].ndex[m-1])
			{
				fseek(fpin, v[nph].loc[m], 0);
				fread(&z0, 8, 1, fpin);
				fread(&n, 4, 1, fpin);
				fread(v[0].tmp, 8, n, fpin);
				fread(v[1].tmp, 8, n, fpin);
				fprintf(fp10, "m nph ndex n %d %d %d %d\n",
				        m, nph, v[nph].ndex[m], n);
		
				for(i = k = l = 0; i < n; i++) {
					if(v[nph].kuse[i] >= 0)
					{
						if(fabs(v[nph].pux[l] - pb[i]) <= dtol)
						{
							v[1].tmp[l++] = v[1].tmp[i];
						}
						v[0].tmp[k++] = v[0].tmp[i];
					}
				}
				fprintf(fp10, "k l nrec %d %d %d %d %E\n",
				k, l, nrec+1, v[nph].ndx2[m], v[0].tmp[0]);

				for(; k < n1; k++) v[0].tmp[k] = 0.;
				for(; l < k1; l++) v[1].tmp[l] = 0.;

				v[nph].oloc[m] = ftell(fpout);
				fwrite(v[0].tmp, 8, n1, fpout);
				fwrite(v[1].tmp, 8, k1, fpout);
				nrec++;
			}
			else
			{
				v[nph].oloc[m] = v[nph].oloc[m-1];
			}
		}
	}
	fclose(fpout);

	fpout = fopen(hedfile, "wb");
	fwrite(&nasgr, 4, 1, fpout); fwrite(&nl, 4, 1, fpout);
	fwrite(&len2, 4, 1, fpout); fwrite(&xn, 4, 1, fpout);
	fwrite(&pn, 4, 1, fpout); fwrite(&tn, 4, 1, fpout);
	fwrite(mt, 4, 2, fpout); fwrite(&nseg, 4, 1, fpout);
	fwrite(&nbrn, 4, 1, fpout); fwrite(ku, 4, 2, fpout);
	fwrite(km, 4, 2, fpout); fwrite(s, sizeof(struct s_struct), 3, fpout);
	fwrite(v[0].indx, 4, jseg, fpout); fwrite(v[1].indx, 4, jseg, fpout);
	fwrite(v[0].kndx, 4, jseg, fpout); fwrite(v[1].kndx, 4, jseg, fpout);
	fwrite(v[0].pm, 8, nsr0, fpout); fwrite(v[1].pm, 8, nsr0, fpout);
	fwrite(v[0].zm, 8, nsr0, fpout); fwrite(v[1].zm, 8, nsr0, fpout);
	fwrite(v[0].ndx2, 4, nsr0, fpout); fwrite(v[1].ndx2, 4, nsr0, fpout);
	fwrite(v[0].oloc, 4, nsr0, fpout); fwrite(v[1].oloc, 4, nsr0, fpout);
	fwrite(v[0].pu, 8, nsl1, fpout); fwrite(v[1].pu, 8, nsl1, fpout);
	fwrite(v[0].pux, 8, jbrn, fpout); fwrite(v[1].pux, 8, jbrn, fpout);
	for(i = 0; i < jbrn; i++) fwrite(phcd[i], 8, 1, fpout);
	fwrite(v[0].px, 8, jbrn, fpout); fwrite(v[1].px, 8, jbrn, fpout);
	fwrite(v[0].xt, 8, jbrn, fpout); fwrite(v[1].xt, 8, jbrn, fpout);
	fwrite(v[0].jndx, 4, jbrn, fpout); fwrite(v[1].jndx, 4, jbrn, fpout);
	fwrite(pt, 8, jout, fpout); fwrite(taut, 8, jout, fpout);
	fwrite(c1, 8, jout, fpout); fwrite(c2, 8, jout, fpout);
	fwrite(c3, 8, jout, fpout); fwrite(c4, 8, jout, fpout);
	fwrite(c5, 8, jout, fpout);
	fclose(fpout);
	return 0;
}

static void
pdecx(int n1, int nph, float fac)
{
	int i, j, k, i1, m;
	static double ptol = .03, pa, pax, plim;

	collct(0, n1-1, v[nph].xm, fac*xmin);
	k = -1;
	plim = .7*v[nph].pu[n1-1];
	for(i = 0; i < n1; i++)  if(v[nph].xm[i] >= 0.)
	{
		if(v[nph].pu[i] >= plim && (v[nph].pu[i]-v[nph].pu[k]) > ptol)
		{
			pa = v[nph].pu[k] + .75*(v[nph].pu[i] - v[nph].pu[k]);
			pax = 1.e+10;
			for(j = i1, m = -1; j <= i; j++)
				if(fabs(v[nph].pu[j]-pa) < pax)
			{
				m = j;
				pax = fabs(v[nph].pu[j] - pa);
			}
			if(m != i1 && m != i)
			{
				k++;
				v[nph].pu[k] = v[nph].pu[m];
				v[nph].xm[k] = 0.;
				v[nph].kuse[m] = 1;
			}
		}
		i1 = i;
		k++;
		v[nph].pu[k] = v[nph].pu[i];
		v[nph].xm[k] = v[nph].xm[i];
		v[nph].kuse[i] = 1;
	}
	ku[nph] = k+1;
}

#define amin1(a,b) (((a) <= (b)) ? a : b)

static void
collct(int i1, int i2, double *x, float xmn)
{
	int i, m, is, ie, k0, k1, k2, ks, kb, nch, m1, m2;
/*	static float cn = 6371.; // unused */
	double v1, v2, var, var1, var2, dx1, dx2;

	is = i1+1;
	ie = i2-1;
	if(ie < is) return;
	k1 = i1;
	var = 0.;
	m = 0;
	for(i = is; i <= ie; i++)
	{
		dx1 = fabs(x[k1]-x[i]) - xmn;
		dx2 = fabs(x[k1]-x[i+1]) - xmn;
		if(fabs(dx2) < fabs(dx1))
		{
			x[i] = -x[i];
		}
		else
		{
			if(k1 <= i1) kb = i;
			k1 = i;
			var += dx1*dx1;
			m++;
		}
	}
	dx1 = fabs(x[k1]-x[i2]) - xmn;
	var += dx1*dx1;
	m++;
	do
	{
		if(m <= 1) return;
		k1 = i1;
		k2 = kb;
		ks = kb + 1;
		nch = 0;
		for(i = ks; i <= i2; i++) if(x[i] >= 0.)
		{
			k0 = k1;
			k1 = k2;
			k2 = i;
			var1 = varn(x, k0, k1, k2, k1-1, xmn, var, m, &m1);
			var2 = varn(x, k0, k1, k2, k1+1, xmn, var, m, &m2);
			v1 = var1/m1;
			v2 = var2/m2;
			if(amin1(v1,v2) < var/m)
			{
				nch++;
				x[k1] = -x[k1];
				if(v1 < v2 || (v1 == v2 && m1 <= m2))
				{
					k1--;
					x[k1] = fabs(x[k1]);
					var = var1;
					m = m1;
				}
				else if(v1 > v2 || (v1 == v2 && m1 > m2))
				{
					k1++;
					x[k1] = fabs(x[k1]);
					var = var2;
					m = m2;
				}
			}
			if(k0 == i1) kb = k1;
		}
	} while(nch > 0);
}

static float
varn(double *x, int k0, int k1, int k2, int kt, float xmn,
     double var, int m, int *mn)
{
	double dx1, dx2, v;

	dx1 = fabs(x[k0] - x[k1]) - xmn;
	dx2 = fabs(x[k1] - x[k2]) - xmn;
	v = var - dx1*dx1 - dx2*dx2;
	if(kt > k0 && kt < k2)
	{
		dx1 = fabs(x[k0] - fabs(x[kt])) - xmn;
		dx2 = fabs(fabs(x[kt]) - x[k2]) - xmn;
		v += dx1*dx1 + dx2*dx2;
		*mn = m;
		return(v);
	}
	else
	{
		dx1 = fabs(x[k0] - fabs(x[k2])) - xmn;
		v += dx1*dx1;
		*mn = m-1;
		return(v);
	}
}

static void
layout()
{
	/*
	 * Layout contains the program for the desired travel-time segments
	 * implemented as calls to the mk_br entry points.  Each call does
	 * one segment (which may have many branches).
	 */
	static double dir[3] = {1., 1., 1.}, cref[3] = {1., 2., 2.},
			sref[3] = {2., 2., 2.};
	int i, j;

	/* Initialize variables.
	 */
	nseg = 0;
	nbrn = 0;
	nl = 0;
	for(j = 0; j < 3; j++)
	{
		for(i = 0; i < jseg; i++) s[j].fcs[i] = 0.;
	}
	for(i = 0; i < jout; i++) taut[i] = 0.;
	for(i = 0; i < jbrn; i++) v[0].xt[i] = v[1].xt[i] = 0.;
	/*
	 * Do all of the segments.;
	 */
	printf("Layout:  do Pup\n");
	mkubr(ku[0],      1);			/* P (up-going branch) */
	printf("Layout:  do P and PKP\n");
	mkdbr(0, lbb[0]-1, -1, 3, 0, 0, dir);	/* P, Pdiff, and PKP */
	printf("Layout:  do PKiKP\n");
	mkrbr(1,         -1, 2, 0, 0, dir);	/* PKiKP */
	printf("Layout:  do pP\n");
	mkdbr(0, lbb[0]-1,  1, 3, 0, 0, dir);	/* pP */
	printf("Layout:  do sP\n");
	mkdbr(0, lbb[0]-1,  2, 3, 0, 0, dir);	/* sP */
	printf("Layout:  do pPKiKP\n");
	mkrbr(1,          1, 2, 0, 0, dir);	/* pPKiKP */
	printf("Layout:  do sPKiKP\n");
	mkrbr(1,          2, 2, 0, 0, dir);	/* sPKiKP */
	printf("Layout:  do PcP\n");
	mkrbr(2,         -1, 1, 0, 0, dir);	/* PcP */
	printf("Layout:  do ScP\n");
	mkrbr(2,         -2, 1, 1, 0, dir);	/* ScP */
	printf("Layout:  do SKP\n");
	mkdbr(0,      2, -2, 3, 1, 0, dir);	/* SKP */
	printf("Layout:  do SKiKP\n");
	mkrbr(1,         -2, 2, 1, 0, dir);	/*SKiKP */
	printf("Layout:  do PKKP\n");
	mkdbr(0,      2, -1, 3, 0, 0, cref);	/* PKKP */
	printf("Layout:  do SKKP\n");
	mkdbr(0,      2, -2, 3, 1, 0, cref);	/* SKKP */
	printf("Layout:  do PP, P''P''\n");
	mkdbr(0, lbb[0]-1, -1, 3, 0, 0, sref);	/* PP and P'P' */
	printf("Layout:  do Sup\n");
	mkubr(ku[1],      2);			/* S (up-going branch) */
	printf("Layout:  do S and SKS\n");
	mkdbr(0, lbb[1]-1, -2, 3, 1, 1, dir);	/* S,  Sdiff,  and SKS */
	printf("Layout:  do pS\n");
	mkdbr(0, lbb[0]-1,  1, 3, 1, 1, dir);	/* pS */
	printf("Layout:  do sS\n");
	mkdbr(0, lbb[1]-1,  2, 3, 1, 1, dir);	/* sS */
	printf("Layout:  do ScS\n");
	mkrbr(3,         -2, 1, 1, 1, dir);	/* ScS */
	printf("Layout:  do PcS\n");
	mkrbr(2,         -1, 1, 0, 1, dir);	/* PcS */
	printf("Layout:  do PKS\n");
	mkdbr(0,      2, -1, 3, 0, 1, dir);	/* PKS */
	printf("Layout:  do PKKS\n");
	mkdbr(0,      2, -1, 3, 0, 1, cref);	/* PKKS */
	printf("Layout:  do SKKS\n");
	mkdbr(0,      2, -2, 3, 1, 1, cref);	/* SKKS */
	printf("Layout:  do SS and S''S''\n");
	mkdbr(0, lbb[1]-1, -2, 3, 1, 1, sref);	/* SS and S'S' */
	printf("Layout:  do SP\n");
	mkcbr(3, lbb[0]-1, -2, 1, 1, 0, sref);	/* SP */
	printf("Layout:  do PS\n");
	mkcbr(3, lbb[0]-1, -1, 1, 0, 1, sref);	/* PS */
}

static void
mkdbr(int l1, int l2, int isgn, int lyr, int nph, int kph, double *fac)
{
/*
 * Mkdbr sets up a simple refracted wave segment.  L1 and l2 point to the 
 * lbrk array of slowness break point pointers.  Note that the P and S 
 * break point arrays don't necessarily line up layer by layer.  This is 
 * not generally a problem as most phases need only worry about the 
 * pointer to the surface slowness for one wave type and a pointer 
 * somewhere in the core (which is constrained to be the same for both 
 * P and S).  Isgn is positive if the wave starts out going up and 
 * negative if the wave starts out going down.  Iabs(isng) is 1 if the 
 * wave starts out as a P wave and 2 if the wave starts out as an S wave.  
 * Lyr gives the number of major layers (mantle, outer core, and inner 
 * core) that the wave penetrates.  Nph and kph give the wave type (1 for 
 * P and 2 for S) on the down-going and up-going legs of the ray path 
 * respectively.  Fac is a three element array giving the number of 
 * repeats of the ray path in each major layer.  This scheme incorporates 
 * turning rays (e.g., P and S), turning rays reflected, but not 
 * converted at the surface (e.g., PP and SS), up-going rays reflected 
 * and/or converted at the surface into turning rays (e.g., pP and sP), 
 * turning rays converted during transmission through an interface (e.g., 
 * SKP and PKS), and rays which turn multiple times while reflecting from 
 * the bottom side of a layer (e.g., PKKP or SKKP).  Mkdbr does not 
 * include up-going rays (to the receiver), rays reflected from the top 
 * side of a discontinuity, or rays which are reflected and converted at 
 * the free surface.  See mkubr, mkrbr, and mkcbr respectively for 
 * routines which handle these types of rays.
 */
	static char ks[10] = "KKKKKKKK";
	char tmp[10];
	int i, j, l, lz1, lz2, k, m, nt, ind, ind1;
	double xfc;

	/* Remember the programming as part of the final phase construction
	 * is done in depcor.
	 */
	s[0].nafl[nseg] = isgn;
	s[1].nafl[nseg] = nph+1;
	s[2].nafl[nseg] = kph+1;
	v[0].indx[nseg] = nl;
	v[0].kndx[nseg] = 0;
	/*
	 * Using l1 and l2 to get the breakpoints has some shortcommings,
	 * particularly for converted phases.  It would be more general to
	 * have separate indicies for the breakpoints and the layers covered.
	 */
	if(l1 > 0) v[0].kndx[nseg] = v[nph].lbrk[l1-1];
	v[1].kndx[nseg] = (v[nph].lbrk[l2] < v[kph].lbrk[l2]) ?
				v[nph].lbrk[l2] : v[kph].lbrk[l2];
	if(v[1].kndx[nseg] > v[abs(isgn)-1].lbrk[l2])
			v[1].kndx[nseg] = v[abs(isgn)-1].lbrk[l2];
	printf("Mkdbr:  l1 l2 isgn lyr nph kph  = %d %d %d %d %d %d\n",
		l1, l2, isgn, lyr, nph, kph);
	printf("Mkdbr:  nseg kndx indx  = %d %d %d %d\n",
		nseg, v[0].kndx[nseg], v[1].kndx[nseg], v[0].indx[nseg]);
	xfc = 0.;
	for(m = 0; m < lyr; m++)
	{
		s[m].fcs[nseg] = fac[m];
		if(s[m].fcs[nseg] > xfc) xfc = s[m].fcs[nseg];
	}
	/*
	 * Set up the required slownesses, taus and distances.
	 */
	j = v[0].kndx[nseg];
	lz1 = 0;
	lz2 = 0;
	/* Loop over the layers of interest.
	 */
	for(i = l1; i <= l2; i++)
	{
		/* Be sure that the phase cuts off at the right place.
		 */
		l = (v[nph].lbrk[i] < v[1].kndx[nseg]) ?
				v[nph].lbrk[i] : v[1].kndx[nseg];
		/* Skip all total internal reflections.
		 */
		if(v[nph].code[i][0] != 'r' && j < l)
		{
			/* Set the starting branch pointer.
			 */
 			nt = nl;
			v[0].jndx[nbrn] = nt;
			/* Copy in the desired slownesses.
			 */
			for(k = j; k <= l; k++)
			{
				pt[nl] = pb[k];
				/* Add up the tau contributions.
				 */
				for(m = 0; m < lyr; m++)
					taut[nl] += fac[m]*(v[nph].t[m].taup[k]
							+ v[kph].t[m].taup[k]);
				nl++;
			}
			/* Take care of branch end pointers and slownesses.
			 */
			v[0].mndx[nbrn] = j;
			v[1].mndx[nbrn] = l;
			v[0].px[nbrn] = pb[j];
			v[1].px[nbrn] = pb[l];
			/*
		       	 * Add up distance contributions for the branch end
			 * points only.
			 */
			for(m = 0; m < lyr; m++)
			{
				v[0].xt[nbrn] += fac[m]*(v[nph].t[m].xp[j]
							+ v[kph].t[m].xp[j]);
				v[1].xt[nbrn] += fac[m]*(v[nph].t[m].xp[l]
							+ v[kph].t[m].xp[l]);
			}
			/* Take care of the contribution due to low velocity
			 * zones for the down-going leg(s).
			 */
			if(lz1 < lt[nph] && j == v[nph].lvz[lz1])
			{
				for(m = 0; m < lyr; m++)
				{
					taut[nt] -= fac[m]*v[nph].t[m].taup[j];
					v[0].xt[nbrn] -=
						fac[m]*v[nph].t[m].xp[j];
				}
				taut[nt] += fac[0]*v[nph].taul[lz1];
				v[0].xt[nbrn] += fac[0]*v[nph].xl[lz1];
				lz1++;
			}
			/* Take care of the contributions due to low velocity
			 * zones for the up-going leg(s).
			 */
			if(lz2 < lt[kph] && j == v[kph].lvz[lz2])
			{
				for(m = 0; m < lyr; m++)
				{
					taut[nt] -= fac[m]*v[kph].t[m].taup[j];
					v[0].xt[nbrn] -=
						fac[m]*v[kph].t[m].xp[j];
				}
				taut[nt] += fac[0]*v[kph].taul[lz2];
				v[0].xt[nbrn] += fac[0]*v[kph].xl[lz2];
				lz2++;
			}
			/* Decimate the slownesses if the branch is oversampled
			 * in distance.
			 */
			pdect(v[0].jndx[nbrn], nl-1, j, abs(isgn)-1, xfc);
			/*
			 *  Set up the interpolation.
			 */
			tauspl(v[0].jndx[nbrn], nl-1, pt, c1, c2, c3, c4, c5);
			/* 
			 * Remember the final branch end slowness value.
			 */
			v[1].jndx[nbrn] = nl-1;
			/*
			 * Take care of the branch name.  Set up a default.
			 */
			phcd[nbrn][0] = v[nph].code[i][1];
			printf("initial phase[%d, %d]: %s\n", kph, i, v[kph].code[i]+2);
			strcpy(phcd[nbrn]+1, v[kph].code[i]+2);
			if((int)(fac[0] + .5) > 1)
			{
				/* Re-do the name if the ray is reflected from
				 * the surface.
				 */
				if(v[nph].code[i][2] == '\0')
				{
					phcd[nbrn][0] = v[nph].code[i][1];
					strcpy(phcd[nbrn]+1, v[kph].code[i]+1);
				}
	      			else if(v[nph].code[i][2] != 'K')
				{
/*					warning: statement with no effect
					strncmp(phcd[nbrn],v[nph].code[i]+1, 2);
*/
					strcpy(phcd[nbrn]+2, v[kph].code[i]+1);
				}
	      			if(v[nph].code[i][2] == 'K')
				{
					phcd[nbrn][0] = v[nph].code[i][1];
					phcd[nbrn][1] = '\'';
					phcd[nbrn][2] = v[kph].code[i][1];
					phcd[nbrn][3] = '\'';
					strcpy(phcd[nbrn]+4, v[kph].code[i]+4);
				}
			}
			else if((int)(fac[1] + .5) > 1)
			{
				/* Re-do the name if the ray is reflected from
				 * the underside of the core-mantle boundary.
				 */
				ind = (int)(fac[1] - .5);
				phcd[nbrn][0] = v[nph].code[i][1];
				strncpy(phcd[nbrn]+1, ks, ind);
				strcpy(phcd[nbrn]+1+ind, v[kph].code[i]+2);
			}
			/* Take care.
			 */
			for(ind1 = 0; phcd[nbrn][ind1] != '\0'; ind1++)
				if(!strncmp(phcd[nbrn]+ind1, "KSab", 4)) break;
			if(phcd[nbrn][ind1] == '\0') ind1 = -1;
			for(ind = 0; phcd[nbrn][ind] != '\0'; ind++)
				if(!strncmp(phcd[nbrn]+ind, "S'ab", 4)) break;
			if(phcd[nbrn][ind] == '\0') ind = -1;
			if(ind1 > ind) ind = ind1;
			if(phcd[nbrn][0] == 'S' && ind >= 0)
				strcpy(phcd[nbrn]+ind+2, "ac");
			if(isgn == 1)
			{
				tmp[0] = 'p';
				printf("phase: %s\n", phcd[nbrn]);
				strcpy(tmp+1, phcd[nbrn]);
				strcpy(phcd[nbrn], tmp);
			}
			else if(isgn == 2)
			{
				tmp[0] = 's';
				printf("phase: %s\n", phcd[nbrn]);
				strcpy(tmp+1, phcd[nbrn]);
				strcpy(phcd[nbrn], tmp);
			}
			nbrn++;
		}
		j = l;
	}
	v[1].indx[nseg] = nl-1;
	nseg++;
}

static void
mkubr(int l1, int isgn)
{
/*
 * Mkubr handles up-going P and S.  L1 and isgn are as for mkdbr (except
 * that l1 actually plays the role of l2 with the beginning break point
 * assumed to be zero).  The other arguments are not needed.
 */
	int k, l;

	s[0].nafl[nseg] = isgn;
	s[1].nafl[nseg] = 0;
	s[2].nafl[nseg] = 0;
	v[0].indx[nseg] = nl;
	v[0].kndx[nseg] = 0;
	l = kb[abs(isgn)-1] - 1;
	v[1].kndx[nseg] = l;
	printf("Mkubr:  l1 isgn  = %d %d\n", l1, isgn);
	printf("Mkubr:  nseg kndx indx  = %d %d %d %d\n",
		nseg, v[0].kndx[nseg], v[1].kndx[nseg], v[0].indx[nseg]);
	v[0].jndx[nbrn] = nl;
	for(k = 0; k < l1; k++, nl++)
	{
		pt[nl] = v[abs(isgn)-1].pu[k];
		xa[nl] = 0.0;
	}
	v[0].mndx[nbrn] = 0;
	v[1].mndx[nbrn] = l;
	v[0].px[nbrn] = pb[0];
	v[1].px[nbrn] = pb[l];
	tauspl(v[0].jndx[nbrn], nl-1, pt, c1, c2, c3, c4, c5);
	v[1].jndx[nbrn] = nl-1;
	phcd[nbrn][0] = v[abs(isgn)-1].code[0][1];
	phcd[nbrn][1] = '\0';
	v[1].indx[nseg] = nl-1;
	nbrn++;
	nseg++;
}

static void
mkrbr(int l1, int isgn, int lyr, int nph, int kph, double *fac)
{
/* Mkrbr handles reflected phases possibly with a conversion such as 
 * PcP, PcS, and PkiKP.  Arguments are as for mkdbr (except that l1
 * actually plays the role of l2 with the beginning break point assumed
 * to be zero).
 */
	char tmp[10];
	int l, k, m;
	double xfc;

	s[0].nafl[nseg] = isgn;
	s[1].nafl[nseg] = nph+1;
	s[2].nafl[nseg] = kph+1;
	v[0].indx[nseg] = nl;
	v[0].kndx[nseg] = 0;
	l = (v[nph].lbrk[l1] < v[kph].lbrk[l1]) ?
		v[nph].lbrk[l1] : v[kph].lbrk[l1];
	v[1].kndx[nseg] = l;
	printf("Mkrbr:  l1 isgn lyr nph kph = %d %d %d %d %d\n",
		l1, isgn, lyr, nph, kph);
	printf("Mkrbr:  nseg kndx indx  = %d %d %d %d\n", nseg,
		v[0].kndx[nseg], v[1].kndx[nseg], v[0].indx[nseg]);
	xfc = 0.;
	for(m = 0; m < lyr; m++)
	{
		s[m].fcs[nseg] = fac[m];
		if(xfc < s[m].fcs[nseg]) xfc = s[m].fcs[nseg];
	}
	if(lyr >= 2) xfc = 2.;

	v[0].jndx[nbrn] = nl;
	for(k = 0; k <= l; k++, nl++)
	{
		pt[nl] = pb[k];
		for(m = 0; m < lyr; m++)
			taut[nl] += fac[m]*(v[nph].t[m].taup[k] +
					v[kph].t[m].taup[k]);
	}
	v[0].mndx[nbrn] = 0;
	v[1].mndx[nbrn] = l;
	v[0].px[nbrn] = pb[0];
	v[1].px[nbrn] = pb[l];
	for(m = 0; m < lyr; m++) v[1].xt[nbrn] += fac[m]*(v[nph].t[m].xp[l] +
					v[kph].t[m].xp[l]);
	pdect(v[0].jndx[nbrn], nl-1, 0, abs(isgn)-1, xfc);
	tauspl(v[0].jndx[nbrn], nl-1, pt, c1, c2, c3, c4, c5);
	v[1].jndx[nbrn] = nl-1;
	if(lyr == 1)
	{
		phcd[nbrn][0] = v[nph].code[l1][1];
		phcd[nbrn][1] = 'c';
		phcd[nbrn][2] = v[kph].code[l1][1];
		phcd[nbrn][3] = '\0';
	}
	else if(lyr == 2)
	{
		phcd[nbrn][0] = v[nph].code[l1][1];
		strcpy(phcd[nbrn]+1, v[kph].code[l1]+2);
	}
	if(isgn == 1)
	{
		tmp[0] = 'p';
		strcpy(tmp+1, phcd[nbrn]);
		strcpy(phcd[nbrn], tmp);
	}
	else if(isgn == 2)
	{
		tmp[0] = 's';
		strcpy(tmp+1, phcd[nbrn]);
		strcpy(phcd[nbrn], tmp);
	}
	v[1].indx[nseg] = nl-1;
	nbrn++;
	nseg++;
}

static void
mkcbr(int l1, int l2, int isgn, int lyr, int nph, int kph, double *fac)
{
/* Mkcbr handles phases reflected and converted at the surface such as 
 * PS and SP.  Arguments are as for mkdbr.
 */
	int j, k, l, m, lz1, lz2, ik, in, isw, nt;
	char tmp[10];
	double xfc;

	if(nph < 0 || kph < 0 || nph == kph)
	{
		fprintf(stderr, "Mkcbr: bad call: nph= %d kph= %d\n",nph,kph);
		exit(1);
	}
	s[0].nafl[nseg] = isgn;
	s[1].nafl[nseg] = nph+1;
	s[2].nafl[nseg] = kph+1;
	v[0].indx[nseg] = nl;
	v[0].kndx[nseg] = 0;
	if(l1 > 0) v[0].kndx[nseg] = (v[nph].lbrk[l1] < v[kph].lbrk[l1]) ?
				v[nph].lbrk[l1] : v[kph].lbrk[l1];
	v[1].kndx[nseg] = (v[nph].lbrk[l2] < v[kph].lbrk[l2]) ? 
				v[nph].lbrk[l2] : v[kph].lbrk[l2];
	if(v[1].kndx[nseg] > v[abs(isgn)-1].lbrk[l2])
		v[1].kndx[nseg] = v[abs(isgn)-1].lbrk[l2];
	printf("Mkcbr:  l1 l2 isgn lyr nph kph = %d %d %d %d %d %d\n",
		l1, l2, isgn, lyr, nph, kph);
	printf("Mkcbr:  nseg kndx indx = %d %d %d %d\n",nseg,
		v[0].kndx[nseg], v[1].kndx[nseg], v[0].indx[nseg]);
	xfc = 0.;
	for(m = 0; m < lyr; m++)
	{
		s[m].fcs[nseg] = fac[m];
		if(xfc < s[m].fcs[nseg]) xfc = s[m].fcs[nseg];
	}

	j = v[0].kndx[nseg];
	lz1 = 0;
	lz2 = 0;
	ik = l1;

	printf("Mkcbr:  start loop\n");
	for(in = l1; in <= l2;)
	{
		l = (v[nph].lbrk[in] < v[1].kndx[nseg]) ?
				v[nph].lbrk[in] : v[1].kndx[nseg];
		if(v[nph].code[in][0] == 'r' || j >= l)
		{
			if(j < l) j = l;
			in++;
			continue;
		}
		l = (v[kph].lbrk[ik] < v[1].kndx[nseg]) ?
				v[kph].lbrk[ik] : v[1].kndx[nseg];
		if((v[kph].code[ik][0] == 'r' || j >= l) && ik < l2)
		{
			if(j < l) j = l;
			ik++;
			continue;
		}
		if(v[nph].lbrk[in] > v[kph].lbrk[ik])
		{
			l = (v[kph].lbrk[ik] < v[1].kndx[nseg]) ?
				v[kph].lbrk[ik] : v[1].kndx[nseg];
			printf("kph ik j l code = %d %d %d %d %s\n",
				kph, ik, j, l, v[kph].code[ik]);
			isw = 2;
		}
		else
		{
			l = (v[nph].lbrk[in] < v[1].kndx[nseg]) ?
				v[nph].lbrk[in] : v[1].kndx[nseg];
			printf("nph in j l code = %d %d %d %d %s\n",
				nph, in, j, l, v[nph].code[in]);
			isw = 1;
		}
		nt = nl;
		v[0].jndx[nbrn] = nt;
		for(k = j; k <= l; k++, nl++)
		{
			pt[nl] = pb[k];
			for(m = 0; m < lyr; m++) taut[nl] +=
				fac[m]*(v[nph].t[m].taup[k] +
						v[kph].t[m].taup[k]);
		}
		v[0].mndx[nbrn] = j;
		v[1].mndx[nbrn] = l;
		v[0].px[nbrn] = pb[j];
		v[1].px[nbrn] = pb[l];
		for(m = 0; m < lyr; m++)
		{
			v[0].xt[nbrn] += fac[m]*(v[nph].t[m].xp[j] +
						v[kph].t[m].xp[j]);
			v[1].xt[nbrn] += fac[m]*(v[nph].t[m].xp[l] +
						v[kph].t[m].xp[l]);
		}
		if(lz1 < lt[nph] && j == v[nph].lvz[lz1])
		{
			for(m = 0; m < lyr; m++)
			{
				taut[nt] -= fac[m]*v[nph].t[m].taup[j];
				v[0].xt[nbrn] -= fac[m]*v[nph].t[m].xp[j];
			}
			taut[nt] += fac[0]*v[nph].taul[lz1];
			v[0].xt[nbrn] += fac[0]*v[nph].xl[lz1];
			lz1++;
		}
		if(lz2 < lt[kph] && j == v[kph].lvz[lz2])
		{
			for(m = 0; m < lyr; m++)
			{
				taut[nt] -= fac[m]*v[kph].t[m].taup[j];
				v[0].xt[nbrn] -= fac[m]*v[kph].t[m].xp[j];
			}
			taut[nt] += fac[0]*v[kph].taul[lz2];
			v[0].xt[nbrn] += fac[0]*v[kph].xl[lz2];
			lz2++;
		}
		pdect(v[0].jndx[nbrn], nl-1, j, abs(isgn)-1, xfc);
		tauspl(v[0].jndx[nbrn], nl-1, pt, c1, c2, c3, c4, c5);
		v[1].jndx[nbrn] = nl-1;

		if(v[nph].code[in][2] == '\0')
		{
			phcd[nbrn][0] = v[nph].code[in][1];
			strcpy(phcd[nbrn]+1, v[kph].code[ik]+1);
		}
		if(v[nph].code[in][2] != '\0' && v[nph].code[in][2] != 'K')
		{
			phcd[nbrn][0] = v[nph].code[in][1];
			phcd[nbrn][1] = v[nph].code[in][2];
			strcpy(phcd[nbrn]+2, v[kph].code[ik]+1);
		}
		if(v[nph].code[in][2] == 'K')
		{
			phcd[nbrn][0] = v[nph].code[in][1];
			phcd[nbrn][1] = '\'';
			phcd[nbrn][2] = v[kph].code[ik][1];
			phcd[nbrn][3] = '\'';
			strcpy(phcd[nbrn]+4, v[kph].code[ik]+4);
		}
		if(isgn == 1)
		{
			tmp[0] = 'p';
			strcpy(tmp+1, phcd[nbrn]);
			strcpy(phcd[nbrn], tmp);
		}
		else if(isgn == 2)
		{
			tmp[0] = 's';
			strcpy(tmp+1, phcd[nbrn]);
			strcpy(phcd[nbrn], tmp);
		}
		printf("phcd: in ik phcd = %d %d %s\n", in, ik, phcd[nbrn]);
		if(isw > 1)
		{
			ik++;
			if(j < l) j = l;
			nbrn++;
			continue;
		}
		if(j < l) j = l;
		in++;
		nbrn++;
	}
	v[1].indx[nseg] = nl-1;
	nseg++;
}

static void
pdect(int i1, int i2, int j1, int nph, float fac)
{
	double h1, h2, hh, xmn;
	int ib[2][2], isg, i, ii, ie, it, j, k;

	xmn = fac*xmin;
	isg = 1;
	for(i = 0; i < 2; i++)
	{
		ib[i][0] = i1;
		ib[i][1] = i2;
	}
	ii = i1+1;
	ie = i2-1;
	xa[i1] = v[0].xt[nbrn];
	for(i = ii; i <= ie; i++)
	{
		h1 = pt[i-1] - pt[i];
		h2 = pt[i+1] - pt[i];
		hh = h1*h2*(h1 - h2);
		h1 = h1*h1;
		h2 = -h2*h2;
		xa[i] = -(h2*taut[i-1] - (h2+h1)*taut[i]  +h1*taut[i+1])/hh;
	}
	xa[i2] = v[1].xt[nbrn];
	for(i = ii; i <= ie; i++) if((xa[i+1]-xa[i])*(xa[i]-xa[i-1]) <= 0.0)
	{
		isg = 2;
		ib[0][1] = i - 2;
		ib[1][0] = i + 2;
	}
	for(it = 0; it < isg; it++) collct(ib[it][0],ib[it][1], xa, xmn);
	k = i1-1;
	j = j1;
	for(i = i1; i <= i2; i++, j++) if(xa[i] >= 0.0) 
	{
		k++;
		pt[k] = pt[i];
		taut[k] = taut[i];
		xa[k] = xa[i];
		v[nph].kuse[j] = 1;
	}
	if(k+1 == nl) return;
	for(i = k+1; i < nl; i++) taut[i] = 0.0;
	nl = k+1;
}

static void
kseq()
{
/*
 * Kseq makes a correspondence between model slownesses in array pb and
 * the subset of the same slownesses used for sampling tau which are
 * stored in pu (separate sets for P and S).  The net result is to
 * translate the kndx pointers to critical slowness values (bounding
 * branches actually implemented) from pointing into pb to pointing
 * into pu.
 */
	static int kl[2] = {0, 0}, kk[jseg][2][2];
	int i, j, k, l, nph, m, n1, ki;

	/* Compile a sorted list of unique kndx values in the first column
	 * of  kk.
	 */
	for(i = 0; i < nseg; i++)
	{
		nph = abs(s[0].nafl[i])-1;
		k = kl[nph];
		for(j = 0; j < 2; j++)
		{
			for(m = 0; m < k; m++)
				if(kk[m][nph][0] >= v[j].kndx[i]) break;
			if(m == k)
			{
				kk[k++][nph][0] = v[j].kndx[i];
			}
			else if(kk[m][nph][0] > v[j].kndx[i])
			{
				for(l = k-1; l >= m; l--)
					kk[l+1][nph][0] = kk[l][nph][0];
				kk[m][nph][0] = v[j].kndx[i];
				k++;
			}
		}
		kl[nph] = k;
	}
	/* Make the correspondence between pb and pu for each kndx and save it
	 * in the second column of kk.
	 */
	for(nph = 0; nph < 2; nph++)
	{
		n1 = ku[nph];
		k = 0;
		ki = kk[k][nph][0];
		for(i = 0; i < n1; i++)
		{
			if(v[nph].pu[i] == pb[ki])
			{
				kk[k++][nph][1] = i;
				if(k >= kl[nph]) break;
				ki = kk[k][nph][0];
			}
			else if(v[nph].pu[i] > pb[ki])
			{
				fprintf(stderr,
				"Kseq: pb[%d]=%7.4f not found in pu[*,%d]\n",
					ki, pb[ki], nph);
				exit(1);
			}
		}
	}
	/* Replace each kndx pb pointer with the corresponding pu pointer.
	 */
	for(i = 0; i < nseg; i++)
	{
		nph = abs(s[0].nafl[i])-1;
		k = kl[nph];
		for(j = 0; j < 2; j++)
		{
			for(m = 0; m < k; m++)
				if(kk[m][nph][0] == v[j].kndx[i]) break;
			if(m == k)
			{
				fprintf(stderr,"Kseg: kndx %d not translated\n",
					v[j].kndx[i]);
				exit(1);
			}
			v[j].kndx[i] = kk[m][nph][1];
		}
	}
}

static void
mseq()
{
	/*
	 * partial reordering of tables
	 */
	int i, j, k, l, m, is, nph;

	for(i = is = 0; i < nbrn; i++)
	{
		while(v[1].jndx[i] > v[1].indx[is]) is++;
		nph = abs(s[0].nafl[is])-1;
		k = km[nph];
		for(j = 0; j < 2; j++)
		{
			for(m = 0; m < k; m++)
				if(v[nph].midx[m] >= v[j].mndx[i]) break;	
			if(m == k)
			{
				v[nph].midx[k] = v[j].mndx[i];
				v[nph].pux[k] = v[j].px[i];
				k++;
			}
			else if(v[nph].midx[m] > v[j].mndx[i])
			{
				for(l = k-1; l >= m; l--)
				{
					v[nph].midx[l+1] = v[nph].midx[l];
					v[nph].pux[l+1] = v[nph].pux[l];
				}
				k++;
				v[nph].midx[m] = v[j].mndx[i];
				v[nph].pux[m] = v[j].px[i];
			}
		}
		km[nph] = k;
	}
}
