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



#include <seiscomp3/processing/amplitudes/ML.h>
#include <seiscomp3/math/filter/seismometers.h>

#include <boost/bind.hpp>
#include <cstdio>


namespace Seiscomp {
namespace Processing {

namespace {

AmplitudeProcessor::AmplitudeValue average(
	const AmplitudeProcessor::AmplitudeValue &v0,
	const AmplitudeProcessor::AmplitudeValue &v1)
{
	AmplitudeProcessor::AmplitudeValue v;
	// Average both values
	v.value = (v0.value + v1.value) * 0.5;

	// Compute lower and upper uncertainty
	double v0l = v0.value;
	double v0u = v0.value;
	double v1l = v1.value;
	double v1u = v1.value;

	if ( v0.lowerUncertainty ) v0l -= *v0.lowerUncertainty;
	if ( v0.upperUncertainty ) v0u += *v0.upperUncertainty;
	if ( v1.lowerUncertainty ) v1l -= *v1.lowerUncertainty;
	if ( v1.upperUncertainty ) v1u += *v1.upperUncertainty;

	double l = 0, u = 0;

	l = std::max(l, v.value - v0l);
	l = std::max(l, v.value - v0u);
	l = std::max(l, v.value - v1l);
	l = std::max(l, v.value - v1u);

	u = std::max(l, v0l - v.value);
	u = std::max(l, v0u - v.value);
	u = std::max(l, v1l - v.value);
	u = std::max(l, v1u - v.value);

	v.lowerUncertainty = l;
	v.upperUncertainty = u;

	return v;
}


AmplitudeProcessor::AmplitudeTime average(
	const AmplitudeProcessor::AmplitudeTime &t0,
	const AmplitudeProcessor::AmplitudeTime &t1)
{
	AmplitudeProcessor::AmplitudeTime t;
	t.reference = Core::Time((double(t0.reference) + double(t1.reference)) * 0.5);

	// Compute lower and upper uncertainty
	Core::Time t0b = t0.reference + Core::TimeSpan(t0.begin);
	Core::Time t0e = t0.reference + Core::TimeSpan(t0.end);
	Core::Time t1b = t1.reference + Core::TimeSpan(t1.begin);
	Core::Time t1e = t1.reference + Core::TimeSpan(t1.end);

	Core::Time minTime = t.reference;
	Core::Time maxTime = t.reference;

	minTime = std::min(minTime, t0b);
	minTime = std::min(minTime, t0e);
	minTime = std::min(minTime, t1b);
	minTime = std::min(minTime, t1e);

	maxTime = std::max(maxTime, t0b);
	maxTime = std::max(maxTime, t0e);
	maxTime = std::max(maxTime, t1b);
	maxTime = std::max(maxTime, t1e);

	t.begin = (double)(minTime - t.reference);
	t.end = (double)(maxTime - t.reference);

	return t;
}

}


IMPLEMENT_SC_CLASS_DERIVED(AmplitudeProcessor_ML, AmplitudeProcessor, "AmplitudeProcessor_ML");
REGISTER_AMPLITUDEPROCESSOR(AmplitudeProcessor_ML, "ML");


AmplitudeProcessor_MLh::AmplitudeProcessor_MLh()
{
	_type = "ML";
}


AmplitudeProcessor_ML::AmplitudeProcessor_ML()
 : Processing::AmplitudeProcessor("ML") {
	setSignalEnd(150.);
	setMinSNR(0);
	// Maximum distance is 8 degrees
	setMaxDist(8);
	// Maximum depth is 80 km
	setMaxDepth(80);

	setUsedComponent(Horizontal);

	_combiner = TakeAverage;

	_ampN.setUsedComponent(FirstHorizontal);
	_ampE.setUsedComponent(SecondHorizontal);

	_ampE.setPublishFunction(boost::bind(&AmplitudeProcessor_ML::newAmplitude, this, _1, _2));
	_ampN.setPublishFunction(boost::bind(&AmplitudeProcessor_ML::newAmplitude, this, _1, _2));

	// Propagate configuration to single processors
	_ampN.setConfig(config());
	_ampE.setConfig(config());
}


const AmplitudeProcessor *AmplitudeProcessor_ML::componentProcessor(Component comp) const {
	switch ( comp ) {
		case FirstHorizontalComponent:
			return &_ampN;
		case SecondHorizontalComponent:
			return &_ampE;
		default:
			break;
	}

	return NULL;
}

const DoubleArray *AmplitudeProcessor_ML::processedData(Component comp) const {
	switch ( comp ) {
		case FirstHorizontalComponent:
			return _ampN.processedData(comp);
		case SecondHorizontalComponent:
			return _ampE.processedData(comp);
		default:
			break;
	}

	return NULL;
}


void AmplitudeProcessor_ML::reprocess(OPT(double) searchBegin, OPT(double) searchEnd) {
	setStatus(WaitingForData, 0);
	_ampN.setConfig(config());
	_ampE.setConfig(config());

	_results[0] = _results[1] = Core::None;

	_ampN.reprocess(searchBegin, searchEnd);
	_ampE.reprocess(searchBegin, searchEnd);

	if ( !isFinished() ) {
		if ( _ampN.status() > Finished )
			setStatus(_ampN.status(), _ampN.statusValue());
		else
			setStatus(_ampE.status(), _ampE.statusValue());
	}
}


int AmplitudeProcessor_ML::capabilities() const {
	return _ampN.capabilities() | Combiner;
}


AmplitudeProcessor::IDList
AmplitudeProcessor_ML::capabilityParameters(Capability cap) const {
	if ( cap == Combiner ) {
		IDList params;
		params.push_back("Average");
		params.push_back("Max");
		params.push_back("Min");
		return params;
	}

	return _ampN.capabilityParameters(cap);
}


bool AmplitudeProcessor_ML::setParameter(Capability cap, const std::string &value) {
	if ( cap == Combiner ) {
		if ( value == "Min" ) {
			_combiner = TakeMin;
			return true;
		}
		else if ( value == "Max" ) {
			_combiner = TakeMax;
			return true;
		}
		else if ( value == "Average" ) {
			_combiner = TakeAverage;
			return true;
		}

		return false;
	}

	_ampN.setParameter(cap, value);
	return _ampE.setParameter(cap, value);
}


bool AmplitudeProcessor_ML::setup(const Settings &settings) {
	// Copy the stream configurations (gain, orientation, responses, ...) to
	// the horizontal processors
	_ampN.streamConfig(FirstHorizontalComponent) = streamConfig(FirstHorizontalComponent);
	_ampE.streamConfig(SecondHorizontalComponent) = streamConfig(SecondHorizontalComponent);

	if ( !AmplitudeProcessor::setup(settings) ) return false;

	// Setup each component
	if ( !_ampN.setup(settings) || !_ampE.setup(settings) ) return false;

	return true;
}


void AmplitudeProcessor_ML::setTrigger(const Core::Time& trigger) {
	AmplitudeProcessor::setTrigger(trigger);
	_ampE.setTrigger(trigger);
	_ampN.setTrigger(trigger);
}


void AmplitudeProcessor_ML::computeTimeWindow() {
	// Copy configuration to each component
	_ampN.setConfig(config());
	_ampE.setConfig(config());

	_ampE.computeTimeWindow();
	_ampN.computeTimeWindow();
	setTimeWindow(_ampE.timeWindow() | _ampN.timeWindow());
}


double AmplitudeProcessor_ML::timeWindowLength(double distance_deg) const {
	double endN = _ampN.timeWindowLength(distance_deg);
	double endE = _ampE.timeWindowLength(distance_deg);
	_ampN.setSignalEnd(endN);
	_ampE.setSignalEnd(endE);
	return std::max(endN, endE);
}


void AmplitudeProcessor_ML::reset() {
	AmplitudeProcessor::reset();

	_results[0] = _results[1] = Core::None;

	_ampE.reset();
	_ampN.reset();
}


void AmplitudeProcessor_ML::close() {
	// TODO: Check for best available amplitude here
}


bool AmplitudeProcessor_ML::feed(const Record *record) {
	// Both processors finished already?
	if ( _ampE.isFinished() && _ampN.isFinished() ) return false;

	// Did an error occur?
	if ( status() > WaveformProcessor::Finished ) return false;

	if ( record->channelCode() == _streamConfig[FirstHorizontalComponent].code() ) {
		if ( !_ampN.isFinished() ) {
			_ampN.feed(record);
			if ( _ampN.status() == InProgress )
				setStatus(WaveformProcessor::InProgress, _ampN.statusValue());
			else if ( _ampN.isFinished() && _ampE.isFinished() ) {
				if ( !isFinished() ) {
					if ( _ampE.status() == Finished )
						setStatus(_ampN.status(), _ampN.statusValue());
					else
						setStatus(_ampE.status(), _ampE.statusValue());
				}
			}
		}
	}
	else if ( record->channelCode() == _streamConfig[SecondHorizontalComponent].code() ) {
		if ( !_ampE.isFinished() ) {
			_ampE.feed(record);
			if ( _ampE.status() == InProgress )
				setStatus(WaveformProcessor::InProgress, _ampE.statusValue());
			else if ( _ampE.isFinished() && _ampN.isFinished() ) {
				if ( !isFinished() ) {
					if ( _ampN.status() == Finished )
						setStatus(_ampE.status(), _ampE.statusValue());
					else
						setStatus(_ampN.status(), _ampN.statusValue());
				}
			}
		}
	}

	return true;
}


bool AmplitudeProcessor_ML::computeAmplitude(const DoubleArray &data,
                                             size_t i1, size_t i2,
                                             size_t si1, size_t si2,
                                             double offset,
                                             AmplitudeIndex *dt,
                                             AmplitudeValue *amplitude,
                                             double *period, double *snr) {
	return false;
}


void AmplitudeProcessor_ML::newAmplitude(const AmplitudeProcessor *proc,
                                         const AmplitudeProcessor::Result &res) {

	if ( isFinished() ) return;

	int idx = 0;

	if ( proc == &_ampE ) {
		idx = 0;
	}
	else if ( proc == &_ampN ) {
		idx = 1;
	}

	_results[idx] = ComponentResult();
	_results[idx]->value = res.amplitude;
	_results[idx]->time = res.time;

	if ( _results[0] && _results[1] ) {
		setStatus(Finished, 100.);
		Result newRes;
		newRes.record = res.record;

		switch ( _combiner ) {
			case TakeAverage:
				newRes.amplitude = average(_results[0]->value, _results[1]->value);
				newRes.time = average(_results[0]->time, _results[1]->time);
				newRes.component = Horizontal;
				break;
			case TakeMin:
				if ( _results[0]->value.value >= _results[1]->value.value ) {
					newRes.amplitude =  _results[0]->value;
					newRes.time = _results[0]->time;
					newRes.component = _ampE.usedComponent();
				}
				else {
					newRes.amplitude =  _results[1]->value;
					newRes.time = _results[1]->time;
					newRes.component = _ampN.usedComponent();
				}
				break;
			case TakeMax:
				if ( _results[0]->value.value <= _results[1]->value.value ) {
					newRes.amplitude =  _results[0]->value;
					newRes.time = _results[0]->time;
					newRes.component = _ampE.usedComponent();
				}
				else {
					newRes.amplitude =  _results[1]->value;
					newRes.time = _results[1]->time;
					newRes.component = _ampN.usedComponent();
				}
				break;
		};

		newRes.period = -1;
		newRes.snr = -1;
		emitAmplitude(newRes);
	}
}


}
}
