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


#ifndef __SEISCOMP_DATAMODEL_REALPDF1D_H__
#define __SEISCOMP_DATAMODEL_REALPDF1D_H__


#include <seiscomp3/datamodel/realarray.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(RealPDF1D);


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
class SC_SYSTEM_CORE_API RealPDF1D : public Core::BaseObject {
	DECLARE_SC_CLASS(RealPDF1D);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		RealPDF1D();

		//! Copy constructor
		RealPDF1D(const RealPDF1D& other);

		//! Destructor
		~RealPDF1D();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		RealPDF1D& operator=(const RealPDF1D& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const RealPDF1D& other) const;
		bool operator!=(const RealPDF1D& other) const;

		//! Wrapper that calls operator==
		bool equal(const RealPDF1D& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! List of values
		void setVariable(const RealArray& variable);
		RealArray& variable();
		const RealArray& variable() const;

		//! List of probabilities
		void setProbability(const RealArray& probability);
		RealArray& probability();
		const RealArray& probability() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		RealArray _variable;
		RealArray _probability;
};


}
}


#endif
