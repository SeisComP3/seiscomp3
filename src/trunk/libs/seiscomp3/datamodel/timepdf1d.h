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


#ifndef __SEISCOMP_DATAMODEL_TIMEPDF1D_H__
#define __SEISCOMP_DATAMODEL_TIMEPDF1D_H__


#include <seiscomp3/datamodel/timearray.h>
#include <seiscomp3/datamodel/realarray.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(TimePDF1D);


/**
 * \brief A probability density function description. It can be used
 * \brief in three
 * \brief different modes:
 * \brief 
 * \brief 1) "raw samples mode"
 * \brief 
 * \brief variable is a list of M values, no probability. The values
 * \brief represent
 * \brief samples, no binning/probabilities made.
 * \brief 
 * \brief 2) "implicitly binned PDF"
 * \brief 
 * \brief variable and probabilty arrays have length N. variable
 * \brief values to be
 * \brief interpreted as "bin centers" (or representative values),
 * \brief no bin edges given.
 * \brief 
 * \brief 3) "explicitly binned PDF"
 * \brief 
 * \brief variable has length N+1, probability has length N. variable
 * \brief values
 * \brief describe bin edges (upper bin edge is lower edge of next
 * \brief bin).
 */
class SC_SYSTEM_CORE_API TimePDF1D : public Core::BaseObject {
	DECLARE_SC_CLASS(TimePDF1D);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		TimePDF1D();

		//! Copy constructor
		TimePDF1D(const TimePDF1D& other);

		//! Destructor
		~TimePDF1D();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		TimePDF1D& operator=(const TimePDF1D& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const TimePDF1D& other) const;
		bool operator!=(const TimePDF1D& other) const;

		//! Wrapper that calls operator==
		bool equal(const TimePDF1D& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! List of datetimes
		void setVariable(const TimeArray& variable);
		TimeArray& variable();
		const TimeArray& variable() const;

		//! List of probabilities
		void setProbability(const RealArray& probability);
		RealArray& probability();
		const RealArray& probability() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		TimeArray _variable;
		RealArray _probability;
};


}
}


#endif
