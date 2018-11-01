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


#ifndef __SEISCOMP_DATAMODEL_AXIS_H__
#define __SEISCOMP_DATAMODEL_AXIS_H__


#include <seiscomp3/datamodel/realquantity.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Axis);


/**
 * \brief This class describes an eigenvector of a moment tensor
 * \brief expressed in its
 * \brief principal-axes system. It uses the angles azimuth, plunge,
 * \brief and the
 * \brief eigenvalue length.
 */
class SC_SYSTEM_CORE_API Axis : public Core::BaseObject {
	DECLARE_SC_CLASS(Axis);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Axis();

		//! Copy constructor
		Axis(const Axis& other);

		//! Destructor
		~Axis();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Axis& operator=(const Axis& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Axis& other) const;
		bool operator!=(const Axis& other) const;

		//! Wrapper that calls operator==
		bool equal(const Axis& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Azimuth of eigenvector of moment tensor expressed in
		//! principal-axes system. Measured clockwise
		//! from South-North direction at epicenter in degrees.
		void setAzimuth(const RealQuantity& azimuth);
		RealQuantity& azimuth();
		const RealQuantity& azimuth() const;

		//! Plunge of eigenvector of moment tensor expressed in
		//! principal-axes system. Measured against downward
		//! vertical direction at epicenter in degrees.
		void setPlunge(const RealQuantity& plunge);
		RealQuantity& plunge();
		const RealQuantity& plunge() const;

		//! Eigenvalue of moment tensor expressed in principal-axes
		//! system in Nm.
		void setLength(const RealQuantity& length);
		RealQuantity& length();
		const RealQuantity& length() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		RealQuantity _azimuth;
		RealQuantity _plunge;
		RealQuantity _length;
};


}
}


#endif
