/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_CHANNELEPOCH_H__
#define __SEISCOMP_STATIONXML_CHANNELEPOCH_H__


#include <stationxml/metadata.h>
#include <stationxml/azimuthtype.h>
#include <stationxml/sampleratetype.h>
#include <string>
#include <stationxml/offset.h>
#include <stationxml/epoch.h>
#include <stationxml/frequencytype.h>
#include <stationxml/samplerateratiotype.h>
#include <stationxml/clockdrifttype.h>
#include <vector>
#include <stationxml/distancetype.h>
#include <stationxml/secondtype.h>
#include <stationxml/datalogger.h>
#include <stationxml/voltagetype.h>
#include <stationxml/leastsignificantbittype.h>
#include <stationxml/lontype.h>
#include <stationxml/sensitivity.h>
#include <stationxml/lattype.h>
#include <stationxml/equipment.h>
#include <stationxml/diptype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {

DEFINE_SMARTPOINTER(Output);
DEFINE_SMARTPOINTER(Response);



DEFINE_SMARTPOINTER(ChannelEpoch);


/**
 * \brief Covers SEED blockette 52 and the response blockettes.
 */
class ChannelEpoch : public Epoch {
	DECLARE_CASTS(ChannelEpoch);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ChannelEpoch();

		//! Copy constructor
		ChannelEpoch(const ChannelEpoch& other);

		//! Destructor
		~ChannelEpoch();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ChannelEpoch& operator=(const ChannelEpoch& other);
		bool operator==(const ChannelEpoch& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Latitude coordinates of this channel's sensor.
		//! XML tag: Lat
		void setLatitude(const LatType& latitude);
		LatType& latitude();
		const LatType& latitude() const;

		//! Longitude coordinates of this channel's sensor.
		//! XML tag: Lon
		void setLongitude(const LonType& longitude);
		LonType& longitude();
		const LonType& longitude() const;

		//! Elevation of the sensor.
		//! XML tag: Elevation
		void setElevation(const DistanceType& elevation);
		DistanceType& elevation();
		const DistanceType& elevation() const;

		//! The local depth or overburden of the instrument's location. For
		//! downhole instruments, the depth of the instrument under the surface
		//! ground level. For underground vaults, the distance from the instrument
		//! to the local ground level above.
		//! XML tag: Depth
		void setDepth(const DistanceType& depth);
		DistanceType& depth();
		const DistanceType& depth() const;

		//! Azimuth of the sensor in degrees from north, clockwise.
		//! XML tag: Azimuth
		void setAzimuth(const OPT(AzimuthType)& azimuth);
		AzimuthType& azimuth() throw(Seiscomp::Core::ValueException);
		const AzimuthType& azimuth() const throw(Seiscomp::Core::ValueException);

		//! Dip of the instrument in degrees, down from horizontal.
		//! XML tag: Dip
		void setDip(const OPT(DipType)& dip);
		DipType& dip() throw(Seiscomp::Core::ValueException);
		const DipType& dip() const throw(Seiscomp::Core::ValueException);

		//! Sensor's north, east, and vertical offsets in meters. Corresponds to
		//! V0 real header parameters 50-52.
		//! XML tag: Offset
		void setOffset(const OPT(Offset)& offset);
		Offset& offset() throw(Seiscomp::Core::ValueException);
		const Offset& offset() const throw(Seiscomp::Core::ValueException);

		//! XML tag: SampleRate
		void setSampleRate(const OPT(SampleRateType)& sampleRate);
		SampleRateType& sampleRate() throw(Seiscomp::Core::ValueException);
		const SampleRateType& sampleRate() const throw(Seiscomp::Core::ValueException);

		//! XML tag: SampleRateRatio
		void setSampleRateRatio(const OPT(SampleRateRatioType)& sampleRateRatio);
		SampleRateRatioType& sampleRateRatio() throw(Seiscomp::Core::ValueException);
		const SampleRateRatioType& sampleRateRatio() const throw(Seiscomp::Core::ValueException);

		//! XML tag: StorageFormat
		void setStorageFormat(const std::string& storageFormat);
		const std::string& storageFormat() const;

		//! A tolerance value, measured in seconds per sample, used as a threshold
		//! for time error detection in data from the channel.
		//! XML tag: ClockDrift
		void setClockDrift(const OPT(ClockDriftType)& clockDrift);
		ClockDriftType& clockDrift() throw(Seiscomp::Core::ValueException);
		const ClockDriftType& clockDrift() const throw(Seiscomp::Core::ValueException);

		//! XML tag: CalibrationUnit
		void setCalibrationUnit(const std::string& calibrationUnit);
		const std::string& calibrationUnit() const;

		//! XML tag: DataLogger
		void setDatalogger(const OPT(Datalogger)& datalogger);
		Datalogger& datalogger() throw(Seiscomp::Core::ValueException);
		const Datalogger& datalogger() const throw(Seiscomp::Core::ValueException);

		//! XML tag: Sensor
		void setSensor(const OPT(Equipment)& sensor);
		Equipment& sensor() throw(Seiscomp::Core::ValueException);
		const Equipment& sensor() const throw(Seiscomp::Core::ValueException);

		//! XML tag: PreAmplifier
		void setPreAmplifier(const OPT(Equipment)& preAmplifier);
		Equipment& preAmplifier() throw(Seiscomp::Core::ValueException);
		const Equipment& preAmplifier() const throw(Seiscomp::Core::ValueException);

		//! XML tag: InstrumentSensitivity
		void setInstrumentSensitivity(const OPT(Sensitivity)& instrumentSensitivity);
		Sensitivity& instrumentSensitivity() throw(Seiscomp::Core::ValueException);
		const Sensitivity& instrumentSensitivity() const throw(Seiscomp::Core::ValueException);

		//! Corresponds to SEED real parameter 41.
		//! XML tag: DampingConstant
		void setDampingConstant(const OPT(double)& dampingConstant);
		double dampingConstant() const throw(Seiscomp::Core::ValueException);

		//! XML tag: NaturalFrequency
		void setNaturalFrequency(const OPT(FrequencyType)& naturalFrequency);
		FrequencyType& naturalFrequency() throw(Seiscomp::Core::ValueException);
		const FrequencyType& naturalFrequency() const throw(Seiscomp::Core::ValueException);

		//! XML tag: LeastSignificantBit
		void setLeastSignificantBit(const OPT(LeastSignificantBitType)& leastSignificantBit);
		LeastSignificantBitType& leastSignificantBit() throw(Seiscomp::Core::ValueException);
		const LeastSignificantBitType& leastSignificantBit() const throw(Seiscomp::Core::ValueException);

		//! In Volts. Corresponds to V0 real header paramter 23.
		//! XML tag: FullScaleInput
		void setFullScaleInput(const OPT(VoltageType)& fullScaleInput);
		VoltageType& fullScaleInput() throw(Seiscomp::Core::ValueException);
		const VoltageType& fullScaleInput() const throw(Seiscomp::Core::ValueException);

		//! XML tag: FullScaleOutput
		void setFullScaleOutput(const OPT(VoltageType)& fullScaleOutput);
		VoltageType& fullScaleOutput() throw(Seiscomp::Core::ValueException);
		const VoltageType& fullScaleOutput() const throw(Seiscomp::Core::ValueException);

		//! Full scale sensing capability. Corresponds to V0 real parameter 44.
		//! XML tag: FullScaleCapability
		void setFullScaleCapability(const OPT(VoltageType)& fullScaleCapability);
		VoltageType& fullScaleCapability() throw(Seiscomp::Core::ValueException);
		const VoltageType& fullScaleCapability() const throw(Seiscomp::Core::ValueException);

		//! Pre-trigger memory recording time (secs). Corresponds to V0 real
		//! header parameter 24.
		//! XML tag: PreTriggerTime
		void setPreTriggerTime(const OPT(SecondType)& preTriggerTime);
		SecondType& preTriggerTime() throw(Seiscomp::Core::ValueException);
		const SecondType& preTriggerTime() const throw(Seiscomp::Core::ValueException);

		//! Post-detrigger recording time (secs). Corresponds to V0 real header
		//! parameter 25.
		//! XML tag: PostDetriggerTime
		void setPostDetriggerTime(const OPT(SecondType)& postDetriggerTime);
		SecondType& postDetriggerTime() throw(Seiscomp::Core::ValueException);
		const SecondType& postDetriggerTime() const throw(Seiscomp::Core::ValueException);

	
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
		bool addOutput(Output* obj);
		bool addResponse(Response* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeOutput(Output* obj);
		bool removeResponse(Response* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeOutput(size_t i);
		bool removeResponse(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t outputCount() const;
		size_t responseCount() const;

		//! Index access
		//! @return The object at index i
		Output* output(size_t i) const;
		Response* response(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		LatType _latitude;
		LonType _longitude;
		DistanceType _elevation;
		DistanceType _depth;
		OPT(AzimuthType) _azimuth;
		OPT(DipType) _dip;
		OPT(Offset) _offset;
		OPT(SampleRateType) _sampleRate;
		OPT(SampleRateRatioType) _sampleRateRatio;
		std::string _storageFormat;
		OPT(ClockDriftType) _clockDrift;
		std::string _calibrationUnit;
		OPT(Datalogger) _datalogger;
		OPT(Equipment) _sensor;
		OPT(Equipment) _preAmplifier;
		OPT(Sensitivity) _instrumentSensitivity;
		OPT(double) _dampingConstant;
		OPT(FrequencyType) _naturalFrequency;
		OPT(LeastSignificantBitType) _leastSignificantBit;
		OPT(VoltageType) _fullScaleInput;
		OPT(VoltageType) _fullScaleOutput;
		OPT(VoltageType) _fullScaleCapability;
		OPT(SecondType) _preTriggerTime;
		OPT(SecondType) _postDetriggerTime;

		// Aggregations
		std::vector<OutputPtr> _outputs;
		std::vector<ResponsePtr> _responses;
};


}
}


#endif
