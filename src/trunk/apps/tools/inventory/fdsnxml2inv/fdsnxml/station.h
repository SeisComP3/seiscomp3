/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_STATION_H__
#define __SEISCOMP_FDSNXML_STATION_H__


#include <fdsnxml/metadata.h>
#include <string>
#include <fdsnxml/floattype.h>
#include <fdsnxml/basenode.h>
#include <fdsnxml/countertype.h>
#include <fdsnxml/longitudetype.h>
#include <fdsnxml/distancetype.h>
#include <vector>
#include <fdsnxml/site.h>
#include <fdsnxml/latitudetype.h>
#include <fdsnxml/date.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {

DEFINE_SMARTPOINTER(Equipment);
DEFINE_SMARTPOINTER(Operator);
DEFINE_SMARTPOINTER(ExternalReference);
DEFINE_SMARTPOINTER(Channel);



DEFINE_SMARTPOINTER(Station);


/**
 * \brief This type represents a Station epoch. It is common to only have a
 * \brief single station epoch with the station's creation and termination
 * \brief dates as the epoch start and end dates.
 */
class Station : public BaseNode {
	DECLARE_CASTS(Station);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Station();

		//! Copy constructor
		Station(const Station &other);

		//! Destructor
		~Station();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Station& operator=(const Station &other);
		bool operator==(const Station &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: Latitude
		void setLatitude(const LatitudeType& latitude);
		LatitudeType& latitude();
		const LatitudeType& latitude() const;

		//! XML tag: Longitude
		void setLongitude(const LongitudeType& longitude);
		LongitudeType& longitude();
		const LongitudeType& longitude() const;

		//! XML tag: Elevation
		void setElevation(const DistanceType& elevation);
		DistanceType& elevation();
		const DistanceType& elevation() const;

		//! These fields describe the location of the station using geopolitical
		//! entities (country, city, etc.).
		//! XML tag: Site
		void setSite(const Site& site);
		Site& site();
		const Site& site() const;

		//! Elevation of the water surface in meters for underwater sites, where 0
		//! is sea level.
		//! XML tag: waterLevel
		void setWaterLevel(const OPT(FloatType)& waterLevel);
		FloatType& waterLevel();
		const FloatType& waterLevel() const;

		//! Type of vault, e.g. WWSSN, tunnel, transportable array, etc.
		//! XML tag: Vault
		void setVault(const std::string& vault);
		const std::string& vault() const;

		//! Type of rock and/or geologic formation.
		//! XML tag: Geology
		void setGeology(const std::string& geology);
		const std::string& geology() const;

		//! Date and time (UTC) when the station was first installed.
		//! XML tag: CreationDate
		void setCreationDate(const OPT(DateTime)& creationDate);
		DateTime creationDate() const;

		//! Date and time (UTC) when the station was terminated or will be
		//! terminated. A blank value should be assumed to mean that the station
		//! is still active.
		//! XML tag: TerminationDate
		void setTerminationDate(const OPT(DateTime)& terminationDate);
		DateTime terminationDate() const;

		//! Total number of channels recorded at this station.
		//! XML tag: TotalNumberChannels
		void setTotalNumberChannels(const OPT(CounterType)& totalNumberChannels);
		CounterType& totalNumberChannels();
		const CounterType& totalNumberChannels() const;

		//! Number of channels recorded at this station and selected by the query
		//! that produced this document.
		//! XML tag: SelectedNumberChannels
		void setSelectedNumberChannels(const OPT(CounterType)& selectedNumberChannels);
		CounterType& selectedNumberChannels();
		const CounterType& selectedNumberChannels() const;

	
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
		bool addEquipment(Equipment *obj);
		bool addOperators(Operator *obj);
		bool addExternalReference(ExternalReference *obj);
		bool addChannel(Channel *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeEquipment(Equipment *obj);
		bool removeOperators(Operator *obj);
		bool removeExternalReference(ExternalReference *obj);
		bool removeChannel(Channel *obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeEquipment(size_t i);
		bool removeOperators(size_t i);
		bool removeExternalReference(size_t i);
		bool removeChannel(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t equipmentCount() const;
		size_t operatorsCount() const;
		size_t externalReferenceCount() const;
		size_t channelCount() const;

		//! Index access
		//! @return The object at index i
		Equipment* equipment(size_t i) const;
		Operator* operators(size_t i) const;
		ExternalReference* externalReference(size_t i) const;
		Channel* channel(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		LatitudeType _latitude;
		LongitudeType _longitude;
		DistanceType _elevation;
		Site _site;
		OPT(FloatType) _waterLevel;
		std::string _vault;
		std::string _geology;
		OPT(DateTime) _creationDate;
		OPT(DateTime) _terminationDate;
		OPT(CounterType) _totalNumberChannels;
		OPT(CounterType) _selectedNumberChannels;

		// Aggregations
		std::vector<EquipmentPtr> _equipments;
		std::vector<OperatorPtr> _operatorss;
		std::vector<ExternalReferencePtr> _externalReferences;
		std::vector<ChannelPtr> _channels;
};


}
}


#endif
