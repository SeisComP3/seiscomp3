/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_BASEFILTER_H__
#define __SEISCOMP_FDSNXML_BASEFILTER_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/unitstype.h>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(BaseFilter);


/**
 * \brief The BaseFilterType is derived by all filters.
 */
class BaseFilter : public Core::BaseObject {
	DECLARE_CASTS(BaseFilter);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		BaseFilter();

		//! Copy constructor
		BaseFilter(const BaseFilter &other);

		//! Destructor
		~BaseFilter();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		BaseFilter& operator=(const BaseFilter &other);
		bool operator==(const BaseFilter &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: Description
		void setDescription(const std::string& description);
		const std::string& description() const;

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

		//! Same meaning as Equipment:resourceId.
		//! XML tag: resourceId
		void setResourceId(const std::string& resourceId);
		const std::string& resourceId() const;

		//! A name given to this filter.
		//! XML tag: name
		void setName(const std::string& name);
		const std::string& name() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _description;
		UnitsType _inputUnits;
		UnitsType _outputUnits;
		std::string _resourceId;
		std::string _name;
};


}
}


#endif
