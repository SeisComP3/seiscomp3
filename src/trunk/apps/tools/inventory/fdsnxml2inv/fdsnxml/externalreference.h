/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_EXTERNALREFERENCE_H__
#define __SEISCOMP_FDSNXML_EXTERNALREFERENCE_H__


#include <fdsnxml/metadata.h>
#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {


DEFINE_SMARTPOINTER(ExternalReference);


/**
 * \brief This type contains a URI and description for external data that
 * \brief users may want to reference in StationXML.
 */
class ExternalReference : public Core::BaseObject {
	DECLARE_CASTS(ExternalReference);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ExternalReference();

		//! Copy constructor
		ExternalReference(const ExternalReference &other);

		//! Destructor
		~ExternalReference();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ExternalReference& operator=(const ExternalReference &other);
		bool operator==(const ExternalReference &other) const;


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
