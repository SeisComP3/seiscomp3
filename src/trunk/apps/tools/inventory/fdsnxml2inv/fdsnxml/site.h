/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_SITE_H__
#define __SEISCOMP_FDSNXML_SITE_H__


#include <fdsnxml/metadata.h>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(Site);


/**
 * \brief Description of a site location using name and optional geopolitical
 * \brief boundaries (country, city, etc.).
 */
class Site : public Core::BaseObject {
	DECLARE_CASTS(Site);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Site();

		//! Copy constructor
		Site(const Site &other);

		//! Destructor
		~Site();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Site& operator=(const Site &other);
		bool operator==(const Site &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! The commonly used S name of this station, equivalent to the SEED
		//! blockette 50, field 9.
		//! XML tag: Name
		void setName(const std::string& name);
		const std::string& name() const;

		//! A longer description of the location of this station, e.g. "20 miles
		//! west of Highway 40."
		//! XML tag: Description
		void setDescription(const std::string& description);
		const std::string& description() const;

		//! The town or city closest to the station.
		//! XML tag: Town
		void setTown(const std::string& town);
		const std::string& town() const;

		//! XML tag: County
		void setCounty(const std::string& county);
		const std::string& county() const;

		//! The state, province, or region of this site.
		//! XML tag: Region
		void setRegion(const std::string& region);
		const std::string& region() const;

		//! XML tag: Country
		void setCountry(const std::string& country);
		const std::string& country() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _name;
		std::string _description;
		std::string _town;
		std::string _county;
		std::string _region;
		std::string _country;
};


}
}


#endif
