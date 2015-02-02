/***************************************************************************
 * Copyright
 * ---------
 * This file is part of the Virtual Seismologist (VS) software package.
 * VS is free software: you can redistribute it and/or modify it under
 * the terms of the "SED Public License for Seiscomp Contributions"
 *
 * VS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the SED Public License for Seiscomp
 * Contributions for more details.
 *
 * You should have received a copy of the SED Public License for Seiscomp
 * Contributions with VS. If not, you can find it at
 * http://www.seismo.ethz.ch/static/seiscomp_contrib/license.txt
 *
 * Authors of the Software: Michael Fischer and Yannik Behr
 * Copyright (C) 2006-2013 by Swiss Seismological Service
 ***************************************************************************/

#include <cmath>
#include <iostream>
#include <seiscomp3/math/math.h>
#include <seiscomp3/math/geo.h>

#include "equations.h"

VsEquations::VsEquations() {
	norm = 1.0;
}

VsEquations::~VsEquations() {
}

const float VsEquations::attenuation[2][6][2][7] =
//   a       b       c1     c2     d     e       sigma
{{{{ 0.72, -3.3e-3 , 1.6 , 1.05, -1.2 , -1.06   , 0.31 }, //P,HA,rock
   { 0.74, -2.5e-3 , 2.41, 0.95, -1.26, -1.05   , 0.29 }}, //P,HA,soil
  {{ 0.80, -8.4e-4 , 0.76, 1.03, -1.24, -3.103  , 0.27 }, //P,HV,rock
   { 0.84, -5.4e-4 , 1.21, 0.97, -1.28, -3.13   , 0.26 }}, //P,HV,soil
  {{ 0.95, -1.7e-7 , 2.16, 1.08, -1.27, -4.96   , 0.28 }, //P,HD,rock
   { 0.94, -5.17e-7, 2.26, 1.02, -1.16, -5.01   , 0.3  }}, //P,HD,soil
  {{ 0.74, -4.01e-3, 1.75, 1.09, -1.2 , -0.96   , 0.29 }, //P,ZA,rock
   { 0.74, -5.17e-7, 2.03, 0.97, -1.2 , -0.77   , 0.31 }}, //P,ZA,soil
  {{ 0.82, -8.54e-4, 1.14, 1.10, -1.36, -2.901  , 0.26 }, //P,ZV,rock
   { 0.81, -2.65e-6, 1.4 , 1.0 , -1.48, -2.55   , 0.30 }}, //P,ZV,soil
  {{ 0.96, -1.98e-6, 1.66, 1.16, -1.34, -4.79   , 0.28 }, //P,ZD,rock
   { 0.93, -1.09e-7, 1.5 , 1.04, -1.23, -4.74   , 0.31 }}}, //P,ZD,soil
 {{{ 0.73, -7.2e-4 , 1.16, 0.96, -1.48, -0.42   , 0.31 }, //S,HA,rock
   { 0.71, -2.38e-3, 1.72, 0.96, -1.44, -2.45e-2, 0.33 }}, //S,HA,soil
  {{ 0.86, -5.58e-4, 0.84, 0.98, -1.37, -2.58   , 0.28 }, //S,HV,rock
   { 0.89, -8.4e-4 , 1.39, 0.95, -1.47, -2.24   , 0.32 }}, //S,HV,soil
  {{ 1.03, -1.01e-7, 1.09, 1.13, -1.43, -4.34   , 0.27 }, //S,HD,rock
   { 1.08, -1.2e-6 , 1.95, 1.09, -1.56, -4.1    , 0.32 }}, //S,HD,soil
  {{ 0.78, -2.7e-3 , 1.76, 1.11, -1.38, -0.75   , 0.30 }, //S,ZA,rock
   { 0.75, -2.47e-3, 1.59, 1.01, -1.47, -0.36   , 0.30 }}, //S,ZA,soil
  {{ 0.90, -1.03e-3, 1.39, 1.09, -1.51, -2.78   , 0.25 }, //S,ZV,rock
   { 0.88, -5.41e-4, 1.53, 1.04, -1.48, -2.54   , 0.27 }}, //S,ZV,soil
  {{ 1.04, -1.12e-5, 1.38, 1.18, -1.37, -4.74   , 0.25 }, //S,ZD,rock
   { 1.03, -4.92e-6, 1.55, 1.08, -1.36, -4.57   , 0.28 }}}}; //S,ZD,soil

const float VsEquations::geta(int PSclass, int SoilClass, int WaveType) {
	return VsEquations::attenuation[PSclass][SoilClass][WaveType][0];
}

const float VsEquations::getb(int PSclass, int SoilClass, int WaveType) {
	return VsEquations::attenuation[PSclass][SoilClass][WaveType][1];
}

const float VsEquations::getc1(int PSclass, int SoilClass, int WaveType) {
	return VsEquations::attenuation[PSclass][SoilClass][WaveType][2];
}

const float VsEquations::getc2(int PSclass, int SoilClass, int WaveType) {
	return VsEquations::attenuation[PSclass][SoilClass][WaveType][3];
}

const float VsEquations::getd(int PSclass, int SoilClass, int WaveType) {
	return VsEquations::attenuation[PSclass][SoilClass][WaveType][4];
}

const float VsEquations::gete(int PSclass, int SoilClass, int WaveType) {
	return VsEquations::attenuation[PSclass][SoilClass][WaveType][5];
}

const float VsEquations::getsigma(int PSclass, int SoilClass, int WaveType) {
	return VsEquations::attenuation[PSclass][SoilClass][WaveType][6];
}

float VsEquations::ground_motion_ratio(float ZA, float ZD) {
	return 0.36 * log10(ZA) - 0.93 * log10(ZD);
}

int VsEquations::psclass(float ZA, float ZV, float HA, float HV) {
	// return 0 for P-wave and 1 for S-wave
	float ps = 0.43 * log10(ZA) + 0.55 * log10(ZV) - 0.46 * log10(HA)
			- 0.55 * log10(HV);
	return ps > -0.01 ? 0 : 1;
}

float VsEquations::mest(float ZAD, const int PSclass) {
	return PSclass == 1 ? -1.45 * ZAD + 8.05 : -1.627 * ZAD + 8.94;
}

float VsEquations::mesterr(const int PSclass) {
	return PSclass == 1 ? 0.41 : 0.45;
}

float VsEquations::zavg(const int PSclass) {
	return PSclass == 1 ? -0.685 * mag + 5.52 : -0.615 * mag + 5.496;
}

float VsEquations::zavgerr(const int PSclass) {
	return PSclass == 1 ? 0.25 : 0.276;
}

float VsEquations::saturation(const float c1, const float c2) {
	float f1 = c1 * (atan(mag - 5) + 1.4);
	float f2 = exp(c2 * (mag - 5));
	return f1 * f2;
}

float VsEquations::edist(const float stlat, const float stlon) {
	// compute epicentral distance in km
	double distdg, azi1, azi2;
	Seiscomp::Math::Geo::delazi(eqlat, eqlon, stlat, stlon, &distdg, &azi1,
			&azi2);
	return (float) Seiscomp::Math::Geo::deg2km(distdg);
}

float VsEquations::edistalt(const float stlat, const float stlon) {
	// assume a constant depth of 3 km
	return sqrt(pow(VsEquations::edist(stlat, stlon), 2) + 9);
}

float VsEquations::amplitude(const float a, const float b, const float c1,
		const float c2, const float d, const float e, const float stlat,
		const float stlon) {
	// Envelope attenuation relationship
	float f0 = VsEquations::edistalt(stlat, stlon)
			+ VsEquations::saturation(c1, c2);
	float f1 = a * mag;
	float f2 = b * f0;
	float f3 = d * log10(f0);
	float f4 = e;
	// I think according to the specifications the following should be:
	// f1 - f2 - f3 + f4
	// It looks like the right sign is handled by changing the sign of b
	// and d in the corresponding table
	return f1 + f2 + f3 + f4;
}

enum {
	ha, hv, hd, za, zv, zd
};

float VsEquations::likelihood(float ZA, float ZV, float ZD, float HA, float HV,
		float HD, int PSclass, int Soilclass, float stlat, float stlon) {
	float a, b, c1, c2, d, e, sigma, sigma_zad, f3;
	float envelopes[4] = { ZV, HA, HV, HD };
	int wavetypes[4] = { zv, ha, hv, hd };
	sigma_zad = VsEquations::zavgerr(PSclass);
	float f1 = pow(
			VsEquations::ground_motion_ratio(ZA, ZD)
					- VsEquations::zavg(PSclass), 2)
			/ (2 * pow(sigma_zad, 2));
	norm *= 1.0/(sqrt(2*M_PI)*sigma_zad);

	float f2 = 0;
	for ( int kk = 0; kk < 4; kk++ ) {
		a = VsEquations::geta(PSclass, wavetypes[kk], Soilclass);
		b = VsEquations::getb(PSclass, wavetypes[kk], Soilclass);
		c1 = VsEquations::getc1(PSclass, wavetypes[kk], Soilclass);
		c2 = VsEquations::getc2(PSclass, wavetypes[kk], Soilclass);
		d = VsEquations::getd(PSclass, wavetypes[kk], Soilclass);
		e = VsEquations::gete(PSclass, wavetypes[kk], Soilclass);
		sigma = VsEquations::getsigma(PSclass, wavetypes[kk], Soilclass);
		norm *= 1.0/(sqrt(2*M_PI)*sigma);
		f3 = pow((log10(envelopes[kk]) -
				VsEquations::amplitude(a, b, c1, c2, d, e, stlat,stlon)), 2) /
				(2 * pow(sigma, 2));
		f2 += f3;
	}
	return f1 + f2;
}

void VsEquations::setmag(float magnitude) {
	mag = magnitude;
}

const float VsEquations::getmag() {
	return mag;
}

void VsEquations::setnorm(float norminit) {
	norm = norminit;
}

const float VsEquations::getnorm() {
	return norm;
}

void VsEquations::seteqlat(float eventlat) {
	eqlat = eventlat;
}

const float VsEquations::geteqlat() {
	return eqlat;
}

void VsEquations::seteqlon(float eventlon) {
	eqlon = eventlon;
}

const float VsEquations::geteqlon() {
	return eqlon;
}
