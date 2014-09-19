/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_STATION_H__
#define __SEISCOMP_STATIONXML_STATION_H__


#include <stationxml/metadata.h>
#include <vector>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {

DEFINE_SMARTPOINTER(StationEpoch);



DEFINE_SMARTPOINTER(Station);


/**
 * \brief Station container. Includes one or more epochs.
 */
class Station : public Core::BaseObject {
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
		Station(const Station& other);

		//! Destructor
		~Station();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Station& operator=(const Station& other);
		bool operator==(const Station& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: net_code
		void setNetCode(const std::string& netCode);
		const std::string& netCode() const;

		//! XML tag: sta_code
		void setCode(const std::string& code);
		const std::string& code() const;

		//! XML tag: sta_number
		void setStaNumber(const OPT(int)& staNumber);
		int staNumber() const throw(Seiscomp::Core::ValueException);

		//! XML tag: agency_code
		void setAgencyCode(const std::string& agencyCode);
		const std::string& agencyCode() const;

		//! XML tag: deployment_code
		void setDeploymentCode(const std::string& deploymentCode);
		const std::string& deploymentCode() const;

	
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
		bool addEpoch(StationEpoch* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeEpoch(StationEpoch* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeEpoch(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t epochCount() const;

		//! Index access
		//! @return The object at index i
		StationEpoch* epoch(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _netCode;
		std::string _code;
		OPT(int) _staNumber;
		std::string _agencyCode;
		std::string _deploymentCode;

		// Aggregations
		std::vector<StationEpochPtr> _epochs;
};


}
}


#endif
