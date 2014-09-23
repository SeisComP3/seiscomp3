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

// This file was created by a source code generator.
// Do not modify the contents. Change the definition and run the generator
// again!


#ifndef __SEISCOMP_DATAMODEL_OUTAGE_H__
#define __SEISCOMP_DATAMODEL_OUTAGE_H__


#include <seiscomp3/core/datetime.h>
#include <string>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Outage);

class QualityControl;


class SC_SYSTEM_CORE_API OutageIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		OutageIndex();
		OutageIndex(const WaveformStreamID& waveformID,
		            Seiscomp::Core::Time start);

		//! Copy constructor
		OutageIndex(const OutageIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const OutageIndex&) const;
		bool operator!=(const OutageIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		WaveformStreamID waveformID;
		Seiscomp::Core::Time start;
};


class SC_SYSTEM_CORE_API Outage : public Object {
	DECLARE_SC_CLASS(Outage);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Outage();

		//! Copy constructor
		Outage(const Outage& other);

		//! Destructor
		~Outage();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Outage& operator=(const Outage& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Outage& other) const;
		bool operator!=(const Outage& other) const;

		//! Wrapper that calls operator==
		bool equal(const Outage& other) const;


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
		Seiscomp::Core::Time end() const throw(Seiscomp::Core::ValueException);


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const OutageIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const Outage* lhs) const;

	
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
		OutageIndex _index;

		// Attributes
		std::string _creatorID;
		Seiscomp::Core::Time _created;
		OPT(Seiscomp::Core::Time) _end;
};


}
}


#endif
