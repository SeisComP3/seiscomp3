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

#ifndef __SEISCOMP_APPLICATIONS_SCOREPROCESSOR_H__
#define __SEISCOMP_APPLICATIONS_SCOREPROCESSOR_H__


#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/config/config.h>
#include <seiscomp3/plugins/events/api.h>


namespace Seiscomp {


namespace DataModel {

class Origin;
class FocalMechanism;

}


namespace Client {


DEFINE_SMARTPOINTER(ScoreProcessor);


class SC_EVPLUGIN_API ScoreProcessor : public Seiscomp::Core::BaseObject {
	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		ScoreProcessor();


	// ----------------------------------------------------------------------
	// Virtual public interface
	// ----------------------------------------------------------------------
	public:
		//! Setup all configuration parameters
		virtual bool setup(const Config::Config &config) = 0;

		//! Evaluates an origin.
		//! This method should return a score value. The higher the score
		//! the higher the origins priority in the process of selecting the
		//! preferred origin.
		virtual double evaluate(DataModel::Origin *origin) = 0;
		virtual double evaluate(DataModel::FocalMechanism *fm) = 0;
};


DEFINE_INTERFACE_FACTORY(ScoreProcessor);


}
}


#define REGISTER_ORIGINSCOREPROCESSOR(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Client::ScoreProcessor, Class> __##Class##InterfaceFactory__(Service)


#endif
