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


#ifndef __SEISCOMP_DATAMODEL_REALARRAY_H__
#define __SEISCOMP_DATAMODEL_REALARRAY_H__


#include <vector>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(RealArray);


class SC_SYSTEM_CORE_API RealArray : public Core::BaseObject {
	DECLARE_SC_CLASS(RealArray);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		RealArray();

		//! Copy constructor
		RealArray(const RealArray& other);

		//! Destructor
		~RealArray();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		RealArray& operator=(const RealArray& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const RealArray& other) const;
		bool operator!=(const RealArray& other) const;

		//! Wrapper that calls operator==
		bool equal(const RealArray& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setContent(const std::vector< double >&);
		const std::vector< double >& content() const;
		std::vector< double >& content();


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::vector< double > _content;
};


}
}


#endif
