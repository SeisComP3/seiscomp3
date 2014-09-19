/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_SENSITIVITY_H__
#define __SEISCOMP_FDSNXML_SENSITIVITY_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/unitstype.h>
#include <fdsnxml/gain.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(Sensitivity);


/**
 * \brief Sensitivity and frequency ranges. The FrequencyRangeGroup is an
 * \brief optional construct that defines a pass band in Hertz
 * \brief (FrequencyStart and FrequencyEnd) in which the SensitivityValue is
 * \brief valid within the number of decibels specified in
 * \brief FrequencyDBVariation.
 */
class Sensitivity : public Gain {
	DECLARE_CASTS(Sensitivity);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Sensitivity();

		//! Copy constructor
		Sensitivity(const Sensitivity &other);

		//! Destructor
		~Sensitivity();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Sensitivity& operator=(const Sensitivity &other);
		bool operator==(const Sensitivity &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! The units of the data as input from the perspective of data
		//! acquisition. After correcting data for this response, these would be
		//! the resulting units.
		//! XML tag: InputUnits
		void setInputUnits(const UnitsType& inputUnits);
		UnitsType& inputUnits();
		const UnitsType& inputUnits() const;

		//! The units of the data as output from the perspective of data
		//! acquisition. These would be the units of the data prior to correcting
		//! for this response.
		//! XML tag: OutputUnits
		void setOutputUnits(const UnitsType& outputUnits);
		UnitsType& outputUnits();
		const UnitsType& outputUnits() const;

		//! XML tag: FrequencyStart
		void setFrequencyStart(const OPT(double)& frequencyStart);
		double frequencyStart() const throw(Seiscomp::Core::ValueException);

		//! XML tag: FrequencyEnd
		void setFrequencyEnd(const OPT(double)& frequencyEnd);
		double frequencyEnd() const throw(Seiscomp::Core::ValueException);

		//! Variation in decibels within the specified range.
		//! XML tag: FrequencyDBVariation
		void setFrequencyDBVariation(const OPT(double)& frequencyDBVariation);
		double frequencyDBVariation() const throw(Seiscomp::Core::ValueException);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		UnitsType _inputUnits;
		UnitsType _outputUnits;
		OPT(double) _frequencyStart;
		OPT(double) _frequencyEnd;
		OPT(double) _frequencyDBVariation;
};


}
}


#endif
