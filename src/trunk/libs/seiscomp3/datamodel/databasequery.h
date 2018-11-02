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


#ifndef __SEISCOMP_DATAMODEL_DATABASE_QUERY_H__
#define __SEISCOMP_DATAMODEL_DATABASE_QUERY_H__



#include <seiscomp3/datamodel/originuncertainty.h>
#include <string>
#include <seiscomp3/datamodel/phase.h>
#include <seiscomp3/datamodel/complexarray.h>
#include <seiscomp3/datamodel/nodalplanes.h>
#include <seiscomp3/datamodel/arclinkstatusline.h>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/principalaxes.h>
#include <seiscomp3/datamodel/timewindow.h>
#include <seiscomp3/datamodel/realquantity.h>
#include <seiscomp3/datamodel/integerquantity.h>
#include <seiscomp3/datamodel/creationinfo.h>
#include <seiscomp3/datamodel/realarray.h>
#include <seiscomp3/datamodel/originquality.h>
#include <seiscomp3/datamodel/blob.h>
#include <seiscomp3/datamodel/arclinkrequestsummary.h>
#include <seiscomp3/datamodel/types.h>
#include <vector>
#include <seiscomp3/datamodel/tensor.h>
#include <seiscomp3/datamodel/timequantity.h>
#include <seiscomp3/datamodel/sourcetimefunction.h>
#include <seiscomp3/datamodel/databasereader.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(DatabaseQuery);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief Query class for the scheme packages
 *  This class implements the query interface and executes a query
 *  on a database.
 */
class SC_SYSTEM_CORE_API DatabaseQuery : public DatabaseReader {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		DatabaseQuery(Seiscomp::IO::DatabaseInterface* dbDriver);

		//! Destructor
		~DatabaseQuery();


	// ----------------------------------------------------------------------
	//  Query interface
	// ----------------------------------------------------------------------
	public:
		Station* getStation(const std::string& network_code,
		                    const std::string& station_code,
		                    Seiscomp::Core::Time time);

		/**
		 * Returns the Event referencing a particular Origin.
		 * @param originID The publicID of the Origin
		 * @return A pointer to the Event object
		 */
		Event* getEvent(const std::string& originID);

		/**
		 * Returns the Event having a particular preferredMagnitudeID.
		 * @param magnitudeID The publicID of the NetworkMagnitude
		 * @return A pointer to the Event object
		 */
		Event* getEventByPreferredMagnitudeID(const std::string& magnitudeID);

		/**
		 * Returns the Event referencing a particular FocalMechanism.
		 * @param focalMechanismID The publicID of the FocalMechanism
		 * @return A pointer to the Event object
		 */
		Event* getEventForFocalMechanism(const std::string& focalMechanismID);

		/**
		 * Returns the Event.
		 * @param eventID The publicID of the Event
		 * @return A pointer to the Event object
		 */
		Event* getEventByPublicID(const std::string& eventID);

		/**
		 * Returns an Amplitude of a particular type and
		 * references a certain Pick.
		 * @param pickID The referenced publicID of a Pick
		 * @param type The type of the StationAmplitude
		 * @return A pointer to the StationAmplitude object
		 */
		Amplitude* getAmplitude(const std::string& pickID,
		                        const std::string& type);

		/**
		 * Returns all Amplitudes in a given timewindow. As reference
		 * time
		 * for the Amplitude is timeWindow.reference used.
		 * @param startTime The starttime of the timewindow
		 * @param endTime The endtime of the timewindow
		 * @return An iterator to iterate over the amplitudes.
		 */
		DatabaseIterator getAmplitudes(Seiscomp::Core::Time startTime,
		                               Seiscomp::Core::Time endTime);

		/**
		 * Returns all Amplitudes referencing a certain Pick.
		 * @param pickID The referenced publicID of a Pick
		 * @return An iterator to iterate over the result set
		 */
		DatabaseIterator getAmplitudesForPick(const std::string& pickID);

		/**
		 * Returns all Amplitudes that are references by the Arrivals
		 * of an Origin.
		 * @param originID The publicID of the Origin
		 * @return An iterator to iterate over the result set
		 */
		DatabaseIterator getAmplitudesForOrigin(const std::string& originID);

		/**
		 * Returns all Origins where an assoziated pick is references
		 * by the
		 * AmplitudeID.
		 * @param amplitudeID The publicID of the Amplitude
		 * @return An iterator to iterate over the result set
		 */
		DatabaseIterator getOriginsForAmplitude(const std::string& amplitudeID);

		/**
		 * Returns all the origin that holds the given magnitude.
		 * @param magnitudeID The publicID of the magnitude
		 * @return A pointer to the Origin object
		 */
		Origin* getOriginByMagnitude(const std::string& magnitudeID);

		/**
		 * Returns all Arrivals where an assoziated pick is references
		 * by the
		 * AmplitudeID.
		 * @param amplitudeID The publicID of the Amplitude
		 * @return An iterator to iterate over the result set
		 */
		DatabaseIterator getArrivalsForAmplitude(const std::string& amplitudeID);

		/**
		 * Returns all Picks that are references by the Arrivals of an
		 * Origin.
		 * @param originID The publicID of the Origin
		 * @return An iterator to iterate over the result set
		 */
		DatabaseIterator getPicks(const std::string& originID);

		/**
		 * Returns all Picks in a given timewindow
		 * @param startTime The starttime of the timewindow
		 * @param endTime The endtime of the timewindow
		 * @return An iterator to iterate over the picks
		 */
		DatabaseIterator getPicks(Seiscomp::Core::Time startTime,
		                          Seiscomp::Core::Time endTime);

		/**
		 * Returns all Picks in a given timewindow for a given stream
		 * @param startTime The starttime of the timewindow
		 * @param endTime The endtime of the timewindow
		 * @param The waveformStreamID
		 * @return An iterator to iterate over the picks
		 */
		DatabaseIterator getPicks(Seiscomp::Core::Time startTime,
		                          Seiscomp::Core::Time endTime,
		                          const WaveformStreamID& waveformID);

		/**
		 * Returns current waveform quality reports or alerts
		 * (assuming the end time is not set).
		 * @param type report/alert
		 * @return An iterator to iterate over the result set
		 */
		DatabaseIterator getWaveformQuality(const std::string& type);

		/**
		 * Returns waveform quality reports in a given time window for
		 * a given streamID and parameter.
		 * @param streamID a WaveformStreamID
		 * @param parameter latency/delay/timing quality/gaps
		 * interval/gaps length/spikes interval/spikes amplitude/
		 * spikes count/offset/rms/availability
		 * @param startTime start time
		 * @param endTime end time
		 * @return an iterator to iterate over the WaveformQuality
		 * objects
		 */
		DatabaseIterator getWaveformQuality(const WaveformStreamID& waveformID,
		                                    const std::string& parameter,
		                                    Seiscomp::Core::Time startTime,
		                                    Seiscomp::Core::Time endTime);

		/**
		 * Returns waveform quality reports in a given time window for
		 * a given streamID and parameter.
		 * @param parameter latency/delay/timing quality/gaps
		 * interval/gaps length/spikes interval/spikes amplitude/
		 * spikes count/offset/rms/availability
		 * @param startTime start time
		 * @param endTime end time
		 * @return an iterator to iterate over the WaveformQuality
		 * objects
		 */
		DatabaseIterator getWaveformQuality(Seiscomp::Core::Time startTime,
		                                    Seiscomp::Core::Time endTime);

		/**
		 * Returns waveform quality of a certain type (report/alert)
		 * in a given time window for a given streamID and parameter.
		 * @param streamID a WaveformStreamID
		 * @param parameter latency/delay/timing quality/gaps
		 * interval/gaps length/spikes interval/spikes amplitude/
		 * spikes count/offset/rms/availability
		 * @param type report/alert
		 * @param startTime start time
		 * @param endTime end time
		 * @return an iterator to iterate over the WaveformQuality
		 * objects
		 */
		DatabaseIterator getWaveformQuality(const WaveformStreamID& waveformID,
		                                    const std::string& parameter,
		                                    const std::string& type,
		                                    Seiscomp::Core::Time startTime,
		                                    Seiscomp::Core::Time endTime);

		/**
		 * Returns WaveformQualities for a given waveformID, parameter
		 * and type, ordered by end date
		 * the youngest first.
		 * @param streamID a WaveformStreamID
		 * @param parameter latency/delay/timing quality/gaps
		 * interval/gaps
		 * length/spikes interval/spikes amplitude/ spikes
		 * count/offset/rms/availability
		 * @param type report/alert
		 * @return An iterator to iterate over the WaveformQualities
		 */
		DatabaseIterator getWaveformQualityDescending(const WaveformStreamID& waveformID,
		                                              const std::string& parameter,
		                                              const std::string& type);

		/**
		 * Returns outages for the given waveform stream ID and the
		 * given time window
		 * @param streamID a WaveformStreamID
		 * @param startTime start time
		 * @param endTime end time
		 * @return an iterator to iterate over the Outage objects
		 */
		DatabaseIterator getOutage(const WaveformStreamID& waveformID,
		                           Seiscomp::Core::Time startTime,
		                           Seiscomp::Core::Time endTime);

		/**
		 * Returns qclogs for the given waveform stream ID and the
		 * given time window
		 * @param streamID a WaveformStreamID
		 * @param startTime start time
		 * @param endTime end time
		 * @return an iterator to iterate over the QCLog objects
		 */
		DatabaseIterator getQCLog(const WaveformStreamID& waveformID,
		                          Seiscomp::Core::Time startTime,
		                          Seiscomp::Core::Time endTime);

		/**
		 * Returns preferred origins of events in a given time range
		 * @param startTime start time
		 * @param endTime end time
		 * @param referenceOriginID origin to exclude
		 * @return An iterator to iterate over the result set
		 */
		DatabaseIterator getPreferredOrigins(Seiscomp::Core::Time startTime,
		                                     Seiscomp::Core::Time endTime,
		                                     const std::string& referenceOriginID);

		/**
		 * Returns preferred magnitudes of events in a given time range
		 * @param startTime start time
		 * @param endTime end time
		 * @param referenceMagnitudeID magnitude to exclude
		 * @return An iterator to iterate over the result set
		 */
		DatabaseIterator getPreferredMagnitudes(Seiscomp::Core::Time startTime,
		                                        Seiscomp::Core::Time endTime,
		                                        const std::string& referenceMagnitudeID);

		/**
		 * Returns events in a given time range
		 * @param startTime start time
		 * @param endTime end time
		 * @return An iterator to iterate over the events
		 */
		DatabaseIterator getEvents(Seiscomp::Core::Time startTime,
		                           Seiscomp::Core::Time endTime);

		/**
		 * Returns origins for a given event ordered by creation date
		 * the oldest first.
		 * @param eventID The ID of the event
		 * @return An iterator to iterate over the origins
		 */
		DatabaseIterator getOrigins(const std::string& eventID);

		/**
		 * Returns origins for a given event ordered by creation date
		 * the youngest first.
		 * @param eventID The ID of the event
		 * @return An iterator to iterate over the origins
		 */
		DatabaseIterator getOriginsDescending(const std::string& eventID);

		/**
		 * Returns focal mechanisms for a given event ordered by
		 * creation date
		 * the youngest first.
		 * @param eventID The ID of the event
		 * @return An iterator to iterate over the focal mechanisms
		 */
		DatabaseIterator getFocalMechanismsDescending(const std::string& eventID);

		/**
		 * Returns the pickID's of all origins of an event with
		 * given publicID
		 * @param publicID The event's publicID (eventID)
		 * @return An iterator to iterator over the result ID's
		 * that are unique in the result set.
		 */
		DatabaseIterator getEventPickIDs(const std::string& publicID);

		/**
		 * Returns the pickID's of all origins of an event with
		 * given publicID and with weight greater a given weight
		 * @param publicID The event's publicID (eventID)
		 * @param weight The minimum weight for an arrival
		 * @return An iterator to iterator over the result ID's
		 * that are unique in the result set.
		 */
		DatabaseIterator getEventPickIDsByWeight(const std::string& publicID,
		                                         double weight);

		/**
		 * Returns the picks of all origins of an event with
		 * given eventID.
		 * @param eventID The event's publicID
		 * @return An iterator to iterate over the picks where the
		 * publicID is unique in the result set.
		 */
		DatabaseIterator getEventPicks(const std::string& eventID);

		/**
		 * Returns the picks of all origins of an event with
		 * given publicID and with weight greater a given weight
		 * @param publicID The event's publicID (eventID)
		 * @param weight The minimum weight for an arrival
		 * @return An iterator to iterator over the result ID's
		 * that are unique in the result set.
		 */
		DatabaseIterator getEventPicksByWeight(const std::string& publicID,
		                                       double weight);

		/**
		 * Returns the ConfigModule objects by name and state.
		 */
		DatabaseIterator getConfigModule(const std::string& name,
		                                 bool enabled);

		/**
		 */
		DatabaseIterator getEquivalentPick(const std::string& stationCode,
		                                   const std::string& networkCode,
		                                   const std::string& locationCode,
		                                   const std::string& channelCode,
		                                   Seiscomp::Core::Time startTime,
		                                   Seiscomp::Core::Time endTime);

		/**
		 * Returns all journal entries for a particular PublicObject.
		 */
		DatabaseIterator getJournal(const std::string& objectID);

		/**
		 * Returns all journal entries for a particular PublicObject
		 * related to an action.
		 */
		DatabaseIterator getJournalAction(const std::string& objectID,
		                                  const std::string& action);

		DatabaseIterator getArclinkRequestByStreamCode(Seiscomp::Core::Time startTime,
		                                               Seiscomp::Core::Time endTime,
		                                               const std::string& networkCode,
		                                               const std::string& stationCode,
		                                               const std::string& locationCode,
		                                               const std::string& channelCode,
		                                               const std::string& type);

		DatabaseIterator getArclinkRequestByRequestID(const std::string& requestID);

		DatabaseIterator getArclinkRequestByUserID(const std::string& userID,
		                                           Seiscomp::Core::Time startTime,
		                                           Seiscomp::Core::Time endTime,
		                                           const std::string& type);

		DatabaseIterator getArclinkRequestByTime(Seiscomp::Core::Time startTime,
		                                         Seiscomp::Core::Time endTime,
		                                         const std::string& type);

		DatabaseIterator getArclinkRequest(const std::string& userID,
		                                   Seiscomp::Core::Time startTime,
		                                   Seiscomp::Core::Time endTime,
		                                   const std::string& networkCode,
		                                   const std::string& stationCode,
		                                   const std::string& locationCode,
		                                   const std::string& channelCode,
		                                   const std::string& type,
		                                   const std::string& netClass);

		DatabaseIterator getArclinkRequestRestricted(const std::string& userID,
		                                             Seiscomp::Core::Time startTime,
		                                             Seiscomp::Core::Time endTime,
		                                             const std::string& networkCode,
		                                             const std::string& stationCode,
		                                             const std::string& locationCode,
		                                             const std::string& channelCode,
		                                             const std::string& type,
		                                             const std::string& netClass,
		                                             bool restricted);
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}


#endif
