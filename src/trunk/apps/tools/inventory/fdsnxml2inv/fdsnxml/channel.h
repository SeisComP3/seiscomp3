/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_CHANNEL_H__
#define __SEISCOMP_FDSNXML_CHANNEL_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/unitstype.h>
#include <fdsnxml/azimuthtype.h>
#include <string>
#include <fdsnxml/floattype.h>
#include <fdsnxml/basenode.h>
#include <fdsnxml/longitudetype.h>
#include <fdsnxml/distancetype.h>
#include <fdsnxml/samplerateratiotype.h>
#include <fdsnxml/diptype.h>
#include <fdsnxml/clockdrifttype.h>
#include <vector>
#include <fdsnxml/equipment.h>
#include <fdsnxml/sampleratetype.h>
#include <fdsnxml/response.h>
#include <fdsnxml/latitudetype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {

DEFINE_SMARTPOINTER(Output);
DEFINE_SMARTPOINTER(Equipment);



DEFINE_SMARTPOINTER(Channel);


/**
 * \brief Equivalent to SEED blockette 52 and parent element for the related
 * \brief the response blockettes.
 */
class Channel : public BaseNode {
	DECLARE_CASTS(Channel);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Channel();

		//! Copy constructor
		Channel(const Channel &other);

		//! Destructor
		~Channel();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Channel& operator=(const Channel &other);
		bool operator==(const Channel &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Latitude coordinate of this channel's sensor.
		//! XML tag: Latitude
		void setLatitude(const LatitudeType& latitude);
		LatitudeType& latitude();
		const LatitudeType& latitude() const;

		//! Longitude coordinates of this channel's sensor.
		//! XML tag: Longitude
		void setLongitude(const LongitudeType& longitude);
		LongitudeType& longitude();
		const LongitudeType& longitude() const;

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
		AzimuthType& azimuth();
		const AzimuthType& azimuth() const;

		//! Dip of the instrument in degrees, down from horizontal.
		//! XML tag: Dip
		void setDip(const OPT(DipType)& dip);
		DipType& dip();
		const DipType& dip() const;

		//! Elevation of the water surface in meters for underwater sites, where 0
		//! is sea level.
		//! XML tag: waterLevel
		void setWaterLevel(const OPT(FloatType)& waterLevel);
		FloatType& waterLevel();
		const FloatType& waterLevel() const;

		//! XML tag: SampleRate
		void setSampleRate(const OPT(SampleRateType)& sampleRate);
		SampleRateType& sampleRate();
		const SampleRateType& sampleRate() const;

		//! XML tag: SampleRateRatio
		void setSampleRateRatio(const OPT(SampleRateRatioType)& sampleRateRatio);
		SampleRateRatioType& sampleRateRatio();
		const SampleRateRatioType& sampleRateRatio() const;

		//! A tolerance value, measured in seconds per sample, used as a threshold
		//! for time error detection in data from the channel.
		//! XML tag: ClockDrift
		void setClockDrift(const OPT(ClockDriftType)& clockDrift);
		ClockDriftType& clockDrift();
		const ClockDriftType& clockDrift() const;

		//! XML tag: CalibrationUnits
		void setCalibrationUnits(const OPT(UnitsType)& calibrationUnits);
		UnitsType& calibrationUnits();
		const UnitsType& calibrationUnits() const;

		//! XML tag: Sensor
		void setSensor(const OPT(Equipment)& sensor);
		Equipment& sensor();
		const Equipment& sensor() const;

		//! XML tag: PreAmplifier
		void setPreAmplifier(const OPT(Equipment)& preAmplifier);
		Equipment& preAmplifier();
		const Equipment& preAmplifier() const;

		//! XML tag: DataLogger
		void setDataLogger(const OPT(Equipment)& dataLogger);
		Equipment& dataLogger();
		const Equipment& dataLogger() const;

		//! XML tag: Response
		void setResponse(const OPT(Response)& response);
		Response& response();
		const Response& response() const;

		//! XML tag: locationCode
		void setLocationCode(const std::string& locationCode);
		const std::string& locationCode() const;

	
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
		bool addType(Output *obj);
		bool addEquipment(Equipment *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeType(Output *obj);
		bool removeEquipment(Equipment *obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeType(size_t i);
		bool removeEquipment(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t typeCount() const;
		size_t equipmentCount() const;

		//! Index access
		//! @return The object at index i
		Output* type(size_t i) const;
		Equipment* equipment(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		LatitudeType _latitude;
		LongitudeType _longitude;
		DistanceType _elevation;
		DistanceType _depth;
		OPT(AzimuthType) _azimuth;
		OPT(DipType) _dip;
		OPT(FloatType) _waterLevel;
		OPT(SampleRateType) _sampleRate;
		OPT(SampleRateRatioType) _sampleRateRatio;
		OPT(ClockDriftType) _clockDrift;
		OPT(UnitsType) _calibrationUnits;
		OPT(Equipment) _sensor;
		OPT(Equipment) _preAmplifier;
		OPT(Equipment) _dataLogger;
		OPT(Response) _response;
		std::string _locationCode;

		// Aggregations
		std::vector<OutputPtr> _types;
		std::vector<EquipmentPtr> _equipments;
};


}
}


#endif
