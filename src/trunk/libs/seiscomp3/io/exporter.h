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


#ifndef __SEISCOMP_IO_EXPORTER_H__
#define __SEISCOMP_IO_EXPORTER_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/core.h>

#include <streambuf>


namespace Seiscomp {
namespace IO {


struct SC_SYSTEM_CORE_API ExportSink {
	virtual ~ExportSink() {}
	virtual int write(const char *data, int size) { return 0; }
};


typedef std::vector<Core::BaseObject*> ExportObjectList;


DEFINE_SMARTPOINTER(Exporter);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief An abstract exporter interface and factory

	This class provides an interface to export sc3 (meta) data
	formats into any other format such as QuakeML.
	\endcode
 */
class SC_SYSTEM_CORE_API Exporter : public Core::BaseObject {
	DECLARE_SC_CLASS(Exporter);


	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		Exporter();


	public:
		//! Destructor
		virtual ~Exporter();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		static Exporter *Create(const char *type);

		void setFormattedOutput(bool enable);
		void setIndent(int);

		bool write(std::streambuf* buf, Core::BaseObject *);
		bool write(std::string filename, Core::BaseObject *);

		//! Converts the object using the Sink interface to write
		//! the data.
		bool write(ExportSink *sink, Core::BaseObject *);

		bool write(std::streambuf* buf, const ExportObjectList &objects);
		bool write(std::string filename, const ExportObjectList &objects);

		//! Converts the objects using the Sink interface to write
		//! the data.
		bool write(ExportSink *sink, const ExportObjectList &objects);


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		//! Interface method that must be implemented by real exporters.
		virtual bool put(std::streambuf* buf, Core::BaseObject *) = 0;

		//! Interface method that should be implemented by real exporters. The
		//! default implementation does nothing and returns false. Since that
		//! method has been introduced with API 12 it is not abstract to
		//! maintain compilation of existing exporters.
		virtual bool put(std::streambuf* buf, const ExportObjectList &objects);


	protected:
		bool _prettyPrint;
		int  _indentation;
};


DEFINE_INTERFACE_FACTORY(Exporter);

#define REGISTER_EXPORTER_INTERFACE(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::IO::Exporter, Class> __##Class##InterfaceFactory__(Service)

}
}


#endif
