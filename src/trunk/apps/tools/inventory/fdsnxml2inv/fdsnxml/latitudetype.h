/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_LATITUDETYPE_H__
#define __SEISCOMP_FDSNXML_LATITUDETYPE_H__


#include <fdsnxml/metadata.h>
#include <string>
#include <fdsnxml/floattype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(LatitudeType);


/**
 * \brief Type for latitude coordinates.
 */
class LatitudeType : public FloatType {
	DECLARE_CASTS(LatitudeType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		LatitudeType();

		//! Copy constructor
		LatitudeType(const LatitudeType &other);

		//! Destructor
		~LatitudeType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		LatitudeType& operator=(const LatitudeType &other);
		bool operator==(const LatitudeType &other) const;


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
