/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_XML_H__
#define __SEISCOMP_FDSNXML_XML_H__


#include <seiscomp3/io/xml/importer.h>
#include <seiscomp3/io/xml/exporter.h>


namespace Seiscomp {
namespace FDSNXML {


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
};


}
}


#endif
