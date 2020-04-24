/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_NETWORK_H__
#define __SEISCOMP_FDSNXML_NETWORK_H__


#include <fdsnxml/metadata.h>
#include <fdsnxml/basenode.h>
#include <vector>
#include <fdsnxml/countertype.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {

DEFINE_SMARTPOINTER(Operator);
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
class Network : public BaseNode {
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
		Network(const Network &other);

		//! Destructor
		~Network();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Network& operator=(const Network &other);
		bool operator==(const Network &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! TotalNumberStations the total number of stations contained in this
		//! network, including inactive or terminated stations.
		//! XML tag: TotalNumberStations
		void setTotalNumberOfStations(const OPT(CounterType)& totalNumberOfStations);
		CounterType& totalNumberOfStations();
		const CounterType& totalNumberOfStations() const;

		//! SelectedNumberStations the total number of stations in this network
		//! that were selected by the query that produced this StationXML
		//! document, even if the stations do not appear in the document. (This
		//! might happen if the user only wants a StationXML document goes no
		//! further than the Network level.)
		//! XML tag: SelectedNumberStations
		void setSelectedNumberStations(const OPT(CounterType)& selectedNumberStations);
		CounterType& selectedNumberStations();
		const CounterType& selectedNumberStations() const;

	
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
		bool addOperators(Operator *obj);
		bool addStation(Station *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeOperators(Operator *obj);
		bool removeStation(Station *obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeOperators(size_t i);
		bool removeStation(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t operatorsCount() const;
		size_t stationCount() const;

		//! Index access
		//! @return The object at index i
		Operator* operators(size_t i) const;
		Station* station(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		OPT(CounterType) _totalNumberOfStations;
		OPT(CounterType) _selectedNumberStations;

		// Aggregations
		std::vector<OperatorPtr> _operatorss;
		std::vector<StationPtr> _stations;
};


}
}


#endif
