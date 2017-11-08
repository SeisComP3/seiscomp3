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


#ifndef __SEISCOMP_DATAMODEL_MOMENTTENSORSTATIONCONTRIBUTION_H__
#define __SEISCOMP_DATAMODEL_MOMENTTENSORSTATIONCONTRIBUTION_H__


#include <vector>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/datamodel/momenttensorcomponentcontribution.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(MomentTensorStationContribution);
DEFINE_SMARTPOINTER(MomentTensorComponentContribution);

class MomentTensor;


class SC_SYSTEM_CORE_API MomentTensorStationContribution : public PublicObject {
	DECLARE_SC_CLASS(MomentTensorStationContribution);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		MomentTensorStationContribution();

	public:
		//! Copy constructor
		MomentTensorStationContribution(const MomentTensorStationContribution& other);

		//! Constructor with publicID
		MomentTensorStationContribution(const std::string& publicID);

		//! Destructor
		~MomentTensorStationContribution();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static MomentTensorStationContribution* Create();
		static MomentTensorStationContribution* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static MomentTensorStationContribution* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		MomentTensorStationContribution& operator=(const MomentTensorStationContribution& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const MomentTensorStationContribution& other) const;
		bool operator!=(const MomentTensorStationContribution& other) const;

		//! Wrapper that calls operator==
		bool equal(const MomentTensorStationContribution& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setActive(bool active);
		bool active() const;

		void setWaveformID(const OPT(WaveformStreamID)& waveformID);
		WaveformStreamID& waveformID();
		const WaveformStreamID& waveformID() const;

		void setWeight(const OPT(double)& weight);
		double weight() const;

		void setTimeShift(const OPT(double)& timeShift);
		double timeShift() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * Add an object.
		 * @param obj The object pointer
		 * @return true The object has been added
		 * @return false The object has not been added
		 *               because it already exists in the list
		 *               or it already has another parent
		 */
		bool add(MomentTensorComponentContribution* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(MomentTensorComponentContribution* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeMomentTensorComponentContribution(size_t i);
		bool removeMomentTensorComponentContribution(const MomentTensorComponentContributionIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t momentTensorComponentContributionCount() const;

		//! Index access
		//! @return The object at index i
		MomentTensorComponentContribution* momentTensorComponentContribution(size_t i) const;
		MomentTensorComponentContribution* momentTensorComponentContribution(const MomentTensorComponentContributionIndex& i) const;

		//! Find an object by its unique attribute(s)

		MomentTensor* momentTensor() const;

		//! Implement Object interface
		bool assign(Object* other);
		bool attachTo(PublicObject* parent);
		bool detachFrom(PublicObject* parent);
		bool detach();

		//! Creates a clone
		Object* clone() const;

		//! Implement PublicObject interface
		bool updateChild(Object* child);

		void accept(Visitor*);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		bool _active;
		OPT(WaveformStreamID) _waveformID;
		OPT(double) _weight;
		OPT(double) _timeShift;

		// Aggregations
		std::vector<MomentTensorComponentContributionPtr> _momentTensorComponentContributions;

	DECLARE_SC_CLASSFACTORY_FRIEND(MomentTensorStationContribution);
};


}
}


#endif
