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

#ifndef __SEISCOMP_COMMUNICATION_PLUGIN_H__
#define __SEISCOMP_COMMUNICATION_PLUGIN_H__

#include <string>
#include <ostream>

#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/communication/systemmessages.h>
#include <seiscomp3/config/config.h>


namespace Seiscomp {
namespace Communication {

DEFINE_SMARTPOINTER(MasterPluginInterface);

class MasterPluginInterface : public Core::BaseObject {
	DECLARE_SC_CLASS(MasterPluginInterface);

public:
	MasterPluginInterface() : _operational(false) {}
	virtual ~MasterPluginInterface() {}

public:
	virtual bool process(NetworkMessage *nmsg, Core::Message *msg) = 0;
	virtual NetworkMessage *service() { return NULL; }
	virtual bool init(Config::Config &conf, const std::string &configPrefix) = 0;
	virtual bool close() = 0;

	//! Creates custom state of health information in the form
	//! name1=value1&name2=value2&
	virtual void printStateOfHealthInformation(std::ostream &) const {};

	bool operational() const { return _operational; }
	void setOperational(bool operational) { _operational = operational; }

private:
	bool _operational;

};

DEFINE_INTERFACE_FACTORY(MasterPluginInterface);

#define REGISTER_MASTER_PLUGIN_INTERFACE(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Communication::MasterPluginInterface, Class> __##Class##InterfaceFactory__(Service)


} // namespace Communication
} // namespace Seiscomp

#endif
