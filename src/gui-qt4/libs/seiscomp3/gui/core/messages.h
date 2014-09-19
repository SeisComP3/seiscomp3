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



#ifndef __SEISCOMP_GUI_MESSAGE_H__
#define __SEISCOMP_GUI_MESSAGE_H__

#include <seiscomp3/core/message.h>
#include <seiscomp3/core/enumeration.h>
#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {
namespace Gui {


MAKEENUM(
	Command,
	EVALUES(
		CM_UNDEFINED,
		CM_SHOW_ORIGIN,
		CM_SHOW_STREAMS,
		CM_SHOW_MAGNITUDE,
		CM_OBSERVE_LOCATION
	),
	ENAMES(
		"undefined",
		"show origin",
		"show streams",
		"show magnitude",
		"observe location"
	)
);


DEFINE_SMARTPOINTER(CommandMessage);

class SC_GUI_API CommandMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(CommandMessage);
	DECLARE_SERIALIZATION;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	private:
		CommandMessage();
		CommandMessage(const std::string client, Command command);

	// ------------------------------------------------------------------
	//  Message interface
	// ------------------------------------------------------------------
	public:
		void setParameter(const std::string&);
		void setObject(Core::BaseObject*);

		const std::string& client() const;
		Command command() const;

		const std::string& parameter() const;
		Core::BaseObject* object() const;

		bool empty() const;

	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		std::string _client;
		Command _command;
		std::string _parameter;
		Core::BaseObjectPtr _object;

	DECLARE_SC_CLASSFACTORY_FRIEND(CommandMessage);

	friend class Application;
};


}
}


#endif
