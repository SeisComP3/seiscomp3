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
 * Authors of the Software: Stefan Heimers and Yannik Behr
 * Copyright (C) 2006-2013 by Swiss Seismological Service
 ***************************************************************************/

#define SEISCOMP_COMPONENT VsMagnitude

#include <seiscomp3/logging/log.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/client/inventory.h>

#include "scvsmag.h"

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Math;
using namespace Seiscomp::Processing;

/*!
 \brief Test if the vs event is valid
 \param stmag median of all single station magnitudes
 \param evt  pointer to an event to be checked for validity
 */
bool VsMagnitude::isEventValid(double stmag, VsEvent *evt, double &likelihood,
		double &deltamag, double &deltapick) {

	double lh = 1.0;
	double merr;

	deltamag = deltaMag(stmag, *evt->vsMagnitude);
	deltapick = deltaPick(evt);
	merr = valid_magnitude_error(stmag);
	if (deltamag > merr)
		lh *= 0.4;
	if (deltapick > 0.5)
		lh *= 0.3;
	if (evt->azGap > evt->maxAzGap)
		lh *= 0.2;
	if ((1.0 - lh) < 0.001) {
		likelihood = 0.99;
	} else {
		likelihood = lh;
	}
	if ( lh > 0.5 )
		return true;
	return false;
}

/*
 * Lookup the permissible magnitude error
 */
double VsMagnitude::valid_magnitude_error(double stmag){
	if ( stmag < 1.5 ) {
		return 0.5;
	} else if ( stmag < 2.0 ) {
		return 0.4;
	} else if ( stmag < 2.5 ) {
		return 0.3;
	} else if ( stmag < 3.0 ) {
		return 0.25;
	} else if ( stmag < 4.0 ) {
		return 0.2;
	} else {
		return 0.18;
	}
}

/*!
 \brief ForQC:  evaluates the difference between the single station magnitudes 
 and the VS magnitude.

 \param vsmag VS magnitude
 \param stmag median of single station magnitudes

 Currently two quality criteria are applied once a new magnitude estimate is available.

 The first (deltaMag) evaluates the difference between the single station magnitudes 
 (see Appendix A: Single station magnitudes) and the VS magnitude. Locations are rejected
 if there is too large scatter across individual magnitude estimates. The difference is
 computed as follows:

 fabs(vsmag - stmag)/max(vsmag,stmag);

 */
double VsMagnitude::deltaMag(double vsmag, double stmag) {

	return fabs(vsmag - stmag) / max(vsmag, stmag);
}

/*!
 \brief For QC: ratio of picked stations to total stations inside a limited radius around the epicenter

 \param evt VsEvent to be evaluated for the picked station ratio

 The second quality criteria measures the ratio between the stations that reported a pick to the overall
 number of stations within a given radius. Included are only stations that are also processed by the
 Envelope module and which are used for picking by scautopick. Locations are rejected if there are
 too many stations in the epicentral region that have not been used for the picking.

 The numbers are already in the evt class, this only computes the ratio as follows:

 deltapick =  evt->pickedThresholdStationsCount/(double)evt->allThresholdStationsCount

 The numbers used are computed in VsMagnitude::handleEvent()

 */
double VsMagnitude::deltaPick(VsEvent *evt) {
	double deltapick;

	deltapick = (double) evt->pickedThresholdStationsCount
			- (double) evt->allThresholdStationsCount;
	return fabs(deltapick)
			/ max((double) evt->pickedThresholdStationsCount,
					(double) evt->allThresholdStationsCount);
}

