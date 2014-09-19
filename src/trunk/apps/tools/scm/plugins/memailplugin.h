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

#ifndef __SEISCOMP_APPLICATIONS_MEMAILPLUGIN_H__
#define __SEISCOMP_APPLICATIONS_MEMAILPLUGIN_H__

#include <seiscomp3/plugins/monitor/monitorplugininterface.h>

#include <seiscomp3/core/datetime.h>

namespace Seiscomp {
namespace Applications {


class EmailMessage {

	public:
		EmailMessage();
		~EmailMessage() {}

	public:
		void setHeader(const std::string& header);
		void setHeaderFilteredClients(const std::string& header);
		void setHeaderRequiredClients(const std::string& header);
		void setHeaderSilentClients(const std::string& header);

		void setFilteredClients(const std::vector<std::string>& clients);
		void setSilentClients(
				const std::vector<std::string>& silentClients,
				const std::vector<std::string>& recoveredClients);
		void setRequiredClients(
				const std::vector<std::string>& missing,
				const std::vector<std::string>& reappeared);

		// 100 characters maximum
		void setUserData(const std::string& data);

		const std::string& message();
		bool empty();
		void clear();

	private:
		void init();

	private:
		bool        _hasChanged;

		std::string _userData;

		std::string _header;

		std::string _headerFilteredClients;
		std::string _filteredClients;

		std::string _headerSilentClients;
		std::string _silentClients;

		std::string _headerRequiredClients;
		std::string _requiredClients;

		std::string _message;

};


class EmailSender {
	public:
		static EmailSender* Create();
		bool sendEmail(const std::string& text, const std::string& recipient);
};




DEFINE_SMARTPOINTER(MEmailPlugin);

class MEmailPlugin : public MonitorPluginInterface {
	DECLARE_SC_CLASS(MEmailPlugin);

	// ----------------------------------------------------------------------
	// Nested types
	// ----------------------------------------------------------------------
	private:
		typedef std::map<std::string, bool> RequiredClients;
		typedef std::list<std::string> SilentClients;

	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		MEmailPlugin();
		virtual ~MEmailPlugin() {}


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		virtual bool init(const Config::Config& cfg);
		virtual void process(const ClientTable& clientTable);


	// ----------------------------------------------------------------------
	// Private interface
	// ----------------------------------------------------------------------
	private:
		void addRecipient(const std::string& recipient);
		void addRecipients(const std::vector<std::string>& recipients);
		const std::vector<std::string>& recipients() const;


	// ----------------------------------------------------------------------
	// Private data members
	// ----------------------------------------------------------------------
	private:
		std::auto_ptr<EmailSender> _sender;
		std::vector<std::string>   _recipients;

		std::string _template;

		Core::TimeSpan                    _filterResponseInterval;
		std::map<std::string, Core::Time> _filterResponseUpdate;

		Core::Time      _requiredClientsTimeMarker;
		Core::TimeSpan  _reportRequiredClientsTimeSpan;
		RequiredClients _requiredClients;
		SilentClients   _silentClients;

		bool           _reportSilentClients;
		Core::TimeSpan _reportSilentClientsTimeSpan;

		bool         _sendEmail;
		EmailMessage _message;

};





} // namespace Applications
} // namespace Seiscomp

#endif
