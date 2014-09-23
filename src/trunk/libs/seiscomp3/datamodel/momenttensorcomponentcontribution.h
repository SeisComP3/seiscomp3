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


#ifndef __SEISCOMP_DATAMODEL_MOMENTTENSORCOMPONENTCONTRIBUTION_H__
#define __SEISCOMP_DATAMODEL_MOMENTTENSORCOMPONENTCONTRIBUTION_H__


#include <vector>
#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(MomentTensorComponentContribution);

class MomentTensorStationContribution;


class SC_SYSTEM_CORE_API MomentTensorComponentContributionIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		MomentTensorComponentContributionIndex();
		MomentTensorComponentContributionIndex(const std::string& phaseCode,
		                                       int component);

		//! Copy constructor
		MomentTensorComponentContributionIndex(const MomentTensorComponentContributionIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const MomentTensorComponentContributionIndex&) const;
		bool operator!=(const MomentTensorComponentContributionIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string phaseCode;
		int component;
};


class SC_SYSTEM_CORE_API MomentTensorComponentContribution : public Object {
	DECLARE_SC_CLASS(MomentTensorComponentContribution);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		MomentTensorComponentContribution();

		//! Copy constructor
		MomentTensorComponentContribution(const MomentTensorComponentContribution& other);

		//! Custom constructor
		MomentTensorComponentContribution(const std::string& phaseCode);
		MomentTensorComponentContribution(const std::string& phaseCode,
		                                  int component,
		                                  bool active,
		                                  double weight,
		                                  double timeShift,
		                                  double dataTimeWindow,
		                                  const OPT(double)& misfit,
		                                  const OPT(double)& snr);

		//! Destructor
		~MomentTensorComponentContribution();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		MomentTensorComponentContribution& operator=(const MomentTensorComponentContribution& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const MomentTensorComponentContribution& other) const;
		bool operator!=(const MomentTensorComponentContribution& other) const;

		//! Wrapper that calls operator==
		bool equal(const MomentTensorComponentContribution& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setPhaseCode(const std::string& phaseCode);
		const std::string& phaseCode() const;

		void setComponent(int component);
		int component() const;

		void setActive(bool active);
		bool active() const;

		void setWeight(double weight);
		double weight() const;

		void setTimeShift(double timeShift);
		double timeShift() const;

		void setDataTimeWindow(const std::vector< double >&);
		const std::vector< double >& dataTimeWindow() const;
		std::vector< double >& dataTimeWindow();

		void setMisfit(const OPT(double)& misfit);
		double misfit() const throw(Seiscomp::Core::ValueException);

		void setSnr(const OPT(double)& snr);
		double snr() const throw(Seiscomp::Core::ValueException);


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const MomentTensorComponentContributionIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const MomentTensorComponentContribution* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		MomentTensorStationContribution* momentTensorStationContribution() const;

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
		MomentTensorComponentContributionIndex _index;

		// Attributes
		bool _active;
		double _weight;
		double _timeShift;
		std::vector< double > _dataTimeWindow;
		OPT(double) _misfit;
		OPT(double) _snr;
};


}
}


#endif
