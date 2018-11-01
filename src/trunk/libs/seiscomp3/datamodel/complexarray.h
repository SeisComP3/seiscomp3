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


#ifndef __SEISCOMP_DATAMODEL_COMPLEXARRAY_H__
#define __SEISCOMP_DATAMODEL_COMPLEXARRAY_H__


#include <vector>
#include <complex>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ComplexArray);


class SC_SYSTEM_CORE_API ComplexArray : public Core::BaseObject {
	DECLARE_SC_CLASS(ComplexArray);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ComplexArray();

		//! Copy constructor
		ComplexArray(const ComplexArray& other);

		//! Destructor
		~ComplexArray();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ComplexArray& operator=(const ComplexArray& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const ComplexArray& other) const;
		bool operator!=(const ComplexArray& other) const;

		//! Wrapper that calls operator==
		bool equal(const ComplexArray& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setContent(const std::vector< std::complex<double> >&);
		const std::vector< std::complex<double> >& content() const;
		std::vector< std::complex<double> >& content();


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::vector< std::complex<double> > _content;
};


}
}


#endif
