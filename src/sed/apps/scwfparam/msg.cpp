/***************************************************************************
 *   Copyright (C) by ETHZ/SED, GNS New Zealand, GeoScience Australia      *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Developed by gempa GmbH                                               *
 ***************************************************************************/


#define SEISCOMP_COMPONENT WfParam
#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/application.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/eventparameters_package.h>
#include <seiscomp3/datamodel/strongmotion/strongmotionparameters_package.h>
#include <seiscomp3/io/archive/xmlarchive.h>

#include "msg.h"


using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::DataModel::StrongMotion;


typedef pair<bool, int> FilterType;
typedef pair<FilterType, Seiscomp::FilterFreqs> FilterDef;


void addFilterParam(SimpleFilter *f, const char *name, double value) {
	FilterParameterPtr p = new FilterParameter;
	p->setName(name);
	p->setValue(RealQuantity(value));
	f->add(p.get());
}


SimpleFilter *createFilter(StrongMotionParameters *smp, const FilterDef &def) {
	if ( def.first.second <= 0 ) return NULL;

	if ( def.second.first > 0 && def.second.second > 0 ) {
		SimpleFilter *f = SimpleFilter::Create();
		if ( def.first.first )
			f->setType("acausal_bandpass");
		else
			f->setType("causal_bandpass");
		smp->add(f);

		addFilterParam(f, "order", def.first.second);
		addFilterParam(f, "fmin", def.second.first);
		addFilterParam(f, "fmax", def.second.second);

		return f;
	}
	else if ( def.second.first > 0 ) {
		SimpleFilter *f = SimpleFilter::Create();
		if ( def.first.first )
			f->setType("acausal_hipass");
		else
			f->setType("causal_hipass");
		smp->add(f);

		addFilterParam(f, "order", def.first.second);
		addFilterParam(f, "fmin", def.second.first);

		return f;
	}
	else if ( def.second.second > 0 ) {
		SimpleFilter *f = SimpleFilter::Create();
		if ( def.first.first )
			f->setType("acausal_lowpass");
		else
			f->setType("causal_lowpass");
		smp->add(f);

		addFilterParam(f, "order", def.first.second);
		addFilterParam(f, "fmax", def.second.second);

		return f;
	}

	return NULL;
}


void flushMessage(Seiscomp::Communication::Connection *con) {
	NotifierMessagePtr msg = Notifier::GetMessage();
	if ( con != NULL ) {
		con->send(msg.get());
		SEISCOMP_DEBUG("Flushing message with %d notifiers", (int)msg->size());
	}
}


void checkAndSendMessage(Seiscomp::Communication::Connection *con) {
	if ( Notifier::Size() > 100 )
		flushMessage(con);
}


bool sendMessages(Seiscomp::Communication::Connection *con,
                  Event *evt, Origin *org, Magnitude *mag,
                  const Seiscomp::StationMap &results) {
	Seiscomp::StationMap::const_iterator sit;
	Seiscomp::StationResults::const_iterator rit;

	CreationInfo ci;
	ci.setAgencyID(SCCoreApp->agencyID());
	ci.setAuthor(SCCoreApp->author());
	ci.setCreationTime(Time::GMT());

	StrongMotionParameters smp;

	bool saveNotifierState = Notifier::IsEnabled();
	Notifier::SetEnabled(true);

	StrongOriginDescriptionPtr smd = StrongOriginDescription::Create();
	smd->setOriginID(org->publicID());
	smd->setCreationInfo(ci);

	smp.add(smd.get());

	map<FilterDef, string> filterCache;
	map<FilterDef, string>::iterator filterCacheIt;

	for ( sit = results.begin(); sit != results.end(); ++sit ) {
		for ( rit = sit->second.begin(); rit != sit->second.end(); ++rit ) {
			Seiscomp::PGAVResult *pgavResult = *rit;

			// Already stored?
			if ( pgavResult->recordID.empty() ) {
				RecordPtr rec = Record::Create();
				rec->setCreationInfo(ci);
				rec->setWaveformID(pgavResult->streamID);

				if ( pgavResult->isVelocity )
					rec->setGainUnit("M/S");
				else
					rec->setGainUnit("M/S**2");

				rec->setDuration(pgavResult->duration);

				// waveformFile
				if ( !pgavResult->filename.empty() ) {
					FileResource file;
					file.setFilename(pgavResult->filename);
					file.setCreationInfo(ci);
					file.setType("MSEED");
					rec->setWaveformFile(file);
				}

				// Add record
				smp.add(rec.get());

				// Create filter stages
				int filterSeqNo = 0;
				string filterID;

				FilterDef scfdef(FilterType(pgavResult->isAcausal,
				                            pgavResult->pdFilterOrder),
				                            pgavResult->pdFilter);

				filterCacheIt = filterCache.find(scfdef);

				if ( filterCacheIt != filterCache.end() )
					filterID = filterCacheIt->second;
				// Add sensitivity correction filter stage (if available)
				else {
					SimpleFilterPtr f = createFilter(&smp, scfdef);
					if ( f ) {
						filterID = f->publicID();
						filterCache[scfdef] = filterID;
					}
					else
						filterID.clear();
				}

				if ( !filterID.empty() ) {
					SimpleFilterChainMemberPtr filterStage =
						new SimpleFilterChainMember;
					filterStage->setSequenceNo(filterSeqNo++);
					filterStage->setSimpleFilterID(filterID);

					rec->add(filterStage.get());
				}


				FilterDef fdef(FilterType(pgavResult->isAcausal,
				                          pgavResult->filterOrder),
				               pgavResult->filter);

				filterCacheIt = filterCache.find(fdef);

				if ( filterCacheIt != filterCache.end() )
					filterID = filterCacheIt->second;
				// Add second filter stage (if available)
				else {
					SimpleFilterPtr f = createFilter(&smp, fdef);
					if ( f ) {
						filterID = f->publicID();
						filterCache[fdef] = filterID;
					}
					else
						filterID.clear();
				}

				if ( !filterID.empty() ) {
					SimpleFilterChainMemberPtr filterStage =
						new SimpleFilterChainMember;
					filterStage->setSequenceNo(filterSeqNo++);
					filterStage->setSimpleFilterID(filterID);

					rec->add(filterStage.get());
				}

				rec->setStartTime(TimeQuantity(pgavResult->startTime));

				PeakMotionPtr peakMotion;

				peakMotion = new PeakMotion;
				peakMotion->setType("pga");
				peakMotion->setMotion(RealQuantity(pgavResult->pga));
				rec->add(peakMotion.get());

				peakMotion = new PeakMotion;
				peakMotion->setType("pgv");
				peakMotion->setMotion(RealQuantity(pgavResult->pgv));
				rec->add(peakMotion.get());

				Seiscomp::Processing::PGAV::ResponseSpectra::const_iterator rit;
				for ( rit = pgavResult->responseSpectra.begin();
				      rit != pgavResult->responseSpectra.end(); ++rit ) {

					for ( size_t i = 0; i < rit->second.size(); ++i ) {
						// Ignore PGA and PGV values
						if ( rit->second[i].period <= 0 ) continue;

						peakMotion = new PeakMotion;
						peakMotion->setType("psa");
						peakMotion->setDamping(rit->first);
						peakMotion->setMotion(RealQuantity(rit->second[i].psa));
						peakMotion->setPeriod(rit->second[i].period);
						rec->add(peakMotion.get());

						peakMotion = new PeakMotion;
						peakMotion->setType("drs");
						peakMotion->setDamping(rit->first);
						peakMotion->setMotion(RealQuantity(rit->second[i].sd));
						peakMotion->setPeriod(rit->second[i].period);
						rec->add(peakMotion.get());
					}
				}

				// Store record ID to reuse it in later revisions
				pgavResult->recordID = rec->publicID();
			}

			checkAndSendMessage(con);

			EventRecordReferencePtr ref = new EventRecordReference;
			ref->setRecordID(pgavResult->recordID);

			double len = (double)(pgavResult->trigger - pgavResult->startTime);
			ref->setPreEventLength(len);

			if ( pgavResult->duration ) {
				len = (double)(pgavResult->endTime - pgavResult->trigger) - *pgavResult->duration;
				ref->setPostEventLength(len);
			}

			smd->add(ref.get());
		}
	}

	smd->setWaveformCount(smd->eventRecordReferenceCount());
	smd->update();

	flushMessage(con);

	Notifier::SetEnabled(saveNotifierState);

	// XML Output if offline
	if ( con == NULL ) {
		StrongMotionParameters *smp_ptr = &smp;

		Seiscomp::IO::XMLArchive ar;
		ar.setFormattedOutput(true);
		ar.create("-");
		ar << smp_ptr;
		ar.close();
		cout.flush();
	}

	return true;
}
