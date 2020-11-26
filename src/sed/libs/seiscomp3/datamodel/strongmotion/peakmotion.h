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


#ifndef __SEISCOMP_DATAMODEL_STRONGMOTION_PEAKMOTION_H__
#define __SEISCOMP_DATAMODEL_STRONGMOTION_PEAKMOTION_H__


#include <seiscomp3/datamodel/timequantity.h>
#include <string>
#include <seiscomp3/datamodel/realquantity.h>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/strongmotion/api.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


DEFINE_SMARTPOINTER(PeakMotion);

class Record;


class SC_STRONGMOTION_API PeakMotion : public Object {
	DECLARE_SC_CLASS(PeakMotion);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		PeakMotion();

		//! Copy constructor
		PeakMotion(const PeakMotion& other);

		//! Destructor
		~PeakMotion();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		PeakMotion& operator=(const PeakMotion& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const PeakMotion& other) const;
		bool operator!=(const PeakMotion& other) const;

		//! Wrapper that calls operator==
		bool equal(const PeakMotion& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setMotion(const RealQuantity& motion);
		RealQuantity& motion();
		const RealQuantity& motion() const;

		void setType(const std::string& type);
		const std::string& type() const;

		void setPeriod(const OPT(double)& period);
		double period() const;

		void setDamping(const OPT(double)& damping);
		double damping() const;

		void setMethod(const std::string& method);
		const std::string& method() const;

		void setAtTime(const OPT(TimeQuantity)& atTime);
		TimeQuantity& atTime();
		const TimeQuantity& atTime() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Record* record() const;

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
		RealQuantity _motion;
		std::string _type;
		OPT(double) _period;
		OPT(double) _damping;
		std::string _method;
		OPT(TimeQuantity) _atTime;
};


}
}
}


#endif
