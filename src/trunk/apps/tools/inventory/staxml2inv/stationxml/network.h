/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_NETWORK_H__
#define __SEISCOMP_STATIONXML_NETWORK_H__


#include <stationxml/metadata.h>
#include <vector>
#include <stationxml/date.h>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {

DEFINE_SMARTPOINTER(Station);



DEFINE_SMARTPOINTER(Network);


/**
 * \brief This type represents the Network layer. Its attributes are network
 * \brief code and, optionally, the total number of stations in the network.
 * \brief This number may differ from the actual number of Stations in the
 * \brief StationXML file. The official name of the network or other
 * \brief descriptive information can be included in the Description element.
 * \brief The Network can contain 0 or more Stations.
 */
class Network : public Core::BaseObject {
	DECLARE_CASTS(Network);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Network();

		//! Copy constructor
		Network(const Network& other);

		//! Destructor
		~Network();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Network& operator=(const Network& other);
		bool operator==(const Network& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: StartDate
		void setStart(const OPT(DateTime)& start);
		DateTime start() const throw(Seiscomp::Core::ValueException);

		//! XML tag: EndDate
		void setEnd(const OPT(DateTime)& end);
		DateTime end() const throw(Seiscomp::Core::ValueException);

		//! XML tag: Description
		void setDescription(const std::string& description);
		const std::string& description() const;

		//! TotalNumberStations the total number of stations contained in this
		//! network, including inactive or terminated stations.
		//! XML tag: TotalNumberStations
		void setTotalNumberOfStations(const OPT(int)& totalNumberOfStations);
		int totalNumberOfStations() const throw(Seiscomp::Core::ValueException);

		//! SelectedNumberStations the total number of stations in this network
		//! that were selected by the query that produced this StationXML
		//! document, even if the stations do not appear in the document. (This
		//! might happen if the user only wants a StationXML document goes no
		//! further than the Network level.)
		//! XML tag: SelectedNumberStations
		void setSelectedNumberStations(const OPT(int)& selectedNumberStations);
		int selectedNumberStations() const throw(Seiscomp::Core::ValueException);

		//! XML tag: net_code
		void setCode(const std::string& code);
		const std::string& code() const;

	
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
		bool addStation(Station* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeStation(Station* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeStation(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t stationCount() const;

		//! Index access
		//! @return The object at index i
		Station* station(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		OPT(DateTime) _start;
		OPT(DateTime) _end;
		std::string _description;
		OPT(int) _totalNumberOfStations;
		OPT(int) _selectedNumberStations;
		std::string _code;

		// Aggregations
		std::vector<StationPtr> _stations;
};


}
}


#endif
