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


#ifndef __SEISCOMP_DATAMODEL_ROUTEARCLINK_H__
#define __SEISCOMP_DATAMODEL_ROUTEARCLINK_H__


#include <seiscomp3/core/datetime.h>
#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(RouteArclink);

class Route;


class SC_SYSTEM_CORE_API RouteArclinkIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		RouteArclinkIndex();
		RouteArclinkIndex(const std::string& address,
		                  Seiscomp::Core::Time start);

		//! Copy constructor
		RouteArclinkIndex(const RouteArclinkIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const RouteArclinkIndex&) const;
		bool operator!=(const RouteArclinkIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string address;
		Seiscomp::Core::Time start;
};


/**
 * \brief This type describes an ArcLink route (data source)
 */
class SC_SYSTEM_CORE_API RouteArclink : public Object {
	DECLARE_SC_CLASS(RouteArclink);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		RouteArclink();

		//! Copy constructor
		RouteArclink(const RouteArclink& other);

		//! Destructor
		~RouteArclink();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		RouteArclink& operator=(const RouteArclink& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const RouteArclink& other) const;
		bool operator!=(const RouteArclink& other) const;

		//! Wrapper that calls operator==
		bool equal(const RouteArclink& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Server address in ip:port format
		void setAddress(const std::string& address);
		const std::string& address() const;

		//! Start of data
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! End of data
		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const throw(Seiscomp::Core::ValueException);

		//! priority (1 is highest)
		void setPriority(const OPT(int)& priority);
		int priority() const throw(Seiscomp::Core::ValueException);


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const RouteArclinkIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const RouteArclink* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Route* route() const;

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
		RouteArclinkIndex _index;

		// Attributes
		OPT(Seiscomp::Core::Time) _end;
		OPT(int) _priority;
};


}
}


#endif
