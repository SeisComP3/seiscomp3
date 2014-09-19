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


#define SEISCOMP_COMPONENT SCQC
#include <seiscomp3/logging/log.h>

#include <seiscomp3/datamodel/waveformquality.h>
#include <seiscomp3/plugins/qc/qcbuffer.h>
#include <seiscomp3/plugins/qc/qcconfig.h>
#include <seiscomp3/qc/qcprocessor_spike.h>
#include "qcplugin_spike.h"

namespace Seiscomp {
namespace Applications {
namespace Qc {


using namespace std;
using namespace Seiscomp::Processing;
using namespace Seiscomp::DataModel;


#define REGISTERED_NAME "QcSpike"

IMPLEMENT_SC_CLASS_DERIVED(QcPluginSpike, QcPlugin, "QcPluginSpike");
ADD_SC_PLUGIN("Qc Parameter Spike", "GFZ Potsdam <seiscomp-devel@gfz-potsdam.de>", 0, 1, 0)
REGISTER_QCPLUGIN(QcPluginSpike, REGISTERED_NAME);


QcPluginSpike::QcPluginSpike(): QcPlugin() {
	_qcProcessor = new QcProcessorSpike();
	_qcProcessor->subscribe(this);

	_name = REGISTERED_NAME;
	_parameterNames.push_back("spikes interval");
	_parameterNames.push_back("spikes amplitude");
	_parameterNames.push_back("spikes count");
}

string QcPluginSpike::registeredName() const {
	return _name;
}

vector<string> QcPluginSpike::parameterNames() const {
	return _parameterNames;
}

void QcPluginSpike::generateReport(const QcBuffer* buf) const {
	if (buf->empty()) return;

	std::vector<double> mean = _mean(buf);
	std::vector<double> stdDev = _stdDev(buf, mean[0], mean[1]);

	SEISCOMP_DEBUG("%s spike count: %d   interval mean: %f   ampl mean: %f", _streamID.c_str(), (int)mean[2], mean[0], mean[1]);

	WaveformQuality* obj1 = new WaveformQuality();
	obj1->setWaveformID(getWaveformID(_streamID));
	obj1->setCreatorID(_app->creatorID());
	obj1->setCreated(Core::Time::GMT());
	obj1->setStart(buf->startTime());
	obj1->setEnd(buf->endTime());
	obj1->setType("report");
	obj1->setParameter(_parameterNames[0]);
	obj1->setValue(mean[0]);
	obj1->setLowerUncertainty(stdDev[0]);
	obj1->setUpperUncertainty(stdDev[0]);
	obj1->setWindowLength((double)buf->length());
	pushObject(Object::Cast(obj1));
	
	WaveformQuality* obj2 = new WaveformQuality();
	obj2->setWaveformID(getWaveformID(_streamID));
	obj2->setCreatorID(_app->creatorID());
	obj2->setCreated(Core::Time::GMT());
	obj2->setStart(buf->startTime());
	obj2->setEnd(buf->endTime());
	obj2->setType("report");
	obj2->setParameter(_parameterNames[1]);
	obj2->setValue(mean[1]);
	obj2->setLowerUncertainty(stdDev[1]);
	obj2->setUpperUncertainty(stdDev[1]);
	obj2->setWindowLength((double)buf->length());
	pushObject(Object::Cast(obj2));

	WaveformQuality* obj3 = new WaveformQuality();
	obj3->setWaveformID(getWaveformID(_streamID));
	obj3->setCreatorID(_app->creatorID());
	obj3->setCreated(Core::Time::GMT());
	obj3->setStart(buf->startTime());
	obj3->setEnd(buf->endTime());
	obj3->setType("report");
	obj3->setParameter(_parameterNames[2]);
	obj3->setValue(mean[2]);
	obj3->setLowerUncertainty(0.0);
	obj3->setUpperUncertainty(0.0);
	obj3->setWindowLength((double)buf->length());
	pushObject(Object::Cast(obj3));
}

void QcPluginSpike::generateAlert(const QcBuffer* shortBuffer, const QcBuffer* longBuffer) const {
	if (shortBuffer->empty() || longBuffer->empty()) return;

	std::vector<double> sta = _mean(shortBuffer);
	std::vector<double> staStdDev = _stdDev(shortBuffer, sta[0], sta[1]);
	std::vector<double> lta = _mean(longBuffer);
	
	double iRelative = 0.0;
	double lRelative = 0.0;
	
	//! HACK
	if (lta[0] != 0.0)
		iRelative = fabs((lta[0] - sta[0]) / lta[0] * 100.0);
	else if (sta[0] != 0.0)
		iRelative = 100.0;
	
	if (lta[1] != 0.0)
		lRelative = fabs((lta[1] - sta[1]) / lta[1] * 100.0);
	else if (sta[1] != 0.0)
		lRelative = 100.0;
	
	
	if (iRelative > *(_qcConfig->alertThresholds().begin())) { // HACK multi thresholds not yet implemented!
		WaveformQuality* obj1 = new WaveformQuality();
		obj1->setWaveformID(getWaveformID(_streamID));
		obj1->setCreatorID(_app->creatorID());
		obj1->setCreated(Core::Time::GMT());
		obj1->setStart(shortBuffer->startTime());
		obj1->setEnd(shortBuffer->endTime());
		obj1->setType("alert");
		obj1->setParameter(_parameterNames[0]);
		obj1->setValue(sta[0]);
		obj1->setLowerUncertainty(staStdDev[0]);
		obj1->setUpperUncertainty(staStdDev[0]);
		obj1->setWindowLength((double)shortBuffer->length());
		pushObject(Object::Cast(obj1));
	}

	if (lRelative > *(_qcConfig->alertThresholds().begin())) { // HACK multi thresholds not yet implemented!
		WaveformQuality* obj2 = new WaveformQuality();
		obj2->setWaveformID(getWaveformID(_streamID));
		obj2->setCreatorID(_app->creatorID());
		obj2->setCreated(Core::Time::GMT());
		obj2->setStart(shortBuffer->startTime());
		obj2->setEnd(shortBuffer->endTime());
		obj2->setType("alert");
		obj2->setParameter(_parameterNames[1]);
		obj2->setValue(sta[1]);
		obj2->setLowerUncertainty(staStdDev[1]);
		obj2->setUpperUncertainty(staStdDev[1]);
		obj2->setWindowLength((double)shortBuffer->length());
		pushObject(Object::Cast(obj2));
	}
}

std::vector<double> QcPluginSpike::_mean(const QcBuffer* buf) const {
	std::vector<double> returnVector(3);
	returnVector[0] = 0.0;
	returnVector[1] = 0.0;
	returnVector[2] = 0.0;
	
	if (buf->size() < 1) return returnVector;

	Core::Time lastSpikeTime;
	double iSum = 0.0;
	double aSum = 0.0;
	int count = 0;

	for (QcBuffer::const_iterator p = buf->begin(); p != buf->end(); p++) {
		Spikes spikes = boost::any_cast<Spikes>((*p)->parameter);
	
		for (Spikes::iterator s = spikes.begin(); s != spikes.end(); s++) {
			if (s->second > 1e12) //! DEBUG
				SEISCOMP_DEBUG("%f found spike @ %s", s->second, s->first.iso().c_str());
			if (count != 0.0) 
				iSum += (double)(s->first - lastSpikeTime);
			
			aSum += s->second;
			count++;
			lastSpikeTime = s->first;			
		}
	}

	if (count > 0) {
		if (count > 1)
			returnVector[0] = iSum / (count -1);
		returnVector[1] = aSum / count;
		returnVector[2] = (double)count;
	}

	return returnVector;
}

std::vector<double> QcPluginSpike::_stdDev(const QcBuffer* buf, double iMean, double aMean) const {
	std::vector<double> returnVector(2);
	returnVector[0] = 0.0;
	returnVector[1] = 0.0;
	
	if (buf->size() < 2) return returnVector;
	
	Core::Time lastSpikeTime;
	double iSum = 0.0;
	double aSum = 0.0;
	int count = 0;
	
	for (QcBuffer::const_iterator p = buf->begin(); p != buf->end(); p++) {
		Spikes spikes = boost::any_cast<Spikes>((*p)->parameter);
	
		for (Spikes::iterator s = spikes.begin(); s != spikes.end(); s++) {
			if (count != 0.0) 
				iSum += pow((double)((s->first - lastSpikeTime)) - iMean, 2);

			aSum += pow(s->second - aMean, 2);
			count++;
			lastSpikeTime = s->first;            
		}
	}

	if (count > 1) {
		if (count > 2) 
			returnVector[0] = sqrt(iSum / (count - 2));
		returnVector[1] = sqrt(aSum / (count - 1));
	}

	return returnVector;
}


}
}
}

