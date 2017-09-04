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


#define SEISCOMP_COMPONENT STAXML
#include "convert2sc3.h"

#include <fdsnxml/fdsnstationxml.h>
#include <fdsnxml/network.h>
#include <fdsnxml/station.h>
#include <fdsnxml/channel.h>
#include <fdsnxml/response.h>
#include <fdsnxml/responsestage.h>
#include <fdsnxml/coefficients.h>
#include <fdsnxml/fir.h>
#include <fdsnxml/numeratorcoefficient.h>
#include <fdsnxml/polynomial.h>
#include <fdsnxml/polynomialcoefficient.h>
#include <fdsnxml/polesandzeros.h>
#include <fdsnxml/poleandzero.h>
#include <fdsnxml/responselist.h>
#include <fdsnxml/responselistelement.h>
#include <fdsnxml/output.h>

#include <seiscomp3/core/timewindow.h>
#include <seiscomp3/datamodel/inventory_package.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/utils/replace.h>
#include <seiscomp3/logging/log.h>

#include <boost/lexical_cast.hpp>

#include <iostream>
#include <cstdio>
#include <cstring>
#include <map>
#include <set>


using namespace std;

#define LOG_STAGES 1


namespace Seiscomp {

namespace {


#define UNDEFINED   ""
#define VOLTAGE     "V"
#define AMPERE      "A"
#define DIGITAL     "COUNTS"


typedef pair<double,double> Location;
typedef pair<Location,double> LocationElevation;
typedef pair<string,Location> EpochIndex;
typedef pair<string,FDSNXML::Channel*> ChannelEpoch;


bool epochLowerThan(const ChannelEpoch &e1, const ChannelEpoch &e2) {
	return e1.second->startDate() < e2.second->startDate();
}

typedef list<ChannelEpoch> Epochs;
struct EpochEntry {
	EpochEntry() {}
	Epochs epochs;
};


inline bool leap(int y) {
	return (y % 400 == 0 || (y % 4 == 0 && y % 100 != 0));
}


inline int doy(int y, int m) {
	static const int DOY[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
	return (DOY[m] + (leap(y) && m >= 2));
}


inline string date2str(const Core::Time &t) {
	int year, month, day, hour, minute, second;
	t.get(&year, &month, &day, &hour, &minute, &second);

	if ( month < 1 || month > 12 || day < 1 || day > 31 ) {
		SEISCOMP_ERROR("invalid date: month=%d, day=%d", month, day);
		month = 1;
		day = 0;
	}

	char buf[20];
	snprintf(buf, sizeof(buf)-1, "%d.%03d.%02d.%02d.%02d", year, doy(year, month - 1) + day, hour, minute, second);
	return buf;
}


string timeToStr(const Core::Time &time) {
	if ( time.microseconds() == 0 && time.seconds() % 86400 == 0 ) {
		return time.toString("%F");
	}

	return time.toString("%FT%T");
}


bool respLowerThan(const FDSNXML::ResponseStage *r1, const FDSNXML::ResponseStage *r2) {
	return r1->number() < r2->number();
}


typedef pair<int,int> Fraction;

Fraction double2frac(double d) {
	double df = 1;
	Fraction::first_type top = d >= 2.0 ? d-1 : 1, ctop = top;
	Fraction::second_type bot = d <= 0.5 ? 1/d-1 : 1, cbot = bot;
	double error = fabs(df-d);
	double last_error = error*2;
	bool fixed_top = false;

	if ( fabs(d) < 1E-20 )
		return Fraction(0,1);

	while ( error < last_error ) {
		ctop = top;
		cbot = bot;

		//cerr << error << "  " << top << "/" << bot << endl;
		if ( df < d )
			++top;
		else {
			++bot;
			top = Fraction::first_type(d * bot);
		}

		df = (double)top / (double)bot;
		if ( top > 0 ) {
			last_error = error;
			error = fabs(df-d);
			fixed_top = false;
		}
		else if ( fixed_top ) {
			cbot = 1;
			break;
		}
		else
			fixed_top = true;

		if ( top < 0 || bot < 0 )
			return Fraction(0,0);
	}

	return Fraction(ctop,cbot);
}


// Special overlap check for time windows where end time might be open
bool overlaps(const Core::TimeWindow &tw1, const Core::TimeWindow &tw2) {
    if ( tw2.startTime() < tw1.startTime() ) return overlaps(tw2, tw1);
    return !tw1.endTime().valid() || tw1.endTime() > tw2.startTime();
}



template <typename T>
bool overlaps(const T *epoch1, const T *epoch2) {
	Core::TimeWindow tw1, tw2;
	try { tw1.setStartTime(epoch1->startDate()); }
	catch ( ... ) { tw1.setStartTime(Core::Time(1980,1,1)); }
	try { tw1.setEndTime(epoch1->endDate()); } catch ( ... ) {}

	try { tw2.setStartTime(epoch2->startDate()); }
	catch ( ... ) { tw2.setStartTime(Core::Time(1980,1,1)); }
	try { tw2.setEndTime(epoch2->endDate()); } catch ( ... ) {}

	return overlaps(tw1, tw2);
}


template <typename T>
bool overlaps2(const T *epoch1, const T *epoch2) {
	Core::TimeWindow tw1, tw2;
	try { tw1.setStartTime(epoch1->start()); }
	catch ( ... ) { tw1.setStartTime(Core::Time(1980,1,1)); }
	try { tw1.setEndTime(epoch1->end()); } catch ( ... ) {}

	try { tw2.setStartTime(epoch2->start()); }
	catch ( ... ) { tw2.setStartTime(Core::Time(1980,1,1)); }
	try { tw2.setEndTime(epoch2->end()); } catch ( ... ) {}

	return overlaps(tw1, tw2);
}


void checkFIR(DataModel::ResponseFIR *rf) {
	vector<double> &v = rf->coefficients().content();
	int nc = v.size();

	if ( rf->numberOfCoefficients() != nc ) {
		SEISCOMP_WARNING("expected %d coefficients, found %d: will be corrected",
		                 rf->numberOfCoefficients(), nc);
		rf->setNumberOfCoefficients(nc);
	}

	if ( nc == 0 || rf->symmetry() != "A" ) return;

	int i = 0;
	for ( ; 2 * i < nc; ++i )
		if ( v[i] != v[nc - 1 - i] ) break;

	if ( 2 * i > nc ) {
		rf->setNumberOfCoefficients(i);
		rf->setSymmetry("B");
		v.resize(i);
		//SEISCOMP_DEBUG("A(%d) -> B(%d)", nc, i);
	}
	else if ( 2 * i == nc ) {
		rf->setNumberOfCoefficients(i);
		rf->setSymmetry("C");
		v.resize(i);
		//SEISCOMP_DEBUG("A(%d) -> C(%d)", nc, i);
	}
	else {
		//SEISCOMP_DEBUG("A(%d) -> A(%d)", nc, nc);
	}
}


void checkIIR(DataModel::ResponseIIR *rf) {
	vector<double> &nums = rf->numerators().content();
	int nc = nums.size();

	if ( rf->numberOfNumerators() != nc ) {
		SEISCOMP_WARNING("expected %d numerators, found %d: will be corrected",
		                 rf->numberOfNumerators(), nc);
		rf->setNumberOfNumerators(nc);
	}

	vector<double> &denoms = rf->denominators().content();
	int dc = denoms.size();

	if ( rf->numberOfDenominators() != dc ) {
		SEISCOMP_WARNING("expected %d denominators, found %d: will be corrected",
		                 rf->numberOfDenominators(), dc);
		rf->setNumberOfDenominators(dc);
	}
}


void checkPAZ(DataModel::ResponsePAZ *rp) {
	if ( rp->numberOfPoles() != (int)rp->poles().content().size() ) {
		SEISCOMP_WARNING("expected %d poles, found %lu", rp->numberOfPoles(),
		                 (unsigned long)rp->poles().content().size());
		rp->setNumberOfPoles(rp->poles().content().size());
	}

	if ( rp->numberOfZeros() != (int)rp->zeros().content().size() ) {
		SEISCOMP_WARNING("expected %d zeros, found %lu", rp->numberOfZeros(),
		                 (unsigned long)rp->zeros().content().size());
		rp->setNumberOfZeros(rp->zeros().content().size());
	}
}


void checkFAP(DataModel::ResponseFAP *rp) {
	if ( rp->numberOfTuples() != (int)(rp->tuples().content().size()*3) ) {
		SEISCOMP_WARNING("expected %d tuples, found %lu", rp->numberOfTuples(),
		                 (unsigned long)(rp->tuples().content().size()/3));
		rp->setNumberOfTuples(rp->tuples().content().size()/3);
	}
}


void checkPoly(DataModel::ResponsePolynomial *rp) {
	if ( rp->numberOfCoefficients() != (int)rp->coefficients().content().size() ) {
		SEISCOMP_WARNING("expected %d coefficients, found %lu", rp->numberOfCoefficients(),
		                 (unsigned long)rp->coefficients().content().size());
		rp->setNumberOfCoefficients(rp->coefficients().content().size());
	}
}


// Macro to backup the result of an optional value
#define BCK(name, type, query) \
	OPT(type) name;\
	try { name = query; } catch ( ... ) {}

// Macro to update the need-update state using the backup'd value
// and the current value
#define UPD(target, bck, type, query) \
	if ( !target ) {\
		OPT(type) tmp; try { tmp = query; } catch ( ... ) {}\
		if ( tmp != bck ) target = true;\
	}

#define UPD2(target, bck, type, query) \
	if ( !target ) {\
		OPT(type) tmp; try { tmp = query; } catch ( ... ) {}\
		if ( tmp != bck ) { cout << #query" -> " << Core::toString(bck) << " != " << Core::toString(query) << endl; target = true;}\
	}


#define UPD_DBG(bck, type, query) \
	{ OPT(type) tmp; try { tmp = query; } catch ( ... ) {}\
	  if ( tmp != bck ) { SEISCOMP_DEBUG(#query" changed"); } }


#define COMPARE_AND_RETURN(type, inst1, inst2, query) \
	{\
		BCK(tmp1, type, inst1->query)\
		BCK(tmp2, type, inst2->query)\
		if ( tmp1 != tmp2 ) return false;\
	}


bool equal(const DataModel::ResponseFIR *f1, const DataModel::ResponseFIR *f2) {
	COMPARE_AND_RETURN(double, f1, f2, gain())
	COMPARE_AND_RETURN(int, f1, f2, decimationFactor())
	COMPARE_AND_RETURN(double, f1, f2, delay())
	COMPARE_AND_RETURN(double, f1, f2, correction())
	COMPARE_AND_RETURN(int, f1, f2, numberOfCoefficients())

	if ( f1->symmetry() != f2->symmetry() ) return false;

	const DataModel::RealArray *coeff1 = NULL;
	const DataModel::RealArray *coeff2 = NULL;

	try { coeff1 = &f1->coefficients(); } catch ( ... ) {}
	try { coeff2 = &f2->coefficients(); } catch ( ... ) {}

	// One set and not the other?
	if ( (!coeff1 && coeff2) || (coeff1 && !coeff2) ) return false;

	// Both unset?
	if ( !coeff1 && !coeff2 ) return true;

	// Both set, compare content
	const vector<double> &c1 = coeff1->content();
	const vector<double> &c2 = coeff2->content();
	if ( c1.size() != c2.size() ) return false;
	for ( size_t i = 0; i < c1.size(); ++i )
		if ( c1[i] != c2[i] ) return false;

	return true;
}


bool equal(const DataModel::ResponseIIR *f1, const DataModel::ResponseIIR *f2) {
	COMPARE_AND_RETURN(string, f1, f2, type())
	COMPARE_AND_RETURN(double, f1, f2, gain())
	COMPARE_AND_RETURN(int, f1, f2, decimationFactor())
	COMPARE_AND_RETURN(double, f1, f2, delay())
	COMPARE_AND_RETURN(double, f1, f2, correction())
	COMPARE_AND_RETURN(int, f1, f2, numberOfNumerators())
	COMPARE_AND_RETURN(DataModel::Blob, f1, f2, remark())

	const DataModel::RealArray *numerator1 = NULL;
	const DataModel::RealArray *numerator2 = NULL;

	try { numerator1 = &f1->numerators(); } catch ( ... ) {}
	try { numerator2 = &f2->numerators(); } catch ( ... ) {}

	// One set and not the other?
	if ( (!numerator1 && numerator2) || (numerator1 && !numerator2) ) return false;

	// Both unset?
	if ( !numerator1 && !numerator2 ) return true;

	// Both set, compare content
	const vector<double> &n1 = numerator1->content();
	const vector<double> &n2 = numerator2->content();
	if ( n1.size() != n2.size() ) return false;
	for ( size_t i = 0; i < n1.size(); ++i )
		if ( n1[i] != n2[i] ) return false;

	const DataModel::RealArray *denominators1 = NULL;
	const DataModel::RealArray *denominators2 = NULL;

	try { denominators1 = &f1->denominators(); } catch ( ... ) {}
	try { denominators2 = &f2->denominators(); } catch ( ... ) {}

	// One set and not the other?
	if ( (!denominators1 && denominators2) || (denominators1 && !denominators2) ) return false;

	// Both unset?
	if ( !denominators1 && !denominators2 ) return true;

	// Both set, compare content
	const vector<double> &d1 = denominators1->content();
	const vector<double> &d2 = denominators2->content();
	if ( d1.size() != d2.size() ) return false;
	for ( size_t i = 0; i < d1.size(); ++i )
		if ( d1[i] != d2[i] ) return false;

	return true;
}


bool equal(const DataModel::ResponsePAZ *p1, const DataModel::ResponsePAZ *p2) {
	if ( p1->type() != p2->type() ) return false;

	COMPARE_AND_RETURN(string, p1, p2, type())
	COMPARE_AND_RETURN(double, p1, p2, gain())
	COMPARE_AND_RETURN(double, p1, p2, gainFrequency())
	COMPARE_AND_RETURN(double, p1, p2, normalizationFactor())
	COMPARE_AND_RETURN(double, p1, p2, normalizationFrequency())
	COMPARE_AND_RETURN(int, p1, p2, numberOfPoles())
	COMPARE_AND_RETURN(int, p1, p2, numberOfZeros())
	COMPARE_AND_RETURN(int, p1, p2, decimationFactor())
	COMPARE_AND_RETURN(double, p1, p2, delay())
	COMPARE_AND_RETURN(double, p1, p2, correction())

	// Compare poles
	const DataModel::ComplexArray *poles1 = NULL;
	const DataModel::ComplexArray *poles2 = NULL;

	try { poles1 = &p1->poles(); } catch ( ... ) {}
	try { poles2 = &p2->poles(); } catch ( ... ) {}

	// One set and not the other?
	if ( (!poles1 && poles2) || (poles1 && !poles2) ) return false;

	// Both unset?
	if ( !poles1 && !poles2 ) return true;

	// Both set, compare content
	const vector< complex<double> > &pc1 = poles1->content();
	const vector< complex<double> > &pc2 = poles2->content();

	if ( pc1.size() != pc2.size() ) return false;
	for ( size_t i = 0; i < pc1.size(); ++i )
		if ( pc1[i] != pc2[i] ) return false;

	// Compare zeros
	const DataModel::ComplexArray *zeros1 = NULL;
	const DataModel::ComplexArray *zeros2 = NULL;

	try { zeros1 = &p1->zeros(); } catch ( ... ) {}
	try { zeros2 = &p2->zeros(); } catch ( ... ) {}

	// One set and not the other?
	if ( (!zeros1 && zeros2) || (zeros1 && !zeros2) ) return false;

	// Both unset?
	if ( !zeros1 && !zeros2 ) return true;

	// Both set, compare content
	const vector< complex<double> > &zc1 = zeros1->content();
	const vector< complex<double> > &zc2 = zeros2->content();

	if ( zc1.size() != zc2.size() ) return false;
	for ( size_t i = 0; i < zc1.size(); ++i )
		if ( zc1[i] != zc2[i] ) return false;

	// Everything is equal
	return true;
}


bool equal(const DataModel::ResponseFAP *p1, const DataModel::ResponseFAP *p2) {
	COMPARE_AND_RETURN(double, p1, p2, gain())
	COMPARE_AND_RETURN(double, p1, p2, gainFrequency())

	COMPARE_AND_RETURN(int, p1, p2, numberOfTuples())

	const DataModel::RealArray *tuples1 = NULL;
	const DataModel::RealArray *tuples2 = NULL;

	try { tuples1 = &p1->tuples(); } catch ( ... ) {}
	try { tuples2 = &p2->tuples(); } catch ( ... ) {}

	// One set and not the other?
	if ( (!tuples1 && tuples2) || (tuples1 && !tuples2) ) return false;

	// Both unset?
	if ( !tuples1 && !tuples2 ) return true;

	// Both set, compare content
	const vector<double> &t1 = tuples1->content();
	const vector<double> &t2 = tuples2->content();
	if ( t1.size() != t2.size() ) return false;
	for ( size_t i = 0; i < t1.size(); ++i )
		if ( t1[i] != t2[i] ) return false;

	return true;
}


bool equal(const DataModel::ResponsePolynomial *p1, const DataModel::ResponsePolynomial *p2) {
	COMPARE_AND_RETURN(double, p1, p2, gain())
	COMPARE_AND_RETURN(double, p1, p2, gainFrequency())

	if ( p1->frequencyUnit() != p2->frequencyUnit() ) return false;
	if ( p1->approximationType() != p2->approximationType() ) return false;

	COMPARE_AND_RETURN(double, p1, p2, approximationLowerBound())
	COMPARE_AND_RETURN(double, p1, p2, approximationUpperBound())
	COMPARE_AND_RETURN(double, p1, p2, approximationError())
	COMPARE_AND_RETURN(int, p1, p2, numberOfCoefficients())

	const DataModel::RealArray *coeff1 = NULL;
	const DataModel::RealArray *coeff2 = NULL;

	try { coeff1 = &p1->coefficients(); } catch ( ... ) {}
	try { coeff2 = &p2->coefficients(); } catch ( ... ) {}

	// One set and not the other?
	if ( (!coeff1 && coeff2) || (coeff1 && !coeff2) ) return false;

	// Both unset?
	if ( !coeff1 && !coeff2 ) return true;

	// Both set, compare content
	const vector<double> &c1 = coeff1->content();
	const vector<double> &c2 = coeff2->content();
	if ( c1.size() != c2.size() ) return false;
	for ( size_t i = 0; i < c1.size(); ++i )
		if ( c1[i] != c2[i] ) return false;

	return true;
}


bool equal(const DataModel::Datalogger *d1, const DataModel::Datalogger *d2) {
	if ( d1->description() != d2->description() ) return false;
	if ( d1->digitizerModel() != d2->digitizerModel() ) return false;
	if ( d1->digitizerManufacturer() != d2->digitizerManufacturer() ) return false;
	if ( d1->recorderModel() != d2->recorderModel() ) return false;
	if ( d1->recorderManufacturer() != d2->recorderManufacturer() ) return false;
	if ( d1->clockModel() != d2->clockModel() ) return false;
	if ( d1->clockManufacturer() != d2->clockManufacturer() ) return false;
	if ( d1->clockType() != d2->clockType() ) return false;

	COMPARE_AND_RETURN(double, d1, d2, gain())
	COMPARE_AND_RETURN(double, d1, d2, maxClockDrift())

	return true;
}


bool equal(const DataModel::Sensor *s1, const DataModel::Sensor *s2) {
	if ( s1->description() != s2->description() ) return false;
	if ( s1->model() != s2->model() ) return false;
	if ( s1->manufacturer() != s2->manufacturer() ) return false;
	if ( s1->type() != s2->type() ) return false;
	if ( s1->unit() != s2->unit() ) return false;
	if ( s1->response() != s2->response() ) return false;

	COMPARE_AND_RETURN(double, s1, s2, lowFrequency())
	COMPARE_AND_RETURN(double, s1, s2, highFrequency())

	return true;
}


bool isElectric(const string &unit) {
	return unit == AMPERE || unit == VOLTAGE;
}


bool isSensorStage(const string &inputUnit, const string &outputUnit) {
	return !isElectric(inputUnit) && isElectric(outputUnit);
}


bool isAnalogDataloggerStage(const string &inputUnit, const string &outputUnit) {
	return isElectric(inputUnit) && isElectric(outputUnit);
}


bool isDigitalDataloggerStage(const string &inputUnit, const string &outputUnit) {
	return !isElectric(outputUnit);
}


bool isADCStage(const string &inputUnit, const string &outputUnit) {
	return isElectric(inputUnit) && !isElectric(outputUnit);
}


bool IsDummy(const FDSNXML::ResponseStage *stage, ResponseType type) {
	switch ( type ) {
		case RT_FIR:
			if ( stage->fIR().numeratorCoefficientCount() == 0 )
				return true;
			break;

		case RT_RC:
			if ( stage->coefficients().numeratorCount() == 0 &&
			     stage->coefficients().denominatorCount() == 0 )
				return true;

			if ( stage->coefficients().numeratorCount() == 1 &&
			     stage->coefficients().denominatorCount() == 0 &&
			     stage->coefficients().numerator(0)->value() == 1.0 ) {
				try {
					if ( stage->coefficients().numerator(0)->lowerUncertainty() != 0 )
						return false;
				}
				catch ( ... ) {}

				try {
					if ( stage->coefficients().numerator(0)->upperUncertainty() != 0 )
						return false;
				}
				catch ( ... ) {}

				return true;
			}
			break;

		case RT_PAZ:
			if ( (stage->polesZeros().poleCount() == 0) &&
			     (stage->polesZeros().zeroCount() == 0) ) {
				return true;
			}
			break;

		default:
			break;
	}

	return false;
}


string getBaseUnit(const string &unitText) {
	size_t pos = unitText.find(' ');
	if ( pos == string::npos ) return unitText;
	return unitText.substr(0, pos);
}


const FDSNXML::BaseFilter *getFilter(const FDSNXML::ResponseStage *stage, ResponseType &type) {
	try {
		type = RT_PAZ;
		return &stage->polesZeros();
	}
	catch ( ... ) {
		try {
			type = RT_RC;
			return &stage->coefficients();
		}
		catch ( ... ) {
			try {
				type = RT_FAP;
				return &stage->responseList();
			}
			catch ( ... ) {
				try {
					type = RT_FIR;
					return &stage->fIR();
				}
				catch ( ... ) {
					try {
						type = RT_Poly;
						return &stage->polynomial();
					}
					catch ( ... ) {}
				}
			}
		}
	}

	type = RT_None;
	return NULL;
}


template <typename T>
T *create(const FDSNXML::BaseFilter *n) {
	T *o;
	if ( n->resourceId().empty() )
		o = T::Create();
	else if ( DataModel::PublicObject::Find(n->resourceId()) != NULL ) {
		o = T::Create();
		cerr << "W  ambiguous resourceId '" << n->resourceId() << "' for "
		     << T::ClassName() << endl;
		cerr << "   generated new resourceId '" << o->publicID() << "'" << endl;
	}
	else
		o = T::Create(n->resourceId());

	if ( n->name().empty() )
		o->setName(o->publicID());
	else
		o->setName(n->name());

	return o;
}


DataModel::ResponseFIRPtr convert(const FDSNXML::ResponseStage *resp,
                                  const FDSNXML::Coefficients *coeff) {
	if ( coeff->cfTransferFunctionType() != FDSNXML::CFTFT_DIGITAL ) {
		SEISCOMP_ERROR("only coefficient responses with transfer function "
		               "type \"DIGITAL\" supported");
		return NULL;
	}

	if ( coeff->denominatorCount() > 0 ) {
		if ( (coeff->denominatorCount() > 1) ||
		     (coeff->denominator(0)->value() != 1.0) ) {
			SEISCOMP_ERROR("coefficient responses with non-trivial "
			               "denominators are not supported");
			return NULL;
		}
	}

	DataModel::ResponseFIRPtr rf = create<DataModel::ResponseFIR>(coeff);

	try { rf->setGain(resp->stageGain().value()); } catch ( ... ) {}
	try { rf->setDecimationFactor(resp->decimation().factor()); }
	catch ( ... ) {}
	try { rf->setDelay(resp->decimation().delay().value()*resp->decimation().inputSampleRate().value()); }
	catch ( ... ) {}
	try { rf->setCorrection(resp->decimation().correction().value()*resp->decimation().inputSampleRate().value()); }
	catch ( ... ) {}

	rf->setNumberOfCoefficients(coeff->numeratorCount());
	rf->setSymmetry("A");
	rf->setCoefficients(DataModel::RealArray());
	vector<double> &numerators = rf->coefficients().content();
	for ( size_t n = 0; n < coeff->numeratorCount(); ++n ) {
		FDSNXML::FloatType *num = coeff->numerator(n);
		numerators.push_back(num->value());
	}

	return rf;
}


DataModel::ResponseIIRPtr convertIIR(const FDSNXML::ResponseStage *resp,
                                     const FDSNXML::Coefficients *coeff) {
	DataModel::ResponseIIRPtr rp = create<DataModel::ResponseIIR>(coeff);

	switch ( coeff->cfTransferFunctionType() ) {
		case FDSNXML::PZTFT_LAPLACE_RAD:
			rp->setType("A");
			break;
		case FDSNXML::PZTFT_LAPLACE_HZ:
			rp->setType("B");
			break;
		case FDSNXML::PZTFT_DIGITAL_Z_TRANSFORM:
			rp->setType("D");
			break;
		default:
			break;
	}

	try { rp->setGain(resp->stageGain().value()); } catch ( ... ) {}
	try { rp->setDecimationFactor(resp->decimation().factor()); }
	catch ( ... ) {}
	try { rp->setDelay(resp->decimation().delay().value()*resp->decimation().inputSampleRate().value()); }
	catch ( ... ) {}
	try { rp->setCorrection(resp->decimation().correction().value()*resp->decimation().inputSampleRate().value()); }
	catch ( ... ) {}

	rp->setNumberOfNumerators(coeff->numeratorCount());
	rp->setNumberOfDenominators(coeff->denominatorCount());

	rp->setNumerators(DataModel::RealArray());
	vector<double> &numerators = rp->numerators().content();

	for ( size_t n = 0; n < coeff->numeratorCount(); ++n ) {
		FDSNXML::FloatType *num = coeff->numerator(n);
		numerators.push_back(num->value());
	}

	rp->setDenominators(DataModel::RealArray());
	vector<double> &denominators = rp->denominators().content();

	for ( size_t n = 0; n < coeff->denominatorCount(); ++n ) {
		FDSNXML::FloatType *num = coeff->denominator(n);
		denominators.push_back(num->value());
	}

	return rp;
}


DataModel::ResponseFIRPtr convert(const FDSNXML::ResponseStage *resp,
                                  const FDSNXML::FIR *fir) {
	DataModel::ResponseFIRPtr rf = create<DataModel::ResponseFIR>(fir);

	try { rf->setGain(resp->stageGain().value()); } catch ( ... ) {}
	try { rf->setDecimationFactor(resp->decimation().factor()); }
	catch ( ... ) {}
	try { rf->setDelay(resp->decimation().delay().value()*resp->decimation().inputSampleRate().value()); }
	catch ( ... ) {}
	try { rf->setCorrection(resp->decimation().correction().value()*resp->decimation().inputSampleRate().value()); }
	catch ( ... ) {}

	rf->setNumberOfCoefficients(fir->numeratorCoefficientCount());

	try {
		switch ( fir->symmetry() ) {
			case FDSNXML::ST_ODD:
				// According to SEED manual chapter 5/50p, blockette 41
				rf->setSymmetry("B");
				break;
			case FDSNXML::ST_EVEN:
				rf->setSymmetry("C");
				break;
			default:
				rf->setSymmetry("A");
				break;
		}
	}
	catch ( ... ) {}

	rf->setCoefficients(DataModel::RealArray());
	vector<double> &numerators = rf->coefficients().content();

	try {
		// Sort coefficients according to its i attribute
		vector< pair<int,int> > sortedIdx;
		for ( size_t n = 0; n < fir->numeratorCoefficientCount(); ++n ) {
			FDSNXML::NumeratorCoefficient *num = fir->numeratorCoefficient(n);
			sortedIdx.push_back(pair<int,int>(num->i(), n));
		}
		sort(sortedIdx.begin(), sortedIdx.end());

		for ( size_t n = 0; n < fir->numeratorCoefficientCount(); ++n ) {
			FDSNXML::NumeratorCoefficient *num = fir->numeratorCoefficient(sortedIdx[n].second);
			numerators.push_back(num->value());
		}
	}
	catch ( ... ) {
		// Since NumeratorCoefficient.i is optional, just one unset attribute
		// will make sorting impossible, so use the order as given in the XML.
		for ( size_t n = 0; n < fir->numeratorCoefficientCount(); ++n ) {
			FDSNXML::NumeratorCoefficient *num = fir->numeratorCoefficient(n);
			numerators.push_back(num->value());
		}
	}

	return rf;
}



DataModel::ResponsePAZPtr convert(const FDSNXML::ResponseStage *resp,
                                  const FDSNXML::PolesAndZeros *paz) {
	DataModel::ResponsePAZPtr rp = create<DataModel::ResponsePAZ>(paz);

	switch ( paz->pzTransferFunctionType() ) {
		case FDSNXML::PZTFT_LAPLACE_RAD:
			rp->setType("A");
			break;
		case FDSNXML::PZTFT_LAPLACE_HZ:
			rp->setType("B");
			break;
		case FDSNXML::PZTFT_DIGITAL_Z_TRANSFORM:
			rp->setType("D");
			break;
		default:
			break;
	}

	try { rp->setGain(resp->stageGain().value()); } catch ( ... ) {}
	try { rp->setGainFrequency(resp->stageGain().frequency()); }
	catch ( ... ) {}

	rp->setNormalizationFactor(paz->normalizationFactor());
	rp->setNormalizationFrequency(paz->normalizationFrequency().value());
	rp->setNumberOfZeros(paz->zeroCount());
	rp->setNumberOfPoles(paz->poleCount());

	rp->setZeros(DataModel::ComplexArray());
	vector< complex<double> > &zeros = rp->zeros().content();

	// Sort zeros according to its number attribute
	vector< pair<int,int> > sortedIdx;
	for ( size_t z = 0; z < paz->zeroCount(); ++z ) {
		FDSNXML::PoleAndZero *val = paz->zero(z);
		sortedIdx.push_back(pair<int,int>(val->number(), z));
	}
	sort(sortedIdx.begin(), sortedIdx.end());

	for ( size_t z = 0; z < sortedIdx.size(); ++z ) {
		FDSNXML::PoleAndZero *val = paz->zero(sortedIdx[z].second);
		zeros.push_back(complex<double>(val->real().value(),val->imaginary().value()));
	}

	rp->setPoles(DataModel::ComplexArray());
	vector< complex<double> > &poles = rp->poles().content();

	// Sort poles according to its number attribute
	sortedIdx.clear();
	for ( size_t p = 0; p < paz->poleCount(); ++p ) {
		FDSNXML::PoleAndZero *val = paz->pole(p);
		sortedIdx.push_back(pair<int,int>(val->number(), p));
	}
	sort(sortedIdx.begin(), sortedIdx.end());

	for ( size_t p = 0; p < sortedIdx.size(); ++p ) {
		FDSNXML::PoleAndZero *val = paz->pole(sortedIdx[p].second);
		poles.push_back(complex<double>(val->real().value(),val->imaginary().value()));
	}

	return rp;
}


bool orderByFreq(const FDSNXML::ResponseListElement *e1,
                 const FDSNXML::ResponseListElement *e2) {
	return e1->frequency().value() < e2->frequency().value();
}


DataModel::ResponseFAPPtr convert(const FDSNXML::ResponseStage *resp,
                                  const FDSNXML::ResponseList *rl) {
	DataModel::ResponseFAPPtr rp = create<DataModel::ResponseFAP>(rl);

	try { rp->setGain(resp->stageGain().value()); } catch ( ... ) {}
	try { rp->setGainFrequency(resp->stageGain().frequency()); }
	catch ( ... ) {}

	vector<FDSNXML::ResponseListElement*> sortedFAP;
	for ( size_t i = 0; i < rl->elementCount(); ++i )
		sortedFAP.push_back(rl->element(i));

	sort(sortedFAP.begin(), sortedFAP.end(), orderByFreq);

	rp->setTuples(DataModel::RealArray());
	vector<double> &tuples = rp->tuples().content();

	for ( size_t i = 0; i < sortedFAP.size(); ++i ) {
		FDSNXML::ResponseListElement *elem = rl->element(i);
		tuples.push_back(elem->frequency().value());
		tuples.push_back(elem->amplitude().value());
		tuples.push_back(elem->phase().value());
	}

	return rp;
}


DataModel::ResponsePolynomialPtr convert(const FDSNXML::ResponseStage *resp,
                                         const FDSNXML::Polynomial *poly) {
	DataModel::ResponsePolynomialPtr rp = create<DataModel::ResponsePolynomial>(poly);

	try { rp->setGain(resp->stageGain().value()); } catch ( ... ) {}
	try { rp->setGainFrequency(resp->stageGain().frequency()); }
	catch ( ... ) {}

	// Frequency unit is fixed at Hz.
	// https://github.com/FDSN/StationXML/blob/v1.0/fdsn-station.xsd#L790
	rp->setFrequencyUnit("B");
	const char *atstr = poly->approximationType().toString();
	if ( atstr != NULL )
		rp->setApproximationType(atstr);
	rp->setApproximationLowerBound(poly->approximationLowerBound());
	rp->setApproximationUpperBound(poly->approximationUpperBound());

	rp->setApproximationError(poly->maximumError());

	rp->setNumberOfCoefficients(poly->coefficientCount());

	rp->setCoefficients(DataModel::RealArray());
	vector<double> &coeff = rp->coefficients().content();

	// Sort zeros according to its number attribute
	vector< pair<int,int> > sortedIdx;
	for ( size_t c = 0; c < poly->coefficientCount(); ++c ) {
		FDSNXML::PolynomialCoefficient *val = poly->coefficient(c);
		sortedIdx.push_back(pair<int,int>(val->number(), c));
	}
	sort(sortedIdx.begin(), sortedIdx.end());

	for ( size_t c = 0; c < sortedIdx.size(); ++c ) {
		FDSNXML::PolynomialCoefficient *val = poly->coefficient(sortedIdx[c].second);
		coeff.push_back(val->value());
	}

	return rp;
}


DataModel::Network *
createNetwork(const string &code)
{
	string id = "NET/" + code + "/" +
	            Core::Time::GMT().toString("%Y%m%d%H%M%S.%f") + "." +
	            Core::toString(Core::BaseObject::ObjectCount());
	return DataModel::Network::Create(id);
}


DataModel::Station *
createStation(const string &net, const string &code)
{
	string id = "STA/" + net + "/" + code + "/" +
	            Core::Time::GMT().toString("%Y%m%d%H%M%S.%f") + "." +
	            Core::toString(Core::BaseObject::ObjectCount());
	return DataModel::Station::Create(id);
}


DataModel::SensorLocation *
createSensorLocation(const string &net, const string &sta, const string &code)
{
	string id = "LOC/" + net + "/" + sta + "/" + code + "/" +
	            Core::Time::GMT().toString("%Y%m%d%H%M%S.%f") + "." +
	            Core::toString(Core::BaseObject::ObjectCount());
	return DataModel::SensorLocation::Create(id);
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Convert2SC3::Convert2SC3(DataModel::Inventory *inv) : _inv(inv) {
	if ( !_inv ) return;

	for ( size_t i = 0; i < _inv->dataloggerCount(); ++i ) {
		DataModel::Datalogger *d = _inv->datalogger(i);
		_dataloggerLookup[d->name()] = d;
	}

	for ( size_t i = 0; i < _inv->sensorCount(); ++i ) {
		DataModel::Sensor *s = _inv->sensor(i);
		_sensorLookup[s->name()] = s;
	}

	for ( size_t i = 0; i < _inv->responsePAZCount(); ++i ) {
		DataModel::ResponsePAZ *r = _inv->responsePAZ(i);
		_respPAZLookup[r->name()] = r;
	}

	for ( size_t i = 0; i < _inv->responseFAPCount(); ++i ) {
		DataModel::ResponseFAP *r = _inv->responseFAP(i);
		_respFAPLookup[r->name()] = r;
	}

	for ( size_t i = 0; i < _inv->responsePolynomialCount(); ++i ) {
		DataModel::ResponsePolynomial *r = _inv->responsePolynomial(i);
		_respPolyLookup[r->name()] = r;
	}

	for ( size_t i = 0; i < _inv->responseFIRCount(); ++i ) {
		DataModel::ResponseFIR *r = _inv->responseFIR(i);
		_respFIRLookup[r->name()] = r;
	}

	for ( size_t i = 0; i < _inv->responseIIRCount(); ++i ) {
		DataModel::ResponseIIR *r = _inv->responseIIR(i);
		_respIIRLookup[r->name()] = r;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void add(DataModel::Inventory *inv,
         std::map<std::string, const DataModel::Object*> &lookup, T *o) {
	std::map<std::string, const DataModel::Object*>::iterator it = lookup.find(o->name());
	if ( it != lookup.end() ) {
		cerr << "C  name '" << o->name() << "' of " << o->className()
		     << " is not unique" << endl;
		cerr << "   set publicID as name" << endl;
		o->setName(o->publicID());
	}

	inv->add(o);
	lookup[o->name()] = o;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
void Convert2SC3::addRespToInv<DataModel::ResponsePAZ>(DataModel::ResponsePAZ *o) {
	add(_inv, _respPAZLookup, o);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
void Convert2SC3::addRespToInv<DataModel::ResponseFAP>(DataModel::ResponseFAP *o) {
	add(_inv, _respFAPLookup, o);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
void Convert2SC3::addRespToInv<DataModel::ResponsePolynomial>(DataModel::ResponsePolynomial *o) {
	add(_inv, _respPolyLookup, o);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
void Convert2SC3::addRespToInv<DataModel::ResponseFIR>(DataModel::ResponseFIR *o) {
	add(_inv, _respFIRLookup, o);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <>
void Convert2SC3::addRespToInv<DataModel::ResponseIIR>(DataModel::ResponseIIR *o) {
	add(_inv, _respIIRLookup, o);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Convert2SC3::TupleSet &Convert2SC3::visitedStations() const {
	return _visitedStations;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Convert2SC3::NetworkSet &Convert2SC3::touchedNetworks() const {
	return _touchedNetworks;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Convert2SC3::StationSet &Convert2SC3::touchedStations() const {
	return _touchedStations;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Convert2SC3::push(const FDSNXML::FDSNStationXML *msg) {
	if ( _inv == NULL ) return false;

	// Process networks. All networks are then under our control and
	// unprocessed epochs will be removed later
	for ( size_t n = 0; n < msg->networkCount(); ++n ) {
		if ( _interrupted ) return false;
		FDSNXML::Network *net = msg->network(n);

		string netCode = net->code();
		Core::trim(netCode);

		if ( netCode.empty() ) {
			SEISCOMP_WARNING("network[%d]: code empty: ignoring", (int)n);
			continue;
		}

		Core::Time start;
		try {
			start = net->startDate();
		}
		catch ( ... ) {
			start = Core::Time(1980,1,1);
		}

		// Mark network as seen
		_visitedStations.insert(StringTuple(netCode, ""));

		SEISCOMP_INFO("Processing network %s (%s)",
		              netCode.c_str(), start.toString("%F %T").c_str());

		bool newInstance = false;
		bool needUpdate = false;

		DataModel::NetworkPtr sc_net;
		sc_net = _inv->network(DataModel::NetworkIndex(netCode, start));

		if ( !sc_net ) {
			sc_net = createNetwork(netCode);
			sc_net->setCode(netCode);
			sc_net->setStart(start);
			newInstance = true;
		}

		BCK(oldRestricted, bool, sc_net->restricted());
		BCK(oldShared, bool, sc_net->shared());
		BCK(oldEnd, Core::Time, sc_net->end());
		string oldDescription = sc_net->description();

		try {
			sc_net->setRestricted(net->restrictedStatus() != FDSNXML::RST_OPEN);
		}
		catch ( ... ) {
			sc_net->setRestricted(Core::None);
		}

		sc_net->setShared(true);

		try { sc_net->setEnd(net->endDate()); }
		catch ( ... ) { sc_net->setEnd(Core::None); }

		sc_net->setDescription(net->description());

		UPD(needUpdate, oldRestricted, bool, sc_net->restricted());
		UPD(needUpdate, oldShared, bool, sc_net->shared());
		UPD(needUpdate, oldEnd, Core::Time, sc_net->end());
		if ( oldDescription != sc_net->description() ) needUpdate = true;

		if ( newInstance ) {
			SEISCOMP_DEBUG("Added new network epoch: %s (%s)",
			               sc_net->code().c_str(), sc_net->start().iso().c_str());
			_inv->add(sc_net.get());
		}
		else if ( needUpdate ) {
			SEISCOMP_DEBUG("Updated network epoch: %s (%s)",
			               sc_net->code().c_str(), sc_net->start().iso().c_str());
			sc_net->update();
		}

		_touchedNetworks.insert(NetworkIndex(sc_net->code(), sc_net->start()));

		for ( size_t s = 0; s < net->stationCount(); ++s ) {
			if ( _interrupted ) break;
			FDSNXML::Station *sta = net->station(s);
			string staCode = sta->code();
			Core::trim(staCode);

			if ( staCode.empty() ) {
				SEISCOMP_WARNING("network[%d]/station[%d]: empty code: ignoring",
				                 (int)n, (int)s);
				continue;
			}

			// Mark station as seen
			_visitedStations.insert(StringTuple(netCode, staCode));

			process(sc_net.get(), sta);
		}
	}

	return !_interrupted;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Convert2SC3::cleanUp() {
	SEISCOMP_INFO("Clean up inventory");

	for ( size_t n = 0; n < _inv->networkCount(); ) {
		DataModel::Network *net = _inv->network(n);
		if ( _visitedStations.find(StringTuple(net->code(), "")) == _visitedStations.end() ) {
			SEISCOMP_DEBUG("Leaving unknown network %s untouched", net->code().c_str());
			++n;
			continue;
		}

		NetworkIndex net_idx(net->code(), net->start());
		if ( _touchedNetworks.find(net_idx) == _touchedNetworks.end() ) {
			SEISCOMP_INFO("Removing epoch %s (%s)", net->code().c_str(), net->start().iso().c_str());
			// TODO: Remove epoch
			_inv->removeNetwork(n);
			continue;
		}

		++n;

		for ( size_t s = 0; s < net->stationCount(); ) {
			DataModel::Station *sta = net->station(s);
			if ( _visitedStations.find(StringTuple(net->code(), sta->code())) == _visitedStations.end() ) {
				SEISCOMP_DEBUG("Leaving unknown station %s.%s untouched",
				               net->code().c_str(), sta->code().c_str());
				++s;
				continue;
			}

			StationIndex sta_idx(net_idx, EpochIndex(sta->code(), sta->start()));
			if ( _touchedStations.find(sta_idx) == _touchedStations.end() ) {
				SEISCOMP_INFO("Removing epoch %s.%s (%s)",
				              net->code().c_str(), sta->code().c_str(),
				              sta->start().iso().c_str());
				// TODO: Remove epoch
				net->removeStation(s);
				continue;
			}

			++s;

			for ( size_t l = 0; l < sta->sensorLocationCount(); ) {
				DataModel::SensorLocation *loc = sta->sensorLocation(l);
				LocationIndex loc_idx(sta_idx, EpochIndex(loc->code(), loc->start()));
				if ( _touchedSensorLocations.find(loc_idx) == _touchedSensorLocations.end() ) {
					SEISCOMP_INFO("Removing epoch %s.%s.%s (%s)",
					              net->code().c_str(), sta->code().c_str(),
					              loc->code().c_str(), loc->start().iso().c_str());
					sta->removeSensorLocation(l);
					continue;
				}

				++l;

				for ( size_t c = 0; c < loc->streamCount(); ) {
					DataModel::Stream *stream = loc->stream(c);
					StreamIndex cha_idx(loc_idx, EpochIndex(stream->code(), stream->start()));
					if ( _touchedStreams.find(cha_idx) == _touchedStreams.end() ) {
						SEISCOMP_INFO("Removing epoch %s.%s.%s.%s (%s)",
						              net->code().c_str(), sta->code().c_str(),
						              loc->code().c_str(), stream->code().c_str(),
						              stream->start().iso().c_str());
						loc->removeStream(c);
					}
					else
						++c;
				}

				for ( size_t c = 0; c < loc->auxStreamCount(); ++c ) {
					DataModel::AuxStream *aux = loc->auxStream(c);
					StreamIndex cha_idx(loc_idx, EpochIndex(aux->code(), aux->start()));
					if ( _touchedAuxStreams.find(cha_idx) == _touchedAuxStreams.end() ) {
						SEISCOMP_INFO("Removing epoch %s.%s.%s.%s (%s)",
						              net->code().c_str(), sta->code().c_str(),
						              loc->code().c_str(), aux->code().c_str(),
						              aux->start().iso().c_str());
						loc->removeAuxStream(c);
					}
					else
						++c;
				}
			}
		}
	}

	/*
	int unusedResponses = 0;

	// Collect all publicID's of used responses so far
	StringSet usedResponses;
	for ( size_t s = 0; s < _inv->sensorCount(); ++s ) {
		DataModel::Sensor *sensor = _inv->sensor(s);
		if ( sensor->response().empty() ) continue;
		usedResponses.insert(sensor->response());
	}

	for ( size_t d = 0; d < _inv->dataloggerCount(); ++d ) {
		if ( _interrupted ) return;

		DataModel::Datalogger *dl = _inv->datalogger(d);
		for ( size_t i = 0; i < dl->decimationCount(); ++i ) {
			DataModel::Decimation *deci = dl->decimation(i);
			try {
				const string &c = deci->analogueFilterChain().content();
				if ( !c.empty() ) {
					vector<string> ids;
					Core::fromString(ids, c);
					usedResponses.insert(ids.begin(), ids.end());
				}
			}
			catch ( ... ) {}

			try {
				const string &c = deci->digitalFilterChain().content();
				if ( !c.empty() ) {
					vector<string> ids;
					Core::fromString(ids, c);
					usedResponses.insert(ids.begin(), ids.end());
				}
			}
			catch ( ... ) {}
		}
	}

	if ( _interrupted ) return;

	// Go through all available responses and remove unused ones
	for ( size_t r = 0; r < _inv->responseFIRCount(); ) {
		DataModel::ResponseFIR *resp = _inv->responseFIR(r);
		// Response not used -> remove it
		if ( usedResponses.find(resp->publicID()) == usedResponses.end() ) {
			_inv->removeResponseFIR(r);
			++unusedResponses;
		}
		else
			++r;
	}

	for ( size_t r = 0; r < _inv->responsePAZCount(); ) {
		DataModel::ResponsePAZ *resp = _inv->responsePAZ(r);
		// Response not used -> remove it
		if ( usedResponses.find(resp->publicID()) == usedResponses.end() ) {
			_inv->removeResponsePAZ(r);
			++unusedResponses;
		}
		else
			++r;
	}

	for ( size_t r = 0; r < _inv->responsePolynomialCount(); ) {
		DataModel::ResponsePolynomial *resp = _inv->responsePolynomial(r);
		// Response not used -> remove it
		if ( usedResponses.find(resp->publicID()) == usedResponses.end() ) {
			_inv->removeResponsePolynomial(r);
			++unusedResponses;
		}
		else
			++r;
	}

	if ( unusedResponses )
		SEISCOMP_INFO("Removed %d unused responses", unusedResponses);
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
//
// Process a station epoch. If sc_net is NULL then the network has not
// yet been created because this epoch comes from a station without a
// network tag in the message.
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Convert2SC3::process(DataModel::Network *sc_net,
                          const FDSNXML::Station *sta) {
	typedef map<LocationElevation, EpochEntry> EpochLocationMap;
	typedef map<string, EpochLocationMap> EpochCodeMap;

	Core::Time start;
	try {
		start = sta->startDate();
	}
	catch ( ... ) {
		start = Core::Time(1980,1,1);
	}

	string staCode = sta->code();
	Core::trim(staCode);

	SEISCOMP_INFO("Processing station %s/%s (%s)",
		          sc_net->code().c_str(), staCode.c_str(),
		          start.toString("%F %T").c_str());

	bool newInstance = false;
	bool needUpdate = false;

	DataModel::StationPtr sc_sta;
	sc_sta = sc_net->station(DataModel::StationIndex(staCode, start));
	if ( !sc_sta ) {
		sc_sta = createStation(sc_net->code(), staCode);
		sc_sta->setCode(staCode);
		sc_sta->setStart(start);
		newInstance = true;
	}

	BCK(oldLat, double, sc_sta->latitude());
	BCK(oldLon, double, sc_sta->longitude());
	BCK(oldElev, double, sc_sta->elevation());
	BCK(oldEnd, Core::Time, sc_sta->end());

	try {
		if ( sta->endDate().valid() )
			sc_sta->setEnd(sta->endDate());
		else
			sc_sta->setEnd(Core::None);
	}
	catch ( ... ) {
		sc_sta->setEnd(Core::None);
	}

	string oldDesc = sc_sta->description();
	string oldCountry = sc_sta->country();
	BCK(oldRestricted, bool, sc_sta->restricted());
	BCK(oldShared, bool, sc_sta->shared());
	string oldPlace = sc_sta->place();
	string oldAffiliation = sc_sta->affiliation();
	string oldType = sc_sta->type();

	sc_sta->setLatitude(sta->latitude().value());
	sc_sta->setLongitude(sta->longitude().value());
	sc_sta->setElevation(sta->elevation().value());
	if ( !sta->description().empty() )
		sc_sta->setDescription(sta->description());
	else if ( !sta->site().description().empty() )
		sc_sta->setDescription(sta->site().description());
	else
		sc_sta->setDescription(sta->site().name());
	sc_sta->setCountry(sta->site().country());
	sc_sta->setType("");

	string place;
	if ( !sta->site().region().empty() )
		place += sta->site().region();
	if ( !sta->site().county().empty() ) {
		if ( !place.empty() ) place += " / ";
		place += sta->site().county();
	}
	if ( !sta->site().town().empty() ) {
		if ( !place.empty() ) place += " / ";
		place += sta->site().town();
	}

	sc_sta->setPlace(place);
	try { sc_sta->setRestricted(sta->restrictedStatus() != FDSNXML::RST_OPEN); }
	catch ( ... ) { sc_sta->setRestricted(Core::None); }
	sc_sta->setShared(true);

	UPD(needUpdate, oldLat, double, sc_sta->latitude());
	UPD(needUpdate, oldLon, double, sc_sta->longitude());
	UPD(needUpdate, oldElev, double, sc_sta->elevation());
	UPD(needUpdate, oldEnd, Core::Time, sc_sta->end());
	if ( oldDesc != sc_sta->description() ) needUpdate = true;
	if ( oldCountry != sc_sta->country() ) needUpdate = true;
	UPD(needUpdate, oldRestricted, bool, sc_sta->restricted());
	UPD(needUpdate, oldShared, bool, sc_sta->shared());
	if ( oldPlace != sc_sta->place() ) needUpdate = true;
	if ( oldAffiliation != sc_sta->affiliation() ) needUpdate = true;
	if ( oldType != sc_sta->type() ) needUpdate = true;

	if ( newInstance ) {
		sc_net->add(sc_sta.get());
		SEISCOMP_DEBUG("Added new station epoch: %s (%s)",
		               sc_sta->code().c_str(), sc_sta->start().iso().c_str());
	}
	else if ( needUpdate ) {
		sc_sta->update();
		SEISCOMP_DEBUG("Updated station epoch: %s (%s)",
		               sc_sta->code().c_str(), sc_sta->start().iso().c_str());
	}

	// Register station
	_touchedStations.insert(
		StationIndex(
			NetworkIndex(sc_net->code(), sc_net->start()),
			EpochIndex(sc_sta->code(), sc_sta->start())
		)
	);

	EpochCodeMap epochMap;

	for ( size_t c = 0; c < sta->channelCount(); ++c ) {
		FDSNXML::Channel *cha = sta->channel(c);
		string locationCode = cha->locationCode();
		Core::trim(locationCode);

		// Create index based on code and location to form
		// SensorLocation objects later
		LocationElevation loc_loc(Location(cha->latitude().value(),
		                                   cha->longitude().value()),
		                                   cha->elevation().value());

		EpochEntry &entry = epochMap[locationCode][loc_loc];
		entry.epochs.push_back(ChannelEpoch(cha->code(),cha));
	}

	// After collecting all channel epoch, check for overlapping
	// time windows of different locations. If they overlap
	// valid SensorLocation definitions cannot be formed.
	EpochCodeMap::iterator it;

	// Iterate over all location codes
	for ( it = epochMap.begin(); it != epochMap.end(); ++it ) {
		// Iterate over all locations (lat/lon/elev) of this sensor
		EpochLocationMap::iterator lit, lit2;
		Epochs::iterator eit, eit2;

		set<FDSNXML::Channel*> overlappingEpochs;

		// Check for location/channel epoch overlaps for each lat/lon/elev
		// group
		for ( lit = it->second.begin(); lit != it->second.end(); ++lit ) {
			EpochEntry &entry = lit->second;

			// Iterate over all channel epochs of a group
			for ( eit = entry.epochs.begin(); eit != entry.epochs.end(); ++eit ) {
				FDSNXML::Channel *chaepoch = eit->second;

				// Already checked?
				if ( overlappingEpochs.find(chaepoch) != overlappingEpochs.end() )
					continue;

				// Go through all other groups
				for ( lit2 = it->second.begin(); lit2 != it->second.end(); ++lit2 ) {
					EpochEntry &entry2 = lit2->second;

					if ( lit == lit2 ) continue;

					// Iterate over all channel epochs of a group
					for ( eit2 = entry2.epochs.begin(); eit2 != entry2.epochs.end(); ++eit2 ) {
						FDSNXML::Channel *chaepoch2 = eit2->second;

						if ( eit->first != eit2->first ) continue;

						if ( overlaps(chaepoch, chaepoch2) ) {
							overlappingEpochs.insert(chaepoch);
							overlappingEpochs.insert(chaepoch2);
						}
					}
				}
			}
		}

		if ( !overlappingEpochs.empty() ) {
			cerr << "C  " << sc_net->code() << "." << sta->code()
			     << " location '" << it->first << "' has overlapping epochs: skipping" << endl;

			for ( lit = it->second.begin(); lit != it->second.end(); ++lit ) {
				EpochEntry &entry = lit->second;
				bool firstHit = true;
				for ( eit = entry.epochs.begin(); eit != entry.epochs.end(); ++eit ) {
					FDSNXML::Channel *cha = eit->second;
					if ( overlappingEpochs.find(cha) == overlappingEpochs.end() )
						continue;

					if ( firstHit ) {
						cerr << "     " << Core::toString(lit->first.first.first) << "° "
						                << Core::toString(lit->first.first.second) << "° "
						                << Core::toString(lit->first.second) << "m" << endl;
						firstHit = false;
					}

					cerr << "       " << cha->code() << "  ";
					try {
						Core::Time start = cha->startDate();
						cerr << timeToStr(start);
					}
					catch ( ... ) {}
					try {
						Core::Time end = cha->endDate();
						cerr << " ~ " << timeToStr(end);
					}
					catch ( ... ) {}
					cerr << endl;
				}
			}

			// Skip this location code
			continue;
		}

		for ( lit = it->second.begin(); lit != it->second.end(); ++lit ) {
			EpochEntry &entry = lit->second;

			entry.epochs.sort(epochLowerThan);

			DataModel::SensorLocationPtr sc_loc;

			// Iterate sorted epochs
			Core::Time sensorLocationStart;
			OPT(Core::Time) sensorLocationEnd;
			bool newTw = true;
			for ( eit = entry.epochs.begin(); eit != entry.epochs.end(); ++eit) {
				FDSNXML::Channel *cha = eit->second;

				// A new time window should be started
				if ( newTw ) {
					try { sensorLocationStart = cha->startDate(); }
					catch ( ... ) { sensorLocationStart = Core::Time(1980,1,1); }

					try { sensorLocationEnd = cha->endDate(); }
					catch ( ... ) { sensorLocationEnd = Core::None; }
					newTw = false;
					sc_loc = NULL;
				}
				// Extend the existing time window or create a new one
				// if the start time of the epoch overlaps with the
				// current tw
				else if ( sensorLocationEnd ) {
					// Extend end time of sensor location if necessary
					try {
						if ( cha->endDate() > *sensorLocationEnd ) {
							sensorLocationEnd = cha->endDate();
							sc_loc->setEnd(*sensorLocationEnd);
						}
					}
					catch ( ... ) {
						sensorLocationEnd = Core::None;
						sc_loc->setEnd(Core::None);
					}
				}

				if ( !sc_loc ) {
					newInstance = false;
					needUpdate = false;

					sc_loc = sc_sta->sensorLocation(DataModel::SensorLocationIndex(it->first, sensorLocationStart));
					if ( !sc_loc ) {
						sc_loc = createSensorLocation(sc_net->code(), sc_sta->code(), it->first);
						sc_loc->setCode(it->first);
						sc_loc->setStart(sensorLocationStart);
						newInstance = true;
					}

					BCK(oldLat, double, sc_loc->latitude());
					BCK(oldLon, double, sc_loc->longitude());
					BCK(oldElev, double, sc_loc->elevation());
					BCK(oldEnd, Core::Time, sc_loc->end());

					sc_loc->setLatitude(lit->first.first.first);
					sc_loc->setLongitude(lit->first.first.second);
					sc_loc->setElevation(lit->first.second);
					sc_loc->setEnd(sensorLocationEnd);

					UPD(needUpdate, oldLat, double, sc_loc->latitude());
					UPD(needUpdate, oldLon, double, sc_loc->longitude());
					UPD(needUpdate, oldElev, double, sc_loc->elevation());

					// Possible conflicting geo locations?
					if ( !newInstance && needUpdate ) {
						cerr << "W  " << sc_net->code() << "." << sta->code()
						     << " location '" << it->first << "' starting " << sc_loc->start().toString("%Y-%m-%d %H:%M:%S") << " has conflicting coordinates: using the last read" << endl;
						if ( *oldLat != sc_loc->latitude() )
							cerr << "   lat " << *oldLat << " != " << sc_loc->latitude() << endl;
						if ( *oldLon != sc_loc->longitude() )
							cerr << "   lon " << *oldLon << " != " << sc_loc->longitude() << endl;
						if ( *oldElev != sc_loc->elevation() )
							cerr << "   elevation " << *oldElev << " != " << sc_loc->elevation() << endl;
					}
					else if ( newInstance ) {
						for ( size_t l = 0; l < sc_sta->sensorLocationCount(); ++l ) {
							DataModel::SensorLocation *ref_loc = sc_sta->sensorLocation(l);
							if ( ref_loc->latitude() != sc_loc->latitude() ||
							     ref_loc->longitude() != sc_loc->longitude() ||
							     ref_loc->elevation() != sc_loc->elevation() ) {
								if ( overlaps2(ref_loc, sc_loc.get()) ) {
									cerr << "W  " << sc_net->code() << "." << sta->code() << " " << cha->code()
									     << " location '" << it->first << "' starting " << sc_loc->start().toString("%Y-%m-%d %H:%M:%S") << " has conflicting coordinates with location epoch starting " << ref_loc->start().toString("%Y-%m-%d %H:%M:%S") << endl;
									if ( ref_loc->latitude() != sc_loc->latitude() )
										cerr << "   lat " << ref_loc->latitude() << " != " << sc_loc->latitude() << endl;
									if ( ref_loc->longitude() != sc_loc->longitude() )
										cerr << "   lon " << ref_loc->longitude() << " != " << sc_loc->longitude() << endl;
									if ( ref_loc->elevation() != sc_loc->elevation() )
										cerr << "   elevation " << ref_loc->elevation() << " != " << sc_loc->elevation() << endl;
									break;
								}
							}
						}
					}

					UPD(needUpdate, oldEnd, Core::Time, sc_loc->end());

					if ( newInstance ) {
						sc_sta->add(sc_loc.get());
						SEISCOMP_DEBUG("Added new sensor location epoch: %s (%s)",
						               sc_loc->code().c_str(), sc_loc->start().iso().c_str());
					}
					else if ( needUpdate ) {
						sc_loc->update();
						SEISCOMP_DEBUG("Updated sensor location epoch: %s (%s)",
						               sc_loc->code().c_str(), sc_loc->start().iso().c_str());
					}

					// Register sensor location
					_touchedSensorLocations.insert(
						LocationIndex(
							StationIndex(
								NetworkIndex(sc_net->code(), sc_net->start()),
								EpochIndex(sc_sta->code(), sc_sta->start())
							),
							EpochIndex(sc_loc->code(), sc_loc->start())
						)
					);
				}

				process(sc_loc.get(), cha);
			}
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Convert2SC3::process(DataModel::SensorLocation *sc_loc,
                          const FDSNXML::Channel *cha) {
	bool newInstance = false;
	bool needUpdate = false;

	Core::Time start;
	try { start = cha->startDate(); }
	catch ( ... ) { start = Core::Time(1980,1,1); }

	string chaCode = cha->code();
	Core::trim(chaCode);

	DataModel::StreamPtr sc_stream;
	sc_stream = sc_loc->stream(DataModel::StreamIndex(chaCode, start));
	if ( !sc_stream ) {
		sc_stream = new DataModel::Stream;
		sc_stream->setCode(chaCode);
		sc_stream->setStart(start);
		newInstance = true;
	}

#if LOG_STAGES
	cerr << "[" << sc_loc->code() << chaCode << "]" << endl;
	cerr << " + Start " << start.iso() << endl;
#endif

	BCK(oldEnd, Core::Time, sc_stream->end());
	BCK(oldDep, double, sc_stream->depth());
	string oldFormat = sc_stream->format();
	BCK(oldRestricted, bool, sc_stream->restricted());
	BCK(oldsrNum, int, sc_stream->sampleRateNumerator());
	BCK(oldsrDen, int, sc_stream->sampleRateDenominator());
	BCK(oldAzi, double, sc_stream->azimuth());
	BCK(oldDip, double, sc_stream->dip());
	BCK(oldDLcha, int, sc_stream->dataloggerChannel());
	BCK(oldSNcha, int, sc_stream->sensorChannel());
	BCK(oldGain, double, sc_stream->gain());
	BCK(oldGainFreq, double, sc_stream->gainFrequency());
	string oldGainUnit = sc_stream->gainUnit();
	string oldDL = sc_stream->datalogger();
	string oldDLSN = sc_stream->dataloggerSerialNumber();
	string oldSN = sc_stream->sensor();
	string oldSNSN = sc_stream->sensorSerialNumber();
	string oldFlags = sc_stream->flags();

	string flags;

	try {
		if ( cha->endDate().valid() )
			sc_stream->setEnd(cha->endDate());
		else
			sc_stream->setEnd(Core::None);
	}
	catch ( ... ) { sc_stream->setEnd(Core::None); }

	sc_stream->setDepth(cha->depth().value());
	sc_stream->setFormat(cha->storageFormat());

	for ( size_t i = 0; i < cha->typeCount(); ++i )
		flags += cha->type(i)->type().toString()[0];

	/* Should it default to "GC"? Currently no.
	if ( flags.empty() )
		flags = "GC";
	*/

	sc_stream->setFlags(flags);

	try { sc_stream->setRestricted(cha->restrictedStatus() != FDSNXML::RST_OPEN); }
	catch ( ... ) { sc_stream->setRestricted(Core::None); }

	// Set sample rate
	try {
		sc_stream->setSampleRateNumerator(cha->sampleRateRatio().numberSamples());
		sc_stream->setSampleRateDenominator(cha->sampleRateRatio().numberSeconds());
	}
	catch ( ... ) {
		try {
			Fraction rat = double2frac(cha->sampleRate().value());
			sc_stream->setSampleRateNumerator(rat.first);
			sc_stream->setSampleRateDenominator(rat.second);
		}
		catch ( ... ) {}
	}

	// Set orientation
	try { sc_stream->setAzimuth(cha->azimuth().value()); }
	catch ( ... ) {}
	try { sc_stream->setDip(cha->dip().value()); }
	catch ( ... ) {}

	// Set datalogger/sensor channel according to the component code
	if ( sc_stream->code().substr(2,1) == "N" || sc_stream->code().substr(2,1) == "1" ) {
		sc_stream->setDataloggerChannel(1);
		sc_stream->setSensorChannel(1);
	}
	else if ( sc_stream->code().substr(2,1) == "E" || sc_stream->code().substr(2,1) == "2" ) {
		sc_stream->setDataloggerChannel(2);
		sc_stream->setSensorChannel(2);
	}
	else {
		sc_stream->setDataloggerChannel(0);
		sc_stream->setSensorChannel(0);
	}

	// Store responses
	Stages stages;

	const FDSNXML::Response *resp0 = NULL;

	try {
		resp0 = &cha->response();
		for ( size_t i = 0; i < resp0->stageCount(); ++i )
			stages.push_back(resp0->stage(i));
	}
	catch ( ... ) {}

	stages.sort(respLowerThan);

	if ( resp0 != NULL ) {
		try {
			sc_stream->setGain(resp0->instrumentSensitivity().value());
#if LOG_STAGES
			cerr << " + Gain " << sc_stream->gain() << endl;
#endif
		}
		catch ( ... ) { sc_stream->setGain(Core::None); }
		try { sc_stream->setGainFrequency(resp0->instrumentSensitivity().frequency()); }
		catch ( ... ) { sc_stream->setGainFrequency(Core::None); }
		try {
			sc_stream->setGainUnit(resp0->instrumentSensitivity().inputUnits().name());
#if LOG_STAGES
			cerr << " + Unit " << sc_stream->gainUnit() << endl;
#endif
		}
		catch ( ... ) { sc_stream->setGainUnit(""); }
	}
	else {
		sc_stream->setGain(Core::None);
		SEISCOMP_WARNING("%s.%s.%s.%s: response stage 0 not found and "
		                 "instrument sensitivity not set: gain undefined",
		                 sc_loc->station()->network()->code().c_str(),
		                 sc_loc->station()->code().c_str(),
		                 sc_loc->code().c_str(), sc_stream->code().c_str());

		sc_stream->setGainFrequency(Core::None);
		SEISCOMP_WARNING("%s.%s.%s.%s: response stage 0 not found and "
		                 "instrument sensitivity freq not set: freq undefined",
		                 sc_loc->station()->network()->code().c_str(),
		                 sc_loc->station()->code().c_str(),
		                 sc_loc->code().c_str(), sc_stream->code().c_str());

		sc_stream->setGainUnit("");
		SEISCOMP_WARNING("%s.%s.%s.%s: response stage 0 not found and "
		                 "instrument sensitivity unit not set: unit undefined",
		                 sc_loc->station()->network()->code().c_str(),
		                 sc_loc->station()->code().c_str(),
		                 sc_loc->code().c_str(), sc_stream->code().c_str());
	}

	sc_stream->setDataloggerSerialNumber("");

	try {
		sc_stream->setDataloggerSerialNumber(cha->dataLogger().serialNumber());
	}
	catch ( ... ) {}

	try {
		sc_stream->setSensorSerialNumber(cha->sensor().serialNumber());
	}
	catch ( ... ) {}

	// Iterate over all response stages and create the datalogger
	Stages::iterator it;
	bool hasDigitizerGain = false;

	DataModel::SensorPtr sc_sens;
	DataModel::DataloggerPtr sc_dl;
	DataModel::DecimationPtr sc_deci;
	bool newDeciInstance = false;
	string oldAnalogueChain, analogueChain;
	string oldDigitalChain, digitalChain;

	string dataloggerName = sc_loc->station()->network()->code() + "." +
	                        sc_loc->station()->code() + "." + sc_loc->code() +
	                        sc_stream->code() + "." +
	                        date2str(sc_stream->start());
	sc_dl = updateDatalogger(dataloggerName, cha);
	if ( !sc_dl ) {
		SEISCOMP_ERROR("Something wrong happened when creating a datalogger");
		return false;
	}

	sc_stream->setDatalogger(sc_dl->publicID());

	int numerator, denominator;

	try {
		numerator = sc_stream->sampleRateNumerator();
		denominator = sc_stream->sampleRateDenominator();

		sc_deci = sc_dl->decimation(DataModel::DecimationIndex(numerator, denominator));
		if ( !sc_deci ) {
			sc_deci = new DataModel::Decimation();
			sc_deci->setSampleRateNumerator(numerator);
			sc_deci->setSampleRateDenominator(denominator);
			sc_dl->add(sc_deci.get());
			newDeciInstance = true;
		}
		else {
			try { oldAnalogueChain = sc_deci->analogueFilterChain().content(); }
			catch ( ... ) {}
			try { oldDigitalChain = sc_deci->digitalFilterChain().content(); }
			catch ( ... ) {}
			newDeciInstance = false;
			SEISCOMP_DEBUG("Reused datalogger decimation for stream %s",
			               sc_stream->code().c_str());
		}
	}
	catch ( ... ) {
		SEISCOMP_WARNING("%s: no sampling rate given, ignoring all decimation stages",
		                 chaCode.c_str());
	}

	double dataloggerGainScale = 1.0;

	for ( it = stages.begin(); it != stages.end(); ++it ) {
		const FDSNXML::ResponseStage *stage = *it;

		OPT(double) stageGain;
		try { stageGain = stage->stageGain().value(); }
		catch ( ... ) {}

		ResponseType stageType;
		const FDSNXML::BaseFilter *filter = getFilter(stage, stageType);
		if ( filter == NULL ) {
			if ( stageGain ) {
#if LOG_STAGES
				cerr << " + D#" << stage->number() << " " << stageType.toString() << " datalogger gain times "
				     << *stageGain << endl;
#endif
				dataloggerGainScale *= *stageGain;
				continue;
			}
			else {
				cerr << "Channel " << chaCode << ", stage " << stage->number() << " has no filter and no gain. This is currently not supported" << endl;
				return false;
			}
		}

		string inputUnit = getBaseUnit(filter->inputUnits().name());
		string outputUnit = getBaseUnit(filter->outputUnits().name());

		bool isAnalogue;
		if ( isSensorStage(inputUnit, outputUnit) ) {
			if ( sc_sens ) {
				SEISCOMP_ERROR("%s: found another sensor stage but only one is expected: bail out",
				               chaCode.c_str());
				return false;
			}

#if LOG_STAGES
			cerr << " + S#" << stage->number() << " " << stageType.toString() << " "
			     << inputUnit << " " << outputUnit << endl;
#endif
			if ( inputUnit != sc_stream->gainUnit() ) {
				SEISCOMP_WARNING("%s: sensor input unit does not match channel instrument unit: %s != %s",
				                 chaCode.c_str(), inputUnit.c_str(), sc_stream->gainUnit().c_str());
			}

			// This is out sensor stage
			// Update sensor information
			string sensorName = sc_loc->station()->network()->code() + "." +
			                    sc_loc->station()->code() + "." + sc_loc->code() +
			                    sc_stream->code() + "." +
			                    date2str(sc_stream->start());

			sc_sens = updateSensor(sensorName, cha, stage, stageType, filter);
			if ( sc_sens ) {
				sc_stream->setSensor(sc_sens->publicID());
				process(sc_sens.get(), sc_stream.get(), cha, stage);
			}
			else {
				SEISCOMP_ERROR("Something wrong happened when creating a sensor, unsupported filter type %s?",
				               stageType.toString());
				return false;
			}

			continue;
		}
		else if ( isAnalogDataloggerStage(inputUnit, outputUnit) )
			isAnalogue = true;
		else if ( isDigitalDataloggerStage(inputUnit, outputUnit) )
			isAnalogue = false;
		else {
			SEISCOMP_ERROR("%s: stage %d is neither an analogue nor a digital stage, don't know what to do",
			               chaCode.c_str(), stage->number());
			return false;
		}

#if LOG_STAGES
		cerr << " + D#" << stage->number() << " " << stageType.toString() << " "
		     << inputUnit << " " << outputUnit << " ";
		if ( stageGain )
			cerr << *stageGain;
		else
			cerr << "-";
#endif

		if ( IsDummy(stage, stageType) ) {
			bool ignoreStage = false;

			// Ignore dummy stages without a defining gain
			if ( stageGain == 1.0 ) {
#if LOG_STAGES
				cerr << " (dummy)";
				ignoreStage = true;
#endif
			}

			// Potential preamplifier gain
			if ( !hasDigitizerGain && isADCStage(inputUnit, outputUnit) ) {
				hasDigitizerGain = true;
				sc_dl->setGain(stageGain);
#if LOG_STAGES
				cerr << " (digitizer gain)";
				ignoreStage = true;
#endif
			}

			if ( ignoreStage ) {
#if LOG_STAGES
				cerr << endl;
#endif
				continue;
			}
		}
		else {
			// Potential preamplifier gain
			if ( !hasDigitizerGain && isADCStage(inputUnit, outputUnit) ) {
				hasDigitizerGain = true;
				sc_dl->setGain(stageGain);
#if LOG_STAGES
				cerr << " (digitizer gain. forward filter with gain 1)";
#endif
				stageGain = 1.0;
			}
		}

#if LOG_STAGES
		cerr << endl;
#endif

		DataModel::PublicObject *abstractResponse = NULL;

		switch ( stageType ) {
			case RT_FIR:
			{
				bool newFIR = true;

				DataModel::ResponseFIRPtr rf;
				const FDSNXML::FIR *fir = &stage->fIR();
				rf = convert(stage, fir);

				if ( !rf ) {
					SEISCOMP_ERROR("%s: stage %d contains an unsupported filter configuration",
					               chaCode.c_str(), stage->number());
					return false;
				}

				checkFIR(rf.get());

				for ( size_t f = 0; f < _inv->responseFIRCount(); ++f ) {
					DataModel::ResponseFIR *fir = _inv->responseFIR(f);
					if ( equal(fir, rf.get()) ) {
						rf = fir;
						newFIR = false;
						break;
					}
				}

				if ( newFIR ) {
					addRespToInv(rf.get());
					//SEISCOMP_DEBUG("Added new FIR filter from coefficients: %s", rf->publicID().c_str());
				}
				else {
					//SEISCOMP_DEBUG("Reuse FIR filter from coefficients: %s", rf->publicID().c_str());
				}

				abstractResponse = rf.get();
				break;
			}
			case RT_RC:
			{
				const FDSNXML::Coefficients *coeff = &stage->coefficients();

				if ( (coeff->cfTransferFunctionType() != FDSNXML::CFTFT_DIGITAL) ||
				     ((coeff->denominatorCount() > 0) &&
				      (coeff->denominatorCount() > 1 || coeff->denominator(0)->value() != 1.0)) ) {
					bool newIIR = true;
					DataModel::ResponseIIRPtr iir;
					iir = convertIIR(*it, coeff);

					if ( !iir ) {
						SEISCOMP_ERROR("%s: stage %d contains an unconvertible IIR coefficient filter configuration",
						               chaCode.c_str(), stage->number());
						return false;
					}

					checkIIR(iir.get());

					for ( size_t f = 0; f < _inv->responseIIRCount(); ++f ) {
						DataModel::ResponseIIR *iir_ = _inv->responseIIR(f);
						if ( equal(iir_, iir.get()) ) {
							iir = iir_;
							newIIR = false;
							break;
						}
					}

					if ( newIIR ) {
						addRespToInv(iir.get());
						//SEISCOMP_DEBUG("Added new PAZ response from paz: %s", rp->publicID().c_str());
					}
					else {
						//SEISCOMP_DEBUG("Reused PAZ response from paz: %s", rp->publicID().c_str());
					}

					abstractResponse = iir.get();
				}
				else {
					bool newFIR = true;
					DataModel::ResponseFIRPtr rf;
					rf = convert(*it, coeff);

					if ( !rf ) {
						SEISCOMP_ERROR("%s: stage %d contains an unconvertible FIR coefficient filter configuration",
						               chaCode.c_str(), stage->number());
						return false;
					}

					checkFIR(rf.get());

					for ( size_t f = 0; f < _inv->responseFIRCount(); ++f ) {
						DataModel::ResponseFIR *fir = _inv->responseFIR(f);
						if ( equal(fir, rf.get()) ) {
							rf = fir;
							newFIR = false;
							break;
						}
					}

					if ( newFIR ) {
						addRespToInv(rf.get());
						//SEISCOMP_DEBUG("Added new FIR filter from coefficients: %s", rf->publicID().c_str());
					}
					else {
						//SEISCOMP_DEBUG("Reuse FIR filter from coefficients: %s", rf->publicID().c_str());
					}

					abstractResponse = rf.get();
				}

				break;
			}
			case RT_PAZ:
			{
				const FDSNXML::PolesAndZeros *paz = &stage->polesZeros();
				bool newPAZ = true;

				// Create PAZ ...
				DataModel::ResponsePAZPtr rp = convert(stage, paz);
				checkPAZ(rp.get());

				for ( size_t f = 0; f < _inv->responsePAZCount(); ++f ) {
					DataModel::ResponsePAZ *paz = _inv->responsePAZ(f);
					if ( equal(paz, rp.get()) ) {
						rp = paz;
						newPAZ = false;
						break;
					}
				}

				if ( newPAZ ) {
					addRespToInv(rp.get());
					//SEISCOMP_DEBUG("Added new PAZ response from paz: %s", rp->publicID().c_str());
				}
				else {
					//SEISCOMP_DEBUG("Reused PAZ response from paz: %s", rp->publicID().c_str());
				}

				abstractResponse = rp.get();
				break;
			}
			case RT_Poly:
			{
				const FDSNXML::Polynomial *poly = &stage->polynomial();
				bool newPoly = true;

				DataModel::ResponsePolynomialPtr rp = convert(stage, poly);
				checkPoly(rp.get());

				for ( size_t f = 0; f < _inv->responsePolynomialCount(); ++f ) {
					DataModel::ResponsePolynomial *poly = _inv->responsePolynomial(f);
					if ( equal(poly, rp.get()) ) {
						rp = poly;
						newPoly = false;
						break;
					}
				}

				if ( newPoly ) {
					addRespToInv(rp.get());
					//SEISCOMP_DEBUG("Added new polynomial response from poly: %s", rp->publicID().c_str());
				}
				else {
					//SEISCOMP_DEBUG("reused polynomial response from poly: %s", rp->publicID().c_str());
				}

				abstractResponse = rp.get();
				break;
			}
			case RT_FAP:
			{
				const FDSNXML::ResponseList *rl = &stage->responseList();
				bool newFAP = true;

				// Create FAP ...
				DataModel::ResponseFAPPtr rp = convert(stage, rl);
				checkFAP(rp.get());

				for ( size_t f = 0; f < _inv->responseFAPCount(); ++f ) {
					DataModel::ResponseFAP *fap = _inv->responseFAP(f);
					if ( equal(fap, rp.get()) ) {
						rp = fap;
						newFAP = false;
						break;
					}
				}

				if ( newFAP ) {
					addRespToInv(rp.get());
					//SEISCOMP_DEBUG("Added new PAZ response from paz: %s", rp->publicID().c_str());
				}
				else {
					//SEISCOMP_DEBUG("Reused PAZ response from paz: %s", rp->publicID().c_str());
				}

				abstractResponse = rp.get();
				break;
			}
			default:
				SEISCOMP_ERROR("Invalid response type");
				continue;
		}

		if ( isAnalogue ) {
			if ( !analogueChain.empty() ) analogueChain += " ";
			analogueChain += abstractResponse->publicID();
		}
		else {
			if ( !digitalChain.empty() ) digitalChain += " ";
			digitalChain += abstractResponse->publicID();
		}
	}

	if ( sc_deci ) {
		bool needUpdate = false;

		if ( analogueChain != oldAnalogueChain ) {
			DataModel::Blob blob;
			blob.setContent(analogueChain);
			sc_deci->setAnalogueFilterChain(blob);
			needUpdate = true;
		}

		if ( digitalChain != oldDigitalChain ) {
			DataModel::Blob blob;
			blob.setContent(digitalChain);
			sc_deci->setDigitalFilterChain(blob);
			needUpdate = true;
		}

		if ( !newDeciInstance && needUpdate )
			sc_deci->update();
	}

	// Set a default serial number if there is none
	if ( sc_stream->dataloggerSerialNumber().empty() ) {
		/*
		sc_stream->setDataloggerSerialNumber(
			sc_loc->station()->code() + "." + sc_loc->code() +
			sc_stream->code() + "." +
			date2str(sc_stream->start())
		);
		*/
		sc_stream->setDataloggerSerialNumber("xxxx");
	}

	// Set a default serial number if there is none
	if ( sc_stream->sensorSerialNumber().empty() )
		sc_stream->setSensorSerialNumber("yyyy");

	if ( dataloggerGainScale != 1.0 ) {
#if LOG_STAGES
		cerr << "+ Scale datalogger gain by " << dataloggerGainScale << endl;
#endif
		sc_dl->setGain(sc_dl->gain()*dataloggerGainScale);
	}

	// All stages have been converted, finalize configuration
	if ( sc_dl )
		process(sc_dl.get(), sc_stream.get(), cha);

	UPD(needUpdate, oldEnd, Core::Time, sc_stream->end());
	UPD(needUpdate, oldDep, double, sc_stream->depth());
	if ( oldFlags != sc_stream->flags() ) needUpdate = true;
	if ( oldFormat != sc_stream->format() ) needUpdate = true;
	UPD(needUpdate, oldRestricted, bool, sc_stream->restricted());
	UPD(needUpdate, oldsrNum, int, sc_stream->sampleRateNumerator());
	UPD(needUpdate, oldsrDen, int, sc_stream->sampleRateDenominator());
	UPD(needUpdate, oldAzi, double, sc_stream->azimuth());
	UPD(needUpdate, oldDip, double, sc_stream->dip());
	UPD(needUpdate, oldDLcha, int, sc_stream->dataloggerChannel());
	UPD(needUpdate, oldSNcha, int, sc_stream->sensorChannel());
	UPD(needUpdate, oldGain, double, sc_stream->gain());
	UPD(needUpdate, oldGainFreq, double, sc_stream->gainFrequency());
	if ( oldGainUnit != sc_stream->gainUnit() ) needUpdate = true;
	if ( oldDL != sc_stream->datalogger() ) needUpdate = true;
	if ( oldDLSN != sc_stream->dataloggerSerialNumber() ) needUpdate = true;
	if ( oldSN != sc_stream->sensor() ) needUpdate = true;
	if ( oldSNSN != sc_stream->sensorSerialNumber() ) needUpdate = true;

	if ( newInstance ) {
		sc_loc->add(sc_stream.get());
		SEISCOMP_DEBUG("Added new stream epoch: %s (%s)",
		               sc_stream->code().c_str(), sc_stream->start().iso().c_str());
	}
	else if ( needUpdate ) {
		sc_stream->update();
		SEISCOMP_DEBUG("Updated stream epoch: %s (%s)",
		               sc_stream->code().c_str(), sc_stream->start().iso().c_str());
	}

	// Register aux stream
	_touchedStreams.insert(
		StreamIndex(
			LocationIndex(
				StationIndex(
					NetworkIndex(sc_loc->station()->network()->code(), sc_loc->station()->network()->start()),
					EpochIndex(sc_loc->station()->code(), sc_loc->station()->start())
				),
				EpochIndex(sc_loc->code(), sc_loc->start())
			),
			EpochIndex(sc_stream->code(), sc_stream->start())
		)
	);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Convert2SC3::process(DataModel::Datalogger *sc_dl, DataModel::Stream *sc_stream,
                          const FDSNXML::Channel *epoch) {
	//updateDataloggerCalibration(sc_dl, sc_stream, epoch);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Convert2SC3::process(DataModel::Sensor *sc_sens, DataModel::Stream *sc_stream,
                          const FDSNXML::Channel *epoch,
                          const FDSNXML::ResponseStage *resp) {
	/*
	 * Actually a sensor calibration should not be created automatically. That
	 * be used from the historic values of the sensitivity blockette. In FDSN
	 * StationXML this information is not available anymore.
	*/
	//updateSensorCalibration(sc_sens, sc_stream, epoch, resp);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Datalogger *
Convert2SC3::updateDatalogger(const std::string &name,
                              const FDSNXML::Channel *epoch) {
	DataModel::DataloggerPtr sc_dl = DataModel::Datalogger::Create();
	//sc_dl->setName(sc_dl->publicID());
	sc_dl->setName(name);

	try { sc_dl->setDescription(epoch->dataLogger().description()); }
	catch ( ... ) { sc_dl->setDescription(""); }

	try { sc_dl->setDigitizerModel(epoch->dataLogger().model()); }
	catch ( ... ) { sc_dl->setDigitizerModel(""); }

	try { sc_dl->setDigitizerManufacturer(epoch->dataLogger().manufacturer()); }
	catch ( ... ) { sc_dl->setDigitizerManufacturer(""); }

	sc_dl->setGain(1.0);

	try {
		// Convert fdsnxml clockdrift (seconds/sample) to seconds/second
		double drift = epoch->clockDrift().value() * epoch->sampleRateRatio().numberSamples() / epoch->sampleRateRatio().numberSeconds();
		sc_dl->setMaxClockDrift(drift);
	}
	catch ( ... ) {
		try {
			double drift = epoch->clockDrift().value() * epoch->sampleRate().value();
			sc_dl->setMaxClockDrift(drift);
		}
		catch ( ... ) {
			sc_dl->setMaxClockDrift(Core::None);
		}
	}

	/*
	bool newInstance = true;
	for ( size_t d = 0; d < _inv->dataloggerCount(); ++d ) {
		DataModel::Datalogger *dl = _inv->datalogger(d);
		if ( equal(dl, sc_dl.get()) ) {
			sc_dl = dl;
			newInstance = false;
			break;
		}
	}
	*/

	sc_dl = pushDatalogger(sc_dl.get());

	return sc_dl.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::DataloggerCalibration *
Convert2SC3::updateDataloggerCalibration(DataModel::Datalogger *sc_dl,
                                         DataModel::Stream *sc_stream,
                                         const FDSNXML::Channel *epoch) {
	bool newInstance = false;
	bool needUpdate = false;
	bool gain;

	try {
		gain = sc_dl->gain();
	}
	catch ( ... ) {
		// No gain, no calibration
		return NULL;
	}

	DataModel::DataloggerCalibrationIndex idx(
		sc_stream->dataloggerSerialNumber(),
		sc_stream->dataloggerChannel(), sc_stream->start()
	);

	DataModel::DataloggerCalibrationPtr sc_cal = sc_dl->dataloggerCalibration(idx);
	if ( sc_cal == NULL ) {
		sc_cal = new DataModel::DataloggerCalibration();
		sc_cal->setSerialNumber(sc_stream->dataloggerSerialNumber());
		sc_cal->setChannel(sc_stream->dataloggerChannel());
		sc_cal->setStart(sc_stream->start());
		sc_dl->add(sc_cal.get());
		newInstance = true;
	}

	BCK(oldEnd, Core::Time, sc_cal->end());
	BCK(oldGain, double, sc_cal->gain());
	BCK(oldGainFreq, double, sc_cal->gainFrequency());

	try { sc_cal->setEnd(sc_stream->end()); }
	catch ( ... ) { sc_cal->setEnd(Core::None); }

	sc_cal->setGain(gain);
	sc_cal->setGainFrequency(Core::None);

	UPD(needUpdate, oldEnd, Core::Time, sc_cal->end());
	UPD(needUpdate, oldGain, double, sc_cal->gain());
	UPD(needUpdate, oldGainFreq, double, sc_cal->gainFrequency());

	if ( !newInstance && needUpdate ) {
		sc_cal->update();
		SEISCOMP_DEBUG("Reused datalogger calibration for stream %s",
		               sc_stream->code().c_str());
	}

	return sc_cal.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Sensor *
Convert2SC3::updateSensor(const std::string &name,
                          const FDSNXML::Channel *epoch,
                          const FDSNXML::ResponseStage *resp,
                          ResponseType stageType,
                          const FDSNXML::BaseFilter *filter) {
	FDSNXML::UnitsType inputUnit = filter->inputUnits();

	DataModel::SensorPtr sc_sens = DataModel::Sensor::Create();
	//sc_sens->setName(sc_sens->publicID());
	sc_sens->setName(name);

	bool emptySensor = true;

	// No type yet: we use values as SM, SP, BB, VBB, ...
	// but there is no equivalent to fdsnxml
	//sc_sens->setType("...");

	try { epoch->sensor(); emptySensor = false; }
	catch ( ... ) {}

	try {
		sc_sens->setDescription(epoch->sensor().description());
		sc_sens->setManufacturer(epoch->sensor().manufacturer());
		sc_sens->setModel(epoch->sensor().model());

		if ( sc_sens->description().empty() )
			sc_sens->setDescription(epoch->sensor().type());
		else if ( sc_sens->model().empty() )
			sc_sens->setModel(epoch->sensor().type());
	}
	catch ( ... ) {}

	sc_sens->setUnit(inputUnit.name());
	if ( !inputUnit.description().empty() ) {
		DataModel::Blob blob;
		blob.setContent("{\"unit\":\"" + inputUnit.description() + "\"}");
		sc_sens->setRemark(blob);
	}

	if ( !sc_sens->unit().empty() ) emptySensor = false;

	switch ( stageType ) {
		case RT_PAZ:
		{
			DataModel::ResponsePAZPtr rp = convert(resp, &resp->polesZeros());
			checkPAZ(rp.get());

			bool newPAZ = true;
			for ( size_t f = 0; f < _inv->responsePAZCount(); ++f ) {
				DataModel::ResponsePAZ *paz = _inv->responsePAZ(f);
				if ( equal(paz, rp.get()) ) {
					rp = paz;
					newPAZ = false;
					break;
				}
			}

			if ( newPAZ ) {
				addRespToInv(rp.get());
				SEISCOMP_DEBUG("Added new Sensor.ResponsePAZ from paz: %s", rp->publicID().c_str());
			}
			else {
				SEISCOMP_DEBUG("Reused Sensor.ResponsePAZ from paz: %s", rp->publicID().c_str());
			}

			SEISCOMP_DEBUG("Update Sensor.response: %s -> %s",
			               sc_sens->publicID().c_str(), rp->publicID().c_str());

			sc_sens->setResponse(rp->publicID());
			emptySensor = false;
			break;
		}

		case RT_FAP:
		{
			DataModel::ResponseFAPPtr rp = convert(resp, &resp->responseList());
			checkFAP(rp.get());

			bool newFAP = true;
			for ( size_t f = 0; f < _inv->responseFAPCount(); ++f ) {
				DataModel::ResponseFAP *fap = _inv->responseFAP(f);
				if ( equal(fap, rp.get()) ) {
					rp = fap;
					newFAP = false;
					break;
				}
			}

			if ( newFAP ) {
				addRespToInv(rp.get());
				//SEISCOMP_DEBUG("Added new polynomial response from poly: %s", rp->publicID().c_str());
			}
			else {
				//SEISCOMP_DEBUG("reused polynomial response from poly: %s", rp->publicID().c_str());
			}

			sc_sens->setResponse(rp->publicID());
			emptySensor = false;

			break;
		}

		case RT_Poly:
		{
			DataModel::ResponsePolynomialPtr rp = convert(resp, &resp->polynomial());
			checkPoly(rp.get());

			bool newPoly = true;
			for ( size_t f = 0; f < _inv->responsePolynomialCount(); ++f ) {
				DataModel::ResponsePolynomial *poly = _inv->responsePolynomial(f);
				if ( equal(poly, rp.get()) ) {
					rp = poly;
					newPoly = false;
					break;
				}
			}

			if ( newPoly ) {
				addRespToInv(rp.get());
				//SEISCOMP_DEBUG("Added new polynomial response from poly: %s", rp->publicID().c_str());
			}
			else {
				//SEISCOMP_DEBUG("reused polynomial response from poly: %s", rp->publicID().c_str());
			}

			sc_sens->setLowFrequency(resp->polynomial().frequencyLowerBound().value());
			sc_sens->setHighFrequency(resp->polynomial().frequencyUpperBound().value());
			sc_sens->setResponse(rp->publicID());
			emptySensor = false;

			break;
		}

		default:
			break;
	}

	/*
	bool newInstance = true;
	for ( size_t s = 0; s < _inv->sensorCount(); ++s ) {
		DataModel::Sensor *sens = _inv->sensor(s);
		if ( equal(sens, sc_sens.get()) ) {
			sc_sens = sens;
			newInstance = false;
			break;
		}
	}
	*/

	if ( emptySensor ) return NULL;

	SEISCOMP_DEBUG("Pushing new sensor: %s", sc_sens->publicID().c_str());
	sc_sens = pushSensor(sc_sens.get());

	return sc_sens.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::SensorCalibration *
Convert2SC3::updateSensorCalibration(DataModel::Sensor *sc_sens, DataModel::Stream *sc_stream,
                                     const FDSNXML::Channel *epoch,
                                     const FDSNXML::ResponseStage *resp) {
	bool newInstance = false;
	bool needUpdate = false;

	DataModel::SensorCalibrationIndex idx(
		sc_stream->sensorSerialNumber(),
		sc_stream->sensorChannel(), sc_stream->start()
	);

	DataModel::SensorCalibrationPtr sc_cal = sc_sens->sensorCalibration(idx);
	if ( sc_cal == NULL ) {
		sc_cal = new DataModel::SensorCalibration();
		sc_cal->setSerialNumber(sc_stream->sensorSerialNumber());
		sc_cal->setChannel(sc_stream->sensorChannel());
		sc_cal->setStart(sc_stream->start());
		sc_sens->add(sc_cal.get());
		newInstance = true;
	}

	BCK(oldEnd, Core::Time, sc_cal->end());
	BCK(oldGain, double, sc_cal->gain());
	BCK(oldGainFreq, double, sc_cal->gainFrequency());

	try { sc_cal->setEnd(sc_stream->end()); }
	catch ( ... ) { sc_cal->setEnd(Core::None); }

	sc_cal->setGain(Core::None);
	sc_cal->setGainFrequency(Core::None);

	try { sc_cal->setGain(fabs(resp->stageGain().value())); } catch ( ... ) {}
	try { sc_cal->setGainFrequency(fabs(resp->stageGain().frequency())); }
	catch ( ... ) {}

	UPD(needUpdate, oldEnd, Core::Time, sc_cal->end());
	UPD(needUpdate, oldGain, double, sc_cal->gain());
	UPD(needUpdate, oldGainFreq, double, sc_cal->gainFrequency());

	if ( !newInstance && needUpdate ) sc_cal->update();

	return sc_cal.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Datalogger *
Convert2SC3::pushDatalogger(DataModel::Datalogger *dl) {
	ObjectLookup::iterator it = _dataloggerLookup.find(dl->name());
	if ( it != _dataloggerLookup.end() ) {
		DataModel::Datalogger *cdl = (DataModel::Datalogger*)it->second;
		if ( !equal(cdl, dl) ) {
			*cdl = *dl;
			cdl->update();
			SEISCOMP_DEBUG("Updated datalogger: %s", cdl->publicID().c_str());
		}
		else
			SEISCOMP_DEBUG("Reused datalogger: %s", cdl->publicID().c_str());
		return cdl;
	}

	_inv->add(dl);
	_dataloggerLookup[dl->name()] = dl;
	SEISCOMP_DEBUG("Added new datalogger: %s", dl->publicID().c_str());
	return dl;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Sensor *Convert2SC3::pushSensor(DataModel::Sensor *sens) {
	ObjectLookup::iterator it = _sensorLookup.find(sens->name());
	if ( it != _sensorLookup.end() ) {
		DataModel::Sensor *csens = (DataModel::Sensor*)it->second;
		if ( !equal(csens, sens) ) {
			*csens = *sens;
			csens->update();
			SEISCOMP_DEBUG("Updated sensor: %s", csens->publicID().c_str());
		}
		else
			SEISCOMP_DEBUG("Reused sensor: %s", csens->publicID().c_str());
		return csens;
	}

	_inv->add(sens);
	_sensorLookup[sens->name()] = sens;
	SEISCOMP_DEBUG("Added new sensor: %s", sens->publicID().c_str());
	return sens;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
