/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_IO_IMPORTER_H__
#define __SEISCOMP_IO_IMPORTER_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/core.h>

#include <streambuf>
#include <string>


namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(Importer);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief An abstract importer interface and factory

	This class provides an interface to import foreign (meta) data
	formats such as QuakeML.
	\endcode
 */
class SC_SYSTEM_CORE_API Importer : public Core::BaseObject {
	DECLARE_SC_CLASS(Importer);
 
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		Importer();


	public:
		//! Destructor
		virtual ~Importer();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		static Importer *Create(const char *type);

		Core::BaseObject *read(std::streambuf* buf);
		Core::BaseObject *read(std::string filename);

		bool withoutErrors() const;

	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		//! Interface method that must be implemented by real importers.
		virtual Core::BaseObject *get(std::streambuf* buf) = 0;


	protected:
		bool _hasErrors;
};


DEFINE_INTERFACE_FACTORY(Importer);

#define REGISTER_IMPORTER_INTERFACE(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::IO::Importer, Class> __##Class##InterfaceFactory__(Service)

}
}


#endif
