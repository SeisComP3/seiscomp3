/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_XML_H__
#define __SEISCOMP_STATIONXML_XML_H__


#include <seiscomp3/io/xml/importer.h>
#include <seiscomp3/io/xml/exporter.h>


namespace Seiscomp {
namespace StationXML {


class Importer : public Seiscomp::IO::XML::Importer {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		Importer();
};


class Exporter : public Seiscomp::IO::XML::Exporter {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		Exporter();


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		virtual void collectNamespaces(Core::BaseObject *);
};


}
}


#endif
