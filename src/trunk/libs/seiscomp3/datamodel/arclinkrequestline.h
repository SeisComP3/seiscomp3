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


#ifndef __SEISCOMP_DATAMODEL_ARCLINKREQUESTLINE_H__
#define __SEISCOMP_DATAMODEL_ARCLINKREQUESTLINE_H__


#include <seiscomp3/core/datetime.h>
#include <string>
#include <seiscomp3/datamodel/arclinkstatusline.h>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ArclinkRequestLine);

class ArclinkRequest;


class SC_SYSTEM_CORE_API ArclinkRequestLineIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ArclinkRequestLineIndex();
		ArclinkRequestLineIndex(Seiscomp::Core::Time start,
		                        Seiscomp::Core::Time end,
		                        const WaveformStreamID& streamID);

		//! Copy constructor
		ArclinkRequestLineIndex(const ArclinkRequestLineIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const ArclinkRequestLineIndex&) const;
		bool operator!=(const ArclinkRequestLineIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		Seiscomp::Core::Time start;
		Seiscomp::Core::Time end;
		WaveformStreamID streamID;
};


class SC_SYSTEM_CORE_API ArclinkRequestLine : public Object {
	DECLARE_SC_CLASS(ArclinkRequestLine);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ArclinkRequestLine();

		//! Copy constructor
		ArclinkRequestLine(const ArclinkRequestLine& other);

		//! Destructor
		~ArclinkRequestLine();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ArclinkRequestLine& operator=(const ArclinkRequestLine& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const ArclinkRequestLine& other) const;
		bool operator!=(const ArclinkRequestLine& other) const;

		//! Wrapper that calls operator==
		bool equal(const ArclinkRequestLine& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		void setEnd(Seiscomp::Core::Time end);
		Seiscomp::Core::Time end() const;

		void setStreamID(const WaveformStreamID& streamID);
		WaveformStreamID& streamID();
		const WaveformStreamID& streamID() const;

		void setRestricted(const OPT(bool)& restricted);
		bool restricted() const;

		void setShared(const OPT(bool)& shared);
		bool shared() const;

		void setNetClass(const std::string& netClass);
		const std::string& netClass() const;

		void setConstraints(const std::string& constraints);
		const std::string& constraints() const;

		void setStatus(const ArclinkStatusLine& status);
		ArclinkStatusLine& status();
		ArclinkStatusLine status() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const ArclinkRequestLineIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const ArclinkRequestLine* lhs) const;

	
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
		ArclinkRequestLineIndex _index;

		// Attributes
		OPT(bool) _restricted;
		OPT(bool) _shared;
		std::string _netClass;
		std::string _constraints;
		ArclinkStatusLine _status;
};


}
}


#endif
