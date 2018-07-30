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


#ifndef __SEISCOMP_DATAMODEL_DATAATTRIBUTEEXTENT_H__
#define __SEISCOMP_DATAMODEL_DATAATTRIBUTEEXTENT_H__


#include <seiscomp3/core/datetime.h>
#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(DataAttributeExtent);

class DataExtent;


class SC_SYSTEM_CORE_API DataAttributeExtentIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DataAttributeExtentIndex();
		DataAttributeExtentIndex(double sampleRate,
		                         const std::string& quality);

		//! Copy constructor
		DataAttributeExtentIndex(const DataAttributeExtentIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const DataAttributeExtentIndex&) const;
		bool operator!=(const DataAttributeExtentIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		double sampleRate;
		std::string quality;
};


class SC_SYSTEM_CORE_API DataAttributeExtent : public Object {
	DECLARE_SC_CLASS(DataAttributeExtent);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DataAttributeExtent();

		//! Copy constructor
		DataAttributeExtent(const DataAttributeExtent& other);

		//! Destructor
		~DataAttributeExtent();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		DataAttributeExtent& operator=(const DataAttributeExtent& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const DataAttributeExtent& other) const;
		bool operator!=(const DataAttributeExtent& other) const;

		//! Wrapper that calls operator==
		bool equal(const DataAttributeExtent& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Time of first sample of data attribute extent.
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! Time after last sample of data attribute extent.
		void setEnd(Seiscomp::Core::Time end);
		Seiscomp::Core::Time end() const;

		//! Sample rate of the current data attribute extent.
		void setSampleRate(double sampleRate);
		double sampleRate() const;

		//! Quality indicator of current data attribute extent.
		void setQuality(const std::string& quality);
		const std::string& quality() const;

		//! The time of the last update or creation of this data
		//! attribute extent.
		void setUpdated(Seiscomp::Core::Time updated);
		Seiscomp::Core::Time updated() const;

		//! Number of data segments covered by this data attribute
		//! extent.
		void setSegmentCount(int segmentCount);
		int segmentCount() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const DataAttributeExtentIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const DataAttributeExtent* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		DataExtent* dataExtent() const;

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
		DataAttributeExtentIndex _index;

		// Attributes
		Seiscomp::Core::Time _start;
		Seiscomp::Core::Time _end;
		Seiscomp::Core::Time _updated;
		int _segmentCount;
};


}
}


#endif
