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


#ifndef __SEISCOMP_DATAMODEL_ARCLINKUSER_H__
#define __SEISCOMP_DATAMODEL_ARCLINKUSER_H__


#include <string>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ArclinkUser);

class ArclinkLog;


class SC_SYSTEM_CORE_API ArclinkUserIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ArclinkUserIndex();
		ArclinkUserIndex(const std::string& name,
		                 const std::string& email);

		//! Copy constructor
		ArclinkUserIndex(const ArclinkUserIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const ArclinkUserIndex&) const;
		bool operator!=(const ArclinkUserIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
		std::string email;
};


class SC_SYSTEM_CORE_API ArclinkUser : public PublicObject {
	DECLARE_SC_CLASS(ArclinkUser);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		ArclinkUser();

	public:
		//! Copy constructor
		ArclinkUser(const ArclinkUser& other);

		//! Constructor with publicID
		ArclinkUser(const std::string& publicID);

		//! Destructor
		~ArclinkUser();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static ArclinkUser* Create();
		static ArclinkUser* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static ArclinkUser* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		ArclinkUser& operator=(const ArclinkUser& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const ArclinkUser& other) const;
		bool operator!=(const ArclinkUser& other) const;

		//! Wrapper that calls operator==
		bool equal(const ArclinkUser& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setName(const std::string& name);
		const std::string& name() const;

		void setEmail(const std::string& email);
		const std::string& email() const;

		void setPassword(const std::string& password);
		const std::string& password() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const ArclinkUserIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const ArclinkUser* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		ArclinkLog* arclinkLog() const;

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
		ArclinkUserIndex _index;

		// Attributes
		std::string _password;

	DECLARE_SC_CLASSFACTORY_FRIEND(ArclinkUser);
};


}
}


#endif
