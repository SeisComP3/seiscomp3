/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_SAMPLERATETYPE_H__
#define __SEISCOMP_FDSNXML_SAMPLERATETYPE_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/floattype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(SampleRateType);


class SampleRateType : public FloatType {
	DECLARE_CASTS(SampleRateType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SampleRateType();

		//! Copy constructor
		SampleRateType(const SampleRateType &other);

		//! Destructor
		~SampleRateType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		SampleRateType& operator=(const SampleRateType &other);
		bool operator==(const SampleRateType &other) const;

};


}
}


#endif
