/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_SAMPLERATERATIOTYPE_H__
#define __SEISCOMP_FDSNXML_SAMPLERATERATIOTYPE_H__


#include <fdsnxml/metadata.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(SampleRateRatioType);


/**
 * \brief Sample rate expressed as number of samples in a number of seconds.
 */
class SampleRateRatioType : public Core::BaseObject {
	DECLARE_CASTS(SampleRateRatioType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SampleRateRatioType();

		//! Copy constructor
		SampleRateRatioType(const SampleRateRatioType &other);

		//! Destructor
		~SampleRateRatioType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		SampleRateRatioType& operator=(const SampleRateRatioType &other);
		bool operator==(const SampleRateRatioType &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: NumberSamples
		void setNumberSamples(double numberSamples);
		double numberSamples() const;

		//! XML tag: NumberSeconds
		void setNumberSeconds(double numberSeconds);
		double numberSeconds() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		double _numberSamples;
		double _numberSeconds;
};


}
}


#endif
