/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_FIR_H__
#define __SEISCOMP_STATIONXML_FIR_H__


#include <stationxml/metadata.h>
#include <stationxml/types.h>
#include <vector>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {

DEFINE_SMARTPOINTER(NumeratorCoefficient);



DEFINE_SMARTPOINTER(FIR);


class FIR : public Core::BaseObject {
	DECLARE_CASTS(FIR);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		FIR();

		//! Copy constructor
		FIR(const FIR& other);

		//! Destructor
		~FIR();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		FIR& operator=(const FIR& other);
		bool operator==(const FIR& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: ResponseName
		void setResponseName(const std::string& responseName);
		const std::string& responseName() const;

		//! XML tag: Symmetry
		void setSymmetry(const OPT(SymmetryType)& symmetry);
		SymmetryType symmetry() const throw(Seiscomp::Core::ValueException);

		//! XML tag: InputUnits
		void setInputUnits(const std::string& inputUnits);
		const std::string& inputUnits() const;

		//! XML tag: OutputUnits
		void setOutputUnits(const std::string& outputUnits);
		const std::string& outputUnits() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * Add an object.
		 * @param obj The object pointer
		 * @return true The object has been added
		 * @return false The object has not been added
		 *               because it already exists in the list
		 *               or it already has another parent
		 */
		bool addNumeratorCoefficient(NumeratorCoefficient* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeNumeratorCoefficient(NumeratorCoefficient* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeNumeratorCoefficient(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t numeratorCoefficientCount() const;

		//! Index access
		//! @return The object at index i
		NumeratorCoefficient* numeratorCoefficient(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _responseName;
		OPT(SymmetryType) _symmetry;
		std::string _inputUnits;
		std::string _outputUnits;

		// Aggregations
		std::vector<NumeratorCoefficientPtr> _numeratorCoefficients;
};


}
}


#endif
