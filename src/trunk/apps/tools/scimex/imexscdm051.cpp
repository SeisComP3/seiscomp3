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

#include <iostream>

#include "imexscdm051.h"
#include "imexmessage.h"


using namespace Seiscomp;
using namespace DataModel;


REGISTER_IMPORTER_INTERFACE(ImexImporterScDm051, "imexscdm0.51");
REGISTER_EXPORTER_INTERFACE(ImexExporterScDm051, "imexscdm0.51");

namespace {


struct ImexMessageHandler : public IO::XML::ClassHandler {
	ImexMessageHandler() {}

	bool get(Core::BaseObject *obj, void *n) {
		SCDM051::GenericHandler gh;
		if ( !gh.get(obj, n) )
			return false;

		(IO::XML::NodeHandler&)(*this) = gh;
		return true;
	}

	bool finalize(Core::BaseObject *obj, IO::XML::ChildList *cl) {
		for ( IO::XML::ChildList::iterator it = cl->begin(); it != cl->end(); ++it ) {
			DataModel::NotifierMessagePtr notifierMsg = DataModel::NotifierMessage::Cast(*it);
			if ( !notifierMsg ) continue;
			DataModel::NotifierMessage::iterator notifierIt = notifierMsg->begin();
			for ( ; notifierIt != notifierMsg->end(); ++notifierIt ) {
				Notifier *notifier = Notifier::Cast(*notifierIt);
				if ( notifier ) {
					bool success = static_cast<Applications::IMEXMessage*>(obj)->notifierMessage().attach(notifier);
					if ( success ) *it = NULL;
				}
			}
		}
		return true;
	}

	bool put(Core::BaseObject *obj, const char *tag, const char *ns, IO::XML::OutputHandler *output) {
		output->openElement(tag, ns);

		Applications::IMEXMessage* imexMessage = Applications::IMEXMessage::Cast(obj);
		NotifierMessage *nm = static_cast<NotifierMessage*>(&imexMessage->notifierMessage());
		output->handle(nm, "notifierMessage", NULL);

		output->closeElement(tag, ns);

		return true;
	}

};

ImexMessageHandler __imexMessageHandler;

} // namespace



ImexImporterScDm051::ImexImporterScDm051() {
	typeMap()->registerMapping<Applications::IMEXMessage>("IMEXMessage", "", &__imexMessageHandler);
}


ImexExporterScDm051::ImexExporterScDm051() {
	typeMap()->registerMapping<Applications::IMEXMessage>("IMEXMessage", "", &__imexMessageHandler);
}

