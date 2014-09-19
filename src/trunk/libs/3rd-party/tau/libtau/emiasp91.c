#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int np = 11;
static float rd[] = {1217.1, 3482.0, 3631., 5611., 5711., 5961., 6161.,
			6251., 6336., 6351., 6371.};
static float rn = 1.5696123e-4, vn = 6.8501006;
char modnam[21] = "iasp91";

static void emiask(float x0, float *ro, float *vp, float *vs);

void
emdlv(float r, float *vp, float *vs)
{
	/* set up information on earth model (specified by
	 * subroutine call emiasp)
	 * set dimension of cpr,rd  equal to number of radial
	 * discontinuities in model
	 */
	float rho;

	emiask(rn*r, &rho, vp, vs);
	*vp = vn * *vp;
	*vs = vn * *vs;
}

void
emdld(int *n, float *cpr, char *name, char *path)
{
	int i;

	*n = np;
	for(i = 0; i < np; i++)
		cpr[i] = rd[i];
	strcpy(name, modnam);
}

static void emiask(float x0, float *ro, float *vp, float *vs)
{

/* calls no other routine
 * 
 * Emiask returns model parameters for the IASPEI working model 
 * (September 1990.1).  
 * Given non-dimensionalized radius x0, emiasp returns
 * non-dimensionalized density, ro, compressional velocity, vp, and
 * shear velocity, vs.  Non-dimensionalization is according to the
 * scheme of Gilbert in program EOS:  x0 by a (the radius of the
 * Earth), ro by robar (the mean density of the Earth), and velocity
 * by a*sqrt(pi*G*robar) (where G is the universal gravitational
 * constant.
 */
	static float r[] = {0., 1217.1, 3482.0, 3631., 5611., 5711.,
		5961., 6161., 6251., 6336., 6351., 6371., 6371., 6371.};
	static float d1[] = {13.01219, 12.58416, 6.8143, 6.8143, 6.8143,
		11.11978, 7.15855, 7.15855, 7.15855, 2.92, 2.72, 0., 0.};
	static float d2[] = {0., -1.69929, -1.66273, -1.66273, -1.66273,
		-7.87054, -3.85999, -3.85999, -3.85999, 0., 0., 0., 0.};
	static float d3[] = {-8.45292, -1.94128, -1.18531, -1.18531, -1.18531,
		0., 0., 0., 0., 0., 0., 0., 0.};
	static float d4[] = {0., -7.11215, 0., 0., 0., 0., 0., 0., 0., 0., 0.,
		0., 0.};

	static float p1[] = {11.12094, 10.03904, 14.49470, 25.1486, 25.969838,
		29.38896, 30.78765, 25.41389, 8.785412, 6.5, 5.8, 0., 0.};
	static float p2[] = {0., 3.75665, -1.47089, -41.1538, -16.934118,
		-21.40656, -23.25415, -17.69722, -0.7495294, 0., 0., 0., 0.};
	static float p3[] = {-4.09689, -13.67046, 0.0, 51.9932, 0., 0., 0.,
		0., 0., 0., 0., 0., 0.};
	static float p4[] = {0., 0., 0., -26.6083, 0., 0., 0., 0., 0., 0., 0.,
		0., 0.};

	static float s1[] = {3.56454, 0., 8.16616, 12.9303, 20.768902, 17.70732,
		15.24213, 5.750203, 6.706232, 3.75, 3.36, 0., 0.};
	static float s2[] = {0., 0., -1.58206, -21.2590, -16.531471, -13.50652,
		-11.08553, -1.274202, -2.248585, 0., 0., 0., 0.};
	static float s3[] = {-3.45241, 0., 0., 27.8988, 0., 0., 0., 0., 0.,
		0., 0., 0., 0.};
	static float s4[] = {0., 0., 0., -14.1080, 0., 0., 0., 0., 0., 0.,
		0., 0., 0.};
	static float xn = 6371., rn = .18125793, vn = .14598326;
	static int i = 0;
	float x1;

	if(x0 < 0.) x0 = 0.;
	else if(x0 > 1.) x0 = 1.;
	x1 = xn*x0;
	/*
	 * find i such that r[i] < x1 && x1 < r[i+1].
	 */
	while(x1 < r[i]) i--;
	while(i < 10 && x1 > r[i+1]) i++;

	*ro = rn*(d1[i] + x0*(d2[i] + x0*(d3[i] + x0*d4[i])));
	*vp = vn*(p1[i] + x0*(p2[i] + x0*(p3[i] + x0*p4[i])));
	*vs = vn*(s1[i] + x0*(s2[i] + x0*(s3[i] + x0*s4[i])));
}
