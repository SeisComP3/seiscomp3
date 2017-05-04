/* NAME */
/* 	hermit -- Two-point Hermite cubic interpolation routine. */
/* FILE */
/* 	hermit.f */
/* SYNOPSIS */
/* 	A simple two-point Hermitian cubic interpolation routine. */
/* DESCRIPTION */
/* 	Subroutine.  Perform a Hermite cubic interpolation of function y(x) */
/* 	bewteen two sample points. */
/* 	---- On entry ---- */
/* 	x1, x2:		Sample values of independent variable */
/* 	y1, y2:		Values of function at x1 and x2, respectively */
/* 	yp1, yp2:	Values of derivative of function at x1 and x2 */
/* 	x0:		Value of independent variable for interpolation */
/* 	---- On return ---- */
/* 	y0:		Interpolated value of function at x0 */
/* 	yp0:		Interpolated value of derivative at x0 */
/* DIAGNOSTICS */

/* FILES */

/* NOTES */

/* SEE ALSO */

/* AUTHOR */

int hermit_(float *x1, float *x2, float *y1, float *y2, float *yp1, float *yp2,
            float *x0, float *y0, float *yp0) {
	float a, b, c__, d__, t, f1, f2, df, dx, fp1, fp2, sfp;

	/* K.S. 1-Dec-97, changed 'undefined' to 'none' */
	/*     ---- On entry ---- */
	/*     ---- On return ---- */
	/*     ---- Internal variables ---- */
	dx = *x2 - *x1;
	t = (*x0 - *x1) / dx;

	if (t <= (float).5) {
		f1 = *y1;
		f2 = *y2;
		fp1 = *yp1;
		fp2 = *yp2;
	}
	else {
		t = (float)1. - t;
		dx = -dx;
		f1 = *y2;
		f2 = *y1;
		fp1 = *yp2;
		fp2 = *yp1;
	}

	fp1 *= dx;
	fp2 *= dx;
	df = f2 - f1;
	sfp = fp1 + fp2;
	a = f1;
	b = fp1;
	c__ = df * (float)3. - sfp - fp1;
	d__ = df * (float)-2. + sfp;
	*y0 = ((d__ * t + c__) * t + b) * t + a;
	*yp0 = ((d__ * (float)3. * t + c__ * (float)2.) * t + b) / dx;

	return 0;
}
