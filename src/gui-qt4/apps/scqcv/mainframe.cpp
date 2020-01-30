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




#define SEISCOMP_COMPONENT Gui::QcView

#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/core/datamessage.h>
#include <seiscomp3/core/message.h>
#include <seiscomp3/logging/log.h>

#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/datamodel/configstation.h>
#include <seiscomp3/datamodel/stream.h>
#include <seiscomp3/datamodel/network.h>
#include <seiscomp3/datamodel/inventory.h>
#include <seiscomp3/datamodel/inventory_package.h>
#include <seiscomp3/datamodel/config_package.h>
#include <seiscomp3/client/inventory.h>

#include "mainframe.h"

#include <QMessageBox>

using namespace Seiscomp::DataModel;

namespace Seiscomp {
namespace Gui {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MainFrame::MainFrame() {
	_ui.setupUi(this);

	SCApp->setAutoApplyNotifierEnabled(false);
	SCApp->setInterpretNotifierEnabled(false);

	// don't show alertViewTab
 	_ui.tabWidget->removeTab(1);

	_ui.menuOptions->insertAction(_ui.actionAutoSelect, _actionShowSettings);
	_ui.menuView->addAction(_actionToggleFullScreen);

	this->setWindowTitle("QcView");

	// action buttons
	addAction(_ui.actionAutoSelect);
	_ui.actionAutoSelect->setChecked(true);

	_ui.tabWidget->setCurrentIndex(0);

	// get configuration
	_config = new QcViewConfig(SCApp);

	// create the model, holding all the data
	_qcModel = new QcModel(_config);

	// optional: pre-fill the model with streams
	_qcModel->setStreams(streams());

	// create the views and associate them with the model

	//! REPORT MESSAGES
	_qcReportView = new QcTableView(_qcModel);
	_qcReportView->setRecordStreamURL(SCApp->recordStreamURL());
	_qcReportView->setDatabaseQueryInterface(SCApp->query());
	QLayout *Rlayout = new QVBoxLayout(_ui.tabQcReport);
	Rlayout->setMargin(2);
	Rlayout->addWidget(_qcReportView);

	//! OVERVIEW
	_qcOverView = new QcOverView(_qcModel, this);
	_qcOverView->setRecordStreamURL(SCApp->recordStreamURL());
	_qcOverView->setDatabaseQueryInterface(SCApp->query());
	QLayout *Olayout = new QVBoxLayout(_ui.tabQcOver);
	Olayout->setMargin(2);
	Olayout->addWidget(_qcOverView);

	connect(SCApp, SIGNAL(messageAvailable(Seiscomp::Core::Message*, Seiscomp::Communication::NetworkMessage*)),
	        this, SLOT(readMessage(Seiscomp::Core::Message*, Seiscomp::Communication::NetworkMessage*)) );

	connect(_qcModel, SIGNAL(stationStateChanged(QString, bool)), this, SLOT(sendStationState(QString, bool)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MainFrame::~MainFrame() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainFrame::toggleDock() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainFrame::toggledFullScreen(bool isFullScreen) {
	if ( menuBar() )
		menuBar()->setVisible(!isFullScreen);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainFrame::readMessage(Core::Message *msg, Communication::NetworkMessage *nmsg) {
	//! handle data messages
	Core::DataMessage *dataMessage = Core::DataMessage::Cast(msg);
	if ( dataMessage != NULL ) {
		// read contents
		for ( Core::DataMessage::iterator it = dataMessage->begin(); it != dataMessage->end(); ++it ) {
			// QualitiyControl messages
			DataModel::WaveformQuality *waveformQuality = DataModel::WaveformQuality::Cast(*it);
			if ( waveformQuality != NULL )
				_qcModel->setWaveformQuality(waveformQuality);
		}

		return;
	}

	//! handle Notifier Messages
	DataModel::NotifierMessage *notifierMessage = DataModel::NotifierMessage::Cast(msg);
	if ( notifierMessage != NULL ) {
		// Read contents
		for ( DataModel::NotifierMessage::iterator it = notifierMessage->begin(); it != notifierMessage->end(); ++it ) {
			// Station-Config messages
			DataModel::ConfigStation *configStation = DataModel::ConfigStation::Cast((*it)->object());
			if ( configStation != NULL )
				_qcModel->setStationEnabled(QString(configStation->networkCode().c_str()), QString(configStation->stationCode().c_str()), configStation->enabled());
		}

		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



bool compareChannelCode(const QString& left, const QString& right) {
	if ( left.count() < 2 || right.count() < 2 ) return false;
	return (left[0] == right[0] || left[0] == '?' || right[0] == '?') &&
	       (left[1] == right[1] || left[1] == '?' || right[1] == '?');
}


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::list<std::pair<std::string, bool> > MainFrame::streams() {
	std::list<std::pair<std::string, bool> > streamIDs;
	typedef QPair<QString,int> ChannelEntry;
	typedef QMap<QString, DataModel::ConfigStation*> ConfigStationMap;
	ConfigStationMap configStationMap;
	QMap<QString, QMap<QString, QMultiMap<QString, ChannelEntry> > > streamMap;
	QList<DataModel::WaveformStreamID> requestMap;

	bool usePreconfigured = false;

	try {
		std::vector<std::string> vstreams = SCApp->configGetStrings("streams.codes");
		if ( vstreams.empty() ) usePreconfigured = true;
		else if ( vstreams[0] == "default" ) usePreconfigured = true;

		QStringList streams;
		for ( size_t i = 0; i < vstreams.size(); ++i )
			streams << vstreams[i].c_str();

		int index = 0;
		foreach ( const QString& stream, streams ) {
			QStringList tokens = stream.split(".");

			if ( tokens.count() >= 1 ) {
				if ( tokens.count() > 4 )
					std::cout << "error in entry '" << stream.toStdString() << "': too many tokens, missing ',' ? -> ignoring" << std::endl;
				else
					requestMap.append(DataModel::WaveformStreamID(tokens[0].toStdString(),
					                                   tokens.count()>1?tokens[1].toStdString():"*",
					                                   tokens.count()>2?tokens[2].toStdString():"*",
					                                   tokens.count()>3?tokens[3].toStdString():"*",""));

				QMap<QString, QMultiMap<QString, ChannelEntry> > & stationMap = streamMap[requestMap.last().networkCode().c_str()];
				stationMap[requestMap.last().stationCode().c_str()].insert(requestMap.last().locationCode().c_str(), ChannelEntry(requestMap.last().channelCode().c_str(),index));
			}

			++index;
		}
	}
	catch ( ... ) {
		usePreconfigured = true;
	}

	if ( usePreconfigured ) {
		std::cout << "using configured streams of config module" << std::endl;
		//streamMap["*"]["*"].insert("*", ChannelEntry("*",0));

		DataModel::ConfigModule* module = SCApp->configModule();

		if ( module ) {
			int index = 0;
	
			for ( size_t j = 0; j < module->configStationCount(); ++j ) {
				DataModel::ConfigStation* station = module->configStation(j);

				configStationMap.insert(QString((station->networkCode()+"."+station->stationCode()).c_str()), station);

				DataModel::Setup *setup = DataModel::findSetup(station, SCApp->name());
				if ( setup ) {
					DataModel::ParameterSet *ps = DataModel::ParameterSet::Find(setup->parameterSetID());
					if ( !ps ) {
						SEISCOMP_ERROR("Cannot find parameter set %s", setup->parameterSetID().c_str());
						continue;
					}
	
					std::string net, sta, loc, cha;
					net = station->networkCode();
					sta = station->stationCode();
					for ( size_t n = 0; n < ps->parameterCount(); ++n) {
						DataModel::Parameter* par = ps->parameter(n);
						if ( par->name() == "detecLocid" )
							loc = par->value();
						else if ( par->name() == "detecStream" )
							cha = par->value();
					}

					if ( !cha.empty() ) {
						bool isFixedChannel = cha.size() > 2;

						if ( !isFixedChannel ) {
							DataModel::SensorLocation *sloc =
								Client::Inventory::Instance()->getSensorLocation(net, sta, loc, Core::Time::GMT());

							if ( sloc != NULL ) {
								DataModel::Stream *stream = DataModel::getVerticalComponent(sloc, cha.c_str(), Core::Time::GMT());
								if ( stream != NULL )
									cha = stream->code();
								else
									cha += 'Z';
							}
							else
								cha += 'Z';
						}

						std::cerr << " * " << net << "." << sta << "." << loc << "." << cha << std::endl;

						requestMap.append(DataModel::WaveformStreamID(net, sta, loc, cha, ""));

						QMap<QString, QMultiMap<QString, ChannelEntry> > & stationMap = streamMap[requestMap.last().networkCode().c_str()];
						stationMap[requestMap.last().stationCode().c_str()].insert(requestMap.last().locationCode().c_str(), ChannelEntry(requestMap.last().channelCode().c_str(),index));

						++index;
					}
				}
			}
		}
	}



	Inventory* inv = Seiscomp::Client::Inventory::Instance()->inventory();
	if ( inv == NULL ) {
		QMessageBox::information(this, "Error", "Could not read inventory.\nProceeding without presetting columns.");
		return streamIDs;
	}

	for ( size_t i = 0; i < inv->networkCount(); ++i ) {
		Network* net = inv->network(i);
		try {
			if ( net->end() < Core::Time::GMT() ) continue;
		}
		catch ( ... ) {}

		foreach ( const DataModel::WaveformStreamID& wfsi, requestMap ) {
			if ( wfsi.networkCode() == "*" ) {
				DataModel::WaveformStreamID nwfsi(net->code(), wfsi.stationCode(),
				                       wfsi.locationCode(), wfsi.channelCode(), "");
				if ( requestMap.contains(nwfsi) ) continue;
				requestMap.append(nwfsi);
			}
		}

		QMap<QString, QMultiMap<QString, ChannelEntry> >& staCodes = streamMap[net->code().c_str()];
		if ( staCodes.isEmpty() ) staCodes = streamMap["*"];

		if ( staCodes.isEmpty() ) continue;

		for ( size_t j = 0; j < net->stationCount(); ++j ) {
			Station* sta = net->station(j);
			try {
				if ( sta->end() < Core::Time::GMT() ) continue;
			}
			catch ( ... ) {}


			foreach ( const DataModel::WaveformStreamID& wfsi, requestMap ) {
				if ( wfsi.stationCode() == "*" && wfsi.networkCode() == net->code() ) {
					DataModel::WaveformStreamID nwfsi(wfsi.networkCode(), sta->code(),
					                       wfsi.locationCode(), wfsi.channelCode(), "");
					if ( requestMap.contains(nwfsi) ) continue;
					requestMap.append(nwfsi);
				}
			}

			QMultiMap<QString, ChannelEntry>& locCodes = staCodes[sta->code().c_str()];
			if ( locCodes.isEmpty() ) locCodes = staCodes["*"];

			if ( locCodes.isEmpty() ) continue;

			for ( size_t l = 0; l < sta->sensorLocationCount(); ++l ) {
				SensorLocation *loc = sta->sensorLocation(l);

				try {
					if ( loc->end() < Core::Time::GMT() ) continue;
				}
				catch ( ... ) {}

				QList<ChannelEntry> chaCodes = locCodes.values(loc->code().c_str());
				if ( chaCodes.isEmpty() ) chaCodes = locCodes.values("*");
				if ( chaCodes.isEmpty() ) continue;

				for ( size_t s = 0; s < loc->streamCount(); ++s ) {
					Stream* stream = loc->stream(s);

					try {
						if ( stream->end() < Core::Time::GMT() ) continue;
					}
					catch ( ... ) {}

					QString compCode;

					bool foundChaCode = false;

					foreach ( const ChannelEntry& chaCode, chaCodes ) {
						if ( chaCode.first == "*" || Core::wildcmp(chaCode.first.toStdString(), stream->code()) )
							foundChaCode = true;
					}

					if ( !foundChaCode ) continue;

					bool enabled = true;
					if ( configStationMap.contains(QString((net->code()+"."+sta->code()).c_str())) )
						enabled = configStationMap.value(QString((net->code()+"."+sta->code()).c_str()))->enabled();

					streamIDs.push_back(std::make_pair(net->code()+"."+sta->code()+"."+loc->code()+"."+stream->code(), enabled));
				}
			}
		}
	}

	std::cout << "Adding " << streamIDs.size() << " streams" << std::endl;


	return streamIDs;

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::ConfigStation* MainFrame::configStation(const std::string& networkCode,
                                                    const std::string& stationCode) const {
	if ( !SCApp->configModule() )
		return NULL;

	for ( size_t i = 0; i < SCApp->configModule()->configStationCount(); ++i ) {
		DataModel::ConfigStation* cs = SCApp->configModule()->configStation(i);
		if ( cs->networkCode() == networkCode &&
		     cs->stationCode() == stationCode )
			return cs;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainFrame::prepareNotifier(QString streamID, bool enable) {
	const std::string& networkCode = streamID.split(".")[0].toLatin1().data();
	const std::string& stationCode = streamID.split(".")[1].toLatin1().data();

	DataModel::ConfigStation* cs = configStation(networkCode, stationCode);
	if ( !cs ) {
		ConfigModule *module = SCApp->configModule();
		if ( !module ) {
			SEISCOMP_WARNING("Settings the station state is disabled, no configmodule available");
			return;
		}

		DataModel::ConfigStationPtr newCs = DataModel::ConfigStation::Create("Config/" + module->name() + "/" + networkCode + "/" + stationCode);
		newCs->setNetworkCode(networkCode);
		newCs->setStationCode(stationCode);
		newCs->setEnabled(enable);

		CreationInfo ci;
		ci.setAuthor(SCApp->author());
		ci.setAgencyID(SCApp->agencyID());
		ci.setCreationTime(Core::Time::GMT());

		newCs->setCreationInfo(ci);

		Notifier::Enable();
		module->add(newCs.get());
		Notifier::Disable();
	}
	else {
		cs->setEnabled(enable);
		SEISCOMP_INFO("Set station %s.%s state to: %d",
		              cs->networkCode().c_str(),
		              cs->stationCode().c_str(),
		              enable);

		CreationInfo *ci;
		try {
			ci = &cs->creationInfo();
			ci->setModificationTime(Core::Time::GMT());
		}
		catch ( ... ) {
			cs->setCreationInfo(CreationInfo());
			ci = &cs->creationInfo();
			ci->setCreationTime(Core::Time::GMT());
		}

		ci->setAuthor(SCApp->author());
		ci->setAgencyID(SCApp->agencyID());

		Notifier::Enable();
		cs->update();
		Notifier::Disable();
	}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MainFrame::sendStationState(QString streamID, bool enable) {

	prepareNotifier(streamID, enable);

	Core::MessagePtr msg = Notifier::GetMessage(true);
	
	if ( msg ) {
		if (! SCApp->sendMessage("CONFIG", msg.get())) {
			prepareNotifier(streamID, !enable);
			_qcModel->setStreams(streams());
		}
		return true;
	}
	
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}
}
