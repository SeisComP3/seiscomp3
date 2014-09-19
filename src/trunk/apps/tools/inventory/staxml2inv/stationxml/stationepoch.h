/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_STATIONEPOCH_H__
#define __SEISCOMP_STATIONXML_STATIONEPOCH_H__


#include <stationxml/metadata.h>
#include <string>
#include <stationxml/site.h>
#include <stationxml/date.h>
#include <stationxml/distancetype.h>
#include <vector>
#include <stationxml/epoch.h>
#include <stationxml/lattype.h>
#include <stationxml/lontype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {

DEFINE_SMARTPOINTER(Equipment);
DEFINE_SMARTPOINTER(Operator);
DEFINE_SMARTPOINTER(ExternalDocument);
DEFINE_SMARTPOINTER(Channel);



DEFINE_SMARTPOINTER(StationEpoch);


/**
 * \brief This type represents a station epoch. An organization that does not
 * \brief keep track of station epochs should just include one epoch with the
 * \brief station's creation and termination dates as the epoch start and end
 * \brief dates.
 */
class StationEpoch : public Epoch {
	DECLARE_CASTS(StationEpoch);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		StationEpoch();

		//! Copy constructor
		StationEpoch(const StationEpoch& other);

		//! Destructor
		~StationEpoch();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		StationEpoch& operator=(const StationEpoch& other);
		bool operator==(const StationEpoch& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: Lat
		void setLatitude(const LatType& latitude);
		LatType& latitude();
		const LatType& latitude() const;

		//! XML tag: Lon
		void setLongitude(const LonType& longitude);
		LonType& longitude();
		const LonType& longitude() const;

		//! XML tag: Elevation
		void setElevation(const DistanceType& elevation);
		DistanceType& elevation();
		const DistanceType& elevation() const;

		//! XML tag: Site
		void setSite(const Site& site);
		Site& site();
		const Site& site() const;

		//! Type of vault, e.g. WWSSN, tunnel, transportable array, etc.
		//! XML tag: Vault
		void setVault(const std::string& vault);
		const std::string& vault() const;

		//! Type of rock and/or geologic formation.
		//! XML tag: Geology
		void setGeology(const std::string& geology);
		const std::string& geology() const;

		//! Station structure type, as defined in V0.
		//! XML tag: Structure
		void setStructure(const std::string& structure);
		const std::string& structure() const;

		//! Datetime (UTC) the station was created.
		//! XML tag: CreationDate
		void setCreationDate(DateTime creationDate);
		DateTime creationDate() const;

		//! Datetime (UTC) the station was terminated or will be terminated. A
		//! blank value should be assumed to mean that the station is still active
		//! indefinitely.
		//! XML tag: TerminationDate
		void setTerminationDate(const OPT(DateTime)& terminationDate);
		DateTime terminationDate() const throw(Seiscomp::Core::ValueException);

		//! XML tag: NumberRecorders
		void setNumberRecorders(const OPT(int)& numberRecorders);
		int numberRecorders() const throw(Seiscomp::Core::ValueException);

		//! Total number of channels recorded at this station.
		//! XML tag: TotalNumberChannels
		void setTotalNumberChannels(const OPT(int)& totalNumberChannels);
		int totalNumberChannels() const throw(Seiscomp::Core::ValueException);

		//! Number of channels recorded at this station and selected by the query
		//! that produced this StationXML document.
		//! XML tag: SelectedNumberChannels
		void setSelectedNumberChannels(const OPT(int)& selectedNumberChannels);
		int selectedNumberChannels() const throw(Seiscomp::Core::ValueException);

	
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
		bool addEquipment(Equipment* obj);
		bool addOperators(Operator* obj);
		bool addExternalReport(ExternalDocument* obj);
		bool addChannel(Channel* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeEquipment(Equipment* obj);
		bool removeOperators(Operator* obj);
		bool removeExternalReport(ExternalDocument* obj);
		bool removeChannel(Channel* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeEquipment(size_t i);
		bool removeOperators(size_t i);
		bool removeExternalReport(size_t i);
		bool removeChannel(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t equipmentCount() const;
		size_t operatorsCount() const;
		size_t externalReportCount() const;
		size_t channelCount() const;

		//! Index access
		//! @return The object at index i
		Equipment* equipment(size_t i) const;
		Operator* operators(size_t i) const;
		ExternalDocument* externalReport(size_t i) const;
		Channel* channel(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		LatType _latitude;
		LonType _longitude;
		DistanceType _elevation;
		Site _site;
		std::string _vault;
		std::string _geology;
		std::string _structure;
		DateTime _creationDate;
		OPT(DateTime) _terminationDate;
		OPT(int) _numberRecorders;
		OPT(int) _totalNumberChannels;
		OPT(int) _selectedNumberChannels;

		// Aggregations
		std::vector<EquipmentPtr> _equipments;
		std::vector<OperatorPtr> _operatorss;
		std::vector<ExternalDocumentPtr> _externalReports;
		std::vector<ChannelPtr> _channels;
};


}
}


#endif
