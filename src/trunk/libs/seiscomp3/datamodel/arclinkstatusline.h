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


#ifndef __SEISCOMP_DATAMODEL_ARCLINKSTATUSLINE_H__
#define __SEISCOMP_DATAMODEL_ARCLINKSTATUSLINE_H__


#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ArclinkStatusLine);

class ArclinkRequest;


class SC_SYSTEM_CORE_API ArclinkStatusLineIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ArclinkStatusLineIndex();
		ArclinkStatusLineIndex(const std::string& volumeID,
		                       const std::string& type,
		                       const std::string& status);

		//! Copy constructor
		ArclinkStatusLineIndex(const ArclinkStatusLineIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const ArclinkStatusLineIndex&) const;
		bool operator!=(const ArclinkStatusLineIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string volumeID;
		std::string type;
		std::string status;
};


class SC_SYSTEM_CORE_API ArclinkStatusLine : public Object {
	DECLARE_SC_CLASS(ArclinkStatusLine);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ArclinkStatusLine();

		//! Copy constructor
		ArclinkStatusLine(const ArclinkStatusLine& other);

		//! Destructor
		~ArclinkStatusLine();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ArclinkStatusLine& operator=(const ArclinkStatusLine& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const ArclinkStatusLine& other) const;
		bool operator!=(const ArclinkStatusLine& other) const;

		//! Wrapper that calls operator==
		bool equal(const ArclinkStatusLine& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setType(const std::string& type);
		const std::string& type() const;

		void setStatus(const std::string& status);
		const std::string& status() const;

		void setSize(const OPT(int)& size);
		int size() const;

		void setMessage(const std::string& message);
		const std::string& message() const;

		void setVolumeID(const std::string& volumeID);
		const std::string& volumeID() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const ArclinkStatusLineIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const ArclinkStatusLine* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		ArclinkRequest* arclinkRequest() const;

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
		ArclinkStatusLineIndex _index;

		// Attributes
		OPT(int) _size;
		std::string _message;
};


}
}


#endif
