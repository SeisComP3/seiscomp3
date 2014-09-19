/*===========================================================================================================================
    Name:       dataless.h

    Purpose:    setting up synchronization between GDI and SeisComp

    Language:   C++, ANSI standard.

    Author:     Peter de Boer

    Revision:	2007-11-26	0.1	initial version

===========================================================================================================================*/
#ifndef DATALESS_H
#define DATALESS_H

#include <sstream>
#ifndef WIN32
#include <unistd.h>
#endif
#include "inventory.h"
#include "define.h"
#include "tmanip.h"
#include "mystring.h"
#include "seed.h"

class Dataless
{
	public:
		Dataless(const std::string &dcid, const std::string &net_description, const std::string &net_type,
			const Seiscomp::Core::Time &net_start, const OPT(Seiscomp::Core::Time) &net_end,
			bool temporary, bool restricted, bool shared):
			_dcid(dcid), _net_description(net_description), _net_type(net_type),
			_net_start(net_start), _net_end(net_end), _temporary(temporary),
			_restricted(restricted), _shared(shared) {};
		bool SynchronizeDataless(Seiscomp::DataModel::Inventory *inv,
		                         const std::string &dataless);

	private:
		std::string _dcid;
		std::string _net_description;
		std::string _net_type;
		Seiscomp::Core::Time _net_start;
		OPT(Seiscomp::Core::Time) _net_end;
		bool _temporary;
		bool _restricted;
		bool _shared;
		bool _dump;
		Inventory *invent;
		bool ParseDataless(const std::string &file);
};
#endif /* DATALESS_H */
