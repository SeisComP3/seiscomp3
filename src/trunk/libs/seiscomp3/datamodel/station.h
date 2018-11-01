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


#ifndef __SEISCOMP_DATAMODEL_STATION_H__
#define __SEISCOMP_DATAMODEL_STATION_H__


#include <seiscomp3/datamodel/blob.h>
#include <seiscomp3/core/datetime.h>
#include <vector>
#include <string>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/datamodel/sensorlocation.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Station);
DEFINE_SMARTPOINTER(Comment);
DEFINE_SMARTPOINTER(SensorLocation);

class Network;


class SC_SYSTEM_CORE_API StationIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		StationIndex();
		StationIndex(const std::string& code,
		             Seiscomp::Core::Time start);

		//! Copy constructor
		StationIndex(const StationIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const StationIndex&) const;
		bool operator!=(const StationIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string code;
		Seiscomp::Core::Time start;
};


/**
 * \brief This type describes a seismic station
 */
class SC_SYSTEM_CORE_API Station : public PublicObject {
	DECLARE_SC_CLASS(Station);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Station();

	public:
		//! Copy constructor
		Station(const Station& other);

		//! Constructor with publicID
		Station(const std::string& publicID);

		//! Destructor
		~Station();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Station* Create();
		static Station* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Station* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Station& operator=(const Station& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Station& other) const;
		bool operator!=(const Station& other) const;

		//! Wrapper that calls operator==
		bool equal(const Station& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Station code (50.03)
		void setCode(const std::string& code);
		const std::string& code() const;

		//! Start of station epoch in ISO datetime format
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! End of station epoch. Empty string if the station is open
		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const;

		//! Station description in ASCII (50.09)
		void setDescription(const std::string& description);
		const std::string& description() const;

		//! Station latitude (50.04)
		void setLatitude(const OPT(double)& latitude);
		double latitude() const;

		//! Station longitude (50.05)
		void setLongitude(const OPT(double)& longitude);
		double longitude() const;

		//! Station elevation (50.06)
		void setElevation(const OPT(double)& elevation);
		double elevation() const;

		//! Place where the station is located (UTF-8)
		void setPlace(const std::string& place);
		const std::string& place() const;

		//! Country where the station is located (UTF-8)
		void setCountry(const std::string& country);
		const std::string& country() const;

		//! Station affiliation (eg., GEOFON)
		void setAffiliation(const std::string& affiliation);
		const std::string& affiliation() const;

		//! Type of station (eg., VBB, SP)
		void setType(const std::string& type);
		const std::string& type() const;

		//! Archive/Datacenter ID (metadata authority)
		void setArchive(const std::string& archive);
		const std::string& archive() const;

		//! Internal network code, under which this station is archived
		void setArchiveNetworkCode(const std::string& archiveNetworkCode);
		const std::string& archiveNetworkCode() const;

		//! Whether the station is "restricted"
		void setRestricted(const OPT(bool)& restricted);
		bool restricted() const;

		//! Whether the metadata is synchronized with other datacenters
		void setShared(const OPT(bool)& shared);
		bool shared() const;

		//! Any notes
		void setRemark(const OPT(Blob)& remark);
		Blob& remark();
		const Blob& remark() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const StationIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const Station* lhs) const;

	
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
		bool add(SensorLocation* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(Comment* obj);
		bool remove(SensorLocation* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeComment(size_t i);
		bool removeComment(const CommentIndex& i);
		bool removeSensorLocation(size_t i);
		bool removeSensorLocation(const SensorLocationIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t commentCount() const;
		size_t sensorLocationCount() const;

		//! Index access
		//! @return The object at index i
		Comment* comment(size_t i) const;
		Comment* comment(const CommentIndex& i) const;

		SensorLocation* sensorLocation(size_t i) const;
		SensorLocation* sensorLocation(const SensorLocationIndex& i) const;

		//! Find an object by its unique attribute(s)
		SensorLocation* findSensorLocation(const std::string& publicID) const;

		Network* network() const;

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
		StationIndex _index;

		// Attributes
		OPT(Seiscomp::Core::Time) _end;
		std::string _description;
		OPT(double) _latitude;
		OPT(double) _longitude;
		OPT(double) _elevation;
		std::string _place;
		std::string _country;
		std::string _affiliation;
		std::string _type;
		std::string _archive;
		std::string _archiveNetworkCode;
		OPT(bool) _restricted;
		OPT(bool) _shared;
		OPT(Blob) _remark;

		// Aggregations
		std::vector<CommentPtr> _comments;
		std::vector<SensorLocationPtr> _sensorLocations;

	DECLARE_SC_CLASSFACTORY_FRIEND(Station);
};


}
}


#endif
