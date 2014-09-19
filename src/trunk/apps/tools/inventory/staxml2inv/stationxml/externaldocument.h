/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_EXTERNALDOCUMENT_H__
#define __SEISCOMP_STATIONXML_EXTERNALDOCUMENT_H__


#include <stationxml/metadata.h>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {


DEFINE_SMARTPOINTER(ExternalDocument);


/**
 * \brief This type contains a URI and description for external data that
 * \brief users may want to reference in Station XML.
 */
class ExternalDocument : public Core::BaseObject {
	DECLARE_CASTS(ExternalDocument);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ExternalDocument();

		//! Copy constructor
		ExternalDocument(const ExternalDocument& other);

		//! Destructor
		~ExternalDocument();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ExternalDocument& operator=(const ExternalDocument& other);
		bool operator==(const ExternalDocument& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: URI
		void setURI(const std::string& uRI);
		const std::string& uRI() const;

		//! XML tag: Description
		void setDescription(const std::string& description);
		const std::string& description() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:

	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _uRI;
		std::string _description;
};


}
}


#endif
