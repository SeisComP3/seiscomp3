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


#ifndef __SEISCOMP_DATAMODEL_SENSORCALIBRATION_H__
#define __SEISCOMP_DATAMODEL_SENSORCALIBRATION_H__


#include <seiscomp3/datamodel/blob.h>
#include <seiscomp3/core/datetime.h>
#include <string>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(SensorCalibration);

class Sensor;


class SC_SYSTEM_CORE_API SensorCalibrationIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SensorCalibrationIndex();
		SensorCalibrationIndex(const std::string& serialNumber,
		                       int channel,
		                       Seiscomp::Core::Time start);

		//! Copy constructor
		SensorCalibrationIndex(const SensorCalibrationIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const SensorCalibrationIndex&) const;
		bool operator!=(const SensorCalibrationIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string serialNumber;
		int channel;
		Seiscomp::Core::Time start;
};


/**
 * \brief This type describes a sensor calibration
 */
class SC_SYSTEM_CORE_API SensorCalibration : public Object {
	DECLARE_SC_CLASS(SensorCalibration);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SensorCalibration();

		//! Copy constructor
		SensorCalibration(const SensorCalibration& other);

		//! Destructor
		~SensorCalibration();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		SensorCalibration& operator=(const SensorCalibration& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const SensorCalibration& other) const;
		bool operator!=(const SensorCalibration& other) const;

		//! Wrapper that calls operator==
		bool equal(const SensorCalibration& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Referred from network/station/Stream/@sensorSerialNumber
		void setSerialNumber(const std::string& serialNumber);
		const std::string& serialNumber() const;

		//! Referred from network/station/Stream/@sensorChannel
		void setChannel(int channel);
		int channel() const;

		//! Start of validity
		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		//! End of validity
		void setEnd(const OPT(Seiscomp::Core::Time)& end);
		Seiscomp::Core::Time end() const;

		//! Overrides nominal gain of calibrated sensor (48.05/58.04)
		void setGain(const OPT(double)& gain);
		double gain() const;

		//! Gain frequency (48.06/58.05)
		void setGainFrequency(const OPT(double)& gainFrequency);
		double gainFrequency() const;

		void setRemark(const OPT(Blob)& remark);
		Blob& remark();
		const Blob& remark() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const SensorCalibrationIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const SensorCalibration* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Sensor* sensor() const;

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
		SensorCalibrationIndex _index;

		// Attributes
		OPT(Seiscomp::Core::Time) _end;
		OPT(double) _gain;
		OPT(double) _gainFrequency;
		OPT(Blob) _remark;
};


}
}


#endif
