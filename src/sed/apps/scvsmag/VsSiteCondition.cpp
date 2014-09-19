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

#define SEISCOMP_COMPONENT VsMagnitude

#include "VsSiteCondition.h"

namespace ch {
namespace sed {

VsSiteCondition::SAF::SAF() {
	// nothing to do
}

VsSiteCondition::SAF::SAF(std::string cn, float vel, float c0, float c1,
		float c2, float c3) {
	_cn = cn;
	_vmax = vel;
	_corr[0] = c0;
	_corr[1] = c1;
	_corr[2] = c2;
	_corr[3] = c3;
}

VsSiteCondition::SAF::~SAF() {
	// nothing to do
}

float VsSiteCondition::SAF::getVmax() {
	return _vmax;
}

float VsSiteCondition::SAF::getCorr(int idx) {
	if ( !(0 <= idx) ) {
		SEISCOMP_ERROR("Index must be equal or greater than zero.");
		return -1;
	}
	if ( !(idx < 4) ) {
		SEISCOMP_ERROR("Index must be smaller than four.");
		return -1;
	}
	return _corr[idx];
}

std::string VsSiteCondition::SAF::toString() {
	return "SAF";
}

VsSiteCondition::SafList::SafList() {
	/**
	 From the ShakeMap manual:

	 Table 2.1 Site Correction Amplification factors. Short-Period (.1 to .5 s) 
	 fact equation 7a, Mid-Period (.4 to 2. s) from equation 7b of Borcherdt 
	 (1994). Clas NEHRP letter classification; Vel is velocity (m/s) maximum 
	 and PGA is cutoff in gals.

	 Site Amplification Factors:
	 Short-Period (PGA)        Mid-Period (PGV)
	 Class Vel  150   250   350           150   250   350
	 */
	_pgalist.push_back(SAF("BC", 724, 0.98, 0.99, 0.99, 1.00));
	_pgalist.push_back(SAF("B", 686, 1.00, 1.00, 1.00, 1.00));
	_pgalist.push_back(SAF("C", 464, 1.15, 1.10, 1.04, 0.98));
	_pgalist.push_back(SAF("CD", 372, 1.24, 1.17, 1.06, 0.97));
	_pgalist.push_back(SAF("E", 301, 1.33, 1.23, 1.09, 0.96));
	_pgalist.push_back(SAF("DE", 298, 1.34, 1.23, 1.09, 0.96));
	_pgalist.push_back(SAF("E", 163, 1.65, 1.43, 1.15, 0.93));

	_pgvlist.push_back(SAF("BC", 724, 0.97, 0.97, 0.97, 0.98));
	_pgvlist.push_back(SAF("B", 686, 1.00, 1.00, 1.00, 1.00));
	_pgvlist.push_back(SAF("C", 464, 1.29, 1.26, 1.23, 1.19));
	_pgvlist.push_back(SAF("CD", 372, 1.49, 1.44, 1.38, 1.32));
	_pgvlist.push_back(SAF("E", 301, 1.71, 1.64, 1.55, 1.45));
	_pgvlist.push_back(SAF("DE", 298, 1.72, 1.65, 1.56, 1.46));
	_pgvlist.push_back(SAF("E", 163, 2.55, 2.37, 2.14, 1.91));
}

VsSiteCondition::SafList::~SafList() {
	_pgvlist.clear();
	_pgalist.clear();
}

float VsSiteCondition::SafList::getCorr(Seiscomp::ValueType valuetype,
		float vs30, float MA) {

	// check if vs30 is defined
	if ( 0 > vs30 ) {
		return 1;
	}

	// MA: maximum acceleration in cm/s/s
	int col = 0;
	if ( false ) {
	} else if ( MA > 350 ) {
		col = 3; //ActEnsure(false);
	} else if ( MA > 250 ) {
		col = 2; //ActEnsure(false);
	} else if ( MA > 150 ) {
		col = 1;
	} else {
		col = 0;
	}

	std::vector<SAF> * saflistP = NULL;
	if ( valuetype == Seiscomp::Acceleration ) {
		saflistP = &(_pgalist);
	}
	if ( valuetype == Seiscomp::Velocity ) {
		saflistP = &(_pgvlist);
	}
	if ( valuetype == Seiscomp::Displacement ) {
		saflistP = &(_pgvlist);
	}
	if ( NULL == saflistP ) {
		SEISCOMP_ERROR(
				"ValueType has to be Acceleration, Velocity or Displacement.");
		return 1.0;
	}

	for ( size_t i = 0; i < saflistP->size(); i++ ) {
		SAF * safP = &(saflistP->at(i));
		if ( vs30 >= safP->getVmax() ) {
			return safP->getCorr(col);
		}
	}

	return 1.0;
}

}
}

// eof
