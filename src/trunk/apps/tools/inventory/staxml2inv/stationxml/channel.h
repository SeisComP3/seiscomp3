/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_CHANNEL_H__
#define __SEISCOMP_STATIONXML_CHANNEL_H__


#include <stationxml/metadata.h>
#include <vector>
#include <string>
#include <stationxml/date.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {

DEFINE_SMARTPOINTER(ExternalDocument);
DEFINE_SMARTPOINTER(ChannelEpoch);



DEFINE_SMARTPOINTER(Channel);


/**
 * \brief This container describes a channel and at least one of its epochs.
 * \brief Attributes of ChannelType specify the channel code and the SEED
 * \brief channel code if different. The "restricted" attribute also
 * \brief specifies whether data or metadata from the channel is restricted
 * \brief to authenticated users (1) or open(0). Omitting the "restricted"
 * \brief attribute implies the data is open.
 */
class Channel : public Core::BaseObject {
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
		Channel(const Channel& other);

		//! Destructor
		~Channel();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Channel& operator=(const Channel& other);
		bool operator==(const Channel& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: chan_code
		void setCode(const std::string& code);
		const std::string& code() const;

		//! XML tag: seed_chan_code
		void setSeedCode(const std::string& seedCode);
		const std::string& seedCode() const;

		//! XML tag: historical_chan_code
		void setHistoricalCode(const std::string& historicalCode);
		const std::string& historicalCode() const;

		//! XML tag: loc_code
		void setLocationCode(const std::string& locationCode);
		const std::string& locationCode() const;

		//! XML tag: restricted
		void setRestricted(const OPT(int)& restricted);
		int restricted() const throw(Seiscomp::Core::ValueException);

		//! Datetime (UTC) this channel was first created.
		//! XML tag: CreationDate
		void setCreationDate(DateTime creationDate);
		DateTime creationDate() const;

		//! Datetime (UTC) this channel was terminated or will be terminated. If
		//! blank, the channel should be assumed to be active.
		//! XML tag: TerminationDate
		void setTerminationDate(const OPT(DateTime)& terminationDate);
		DateTime terminationDate() const throw(Seiscomp::Core::ValueException);

		//! XML tag: Dataless
		void setDataless(const std::string& dataless);
		const std::string& dataless() const;

	
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
		bool addExternalReport(ExternalDocument* obj);
		bool addEpoch(ChannelEpoch* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeExternalReport(ExternalDocument* obj);
		bool removeEpoch(ChannelEpoch* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeExternalReport(size_t i);
		bool removeEpoch(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t externalReportCount() const;
		size_t epochCount() const;

		//! Index access
		//! @return The object at index i
		ExternalDocument* externalReport(size_t i) const;
		ChannelEpoch* epoch(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _code;
		std::string _seedCode;
		std::string _historicalCode;
		std::string _locationCode;
		OPT(int) _restricted;
		DateTime _creationDate;
		OPT(DateTime) _terminationDate;
		std::string _dataless;

		// Aggregations
		std::vector<ExternalDocumentPtr> _externalReports;
		std::vector<ChannelEpochPtr> _epochs;
};


}
}


#endif
