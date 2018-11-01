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


#ifndef __SEISCOMP_DATAMODEL_AMPLITUDE_H__
#define __SEISCOMP_DATAMODEL_AMPLITUDE_H__


#include <seiscomp3/datamodel/creationinfo.h>
#include <seiscomp3/datamodel/timewindow.h>
#include <string>
#include <seiscomp3/datamodel/types.h>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <vector>
#include <seiscomp3/datamodel/timequantity.h>
#include <seiscomp3/datamodel/realquantity.h>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Amplitude);
DEFINE_SMARTPOINTER(Comment);

class EventParameters;


/**
 * \brief This class represents a quantification of the waveform
 * \brief anomaly, usually
 * \brief a single amplitude measurement or a measurement of the
 * \brief visible signal
 * \brief duration for duration magnitudes.
 */
class SC_SYSTEM_CORE_API Amplitude : public PublicObject {
	DECLARE_SC_CLASS(Amplitude);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Amplitude();

	public:
		//! Copy constructor
		Amplitude(const Amplitude& other);

		//! Constructor with publicID
		Amplitude(const std::string& publicID);

		//! Destructor
		~Amplitude();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Amplitude* Create();
		static Amplitude* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Amplitude* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Amplitude& operator=(const Amplitude& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Amplitude& other) const;
		bool operator!=(const Amplitude& other) const;

		//! Wrapper that calls operator==
		bool equal(const Amplitude& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! String that describes the type of amplitude using the
		//! nomenclature
		//! from Storchak et al. (2003). Possible values include
		//! unspecified
		//! amplitude reading (A), amplitude reading for local
		//! magnitude (ML),
		//! amplitude reading for body wave magnitude (MB), amplitude
		//! reading
		//! for surface wave magnitude (MS), and time of visible end of
		//! record
		//! for duration magnitude (MD). It has a maximum length of 16
		//! characters.
		void setType(const std::string& type);
		const std::string& type() const;

		//! Measured amplitude value for the given waveformID. Note
		//! that this
		//! attribute can describe different physical quantities,
		//! depending on
		//! the type of the amplitude. These can be, e.g.,
		//! displacement, velocity,
		//! or a period. If the only amplitude information is a period,
		//! it has
		//! to specified here, not in the period attribute. The latter
		//! can be used
		//! if the amplitude measurement contains information on, e.g.,
		//! displacement and an additional period. Since the physical
		//! quantity
		//! described by this attribute is not fixed, the unit of
		//! measurement
		//! cannot be defined in advance. However, the quantity has to
		//! be
		//! specified in SI base units. The enumeration given in
		//! attribute unit
		//! provides the most likely units that could be needed here.
		//! For clarity, using the optional unit attribute is highly
		//! encouraged.
		void setAmplitude(const OPT(RealQuantity)& amplitude);
		RealQuantity& amplitude();
		const RealQuantity& amplitude() const;

		//! Description of the time window used for amplitude
		//! measurement.
		//! Recommended for duration magnitudes.
		void setTimeWindow(const OPT(TimeWindow)& timeWindow);
		TimeWindow& timeWindow();
		const TimeWindow& timeWindow() const;

		//! Dominant period in the timeWindow in case of amplitude
		//! measurements.
		//! Not used for duration magnitude. The unit is seconds.
		void setPeriod(const OPT(RealQuantity)& period);
		RealQuantity& period();
		const RealQuantity& period() const;

		//! Signal-to-noise ratio of the spectrogram at the location
		//! the amplitude was measured.
		void setSnr(const OPT(double)& snr);
		double snr() const;

		//! This attribute provides the most likely measurement units
		//! for the
		//! physical quantity described in the amplitude attribute.
		//! Possible values are specified as combinations of SI base
		//! units.
		void setUnit(const std::string& unit);
		const std::string& unit() const;

		//! Refers to the publicID of an associated Pick object.
		void setPickID(const std::string& pickID);
		const std::string& pickID() const;

		//! Identifies the waveform stream on which the amplitude was
		//! measured.
		void setWaveformID(const OPT(WaveformStreamID)& waveformID);
		WaveformStreamID& waveformID();
		const WaveformStreamID& waveformID() const;

		//! Identifies the filter or filter setup used for filtering
		//! the waveform stream referenced by waveformID.
		void setFilterID(const std::string& filterID);
		const std::string& filterID() const;

		void setMethodID(const std::string& methodID);
		const std::string& methodID() const;

		//! Scaling time for amplitude measurement.
		void setScalingTime(const OPT(TimeQuantity)& scalingTime);
		TimeQuantity& scalingTime();
		const TimeQuantity& scalingTime() const;

		//! Type of magnitude the amplitude measurement is used for.
		//! For valid
		//! values see class Magnitude. String value with a maximum
		//! length of
		//! 16 characters.
		void setMagnitudeHint(const std::string& magnitudeHint);
		const std::string& magnitudeHint() const;

		//! Evaluation mode of Amplitude.
		void setEvaluationMode(const OPT(EvaluationMode)& evaluationMode);
		EvaluationMode evaluationMode() const;

		//! CreationInfo for the Amplitude object.
		void setCreationInfo(const OPT(CreationInfo)& creationInfo);
		CreationInfo& creationInfo();
		const CreationInfo& creationInfo() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * Add an object.
		 * @param obj The object pointer
		 * @return true The object has been added
		 * @return false The object has not been added
		 *               because it already exists in the list
		 *               or it already has another parent
		 */
		bool add(Comment* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(Comment* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeComment(size_t i);
		bool removeComment(const CommentIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t commentCount() const;

		//! Index access
		//! @return The object at index i
		Comment* comment(size_t i) const;
		Comment* comment(const CommentIndex& i) const;

		//! Find an object by its unique attribute(s)

		EventParameters* eventParameters() const;

		//! Implement Object interface
		bool assign(Object* other);
		bool attachTo(PublicObject* parent);
		bool detachFrom(PublicObject* parent);
		bool detach();

		//! Creates a clone
		Object* clone() const;

		//! Implement PublicObject interface
		bool updateChild(Object* child);

		void accept(Visitor*);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _type;
		OPT(RealQuantity) _amplitude;
		OPT(TimeWindow) _timeWindow;
		OPT(RealQuantity) _period;
		OPT(double) _snr;
		std::string _unit;
		std::string _pickID;
		OPT(WaveformStreamID) _waveformID;
		std::string _filterID;
		std::string _methodID;
		OPT(TimeQuantity) _scalingTime;
		std::string _magnitudeHint;
		OPT(EvaluationMode) _evaluationMode;
		OPT(CreationInfo) _creationInfo;

		// Aggregations
		std::vector<CommentPtr> _comments;

	DECLARE_SC_CLASSFACTORY_FRIEND(Amplitude);
};


}
}


#endif
