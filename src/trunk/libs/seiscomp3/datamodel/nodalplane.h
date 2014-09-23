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


#ifndef __SEISCOMP_DATAMODEL_NODALPLANE_H__
#define __SEISCOMP_DATAMODEL_NODALPLANE_H__


#include <seiscomp3/datamodel/realquantity.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(NodalPlane);


/**
 * \brief This class describes a nodal plane using the attributes
 * \brief strike, dip, and
 * \brief rake. For a definition of the angles see Aki and Richards
 * \brief (1980).
 */
class SC_SYSTEM_CORE_API NodalPlane : public Core::BaseObject {
	DECLARE_SC_CLASS(NodalPlane);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		NodalPlane();

		//! Copy constructor
		NodalPlane(const NodalPlane& other);

		//! Destructor
		~NodalPlane();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		NodalPlane& operator=(const NodalPlane& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const NodalPlane& other) const;
		bool operator!=(const NodalPlane& other) const;

		//! Wrapper that calls operator==
		bool equal(const NodalPlane& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Strike angle of nodal plane in degrees.
		void setStrike(const RealQuantity& strike);
		RealQuantity& strike();
		const RealQuantity& strike() const;

		//! Dip angle of nodal plane in degrees.
		void setDip(const RealQuantity& dip);
		RealQuantity& dip();
		const RealQuantity& dip() const;

		//! Rake angle of nodal plane in degrees.
		void setRake(const RealQuantity& rake);
		RealQuantity& rake();
		const RealQuantity& rake() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		RealQuantity _strike;
		RealQuantity _dip;
		RealQuantity _rake;
};


}
}


#endif
