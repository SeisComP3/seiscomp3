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


#ifndef __SEISCOMP_DATAMODEL_PRINCIPALAXES_H__
#define __SEISCOMP_DATAMODEL_PRINCIPALAXES_H__


#include <seiscomp3/datamodel/axis.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(PrincipalAxes);


/**
 * \brief This class describes the principal axes of a double-couple
 * \brief moment tensor solution. tAxis and pAxis are required,
 * \brief while nAxis is optional.
 */
class SC_SYSTEM_CORE_API PrincipalAxes : public Core::BaseObject {
	DECLARE_SC_CLASS(PrincipalAxes);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		PrincipalAxes();

		//! Copy constructor
		PrincipalAxes(const PrincipalAxes& other);

		//! Destructor
		~PrincipalAxes();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		PrincipalAxes& operator=(const PrincipalAxes& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const PrincipalAxes& other) const;
		bool operator!=(const PrincipalAxes& other) const;

		//! Wrapper that calls operator==
		bool equal(const PrincipalAxes& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! T (tension) axis of a double-couple moment tensor solution.
		void setTAxis(const Axis& tAxis);
		Axis& tAxis();
		const Axis& tAxis() const;

		//! P (pressure) axis of a double-couple moment tensor solution.
		void setPAxis(const Axis& pAxis);
		Axis& pAxis();
		const Axis& pAxis() const;

		//! N (neutral) axis of a double-couple moment tensor solution.
		void setNAxis(const OPT(Axis)& nAxis);
		Axis& nAxis();
		const Axis& nAxis() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		Axis _tAxis;
		Axis _pAxis;
		OPT(Axis) _nAxis;
};


}
}


#endif
