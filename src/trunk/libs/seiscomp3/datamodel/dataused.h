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


#ifndef __SEISCOMP_DATAMODEL_DATAUSED_H__
#define __SEISCOMP_DATAMODEL_DATAUSED_H__


#include <seiscomp3/datamodel/types.h>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(DataUsed);

class MomentTensor;


/**
 * \brief The DataUsed class describes the type of data that has been
 * \brief used for a
 * \brief moment-tensor inversion.
 */
class SC_SYSTEM_CORE_API DataUsed : public Object {
	DECLARE_SC_CLASS(DataUsed);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		DataUsed();

		//! Copy constructor
		DataUsed(const DataUsed& other);

		//! Destructor
		~DataUsed();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		DataUsed& operator=(const DataUsed& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const DataUsed& other) const;
		bool operator!=(const DataUsed& other) const;

		//! Wrapper that calls operator==
		bool equal(const DataUsed& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Type of waveform data.
		void setWaveType(DataUsedWaveType waveType);
		DataUsedWaveType waveType() const;

		//! Number of stations that have contributed data of the type
		//! given in waveType.
		void setStationCount(int stationCount);
		int stationCount() const;

		//! Number of data components of the type given in waveType.
		void setComponentCount(int componentCount);
		int componentCount() const;

		//! Shortest period present in data in seconds.
		void setShortestPeriod(const OPT(double)& shortestPeriod);
		double shortestPeriod() const;

	
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
		// Attributes
		DataUsedWaveType _waveType;
		int _stationCount;
		int _componentCount;
		OPT(double) _shortestPeriod;
};


}
}


#endif
