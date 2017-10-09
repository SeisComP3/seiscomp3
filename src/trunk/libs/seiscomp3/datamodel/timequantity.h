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


#ifndef __SEISCOMP_DATAMODEL_TIMEQUANTITY_H__
#define __SEISCOMP_DATAMODEL_TIMEQUANTITY_H__


#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(TimeQuantity);


/**
 * \brief This type describes a point in time, given in ISO 8601
 * \brief format, with
 * \brief optional symmetric or asymmetric uncertainties given in
 * \brief seconds. The
 * \brief time has to be specified in UTC.
 */
class SC_SYSTEM_CORE_API TimeQuantity : public Core::BaseObject {
	DECLARE_SC_CLASS(TimeQuantity);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		TimeQuantity();

		//! Copy constructor
		TimeQuantity(const TimeQuantity& other);

		//! Custom constructor
		TimeQuantity(Seiscomp::Core::Time value);
		TimeQuantity(Seiscomp::Core::Time value,
		             const OPT(double)& uncertainty,
		             const OPT(double)& lowerUncertainty,
		             const OPT(double)& upperUncertainty,
		             const OPT(double)& confidenceLevel);

		//! Destructor
		~TimeQuantity();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		operator Seiscomp::Core::Time&();
		operator Seiscomp::Core::Time() const;

		//! Copies the metadata of other to this
		TimeQuantity& operator=(const TimeQuantity& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const TimeQuantity& other) const;
		bool operator!=(const TimeQuantity& other) const;

		//! Wrapper that calls operator==
		bool equal(const TimeQuantity& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Point in time (UTC), given in ISO 8601 format.
		void setValue(Seiscomp::Core::Time value);
		Seiscomp::Core::Time value() const;

		//! Symmetric uncertainty of point in time in seconds.
		void setUncertainty(const OPT(double)& uncertainty);
		double uncertainty() const;

		//! Lower uncertainty of point in time in seconds.
		void setLowerUncertainty(const OPT(double)& lowerUncertainty);
		double lowerUncertainty() const;

		//! Upper uncertainty of point in time in seconds.
		void setUpperUncertainty(const OPT(double)& upperUncertainty);
		double upperUncertainty() const;

		//! Confidence level of the uncertainty, given in percent.
		void setConfidenceLevel(const OPT(double)& confidenceLevel);
		double confidenceLevel() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		Seiscomp::Core::Time _value;
		OPT(double) _uncertainty;
		OPT(double) _lowerUncertainty;
		OPT(double) _upperUncertainty;
		OPT(double) _confidenceLevel;
};


}
}


#endif
