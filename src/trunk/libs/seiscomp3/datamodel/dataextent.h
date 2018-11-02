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


#ifndef __SEISCOMP_DATAMODEL_DATAEXTENT_H__
#define __SEISCOMP_DATAMODEL_DATAEXTENT_H__


#include <seiscomp3/core/datetime.h>
#include <vector>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/datamodel/datasegment.h>
#include <seiscomp3/datamodel/dataattributeextent.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(DataExtent);
DEFINE_SMARTPOINTER(DataSegment);
DEFINE_SMARTPOINTER(DataAttributeExtent);

class DataAvailability;


class SC_SYSTEM_CORE_API DataExtentIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DataExtentIndex();
		DataExtentIndex(const WaveformStreamID& waveformID);

		//! Copy constructor
		DataExtentIndex(const DataExtentIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const DataExtentIndex&) const;
		bool operator!=(const DataExtentIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		WaveformStreamID waveformID;
};


class SC_SYSTEM_CORE_API DataExtent : public PublicObject {
	DECLARE_SC_CLASS(DataExtent);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		DataExtent();

	public:
		//! Copy constructor
		DataExtent(const DataExtent& other);

		//! Constructor with publicID
		DataExtent(const std::string& publicID);

		//! Destructor
		~DataExtent();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static DataExtent* Create();
		static DataExtent* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static DataExtent* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		DataExtent& operator=(const DataExtent& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const DataExtent& other) const;
		bool operator!=(const DataExtent& other) const;

		//! Wrapper that calls operator==
		bool equal(const DataExtent& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setWaveformID(const WaveformStreamID& waveformID);
		WaveformStreamID& waveformID();
		const WaveformStreamID& waveformID() const;

		//! Time of first sample of data available
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! Time after last sample of data available
		void setEnd(Seiscomp::Core::Time end);
		Seiscomp::Core::Time end() const;

		//! The time of the last update or creation of this segment
		void setUpdated(Seiscomp::Core::Time updated);
		Seiscomp::Core::Time updated() const;

		//! The time of the last waveform archive scan
		void setLastScan(Seiscomp::Core::Time lastScan);
		Seiscomp::Core::Time lastScan() const;

		//! Flags the stream to be to fragmented for processing.
		void setSegmentOverflow(bool segmentOverflow);
		bool segmentOverflow() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const DataExtentIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const DataExtent* lhs) const;

	
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
		bool add(DataSegment* obj);
		bool add(DataAttributeExtent* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(DataSegment* obj);
		bool remove(DataAttributeExtent* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeDataSegment(size_t i);
		bool removeDataSegment(const DataSegmentIndex& i);
		bool removeDataAttributeExtent(size_t i);
		bool removeDataAttributeExtent(const DataAttributeExtentIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t dataSegmentCount() const;
		size_t dataAttributeExtentCount() const;

		//! Index access
		//! @return The object at index i
		DataSegment* dataSegment(size_t i) const;
		DataSegment* dataSegment(const DataSegmentIndex& i) const;

		DataAttributeExtent* dataAttributeExtent(size_t i) const;
		DataAttributeExtent* dataAttributeExtent(const DataAttributeExtentIndex& i) const;

		//! Find an object by its unique attribute(s)

		DataAvailability* dataAvailability() const;

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
		// Index
		DataExtentIndex _index;

		// Attributes
		Seiscomp::Core::Time _start;
		Seiscomp::Core::Time _end;
		Seiscomp::Core::Time _updated;
		Seiscomp::Core::Time _lastScan;
		bool _segmentOverflow;

		// Aggregations
		std::vector<DataSegmentPtr> _dataSegments;
		std::vector<DataAttributeExtentPtr> _dataAttributeExtents;

	DECLARE_SC_CLASSFACTORY_FRIEND(DataExtent);
};


}
}


#endif
