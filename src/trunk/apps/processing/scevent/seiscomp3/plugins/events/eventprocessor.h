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

#ifndef __SEISCOMP_APPLICATIONS_EVENTPROCESSOR_H__
#define __SEISCOMP_APPLICATIONS_EVENTPROCESSOR_H__


#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/config/config.h>
#include <seiscomp3/plugins/events/api.h>


namespace Seiscomp {


namespace DataModel {

class Event;

}


namespace Client {


DEFINE_SMARTPOINTER(EventProcessor);


class SC_EVPLUGIN_API EventProcessor : public Seiscomp::Core::BaseObject {
	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		EventProcessor();


	// ----------------------------------------------------------------------
	// Virtual public interface
	// ----------------------------------------------------------------------
	public:
		//! Setup all configuration parameters
		virtual bool setup(const Config::Config &config) = 0;

		//! Processes an event. The preferred object (Origin, Magnitude,
		//! FocalMechanism) are guaranteed to be found with *::Find(id)
		//! methods.
		//! This method should return true if the event objects needs
		//! an update.
		virtual bool process(DataModel::Event *event) = 0;
};


DEFINE_INTERFACE_FACTORY(EventProcessor);


}
}


#define REGISTER_EVENTPROCESSOR(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Client::EventProcessor, Class> __##Class##InterfaceFactory__(Service)


#endif
