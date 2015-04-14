/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <seiscomp3/math/geo.h>
#include <seiscomp3/math/math.h>

#define WGS84_SEMI_MAJOR_AXIS 6378137.0
#define WGS84_FLATTENING (1.0/298.2572235630)


namespace Seiscomp
{
namespace Math
{
namespace Geo
{

static int _delazi(double lat1,  double lon1, double lat2, double lon2,
                   double *dist, double *azi, double *baz) {
	double a,b,gam,cosa,cosb,cosc,sina,sinb,sinc,azi1,azi2,delta;
	if (lat1==lat2 && lon1==lon2) {
		*dist = *azi = *baz = 0.;
		return 0;
	}
	a     = M_PI_2 - lat2;
	b     = M_PI_2 - lat1;
	gam   = lon1 - lon2;
	cosa  = cos(a);
	cosb  = cos(b);
	sina  = sin(a);
	sinb  = sin(b);
	cosc  = cosa*cosb + sinb*sina*cos(gam);
	delta = acos(cosc);
	sinc  = sin(delta);
	azi1  = acos((cosa-cosb*cosc)/(sinb*sinc));
	azi2  = acos((cosb-cosa*cosc)/(sina*sinc));

	if ((isNaN(azi1) || isNaN(azi2)) && fabs(lon2-lon1)<0.000001) {
		if (lat1>lat2) {
			azi1 = M_PI;
			azi2 = 0.;
		}
		else {
			azi1 = 0.;
			azi2 = M_PI;
		}
	}
	else {
		if (sin(gam)<0.)
			azi2 = M_PI+M_PI - azi2;
		else
			azi1 = M_PI+M_PI - azi1;
	}
	*dist = delta;
	*azi  = azi1;
	*baz  = azi2;
	return 0;
}

void delazi(double lat1, double lon1, double lat2, double lon2,
            double *dist, double *azi1, double *azi2) {
	double pi180 = M_PI/180., ip180 = 180./M_PI;
	_delazi(lat1*pi180, lon1*pi180, lat2*pi180, lon2*pi180,
	        dist, azi1, azi2);
// XXX	if (err) return err; // FIXME throw an exception
	*dist *= ip180;
	*azi1 *= ip180;
	*azi2 *= ip180;
}

/* !To be moved into another package
void delazi(const Origin &origin, const Station &station,
	    double *out_dist, double *out_azi1, double *out_azi2)
{
	double lat1 = value(origin.latitude());
	double lon1 = value(origin.longitude());
	double lat2 = value(station.latitude());
	double lon2 = value(station.longitude());

	delazi(lat1, lon1, lat2, lon2, out_dist, out_azi1, out_azi2);
}
*/


static void mb_geocr( double lon, double lat, double *a, double *b, double *c ) {
	/* local variables */
	double   blbda, bphi, ep, ug, vg;

	/* executable code */

	blbda = deg2rad( lon );
	bphi = deg2rad( lat );
	ep = 1.0 - WGS84_FLATTENING;
	ug = ep*ep*tan(bphi);
	vg = 1.0/sqrt(1.0+ug*ug);
	*a = vg*cos(blbda);
	*b = vg*sin(blbda);
	*c = ug*vg;

} /* end of mb_geocr */


static double mb_azm(double x, double y) {
	/* local variables */
	double   th;

	/* executable code */

	if  (x == 0.0)  {
		if  (y > 0.0)  return 90.0;
		if  (y < 0.0)  return 270.0;
		return 0.0;
	} /*endif*/

	th = rad2deg( atan(fabs(y/x)) );
	if  (x > 0.0)  {
		if  (y < 0.0)  return (360.0-th);
		return th;
	} else {
		if  (y >= 0.0)  return (180.0-th);
		return (180.0+th);
	} /*endif*/
} /* end of mb_azm */


void delazi_wgs84(double elat, double elon, double slat, double slon,
                  double *distance, double *azim, double *bazim) {
	/* returns distance and azimuth in degrees of two locations on
	 * earth
	 *
	 * parameters of routine
	 * double     slat, slon;    input; latitude and longitude of station
	 * double     elat, elon;    input; latitude and longitude of epicentre
	 * double     *distance;     output; distance in degrees
	 * double     *azim;         output; azimuth in degrees
	 * double     *bazim;        output; back-azimuth in degrees
	 */
	/* local variables */
	double   as, bs, cs, ds;
	double   ae, be, ce, de;
	double   bls, cbls, sbls, ble;
	double   codel, bgdel;
	double   xi, xj, xk;
	double   sindt, cosz, sinz;

	/* executable code */

	/* check for equality */
	as = fabs(slat-elat) + fabs(slon-elon);
	if  (as < 1.0e-5)  {
		*distance = 0.0;
		*azim = 0.0;
		*bazim = 0.0;
		return;
	} /*endif*/

	mb_geocr( slon, slat, &as, &bs, &cs );
	ds = sqrt( 1.0 - cs*cs );
	mb_geocr( elon, elat, &ae, &be, &ce );
	de = sqrt( 1.0 - ce*ce );

	bls = deg2rad( slon );
	cbls = cos( bls );
	sbls = sin( bls );
	codel = ae*as + be*bs + ce*cs;

	sindt = sqrt( 1.0-codel*codel );
	if  (codel == 0.0)  {
		bgdel = M_PI/2.0;
	} else {
		bgdel = atan( fabs(sindt/codel) );
		if  (codel <= 0.0)
			bgdel = M_PI - bgdel;
	} /*endif*/

	*distance = rad2deg( bgdel );

	/* azimuths */
	xi = bs*ce - be*cs;
	xj = as*ce - ae*cs;
	xk = as*be - ae*bs;
	cosz = (xi*sbls + xj*cbls)/sindt;
	sinz = xk/(ds*sindt);
	*bazim = mb_azm( cosz, sinz );
	ble = deg2rad( elon );
	cosz = -(xi*sin(ble) + xj*cos(ble))/sindt;
	sinz = -xk/(de*sindt);
	*azim = mb_azm( cosz, sinz );
}


static int _delandaz2coord(double d, double az, double lat0, double lon0,
                           double *lat, double *lon) {
	double a, b, gam, cosa, cosb, cosd, sina, sinb, sind;

	if ( d > M_PI ) {
		d = M_PI*2 - d;
		az += M_PI;
	}

	b    = (M_PI_2 - lat0);
	cosb = cos(b);  sinb = sin(b);
	cosd = cos(d);  sind = sin(d);
	cosa = cosb*cosd + sinb*sind*cos(az);
	a    = acos(cosa);
	sina = sin(a);
	gam = (cosd - cosa*cosb)/(sina*sinb);
	if (gam > 1.) gam = 1.;
	if (gam <-1.) gam =-1.;
	gam = acos(gam);
	if (sin(az) < 0.) gam = -gam;

	*lat = M_PI_2 - a;
	*lon = fmod (lon0+gam+M_PI, 2*M_PI) - M_PI;

	return 0;
}

void delandaz2coord(double dist, double azi, double lat0, double lon0,
                    double *lat, double *lon) {
	double pi180 = M_PI/180., ip180 = 180./M_PI;
	_delandaz2coord(dist*pi180, azi*pi180, lat0*pi180, lon0*pi180, lat, lon);
// XXX	if (err) return err;
	*lat *= ip180;
	*lon *= ip180;
}


static int _scxsc(double lat1, double lon1, double r1,
                  double lat2, double lon2, double r2,
                  double *latx1, double *lonx1, double *latx2, double *lonx2) {
	double d,a,b,alpha2,cd1,cd2,ca2,sa2,s,px,py,p,dd,latx,lonx;
	double twopi = M_PI+M_PI;

	_delazi(lat1, lon1, lat2, lon2, &d, &a, &b);

	// we transform the coordinate system such that the 1st circle is on
	// top of the sphere, i.e. alpha1 = 0, which results in great
	// simplification of the computation.

	if (d==0.) return 0; // Circles don't intersect (centers coincide)

	alpha2 = d;
	cd1 = cos(r1);
	cd2 = cos(r2);

	ca2 = cos(alpha2);
	sa2 = sin(alpha2);

	s   = (cd2*ca2-cd1)/sa2;

	px = cd2*sa2 + s*ca2;
	py = cd2*ca2 - s*sa2;
	p  = sqrt(px*px+py*py);
	if (p>1.) return 0; // Circles don't intersect
	dd = acos(p);

	p = atan2(px,py);

	_delandaz2coord(p, a, lat1, lon1, &latx, &lonx);
	_delazi(lat1, lon1, latx, lonx, &d, &a, &b);

	// TODO
	// Also treat case that circles just "touch". This is a bit
	// tricky, because it requires a tolerance.

	_delandaz2coord(dd, b+M_PI_2, latx, lonx, latx1, lonx1);
	_delandaz2coord(dd, b-M_PI_2, latx, lonx, latx2, lonx2);

	*lonx1 = fmod(*lonx1+twopi+M_PI, twopi) - M_PI;
	*lonx2 = fmod(*lonx2+twopi+M_PI, twopi) - M_PI;
	return 2;
}

int scxsc(double lat1, double lon1, double r1,
          double lat2, double lon2, double r2,
          double *latx1, double *lonx1, double *latx2, double *lonx2,
          double epsilon) {
	int npt;
	double pi180 = M_PI/180., ip180 = 180./M_PI;
	npt = _scxsc(lat1*pi180, lon1*pi180, r1*pi180,
		     lat2*pi180, lon2*pi180, r2*pi180,
		     latx1, lonx1, latx2, lonx2);
	if(npt==0) {
		*latx1 = *lonx1 = *latx2 = *lonx2 = 0.;
		return 0;
	}
	*latx1 *= ip180;
	*lonx1 *= ip180;
	*latx2 *= ip180;
	*lonx2 *= ip180;
	return 2;
}

int scdraw(double lat0, double lon0, double radius,
           int n, double *lat, double *lon) {
	double da=360./n;
	int i;

	for (i=0; i<n; i++) {
		delandaz2coord(radius, i*da, lat0, lon0, lat+i, lon+i);
	}
	return 0;
}

void _xyz2ltp(double x, double y, double z,
              double *lat, double *lon, double *alt) {
	double a0, a1, a2, a3, a4, b0, b1, b2, b3;
	double b, c, opqk, qk, qkc, qks, f, fprm;
	long int k;
	double ytemp, xtemp;

	// semi major axis squared
	static double asqr = WGS84_SEMI_MAJOR_AXIS * WGS84_SEMI_MAJOR_AXIS;
	// 1st eccentricity squared
	static double esqr = WGS84_FLATTENING * (2.0 - WGS84_FLATTENING);
	// 1st eccentricity to fourth
	static double efor = esqr * esqr;
	// 1 minus eccentricity squared
	static double omes = 1.0 - esqr;

	b = (x * x + y * y) / asqr;
	c = z * z / asqr;

	a0 = omes * c;
	a1 = 2.0 * a0;
	a2 = a0 + b - efor;
	a3 = -2.0 * efor;
	a4 = - efor;

	b0 = 4.0 * a0;
	b1 = 3.0 * a1;
	b2 = 2.0 * a2;
	b3 = a3;

	// compute first eccentricity squared
	qk = esqr;
	for ( k = 1; k <= 3; ++k ) {
		qks = qk * qk;
		qkc = qk *  qks;
		f = a0 *  qks * qks + a1 * qkc + a2 *  qks +
		a3 * qk + a4;
		fprm= b0 * qkc + b1 *  qks + b2 * qk + b3;
		qk = qk - (f / fprm);
	}

	// compute latitude, longitude, altitude
	opqk = 1.0 + qk;

	//special case: point resides on the earth's axis
	if ( x == 0.0 && y == 0.0 ) {
		*lat = z >= 0.0 ? M_PI / 2 : -M_PI / 2;
		*lon = 0.0;
	}
	else {
		ytemp = opqk * z;
		xtemp = sqrt(x * x + y * y);
		*lat = atan2(ytemp, xtemp);
		*lon = atan2(y, x);
	}
	*alt = (1.0 - omes / esqr * qk) * WGS84_SEMI_MAJOR_AXIS *
	       sqrt(b / (opqk * opqk) + c);
}

void xyz2ltp(double x, double y, double z,
             double *lat, double *lon, double *alt) {
	static double rad2deg = 360.0 / (2.0 * M_PI);
	_xyz2ltp(x, y, z, lat, lon, alt);
	*lat *= rad2deg;
	*lon *= rad2deg;
}

void _ltp2xyz(double lat, double lon, double alt,
              double *x, double *y, double *z) {
	// 1st eccentricity squared
	static double esqr = WGS84_FLATTENING * (2.0 - WGS84_FLATTENING);
	// 1 minus eccentricity squared
	static double omes = 1.0 - esqr;

	double slat = sin(lat);
	double clat = cos(lat);
	double r = WGS84_SEMI_MAJOR_AXIS / sqrt(1.0 - esqr * slat * slat);

	*x = (r + alt) * clat * cos(lon);
	*y = (r + alt) * clat * sin(lon);
	*z = (r * omes + alt) * slat;
}

void ltp2xyz(double lat, double lon, double alt,
             double *x, double *y, double *z) {
	static double deg2rad = (2.0 * M_PI) / 360.0;
	_ltp2xyz(lat * deg2rad, lon * deg2rad, alt, x, y, z);
}


const NamedCoordD* nearestHotspot(double lat, double lon, double maxDist,
                                  int nCoords, const NamedCoordD* coordArray,
                                  double *dist, double *azi) {
	double minDist = 180.0;
	double minAzi;
	const NamedCoordD* minCoord = NULL;

	for ( int i = 0; i < nCoords; ++i, ++coordArray ) {
		const NamedCoordD& coord = *coordArray;
		double dist, azi1, azi2;
		delazi(lat, lon, coord.lat, coord.lon, &dist, &azi1, &azi2);

		if ( dist < minDist ) {
			minDist = dist;
			minAzi = azi2;
			minCoord = coordArray;
		}
	}

	if ( minCoord && minDist <= maxDist ) {
		if ( dist ) *dist = minDist;
		if ( azi ) *azi = minAzi;
		return minCoord;
	}

	return NULL;
}


const NamedCoordD* nearestHotspot(double lat, double lon, double maxDist,
                                  const std::vector<NamedCoordD>& cities,
                                  double *dist, double *azi) {
	if ( cities.empty() ) return NULL;
	return nearestHotspot(lat, lon, maxDist, cities.size(),
	                      &cities[0], dist, azi);
}


const CityD* nearestCity(double lat, double lon,
                         double maxDist, double minPopulation,
                         int nCities, const CityD* cityArray,
                         double *dist, double *azi) {
	double minDist = 180.0;
	double minAzi;
	const CityD* minCity = NULL;

	for ( int i = 0; i < nCities; ++i, ++cityArray ) {
		const CityD& city = *cityArray;
		double dist, azi1, azi2;
		delazi(lat, lon, city.lat, city.lon, &dist, &azi1, &azi2);

		if ( city.population() < minPopulation ) continue;

		if ( dist < minDist ) {
			minDist = dist;
			minAzi = azi2;
			minCity = cityArray;
		}
	}

	if ( minCity && minDist <= maxDist ) {
		if ( dist ) *dist = minDist;
		if ( azi ) *azi = minAzi;
		return minCity;
	}

	return NULL;
}

const CityD* nearestCity(double lat, double lon,
                         double maxDist, double minPopulation,
                         const std::vector<CityD>& cities,
                         double *dist, double *azi) {
	if ( cities.empty() ) return NULL;
	return nearestCity(lat, lon, maxDist, minPopulation,
	                   (int)cities.size(), &cities[0], dist, azi);
}

/**
 * Returns a pointer to the largest city within a distanz of 'maxDist' from a
 * given Point('lat', 'lon'). Fills the precise distanz too and azimuth too.
 * If the given set of cities is empty returns NULL.
 */
const CityD* largestCity(double lat, double lon, double maxDist,
                         const std::vector<CityD>& cities,
                         double *dist, double *azi) {
	if( cities.empty() ) return NULL;

	double cdist = 0.0, cazi = 0.0;
	int population = 0;
	const CityD *largest = NULL;
	size_t nCities = cities.size();
	for( size_t i = 0; i < nCities ; ++i ) {
		const CityD *city = &cities[i];
		double dist, azi1, azi2;
		delazi( lat, lon, city->lat, city->lon, &dist, &azi1, &azi2);
		if( dist > maxDist ) continue;
		if( city->population() > population ) {
			population	= (int)city->population();
			cdist		= dist;
			cazi		= azi1;
			largest		= city;
		}
	}
	if( largest ) {
		if (dist) *dist = cdist;
		if (azi)  *azi  = cazi;
	}
	return largest;
}






PositionInterpolator::PositionInterpolator(double lat1, double lon1,
                                           double lat2, double lon2,
                                           int steps)
 : _lat1(lat1), _lon1(lon1), _lat2(lat2), _lon2(lon2), _nSteps(steps) {
	double tmp;
	delazi(_lat1, _lon1, _lat2, _lon2, &_dist, &_azimuth, &tmp);
	_stepDist = _dist / (double)_nSteps;
	_currentDist = 0.0;
	_currentStep = 0;

	update();
}

PositionInterpolator::PositionInterpolator(double lat1, double lon1,
                                           double lat2, double lon2,
                                           double stepsDistScale)
 : _lat1(lat1), _lon1(lon1), _lat2(lat2), _lon2(lon2) {
	double tmp;
	delazi(_lat1, _lon1, _lat2, _lon2, &_dist, &_azimuth, &tmp);
	_nSteps = (int)(_dist*stepsDistScale);
	if ( _nSteps > 0 )
		_stepDist = _dist / (double)_nSteps;
	else {
		_nSteps = 1;
		_stepDist = _dist;
	}
	_currentDist = 0.0;
	_currentStep = 0;

	update();
}

PositionInterpolator& PositionInterpolator::operator++() {
	++_currentStep;
	_currentDist += _stepDist;

	update();

	return *this;
}

PositionInterpolator PositionInterpolator::operator++(int) {
	PositionInterpolator ret(*this);
	++(*this);
	return ret;
}

bool PositionInterpolator::end() const {
	return _currentStep > _nSteps;
}

void PositionInterpolator::update() {
	delandaz2coord(_currentDist, _azimuth, _lat1, _lon1, &_lat, &_lon);
}


} // namespace Seiscomp::Math::Geo

} // namespace Seiscomp::Math

} // namespace Seiscomp
