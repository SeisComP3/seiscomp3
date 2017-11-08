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


#ifndef __SEISCOMP_DATAMODEL_COMMENT_H__
#define __SEISCOMP_DATAMODEL_COMMENT_H__


#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/creationinfo.h>
#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Comment);

class MomentTensor;
class FocalMechanism;
class Amplitude;
class Magnitude;
class StationMagnitude;
class Pick;
class Event;
class Origin;
class Parameter;
class ParameterSet;
class Stream;
class SensorLocation;
class Station;
class Network;


class SC_SYSTEM_CORE_API CommentIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		CommentIndex();
		CommentIndex(const std::string& id);

		//! Copy constructor
		CommentIndex(const CommentIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const CommentIndex&) const;
		bool operator!=(const CommentIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string id;
};


/**
 * \brief Comment holds information on comments to a resource as well
 * \brief as author and creation time information.
 */
class SC_SYSTEM_CORE_API Comment : public Object {
	DECLARE_SC_CLASS(Comment);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Comment();

		//! Copy constructor
		Comment(const Comment& other);

		//! Destructor
		~Comment();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Comment& operator=(const Comment& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Comment& other) const;
		bool operator!=(const Comment& other) const;

		//! Wrapper that calls operator==
		bool equal(const Comment& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Text of comment.
		void setText(const std::string& text);
		const std::string& text() const;

		//! Identifier of comment, possibly in QuakeML RI format.
		void setId(const std::string& id);
		const std::string& id() const;

		//! Start of epoch in ISO datetime format
		void setStart(const OPT(Seiscomp::Core::Time)& start);
		Seiscomp::Core::Time start() const;

		//! End of epoch (empty if the comment epoch is open)
		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const;

		//! CreationInfo for the Comment object.
		void setCreationInfo(const OPT(CreationInfo)& creationInfo);
		CreationInfo& creationInfo();
		const CreationInfo& creationInfo() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const CommentIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const Comment* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * The following methods return the parent object by type.
		 * Because different parent types are possible, just one
		 * of these methods will return a valid pointer at a time.
		 */
		MomentTensor* momentTensor() const;
		FocalMechanism* focalMechanism() const;
		Amplitude* amplitude() const;
		Magnitude* magnitude() const;
		StationMagnitude* stationMagnitude() const;
		Pick* pick() const;
		Event* event() const;
		Origin* origin() const;
		Parameter* parameter() const;
		ParameterSet* parameterSet() const;
		Stream* stream() const;
		SensorLocation* sensorLocation() const;
		Station* station() const;
		Network* network() const;

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
		CommentIndex _index;

		// Attributes
		std::string _text;
		OPT(Seiscomp::Core::Time) _start;
		OPT(Seiscomp::Core::Time) _end;
		OPT(CreationInfo) _creationInfo;
};


}
}


#endif
