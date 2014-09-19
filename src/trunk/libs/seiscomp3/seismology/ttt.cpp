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



#include <seiscomp3/seismology/ttt.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/core/interfacefactory.ipp>


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::TravelTimeTableInterface, SC_SYSTEM_CORE_API);


extern "C" {

#include "f2c.h"
void distaz2_(double *lat1, double *lon1, double *lat2, double *lon2, double *delta, double *azi1, double *azi2);
int elpcor_(const char *phid, real *del, real *z__, real *azi, real *ecolat, real *ecorr, int phid_len);

}


namespace Seiscomp {


bool ellipcorr(const std::string &phase, double lat1, double lon1, double lat2, double lon2, double depth, double &corr)
{
	corr = 0.;
	double delta, azi1, azi2;
	Seiscomp::Math::Geo::delazi(lat1, lon1, lat2, lon2, &delta, &azi1, &azi2);
	real staazi=azi1, stadel=delta, zfoc = depth, colat = 90. - lat1, ecorr=0;

	if (phase=="P" || phase=="Pn" || phase=="Pg" || phase=="Pb" || phase=="Pdif" || phase=="Pdiff")
		elpcor_("P       ", &stadel, &zfoc, &staazi, &colat, &ecorr, 8);
	else if (phase=="PcP")
		elpcor_("PcP     ", &stadel, &zfoc, &staazi, &colat, &ecorr, 8);
	else if (phase=="PKPab")
		elpcor_("PKPab   ", &stadel, &zfoc, &staazi, &colat, &ecorr, 8);
	else if (phase=="PKPbc")
		elpcor_("PKPbc   ", &stadel, &zfoc, &staazi, &colat, &ecorr, 8);
	else if (phase=="PKPdf")
		elpcor_("PKPdf   ", &stadel, &zfoc, &staazi, &colat, &ecorr, 8);
	else if (phase=="PKiKP")
		elpcor_("PKiKP   ", &stadel, &zfoc, &staazi, &colat, &ecorr, 8);
	else if (phase=="S" || phase=="Sn" || phase=="Sg" || phase=="Sb" || phase=="Sdif" || phase=="Sdiff")
		elpcor_("S       ", &stadel, &zfoc, &staazi, &colat, &ecorr, 8);
	else if (phase=="ScS")
		elpcor_("ScS     ", &stadel, &zfoc, &staazi, &colat, &ecorr, 8);
	else if (phase=="SKSac")
		elpcor_("SKSac   ", &stadel, &zfoc, &staazi, &colat, &ecorr, 8);
	else if (phase=="SKSdf")
		elpcor_("SKSdf   ", &stadel, &zfoc, &staazi, &colat, &ecorr, 8);
	else if (phase=="ScP")
		elpcor_("SKP     ", &stadel, &zfoc, &staazi, &colat, &ecorr, 8);
	else if (phase=="SKP")
		elpcor_("ScP     ", &stadel, &zfoc, &staazi, &colat, &ecorr, 8);
	else return false;

	corr = ecorr;
	return true;
}



TravelTime::TravelTime() {}

TravelTime::TravelTime(const std::string &_phase,
                       double _time, double _dtdd, double _dtdh, double _dddp,
                       double _takeoff)
{
	phase   = _phase;
	time    = _time;
	dtdd    = _dtdd;
	dtdh    = _dtdh;
	dddp    = _dddp;
	takeoff = _takeoff;
}

bool TravelTime::operator==(const TravelTime &other) const {
	return phase == other.phase &&
	       time == other.time &&
	       dtdd == other.dtdd &&
	       dtdh == other.dtdh &&
	       dddp == other.dddp &&
	       takeoff == other.takeoff;
}

bool TravelTime::operator<(const TravelTime &other) const {
	return phase < other.phase &&
	       time < other.time &&
	       dtdd < other.dtdd &&
	       dtdh < other.dtdh &&
	       dddp < other.dddp &&
	       takeoff < other.takeoff;
}


namespace {

struct TTpred {
	bool operator()(const TravelTime &t1, const TravelTime &t2)
	{
		return t1.time < t2.time;
	}
};

}


void TravelTimeList::sortByTime() {
	sort(TTpred());
}


const TravelTime *getPhase(const TravelTimeList *list, const std::string &phase) {
	TravelTimeList::const_iterator it;

	for ( it = list->begin(); it != list->end(); ++it ) {
		// direct match
		if ( (*it).phase == phase ) break;

		// no match for 1st character -> don't keep trying
		if ( (*it).phase[0] != phase[0] )
			continue;

		if ( phase == "P" ) {
			if ( list->delta < 120 ) {
				if ( (*it).phase == "Pn"    ) break;
				if ( (*it).phase == "Pb"    ) break;
				if ( (*it).phase == "Pg"    ) break;
				if ( (*it).phase == "Pdiff" ) break;
			}
			else
				if ( (*it).phase.substr(0,3) == "PKP" ) break;
		}
		else if ( phase == "pP" ) {
			if ( list->delta < 120 ) {
				if ( (*it).phase == "pPn"   ) break;
				if ( (*it).phase == "pPb"   ) break;
				if ( (*it).phase == "pPg"   ) break;
				if ( (*it).phase == "pPdiff") break;
			}
			else {
				if ( (*it).phase.substr(0,4) == "pPKP" ) break;
			}
		}
		else if ( phase == "PKP" ) {
			if ( list->delta > 100 ) {
				if ( (*it).phase == "PKPab" ) break;
				if ( (*it).phase == "PKPbc" ) break;
				if ( (*it).phase == "PKPdf" ) break;
			}
		}
		else if ( phase == "PKKP" ) {
			if ( list->delta > 100 && list->delta < 130 ) {
				if ( (*it).phase == "PKKPab" ) break;
				if ( (*it).phase == "PKKPbc" ) break;
				if ( (*it).phase == "PKKPdf" ) break;
			}
		}
		else if ( phase == "SKP" ) {
			if ( list->delta > 115 && list->delta < 145 ) {
				if ( (*it).phase == "SKPab" ) break;
				if ( (*it).phase == "SKPbc" ) break;
				if ( (*it).phase == "SKPdf" ) break;
			}
		}
		else if ( phase == "PP" ) {
			if ( (*it).phase == "PnPn" ) break;
		}
		else if ( phase == "sP" ) {
			if ( list->delta < 120 ) {
				if ( (*it).phase == "sPn"   ) break;
				if ( (*it).phase == "sPb"   ) break;
				if ( (*it).phase == "sPg"   ) break;
				if ( (*it).phase == "sPdiff") break;
			}
			else {
				if ( (*it).phase.substr(0,4)=="sPKP" ) break;
			}
		}
		else if ( phase == "S" ) {
			if ( (*it).phase == "Sn"   ) break;
			if ( (*it).phase == "Sb"   ) break;
			if ( (*it).phase == "Sg"   ) break;
			if ( (*it).phase == "S"    ) break;
			if ( (*it).phase == "Sdiff") break;
			if ( (*it).phase.substr(0,3) == "SKS" ) break;
		}
	}

	if ( it == list->end() )
		return NULL;

	return &(*it);
}


const TravelTime* firstArrivalP(const TravelTimeList *ttlist)
{
	TravelTimeList::const_iterator it;
	for (it = ttlist->begin(); it != ttlist->end(); ++it) {
		// direct match
		if ((*it).phase == "P")
			break;

		// no match for 1st character -> don't keep trying
		if ( (*it).phase[0] != 'P' ) continue;

		if ( ttlist->delta < 120 ) {
			if ( (*it).phase == "Pn"   ) break;
			if ( (*it).phase == "Pb"   ) break;
			if ( (*it).phase == "Pg"   ) break;
			if ( (*it).phase == "Pdiff") break;
		}
		else {
			if ( (*it).phase.substr(0,3)=="PKP" ) break;
		}
	}

	if ( it == ttlist->end() )
		return NULL;

	return &(*it);
}


TravelTimeTableInterface::TravelTimeTableInterface() {}
TravelTimeTableInterface::~TravelTimeTableInterface() {}


TravelTimeTableInterface *TravelTimeTableInterface::Create(const char *name) {
	return TravelTimeTableInterfaceFactory::Create(name);
}


TravelTime TravelTimeTableInterface::compute(const char *phase,
                                             double lat1, double lon1, double dep1,
                                             double lat2, double lon2, double alt2,
                                             int ellc) throw(std::exception) {
	TravelTimeList *ttlist = compute(lat1, lon1, dep1, lat2, lon2, alt2, ellc);
	if ( ttlist == NULL )
		throw NoPhaseError();

	TravelTime ret;
	const TravelTime *tt = getPhase(ttlist, phase);

	if ( tt == NULL ) {
		delete ttlist;
		throw NoPhaseError();
	}

	ret = *tt;
	delete ttlist;
	return ret;
}


TravelTimeTableInterfacePtr TravelTimeTable::_interface;


TravelTimeTable::TravelTimeTable() {
	if ( !_interface )
		_interface = TravelTimeTableInterfaceFactory::Create("libtau");
}


bool TravelTimeTable::setModel(const std::string &model) {
	if ( _interface )
		return _interface->setModel(model);
	return false;
}


const std::string &TravelTimeTable::model() const {
	static std::string empty;
	if ( _interface )
		return _interface->model();
	return empty;
}


TravelTimeList *
TravelTimeTable::compute(double lat1, double lon1, double dep1,
                         double lat2, double lon2, double alt2,
                         int ellc) {
	if ( _interface )
		return _interface->compute(lat1, lon1, dep1, lat2, lon2, alt2, ellc);
	return NULL;
}


TravelTime
TravelTimeTable::compute(const char *phase,
                         double lat1, double lon1, double dep1,
                         double lat2, double lon2, double alt2,
                         int ellc) throw(std::exception) {
	if ( _interface )
		return _interface->compute(phase, lat1, lon1, dep1, lat2, lon2, alt2, ellc);
	throw NoPhaseError();
}


TravelTime
TravelTimeTable::computeFirst(double lat1, double lon1, double dep1,
                              double lat2, double lon2, double alt2,
                              int ellc) throw(std::exception) {
	if ( _interface )
		return _interface->computeFirst(lat1, lon1, dep1, lat2, lon2, alt2, ellc);
	throw NoPhaseError();
}


}
