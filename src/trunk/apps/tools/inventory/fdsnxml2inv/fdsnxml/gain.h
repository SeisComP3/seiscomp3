/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_GAIN_H__
#define __SEISCOMP_FDSNXML_GAIN_H__


#include <fdsnxml/metadata.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(Gain);


/**
 * \brief Complex type for sensitivity and frequency ranges. This complex
 * \brief type can be used to represent both overall sensitivities and
 * \brief individual stage gains. The FrequencyRangeGroup is an optional
 * \brief construct that defines a pass band in Hertz ( FrequencyStart and
 * \brief FrequencyEnd) in which the SensitivityValue is valid within the
 * \brief number of decibels specified in FrequencyDBVariation.
 */
class Gain : public Core::BaseObject {
	DECLARE_CASTS(Gain);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Gain();

		//! Copy constructor
		Gain(const Gain &other);

		//! Destructor
		~Gain();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Gain& operator=(const Gain &other);
		bool operator==(const Gain &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! A scalar that, when applied to the data values, converts the data to
		//! different units (e.g. Earth units).
		//! XML tag: Value
		void setValue(double value);
		double value() const;

		//! The frequency (in Hertz) at which the Value is valid.
		//! XML tag: Frequency
		void setFrequency(double frequency);
		double frequency() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		double _value;
		double _frequency;
};


}
}


#endif
