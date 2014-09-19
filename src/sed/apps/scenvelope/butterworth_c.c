#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>

#include "butterworth_c.h"

#ifndef PI
#define PI  3.1415927
#endif



/**********************************************************************/
/*          add_c                                                     */
/**********************************************************************/
/*
    Routine to add two complex numbers

	w = add_c(u,v)
        w = u + v
*/

complex	    add_c (complex u, complex v)
{
    complex	w ;

    w.real = u.real + v.real ;
    w.imag = u.imag + v.imag ;

    return (w) ;
}

/***********************************************************************/
/*             mul_c                                                   */
/***********************************************************************/
/*
	Routine to multiply two complex numbers

	        w = mul_c(u,v)
		w = u * v

*/
complex	mul_c (complex u, complex v)
{
	complex		w ;

	w.real = u.real*v.real - u.imag*v.imag ;
	w.imag = u.real*v.imag + u.imag*v.real ;

	return (w) ;
}

/***********************************************************************/
/*            cmul_c (a,u)                                             */
/***********************************************************************/
/*
    Routine to multiply a real number times a complex number

	w = cmul_c (a,u)

	a - real number
	u - complex number
*/
complex	    cmul_c (double a, complex u)
{
    complex	w ;

    w.real = a * u.real ;
    w.imag = a * u.imag ;

    return (w) ;
}

/*************************************************************************/
/*               sub_c                                                   */
/*************************************************************************/
/*
    Routine to subtract two complex numbers

	w = sub_c(u,v)
	w = u - v
*/
complex	sub_c(u,v)
complex	    u ;
complex	    v ;
{
    complex	w ;
    w.real = u.real - v.real ;
    w.imag = u.imag - v.imag ;

    return (w) ;
}

/******************************************************************/
/*                    div_c                                       */
/******************************************************************/
/*
    Routine to divide two complex numbers

	w = div_c(u,v)
	w = u/v

*/
complex	div_c (u,v)
complex	    u ;
complex	    v ;
{
    complex	w ;

    /*   check for divide by 0    */
    if (v.real != 0 && v.imag != 0)
    {
	w.real = ((u.real * v.real) + (u.imag * v.imag)) /
		((v.real * v.real) + (v.imag * v.imag)) ;
	w.imag = ((u.imag * v.real) - (u.real * v.imag)) /
		((v.real * v.real) + (v.imag * v.imag)) ;

	return (w) ;
    }
    else
    {
	fprintf (stderr, "ERROR: complex division by 0 in div_c\n") ;
	exit (1) ;
    }
}

/***************************************************************/
/*                 conj_c                                      */
/***************************************************************/
/*
	Routine to calculate the complex conjugate

		w = conjugate(u)

*/
complex	conj_c (u)
complex		u ;
{
	complex		w ;

	w.real = u.real ;
	w.imag = -u.imag ;

	return (w) ;
}

/************************************************************************/
/*        filt (a1, a2, b1, b2, npts, fi, fo)                           */
/************************************************************************/
/*	Routine to apply a second order recursive filter to the data
	denomonator polynomial is z**2 + a1*z + a2
	numerator polynomial is z**2 + b1*z + b2
	    fi = input array
	    fo = output array
	    npts = number of points

*/
void filt (double a1, double a2, double b1, double b2, int npts, double *fi, double *fo, double *d1, double *d2)
{
    double  out ;
    int i ;

   /*  d1 = 0 ; */
   /*  d2 = 0 ; */
    for ( i=0 ; i<npts ; i++)
    {
	out = fi[i] + *d1 ;
	*d1 = b1*fi[i] - a1*out + *d2 ;
	*d2 = b2*fi[i] - a2*out ;
	fo[i] = out ;
    }

}

/***************************************************************/
/*              wdat(FF, npts, dt, ar)                         */
/***************************************************************/
/*
    Routine to write data in format for asci2sac to read
	FF = output file name
	npts = number of points to output
	dt = sample rate in seconds
	ar = array of values to output

*/
void wdat (char *FF, int npts, double *ar)
{
    int i ;
    FILE    *fn ;

    if((fn = fopen(FF,"w+")) == NULL)
    {
	printf("cannot create output file ") ;
	exit (1) ;
    }

    for ( i=0 ; i<npts ; i++)
    {
	fprintf(fn, "%16.8le\n", ar[i]) ;
    }
    fclose(fn) ;
}

/**************************************************************************/
/*                   lowpass (fc,dt,n,p,b)                               */
/**************************************************************************/
/*
    Routine to compute lowpass filter poles for Butterworth filter
	fc = desired cutoff frequency
	dt = sample rate in seconds
	n = number of poles (MUST BE EVEN)
	p = pole locations (RETURNED)
	b = gain factor for filter (RETURNED)

*/
/*   Program calculates a continuous Butterworth low pass IIRs with required */
/*    cut off frequency.                                                     */
/*   This program is limited to using an even number of poles                */
/*   Then a discrete filter is calculated utilizing the bilinear transform   */
/*   Methods used here follow those in Digital Filters and Signal Processing */
/*   by Leland B. Jackson  */

void lowpass	(double fc,double dt, int n, complex *p,double *b)
//double	    fc, dt, *b ;
//complex	    p[] ;
//int	    n ;
{
    double	wcp, wc, b0 ;
    int		i, i1 ;
    complex	add_c(), mul_c(), div_c(), cmul_c(), sub_c() ;
    complex	conj_c() ;
    complex	one, x, y ;

/*			    Initialize variables       */
/*    PI = 3.1415927 ; */
    wcp = 2 * fc * PI ;
    wc = (2./dt)*tan(wcp*dt/2.) ;
    one.real = 1. ;
    one.imag = 0. ;
    for (i=0 ; i<n ; i += 2)
    {
/*               Calculate position of poles for continuous filter    */

	i1 = i + 1 ;
        p[i].real = -wc*cos(i1*PI/(2*n)) ;
	p[i].imag = wc*sin(i1*PI/(2*n)) ;
	p[i+1] = conj_c(p[i]) ;
    }
    for ( i=0 ; i<n ; i += 2)
    {
/*             Calculate position of poles for discrete filter using    */
/*              the bilinear transformation                             */

	p[i] = cmul_c(dt/2,p[i]) ;
	x = add_c(one,p[i]) ;
	y = sub_c(one,p[i]) ;
	p[i] = div_c(x,y) ;
	p[i+1] = conj_c(p[i]) ;
    }

/*	calculate filter gain   */

    b0 = 1. ;
    for (i=0 ; i<n ; i +=2)
    {
	x = sub_c(one,p[i]) ;
	y = sub_c(one,p[i+1]) ;
	x = mul_c(x,y) ;
	b0 = b0*4./x.real ;
    }
    b0 = 1./b0 ;
    *b = b0 ;
}

/**************************************************************************/
/*                   highpass (fc,dt,n,p,b)                               */
/**************************************************************************/
/*
    Routine to compute lowpass filter poles for Butterworth filter
	fc = desired cutoff frequency
	dt = sample rate in seconds
	n = number of poles (MUST BE EVEN)
	p = pole locations (RETURNED)
	b = gain factor for filter (RETURNED)

*/
/*   Program calculates a continuous Butterworth highpass IIRs               */
/*   First a low pass filter is calculated with required cut off frequency.  */
/*   Then this filter is converted to a high pass filter                     */
/*   This program is limited to using an even number of poles                */
/*   Then a discrete filter is calculated utilizing the bilinear transform   */
/*   Methods used here follow those in Digital Filters and Signal Processing */
/*   by Leland B. Jackson  */

void highpass (double fc, double dt, int n, complex *p, double *b)
//double	    fc, dt, *b ;
//complex	    p[] ;
//int	    n ;
{
    double	wcp, wc,  alpha, b0 ;
    int		i ;
    complex	add_c(), mul_c(), div_c(), cmul_c(), sub_c() ;
    complex	conj_c() ;
    complex	one, x, y ;

/*         Initialize variables          */
/*     PI = 3.1415927 ; */
    wcp = 2 * fc * PI ;
    wc = (2./dt)*tan(wcp*dt/2.) ;
    alpha = cos(wc*dt) ;
    one.real = 1. ;
    one.imag = 0. ;

/*            get poles for low pass filter     */

    lowpass(fc,dt,n,p,&b0) ;

/*       now find poles for highpass filter      */

    for (i=0 ; i<n ; i+=2)
    {
	x = cmul_c (alpha,one) ;
	x = sub_c (x,p[i]) ;
	y = cmul_c (alpha,p[i]) ;
	y = sub_c(one,y) ;
	p[i] = div_c(x,y) ;
	p[i+1] = conj_c(p[i]) ;
    }

/*      Calculate gain for high pass filter    */

    b0 = 1. ;
    for (i=0 ; i<n ; i += 2)
    {
	x = add_c(one,p[i]) ;
	y = add_c(one,p[i+1]) ;
	x = mul_c(x,y) ;
	b0 = b0*4./x.real ;
    }
    b0 = 1./b0 ;
    *b = b0 ;
}

/*                                                                     
*  Program to test filters for PASSCAL Instrument                    
*  These filters are Butterworth highpass and lowpass filters     
*  both filters may be implemented or only one filter             
*  This program limits the number of poles to be either 0, 2 or 4 
*  The program to calculate the pole position or to filter does   
*  not have any limits                                            
*                                                                 
*   Input parameters                                              
*     NH = order of the high pass filter can be 0, or an even number
*        up to 12                                               
*     FH = high pass cutoff frequency                                
*     NL = order of the low pass filter can be 0, or an even number
*        up to 12                                               
*     FL = low pass cutoff frequency                                  
*     dt = sample rate                                                
*                                                                     
*       written by jcf   feb 1988
*       modified  july 1993.
*/
	
int *
	butter_filter_data(int *data,
	                      double lowCutoff,
	                      double highCutoff,
	                      int numSamp,
	                      char filterType,
	                      int filterOrder,
	                      int sampRate)
	{
		int *pOutData = NULL;
		int    i;
		complex	pl[12], ph[12] ;
		double	  b0l, b0h ;
//		double	f1, f0 ;
		double	a1, a2, b1, b2 , d1, d2;
		double dt;

		double       *af;



		/*             Get low pass filter parameters    */
		dt = 1./sampRate;

		/*         Get highpass filter poles if necessary    */

		if('l' != filterType)
		{
			highpass(highCutoff,dt,filterOrder,ph,&b0h) ;
		}

		/*      Get low pass filter poles if necessary     */

		if('h' != filterType)
		{
			lowpass(lowCutoff,dt,filterOrder,pl,&b0l) ;
		}

		/*   Through with calculation of poles        */

		/*       now calculate sweep for filtering     */

//		f0 = 0. ;     /*  start frequency    */
//		f1 = 1./(dt*2.) ;	/*  stop frequency = nyquist  */


		if ((af = (double *)calloc(numSamp,sizeof(double))) == NULL)
		{
			fprintf(stderr, "out of memory error in butterworth filter\n");
			return NULL;
		}
		for (i=0; i < numSamp; i++ )
			af[i] = (double)data[i];

		/*   now start filtering the data
		 *  Filter is implemented as a cascade of second order filters
		 *  high pass filter first use poles ph
		 *  Numerator polynomial is z**2 - 2*z + 1
		 */


		if('l' != filterType)
		{
			for ( i=0 ; i<filterOrder ; i +=2)
			{

				/* Get first set of second order filter coeficients from each pair of poles */

				a1 = -2*ph[i].real ;
				a2 = ph[i].real*ph[i].real + ph[i].imag*ph[i].imag ;
				b1 = -2 ;
				b2 = 1 ;
				d1 = 0;
				d2 = 0;

				filt (a1, a2, b1, b2, numSamp, af, af, &d1, &d2) ;
			}
			/*        apply gain section          */
			for ( i=0 ; i<numSamp ; i++)
			{
				af[i] = b0h*af[i] ;
			}
		}

		/*      apply low pass filter using poles pl         */
		/*	Numerator polynomial is z**2 + 2*z + 1       */

		if('h' != filterType)
		{
			for ( i=0 ; i<filterOrder ; i +=2)
			{

				a1 = -2*pl[i].real ;
				a2 = pl[i].real*pl[i].real + pl[i].imag*pl[i].imag ;
				b1 = 2 ;
				b2 = 1 ;
				d1 = 0;
				d2 = 0;

				filt (a1, a2, b1, b2, numSamp, af, af, &d1, &d2) ;
			}

		if ((pOutData = (int *)calloc(numSamp,sizeof(int))) == NULL)
		{
			return NULL;
		}
			for ( i=0 ; i<numSamp ; i++)
			{
				pOutData[i] = (int)(b0l*af[i]) ;
//				fprintf(stdout, "%d %d\n", data[i], pOutData[i]);
			}
//      exit(-1);

			free(af);

		}
		return pOutData;

	}

	




/* EOF */
