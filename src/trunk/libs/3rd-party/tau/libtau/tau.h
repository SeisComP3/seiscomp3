# ifndef LIBTAU_H
# define LIBTAU_H

#include "ttlim.h"

typedef struct { double c1, c2, c3, c4; } c1c2c3c4;

typedef struct {
	struct {
		int	ndex[jsrc], indx[jseg], kndx[jseg], loc[jsrc];
		double	pu[jtsm0], px[jbrn], xt[jbrn], pux[jxsm], pm[jsrc], zm[jsrc],
			tp[jbrnu];
	} v[2];

	struct s_struct {
		int nafl[jseg];
		float fcs[jseg];
	} s[3];

	int mt[2], km[2], ku[2], jidx[jbrn];
	int nseg, nbrn, ka;
	double taut[jout];
	double c1[jout], c2[jout], c3[jout], c4[jout], c5[jout];

	float xn, pn, tn, dn;
	float deplim;
	char segmsk[jseg];

	FILE *fpin;
	FILE *fp10;

	struct Depth {
		char phcd_buf[jbrn*10];
		char *phcd[jbrn];
		struct {
			int	jndx[jbrn];
			double	xu[jxsm], tauu[jtsm], dbrn[jbrn];
		} w[2];

		double pt[jout], tauc[jrec], xc[jxsm];
		struct {
			int idel[jbrn];
			double xbrn[jbrn];
		} t[3];

		c1c2c3c4 tau[jout];

		int int0[2], isrc[2], msrc[2], kk[jseg], iidx[jseg];
		int ki, mbr1, mbr2, nph0;
		double us[2], pk[jseg], xlim1[jout], xlim2[jout];
		double zs;
		float hn, odep;
		double ua[5][2], taua[5][2];
	} depths[2];

	int D; /* depth index */

	/* static temporary buffers for emdld */
	int np;
	float rd[30];

	char *phlst[jseg], *segcd[jbrn], phtmp[10];
	int allocated;
} libtau;


void brnset(libtau *h, const char *branch);
void depset(libtau *h, float zs);
void trtm(libtau *, float delta, int *pn, float *tt, float *ray_p, float *dtdd, float *dtdh, float *dddp, char **phnm);
int emdlv(float r, float *vp, float *vs);
int tabin(libtau *h, const char *);
int tabout(libtau *h);

# endif
