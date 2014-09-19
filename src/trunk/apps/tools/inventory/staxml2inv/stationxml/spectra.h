/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_SPECTRA_H__
#define __SEISCOMP_STATIONXML_SPECTRA_H__


#include <stationxml/metadata.h>
#include <stationxml/countertype.h>
#include <vector>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {

DEFINE_SMARTPOINTER(Sa);



DEFINE_SMARTPOINTER(Spectra);


/**
 * \brief Response spectrum values. Corresponds to the spectrum section in
 * \brief the V0 headers.
 */
class Spectra : public Core::BaseObject {
	DECLARE_CASTS(Spectra);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Spectra();

		//! Copy constructor
		Spectra(const Spectra& other);

		//! Destructor
		~Spectra();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Spectra& operator=(const Spectra& other);
		bool operator==(const Spectra& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: NumberPeriods
		void setNumberPeriods(const CounterType& numberPeriods);
		CounterType& numberPeriods();
		const CounterType& numberPeriods() const;

		//! XML tag: NumberDampingValues
		void setNumberDampingValues(const CounterType& numberDampingValues);
		CounterType& numberDampingValues();
		const CounterType& numberDampingValues() const;

	
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
		bool addSa(Sa* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeSa(Sa* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeSa(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t saCount() const;

		//! Index access
		//! @return The object at index i
		Sa* sa(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		CounterType _numberPeriods;
		CounterType _numberDampingValues;

		// Aggregations
		std::vector<SaPtr> _sas;
};


}
}


#endif
