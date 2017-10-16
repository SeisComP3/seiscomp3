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


#ifndef __SEISCOMP_DATAMODEL_TIMEARRAY_H__
#define __SEISCOMP_DATAMODEL_TIMEARRAY_H__


#include <seiscomp3/core/datetime.h>
#include <vector>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(TimeArray);


class SC_SYSTEM_CORE_API TimeArray : public Core::BaseObject {
	DECLARE_SC_CLASS(TimeArray);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		TimeArray();

		//! Copy constructor
		TimeArray(const TimeArray& other);

		//! Destructor
		~TimeArray();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		TimeArray& operator=(const TimeArray& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const TimeArray& other) const;
		bool operator!=(const TimeArray& other) const;

		//! Wrapper that calls operator==
		bool equal(const TimeArray& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setContent(const std::vector< Seiscomp::Core::Time >&);
		const std::vector< Seiscomp::Core::Time >& content() const;
		std::vector< Seiscomp::Core::Time >& content();


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::vector< Seiscomp::Core::Time > _content;
};


}
}


#endif
