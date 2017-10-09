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


#include <seiscomp3/gui/core/tensorrenderer.h>
#include <seiscomp3/math/conversions.h>

#include <iostream>
#include <math.h>
#include <QPainter>


namespace Seiscomp {
namespace Gui {


namespace {

/*
struct Point {
	Point(double x=0, double y=0, double z=0) : x(x), y(y), z(z) {}
	double x,y,z;
};



class BeachballPlot {

  public:
	int plot(Math::AXIS ax[3]);

	// There are in general nodes, but one may be invisible.
	std::vector<Point> node1, node2, dc_node1, dc_node2;

	// Polygons are the nodes with the adjacent circle segment
	// This can be used for filling.
	std::vector<Point> poly1, poly2, dc_poly1, dc_poly2;

	// The projected axis piercing points
	Point axis[3];

	int d,m,n;
  private:
	void make_node(double f, double I, double *x, double *y, double *z);
	void proj_node(double *x,double *y,double *z, std::vector<Point> &node1, std::vector<Point> &node2);
	void make_poly(const std::vector<Point> &node1, const std::vector<Point> &node2,
			     std::vector<Point> &poly1,       std::vector<Point> &poly2);
	double val[3], dip[3], str[3];
	int zerocrossings;
};


static void normalize(double &x, double &y, double &z) {
	double q = 1./sqrt(x*x+y*y+z*z);
	x*=q; y*=q; z*=q;
}


static double takeoff2radius(double takeoff) {
	// from psmeca
	return sin(takeoff/2)*sqrt(2.);
}

static Point zerox(const Point &p1, const Point &p2) {
	// There must be a zerocrossing between point1 and point2.
	// Return the point at which the zerocrossing occurs.
	double q = -p1.z/(p2.z-p1.z);
	double x = p1.x + q*(p2.x-p1.x);
	double y = p1.y + q*(p2.y-p1.y);
	q = 1./sqrt(x*x+y*y);
	return Point(q*x, q*y);
}

static void mkpoly(const std::vector<Point> &node, std::vector<Point> &poly)
{
	// Make a closed polygon from the node line by including the
	// corresponding unit circle perimeter segment.
	int last = node.size()-1;

	double phi1 = ceil(180*atan2(node[last].x, node[last].y)/M_PI+0.999);
	double phi2 = ceil(180*atan2(node[0].x,    node[0].y)/M_PI);
	if (phi2 < phi1) phi2 += 360;

	poly = node;
	for (int i=(int)phi1; i<(int)phi2; i++) {
		double phi = i*M_PI/180;
	        poly.push_back( Point(sin(phi), cos(phi), 0) );
	}
}

static void sort(Math::AXIS ax[3]) {
	if (ax[0].val<ax[1].val) { Math::AXIS tmp=ax[0]; ax[0]=ax[1]; ax[1]=tmp; }
	if (ax[1].val<ax[2].val) { Math::AXIS tmp=ax[1]; ax[1]=ax[2]; ax[2]=tmp; }
	if (ax[0].val<ax[1].val) { Math::AXIS tmp=ax[0]; ax[0]=ax[1]; ax[1]=tmp; }
}

void BeachballPlot::make_node(double f, double I, double *x, double *y, double *z)
{
	// p must be 361 points long!

	// north x, east y, up z
	Point Ad( cos(dip[d])*cos(str[d]), cos(dip[d])*sin(str[d]), sin(dip[d]) );
	Point An( cos(dip[n])*cos(str[n]), cos(dip[n])*sin(str[n]), sin(dip[n]) );
	Point Am( cos(dip[m])*cos(str[m]), cos(dip[m])*sin(str[m]), sin(dip[m]) );

	for(int i=0; i<=360; i++) {
		// (8)
		double phi = deg2rad(i);
		double alpha_N = asin(sqrt((2.+2.*I)/(3.+(1.-2.*f)*cos(2.*phi))));
		// (6)
		// unit vector describing the node in the DBM system
		double Ed = cos(alpha_N);
		double En = sin(alpha_N)*sin(phi);
		double Em = sin(alpha_N)*cos(phi);

		x[i] = Ed*Ad.x + En*An.x + Em*Am.x;
		y[i] = Ed*Ad.y + En*An.y + Em*Am.y;
		z[i] = Ed*Ad.z + En*An.z + Em*Am.z;
		normalize(x[i],y[i],z[i]); // force point to be on unit sphere

		double tak = acos(fabs(z[i]));
		double azi = atan2(y[i], x[i]);
		double r = takeoff2radius(tak);
		x[i] = r*sin(azi);
		y[i] = r*cos(azi);
	}
}

void BeachballPlot::proj_node(double *x, double *y, double *z, std::vector<Point> &node1, std::vector<Point> &node2)
{
	int first_zerocrossing = 0;
	zerocrossings = 0;
	for(int i=0; i<360; i++) {
		if (z[i]*z[i+1] <= 0) {
			zerocrossings += 1;
			if (z[i+1] > 0)
				first_zerocrossing = i+1;
		}
	}

	Point zero;

	if (zerocrossings) {
		node1.push_back(zero);
		node2.push_back(zero);
	}

	for (int i=0; i<361; i++) {
		int k = (i+first_zerocrossing)%361;
		if (z[k] >= 0) node1.push_back(Point(x[k],y[k],z[k]));
		if (z[k] <= 0) node2.push_back(Point(x[k],y[k],z[k]));
	}

	if (zerocrossings) {
		node1.push_back(zero);
		node2.push_back(zero);
	}

	if (zerocrossings > 1 && node1.size() > 2 && node2.size() > 2) {
		node1[node1.size()-1] = node2[0] = zerox(node1[node1.size()-2], node2[1]);
		node2[node2.size()-1] = node1[0] = zerox(node2[node2.size()-2], node1[1]);
	}

	std::reverse(node2.begin(), node2.end());
	for (int i=0; i<(int)node2.size(); i++) {
		Point &p = node2[i];
		p.x=-p.x; p.y=-p.y; p.z=-p.z;
	}

	// Determine the direction of the node line w.r.t. the dominant axis.
	// We want to draw in clockwise direction, i.e. q < 0
	// whereas if the line is currently drawn in anticlockwise direction
	// (and q>0) the node order needs to be reversed.
	//
	// There must be a more elegant way but for now it works. :)

	double sum=0;
	double dx = axis[d].x, dy = axis[d].y;
	for (int i=1; i<(int)node1.size(); i++) {
		Point &p0 = node1[i-1], &p1 = node1[i];
		double px = p1.x - p0.x, xx = p0.x - dx;
		double py = p1.y - p0.y, yy = p0.y - dy;
		sum += xx*py-yy*px;
	}
	if (sum>0) {
		std::reverse(node1.begin(), node1.end());
		std::reverse(node2.begin(), node2.end());
	}

}

void BeachballPlot::make_poly(
	const std::vector<Point> &node1, const std::vector<Point> &node2,
	      std::vector<Point> &poly1,       std::vector<Point> &poly2)
{
	if (zerocrossings==0) {
		poly1 = node1;
		poly2.clear();
	}
	else if (zerocrossings==2) {
		mkpoly(node1, poly1);
		mkpoly(node2, poly2);
	}
	else {
		// FIXME
	}
}

int BeachballPlot::plot(Math::AXIS ax[3])
{
	// Implemented after:
	//
	// Frohlich, C.
	// Cliff's Nodes Concerning Plotting Nodal Lines for P, SH and SV
	// Seismological Research Letters, 1996, 27, 16-24

	sort(ax); // sort axes in descending order according to value

	node1.clear(); node2.clear();
	poly1.clear(); poly2.clear();

	val[0]=ax[0].val; val[1]=ax[1].val; val[2]=ax[2].val;
	dip[0]=ax[0].dip; dip[1]=ax[1].dip; dip[2]=ax[2].dip;
	str[0]=ax[0].str; str[1]=ax[1].str; str[2]=ax[2].str;

	// In the following cases there are no nodes at all:
	if (val[0]>=0 && val[1]>=0 && val[2]>=0) return 0;
	if (val[0]<=0 && val[1]<=0 && val[2]<=0) return 0;

	double I = (val[0]+val[1]+val[2])/3., f=0;
	double Tx = val[0]-I, Px = val[2]-I, Nx = val[1]-I;
	if (fabs(Tx) > fabs(Px)) {
		d=0; m=2; n=1;
		f = -Nx/Tx;
		I =   I/Tx;
	}
	else if (fabs(Tx) < fabs(Px)) {
		d=2; m=0; n=1;
		f = -Nx/Px;
		I =   I/Px;
	}
	else {  // pure strike-slip
		I = 0;
		// dirty hack
		f = 1E-8;
		d=2; m=0; n=1;
	}

	if (I < -1)       // implosion
		return 0;
	else if (I > 1-f) // explosion
		return 0;

	// from now on there is at least one "visible" node

	for (int i=0; i<3; i++) {
		str[i] = deg2rad(str[i]);
		dip[i] = deg2rad(dip[i]);
	}

	// markers for the principal axes
	for (int i=0; i<3; i++) {
		double r = takeoff2radius(M_PI*.5 - dip[i]);
		axis[i] = Point(r*sin(str[i]), r*cos(str[i]));
	}

	double x[361],y[361],z[361];

	make_node(f, I, x,y,z);
	proj_node(x,y,z, node1, node2);
	make_poly(node1, node2, poly1, poly2);

	// DC
	make_node(1E-8, 0, x,y,z);
	x[0] = x[360] = axis[n].x;
	y[0] = y[360] = axis[n].y;
	z[0] = z[360] = axis[n].z;
	proj_node(x,y,z, dc_node1, dc_node2);
	make_poly(dc_node1, dc_node2, dc_poly1, dc_poly2);

	return 0;
}
*/


}




TensorRenderer::TensorRenderer() {
	_colorT = Qt::red;
	_colorP = Qt::white;
	_border = Qt::black;
	_shading = false;
	_margin = 0;
	_projectMargin = 0;
	_materialAmbient = 0.75;
	_materialDiffuse = 0.25;
}


void TensorRenderer::setShadingEnabled(bool e) {
	_shading = e;
}


void TensorRenderer::setMaterial(float ambient, float diffuse) {
	_materialAmbient = ambient;
	_materialDiffuse = diffuse;
}


void TensorRenderer::setMargin(int m) {
	_margin = m;
}


void TensorRenderer::setProjectMargin(int m) {
	_projectMargin = m;
}


void TensorRenderer::setBorderColor(QColor c) {
	_border = c;
}


void TensorRenderer::setTColor(QColor c) {
	_colorT = c;
}


void TensorRenderer::setPColor(QColor c) {
	_colorP = c;
}


void TensorRenderer::renderBorder(QImage& img) {
	QSize size(img.size());

	int diameter = std::min(size.width(), size.height());
	_radius = diameter / 2;
	_projectRadius = _radius - _projectMargin;
	_ballRadius = _radius - _margin;

	_center = QPoint(size.width()/2, size.height()/2);

	float dt = 1.0f / float(_ballRadius-1);

	float ixf = -_center.x() * dt;
	float iyf = -_center.y() * dt;

	QRgb c;
	QRgb *data = (QRgb *)img.bits();

	//float smoothDist = 1.5f*dt;
	float smoothDist = std::max(0.01f, dt);
	float distLimit = (1.0f - smoothDist) * (1.0f - smoothDist);
	float distLimit2 = (1.0f + smoothDist) * (1.0f + smoothDist);

	QColor fill(Qt::white);

	// Light vector for shading
	Math::Vector3f l(-1,1,-2);
	float yf = iyf;

	l.normalize();

	for ( int i = 0; i < size.height(); ++i, yf += dt ) {
		float xf = ixf;
		for ( int j = 0; j < size.width(); ++j, ++data, xf += dt ) {
			Math::Vector3f v;

			float dist = xf*xf + yf*yf;

			if ( dist > 1.0f ) {
				if ( dist <= distLimit2 ) {
					float d = (sqrt(dist) - 1.0f) / smoothDist;
					*data = qRgba(_border.red(),_border.green(),_border.blue(),(int)(255-d*255));
				}
				else
					*data = qRgba(0,0,0,0);
				continue;
			}

#define STEREO_PROJECTION
#ifdef STEREO_PROJECTION
			float div_z = 1.0f / (1.0f + dist);
			float div_xy = M_SQRT2 * div_z;

			v[0] = yf * div_xy;
			v[1] = -xf * div_xy;
			v[2] = -(1.0f - dist) * div_z;
#else
			v[0] = yf;
			v[1] = xf;
			v[2] = float(sqrt(1.0f - dist));
#endif

			float tmp(smoothDist);
			smoothDist += (1.0 - sqrt(dist)) * smoothDist;

			c = fill.rgb();

			smoothDist = tmp;

			if ( dist > distLimit ) {
				float d = (sqrt(dist) - 1.0f) / smoothDist + 1;
				int intensity = (int)(d * 255);
				if ( intensity > 255 ) intensity = 255;
				if ( intensity < 0 ) intensity = 0;
				int iint = 255 - intensity;
				c = qRgb((qRed(c)*iint + _border.red()*intensity)/255,
				         (qGreen(c)*iint + _border.green()*intensity)/255,
				         (qBlue(c)*iint + _border.blue()*intensity)/255);
			}

			if ( _shading ) {
				float intensity = v.dot(l)*_materialDiffuse + _materialAmbient;
				if ( intensity < 0 ) intensity = 0;
				else if ( intensity > 1 ) intensity = 1;

				*data = qRgb((int)(qRed(c)*intensity),
				             (int)(qGreen(c)*intensity),
				             (int)(qBlue(c)*intensity));
			}
			else
				*data = c;
		}
	}
}


void TensorRenderer::render(QImage& img, double strike, double dip, double slip) {
	Math::Tensor2Sd t;
	Math::Matrix3f m;
	Math::NODAL_PLANE np;
	np.str = strike;
	np.dip = dip;
	np.rake = slip;

	if ( Math::np2tensor(np, t) ) {
		if ( Math::tensor2matrix(t, m) ) {
			render(img, m);
			return;
		}
	}

	img.fill(0);
}


void TensorRenderer::render(QImage& img, const Math::Matrix3d &m) {
	Math::Matrix3f mf;
	for ( int i = 0; i < 3; ++i )
		for ( int j = 0; j < 3; ++j )
			mf.d[i][j] = (float)m.d[i][j];

	render(img, mf);
}


void TensorRenderer::render(QImage& img, const Math::Matrix3f &m) {
	QSize size(img.size());

	int diameter = std::min(size.width(), size.height());
	_radius = diameter / 2;
	_projectRadius = _radius - _projectMargin;
	_ballRadius = _radius - _margin;

	_center = QPoint(size.width()/2, size.height()/2);

	float dt = 1.0f / float(_ballRadius-1);

	float ixf = -_center.x() * dt;
	float iyf = -_center.y() * dt;

	QRgb c;
	QRgb *data = (QRgb *)img.bits();

	//float smoothDist = 1.5f*dt;
	float smoothDist = std::max(0.01f, dt);
	float distLimit = (1.0f - smoothDist) * (1.0f - smoothDist);
	float distLimit2 = (1.0f + smoothDist) * (1.0f + smoothDist);

	// Light vector for shading
	Math::Vector3f l(-1,1,-2);
	float yf = iyf;

	l.normalize();

	for ( int i = 0; i < size.height(); ++i, yf += dt ) {
		float xf = ixf;
		for ( int j = 0; j < size.width(); ++j, ++data, xf += dt ) {
			Math::Vector3f v;

			float dist = xf*xf + yf*yf;

			if ( dist > 1.0f ) {
				if ( dist <= distLimit2 ) {
					float d = (sqrt(dist) - 1.0f) / smoothDist;
					*data = qRgba(_border.red(),_border.green(),_border.blue(),(int)(255-d*255));
				}
				else
					*data = qRgba(0,0,0,0);
				continue;
			}

#define STEREO_PROJECTION
#ifdef STEREO_PROJECTION
			float div_z = 1.0f / (1.0f + dist);
			float div_xy = M_SQRT2 * div_z;

			v[0] = yf * div_xy;
			v[1] = -xf * div_xy;
			v[2] = -(1.0f - dist) * div_z;
#else
			v[0] = yf;
			v[1] = xf;
			v[2] = float(sqrt(1.0f - dist));
#endif

			float tmp(smoothDist);
			smoothDist += (1.0 - sqrt(dist)) * smoothDist;

			Math::Vector3f vr;
			m.invTransform(vr, v);

			float x = vr[0];
			float y = vr[2];

			if ( (x >= 0 && y >= 0) || (x <= 0 && y <= 0) ) {
				float smooth1 = fabs(x) / smoothDist;
				float smooth2 = fabs(y) / smoothDist;

				if ( smooth1 > 1 ) smooth1 = 1;
				if ( smooth2 > 1 ) smooth2 = 1;

				float smooth = smooth1 * smooth2;
				if ( smooth <= 1.0f ) {
					int intens = (int)(smooth * 255);
					int iintens = 255 - intens;
					c = qRgb((_colorT.red() * intens + _border.red() * iintens)/255,
					         (_colorT.green() * intens + _border.green() * iintens)/255,
					         (_colorT.blue() * intens + _border.blue() * iintens)/255);
				}
				else
					c = _colorT.rgb();
			}
			else {
				float smooth1 = fabs(x) / smoothDist;
				float smooth2 = fabs(y) / smoothDist;

				if ( smooth1 > 1 ) smooth1 = 1;
				if ( smooth2 > 1 ) smooth2 = 1;

				float smooth = smooth1 * smooth2;
				if ( smooth <= 1.0f ) {
					int intens = (int)(smooth * 255);
					int iintens = 255 - intens;
					c = qRgb((_colorP.red() * intens + _border.red() * iintens)/255,
					         (_colorP.green() * intens + _border.green() * iintens)/255,
					         (_colorP.blue() * intens + _border.blue() * iintens)/255);
				}
				else
					c = _colorP.rgb();
			}

			smoothDist = tmp;

			if ( dist > distLimit ) {
				float d = (sqrt(dist) - 1.0f) / smoothDist + 1;
				int intensity = (int)(d * 255);
				if ( intensity > 255 ) intensity = 255;
				if ( intensity < 0 ) intensity = 0;
				int iint = 255 - intensity;
				c = qRgb((qRed(c)*iint + _border.red()*intensity)/255,
				         (qGreen(c)*iint + _border.green()*intensity)/255,
				         (qBlue(c)*iint + _border.blue()*intensity)/255);
			}

			if ( _shading ) {
				float intensity = v.dot(l)*_materialDiffuse + _materialAmbient;
				if ( intensity < 0 ) intensity = 0;
				*data = qRgb((int)(qRed(c)*intensity),
				             (int)(qGreen(c)*intensity),
				             (int)(qBlue(c)*intensity));
			}
			else
				*data = c;
		}
	}
}


//#define M_SQRT2 1.41421356237309504880


void TensorRenderer::render(QImage& img, const Math::Tensor2Sd &t, const Math::Matrix3f &m) {
	QSize size(img.size());

	double vi = t.mean();

	double Mxx = t._11 - vi;
	double Myy = t._22 - vi;
	double Mzz = t._33 - vi;

	/*
	memset(img.bits(), 0, img.width()*img.height()*4);

	BeachballPlot bb;
	Math::AXIS ax[3];
	Math::Spectral2Sd spec;
	spec.spect(t);
	Math::spectral2axis(spec, ax[0], ax[1], ax[2], 0);

	bb.plot(ax);

	Math::Vector3d v0;
	Math::angle2vec(ax[0].str, ax[0].dip, v0);

	bool swap = v0.x*v0.x*Mxx + v0.y*v0.y*Myy + v0.z*v0.z*Mzz +
	            2*(v0.x*v0.y*t._12 + v0.x*v0.z*t._13 + v0.y*v0.z*t._23) < 0;

	QPainter p(&img);
	QPolygonF poly;

	int tx = size.width()/2;
	int ty = size.height()/2;
	int sc = std::min(tx,ty);
	sc -= _margin;

	p.translate(tx,ty);
	p.scale(sc,-sc);

	p.setRenderHint(QPainter::Antialiasing);
	p.setBrush(swap?_colorT:_colorP);
	p.drawEllipse(-1,-1,2,2);

	p.setPen(_border);
	p.setBrush(swap?_colorP:_colorT);

	poly.resize(bb.poly1.size());
	for ( int i = 0; i < (int)bb.poly1.size(); ++i )
		poly[i] = QPointF(bb.poly1[i].x, bb.poly1[i].y);
	p.drawPolygon(poly);

	poly.resize(bb.poly2.size());
	for ( int i = 0; i < (int)bb.poly2.size(); ++i )
		poly[i] = QPointF(bb.poly2[i].x, bb.poly2[i].y);
	p.drawPolygon(poly);

	return;
	*/

	double norm = 1.0 / t.norm();

	int diameter = std::min(size.width(), size.height());
	_radius = diameter / 2;
	_projectRadius = _radius - _projectMargin;
	_ballRadius = _radius - _margin;

	_center = QPoint(size.width()/2, size.height()/2);

	float dt = 1.0f / float(_ballRadius-1);

	float ixf = -_center.x() * dt;
	float iyf = -_center.y() * dt;

	QRgb c;
	QRgb *data = (QRgb *)img.bits();

	float smoothDist = std::max(0.01f, dt);

	float distLimit = (1.0f - smoothDist) * (1.0f - smoothDist);
	float distLimit2 = (1.0f + smoothDist) * (1.0f + smoothDist);

	float dist;
	float div_z;
	float div_xy;
	float xx, yy, zz, xy, xz, yz;
	float sign;
	Math::Vector3f v, vr, n, l(-1,1,-2);

	l.normalize();

	n = m.column(2);
	n.normalize();

	float yf = iyf;

	for ( int i = 0; i < size.height(); ++i, yf += dt ) {
		float xf = ixf;
		for ( int j = 0; j < size.width(); ++j, ++data, xf += dt ) {
			dist = xf*xf + yf*yf;
			if ( dist > 1.0f ) {
				if ( dist <= distLimit2 ) {
					float d = (sqrt(dist) - 1.0f) / smoothDist;
					*data = qRgba(_border.red(),_border.green(),_border.blue(),(int)(255-d*255));
				}
				else
					*data = qRgba(0,0,0,0);
				continue;
			}

			div_z = 1.0f / (1.0f + dist);
			div_xy = M_SQRT2 * div_z;

			v[0] = yf * div_xy;
			v[1] = -xf * div_xy;
			v[2] = -(1.0f - dist) * div_z;

			xx = v[0]*v[0];
			xy = v[0]*v[1];
			xz = v[0]*v[2];
			yy = v[1]*v[1];
			yz = v[1]*v[2];
			zz = v[2]*v[2];

			sign = norm*(xx*Mxx + yy*Myy + zz*Mzz + 2*(xy*t._12 + xz*t._13 + yz*t._23));
			if ( sign > 1 ) sign = 1; else if ( sign < -1 ) sign = -1;

			Math::Vector3f vr;
			m.invTransform(vr, v);

			float x = vr[0];
			float y = vr[2];

			float tmp(smoothDist);
			smoothDist += (1.0 - sqrt(dist)) * smoothDist;

			if ( sign >= 0 )
				c = _colorT.rgb();
			else
				c = _colorP.rgb();

			/*
			sign = 0.5 + sign*0.5;
			c = qRgb(qRed(c)*sign, qGreen(c)*sign, qBlue(c)*sign);
			*/

			float smooth1 = fabs(x) / smoothDist;
			float smooth2 = fabs(y) / smoothDist;

			if ( smooth1 > 1 ) smooth1 = 1;
			if ( smooth2 > 1 ) smooth2 = 1;

			float smooth = smooth1 * smooth2;
			if ( smooth <= 1.0f ) {
				int intens = (int)(smooth * 255);
				int iintens = 255 - intens;
				c = qRgb((qRed(c) * intens + _border.red() * iintens)/255,
				         (qGreen(c) * intens + _border.green() * iintens)/255,
				         (qBlue(c) * intens + _border.blue() * iintens)/255);
			}

			smoothDist = tmp;

			if ( dist > distLimit ) {
				float d = (sqrt(dist) - 1.0f) / smoothDist + 1;
				int intensity = (int)(d * 255);
				if ( intensity > 255 ) intensity = 255;
				if ( intensity < 0 ) intensity = 0;
				int iint = 255 - intensity;
				c = qRgb((qRed(c)*iint + _border.red()*intensity)/255,
				         (qGreen(c)*iint + _border.green()*intensity)/255,
				         (qBlue(c)*iint + _border.blue()*intensity)/255);
			}

			if ( _shading ) {
				float intensity = v.dot(l)*_materialDiffuse + _materialAmbient;
				if ( intensity < 0 ) intensity = 0;
				*data = qRgb((int)(qRed(c)*intensity),
				             (int)(qGreen(c)*intensity),
				             (int)(qBlue(c)*intensity));
			}
			else
				*data = c;
		}
	}
}


void TensorRenderer::render(QImage& img, const Math::Tensor2Sd &t) {
	QSize size(img.size());

	double vi = t.mean();

	double Mxx = t._11 - vi;
	double Myy = t._22 - vi;
	double Mzz = t._33 - vi;

	/*
	memset(img.bits(), 0, img.width()*img.height()*4);

	BeachballPlot bb;
	Math::AXIS ax[3];

	Math::Spectral2Sd spec;
	spec.spect(t);
	Math::spectral2axis(spec, ax[0], ax[1], ax[2], 0);

	bb.plot(ax);

	Math::Vector3d v0;
	Math::angle2vec(ax[0].str, ax[0].dip, v0);

	bool swap = v0.x*v0.x*Mxx + v0.y*v0.y*Myy + v0.z*v0.z*Mzz +
	            2*(v0.x*v0.y*t._12 + v0.x*v0.z*t._13 + v0.y*v0.z*t._23) < 0;

	QPainter p(&img);
	QPolygonF poly;

	int tx = size.width()/2;
	int ty = size.height()/2;
	int sc = std::min(tx,ty);
	sc -= _margin;

	p.translate(tx,ty);
	p.scale(sc,-sc);

	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(_border);
	p.setBrush(swap?_colorT:_colorP);
	p.drawEllipse(-1,-1,2,2);

	p.setBrush(swap?_colorP:_colorT);

	poly.resize(bb.poly1.size());
	for ( int i = 0; i < (int)bb.poly1.size(); ++i )
		poly[i] = QPointF(bb.poly1[i].x, bb.poly1[i].y);
	p.drawPolygon(poly);

	poly.resize(bb.poly2.size());
	for ( int i = 0; i < (int)bb.poly2.size(); ++i )
		poly[i] = QPointF(bb.poly2[i].x, bb.poly2[i].y);
	p.drawPolygon(poly);

	return;
	*/

	double norm = 1.0 / t.norm();

	int diameter = std::min(size.width(), size.height());
	_radius = diameter / 2;
	_projectRadius = _radius - _projectMargin;
	_ballRadius = _radius - _margin;

	_center = QPoint(size.width()/2, size.height()/2);

	float dt = 1.0f / float(_ballRadius-1);

	float ixf = -_center.x() * dt;
	float iyf = -_center.y() * dt;

	QRgb c;
	QRgb *data = (QRgb *)img.bits();

	float smoothDist = std::max(0.01f, dt);

	float distLimit = (1.0f - smoothDist) * (1.0f - smoothDist);
	float distLimit2 = (1.0f + smoothDist) * (1.0f + smoothDist);

	float dist;
	float div_z;
	float div_xy;
	float xx, yy, zz, xy, xz, yz;
	float sign;
	Math::Vector3f v, vr, l(-1,1,-2);

	l.normalize();

	float yf = iyf;

	for ( int i = 0; i < size.height(); ++i, yf += dt ) {
		float xf = ixf;
		for ( int j = 0; j < size.width(); ++j, ++data, xf += dt ) {
			dist = xf*xf + yf*yf;
			if ( dist > 1.0f ) {
				if ( dist <= distLimit2 ) {
					float d = (sqrt(dist) - 1.0f) / smoothDist;
					*data = qRgba(_border.red(),_border.green(),_border.blue(),(int)(255-d*255));
				}
				else
					*data = qRgba(0,0,0,0);
				continue;
			}

			div_z = 1.0f / (1.0f + dist);
			div_xy = M_SQRT2 * div_z;

			v[0] = yf * div_xy;
			v[1] = -xf * div_xy;
			v[2] = -(1.0f - dist) * div_z;

			xx = v[0]*v[0];
			xy = v[0]*v[1];
			xz = v[0]*v[2];
			yy = v[1]*v[1];
			yz = v[1]*v[2];
			zz = v[2]*v[2];

			sign = norm*(xx*Mxx + yy*Myy + zz*Mzz + 2*(xy*t._12 + xz*t._13 + yz*t._23));
			if ( sign > 1 ) sign = 1; else if ( sign < -1 ) sign = -1;

			if ( sign >= 0 )
				c = _colorT.rgb();
			else
				c = _colorP.rgb();

			if ( dist > distLimit ) {
				float d = (sqrt(dist) - 1.0f) / smoothDist + 1;
				int intensity = (int)(d * 255);
				if ( intensity > 255 ) intensity = 255;
				if ( intensity < 0 ) intensity = 0;
				int iint = 255 - intensity;
				c = qRgb((qRed(c)*iint + _border.red()*intensity)/255,
				         (qGreen(c)*iint + _border.green()*intensity)/255,
				         (qBlue(c)*iint + _border.blue()*intensity)/255);
			}

			if ( _shading ) {
				float intensity = v.dot(l)*_materialDiffuse + _materialAmbient;
				if ( intensity < 0 ) intensity = 0;
				*data = qRgb((int)(qRed(c)*intensity),
				             (int)(qGreen(c)*intensity),
				             (int)(qBlue(c)*intensity));
			}
			else
				*data = c;
		}
	}
}

void TensorRenderer::renderNP(QImage& img, double strike, double dip, double slip, QColor color) {
	Math::Matrix3f m;
	Math::NODAL_PLANE np;
	np.str = strike;
	np.dip = dip;
	np.rake = slip;

	Math::Vector3f n, d;
	np2nd(np, n, d);

	Math::Vector3f v;
	v = v.cross(n, d);

	m.d[0][0] = d[0];
	m.d[0][1] = v[0];
	m.d[0][2] = n[0];

	m.d[1][0] = d[1];
	m.d[1][1] = v[1];
	m.d[1][2] = n[1];

	m.d[2][0] = d[2];
	m.d[2][1] = v[2];
	m.d[2][2] = n[2];

	QSize size(img.size());

	int diameter = std::min(size.width(), size.height());
	_radius = diameter / 2;
	_projectRadius = _radius - _projectMargin;
	_ballRadius = _radius - _margin;

	_center = QPoint(size.width()/2, size.height()/2);

	float dt = 1.0f / float(_ballRadius-1);

	float ixf = -_center.x() * dt;
	float iyf = -_center.y() * dt;

	QRgb c;
	QRgb *data = (QRgb *)img.bits();

	//float smoothDist = 1.5f*dt;
	float smoothDist = std::max(0.01f, dt);

	float yf = iyf;

	for ( int i = 0; i < size.height(); ++i, yf += dt ) {
		float xf = ixf;
		for ( int j = 0; j < size.width(); ++j, ++data, xf += dt ) {
			Math::Vector3f v;

			float dist = xf*xf + yf*yf;

			if ( dist > 1.0f ) continue;

#define STEREO_PROJECTION
#ifdef STEREO_PROJECTION
			float div_z = 1.0f / (1.0f + dist);
			float div_xy = M_SQRT2 * div_z;

			v[0] = yf * div_xy;
			v[1] = -xf * div_xy;
			v[2] = -(1.0f - dist) * div_z;
#else
			v[0] = yf;
			v[1] = xf;
			v[2] = float(sqrt(1.0f - dist));
#endif

			float tmp(smoothDist);
			smoothDist += (1.0 - sqrt(dist)) * smoothDist;

			Math::Vector3f vr;
			m.invTransform(vr, v);

			float y = vr[2];

			if ( (y >= 0 && y < smoothDist) || (y < 0 && y > -smoothDist) ) {
				float smooth = fabs(y) / smoothDist;
				if ( smooth > 1 ) smooth = 1;

				int intens = (int)(smooth * 255);
				int iintens = 255 - intens;
				c = qRgb((qRed(*data) * intens + color.red() * iintens)/255,
				        (qGreen(*data) * intens + color.green() * iintens)/255,
				        (qBlue(*data) * intens + color.blue() * iintens)/255);
				*data = c;
			}

			smoothDist = tmp;
		}
	}
}


QPoint TensorRenderer::project(Math::Vector3f &v) const {
	float px = -v.y / (1.0 - v.z);
	float py = v.x / (1.0 - v.z);

	return QPoint((int)(_ballRadius*px+_center.x()), (int)(_ballRadius*py+_center.y()));
}


QPoint TensorRenderer::project(Math::Vector3d &v) const {
	double px = -v.y / (1.0 - v.z);
	double py = v.x / (1.0 - v.z);

	return QPoint((int)(_ballRadius*px+_center.x()), (int)(_ballRadius*py+_center.y()));
}


QPoint TensorRenderer::project(double azimuth, double dist) const {
	double azi = deg2rad(azimuth);
	double px = dist*sin(azi);
	double py = dist*cos(azi);

	return QPoint((int)(_projectRadius*px+_center.x()), (int)(_center.y()-_projectRadius*py));
}


QPoint TensorRenderer::projectMargin(double azimuth, int margin, double dist) const {
	double azi = deg2rad(azimuth);
	double px = dist*sin(azi);
	double py = dist*cos(azi);

	return QPoint((int)((_radius-margin)*px+_center.x()), (int)(_center.y()-(_radius-margin)*py));
}


bool TensorRenderer::unproject(Math::Vector3d &v, const QPointF &p) const {
	double xf = double(p.x() - _center.x()) / (double)_ballRadius;
	double yf = double(p.y() - _center.y()) / (double)_ballRadius;

	double dist = xf*xf + yf*yf;

	if ( dist > 1.0 ) {
		v[0] = yf;
		v[1] = -xf;
		v[2] = 0;
		return false;
	}
	else {
		double div_z = 1.0 / (1.0f + dist);
		double div_xy = M_SQRT2 * div_z;
		v[0] = yf * div_xy;
		v[1] = -xf * div_xy;
		v[2] = -(1.0f - dist) * div_z;
		return true;
	}
}


}
}
