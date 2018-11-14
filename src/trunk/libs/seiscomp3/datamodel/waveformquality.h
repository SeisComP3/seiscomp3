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


#ifndef __SEISCOMP_DATAMODEL_WAVEFORMQUALITY_H__
#define __SEISCOMP_DATAMODEL_WAVEFORMQUALITY_H__


#include <seiscomp3/core/datetime.h>
#include <string>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(WaveformQuality);

class QualityControl;


class SC_SYSTEM_CORE_API WaveformQualityIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		WaveformQualityIndex();
		WaveformQualityIndex(Seiscomp::Core::Time start,
		                     const WaveformStreamID& waveformID,
		                     const std::string& type,
		                     const std::string& parameter);

		//! Copy constructor
		WaveformQualityIndex(const WaveformQualityIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const WaveformQualityIndex&) const;
		bool operator!=(const WaveformQualityIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		Seiscomp::Core::Time start;
		WaveformStreamID waveformID;
		std::string type;
		std::string parameter;
};


class SC_SYSTEM_CORE_API WaveformQuality : public Object {
	DECLARE_SC_CLASS(WaveformQuality);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		WaveformQuality();

		//! Copy constructor
		WaveformQuality(const WaveformQuality& other);

		//! Destructor
		~WaveformQuality();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		WaveformQuality& operator=(const WaveformQuality& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const WaveformQuality& other) const;
		bool operator!=(const WaveformQuality& other) const;

		//! Wrapper that calls operator==
		bool equal(const WaveformQuality& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setWaveformID(const WaveformStreamID& waveformID);
		WaveformStreamID& waveformID();
		const WaveformStreamID& waveformID() const;

		void setCreatorID(const std::string& creatorID);
		const std::string& creatorID() const;

		void setCreated(Seiscomp::Core::Time created);
		Seiscomp::Core::Time created() const;

		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const;

		void setType(const std::string& type);
		const std::string& type() const;

		void setParameter(const std::string& parameter);
		const std::string& parameter() const;

		void setValue(double value);
		double value() const;

		void setLowerUncertainty(const OPT(double)& lowerUncertainty);
		double lowerUncertainty() const;

		void setUpperUncertainty(const OPT(double)& upperUncertainty);
		double upperUncertainty() const;

		void setWindowLength(const OPT(double)& windowLength);
		double windowLength() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const WaveformQualityIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const WaveformQuality* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		QualityControl* qualityControl() const;

		//! Implement Object interface
		bool assign(Object* other);
		bool attachTo(PublicObject* parent);
		bool detachFrom(PublicObject* parent);
		bool detach();

		//! Creates a clone
		Object* clone() const;

		void accept(Visitor*);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Index
		WaveformQualityIndex _index;

		// Attributes
		std::string _creatorID;
		Seiscomp::Core::Time _created;
		OPT(Seiscomp::Core::Time) _end;
		double _value;
		OPT(double) _lowerUncertainty;
		OPT(double) _upperUncertainty;
		OPT(double) _windowLength;
};


}
}


#endif
