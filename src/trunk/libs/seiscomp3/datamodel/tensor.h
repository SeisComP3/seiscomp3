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


#ifndef __SEISCOMP_DATAMODEL_TENSOR_H__
#define __SEISCOMP_DATAMODEL_TENSOR_H__


#include <seiscomp3/datamodel/realquantity.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Tensor);


/**
 * \brief The Tensor class represents the six moment-tensor elements
 * \brief Mrr , Mtt ,
 * \brief Mpp , Mrt , Mrp , Mtp in the spherical coordinate system
 * \brief defined by local
 * \brief upward vertical (r), North-South (t), and West-East (p)
 * \brief directions. See
 * \brief Aki and Richards (1980) for conversions to other coordinate
 * \brief systems.
 */
class SC_SYSTEM_CORE_API Tensor : public Core::BaseObject {
	DECLARE_SC_CLASS(Tensor);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Tensor();

		//! Copy constructor
		Tensor(const Tensor& other);

		//! Destructor
		~Tensor();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Tensor& operator=(const Tensor& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Tensor& other) const;
		bool operator!=(const Tensor& other) const;

		//! Wrapper that calls operator==
		bool equal(const Tensor& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Moment-tensor element Mrr in Nm.
		void setMrr(const RealQuantity& Mrr);
		RealQuantity& Mrr();
		const RealQuantity& Mrr() const;

		//! Moment-tensor element Mtt in Nm.
		void setMtt(const RealQuantity& Mtt);
		RealQuantity& Mtt();
		const RealQuantity& Mtt() const;

		//! Moment-tensor element Mpp in Nm.
		void setMpp(const RealQuantity& Mpp);
		RealQuantity& Mpp();
		const RealQuantity& Mpp() const;

		//! Moment-tensor element Mrt in Nm.
		void setMrt(const RealQuantity& Mrt);
		RealQuantity& Mrt();
		const RealQuantity& Mrt() const;

		//! Moment-tensor element Mrp in Nm.
		void setMrp(const RealQuantity& Mrp);
		RealQuantity& Mrp();
		const RealQuantity& Mrp() const;

		//! Moment-tensor element Mtp in Nm.
		void setMtp(const RealQuantity& Mtp);
		RealQuantity& Mtp();
		const RealQuantity& Mtp() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		RealQuantity _mrr;
		RealQuantity _mtt;
		RealQuantity _mpp;
		RealQuantity _mrt;
		RealQuantity _mrp;
		RealQuantity _mtp;
};


}
}


#endif
