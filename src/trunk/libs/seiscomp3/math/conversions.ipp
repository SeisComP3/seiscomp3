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

template <typename T>
void vec2angle(const Math::Vector3<T> &v, T &strike, T &dip) {
	if ( fabs(v.x) < 0.0001 && fabs(v.y) < 0.0001 ) {
		strike = 0;
		dip = 0;
	}
	else {
		// TODO: Is this correct?
		if ( v.z < 0 ) {
			strike = (T)(atan2(-v.y,-v.x));
			dip = (T)(acos(-v.z));
		}
		else {
			strike = (T)(atan2(v.y,v.x));
			dip = (T)(acos(v.z));
		}

		if ( strike < 0 ) strike += (2*M_PI);

		strike = rad2deg(strike);
	}

	dip = rad2deg(HALF_PI-dip);
}


template <typename T>
void angle2vec(T strike, T dip, Math::Vector3<T> &v) {
	T rdip = deg2rad(dip);
	T rstrike = deg2rad(strike);

	v.x = cos(rdip)*cos(rstrike);
	v.y = cos(rdip)*sin(rstrike);
	v.z = sin(rdip);
}


template <typename T>
bool pa2nd(const Math::Vector3<T> &t, const Math::Vector3<T> &p, Math::Vector3<T> &n, Math::Vector3<T> &d) {
	n = t + p;
	n.normalize();

	d = t - p;
	d.normalize();

	if ( n.z > 0 ) {
		n *= -1;
		d *= -1;
	}

	return true;
}



template <typename T>
bool nd2pa(const Math::Vector3<T> &n, const Math::Vector3<T> &d, Math::Vector3<T> &t, Math::Vector3<T> &p) {
	p = n - d;
	p.normalize();
	if ( p.z < 0 )
		p *= -1;

	t = n + d;
	t.normalize();
	if ( t.z < 0 )
		t *= -1;

	return true;
}


template <typename T>
bool nd2tensor(const Math::Vector3<T> &n, const Math::Vector3<T> &d, Math::Tensor2S<T> &t) {
	t._11 = 2*d.x*n.x;
	t._12 = d.x*n.y+d.y*n.x;
	t._13 = d.x*n.z+d.z*n.x;
	t._22 = 2*d.y*n.y;
	t._23 = d.y*n.z+d.z*n.y;
	t._33 = 2*d.z*n.z;

	return true;
}


template <typename T>
bool np2tensor(const NODAL_PLANE &np, Math::Tensor2S<T> &t) {
	Math::Vector3<T> n, d;
	np2nd(np, n, d);
	nd2tensor(n, d, t);
	return true;
}


template <typename T>
bool nd2np(const Math::Vector3<T> &n, const Math::Vector3<T> &d, NODAL_PLANE &np) {
	if ( fabs(n.z) == 1 ) {
		np.dip = 0;
		np.str = 0;
		np.rake = atan2(-d.y,d.x);
	}
	else {
		np.dip = acos(-n.z);
		np.str = atan2(-n.x, n.y);
		np.rake = atan2(-d.z/sin(np.dip),d.x*cos(np.str)+d.y*sin(np.str));
	}

	np.str = (double)fmod(np.str+(2*M_PI),(2*M_PI));

	return true;
}


template <typename T>
bool np2nd(const NODAL_PLANE &np, Math::Vector3<T> &n, Math::Vector3<T> &d) {
	T rdip = deg2rad(np.dip), rstr = deg2rad(np.str), rrake = deg2rad(np.rake);

	n.x = -sin(rdip)*sin(rstr);
	n.y =  sin(rdip)*cos(rstr);
	n.z = -cos(rdip);

	d.x =  cos(rrake)*cos(rstr)+cos(rdip)*sin(rrake)*sin(rstr);
	d.y =  cos(rrake)*sin(rstr)-cos(rdip)*sin(rrake)*cos(rstr);
	d.z = -sin(rdip) *sin(rrake);

	return true;
}


template <typename T>
bool nd2dc(const Math::Vector3<T> &n, const Math::Vector3<T> &d, NODAL_PLANE *NP1, NODAL_PLANE *NP2) {
	// Compute DC from normal and slip vector
	nd2np(n, d, *NP1);

	Math::Vector3<T> n1(d);
	Math::Vector3<T> d1(n);

	// Rotate n-d system be 180 degree
	if ( n1.z > 0 ) {
		n1 *= -1;
		d1 *= -1;
	}

	// Compute DC from normal (rotated slip) and slip (rotated normal) vector
	nd2np(n1, d1, *NP2);

	// Convert both nodal planes from radiant to degree
	np2deg(*NP1);
	np2deg(*NP2);

	return true;
}


template <typename T>
void rtp2tensor(T mrr, T mtt, T mpp, T mrt, T mrp, T mtp, Math::Tensor2S<T> &tensor) {
	tensor._33 = mrr;
	tensor._11 = mtt;
	tensor._22 = mpp;
	tensor._13 = mrt;
	tensor._23 = -mrp;
	tensor._12 = -mtp;
}


template <typename T>
double spectral2clvd(const Math::Spectral2S<T> &spec) {
	double mean = (spec.a1 + spec.a2 + spec.a3) / 3;
	double v[3] = {fabs(spec.a1-mean), fabs(spec.a2-mean), fabs(spec.a3-mean)};
	std::sort(v, v+3);

	double eps = v[0] / v[2];
	return 200.0*eps;
}


template <typename T>
void spectral2axis(const Math::Spectral2S<T> &spec,
                   AXIS &t, AXIS &n, AXIS &p, int expo) {
	t.val = spec.a1; t.expo = expo;
	n.val = spec.a2; n.expo = expo;
	p.val = spec.a3; p.expo = expo;

	vec2angle(spec.n1, t.str, t.dip);
	vec2angle(spec.n2, n.str, n.dip);
	vec2angle(spec.n3, p.str, p.dip);
}


template <typename T>
void axis2spectral(const AXIS &t, const AXIS &n, const AXIS &p, int expo,
                   Math::Spectral2S<T> &spec) {
	angle2vec(t.str, t.dip, spec.n1);
	angle2vec(n.str, n.dip, spec.n2);
	angle2vec(p.str, p.dip, spec.n3);

	spec.a1 = t.val * pow((T)10, (T)t.expo-expo);
	spec.a2 = n.val * pow((T)10, (T)n.expo-expo);
	spec.a3 = p.val * pow((T)10, (T)p.expo-expo);
}


template <typename T>
void axis2tensor(const AXIS &t, const AXIS &n, const AXIS &p, int expo,
                 Math::Tensor2S<T> &tensor) {
	Math::Spectral2S<T> spec;
	axis2spectral(t, n, p, expo, spec);
	spec.compose(tensor);
}


template <typename T1, typename T2>
void spectral2matrix(const Math::Spectral2S<T1> &spec, Math::Matrix3<T2> &m) {
	Math::Vector3<T1> x3, x1, x2;

	// Convert principle axis to tensor orientation
	pa2nd(spec.n1, spec.n3, x3, x1);

	// Calculate null axis
	x2.cross(x3, x1);

	// Slip axis
	m.setColumn(0, Math::Vector3<T2>(x1.x,x1.y,x1.z));
	// Dip axis
	m.setColumn(1, Math::Vector3<T2>(x2.x,x2.y,x2.z));
	// Fault normal
	m.setColumn(2, Math::Vector3<T2>(x3.x,x3.y,x3.z));
}


template <typename T1, typename T2>
bool tensor2matrix(const Math::Tensor2S<T1> &t, Math::Matrix3<T2> &m) {
	Math::Spectral2S<T1> spec;
	if ( !spec.spect(t) ) return false;

	spec.sort();
	spectral2matrix(spec, m);

	return true;
}
