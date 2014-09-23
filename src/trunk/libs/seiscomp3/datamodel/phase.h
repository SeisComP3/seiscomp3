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


#ifndef __SEISCOMP_DATAMODEL_PHASE_H__
#define __SEISCOMP_DATAMODEL_PHASE_H__


#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Phase);


/**
 * \brief Generic and extensible phase description that currently
 * \brief contains the phase code only.
 */
class SC_SYSTEM_CORE_API Phase : public Core::BaseObject {
	DECLARE_SC_CLASS(Phase);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Phase();

		//! Copy constructor
		Phase(const Phase& other);

		//! Custom constructor
		Phase(const std::string& code);

		//! Destructor
		~Phase();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		operator std::string&();
		operator const std::string&() const;

		//! Copies the metadata of other to this
		Phase& operator=(const Phase& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Phase& other) const;
		bool operator!=(const Phase& other) const;

		//! Wrapper that calls operator==
		bool equal(const Phase& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Phase code as given in the IASPEI Standard Seismic Phase
		//! List
		//! (Storchak et al. 2003). String with a maximum length of 32
		//! characters.
		void setCode(const std::string& code);
		const std::string& code() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _code;
};


}
}


#endif
