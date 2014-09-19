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



#ifndef __SEISCOMP_APPLICATIONS_IMEX_H__
#define __SEISCOMP_APPLICATIONS_IMEX_H__

#include <string>
#include <list>
#include <map>
#include <utility>

#include <boost/shared_ptr.hpp>

#include <seiscomp3/communication/systemmessages.h>
#include <seiscomp3/communication/systemconnection.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/client/application.h>
#include <seiscomp3/core/datamessage.h>


namespace Seiscomp {
namespace Applications {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class ImExImpl;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class ImEx : public Client::Application {

	// ----------------------------------------------------------------------
	// Nested Types
	// ----------------------------------------------------------------------
	public:
		enum Mode { IMPORT = 0x0, EXPORT, UNDEFINED, Mode_COUNT };

		typedef std::list<DataModel::PickPtr>      PickList;
		typedef std::list<DataModel::AmplitudePtr> AmplitudeList;
		typedef std::list<DataModel::OriginPtr>    OriginList;
		typedef std::list<DataModel::EventPtr>     EventList;

	private:
		typedef std::list<std::pair<Core::Time, Core::MessagePtr> > MessageList;

	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		ImEx(int argc, char* argv[]);
		~ImEx();

		const PickList&       pickList() const;
		const AmplitudeList&  amplitudeList() const;
		const OriginList&     originList() const;
		const Mode&           mode() const;
		const Core::TimeSpan& cleanUpInterval() const;


	protected:
		virtual bool init();
		virtual void handleNetworkMessage(const Communication::NetworkMessage* msg);
		virtual void handleMessage(Core::Message* message);
		virtual void done();
		virtual bool validateParameters();
		virtual void createCommandLineDescription();

		// ------------------------------------------------------------------
		// Private interface
		// ------------------------------------------------------------------
	private:
		void updateData(DataModel::NotifierMessage* notifierMessage);
		void updateData(Core::DataMessage* dataMessage);
		void updateEventData(DataModel::Event* event);

		Core::MessagePtr convertMessage(Core::Message* message);
		void dispatchMessage(Core::Message* msg);

		template <typename T>
		void cleanUp(T& container);
		void cleanUp(EventList& container);
		void cleanUp(AmplitudeList& container);
		template <typename T>
		void cleanUpSpecial(T& container);


		// ----------------------------------------------------------------------
		// Public data members
		// ----------------------------------------------------------------------
	public:
		PickList      _pickList;
		AmplitudeList _amplitudeList;
		OriginList    _originList;

		Mode           _mode;
		Core::TimeSpan _cleanUpInterval;


		// ------------------------------------------------------------------
		// Private data members
		// ------------------------------------------------------------------
	private:
		std::vector<boost::shared_ptr<ImExImpl> > _imexImpls;
		Core::Time                                _lastCleanUp;
		EventList                                 _eventList;

		Communication::NetworkMessageCPtr         _lastNetworkMessage;
		std::string                               _importMessageConversion;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





} // namepsace Applications
} // namespace Seiscomp

#endif
