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


#ifndef __SEISCOMP_DATAMODEL_SETUP_H__
#define __SEISCOMP_DATAMODEL_SETUP_H__


#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Setup);

class ConfigStation;


class SC_SYSTEM_CORE_API SetupIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SetupIndex();
		SetupIndex(const std::string& name);

		//! Copy constructor
		SetupIndex(const SetupIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const SetupIndex&) const;
		bool operator!=(const SetupIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
};


class SC_SYSTEM_CORE_API Setup : public Object {
	DECLARE_SC_CLASS(Setup);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Setup();

		//! Copy constructor
		Setup(const Setup& other);

		//! Destructor
		~Setup();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Setup& operator=(const Setup& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Setup& other) const;
		bool operator!=(const Setup& other) const;

		//! Wrapper that calls operator==
		bool equal(const Setup& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setName(const std::string& name);
		const std::string& name() const;

		void setParameterSetID(const std::string& parameterSetID);
		const std::string& parameterSetID() const;

		void setEnabled(bool enabled);
		bool enabled() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const SetupIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const Setup* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		ConfigStation* configStation() const;

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
		SetupIndex _index;

		// Attributes
		std::string _parameterSetID;
		bool _enabled;
};


}
}


#endif
