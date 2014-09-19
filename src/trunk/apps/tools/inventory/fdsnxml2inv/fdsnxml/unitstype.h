/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_UNITSTYPE_H__
#define __SEISCOMP_FDSNXML_UNITSTYPE_H__


#include <fdsnxml/metadata.h>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(UnitsType);


/**
 * \brief A type to document units. Corresponds to SEED blockette 34.
 */
class UnitsType : public Core::BaseObject {
	DECLARE_CASTS(UnitsType);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		UnitsType();

		//! Copy constructor
		UnitsType(const UnitsType &other);

		//! Custom constructor
		UnitsType(const std::string& Name);
		UnitsType(const std::string& Name,
		          const std::string& Description);

		//! Destructor
		~UnitsType();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		operator std::string&();
		operator const std::string&() const;

		//! Copies the metadata of other to this
		UnitsType& operator=(const UnitsType &other);
		bool operator==(const UnitsType &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Name of units, e.g. "M/S", "V", "PA".
		//! XML tag: Name
		void setName(const std::string& name);
		const std::string& name() const;

		//! Description of units, e.g. "Velocity in meters per second", "Volts",
		//! "Pascals".
		//! XML tag: Description
		void setDescription(const std::string& description);
		const std::string& description() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _name;
		std::string _description;
};


}
}


#endif
