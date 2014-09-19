/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_EPOCH_H__
#define __SEISCOMP_STATIONXML_EPOCH_H__


#include <stationxml/metadata.h>
#include <stationxml/date.h>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(Epoch);


/**
 * \brief This complex type describes time windows. It is used as a base type
 * \brief for station and channel epochs.
 */
class Epoch : public Core::BaseObject {
	DECLARE_CASTS(Epoch);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Epoch();

		//! Copy constructor
		Epoch(const Epoch& other);

		//! Destructor
		~Epoch();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Epoch& operator=(const Epoch& other);
		bool operator==(const Epoch& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: StartDate
		void setStart(DateTime start);
		DateTime start() const;

		//! XML tag: EndDate
		void setEnd(const OPT(DateTime)& end);
		DateTime end() const throw(Seiscomp::Core::ValueException);

		//! XML tag: Comment
		void setComment(const std::string& comment);
		const std::string& comment() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		DateTime _start;
		OPT(DateTime) _end;
		std::string _comment;
};


}
}


#endif
