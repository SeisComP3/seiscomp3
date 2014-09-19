# include <cstdlib>
# include <iostream>
# include <iomanip>
# include <cmath>
# include <ctime>

using namespace std;

# include "chi2.h"
# include "eigv.h"

//****************************************************************************80

double alngam ( double xvalue, int *ifault )

//****************************************************************************80
//
//  Purpose:
//
//    ALNGAM computes the logarithm of the gamma function.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    13 January 2008
//
//  Author:
//
//    Original FORTRAN77 version by Allan Macleod.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Allan Macleod,
//    Algorithm AS 245,
//    A Robust and Reliable Algorithm for the Logarithm of the Gamma Function,
//    Applied Statistics,
//    Volume 38, Number 2, 1989, pages 397-402.
//
//  Parameters:
//
//    Input, double XVALUE, the argument of the Gamma function.
//
//    Output, int IFAULT, error flag.
//    0, no error occurred.
//    1, XVALUE is less than or equal to 0.
//    2, XVALUE is too big.
//
//    Output, double ALNGAM, the logarithm of the gamma function of X.
//
{
  double alr2pi = 0.918938533204673;
  double r1[9] = {
    -2.66685511495, 
    -24.4387534237, 
    -21.9698958928, 
     11.1667541262, 
     3.13060547623, 
     0.607771387771, 
     11.9400905721, 
     31.4690115749, 
     15.2346874070 };
  double r2[9] = {
    -78.3359299449, 
    -142.046296688, 
     137.519416416, 
     78.6994924154, 
     4.16438922228, 
     47.0668766060, 
     313.399215894, 
     263.505074721, 
     43.3400022514 };
  double r3[9] = {
    -2.12159572323E+05, 
     2.30661510616E+05, 
     2.74647644705E+04, 
    -4.02621119975E+04, 
    -2.29660729780E+03, 
    -1.16328495004E+05, 
    -1.46025937511E+05, 
    -2.42357409629E+04, 
    -5.70691009324E+02 };
  double r4[5] = {
     0.279195317918525, 
     0.4917317610505968, 
     0.0692910599291889, 
     3.350343815022304, 
     6.012459259764103 };
  double value;
  double x;
  double x1;
  double x2;
  double xlge = 510000.0;
  double xlgst = 1.0E+30;
  double y;

  x = xvalue;
  value = 0.0;
//
//  Check the input.
//
  if ( xlgst <= x )
  {
    *ifault = 2;
    return value;
  }

  if ( x <= 0.0 )
  {
    *ifault = 1;
    return value;
  }

  *ifault = 0;
//
//  Calculation for 0 < X < 0.5 and 0.5 <= X < 1.5 combined.
//
  if ( x < 1.5 )
  {
    if ( x < 0.5 )
    {
      value = - log ( x );
      y = x + 1.0;
//
//  Test whether X < machine epsilon.
//
      if ( y == 1.0 )
      {
        return value;
      }
    }
    else
    {
      value = 0.0;
      y = x;
      x = ( x - 0.5 ) - 0.5;
    }

    value = value + x * (((( 
        r1[4]   * y 
      + r1[3] ) * y 
      + r1[2] ) * y 
      + r1[1] ) * y 
      + r1[0] ) / (((( 
                  y 
      + r1[8] ) * y 
      + r1[7] ) * y 
      + r1[6] ) * y 
      + r1[5] );

    return value;
  }
//
//  Calculation for 1.5 <= X < 4.0.
//
  if ( x < 4.0 )
  {
    y = ( x - 1.0 ) - 1.0;

    value = y * (((( 
        r2[4]   * x 
      + r2[3] ) * x 
      + r2[2] ) * x 
      + r2[1] ) * x 
      + r2[0] ) / (((( 
                  x 
      + r2[8] ) * x 
      + r2[7] ) * x 
      + r2[6] ) * x 
      + r2[5] );
  }
//
//  Calculation for 4.0 <= X < 12.0.
//
  else if ( x < 12.0 ) 
  {
    value = (((( 
        r3[4]   * x 
      + r3[3] ) * x 
      + r3[2] ) * x 
      + r3[1] ) * x 
      + r3[0] ) / (((( 
                  x 
      + r3[8] ) * x 
      + r3[7] ) * x 
      + r3[6] ) * x 
      + r3[5] );
  }
//
//  Calculation for 12.0 <= X.
//
  else
  {
    y = log ( x );
    value = x * ( y - 1.0 ) - 0.5 * y + alr2pi;

    if ( x <= xlge )
    {
      x1 = 1.0 / x;
      x2 = x1 * x1;

      value = value + x1 * ( ( 
             r4[2]   * 
        x2 + r4[1] ) * 
        x2 + r4[0] ) / ( ( 
        x2 + r4[4] ) * 
        x2 + r4[3] );
    }
  }

  return value;
}
//****************************************************************************80

double alnorm ( double x, bool upper )

//****************************************************************************80
//
//  Purpose:
//
//    ALNORM computes the cumulative density of the standard normal distribution.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    17 January 2008
//
//  Author:
//
//    Original FORTRAN77 version by David Hill.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    David Hill,
//    Algorithm AS 66:
//    The Normal Integral,
//    Applied Statistics,
//    Volume 22, Number 3, 1973, pages 424-427.
//
//  Parameters:
//
//    Input, double X, is one endpoint of the semi-infinite interval
//    over which the integration takes place.
//
//    Input, bool UPPER, determines whether the upper or lower
//    interval is to be integrated:
//    .TRUE.  => integrate from X to + Infinity;
//    .FALSE. => integrate from - Infinity to X.
//
//    Output, double ALNORM, the integral of the standard normal
//    distribution over the desired interval.
//
{
  double a1 = 5.75885480458;
  double a2 = 2.62433121679;
  double a3 = 5.92885724438;
  double b1 = -29.8213557807;
  double b2 = 48.6959930692;
  double c1 = -0.000000038052;
  double c2 = 0.000398064794;
  double c3 = -0.151679116635;
  double c4 = 4.8385912808;
  double c5 = 0.742380924027;
  double c6 = 3.99019417011;
  double con = 1.28;
  double d1 = 1.00000615302;
  double d2 = 1.98615381364;
  double d3 = 5.29330324926;
  double d4 = -15.1508972451;
  double d5 = 30.789933034;
  double ltone = 7.0;
  double p = 0.398942280444;
  double q = 0.39990348504;
  double r = 0.398942280385;
  bool up;
  double utzero = 18.66;
  double value;
  double y;
  double z;

  up = upper;
  z = x;

  if ( z < 0.0 )
  {
    up = !up;
    z = - z;
  }

  if ( ltone < z && ( ( !up ) || utzero < z ) )
  {
    if ( up )
    {
      value = 0.0;
    }
    else
    {
      value = 1.0;
    }
    return value;
  }

  y = 0.5 * z * z;

  if ( z <= con )
  {
    value = 0.5 - z * ( p - q * y 
      / ( y + a1 + b1 
      / ( y + a2 + b2 
      / ( y + a3 ))));
  }
  else
  {
    value = r * exp ( - y ) 
      / ( z + c1 + d1 
      / ( z + c2 + d2 
      / ( z + c3 + d3 
      / ( z + c4 + d4 
      / ( z + c5 + d5 
      / ( z + c6 ))))));
  }

  if ( !up )
  {
    value = 1.0 - value;
  }

  return value;
}
//****************************************************************************80

double gammad ( double x, double p, int *ifault )

//****************************************************************************80
//
//  Purpose:
//
//    GAMMAD computes the Incomplete Gamma Integral
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    20 January 2008
//
//  Author:
//
//    Original FORTRAN77 version by B Shea.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    B Shea,
//    Algorithm AS 239:
//    Chi-squared and Incomplete Gamma Integral,
//    Applied Statistics,
//    Volume 37, Number 3, 1988, pages 466-473.
//
//  Parameters:
//
//    Input, double X, P, the parameters of the incomplete 
//    gamma ratio.  0 <= X, and 0 < P.
//
//    Output, int IFAULT, error flag.
//    0, no error.
//    1, X < 0 or P <= 0.
//
//    Output, double GAMMAD, the value of the incomplete 
//    Gamma integral.
//
{
  double a;
  double an;
  double arg;
  double b;
  double c;
  double elimit = - 88.0;
  double oflo = 1.0E+37;
  double plimit = 1000.0;
  double pn1;
  double pn2;
  double pn3;
  double pn4;
  double pn5;
  double pn6;
  double rn;
  double tol = 1.0E-14;
  bool upper;
  double value;
  double xbig = 1.0E+08;

  value = 0.0;
//
//  Check the input.
//
  if ( x < 0.0 )
  {
    *ifault = 1;
    return value;
  }

  if ( p <= 0.0 )
  {
    *ifault = 1;
    return value;
  }

  *ifault = 0;

  if ( x == 0.0 )
  {
    value = 0.0;
    return value;
  }
//
//  If P is large, use a normal approximation.
//
  if ( plimit < p )
  {
    pn1 = 3.0 * sqrt ( p ) * ( pow ( x / p, 1.0 / 3.0 ) 
    + 1.0 / ( 9.0 * p ) - 1.0 );

    upper = false;
    value = alnorm ( pn1, upper );
    return value;
  }
//
//  If X is large set value = 1.
//
  if ( xbig < x )
  {
    value = 1.0;
    return value;
  }
//
//  Use Pearson's series expansion.
//  (Note that P is not large enough to force overflow in ALOGAM).
//  No need to test IFAULT on exit since P > 0.
//
  if ( x <= 1.0 || x < p )
  {
    arg = p * log ( x ) - x - alngam ( p + 1.0, ifault );
    c = 1.0;
    value = 1.0;
    a = p;

    for ( ; ; )
    {
      a = a + 1.0;
      c = c * x / a;
      value = value + c;

      if ( c <= tol )
      {
        break;
      }
    }

    arg = arg + log ( value );

    if ( elimit <= arg )
    {
      value = exp ( arg );
    }
    else
    {
      value = 0.0;
    }
  }
//
//  Use a continued fraction expansion.
//
  else 
  {
    arg = p * log ( x ) - x - alngam ( p, ifault );
    a = 1.0 - p;
    b = a + x + 1.0;
    c = 0.0;
    pn1 = 1.0;
    pn2 = x;
    pn3 = x + 1.0;
    pn4 = x * b;
    value = pn3 / pn4;

    for ( ; ; )
    {
      a = a + 1.0;
      b = b + 2.0;
      c = c + 1.0;
      an = a * c;
      pn5 = b * pn3 - an * pn1;
      pn6 = b * pn4 - an * pn2;

      if ( pn6 != 0.0 )
      {
        rn = pn5 / pn6;

        if ( r8_abs ( value - rn ) <= r8_min ( tol, tol * rn ) )
        {
          break;
        }
        value = rn;
      }

      pn1 = pn3;
      pn2 = pn4;
      pn3 = pn5;
      pn4 = pn6;
//
//  Re-scale terms in continued fraction if terms are large.
//
      if ( oflo <= r8_abs ( pn5 ) )
      {
        pn1 = pn1 / oflo;
        pn2 = pn2 / oflo;
        pn3 = pn3 / oflo;
        pn4 = pn4 / oflo;
      }
    }

    arg = arg + log ( value );

    if ( elimit <= arg )
    {
      value = 1.0 - exp ( arg );
    }
    else
    {
      value = 1.0;
    }
  }

  return value;
}
//****************************************************************************80

double ppchi2 ( double p, double v, double g, int *ifault )

//****************************************************************************80
//
//  Purpose:
//
//    PPCHI2 evaluates the percentage points of the Chi-squared PDF.
//
//  Discussion
//
//    Incorporates the suggested changes in AS R85 (vol.40(1),
//    pages 233-5, 1991) which should eliminate the need for the limited
//    range for P, though these limits have not been removed
//    from the routine.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    05 June 2013
//
//  Author:
//
//    Original FORTRAN77 version by Donald Best, DE Roberts.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    Donald Best, DE Roberts,
//    Algorithm AS 91:
//    The Percentage Points of the Chi-Squared Distribution,
//    Applied Statistics,
//    Volume 24, Number 3, 1975, pages 385-390.
//
//  Parameters:
//
//    Input, double P,  value of the chi-squared cumulative
//    probability density function.
//    0.000002 <= P <= 0.999998.
//
//    Input, double V, the parameter of the chi-squared probability
//    density function.
//    0 < V.
//
//    Input, double G, the value of log ( Gamma ( V / 2 ) ).
//
//    Output, int *IFAULT, is nonzero if an error occurred.
//    0, no error.
//    1, P is outside the legal range.
//    2, V is not positive.
//    3, an error occurred in GAMMAD.
//    4, the result is probably as accurate as the machine will allow.
//
//    Output, double PPCHI2, the value of the chi-squared random
//    deviate with the property that the probability that a chi-squared random
//    deviate with parameter V is less than or equal to PPCHI2 is P.
//
{
  double a;
  double aa = 0.6931471806;
  double b;
  double c;
  double c1 = 0.01;
  double c2 = 0.222222;
  double c3 = 0.32;
  double c4 = 0.4;
  double c5 = 1.24;
  double c6 = 2.2;
  double c7 = 4.67;
  double c8 = 6.66;
  double c9 = 6.73;
  double c10 = 13.32;
  double c11 = 60.0;
  double c12 = 70.0;
  double c13 = 84.0;
  double c14 = 105.0;
  double c15 = 120.0;
  double c16 = 127.0;
  double c17 = 140.0;
  double c18 = 175.0;
  double c19 = 210.0;
  double c20 = 252.0;
  double c21 = 264.0;
  double c22 = 294.0;
  double c23 = 346.0;
  double c24 = 420.0;
  double c25 = 462.0;
  double c26 = 606.0;
  double c27 = 672.0;
  double c28 = 707.0;
  double c29 = 735.0;
  double c30 = 889.0;
  double c31 = 932.0;
  double c32 = 966.0;
  double c33 = 1141.0;
  double c34 = 1182.0;
  double c35 = 1278.0;
  double c36 = 1740.0;
  double c37 = 2520.0;
  double c38 = 5040.0;
  double ch;
  double e = 0.5E-06;
  int i;
  int if1;
  int maxit = 20;
  double pmax = 0.999998;
  double pmin = 0.000002;
  double p1;
  double p2;
  double q;
  double s1;
  double s2;
  double s3;
  double s4;
  double s5;
  double s6;
  double t;
  double value;
  double x;
  double xx;
//
//  Test arguments and initialize.
//
  value = - 1.0;

  if ( p < pmin || pmax < p )
  {
    *ifault = 1;
    return value;
  }

  if ( v <= 0.0 )
  {
    *ifault = 2;
    return value;
  }

  *ifault = 0;
  xx = 0.5 * v;
  c = xx - 1.0;
//
//  Starting approximation for small chi-squared
//
  if ( v < - c5 * log ( p ) )
  {
    ch = pow ( p * xx * exp ( g + xx * aa ), 1.0 / xx );

    if ( ch < e )
    {
      value = ch;
      return value;
    }
  }
//
//  Starting approximation for V less than or equal to 0.32
//
  else if ( v <= c3 )
  {
    ch = c4;
    a = log ( 1.0 - p );

    for ( ; ; )
    {
      q = ch;
      p1 = 1.0 + ch * ( c7 + ch );
      p2 = ch * (c9 + ch * ( c8 + ch ) );

      t = - 0.5 + (c7 + 2.0 * ch ) / p1 - ( c9 + ch * ( c10 + 
      3.0 * ch ) ) / p2;

      ch = ch - ( 1.0 - exp ( a + g + 0.5 * ch + c * aa ) * p2 / p1) / t;

      if ( r8_abs ( q / ch - 1.0 ) <= c1 )
      {
        break;
      }
    }
  }
  else
  {
//
//  Call to algorithm AS 111 - note that P has been tested above.
//  AS 241 could be used as an alternative.
//
    x = ppnd ( p, ifault );
//
//  Starting approximation using Wilson and Hilferty estimate
//
    p1 = c2 / v;
    ch = v * pow ( x * sqrt ( p1 ) + 1.0 - p1, 3 );
//
//  Starting approximation for P tending to 1.
//
    if ( c6 * v + 6.0 < ch )
    {
      ch = - 2.0 * ( log ( 1.0 - p ) - c * log ( 0.5 * ch ) + g );
    }
  }
//
//  Call to algorithm AS 239 and calculation of seven term
//  Taylor series
//
  for ( i = 1; i <= maxit; i++ )
  {
    q = ch;
    p1 = 0.5 * ch;
    p2 = p - gammad ( p1, xx, &if1 );

    if ( if1 != 0 )
    {
      *ifault = 3;
      return value;
    }

    t = p2 * exp ( xx * aa + g + p1 - c * log ( ch ) );
    b = t / ch;
    a = 0.5 * t - b * c;
    s1 = ( c19 + a * ( c17 + a * ( c14 + a * ( c13 + a * ( c12 + 
    c11 * a ))))) / c24;
    s2 = ( c24 + a * ( c29 + a * ( c32 + a * ( c33 + c35 * a )))) / c37;
    s3 = ( c19 + a * ( c25 + a * ( c28 + c31 * a ))) / c37;
    s4 = ( c20 + a * ( c27 + c34 * a) + c * ( c22 + a * ( c30 + c36 * a ))) / c38;
    s5 = ( c13 + c21 * a + c * ( c18 + c26 * a )) / c37;
    s6 = ( c15 + c * ( c23 + c16 * c )) / c38;
    ch = ch + t * ( 1.0 + 0.5 * t * s1 - b * c * ( s1 - b * 
    ( s2 - b * ( s3 - b * ( s4 - b * ( s5 - b * s6 ))))));

    if ( e < r8_abs ( q / ch - 1.0 ) )
    {
       value = ch;
       return value;
    }
  }

 *ifault = 4;
 value = ch;

  return value;
}
//****************************************************************************80

double ppnd ( double p, int *ifault )

//****************************************************************************80
//
//  Purpose:
//
//    PPND produces the normal deviate value corresponding to lower tail area = P.
//
//  Licensing:
//
//    This code is distributed under the GNU LGPL license. 
//
//  Modified:
//
//    21 January 2008
//
//  Author:
//
//    Original FORTRAN77 version by J Beasley, S Springer.
//    C++ version by John Burkardt.
//
//  Reference:
//
//    J Beasley, S Springer,
//    Algorithm AS 111:
//    The Percentage Points of the Normal Distribution,
//    Applied Statistics,
//    Volume 26, Number 1, 1977, pages 118-121.
//
//  Parameters:
//
//    Input, double P, the value of the cumulative probability
//    densitity function.  0 < P < 1.
//
//    Output, integer *IFAULT, error flag.
//    0, no error.
//    1, P <= 0 or P >= 1.  PPND is returned as 0.
//
//    Output, double PPND, the normal deviate value with the property that
//    the probability of a standard normal deviate being less than or
//    equal to PPND is P.
//
{
  double a0 = 2.50662823884;
  double a1 = -18.61500062529;
  double a2 = 41.39119773534;
  double a3 = -25.44106049637;
  double b1 = -8.47351093090;
  double b2 = 23.08336743743;
  double b3 = -21.06224101826;
  double b4 = 3.13082909833;
  double c0 = -2.78718931138;
  double c1 = -2.29796479134;
  double c2 = 4.85014127135;
  double c3 = 2.32121276858;
  double d1 = 3.54388924762;
  double d2 = 1.63706781897;
  double r;
  double split = 0.42;
  double value;

  *ifault = 0;
//
//  0.08 < P < 0.92
//
  if ( r8_abs ( p - 0.5 ) <= split )
  {
    r = ( p - 0.5 ) * ( p - 0.5 );

    value = ( p - 0.5 ) * ( ( ( 
        a3   * r 
      + a2 ) * r 
      + a1 ) * r 
      + a0 ) / ( ( ( ( 
        b4   * r 
      + b3 ) * r 
      + b2 ) * r 
      + b1 ) * r 
      + 1.0 );
  }
//
//  P < 0.08 or P > 0.92,
//  R = min ( P, 1-P )
//
  else if ( 0.0 < p && p < 1.0 )
  {
    if ( 0.5 < p )
    {
      r = sqrt ( - log ( 1.0 - p ) );
    }
    else
    {
      r = sqrt ( - log ( p ) );
    }

    value = ( ( ( 
        c3   * r 
      + c2 ) * r 
      + c1 ) * r 
      + c0 ) / ( ( 
        d2   * r 
      + d1 ) * r 
      + 1.0 );

    if ( p < 0.5 )
    {
      value = - value;
    }
  }
//
//  P <= 0.0 or 1.0 <= P
//
  else
  {
    *ifault = 1;
    value = 0.0;
  }

  return value;
}
//****************************************************************************80
