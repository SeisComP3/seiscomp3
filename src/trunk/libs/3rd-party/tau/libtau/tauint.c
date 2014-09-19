#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void
tauint(double ptk, double ptj, double pti, double zj, double zi, double *tau, double *x)
{
/* Tauint evaluates the intercept (tau) and distance (x) integrals  for
 * the spherical earth assuming that slowness is linear between radii
 * for which the model is known.  The partial integrals are performed
 * for ray slowness ptk between model radii with slownesses ptj and pti
 * with equivalent flat earth depths zj and zi respectively.  The partial
 * integrals are returned in tau and x.  Note that ptk, ptj, pti, zj, zi,
 * tau, and x are all double precision.
 */

	double xx, b, d, sqk, sqi, sqj, sqb;

	if(fabs(zj - zi) <= 1.e-9)
	{
		*tau = 0.;
		*x = 0.;
		return;
	}
	if(fabs(ptj - pti) <= 1.e-9)
	{
		if(fabs(ptk - pti) <= 1.e-9)
		{
			*tau = 0.;
			*x = 0.;
			return;
		}
		b = fabs(zj - zi);
		sqj = sqrt(fabs(ptj*ptj - ptk*ptk));
		*tau = b*sqj;
		*x = b*ptk/sqj;
	}
	else if(ptk <= 1.e-9 && pti <= 1.e-9)
	{
		/* Handle the straight through ray.
		 */
		*tau = ptj;
		*x = 1.5707963267948966;
	}
	else if(ptk <= 1.e-9)
	{
		b = ptj - (pti - ptj)/(exp(zi - zj) - 1.0);
		d = (ptj-b)*pti/((pti-b)*ptj);
		if(d < 1.e-30) d = 1.e-30;
		*tau = -(pti - ptj + b*log(pti/ptj) - b*log(d));
		*x = 0.0;
	}
	else if(ptk != pti && ptk != ptj)
	{
		b = ptj - (pti - ptj)/(exp(zi - zj) - 1.0);
		sqk = ptk*ptk;
		sqi = sqrt(fabs(pti*pti - sqk));
		sqj = sqrt(fabs(ptj*ptj - sqk));
		sqb = sqrt(fabs(b*b - sqk));
		if(sqb <= 1.e-30)
		{
			xx = 0.;
			*x = ptk*(sqrt(fabs((pti+b)/(pti-b)))
				-sqrt(fabs((ptj+b)/(ptj-b))))/b;
		}
		else
		{
			if(b*b >= sqk)
			{
				d = (ptj-b)*(sqb*sqi+b*pti-sqk)/
					((pti-b)*(sqb*sqj+b*ptj-sqk));
				if(d < 1.e-30) d = 1.e-30;
				xx = log(d);
				*x = ptk*xx/sqb;
			}
			else
			{
				d = (b*pti-sqk)/(ptk*fabs(pti-b));
				if(d < -1.) d = -1.;
				else if(d > 1.) d = 1.;
				xx = asin(d);
				d = (b*ptj-sqk)/(ptk*fabs(ptj-b));
				if(d < -1.) d = -1.;
				else if(d > 1.) d = 1.;
				xx -= asin(d);
				*x = -ptk*xx/sqb;
			}
		}
		*tau = -(sqi - sqj + b*log((pti+sqi)/(ptj+sqj)) - sqb*xx);
	}
	else if(ptk == pti)
	{
		b = ptj - (pti - ptj)/(exp(zi - zj) - 1.0);
		sqk = pti*pti;
		sqj = sqrt(fabs(ptj*ptj-sqk));
		sqb = sqrt(fabs(b*b-sqk));
		if(b*b >= sqk)
		{
			d = (ptj-b)*(b*pti-sqk)/((pti-b)*(sqb*sqj+b*ptj-sqk));
			if(d < 1.e-30) d = 1.e-30;
			xx = log(d);
			*x = pti*xx/sqb;
		}
		else
		{
			xx = (b >= pti) ? 1.5707963267948966 :
					 -1.5707963267948966;
			d = (b*ptj-sqk)/(pti*fabs(ptj-b));
			if(d < -1.) d = -1.;
			else if(d > 1.) d = 1.;
			xx -= asin(d);
			*x = -pti*xx/sqb;
		}
		*tau = -(b*log(pti/(ptj+sqj)) - sqj - sqb*xx);
	}
	else if(ptk == ptj)
	{
		b = ptj - (pti - ptj)/(exp(zi - zj) - 1.0);
		sqk = ptj*ptj;
		sqi = sqrt(fabs(pti*pti-sqk));
		sqb = sqrt(fabs(b*b-sqk));
		if(b*b >= sqk)
		{
			d = (ptj-b)*(sqb*sqi+b*pti-sqk)/((pti-b)*(b*ptj-sqk));
			if(d < 1.e-30) d = 1.e-30;
			xx = log(d);
			*x = ptj*xx/sqb;
		}
		else
		{
			d = (b*pti-sqk)/(ptj*fabs(pti-b));
			if(d < -1.) d = -1.;
			else if(d > 1.) d = 1.;
				
			xx = (b >= ptj) ? asin(d) - 1.5707963267948966 :
					asin(d) + 1.5707963267948966;
			*x = -ptj*xx/sqb;
		}
		*tau = -(b*log((pti+sqi)/ptj) + sqi - sqb*xx);
	}
	/*
	 * Handle various error conditions.
	 */
	if(*x < -1.e-10)
	{
		fprintf(stderr, "tauint: bad range: %E %E %E %E %E\n",
			ptk, ptj, pti, *tau, *x);
exit(1);
	}
	if(*tau < -1.e-10)
	{
		fprintf(stderr, "tauint: bad tau: %E %E %E %E %E\n",
			ptk, ptj, pti, *tau, *x);
exit(1);
	}
}
