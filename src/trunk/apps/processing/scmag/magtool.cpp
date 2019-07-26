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


#include "component.h"
#include "magtool.h"
#include "dmutil.h"

#include <seiscomp3/math/mean.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/logging/log.h>

#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/realquantity.h>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/utils.h>

#include <seiscomp3/client/application.h>
#include <seiscomp3/utils/timer.h>

#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <string>
#include <vector>
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::Processing;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace Magnitudes {


namespace {


std::string averageMethodToString(const MagTool::AverageDescription &desc) {
	if ( desc.type == MagTool::Default )
		return "default";
	else if ( desc.type == MagTool::Mean )
		return "mean";
	else if ( desc.type == MagTool::TrimmedMean )
		return "trimmed mean(" + Core::toString(desc.parameter) + ")";
	else if ( desc.type == MagTool::Median )
		return "median";
	else if ( desc.type == MagTool::TrimmedMedian )
		return "trimmed median(" + Core::toString(desc.parameter) + ")";

	return "unknown";
}


bool averageMethodFromString(MagTool::AverageDescription &desc, const std::string &method) {
	if ( method.empty() )
		return false;

	if ( method == "mean" ) {
		desc.type = MagTool::Mean;
		return true;
	}

	if ( method == "median" ) {
		desc.type = MagTool::Median;
		return true;
	}

	if ( method.compare(0, 13, "trimmed mean(") == 0 ) {
		size_t p = method.find_last_of(')', 13);
		if ( p == string::npos )
			return false;

		double percent;
		if ( !Core::fromString(percent, method.substr(13, p-13)) )
			return false;

		desc.type = MagTool::TrimmedMean;
		desc.parameter = percent;
		return true;
	}

	if ( method.compare(0, 13, "trimmed median(") == 0 ) {
		size_t p = method.find_last_of(')', 15);
		if ( p == string::npos )
			return false;

		double percent;
		if ( !Core::fromString(percent, method.substr(15, p-15)) )
			return false;

		desc.type = MagTool::TrimmedMedian;
		desc.parameter = percent;
		return true;
	}

	return false;
}


bool hasHigherPriority(const DataModel::Amplitude *candidate,
                       const DataModel::Amplitude *reference) {
	DataModel::EvaluationMode cm = DataModel::AUTOMATIC;
	DataModel::EvaluationMode rm = DataModel::AUTOMATIC;

	try { cm = candidate->evaluationMode(); }
	catch ( ... ) {}

	try { rm = reference->evaluationMode(); }
	catch ( ... ) {}

	// Different evaluationMode: prefer MANUAL solutions
	if ( cm != rm ) {
		if ( cm == DataModel::MANUAL ) return true;
	}

	try {
		// Candidate is more recent than reference: prefer it
		return candidate->creationInfo().creationTime() >
		       reference->creationInfo().creationTime();
	}
	catch ( ... ) {}

	return false;
}


//#define INVALID_MAG std::numeric_limits<double>::quiet_NaN()
#define INVALID_MAG 0


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
typedef vector<DataModel::StationMagnitudeCPtr> StaMagArray;
typedef DataModel::StationMagnitude    StaMag;
typedef DataModel::StationMagnitudePtr StaMagPtr;
typedef DataModel::Magnitude    NetMag;
typedef DataModel::MagnitudePtr NetMagPtr;
typedef multimap<string, NetMagPtr> NetMagMap;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagTool::MagTool() {
	_dbAccesses = 0;
	_allowReprocessing = false;
	_staticUpdate = false;
	_keepWeights = false;
	_warningLevel = numeric_limits<double>::max();

	_summaryMagnitudeEnabled = true;
	_summaryMagnitudeType = "M";
	_summaryMagnitudeMinStationCount = 1;

	_defaultCoefficients.a = 0.0;
	_defaultCoefficients.b = 1.0;

	_magnitudeCoefficients["MLv"] = SummaryMagnitudeCoefficients(None, 2);
	_magnitudeCoefficients["Mw(mB)"] = SummaryMagnitudeCoefficients(0.4, -1);
	_magnitudeCoefficients["Mw(Mwp)"] = SummaryMagnitudeCoefficients(0.4, -1);

	_minimumArrivalWeight = 0.5;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagTool::~MagTool() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagTool::setSummaryMagnitudeEnabled(bool e) {
	_summaryMagnitudeEnabled = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagTool::setSummaryMagnitudeMinStationCount(int n) {
	_summaryMagnitudeMinStationCount = n;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagTool::setSummaryMagnitudeType(const std::string &type) {
	_summaryMagnitudeType = type;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagTool::setSummaryMagnitudeBlacklist(const std::vector<std::string> &list) {
	std::copy(list.begin(), list.end(), std::inserter(_summaryMagnitudeBlacklist, _summaryMagnitudeBlacklist.end()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagTool::setSummaryMagnitudeWhitelist(const std::vector<std::string> &list) {
	std::copy(list.begin(), list.end(), std::inserter(_summaryMagnitudeWhitelist, _summaryMagnitudeWhitelist.end()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagTool::setSummaryMagnitudeDefaultCoefficients(const SummaryMagnitudeCoefficients &c) {
	if ( c.a ) _defaultCoefficients.a = c.a;
	if ( c.b ) _defaultCoefficients.b = c.b;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagTool::setSummaryMagnitudeCoefficients(const Coefficients &c) {
	_magnitudeCoefficients = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagTool::setAverageMethods(const AverageMethods &m) {
	_magnitudeAverageMethods = m;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagTool::setMinimumArrivalWeight(double w) {
	_minimumArrivalWeight = w;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagTool::init(const MagnitudeTypes &mags, const Core::TimeSpan& expiry,
                   bool allowReprocessing, bool staticUpdate, bool keepWeights, double warningLevel) {
	_cacheSize = expiry;
	_objectCache.setDatabaseArchive(SCCoreApp->query());
	_objectCache.setTimeSpan(_cacheSize);
	_objectCache.setPopCallback(std::bind1st(std::mem_fun(&MagTool::publicObjectRemoved), this));

	_dbAccesses = 0;
	_allowReprocessing = allowReprocessing;
	_staticUpdate = staticUpdate;
	_keepWeights = keepWeights;
	_warningLevel = warningLevel;

	MagnitudeTypeList *services = MagnitudeProcessorFactory::Services();

	if ( services ) {
		_registeredMagTypes = *services;
		delete services;
	}

	_magTypes = mags;

	std::string logMagTypes;
	std::string logMagAverageTypes;
	std::string sumMagTypes;
	for ( MagnitudeTypes::iterator it = _magTypes.begin();
	      it != _magTypes.end(); ) {
		logMagTypes += " * ";
		logMagAverageTypes += " * ";

		if ( std::find(_registeredMagTypes.begin(),
		               _registeredMagTypes.end(), *it) == _registeredMagTypes.end() ) {
			logMagTypes += *it;
			logMagTypes += ": Disabled (unknown type)";
			_magTypes.erase(it++);
		}
		else {
			logMagTypes += *it;
			logMagTypes += ": OK";
			logMagAverageTypes += *it + ": ";

			MagnitudeProcessorPtr proc = MagnitudeProcessorFactory::Create(it->c_str());
			if ( proc ) {
				_processors.insert(ProcessorList::value_type(proc->amplitudeType(), proc));
				double estimation, stdError;
				if ( proc->estimateMw(6.0, estimation, stdError) != MagnitudeProcessor::MwEstimationNotSupported ) {
					logMagTypes += '\n';
					logMagTypes += " * ";
					logMagTypes += proc->typeMw();
					logMagTypes += ": OK";

					sumMagTypes += " * ";
					sumMagTypes += proc->typeMw();
					if ( isTypeEnabledForSummaryMagnitude(proc->typeMw()) )
						sumMagTypes += ": OK";
					else
						sumMagTypes += ": Disabled";
					sumMagTypes += '\n';

				}
			}

			AverageMethods::iterator am_it = _magnitudeAverageMethods.find(*it);
			if ( am_it == _magnitudeAverageMethods.end() )
				logMagAverageTypes += "default";
			else
				logMagAverageTypes += averageMethodToString(am_it->second);

			logMagAverageTypes += "\n";

			sumMagTypes += " * ";
			sumMagTypes += *it;
			if ( isTypeEnabledForSummaryMagnitude(*it) )
				sumMagTypes += ": OK";
			else
				sumMagTypes += ": Disabled";
			sumMagTypes += '\n';

			++it;
		}

		logMagTypes += '\n';
	}

	SEISCOMP_INFO("Magnitudes to calculate:\n%s", logMagTypes.c_str());
	SEISCOMP_INFO("Average methods:\n%s", logMagAverageTypes.c_str());
	SEISCOMP_INFO("Summary magnitude enabled = %s", _summaryMagnitudeEnabled?"yes":"no");
	SEISCOMP_INFO("Summary magnitudes:\n%s", sumMagTypes.c_str());
	SEISCOMP_INFO("Using default summary coefficients: a = %.2f, b = %.2f",
	              *_defaultCoefficients.a, *_defaultCoefficients.b);

	for ( Coefficients::iterator it = _magnitudeCoefficients.begin();
	      it != _magnitudeCoefficients.end(); ++it ) {
		SEISCOMP_INFO("Using '%s' summary coefficients: a = %s, b = %s",
		              it->first.c_str(), it->second.a?toString(*it->second.a).c_str():"[default]",
		              it->second.b?toString(*it->second.b).c_str():"[default]");
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagTool::done() {
	SEISCOMP_INFO("Shutting down MagTool\n - database accesses while runtime: %lu", (unsigned long)_dbAccesses);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::StationMagnitude *MagTool::getStationMagnitude(
        DataModel::Origin *origin,
        const DataModel::WaveformStreamID &wfid,
        const string &type, double value, bool update) const {
	StaMag *mag = NULL;

	for ( size_t i = 0; i < origin->stationMagnitudeCount(); ++i ) {
		StaMag *stamag = origin->stationMagnitude(i);
		if ( equivalent(stamag->waveformID(), wfid) && stamag->type() == type ) {
			mag = stamag;
			break;
		}
	}

	if ( !update && mag != NULL ) return NULL;

	// Returns a StationMagnitude for the given Origin, WaveformStreamID
	// and magnitude type. If an instance already exists, it is updated,
	// otherwise a new instance is created.

	if ( mag == NULL ) {
		if ( SCCoreApp->hasCustomPublicIDPattern() )
			mag = StaMag::Create();
		else {
			string id = origin->publicID() + "/staMag/" + type + "/" +
			            wfid.networkCode() + "." + wfid.stationCode();

			mag = StaMag::Create(id);
		}

		if ( mag == NULL ) {
			SEISCOMP_ERROR("Failed to create StaMag");
			return NULL;
		}

		Time now = Time::GMT();
		SCCoreApp->logObject(outputMagLog, now);

		DataModel::CreationInfo ci;
		ci.setCreationTime(now);
		ci.setAgencyID(SCCoreApp->agencyID());
		ci.setAuthor(SCCoreApp->author());
		mag->setCreationInfo(ci);

		mag->setType(type);
		mag->setWaveformID(wfid);

		SEISCOMP_DEBUG("Created new station magnitude %s (%s) for origin %s",
		              mag->publicID().c_str(), mag->type().c_str(),
		              origin->publicID().c_str());

	}
	else {
		Time now = Time::GMT();
		try { mag->creationInfo().setModificationTime(now); }
		catch ( ... ) {
			DataModel::CreationInfo ci;
			ci.setModificationTime(now);
			mag->setCreationInfo(ci);
		}

		mag->update();
		SCCoreApp->logObject(outputMagLog, now);
	}

	if ( origin != mag->parent() ) {
		// otherwise, we get a warning at origin->add.
		// XXX maybe that shouldn't generate a warning if an
		// XXX object is added repeatedly to the same parent?

		if ( mag->parent() != NULL ) {
			SEISCOMP_ERROR(
				"This should never happen origin=%s "
				"but StaMag parent=%s",
				origin->publicID().c_str(),
				mag->parent()->publicID().c_str());
		}
		origin->add(mag);
	}

	//mag->setAmplitudeID(ampl->publicID());
	mag->setMagnitude(value);

	return mag;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Magnitude *MagTool::getMagnitude(DataModel::Origin* origin,
                                            const std::string &type,
                                            bool* newInstance) const {
	DataModel::Magnitude *mag = NULL;
	for ( size_t i = 0; i < origin->magnitudeCount(); ++i ) {
		NetMag *nmag = origin->magnitude(i);
		if ( nmag->type() == type ) {
			mag = nmag;
			break;
		}
	}

	if ( !mag ) {
		if ( SCCoreApp->hasCustomPublicIDPattern() )
			mag = NetMag::Create();
		else {
			std::string id = origin->publicID() + "/netMag/" + type;
			mag = NetMag::Create(id);
		}

		if ( mag == NULL ) {
			SEISCOMP_ERROR("Failed to create NetMag");
			return NULL;
		}

		Time now = Time::GMT();
		SCCoreApp->logObject(outputMagLog, now);

		DataModel::CreationInfo ci;
		ci.setCreationTime(now);
		ci.setAgencyID(SCCoreApp->agencyID());
		ci.setAuthor(SCCoreApp->author());
		mag->setCreationInfo(ci);
		mag->setType(type);
		origin->add(mag);

		if ( newInstance ) *newInstance = true;
	}
	else {
		if ( !_allowReprocessing ) {
			try {
				// Check if evaluation status is set
				mag->evaluationStatus();
				return NULL;
			}
			catch ( ... ) {}
		}

		if ( newInstance ) *newInstance = false;
	}

	return mag;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Magnitude *MagTool::getMagnitude(DataModel::Origin *origin,
                                            const string &type,
                                            double value,
                                            bool* newInstance) const {
	bool tmpNewInstance;
	NetMag *mag = getMagnitude(origin, type, &tmpNewInstance);
	if ( mag ) {
		mag->setMagnitude(value);
		if ( !tmpNewInstance ) {
			Time now = Time::GMT();
			try { mag->creationInfo().setModificationTime(now); }
			catch ( ... ) {
				DataModel::CreationInfo ci;
				ci.setModificationTime(now);
				mag->setCreationInfo(ci);
			}
			mag->update();
			SCCoreApp->logObject(outputMagLog, now);
		}

		if ( newInstance ) *newInstance = tmpNewInstance;
	}

	return mag;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagTool::computeStationMagnitude(const DataModel::Amplitude *ampl,
                                      const DataModel::Origin *origin,
                                      const DataModel::SensorLocation *loc,
                                      double distance, double depth,
                                      MagnitudeList& mags) {
	const string &atype = ampl->type();

	std::pair<ProcessorList::iterator, ProcessorList::iterator> itp =
		_processors.equal_range(atype);

	double period = 0;
	try { period = ampl->period().value(); } catch ( ... ) {}

	double snr = 0;
	try { snr = ampl->snr(); } catch ( ... ) {}

	Util::KeyValues *params = fetchParams(ampl);

	for ( ProcessorList::iterator it = itp.first; it != itp.second; ++it ) {
		Settings settings(
			SCCoreApp->configModuleName(),
			ampl->waveformID().networkCode(),
			ampl->waveformID().stationCode(),
			ampl->waveformID().locationCode(),
			ampl->waveformID().channelCode(),
			&SCCoreApp->configuration(),
			params);
		if ( ! it->second->setup(settings))
			continue;

		double mag;
		double ampValue;
		try {
			ampValue = ampl->amplitude().value();
		}
		catch ( ... ) {
			SEISCOMP_WARNING("Amplitude value not set, this is not expected: sid=%s.%s.%s.%s, type=%s",
			                 ampl->waveformID().networkCode().c_str(),
			                 ampl->waveformID().stationCode().c_str(),
			                 ampl->waveformID().locationCode().c_str(),
			                 ampl->waveformID().channelCode().c_str(),
			                 ampl->type().c_str());
			continue;
		}

		MagnitudeProcessor::Status status =
			it->second->computeMagnitude(ampValue, ampl->unit(),
			                             period, snr, distance, depth,
			                             origin, loc, ampl, mag);

		bool passedQC = true;

		if ( status != MagnitudeProcessor::OK ) {
			SEISCOMP_DEBUG("Magnitude failed: sid=%s.%s.%s.%s, type=%s, error=%s",
			                 ampl->waveformID().networkCode().c_str(),
			                 ampl->waveformID().stationCode().c_str(),
			                 ampl->waveformID().locationCode().c_str(),
			                 ampl->waveformID().channelCode().c_str(),
			                 ampl->type().c_str(), status.toString());
			if ( !it->second->treatAsValidMagnitude() )
				continue;
			passedQC = false;
		}

		mags.push_back(MagnitudeEntry(it->second.get(), mag, passedQC));

		const string &net = ampl->waveformID().networkCode();
		const string &sta = ampl->waveformID().stationCode();

		SEISCOMP_DEBUG(
			"origin '%20s' %5s: d=%6.2f z=%5.1f %2s.%-5s mag=%4.2f",
			origin->publicID().c_str(), atype.c_str(),
			distance, depth, net.c_str(), sta.c_str(), mag);
	}

	return !mags.empty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagTool::computeNetworkMagnitude(DataModel::Origin *origin, const std::string &mtype, DataModel::MagnitudePtr netMag) {
	using namespace DataModel;

	try {
		netMag->evaluationStatus();
		if ( !_allowReprocessing ) return false;
	}
	catch ( ... ) {}

	// Set configured average method
	AverageDescription averageMethod;
	AverageMethods::iterator am_it = _magnitudeAverageMethods.find(mtype);
	if ( am_it == _magnitudeAverageMethods.end() )
		averageMethod.type = Default;
	else
		averageMethod = am_it->second;

	vector<double> mv; // vector of station magnitude values
	double value = 0.0;
	OPT(double) stdev = 0.0;
	string methodID;
	std::vector<double> weights;
	bool invalidMagnitude = false;

	StaMagArray stationMagnitudesZeroWeight;
	StaMagArray stationMagnitudes;

	// retrieve from the origin all station magnitudes of specified type
	for ( size_t i = 0, nmag = origin->stationMagnitudeCount(); i < nmag; ++i ) {
		const DataModel::StationMagnitude *mag = origin->stationMagnitude(i);

		if ( mag->type() != mtype ) continue;

		try {
			if ( !mag->passedQC() ) {
				stationMagnitudesZeroWeight.push_back(mag);
				continue;
			}
		}
		catch ( ... ) {}

		stationMagnitudes.push_back(mag);

		double m = mag->magnitude().value();
		mv.push_back(m);
	}

	int count = mv.size();

	if ( count ) {
		weights.resize(mv.size(), 1);

		if ( !computeAverage(averageMethod, mv, weights, methodID, value, *stdev) )
			return false;
	}
	else if ( stationMagnitudesZeroWeight.empty() )
		return false;
	else {
		stdev = Core::None;
		value = INVALID_MAG;
		invalidMagnitude = true;
	}

	// adding stamag referencomputeStationMagnitudeces and set the weights
	size_t weightIndex = 0;
	size_t staCount = 0;

	// Count contributing station magnitudes
	for ( StaMagArray::iterator it = stationMagnitudes.begin();
	      it != stationMagnitudes.end(); ++it, ++weightIndex ) {
		if ( weights[weightIndex] > 0 ) ++staCount;
	}

	weightIndex = 0;

	for ( StaMagArray::iterator it = stationMagnitudes.begin();
	      it != stationMagnitudes.end(); ++it ) {
		const DataModel::StationMagnitude *stationMagnitude = (*it).get();

		StationMagnitudeContributionPtr magRef =
			netMag->stationMagnitudeContribution(stationMagnitude->publicID());

		if ( !magRef ) {
			magRef = new StationMagnitudeContribution(stationMagnitude->publicID());
			magRef->setWeight(weights[weightIndex]);
			if ( staCount )
				magRef->setResidual(stationMagnitude->magnitude().value() - value);
			netMag->add(magRef.get());
		}
		else {
			double oldWeight = -1, oldResidual = 0;
			double residual = stationMagnitude->magnitude().value() - value;
			bool hasResidual = false;

			try {
				oldWeight = magRef->weight();
			}
			catch ( ... ) {}

			try {
				oldResidual = magRef->residual();
				hasResidual = true;
			}
			catch ( ... ) {}

			if ( oldWeight != weights[weightIndex]
			  || oldResidual != residual
			  || bool(staCount > 0) != hasResidual ) {
				magRef->setWeight(weights[weightIndex]);
				if ( staCount )
					magRef->setResidual(residual);
				else
					magRef->setResidual(Core::None);
				magRef->update();
				SEISCOMP_DEBUG("Updating magnitude reference for %s", stationMagnitude->publicID().c_str());
			}
		}

		++weightIndex;
	}

	// Associate zero weight magnitudes with weight 0
	for ( StaMagArray::iterator it = stationMagnitudesZeroWeight.begin();
	      it != stationMagnitudesZeroWeight.end(); ++it ) {
		const DataModel::StationMagnitude *stationMagnitude = (*it).get();

		StationMagnitudeContributionPtr magRef =
			netMag->stationMagnitudeContribution(stationMagnitude->publicID());

		if ( !magRef ) {
			magRef = new StationMagnitudeContribution(stationMagnitude->publicID());
			magRef->setWeight(0.0);
			if ( staCount )
				magRef->setResidual(stationMagnitude->magnitude().value() - value);
			netMag->add(magRef.get());
		}
		else {
			double oldWeight = -1, oldResidual = 0;
			double residual = stationMagnitude->magnitude().value() - value;
			bool hasResidual = false;

			try {
				oldWeight = magRef->weight();
			}
			catch ( ... ) {}

			try {
				oldResidual = magRef->residual();
				hasResidual = true;
			}
			catch ( ... ) {}

			if ( oldWeight != 0
			  || oldResidual != residual
			  || bool(staCount > 0) != hasResidual ) {
				magRef->setWeight(0.0);
				if ( staCount )
					magRef->setResidual(residual);
				else
					magRef->setResidual(Core::None);
				magRef->update();
				SEISCOMP_DEBUG("Updating magnitude reference for %s", stationMagnitude->publicID().c_str());
			}
		}
	}

	netMag->setMethodID(methodID);
	netMag->setMagnitude(RealQuantity(value, stdev, Core::None, Core::None, Core::None));
	if ( invalidMagnitude )
		netMag->setEvaluationStatus(DataModel::EvaluationStatus(DataModel::REJECTED));
	else
		netMag->setEvaluationStatus(Core::None);

	netMag->setStationCount(staCount);

	ProcessorList::iterator it = _processors.find(mtype);
	if ( it == _processors.end() ) return false;

	if ( staCount ) {
		double Mw;
		double MwStdev;
		MagnitudeProcessor::Status res = it->second->estimateMw(value, Mw, MwStdev);
		if ( res == MagnitudeProcessor::OK ) {
			MwStdev = *stdev > MwStdev ? *stdev : MwStdev;
			//MwStdev = stdev;
			netMag = getMagnitude(origin, it->second->typeMw(), Mw);
			if ( netMag ) {
				netMag->setStationCount(staCount);

				netMag->setEvaluationStatus(Core::None);
				netMag->magnitude().setUncertainty(MwStdev);
				netMag->magnitude().setLowerUncertainty(Core::None);
				netMag->magnitude().setUpperUncertainty(Core::None);
				netMag->magnitude().setConfidenceLevel(Core::None);
			}
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagTool::computeSummaryMagnitude(DataModel::Origin *origin) {
	if ( !_summaryMagnitudeEnabled ) return false;
	if ( _summaryMagnitudeType.empty() ) return false;

	double value = 0.0, totalWeight = 0.0;
	int count = 0;

	for ( size_t i = 0; i < origin->magnitudeCount(); ++i ) {
		NetMag *nmag = origin->magnitude(i);
		const std::string &type = nmag->type();
		int n = 0;

		if ( type == _summaryMagnitudeType )
			continue;

		try {
			// Ignore rejected magnitudes
			if ( nmag->evaluationStatus() == DataModel::REJECTED )
				continue;
		}
		catch ( ... ) {}

		if ( !isTypeEnabledForSummaryMagnitude(type) ) continue;

		try { n = nmag->stationCount(); }
		catch ( ... ) {}

		if ( n < _summaryMagnitudeMinStationCount ) continue;

		double a = *_defaultCoefficients.a, b=*_defaultCoefficients.b; // defaults

		Coefficients::iterator it = _magnitudeCoefficients.find(type);
		if ( it != _magnitudeCoefficients.end() ) {
			if ( it->second.a ) a = *(it->second.a);
			if ( it->second.b ) b = *(it->second.b);
		}

		double weight = a*n+b;
		if ( weight <= 0 )
			continue;

		totalWeight += weight;
		value += weight*nmag->magnitude().value();
		// The total count is currently the maximum count for any individual magnitude.
		// FIXME: Something better is needed here.
		count  = n > count ? n : count;
	}

	// No magnitudes available
	if ( totalWeight == 0 )
		return false;

	// Simple average
	value = value / totalWeight;

	bool newInstance;
	NetMagPtr mag = getMagnitude(origin, _summaryMagnitudeType, &newInstance);
	if ( !mag )
		return false;

	if ( !newInstance ) {
		try {
			// Check for changes otherwise discard the update
			if ( fabs(mag->magnitude().value() - value) < 0.0001 &&
			     mag->stationCount() == count ) {
				SEISCOMP_DEBUG("Skipping summary magnitude update, nothing changed");
				return false;
			}
		}
		catch ( Core::GeneralException &ex ) {
			SEISCOMP_WARNING("Checking existing summary magnitude: %s", ex.what());
		}

		Time now = Time::GMT();
		try { mag->creationInfo().setModificationTime(now); }
		catch ( ... ) {
			DataModel::CreationInfo ci;
			ci.setModificationTime(now);
			mag->setCreationInfo(ci);
		}
		mag->update();
		SCCoreApp->logObject(outputMagLog, now);
	}

	mag->setMagnitude(value);
	// StationCount???
	mag->setMethodID("weighted average");
	mag->setStationCount(count);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MagTool::retrieveMissingPicksAndArrivalsFromDB(const DataModel::Origin *origin) {
	int count = 0;

	// see if any picks are missing; if so, query DB
	set<string> missingPicks;
	for (int i=0, arrivalCount = origin->arrivalCount();
	     i < arrivalCount; i++) {

		const DataModel::Arrival *arr = origin->arrival(i);
		if ( !validArrival(arr, _minimumArrivalWeight) ) continue;

		const string &pickID = arr->pickID();

		// Is the pick already cached?
		if ( DataModel::Pick::Find(pickID) ) continue;

		// In the case of an uncached pick a amplitude pickID
		// association is maybe available
		StaAmpMap::iterator it = _ampl.find(pickID);
		if ( it != _ampl.end() ) {
			SEISCOMP_WARNING("Pick '%s' is not cached but associated to amplitudes",
			                 pickID.c_str());
			continue;
		}

		missingPicks.insert(pickID);
	}

	if (missingPicks.size() == 0)
		return 0;

	if ( !SCCoreApp->query() ) {
		SEISCOMP_WARNING("retrieveMissingPicksAndArrivalsFromDB: DB not configured");
		return 0;
	}

	SEISCOMP_INFO("RETRIEVING %lu MISSING PICKS", (unsigned long)missingPicks.size());

	DataModel::DatabaseIterator dbit;
	DataModel::Object* object;

	dbit = SCCoreApp->query()->getPicks(origin->publicID());
	for ( ; (object=*dbit) != NULL; ++dbit) {

		DataModel::PickPtr pick =
			DataModel::Pick::Cast(object);
		if ( ! pick)
			continue; // actually a bad error!

		const string &id = pick->publicID();
		if (missingPicks.find(id) == missingPicks.end())
			continue;

		if ( !feed(pick.get()))
			continue;

		count++;
	}
	dbit.close();

	dbit = SCCoreApp->query()->getAmplitudesForOrigin(origin->publicID());
	for ( ; (object=*dbit) != NULL; ++dbit) {

		DataModel::AmplitudePtr ampl =
			DataModel::Amplitude::Cast(object);
		if ( !ampl )
			continue; // actually a bad error!

		const string &id = ampl->pickID();
		if ( missingPicks.find(id) == missingPicks.end() )
			continue;

		if ( !_feed(ampl.get(), false) )
			continue;

		count++;
	}
	dbit.close();

	SEISCOMP_INFO("RETRIEVED  %d MISSING OBJECTS", count);

	return count;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagTool::OriginList *MagTool::createBinding(const std::string &pickID) {
	std::pair<OriginMap::iterator, bool>
		itp = _orgs.insert(OriginMap::value_type(pickID, OriginList()));

	return &itp.first->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagTool::bind(const std::string &pickID, DataModel::Origin *origin) {
	OriginList *origins = originsForPick(pickID);
	if ( origins )
		origins->push_back(origin);
	else
		SEISCOMP_DEBUG("No complete binding for pick %s yet", pickID.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagTool::OriginList *MagTool::originsForPick(const std::string &pickID) {
	OriginMap::iterator it = _orgs.find(pickID);

	if ( it == _orgs.end() ) return NULL;

	return &it->second;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagTool::isTypeEnabledForSummaryMagnitude(const std::string &type) const {
	return (_summaryMagnitudeWhitelist.empty()?true:_summaryMagnitudeWhitelist.find(type) != _summaryMagnitudeWhitelist.end())
	    && (_summaryMagnitudeBlacklist.empty()?true:_summaryMagnitudeBlacklist.find(type) == _summaryMagnitudeBlacklist.end());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Util::KeyValues *MagTool::fetchParams(const DataModel::Amplitude *amp) {
	std::string stationID = amp->waveformID().networkCode() + "." +
	                        amp->waveformID().stationCode();
	ParameterMap::const_iterator it = _parameters.find(stationID);

	if ( it != _parameters.end() )
		return it->second.get();
	else if ( SCCoreApp->configModule() != NULL ) {
		for ( size_t i = 0; i < SCCoreApp->configModule()->configStationCount(); ++i ) {
			DataModel::ConfigStation *station = SCCoreApp->configModule()->configStation(i);

			if ( station->networkCode() != amp->waveformID().networkCode() ) continue;
			if ( station->stationCode() != amp->waveformID().stationCode() ) continue;

			DataModel::Setup *setup = DataModel::findSetup(station, SCCoreApp->name());
			if ( setup ) {
				DataModel::ParameterSet* ps = NULL;
				try {
					ps = DataModel::ParameterSet::Find(setup->parameterSetID());
				}
				catch ( Core::ValueException & ) {
					continue;
				}

				if ( !ps ) {
					SEISCOMP_ERROR("Cannot find parameter set %s", setup->parameterSetID().c_str());
					continue;
				}

				Util::KeyValuesPtr keys = new Util::KeyValues;
				keys->init(ps);
				_parameters[stationID] = keys;
				return keys.get();
			}
		}
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagTool::computeAverage(AverageDescription &avg,
                             const std::vector<double> &values,
                             std::vector<double> &weights,
                             std::string &methodID, double &value, double &stdev) {
	double trimPercentage = 0;
	double cumw = 0.0;

	switch( avg.type ) {
		case Default:
			if ( values.size() > 3 ) {
				trimPercentage = 25.;
				methodID = "trimmed mean(25)";
			}
			else
				methodID = "mean";

			// compute the trimmed mean and the corresponding weights
			Math::Statistics::computeTrimmedMean(values, trimPercentage, value, stdev, &weights);
			break;

		case Mean:
			methodID = "mean";
			Math::Statistics::computeTrimmedMean(values, 0, value, stdev, &weights);
			break;

		case TrimmedMean:
			methodID = "trimmed mean(" + Core::toString(avg.parameter) + ")";
			Math::Statistics::computeTrimmedMean(values, avg.parameter, value, stdev, &weights);
			break;

		case Median:
			methodID = "median";
			value = Math::Statistics::median(values);
			if ( values.size() > 1 ) {
				stdev = 0;
				for ( size_t i = 0; i < values.size(); ++i )
					stdev += (values[i] - value) * (values[i] - value);
				stdev /= values.size()-1;
				stdev = sqrt(stdev);
			}
			break;

		case TrimmedMedian:
			methodID = "trimmed median(" + Core::toString(avg.parameter) + ")";
			Math::Statistics::computeTrimmedMean(values, avg.parameter, value, stdev, &weights);
			value = Math::Statistics::median(values);
			stdev = 0;
			for ( size_t i = 0; i < values.size(); ++i ) {
				stdev += (values[i] - value) * (values[i] - value) * weights[i];
				cumw += weights[i];
			}

			if ( cumw > 1 )
				stdev = sqrt(stdev/(cumw-1));
			else
				stdev = 0;

			break;

		default:
			return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagTool::processOrigin(DataModel::Origin* origin) {
	SEISCOMP_INFO("working on origin %s", origin->publicID().c_str());

	retrieveMissingPicksAndArrivalsFromDB(origin);

	if ( _staticUpdate )
		return processOriginUpdateOnly(origin);

	double depth;

	try { depth = origin->depth().value(); }
	catch ( ... ) {
		SEISCOMP_WARNING("%s: depth not set, ignoring origin",
		                 origin->publicID().c_str());
		return false;
	}

	set<string> magTypes;
	typedef pair<DataModel::SensorLocation*, double> LocationAndDistance;
	typedef pair<DataModel::PickCPtr, LocationAndDistance> PickStreamEntry;
	typedef map<string, PickStreamEntry> PickStreamMap;
	PickStreamMap pickStreamMap;

	// find associated picks and amplitudes:
	for ( int i = 0, arrivalCount = origin->arrivalCount(); i < arrivalCount; ++i ) {
		const DataModel::Arrival *arr = origin->arrival(i);

		const string &pickID = arr->pickID();

		DataModel::PickCPtr pick = _objectCache.get<DataModel::Pick>(pickID);
		if ( ! pick) {
			SEISCOMP_WARNING("Pick %s not found", pickID.c_str());
			continue;
		}

		// Store the association from pick to origin.
		// NOTE Even invalid arrivals (low weight and invalid phase) are
		//      stored to enable a fast lookup when an amplitudes
		//      arrives. Otherwise a slow database access is needed to
		//      fetch origins for an amplitudes referencing a "disabled"
		//      pick.
		//      Furthermore we have to make sure that for each pickID
		//      a pick is stored in the cache to be able to track
		//      the expiration of cache lifetime.
		bind(pickID, origin);

		if ( !validArrival(arr, _minimumArrivalWeight) ) continue;

		SEISCOMP_DEBUG("arrival #%3d  pick='%s'", i, pickID.c_str());

		const DataModel::WaveformStreamID &wfid = pick->waveformID();
		const string &net = wfid.networkCode();
		const string &sta = wfid.stationCode();
		const string &loc = wfid.locationCode();
		string        cha = wfid.channelCode().substr(0,2);

		string strStream = net + "." + sta + "." + loc + "." + cha;
		PickStreamEntry &e = pickStreamMap[strStream];
		// When there is already a pick registered for this (abstract) stream
		// which has been picked earlier, ignore the current pick
		if ( e.first && e.first->time().value() < pick->time().value() ) {
			SEISCOMP_DEBUG("Already used pick for P phase");
			continue;
		}

		DataModel::SensorLocation *sloc = NULL;

		try {
			sloc = Client::Inventory::Instance()->getSensorLocation(pick.get());
		}
		catch ( ... ) {}

		if ( !sloc ) {
			SEISCOMP_WARNING("No sensor location meta data for pick %s",
			                 pick->publicID().c_str());
		}

		e.first = pick;
		e.second = LocationAndDistance(sloc, arr->distance());
	}

	for ( PickStreamMap::iterator it = pickStreamMap.begin(); it != pickStreamMap.end(); ++it ) {
		const string &pickID = it->second.first->publicID();
		DataModel::SensorLocation *loc = it->second.second.first;
		double distance = it->second.second.second;

		SEISCOMP_DEBUG("using pick %s", pickID.c_str());

		// Loop over amplitudes
		pair<StaAmpMap::iterator, StaAmpMap::iterator>
			itp = _ampl.equal_range(pickID);

		// Collect all amplitudes of certain type with highest priority
		map<string, const DataModel::Amplitude*> usedAmplitudes;
		
		for (StaAmpMap::iterator
		     it = itp.first; it != itp.second; ++it) {
			const DataModel::Amplitude *ampl  = ((*it).second).get();
			map<string, const DataModel::Amplitude*>::iterator amp_it;

			amp_it = usedAmplitudes.find(ampl->type());
			// New entry? Store amplitude
			if ( amp_it == usedAmplitudes.end() )
				usedAmplitudes[ampl->type()] = ampl;
			// If an amplitude for a particular type is stored already check
			// its priority and replace it
			else {
				const DataModel::Amplitude *stored_ampl = amp_it->second;
				if ( hasHigherPriority(ampl, stored_ampl) )
					amp_it->second = ampl;
			}

			// Highest priority has the amplitude which is already associated
			// if reprocessing is active
			if ( _allowReprocessing ) {
				bool ok = false;

				for ( size_t i = 0; i < origin->stationMagnitudeCount(); ++i ) {
					StaMag *stamag = origin->stationMagnitude(i);
					if ( stamag->type() == ampl->type()
					  && stamag->amplitudeID() == ampl->publicID() ) {
						amp_it->second = ampl;
						ok = true;
						break;
					}
				}

				if ( ok ) break;
			}
		}

		// Compute magnitudes of used amplitudes
		map<string, const DataModel::Amplitude*>::iterator amp_it;
		for ( amp_it = usedAmplitudes.begin(); amp_it != usedAmplitudes.end(); ++amp_it ) {
			const DataModel::Amplitude *ampl  = amp_it->second;
			const string &aid   = ampl->publicID();

			MagnitudeList mags;

			if ( !computeStationMagnitude(ampl, origin, loc, distance, depth, mags) )
				continue;

			for ( MagnitudeList::const_iterator it = mags.begin(); it != mags.end(); ++it ) {
				StaMagPtr stationMagnitude = getStationMagnitude(origin, ampl->waveformID(), it->proc->type(), it->value, _allowReprocessing);
				if ( stationMagnitude ) {
					it->proc->finalizeMagnitude(stationMagnitude.get());
					stationMagnitude->setAmplitudeID(aid);
					stationMagnitude->setPassedQC(it->passedQC);
					magTypes.insert(it->proc->type());
				}
			}
		}
	}

	// loop over all magnitude types found so far
	for (set<string>::iterator
	     it = magTypes.begin(); it != magTypes.end(); ++it) {

		const string &mtype = *it;
		bool newInstance;

		NetMagPtr netMag = getMagnitude(origin, mtype, &newInstance);
		if ( netMag ) {
			computeNetworkMagnitude(origin, mtype, netMag);
			if ( !newInstance ) {
				Time now = Time::GMT();
				try { netMag->creationInfo().setModificationTime(now); }
				catch ( ... ) {
					DataModel::CreationInfo ci;
					ci.setModificationTime(now);
					netMag->setCreationInfo(ci);
				}
				netMag->update();
				SCCoreApp->logObject(outputMagLog, now);
			}
		}
	}

	computeSummaryMagnitude(origin);
	dumpOrigin(origin);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagTool::processOriginUpdateOnly(DataModel::Origin* origin) {
	DataModel::Magnitude *netMag;
	DataModel::StationMagnitudeContribution *contrib;
	DataModel::StationMagnitude *staMag;
	DataModel::Amplitude *amp;
	DataModel::SensorLocation *sloc;
	DataModel::Arrival *arrival;

	bool consistent;
	double depth;
	double distance, az, baz;

	vector<double> mv, weights;

	Time now = Time::GMT();

	try { depth = origin->depth().value(); }
	catch ( ... ) {
		SEISCOMP_WARNING("%s: depth not set, ignoring origin",
		                 origin->publicID().c_str());
		return false;
	}

	for ( size_t i = 0; i < origin->magnitudeCount(); ++i ) {
		netMag = origin->magnitude(i);
		if ( _magTypes.find(netMag->type()) == _magTypes.end() )
			continue;

		if ( !_allowReprocessing ) {
			try {
				// Check if evaluation status is set
				netMag->evaluationStatus();
				SEISCOMP_INFO("Magnitude %s with type %s skipped as it has an evaluation status set",
				              netMag->publicID().c_str(), netMag->type().c_str());
				return NULL;
			}
			catch ( ... ) {}
		}

		AverageDescription averageMethod;
		AverageMethods::iterator am_it = _magnitudeAverageMethods.find(netMag->type());
		if ( am_it == _magnitudeAverageMethods.end() )
			averageMethod.type = Default;
		else
			averageMethod = am_it->second;

		// Parse the average method from the network magnitude
		averageMethodFromString(averageMethod, netMag->methodID());

		consistent = true;

		set<string> usedStationMagnitudes;

		// Check consistency
		for ( size_t j = 0; j < netMag->stationMagnitudeContributionCount(); ++j ) {
			contrib = netMag->stationMagnitudeContribution(j);
			staMag = DataModel::StationMagnitude::Find(contrib->stationMagnitudeID());
			if ( staMag == NULL ) {
				// Cannot update network magnitude
				SEISCOMP_WARNING("Station magnitude %s not found",
				                 contrib->stationMagnitudeID().c_str());
				consistent = false;
				break;
			}

			try {
				if ( !staMag->passedQC() )
					continue;
			}
			catch ( ... ) {}

			usedStationMagnitudes.insert(staMag->publicID());
		}

		if ( !consistent ) {
			SEISCOMP_ERROR("Incomplete information for magnitude %s: skipping update",
			               netMag->publicID().c_str());
			continue;
		}

		ProcessorList::iterator it = _processors.find(netMag->type());
		if ( it == _processors.end() ) {
			SEISCOMP_ERROR("No magnitude processor for type %s available: skipping update",
			               netMag->type().c_str());
			continue;
		}

		for ( size_t j = 0; j < netMag->stationMagnitudeContributionCount(); ++j ) {
			contrib = netMag->stationMagnitudeContribution(j);

			if ( usedStationMagnitudes.find(contrib->stationMagnitudeID()) == usedStationMagnitudes.end() )
				continue;

			staMag = DataModel::StationMagnitude::Find(contrib->stationMagnitudeID());

			amp = DataModel::Amplitude::Find(staMag->amplitudeID());
			if ( amp != NULL ) {
				// Figure out distance
				arrival = origin->arrival(amp->pickID());
				distance = -1;

				if ( arrival ) {
					try {
						distance = arrival->distance();
					}
					catch ( ... ) {}
				}

				Util::KeyValues *params = fetchParams(amp);

				Settings settings(
					SCCoreApp->configModuleName(),
					amp->waveformID().networkCode(),
					amp->waveformID().stationCode(),
					amp->waveformID().locationCode(),
					amp->waveformID().channelCode(),
					&SCCoreApp->configuration(),
					params);

				if ( !it->second->setup(settings))
					continue;

				try {
					sloc = Client::Inventory::Instance()->getSensorLocation(
						amp->waveformID().networkCode(),
						amp->waveformID().stationCode(),
						amp->waveformID().locationCode(),
						amp->timeWindow().reference());
				}
				catch ( ... ) {
					sloc = NULL;
				}

				if ( !sloc ) {
					SEISCOMP_WARNING("No sensor location meta data for amplitude %s",
					                 amp->publicID().c_str());
					continue;
				}

				if ( distance < 0 ) {
					try {
						// Compute distance
						Math::Geo::delazi(origin->latitude().value(), origin->longitude().value(),
						                  sloc->latitude(), sloc->longitude(),
						                  &distance, &az, &baz);
					}
					catch ( ... ) {
						continue;
					}
				}

				double period = 0;
				try { period = amp->period().value(); } catch ( ... ) {}

				double snr = 0;
				try { snr = amp->snr(); } catch ( ... ) {}

				double mag;
				MagnitudeProcessor::Status status =
					it->second->computeMagnitude(amp->amplitude().value(),
					                             amp->unit(),
					                             period, snr, distance, depth,
					                             origin, sloc, amp, mag);

				bool passedQC = true;

				if ( status != MagnitudeProcessor::OK ) {
					SEISCOMP_DEBUG("Magnitude failed: sid=%s.%s.%s.%s, type=%s, error=%s",
					                 amp->waveformID().networkCode().c_str(),
					                 amp->waveformID().stationCode().c_str(),
					                 amp->waveformID().locationCode().c_str(),
					                 amp->waveformID().channelCode().c_str(),
					                 amp->type().c_str(), status.toString());
					if ( !it->second->treatAsValidMagnitude() )
						continue;

					passedQC = false;
				}

				staMag->setMagnitude(mag);
				staMag->setPassedQC(passedQC);

				try { staMag->creationInfo().setModificationTime(now); }
				catch ( ... ) {
					DataModel::CreationInfo ci;
					ci.setModificationTime(now);
					staMag->setCreationInfo(ci);
				}

				staMag->update();
				SCCoreApp->logObject(outputMagLog, now);
			}

			mv.push_back(staMag->magnitude().value());

			try {
				weights.push_back(contrib->weight());
			}
			catch ( ... ) {
				weights.push_back(1.0);
			}
		}


		if ( !mv.empty() ) {
			string methodID;
			double val, stdev;

			if ( _keepWeights ) {
				// Use preset weights
				if ( !Math::Statistics::average(mv, weights, val, stdev) )
					return false;
			}
			else if ( !computeAverage(averageMethod, mv, weights, methodID, val, stdev) )
				return false;

			if ( stdev > _warningLevel ) {
				string mID = "mean with preset weights";
				if ( !_keepWeights )
					mID = methodID;
				SEISCOMP_WARNING("Exceedence of warning level - magnitude type: %s method: %s value: %.2f stdev: %.2f",
				                 netMag->type().c_str(), mID.c_str(), val, stdev);
			}

			netMag->setMagnitude(DataModel::RealQuantity(val, stdev, Core::None, Core::None, Core::None));
			netMag->setMethodID(methodID);

			// Update residuals
			size_t idx = 0;
			for ( size_t i = 0; i < netMag->stationMagnitudeContributionCount(); ++i ) {
				contrib = netMag->stationMagnitudeContribution(i);
				staMag = origin->findStationMagnitude(contrib->stationMagnitudeID());

				double oldWeight = -1, oldResidual = 0;
				double weight;
				double residual = staMag->magnitude().value() - val;
				bool hasResidual = false;

				try {
					oldWeight = contrib->weight();
				}
				catch ( ... ) {}

				try {
					oldResidual = contrib->residual();
					hasResidual = true;
				}
				catch ( ... ) {}

				if ( usedStationMagnitudes.find(contrib->stationMagnitudeID()) == usedStationMagnitudes.end() )
					weight = 0.0;
				else
					weight = weights[idx++];

				if ( oldWeight != weight
				  || oldResidual != residual
				  || !hasResidual ) {
					contrib->setWeight(weight);
					contrib->setResidual(residual);
					contrib->update();
					SEISCOMP_DEBUG("Updating magnitude reference for %s", staMag->publicID().c_str());
				}
			}
		}
		else if ( netMag->stationMagnitudeContributionCount() == 0 ) {
			// That does not work
			return false;
		}
		else {
			netMag->setMagnitude(DataModel::RealQuantity(INVALID_MAG));
			netMag->setMethodID(std::string());
			netMag->setEvaluationStatus(DataModel::EvaluationStatus(DataModel::REJECTED));

			for ( size_t i = 0; i < netMag->stationMagnitudeContributionCount(); ++i ) {
				contrib = netMag->stationMagnitudeContribution(i);
				try {
					contrib->residual();
					contrib->setResidual(Core::None);
					contrib->update();
				}
				catch ( ... ) {}
			}
		}

		try { netMag->creationInfo().setModificationTime(now); }
		catch ( ... ) {
			DataModel::CreationInfo ci;
			ci.setModificationTime(now);
			netMag->setCreationInfo(ci);
		}

		netMag->update();
		SCCoreApp->logObject(outputMagLog, now);
	}

	computeSummaryMagnitude(origin);
	dumpOrigin(origin);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagTool::feed(DataModel::Amplitude* ampl, bool update) {
	if ( SCCoreApp->isAgencyIDBlocked(objectAgencyID(ampl)) )
		return false;

	// Has a magnitude processor for this type been configured?
	if ( _processors.find(ampl->type()) == _processors.end() ) {
		return false;
	}

	if ( !_feed(ampl, update) ) return false;

	const std::string &pickID = ampl->pickID();

	OriginList *origins = originsForPick(pickID);

	if ( origins == NULL && SCCoreApp->query() ) {
		// No pick - origin information cached => read from database

		++_dbAccesses;

		origins = createBinding(pickID);

		// Disable generation of notifiers
		bool oldState = DataModel::Notifier::IsEnabled();
		DataModel::Notifier::Disable();

		std::list<DataModel::OriginPtr> reloadOrigins;

		// Here we can have a race condition. Imagine the following use case:
		// - a list of amplitudes is received in one message
		// - an origin with magnitudes witch reference those amplitudes is being
		//   send in another message
		// - the amplitude messages arrives here and we fetch all origins connected
		//   with an amplitude. the origin itself has not been arrived yet but is already
		//   stored partly in the database (without e.g. magnitudes). we fetch this
		//   origin from the database and start to compute magnitudes which are
		//   probably already part of that origin and will arrive with the next message.

		Core::Time now = Core::Time::GMT();

		DataModel::DatabaseIterator dbit = SCCoreApp->query()->getOriginsForAmplitude(ampl->publicID());
		for ( ; *dbit; ++dbit ) {
			DataModel::OriginPtr origin = DataModel::Origin::Cast(*dbit);
			if ( !origin ) continue;

			// Did we read the origin directly from DB?
			if ( !dbit.cached() ) {
				try {
					// If origin is not in cache but it was saved to database in less
					// than cachesize/2 ago we should have received it and thus it
					// should be in cache. So we ignore this origin and expect it with
					// a message some time in future.
					if ( (now - dbit.lastModified()) < Core::TimeSpan(_cacheSize*0.5) ) {
						SEISCOMP_DEBUG("ignore origin %s: expect its arrival via messaging soon",
						               origin->publicID().c_str());
						continue;
					}
				}
				catch ( ... ) {}

				_objectCache.feed(origin.get());
				reloadOrigins.push_back(origin);
				SEISCOMP_INFO("stored old origin %s in cache, size = %lu",
				              origin->publicID().c_str(), (unsigned long)_objectCache.size());
			}

			origins->push_back(origin);
		}
		dbit.close();

		for ( std::list<DataModel::OriginPtr>::iterator it = reloadOrigins.begin();
		      it != reloadOrigins.end(); ++it )
			SCCoreApp->query()->load(it->get());

		// Restore notifier state
		DataModel::Notifier::SetEnabled(oldState);
	}

	if ( origins == NULL ) {
		SEISCOMP_DEBUG("No historical origin to update");
		return true;
	}

	DataModel::SensorLocation *loc = NULL;

	try {
		loc = Client::Inventory::Instance()->getSensorLocation(
			ampl->waveformID().networkCode(),
			ampl->waveformID().stationCode(),
			ampl->waveformID().locationCode(),
			ampl->timeWindow().reference());
	}
	catch ( ... ) {
		SEISCOMP_WARNING("No sensor location meta data for amplitude %s",
		                 ampl->publicID().c_str());
	}

	for ( OriginList::iterator it = origins->begin(); it != origins->end(); ++it ) {
		DataModel::Origin *origin = it->get();

		if ( SCCoreApp->isAgencyIDBlocked(objectAgencyID(origin)) ) {
			SEISCOMP_DEBUG("Skipping historic origin '%s': agencyID '%s' is blocked",
			               origin->publicID().c_str(), objectAgencyID(origin).c_str());
			continue;
		}

		DataModel::Pick *pPick = NULL;
		DataModel::Arrival *arr = NULL;
		bool anotherFirst = false;
		bool invalidWeightArrival = false;

		for ( size_t i=0, arrivalCount = origin->arrivalCount();
		      i < arrivalCount; ++i ) {
			DataModel::Arrival *a = origin->arrival(i);

			DataModel::PickPtr p = _objectCache.get<DataModel::Pick>(a->pickID());
			if ( !p ) {
				SEISCOMP_WARNING("Pick %s not found -> skipping arrival %s[%lu]",
				                 a->pickID().c_str(), origin->publicID().c_str(), (unsigned long)i);
				continue;
			}

			if ( ampl->waveformID().networkCode() != p->waveformID().networkCode() ||
			     ampl->waveformID().stationCode() != p->waveformID().stationCode() ||
			     ampl->waveformID().locationCode() != p->waveformID().locationCode() )
				continue;

			if ( !validArrival(a, _minimumArrivalWeight) ) {
				invalidWeightArrival = true;
				continue;
			}

			if ( !pPick )
				pPick = p.get();
			else if ( p->time().value() < pPick->time().value() ) {
				// if another "first" pick is found, reset the arrival to use
				anotherFirst = true;
				arr = NULL;
				pPick = p.get();
			}

			// When the ID's match and the "first" p pick is the same as
			// the arrival pick then set the arrival to use
			if ( ampl->pickID() == a->pickID() ) {
				if ( pPick == p )
					arr = a;
				else {
					if ( pPick )
						SEISCOMP_WARNING("Pick %s found for Amplitude but another first P arrival %s has been found",
						                 a->pickID().c_str(),
						                 pPick->publicID().c_str());
					else
						SEISCOMP_WARNING("This should never happen");
				}
			}
		}

		if ( !arr ) {
			if ( !anotherFirst ) {
				if ( !invalidWeightArrival )
					SEISCOMP_WARNING("No matching arrival for pickID '%s' found, but Origin '%s' has been returned in query",
					                 ampl->pickID().c_str(), origin->publicID().c_str());
			}
			else
				SEISCOMP_INFO("There is another first P arrival than %s for amp %s", ampl->pickID().c_str(), ampl->publicID().c_str());
			continue;
		}

		const DataModel::WaveformStreamID &wfid = ampl->waveformID();
		double del, dep;

		try {
			del = arr->distance();
			dep = origin->depth().value();
		}
		catch ( GeneralException& e ) {
			SEISCOMP_ERROR("feed(Amplitude): %s", e.what());
			continue;
		}

		MagnitudeList mags;

		if ( !computeStationMagnitude(ampl, origin, loc, del, dep, mags) )
			continue;

		bool updateSummary = false;

		for ( MagnitudeList::const_iterator it = mags.begin(); it != mags.end(); ++it ) {
			StaMagPtr stationMagnitude = getStationMagnitude(origin, wfid, it->proc->type(), it->value, update);
			if ( stationMagnitude ) {
				it->proc->finalizeMagnitude(stationMagnitude.get());
				stationMagnitude->setAmplitudeID(ampl->publicID());
				stationMagnitude->setPassedQC(it->passedQC);

				const string &mtype = stationMagnitude->type();
				bool newInstance;
				NetMagPtr netMag = getMagnitude(origin, mtype, &newInstance);
				if ( netMag ) {
					computeNetworkMagnitude(origin, mtype, netMag);
					if ( ! newInstance ) netMag->update();

					SEISCOMP_INFO("feed(Amplitude): %s Magnitude '%s' for Origin '%s'",
					              newInstance?"created":"updated", mtype.c_str(), origin->publicID().c_str());

					dumpOrigin(origin);
					updateSummary = true;
				}
			}
		}

		if ( updateSummary )
			computeSummaryMagnitude(origin);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagTool::feed(DataModel::Origin *origin) {
	// This is the entry point for a new or updated Origin.
	// The Origin may be incomplete and if that is the case, we
	// must fetch the missing attributes from the DB before any
	// further processing.

	if ( SCCoreApp->isAgencyIDBlocked(objectAgencyID(origin)) ) {
		SEISCOMP_DEBUG("Skipping origin '%s': agencyID '%s' is blocked",
		               origin->publicID().c_str(), objectAgencyID(origin).c_str());
		return false;
	}

	DataModel::Origin *registered = DataModel::Origin::Find(origin->publicID());
	if ( registered && registered != origin ) {
		// We already read the origin from the database while processing
		// historical origins

		return processOrigin(registered);
	}

	if ( status(origin) == DataModel::REJECTED ) {
		SEISCOMP_INFO("Ignoring rejected origin %s", origin->publicID().c_str());
		return false;
	}

	// If this is an incomplete Origin without arrivals,
	// we first need to fetch the complete Origin from the DB
	if ( origin->arrivalCount() == 0 && SCCoreApp->query() ) {
		// get whole origin from DB
		SCCoreApp->query()->loadArrivals(origin);
	}

	// if still there are no arrivals in this origin,
	// we have to ignore it
	if (origin->arrivalCount() == 0) {
		SEISCOMP_INFO("Ignoring incomplete origin %s", origin->publicID().c_str());
		return false;
	}

	// Load missing magnitudes
	if ( origin->magnitudeCount() == 0 && SCCoreApp->query() ) {
		SCCoreApp->query()->loadMagnitudes(origin);
		for ( size_t i = 0; i < origin->magnitudeCount(); ++i )
			SCCoreApp->query()->load(origin->magnitude(i));
	}

	if ( origin->stationMagnitudeCount() == 0 && SCCoreApp->query() )
		SCCoreApp->query()->loadStationMagnitudes(origin);

	_objectCache.feed(origin);

	SEISCOMP_DEBUG("Inserted origin %s, cache size = %lu",
	               origin->publicID().c_str(), (unsigned long)_objectCache.size());

	return processOrigin(origin);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagTool::feed(DataModel::Pick *pick) {
	if ( SCCoreApp->isAgencyIDBlocked(objectAgencyID(pick)) )
		return false;

	const string &pickID = pick->publicID();

	_objectCache.feed(pick);

	// Create initial pick origin association
	createBinding(pickID);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagTool::publicObjectRemoved(DataModel::PublicObject* po) {
	bool saveState = DataModel::Notifier::IsEnabled();
	DataModel::Notifier::Disable();

	_ampl.erase(po->publicID());

	// Remove all pick - origin associations when a pick leaves the cache
	// to avoid incomplete cache
	OriginMap::iterator it = _orgs.find(po->publicID());
	if ( it != _orgs.end() ) _orgs.erase(it);

	DataModel::Notifier::SetEnabled(saveState);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagTool::_feed(DataModel::Amplitude *ampl, bool update) {
	const string &pickID = ampl->pickID();

	if ( pickID.empty() ) return false;

	DataModel::AmplitudePtr cached_ampl = _objectCache.get<DataModel::Amplitude>(ampl->publicID());
	if ( cached_ampl ) ampl = cached_ampl.get();

	pair<StaAmpMap::iterator, StaAmpMap::iterator>
		itp = _ampl.equal_range(pickID);

	for ( StaAmpMap::iterator it = itp.first; it != itp.second; ++it ) {
		const DataModel::Amplitude *existing = ((*it).second).get();
		if ( ampl->publicID() == existing->publicID() ) {
			if ( !update ) {
				SEISCOMP_WARNING("DUP amplitude '%s' ignored",
				                 ampl->publicID().c_str());
				return false;
			}
			else
				return true;
		}
	}

	_ampl.insert(StaAmpMap::value_type(pickID, ampl));

	// Make sure the referenced pick will be stored as well to be able to
	// remove the associated Amplitudes when the Pick is going to be removed from
	// the cache.
	_objectCache.get<DataModel::Pick>(pickID);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
