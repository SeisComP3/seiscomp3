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


#ifndef __SEISCOMP3_IO_GFARCHIVE_H__
#define __SEISCOMP3_IO_GFARCHIVE_H__


#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/math/coord.h>
#include <seiscomp3/core.h>


namespace Seiscomp {

namespace Core {

class GreensFunction;

}

namespace IO {


struct GFSource : Math::Geo::CoordD {
	explicit GFSource(double lat = 0.0, double lon = 0.0, double d = 0.0)
	: Math::Geo::CoordD(lat, lon), depth(d) {}

	double depth; //!< Depth in kilometer
};


struct GFReceiver : Math::Geo::CoordD {
	explicit GFReceiver(double lat = 0.0, double lon = 0.0, double e = 0.0)
	: Math::Geo::CoordD(lat, lon), elevation(e) {}

	double elevation; //!< Elevation in meter
};


DEFINE_SMARTPOINTER(GFArchive);

class SC_SYSTEM_CORE_API GFArchive : public Core::BaseObject {
	DECLARE_SC_CLASS(GFArchive);

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! C'tor
		GFArchive();

	public:
		//! Virtual d'tor
		virtual ~GFArchive();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		virtual bool setSource(std::string) = 0;
		virtual void close() = 0;

		virtual std::list<std::string> availableModels() const;
		virtual std::list<double> availableDepths(const std::string &model) const;

		//! Sets the default timespan used for requests without
		//! a timespan.
		virtual bool setTimeSpan(const Core::TimeSpan &span) = 0;

		//! Adds a request for a greensfunction.
		virtual bool addRequest(const std::string &id,
		                        const std::string &model,
		                        const GFSource &source,
		                        const GFReceiver &receiver) = 0;

		virtual bool addRequest(const std::string &id,
		                        const std::string &model,
		                        const GFSource &source,
		                        const GFReceiver &receiver,
		                        const Core::TimeSpan &span) = 0;

		//! Retrieves a greensfunction from the archive. The end of
		//! sequence is marked with a NULL pointer.
		virtual Core::GreensFunction* get() = 0;

		/**
		 * @brief Returns the travel time of a given phase from the source
		 *        to the receiver.
		 * @param phase The phase code
		 * @param model The requested model.
		 * @param source The source location.
		 * @param receiver The receiver location.
		 * @return The relative travel time in seconds. If not supported then
		 *         None must be returned.
		 */
		virtual OPT(double) getTravelTime(const std::string &phase,
		                                  const std::string &model,
		                                  const GFSource &source,
		                                  const GFReceiver &receiver) = 0;


	public:
		static GFArchive* Create(const char* service);
		static GFArchive* Open(const char* url);
};


DEFINE_INTERFACE_FACTORY(GFArchive);


#define REGISTER_GFARCHIVE(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::IO::GFArchive, Class> __##Class##InterfaceFactory__(Service)


}
}


#endif
