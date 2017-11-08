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


#ifndef __SEISCOMP_DATAMODEL_MOMENTTENSORPHASESETTING_H__
#define __SEISCOMP_DATAMODEL_MOMENTTENSORPHASESETTING_H__


#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(MomentTensorPhaseSetting);

class MomentTensor;


class SC_SYSTEM_CORE_API MomentTensorPhaseSettingIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		MomentTensorPhaseSettingIndex();
		MomentTensorPhaseSettingIndex(const std::string& code);

		//! Copy constructor
		MomentTensorPhaseSettingIndex(const MomentTensorPhaseSettingIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const MomentTensorPhaseSettingIndex&) const;
		bool operator!=(const MomentTensorPhaseSettingIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string code;
};


class SC_SYSTEM_CORE_API MomentTensorPhaseSetting : public Object {
	DECLARE_SC_CLASS(MomentTensorPhaseSetting);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		MomentTensorPhaseSetting();

		//! Copy constructor
		MomentTensorPhaseSetting(const MomentTensorPhaseSetting& other);

		//! Custom constructor
		MomentTensorPhaseSetting(const std::string& code);
		MomentTensorPhaseSetting(const std::string& code,
		                         double lowerPeriod,
		                         double upperPeriod,
		                         const OPT(double)& minimumSNR = Seiscomp::Core::None,
		                         const OPT(double)& maximumTimeShift = Seiscomp::Core::None);

		//! Destructor
		~MomentTensorPhaseSetting();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		MomentTensorPhaseSetting& operator=(const MomentTensorPhaseSetting& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const MomentTensorPhaseSetting& other) const;
		bool operator!=(const MomentTensorPhaseSetting& other) const;

		//! Wrapper that calls operator==
		bool equal(const MomentTensorPhaseSetting& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setCode(const std::string& code);
		const std::string& code() const;

		void setLowerPeriod(double lowerPeriod);
		double lowerPeriod() const;

		void setUpperPeriod(double upperPeriod);
		double upperPeriod() const;

		void setMinimumSNR(const OPT(double)& minimumSNR);
		double minimumSNR() const;

		void setMaximumTimeShift(const OPT(double)& maximumTimeShift);
		double maximumTimeShift() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const MomentTensorPhaseSettingIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const MomentTensorPhaseSetting* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		MomentTensor* momentTensor() const;

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
		// Index
		MomentTensorPhaseSettingIndex _index;

		// Attributes
		double _lowerPeriod;
		double _upperPeriod;
		OPT(double) _minimumSNR;
		OPT(double) _maximumTimeShift;
};


}
}


#endif
