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


#ifndef __SEISCOMP_DATAMODEL_ACCESS_H__
#define __SEISCOMP_DATAMODEL_ACCESS_H__


#include <seiscomp3/core/datetime.h>
#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Access);

class Routing;


class SC_SYSTEM_CORE_API AccessIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		AccessIndex();
		AccessIndex(const std::string& networkCode,
		            const std::string& stationCode,
		            const std::string& locationCode,
		            const std::string& streamCode,
		            const std::string& user,
		            Seiscomp::Core::Time start);

		//! Copy constructor
		AccessIndex(const AccessIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const AccessIndex&) const;
		bool operator!=(const AccessIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string networkCode;
		std::string stationCode;
		std::string locationCode;
		std::string streamCode;
		std::string user;
		Seiscomp::Core::Time start;
};


/**
 * \brief This type describes an ArcLink access rule
 */
class SC_SYSTEM_CORE_API Access : public Object {
	DECLARE_SC_CLASS(Access);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Access();

		//! Copy constructor
		Access(const Access& other);

		//! Destructor
		~Access();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Access& operator=(const Access& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Access& other) const;
		bool operator!=(const Access& other) const;

		//! Wrapper that calls operator==
		bool equal(const Access& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Network code
		void setNetworkCode(const std::string& networkCode);
		const std::string& networkCode() const;

		//! Station code (empty for any station)
		void setStationCode(const std::string& stationCode);
		const std::string& stationCode() const;

		//! Location code (empty for any location)
		void setLocationCode(const std::string& locationCode);
		const std::string& locationCode() const;

		//! Stream (Channel) code (empty for any stream)
		void setStreamCode(const std::string& streamCode);
		const std::string& streamCode() const;

		//! Username (e-mail) or part of it (must match the end)
		void setUser(const std::string& user);
		const std::string& user() const;

		//! Start of validity
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! End of validity
		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const AccessIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const Access* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Routing* routing() const;

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
		AccessIndex _index;

		// Attributes
		OPT(Seiscomp::Core::Time) _end;
};


}
}


#endif
