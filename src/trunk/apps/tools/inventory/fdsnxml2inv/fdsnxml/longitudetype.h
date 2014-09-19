/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_LONGITUDETYPE_H__
#define __SEISCOMP_FDSNXML_LONGITUDETYPE_H__


#include <fdsnxml/metadata.h>
#include <string>
#include <fdsnxml/floattype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(LongitudeType);


/**
 * \brief Type for longitude coordinates.
 */
class LongitudeType : public FloatType {
	DECLARE_CASTS(LongitudeType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		LongitudeType();

		//! Copy constructor
		LongitudeType(const LongitudeType &other);

		//! Destructor
		~LongitudeType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		LongitudeType& operator=(const LongitudeType &other);
		bool operator==(const LongitudeType &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: datum
		void setDatum(const std::string& datum);
		const std::string& datum() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _datum;
};


}
}


#endif
