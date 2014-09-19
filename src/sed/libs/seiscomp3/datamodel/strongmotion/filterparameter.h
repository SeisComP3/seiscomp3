/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

// This file was created by a source code generator.
// Do not modify the contents. Change the definition and run the generator
// again!


#ifndef __SEISCOMP_DATAMODEL_STRONGMOTION_FILTERPARAMETER_H__
#define __SEISCOMP_DATAMODEL_STRONGMOTION_FILTERPARAMETER_H__


#include <string>
#include <seiscomp3/datamodel/realquantity.h>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/strongmotion/api.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


DEFINE_SMARTPOINTER(FilterParameter);

class SimpleFilter;


class SC_STRONGMOTION_API FilterParameter : public Object {
	DECLARE_SC_CLASS(FilterParameter);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		FilterParameter();

		//! Copy constructor
		FilterParameter(const FilterParameter& other);

		//! Destructor
		~FilterParameter();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		FilterParameter& operator=(const FilterParameter& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const FilterParameter& other) const;
		bool operator!=(const FilterParameter& other) const;

		//! Wrapper that calls operator==
		bool equal(const FilterParameter& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setValue(const RealQuantity& value);
		RealQuantity& value();
		const RealQuantity& value() const;

		void setName(const std::string& name);
		const std::string& name() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		SimpleFilter* simpleFilter() const;

		//! Implement Object interface
		bool assign(Object* other);
		bool attachTo(PublicObject* parent);
		bool detachFrom(PublicObject* parent);
		bool detach();

		//! Creates a clone
		Object* clone() const;

		void accept(Visitor*);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		RealQuantity _value;
		std::string _name;
};


}
}
}


#endif
