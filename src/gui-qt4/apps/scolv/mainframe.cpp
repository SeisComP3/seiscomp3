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


#define SEISCOMP_COMPONENT Gui::OriginLocatorView

#include "mainframe.h"
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/datamodel/eventsummary.h>
#include <seiscomp3/gui/datamodel/eventsummaryview.h>
#include <seiscomp3/gui/datamodel/originlocatorview.h>
#include <seiscomp3/gui/datamodel/magnitudeview.h>
#include <seiscomp3/gui/datamodel/pickerview.h>
#include <seiscomp3/gui/datamodel/amplitudeview.h>
#include <seiscomp3/gui/datamodel/eventlistview.h>
#include <seiscomp3/gui/datamodel/eventedit.h>
#include <seiscomp3/gui/datamodel/pickersettings.h>
#include <seiscomp3/gui/datamodel/origindialog.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/config/config.h>
#include <seiscomp3/system/environment.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/originquality.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/datamodel/configstation.h>
#include <seiscomp3/datamodel/journalentry.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/core/system.h>

#include <seiscomp3/datamodel/utils.h>

#include <seiscomp3/core/datamessage.h>

#include <QFileDialog>

#include <sstream>
#include <iomanip>
//#include <unistd.h>

#define WITH_SMALL_SUMMARY


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::DataModel;

namespace Seiscomp {
namespace Gui {


namespace {


void readPhaseGroups(PickerView::Config::GroupList &groups, const std::string &root) {
	QSet<QString> localPhases;

	try {
		vector<string> entries = SCApp->configGetStrings(root);
		for ( size_t i = 0; i < entries.size(); ++i ) {
			QString ph = entries[i].c_str();

			if ( localPhases.contains(ph) ) {
				SEISCOMP_WARNING("%s: ignoring duplicate phase: %s",
				                 root.c_str(), entries[i].c_str());
				continue;
			}

			groups.append(PickerView::Config::PhaseGroup());
			groups.last().name = ph;
			localPhases.insert(ph);
		}
	}
	catch ( ... ) {
		return;
	}

	PickerView::Config::GroupList::iterator it;
	for ( it = groups.begin(); it != groups.end(); ++it )
		readPhaseGroups(it->childs, root + "." + it->name.toStdString());
}


void readUncertainties(PickerView::Config::UncertaintyList &list,
                       const vector<string> &params,
                       const string &configParam) {
	for ( size_t i = 0; i < params.size(); ++i ) {
		complex<float> value;

		float symmetricUncertainty;
		if ( Core::fromString(symmetricUncertainty, params[i]) )
			value = complex<float>(symmetricUncertainty, symmetricUncertainty);
		else if ( !Core::fromString(value, params[i]) ) {
			cerr << "ERROR: " << configParam << ": " << i+1
				 << ": invalid token, expected float or pair of float" << endl;
			list.clear();
			break;
		}

		list.append(
			PickerView::Config::Uncertainty(value.real(), value.imag())
		);
	}
}


string trim(const std::string &str) {
	string tmp(str);
	Core::trim(tmp);
	return tmp;
}


}


MainFrame::MainFrame(){
	_ui.setupUi(this);

	SCApp->settings().beginGroup(objectName());

	_expertMode = true;

	_actionConfigureAcquisition = new QAction(this);
	_actionConfigureAcquisition->setObjectName(QString::fromUtf8("configureAcquisition"));
	_actionConfigureAcquisition->setShortcut(QApplication::translate("MainWindow", "F3", 0, QApplication::UnicodeUTF8));
	_actionConfigureAcquisition->setText(QApplication::translate("MainWindow", "Configure &OriginLocatorView...", 0, QApplication::UnicodeUTF8));

	addAction(_actionConfigureAcquisition);

	connect(_actionConfigureAcquisition, SIGNAL(triggered(bool)),
	        this, SLOT(configureAcquisition()));

	_ui.menuView->addAction(_actionToggleFullScreen);

	_ui.menuView->addSeparator();
	QMenu *toolBars = _ui.menuView->addMenu("Toolbars");
	toolBars->addAction(_ui.toolBarEdit->toggleViewAction());

	if ( SCApp->isMessagingEnabled() || SCApp->isDatabaseEnabled() )
		_ui.menuSettings->addAction(_actionShowSettings);

	_ui.menuSettings->addAction(_actionConfigureAcquisition);

	Map::ImageTreePtr mapTree = new Map::ImageTree(SCApp->mapsDesc());

	_eventSmallSummary = NULL;
	_eventSmallSummaryCurrent = NULL;

	try { _expertMode = SCApp->configGetBool("mode.expert"); }
	catch ( ... ) { _expertMode = true; }

	try { _exportScript = SCApp->configGetString("scripts.export"); }
	catch ( ... ) { _exportScript = ""; }

	try { _exportScriptTerminate = SCApp->configGetString("scripts.export.silentTerminate") == "true"; }
	catch ( ... ) { _exportScriptTerminate = false; }


#ifdef WITH_SMALL_SUMMARY
	_ui.frameSummary->setFrameShape(QFrame::NoFrame);
	_eventSmallSummary = new EventSummary(mapTree.get(), SCApp->query(), this);
	_eventSmallSummaryCurrent = new EventSummary(mapTree.get(), SCApp->query(), this);
	QTabWidget *tabWidget = new QTabWidget(_ui.frameSummary);

	QWidget *container = new QWidget;
	QLayout* layout = new QVBoxLayout(container);
	layout->addWidget(_eventSmallSummary);
	tabWidget->addTab(container, "Preferred");

	container = new QWidget;
	layout = new QVBoxLayout(container);
	layout->addWidget(_eventSmallSummaryCurrent);
	tabWidget->addTab(container, "Current");

	layout = new QHBoxLayout(_ui.frameSummary);
	layout->setMargin(0);
	//layout->addWidget(_eventSmallSummary);
	layout->addWidget(tabWidget);
	if ( !_exportScript.empty() ) {
		QPushButton *btn = _eventSmallSummary->exportButton();
		btn->setVisible(true);
		btn->setText("");
		btn->setIcon(QIcon(":/icons/icons/publish.png"));
		btn->setFlat(true);
		btn->setToolTip("Publish event");

		connect(btn, SIGNAL(clicked()),
		        this, SLOT(publishEvent()));
	}

#else
	delete _ui.frameSummary;
#endif

	OriginLocatorView::Config locatorConfig;
	PickerView::Config pickerConfig;
	AmplitudeView::Config amplitudeConfig;
	pickerConfig.recordURL = SCApp->recordStreamURL().c_str();
	amplitudeConfig.recordURL = SCApp->recordStreamURL().c_str();

	pickerConfig.hideStationsWithoutData = false;
	amplitudeConfig.hideStationsWithoutData = false;

	try { _computeMagnitudesAutomatically = SCApp->configGetBool("olv.computeMagnitudesAfterRelocate"); }
	catch ( ... ) { _computeMagnitudesAutomatically = false; }

	try { _computeMagnitudesSilently = SCApp->configGetBool("olv.computeMagnitudesSilently"); }
	catch ( ... ) { _computeMagnitudesSilently = false; }

	try { _askForMagnitudeTypes = SCApp->configGetBool("olv.enableMagnitudeSelection"); }
	catch ( ... ) { _askForMagnitudeTypes = true; }

	try { locatorConfig.reductionVelocityP = SCApp->configGetDouble("olv.Pvel"); }
	catch ( ... ) { locatorConfig.reductionVelocityP = 6.0; }

	try { locatorConfig.drawMapLines = SCApp->configGetBool("olv.drawMapLines"); }
	catch ( ... ) { locatorConfig.drawMapLines = true; }

	try { locatorConfig.drawGridLines = SCApp->configGetBool("olv.drawGridLines"); }
	catch ( ... ) { locatorConfig.drawGridLines = true; }

	try { locatorConfig.computeMissingTakeOffAngles = SCApp->configGetBool("olv.computeMissingTakeOffAngles"); }
	catch ( ... ) { locatorConfig.computeMissingTakeOffAngles = true; }

	try { locatorConfig.defaultEventRadius = SCApp->configGetDouble("olv.map.event.defaultRadius"); }
	catch ( ... ) {}

	try { pickerConfig.showCrossHair = SCApp->configGetBool("picker.showCrossHairCursor"); }
	catch ( ... ) { pickerConfig.showCrossHair = false; }

	try { pickerConfig.ignoreUnconfiguredStations = SCApp->configGetBool("picker.ignoreUnconfiguredStations"); }
	catch ( ... ) { pickerConfig.ignoreUnconfiguredStations = false; }

	try { pickerConfig.loadAllComponents = SCApp->configGetBool("picker.loadAllComponents"); }
	catch ( ... ) { pickerConfig.loadAllComponents = true; }

	try { pickerConfig.loadAllPicks = SCApp->configGetBool("picker.loadAllPicks"); }
	catch ( ... ) { pickerConfig.loadAllPicks = true; }

	try { pickerConfig.loadStrongMotionData = SCApp->configGetBool("picker.loadStrongMotion"); }
	catch ( ... ) { pickerConfig.loadStrongMotionData = false; }

	try { pickerConfig.limitStations = SCApp->configGetBool("picker.limitStationAcquisition"); }
	catch ( ... ) { pickerConfig.limitStations = false; }

	try { pickerConfig.limitStationCount = SCApp->configGetInt("picker.limitStationAcquisitionCount"); }
	catch ( ... ) { pickerConfig.limitStationCount = 10; }

	try { pickerConfig.showAllComponents = SCApp->configGetBool("picker.showAllComponents"); }
	catch ( ... ) { pickerConfig.showAllComponents = false; }

	try { pickerConfig.allComponentsMaximumStationDistance = SCApp->configGetDouble("picker.allComponentsMaximumDistance"); }
	catch ( ... ) { pickerConfig.allComponentsMaximumStationDistance = 10.0; }

	try { pickerConfig.usePerStreamTimeWindows = Core::TimeSpan(SCApp->configGetBool("picker.usePerStreamTimeWindows")); }
	catch ( ... ) { pickerConfig.usePerStreamTimeWindows = false; }

	try { pickerConfig.removeAutomaticStationPicks = SCApp->configGetBool("picker.removeAutomaticPicksFromStationAfterManualReview"); }
	catch ( ... ) { pickerConfig.removeAutomaticStationPicks = false; }

	try { pickerConfig.removeAutomaticPicks = SCApp->configGetBool("picker.removeAllAutomaticPicksAfterManualReview"); }
	catch ( ... ) { pickerConfig.removeAutomaticPicks = false; }

	try { pickerConfig.preOffset = Core::TimeSpan(SCApp->configGetInt("picker.preOffset")); }
	catch ( ... ) { pickerConfig.preOffset = Core::TimeSpan(60); }

	try { pickerConfig.postOffset = Core::TimeSpan(SCApp->configGetInt("picker.postOffset")); }
	catch ( ... ) { pickerConfig.postOffset = Core::TimeSpan(120); }

	try { pickerConfig.minimumTimeWindow = Core::TimeSpan(SCApp->configGetInt("picker.minimumTimeWindow")); }
	catch ( ... ) { pickerConfig.minimumTimeWindow = Core::TimeSpan(1800); }

	try { pickerConfig.alignmentPosition = SCApp->configGetDouble("picker.alignmentPosition"); }
	catch ( ... ) { pickerConfig.alignmentPosition = 0.5; }

	if ( pickerConfig.alignmentPosition < 0 )
		pickerConfig.alignmentPosition = 0;
	else if ( pickerConfig.alignmentPosition > 1 )
		pickerConfig.alignmentPosition = 1;

	try {
		vector<string> uncertaintyProfiles =
			SCApp->configGetStrings("picker.uncertainties.preferred");

		for ( size_t i = 0; i < uncertaintyProfiles.size(); ++i ) {
			try {
				if ( uncertaintyProfiles[i].empty() ) continue;
				string param = "picker.uncertainties.profile." + uncertaintyProfiles[i];
				PickerView::Config::UncertaintyList uncertainties;
				readUncertainties(uncertainties,
				                  SCApp->configGetStrings(param),
				                  param);

				if ( uncertainties.isEmpty() ) continue;

				// Set default profile
				if ( pickerConfig.uncertaintyProfiles.isEmpty() )
					pickerConfig.uncertaintyProfile = uncertaintyProfiles[i].c_str();

				pickerConfig.uncertaintyProfiles[uncertaintyProfiles[i].c_str()] =
					uncertainties;
			}
			catch ( ... ) {}
		}
	}
	catch ( ... ) {}

	if ( pickerConfig.uncertaintyProfiles.isEmpty() ) {
		try {
			PickerView::Config::UncertaintyList uncertainties;
			readUncertainties(uncertainties,
			                  SCApp->configGetStrings("picker.uncertainties"),
			                  "picker.uncertainties");
			if ( !uncertainties.isEmpty() ) {
				pickerConfig.uncertaintyProfile = "default";
				pickerConfig.uncertaintyProfiles[
					pickerConfig.uncertaintyProfile
				] = uncertainties;
			}
		}
		catch ( ... ) {}
	}

	try {
		vector<string> streamMap = SCApp->configGetStrings("picker.streamMapping");
		for ( size_t i = 0; i < streamMap.size(); ++i ) {
			size_t pos = streamMap[i].find(':');
			if ( pos == string::npos ) {
				cerr << "ERROR: picker.streamMapping: " << i+1
				     << ": invalid token, expected ':'" << endl;
				continue;
			}

			string stations = trim(streamMap[i].substr(0,pos));
			string channels = trim(streamMap[i].substr(pos+1));
			if ( stations.empty() ) {
				cerr << "ERROR: picker.streamMapping: " << i+1
				     << ": empty left side (stations), ignoring" << endl;
				continue;
			}

			if ( channels.empty() ) {
				cerr << "ERROR: picker.streamMapping: " << i+1
				     << ": empty right side (channels), ignoring" << endl;
				continue;
			}

			vector<string> tmpStringList;
			if ( Core::split(tmpStringList, channels.c_str(), " ") == 0 ) {
				cerr << "ERROR: picker.streamMapping: " << i+1
				     << ": empty right side (channels), ignoring" << endl;
				continue;
			}

			QList<PickerView::Config::ChannelMapItem> channelMapping;
			bool ok = true;
			for ( size_t c = 0; c < tmpStringList.size(); ++c ) {
				pos = tmpStringList[c].find("->");
				if ( pos == string::npos ) {
					cerr << "ERROR: picker.streamMapping: " << i+1
					     << ": wrong channel map entry, expected '->' operator" << endl;
					ok = false;
					break;
				}

				channelMapping.append(
					PickerView::Config::ChannelMapItem(
						trim(tmpStringList[c].substr(0,pos)).c_str(),
						trim(tmpStringList[c].substr(pos+2)).c_str()
					)
				);
			}

			if ( !ok ) continue;

			if ( Core::split(tmpStringList, stations.c_str(), " ") == 0 ) {
				cerr << "ERROR: picker.streamMapping: " << i+1
				     << ": empty left side (stations), ignoring" << endl;
				continue;
			}

			for ( size_t c = 0; c < tmpStringList.size(); ++c ) {
				QList<PickerView::Config::ChannelMapItem>::iterator it;
				for ( it = channelMapping.begin(); it != channelMapping.end(); ++it )
					pickerConfig.channelMap.insert(tmpStringList[c].c_str(), *it);
			}
		}
	}
	catch ( ... ) {}


	pickerConfig.timingQualityLow = SCApp->configGetColor("scheme.colors.picker.timingQuality.low", pickerConfig.timingQualityLow);
	pickerConfig.timingQualityMedium = SCApp->configGetColor("scheme.colors.picker.timingQuality.medium", pickerConfig.timingQualityMedium);
	pickerConfig.timingQualityHigh = SCApp->configGetColor("scheme.colors.picker.timingQuality.high", pickerConfig.timingQualityHigh);
	/*
	try {
		std::vector<std::string> streamMap = SCApp->configGetStrings("streams.mapping");
		for ( size_t i = 0; i < streamMap.size(); ++i ) {
			std::vector<std::string> toks;
			if ( Core::split(toks, streamMap[i].c_str(), ":") != 2 ) {
				SEISCOMP_WARNING("Invalid stream.mapping[%d]: %s, ignoring", i, streamMap[i].c_str());
				continue;
			}

			std::string sourceStream = Core::trim(toks[0]);
			std::string destStream = Core::trim(toks[1]);

			if ( sourceStream.empty() ) {
				SEISCOMP_WARNING("Source stream of stream.mapping[%d] is empty: %s, ignoring", i, streamMap[i].c_str());
				continue;
			}

			if ( destStream.empty() ) {
				SEISCOMP_WARNING("Destination stream of stream.mapping[%d] is empty: %s, ignoring", i, streamMap[i].c_str());
				continue;
			}

			pickerConfig.streamMap.append(PickerView::Config::MapList::value_type(sourceStream.c_str(), destStream.c_str()));
		}
	}
	catch ( ... ) {}
	*/

	readPhaseGroups(pickerConfig.phaseGroups, "picker.phases.groups");
	if ( pickerConfig.phaseGroups.isEmpty() )
		readPhaseGroups(pickerConfig.phaseGroups, "picker.phases");

	try {
		QSet<QString> localPhases;
		vector<string> phases = SCApp->configGetStrings("picker.phases.favourites");
		for ( vector<string>::iterator it = phases.begin(); it != phases.end(); ++it ) {
			QString ph = it->c_str();
			if ( localPhases.contains(ph) ) {
				SEISCOMP_WARNING("picker.phases.favourites: ignoring duplicate phase: %s",
				                 it->c_str());
				continue;
			}

			pickerConfig.favouritePhases.append(ph);
			localPhases.insert(ph);
		}
	}
	catch ( ... ) {}


	try {
		vector<string> phases = SCApp->configGetStrings("picker.showPhases");
		for ( vector<string>::iterator it = phases.begin();
		      it != phases.end(); ++it ) {
			pickerConfig.addShowPhase((*it).c_str());
		}
	}
	catch ( ... ) {}

	try {
		std::vector<std::string> filters;

		try {
			filters = SCApp->configGetStrings("picker.filters");
		}
		catch ( ... ) {}

		for ( std::vector<std::string>::iterator it = filters.begin();
		      it != filters.end(); ++it ) {
			if ( it->empty() ) continue;
			QString filter = (*it).c_str();
			QStringList tokens = filter.split(";");
			if ( tokens.size() != 2 ) {
				SEISCOMP_ERROR("Wrong filter string, expecting ';' to seperate name and definition: %s", (const char*)filter.toAscii());
				continue;
			}

			pickerConfig.addFilter(tokens[0], tokens[1]);
		}
	}
	catch ( ... ) {
		pickerConfig.addFilter("Teleseismic", "BW(3,0.7,2)");
		pickerConfig.addFilter("Regional", "BW(3,2,6)");
		pickerConfig.addFilter("Local", "BW(3,4,10)");
	}

	try { pickerConfig.repickerSignalStart = SCApp->configGetDouble("picker.repickerStart"); }
	catch ( ... ) {}
	try { pickerConfig.repickerSignalEnd = SCApp->configGetDouble("picker.repickerEnd"); }
	catch ( ... ) {}

	try { pickerConfig.defaultAddStationsDistance = SCApp->configGetDouble("olv.defaultAddStationsDistance"); }
	catch ( ... ) { pickerConfig.defaultAddStationsDistance = 15.0; }

	try { pickerConfig.hideStationsWithoutData = SCApp->configGetBool("olv.hideStationsWithoutData"); }
	catch ( ... ) { pickerConfig.hideStationsWithoutData = false; }

	try { pickerConfig.defaultDepth = SCApp->configGetDouble("olv.defaultDepth"); }
	catch ( ... ) { pickerConfig.defaultDepth = 10; }

	try { amplitudeConfig.preOffset = Core::TimeSpan(SCApp->configGetInt("amplitudePicker.preOffset")); }
	catch ( ... ) { amplitudeConfig.preOffset = Core::TimeSpan(300.0); }

	try { amplitudeConfig.postOffset = Core::TimeSpan(SCApp->configGetInt("amplitudePicker.postOffset")); }
	catch ( ... ) { amplitudeConfig.postOffset = Core::TimeSpan(300.0); }

	try {
		std::vector<std::string> filters = SCApp->configGetStrings("amplitudePicker.filters");

		for ( std::vector<std::string>::iterator it = filters.begin();
		      it != filters.end(); ++it ) {
			if ( it->empty() ) continue;
			QString filter = (*it).c_str();
			QStringList tokens = filter.split(";");
			if ( tokens.size() != 2 ) {
				SEISCOMP_ERROR("Wrong filter string, expecting ';' to seperate name and definition: %s", (const char*)filter.toAscii());
				continue;
			}

			amplitudeConfig.addFilter(tokens[0], tokens[1]);
		}
	}
	catch ( ... ) {}

	OriginDialog::SetDefaultDepth(pickerConfig.defaultDepth);

	amplitudeConfig.defaultAddStationsDistance = pickerConfig.defaultAddStationsDistance;
	amplitudeConfig.hideStationsWithoutData = pickerConfig.hideStationsWithoutData;
	amplitudeConfig.loadStrongMotionData = pickerConfig.loadStrongMotionData;

	_originLocator = new OriginLocatorView(mapTree.get(), pickerConfig);
	_originLocator->setDatabase(SCApp->query());
	_originLocator->setConfig(locatorConfig);

	_magnitudes = new MagnitudeView(mapTree.get(), SCApp->query());
	_magnitudes->setAmplitudeConfig(amplitudeConfig);
	_magnitudes->setComputeMagnitudesSilently(_computeMagnitudesSilently);
	_magnitudes->setMagnitudeTypeSelectionEnabled(_askForMagnitudeTypes);
	_magnitudes->setDrawGridLines(locatorConfig.drawGridLines);

	try {
		if ( !_magnitudes->setDefaultAggregationType(SCApp->configGetString("olv.defaultMagnitudeAggregation")) ) {
			SEISCOMP_ERROR("Unknown aggregation in olv.defaultMagnitudeAggregation: %s",
			               SCApp->configGetString("olv.defaultMagnitudeAggregation").c_str());
		}
	}
	catch ( ... ) {}

	_magnitudeCalculationEnabled = _expertMode;

	_originLocator->setMagnitudeCalculationEnabled(_magnitudeCalculationEnabled);
	_eventSmallSummary->setDefaultEventRadius(locatorConfig.defaultEventRadius);

	connect(_magnitudes, SIGNAL(localAmplitudesAvailable(Seiscomp::DataModel::Origin*, AmplitudeSet*, StringSet*)),
	        _originLocator, SLOT(setLocalAmplitudes(Seiscomp::DataModel::Origin*, AmplitudeSet*, StringSet*)));

	connect(_originLocator, SIGNAL(magnitudesAdded(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*)),
	        _magnitudes, SLOT(reload()));

	connect(_originLocator, SIGNAL(artificalOriginCreated(Seiscomp::DataModel::Origin*)),
	        this, SLOT(setArtificialOrigin(Seiscomp::DataModel::Origin*)));

	/*
	_picker = new PickerView();
	_picker->setDatabase(_query.get());
	_picker->setDataSource(rsService.c_str(), rsSource.c_str());
	_picker->setStatusBar(NULL);
	*/

	//_originLocator->setPickerView(_picker);

	QLayout* layoutLocation = new QVBoxLayout(_ui.tabLocation);
	layoutLocation->addWidget(_originLocator);

	QLayout* layoutMagnitudes = new QVBoxLayout(_ui.tabMagnitudes);
	layoutMagnitudes->addWidget(_magnitudes);

	if ( _expertMode ) {
		_tabEventEdit = new QWidget;
		_ui.tabWidget->insertTab(_ui.tabWidget->count()-1, _tabEventEdit, "Event");

		_eventEdit = new EventEdit(SCApp->query(), mapTree.get(), _tabEventEdit);
		QLayout* layoutEventEdit = new QVBoxLayout(_tabEventEdit);
		layoutEventEdit->addWidget(_eventEdit);
	}
	else {
		_tabEventEdit = NULL;
		_eventEdit = NULL;
	}

	/*
	QLayout* layoutPicker = new QVBoxLayout(_ui.tabWaveforms);
	layoutPicker->addWidget(_picker);
	*/

	SCScheme.applyTabPosition(_ui.tabWidget);
	//setCentralWidget(_originLocator);

	_eventList = new EventListView(Application::Instance()->query());
	_eventList->setRelativeMinimumEventTime(Application::Instance()->maxEventAge());
	_eventList->setEventModificationsEnabled(true);
	_eventList->setFMLinkEnabled(true);

	QLayout* layoutEventList = new QVBoxLayout(_ui.tabEventList);
	layoutEventList->addWidget(_eventList);

	_currentTabWidget = _ui.tabWidget->currentWidget();

#ifdef WITH_SMALL_SUMMARY
	addAction(_ui.actionShowSummary);
	_ui.actionShowSummary->setChecked(SCApp->settings().value("showSummary").toBool());
#else
	delete _ui.actionShowSummary;
#endif
	addAction(_ui.actionShowStations);
	addAction(_ui.actionShowEventList);

	connect(SCApp, SIGNAL(addObject(const QString&, Seiscomp::DataModel::Object*)),
	        this, SLOT(objectAdded(const QString&, Seiscomp::DataModel::Object*)));

	connect(SCApp, SIGNAL(removeObject(const QString&, Seiscomp::DataModel::Object*)),
	        this, SLOT(objectRemoved(const QString&, Seiscomp::DataModel::Object*)));

	connect(SCApp, SIGNAL(updateObject(const QString&, Seiscomp::DataModel::Object*)),
	        this, SLOT(objectUpdated(const QString&, Seiscomp::DataModel::Object*)));

#ifdef WITH_SMALL_SUMMARY
	connect(SCApp, SIGNAL(addObject(const QString&, Seiscomp::DataModel::Object*)),
	        _eventSmallSummary, SLOT(addObject(const QString&, Seiscomp::DataModel::Object*)));

	connect(SCApp, SIGNAL(removeObject(const QString&, Seiscomp::DataModel::Object*)),
	        _eventSmallSummary, SLOT(removeObject(const QString&, Seiscomp::DataModel::Object*)));

	connect(SCApp, SIGNAL(updateObject(const QString&, Seiscomp::DataModel::Object*)),
	        _eventSmallSummary, SLOT(updateObject(const QString&, Seiscomp::DataModel::Object*)));

	connect(SCApp, SIGNAL(addObject(const QString&, Seiscomp::DataModel::Object*)),
	        _eventSmallSummaryCurrent, SLOT(addObject(const QString&, Seiscomp::DataModel::Object*)));

	connect(SCApp, SIGNAL(removeObject(const QString&, Seiscomp::DataModel::Object*)),
	        _eventSmallSummaryCurrent, SLOT(removeObject(const QString&, Seiscomp::DataModel::Object*)));

	connect(SCApp, SIGNAL(updateObject(const QString&, Seiscomp::DataModel::Object*)),
	        _eventSmallSummaryCurrent, SLOT(updateObject(const QString&, Seiscomp::DataModel::Object*)));
#endif

	if ( _expertMode ) {
		connect(SCApp, SIGNAL(addObject(const QString&, Seiscomp::DataModel::Object*)),
		        _eventEdit, SLOT(addObject(const QString&, Seiscomp::DataModel::Object*)));

		connect(SCApp, SIGNAL(removeObject(const QString&, Seiscomp::DataModel::Object*)),
		        _eventEdit, SLOT(removeObject(const QString&, Seiscomp::DataModel::Object*)));

		connect(SCApp, SIGNAL(updateObject(const QString&, Seiscomp::DataModel::Object*)),
		        _eventEdit, SLOT(updateObject(const QString&, Seiscomp::DataModel::Object*)));

		connect(_originLocator, SIGNAL(updatedOrigin(Seiscomp::DataModel::Origin*)),
		        _eventEdit, SLOT(updateOrigin(Seiscomp::DataModel::Origin*)));
	}

	connect(SCApp, SIGNAL(messageAvailable(Seiscomp::Core::Message*, Seiscomp::Communication::NetworkMessage*)),
	        _eventList, SLOT(messageAvailable(Seiscomp::Core::Message*, Seiscomp::Communication::NetworkMessage*)));

	connect(SCApp, SIGNAL(notifierAvailable(Seiscomp::DataModel::Notifier*)),
	        _eventList, SLOT(notifierAvailable(Seiscomp::DataModel::Notifier*)));

	connect(SCApp, SIGNAL(addObject(const QString&, Seiscomp::DataModel::Object*)),
	        _originLocator, SLOT(addObject(const QString&, Seiscomp::DataModel::Object*)));
	connect(SCApp, SIGNAL(updateObject(const QString&, Seiscomp::DataModel::Object*)),
	        _originLocator, SLOT(updateObject(const QString&, Seiscomp::DataModel::Object*)));
	/*
	connect(SCApp, SIGNAL(removeObject(const QString&, Seiscomp::DataModel::Object*)),
	        _originLocator, SLOT(removeObject(const QString&, Seiscomp::DataModel::Object*)));
	*/

	connect(_eventEdit, SIGNAL(originMergeRequested(QList<Seiscomp::DataModel::Origin*>)),
	        _originLocator, SLOT(mergeOrigins(QList<Seiscomp::DataModel::Origin*>)));

	// connect add/update msg to MagnitudeView
	connect(SCApp, SIGNAL(addObject(const QString&, Seiscomp::DataModel::Object*)),
	        _magnitudes, SLOT(addObject(const QString&, Seiscomp::DataModel::Object*)));
	connect(SCApp, SIGNAL(updateObject(const QString&, Seiscomp::DataModel::Object*)),
	        _magnitudes, SLOT(updateObject(const QString&, Seiscomp::DataModel::Object*)));
	connect(SCApp, SIGNAL(removeObject(const QString&, Seiscomp::DataModel::Object*)),
	        _magnitudes, SLOT(removeObject(const QString&, Seiscomp::DataModel::Object*)));

	connect(_ui.tabWidget, SIGNAL(currentChanged(int)),
	        this, SLOT(tabChanged(int)));

	connect(_originLocator, SIGNAL(undoStateChanged(bool)),
	        _ui.actionUndo, SLOT(setEnabled(bool)));
	connect(_originLocator, SIGNAL(redoStateChanged(bool)),
	        _ui.actionRedo, SLOT(setEnabled(bool)));

	connect(_ui.actionUndo, SIGNAL(triggered(bool)),
	        _originLocator, SLOT(undo()));
	connect(_ui.actionRedo, SIGNAL(triggered(bool)),
	        _originLocator, SLOT(redo()));
	connect(_ui.actionCreateArtificialOrigin, SIGNAL(triggered(bool)),
	        _originLocator, SLOT(createArtificialOrigin()));
	connect(_ui.actionOpen, SIGNAL(triggered(bool)), this, SLOT(fileOpen()));
	connect(_ui.actionSave, SIGNAL(triggered(bool)), this, SLOT(fileSave()));
	connect(_ui.actionQuit, SIGNAL(triggered(bool)), SCApp, SLOT(quit()));

	connect(_eventList, SIGNAL(originSelected(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*)),
	        this, SLOT(setData(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*)));
	connect(_eventList, SIGNAL(eventFMSelected(Seiscomp::DataModel::Event*)),
	        this, SLOT(setFMData(Seiscomp::DataModel::Event*)));

	if ( _expertMode ) {
		connect(_eventEdit, SIGNAL(originSelected(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*)),
		        this, SLOT(setData(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*)));
	}

#ifdef WITH_SMALL_SUMMARY
	connect(_eventSmallSummary, SIGNAL(selected(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*)),
	        this, SLOT(setData(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*)));

	connect(_eventSmallSummary, SIGNAL(magnitudeSelected(const std::string &)),
	        this, SLOT(showMagnitude(const std::string &)));

	connect(_originLocator, SIGNAL(updatedOrigin(Seiscomp::DataModel::Origin*)),
	        _eventSmallSummary, SLOT(updateOrigin(Seiscomp::DataModel::Origin*)));

	connect(_eventSmallSummaryCurrent, SIGNAL(selected(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*)),
	        this, SLOT(setData(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*)));

	connect(_eventSmallSummaryCurrent, SIGNAL(magnitudeSelected(const std::string &)),
	        this, SLOT(showMagnitude(const std::string &)));

	connect(_originLocator, SIGNAL(updatedOrigin(Seiscomp::DataModel::Origin*)),
	        _eventSmallSummaryCurrent, SLOT(updateOrigin(Seiscomp::DataModel::Origin*)));
#endif

	connect(_originLocator, SIGNAL(newOriginSet(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*, bool, bool)),
	        this, SLOT(setOrigin(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*, bool, bool)));

	connect(_originLocator, SIGNAL(magnitudesAdded(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*)),
	        this, SLOT(updateOrigin(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*)));

#ifdef WITH_SMALL_SUMMARY
	connect(_magnitudes, SIGNAL(magnitudeUpdated(const QString&, Seiscomp::DataModel::Object*)),
	        _eventSmallSummary, SLOT(updateObject(const QString&, Seiscomp::DataModel::Object*)));

	connect(_magnitudes, SIGNAL(magnitudeUpdated(const QString&, Seiscomp::DataModel::Object*)),
	        _eventSmallSummaryCurrent, SLOT(updateObject(const QString&, Seiscomp::DataModel::Object*)));

	connect(_originLocator, SIGNAL(committedOrigin(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*,
	                                               const Seiscomp::Gui::ObjectChangeList<Seiscomp::DataModel::Pick>&,
	                                               const std::vector<Seiscomp::DataModel::AmplitudePtr>&)),
	        this, SLOT(releaseFixedOrigin(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*)));
#endif

	/*
	connect(_originLocator, SIGNAL(newOriginSet(Seiscomp::DataModel::Origin*, bool)),
	        _eventSmallSummary, SLOT(showOrigin(Seiscomp::DataModel::Origin*)));
	*/

	connect(_originLocator, SIGNAL(waveformsRequested()),
	        this, SLOT(showWaveforms()));
	connect(_originLocator, SIGNAL(eventListRequested()),
	        this, SLOT(showEventList()));
	connect(_originLocator, SIGNAL(locatorRequested()),
	        this, SLOT(showLocator()));

	connect(_originLocator, SIGNAL(computeMagnitudesRequested()),
	        _magnitudes, SLOT(computeMagnitudes()));

	connect(_eventList, SIGNAL(originAdded()), this, SLOT(originAdded()));
	connect(_eventList, SIGNAL(originReferenceAdded(const std::string &, Seiscomp::DataModel::OriginReference*)),
	        this, SLOT(originReferenceAdded(const std::string &, Seiscomp::DataModel::OriginReference*)));

	connect(_originLocator, SIGNAL(updatedOrigin(Seiscomp::DataModel::Origin*)),
	        _eventList, SLOT(updateOrigin(Seiscomp::DataModel::Origin*)));

	connect(_originLocator, SIGNAL(updatedOrigin(Seiscomp::DataModel::Origin*)),
	        _magnitudes, SLOT(disableRework()));

	connect(_originLocator, SIGNAL(committedOrigin(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*,
	                                               const Seiscomp::Gui::ObjectChangeList<Seiscomp::DataModel::Pick>&,
	                                               const std::vector<Seiscomp::DataModel::AmplitudePtr>&)),
	        _eventList, SLOT(insertOrigin(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*,
	                                      const Seiscomp::Gui::ObjectChangeList<Seiscomp::DataModel::Pick>&,
	                                      const std::vector<Seiscomp::DataModel::AmplitudePtr>&)));
	connect(_originLocator, SIGNAL(committedOrigin(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*,
	                                               const Seiscomp::Gui::ObjectChangeList<Seiscomp::DataModel::Pick>&,
	                                               const std::vector<Seiscomp::DataModel::AmplitudePtr>&)),
	        _magnitudes, SLOT(disableRework()));
	connect(_originLocator, SIGNAL(committedFocalMechanism(Seiscomp::DataModel::FocalMechanism *,
	                                                       Seiscomp::DataModel::Event *,
	                                                       Seiscomp::DataModel::Origin *)),
	        _eventList, SLOT(insertFocalMechanism(Seiscomp::DataModel::FocalMechanism *,
	                                              Seiscomp::DataModel::Event *,
	                                              Seiscomp::DataModel::Origin *)));

	connect(_originLocator, SIGNAL(requestRaise()), this, SLOT(raiseLocator()));


#ifdef WITH_SMALL_SUMMARY
	connect(_ui.actionShowSummary, SIGNAL(toggled(bool)), _ui.frameSummary, SLOT(setVisible(bool)));
#endif
	connect(_ui.actionAutoSelect, SIGNAL(toggled(bool)), _eventList, SLOT(setAutoSelect(bool)));
	connect(_ui.actionShowStations, SIGNAL(toggled(bool)), _originLocator, SLOT(drawStations(bool)));
	connect(_ui.actionShowStations, SIGNAL(toggled(bool)), _magnitudes, SLOT(drawStations(bool)));
	connect(_ui.actionShowEventList, SIGNAL(triggered(bool)), this, SLOT(showEventList()));

	_originLocator->drawStations(_ui.actionShowStations->isChecked());
	_magnitudes->drawStations(_ui.actionShowStations->isChecked());

#ifdef WITH_SMALL_SUMMARY
	_ui.frameSummary->setVisible(_ui.actionShowSummary->isChecked());
#endif

	try {
		_originLocator->setScript0(Seiscomp::Environment::Instance()->absolutePath(SCApp->configGetString("scripts.script0")));
	}
	catch ( ... ) {}

	try {
		_originLocator->setScript1(Seiscomp::Environment::Instance()->absolutePath(SCApp->configGetString("scripts.script1")));
	}
	catch ( ... ) {}

	SCApp->settings().endGroup();
}


MainFrame::~MainFrame() {
	SCApp->settings().beginGroup(objectName());
	SCApp->settings().setValue("showSummary", _ui.actionShowSummary->isChecked());
	SCApp->settings().endGroup();
}


void MainFrame::loadEvents(float days) {
	if ( days <= 0 ) return;

	SCApp->showMessage("Load event database");

	Core::TimeWindow tw;

	tw.setEndTime(Core::Time::GMT());
	tw.setStartTime(tw.endTime() - Core::TimeSpan(days*86400));

	_eventList->setInterval(tw);
	_eventList->readFromDatabase();
	_eventList->selectFirstEnabledEvent();
}


void MainFrame::setEventID(const std::string &eventID) {
	PublicObjectPtr po = SCApp->query()->loadObject(Event::TypeInfo(), eventID);
	EventPtr e = Event::Cast(po);
	if ( !e ) {
		QMessageBox::critical(
			this,
			"Load event",
			QString("Event %1 has not been found.").arg(eventID.c_str()));
		return;
	}

	_eventList->add(e.get(), NULL);
	_eventList->selectEventID(e->publicID());
}


void MainFrame::setOriginID(const std::string &originID) {
	PublicObjectPtr po = SCApp->query()->loadObject(Origin::TypeInfo(), originID);
	EventPtr e = Event::Cast(SCApp->query()->getEvent(originID));
	OriginPtr o = Origin::Cast(po);

	if ( !o ) {
		QMessageBox::critical(
			this,
			"Load origin",
			QString("Origin %1 has not been found.").arg(originID.c_str()));
		return;
	}

	_eventList->add(e.get(), o.get());
	setData(o.get(), e.get());
}


void MainFrame::setOffline(bool o) {
	_ui.actionOpen->setEnabled(o);
	_ui.actionSave->setEnabled(o);
	if ( _eventList ) _eventList->setMessagingEnabled(!o);
	if ( _eventEdit ) _eventEdit->setMessagingEnabled(!o);
}


void MainFrame::configureAcquisition() {
	PickerSettings dlg(_originLocator->config(), _originLocator->pickerConfig(), _magnitudes->amplitudeConfig());

	dlg.ui().cbComputeMagnitudesAfterRelocate->setEnabled(_magnitudeCalculationEnabled);
	dlg.ui().cbComputeMagnitudesAfterRelocate->setChecked(_computeMagnitudesAutomatically);
	dlg.ui().cbComputeMagnitudesSilently->setEnabled(_magnitudeCalculationEnabled);
	dlg.ui().cbComputeMagnitudesSilently->setChecked(_computeMagnitudesSilently);
	dlg.ui().cbAskForMagnitudeTypes->setChecked(_askForMagnitudeTypes);
	dlg.setSaveEnabled(true);

	if ( dlg.exec() != QDialog::Accepted )
		return;

	_computeMagnitudesAutomatically = dlg.ui().cbComputeMagnitudesAfterRelocate->isChecked();
	_computeMagnitudesSilently = dlg.ui().cbComputeMagnitudesSilently->isChecked();
	_askForMagnitudeTypes = dlg.ui().cbAskForMagnitudeTypes->isChecked();

#ifdef WITH_SMALL_SUMMARY
	_eventSmallSummary->setEvent(_eventSmallSummary->currentEvent());
	_eventSmallSummaryCurrent->setEvent(_eventSmallSummary->currentEvent(), _currentOrigin.get(), true);
#endif

	_magnitudes->setComputeMagnitudesSilently(_computeMagnitudesSilently);
	_magnitudes->setMagnitudeTypeSelectionEnabled(_askForMagnitudeTypes);

	OriginLocatorView::Config lc = dlg.locatorConfig();
	PickerView::Config pc = dlg.pickerConfig();
	AmplitudeView::Config ac = dlg.amplitudeConfig();

	OriginDialog::SetDefaultDepth(pc.defaultDepth);

	_originLocator->setConfig(lc);
	_originLocator->setPickerConfig(pc);
	_magnitudes->setDrawGridLines(lc.drawGridLines);
	_magnitudes->setAmplitudeConfig(ac);

	if ( dlg.saveSettings() ) {
		SCApp->configSetBool("olv.computeMagnitudesAfterRelocate", _computeMagnitudesAutomatically);
		SCApp->configSetBool("olv.computeMagnitudesSilently", _computeMagnitudesSilently);
		SCApp->configSetBool("olv.enableMagnitudeSelection", _askForMagnitudeTypes);
		SCApp->configSetDouble("olv.Pvel", lc.reductionVelocityP);
		SCApp->configSetBool("olv.drawMapLines", lc.drawMapLines);
		SCApp->configSetBool("olv.drawGridLines", lc.drawGridLines);
		SCApp->configSetBool("olv.computeMissingTakeOffAngles", lc.computeMissingTakeOffAngles);
		SCApp->configSetDouble("olv.defaultAddStationsDistance", pc.defaultAddStationsDistance);
		SCApp->configSetBool("olv.hideStationsWithoutData", pc.hideStationsWithoutData);

		SCApp->configSetBool("picker.showCrossHairCursor", pc.showCrossHair);
		SCApp->configSetBool("picker.ignoreUnconfiguredStations", pc.ignoreUnconfiguredStations);
		SCApp->configSetBool("picker.loadAllComponents", pc.loadAllComponents);
		SCApp->configSetBool("picker.loadAllPicks", pc.loadAllPicks);
		SCApp->configSetBool("picker.loadStrongMotion", pc.loadStrongMotionData);
		SCApp->configSetBool("picker.limitStationAcquisition", pc.limitStations);
		SCApp->configSetInt("picker.limitStationAcquisitionCount", pc.limitStationCount);
		SCApp->configSetBool("picker.showAllComponents", pc.showAllComponents);
		SCApp->configSetDouble("picker.allComponentsMaximumDistance", pc.allComponentsMaximumStationDistance);
		SCApp->configSetBool("picker.usePerStreamTimeWindows", pc.usePerStreamTimeWindows);
		SCApp->configSetInt("picker.preOffset", pc.preOffset.seconds());
		SCApp->configSetInt("picker.postOffset", pc.postOffset.seconds());
		SCApp->configSetInt("picker.minimumTimeWindow", pc.minimumTimeWindow.seconds());
		SCApp->configSetDouble("picker.alignmentPosition", pc.alignmentPosition);
		SCApp->configSetBool("picker.removeAutomaticPicksFromStationAfterManualReview", pc.removeAutomaticStationPicks);
		SCApp->configSetBool("picker.removeAllAutomaticPicksAfterManualReview", pc.removeAutomaticPicks);

		QStringList toks = pc.recordURL.split("://");
		if ( toks.count() == 2 ) {
			SCApp->configSetString("recordstream.service", toks[0].toStdString());
			SCApp->configSetString("recordstream.source", toks[1].toStdString());
		}

		std::vector<std::string> filters;
		foreach ( const PickerView::Config::FilterEntry &entry, pc.filters ) {
			filters.push_back(QString("%1;%2").arg(entry.first).arg(entry.second).toStdString());
		}

		SCApp->configSetStrings("picker.filters", filters);

		filters.clear();
		foreach ( const AmplitudeView::Config::FilterEntry &entry, ac.filters ) {
			filters.push_back(QString("%1;%2").arg(entry.first).arg(entry.second).toStdString());
		}

		if ( pc.repickerSignalStart )
			SCApp->configSetDouble("picker.repickerStart", *pc.repickerSignalStart);
		else
			SCApp->configUnset("picker.repickerStart");

		if ( pc.repickerSignalEnd )
			SCApp->configSetDouble("picker.repickerEnd", *pc.repickerSignalEnd);
		else
			SCApp->configUnset("picker.repickerEnd");

		SCApp->configSetInt("amplitudePicker.preOffset", ac.preOffset.seconds());
		SCApp->configSetInt("amplitudePicker.postOffset", ac.postOffset.seconds());
		SCApp->configSetStrings("amplitudePicker.filters", filters);

		SCApp->saveConfiguration();
	}
}


void MainFrame::raiseLocator() {
	// NOTE: Because the Picker does not relocate anymore
	//       the locatorview windows is raised and the relocate
	//       button starts blinking for some time
	activateWindow();
	raise();
}


void MainFrame::fileOpen() {
	QString filename = QFileDialog::getOpenFileName(this,
		tr("Open EventParameters"), "", tr("XML files (*.xml)"));

	if ( filename.isEmpty() ) return;

	IO::XMLArchive ar;
	if ( !ar.open(filename.toStdString().c_str()) ) {
		QMessageBox::critical(this, "Error", QString("Invalid file: %1").arg(filename));
		return;
	}

	DataModel::EventParametersPtr ep;

	_offlineData = NULL;

	//_currentOrigin = NULL;
	_originLocator->clear();
	_magnitudes->setOrigin(NULL, NULL);
	_eventEdit->setEvent(NULL, NULL);
	_eventSmallSummary->setEvent(NULL);
	_eventSmallSummaryCurrent->setEvent(NULL);
	_eventList->clear();

	_currentOrigin = NULL;

	ar >> ep;
	if ( ep == NULL ) {
		QMessageBox::critical(this, "Error", QString("Unable to find EventParameters structures in %1.").arg(filename));
		return;
	}

	_offlineData = ep;
	if ( !_offlineData->registered() ) {
		if ( !_offlineData->setPublicID(_offlineData->publicID()) ) {
			_offlineData = NULL;
			QMessageBox::critical(this, "Error", QString("Unable to register EventParameters globally."));
			return;
		}
	}

	std::set<std::string> associatedOriginIDs;
	for ( size_t i = 0; i < ep->eventCount(); ++i ) {
		DataModel::Event *ev = ep->event(i);
		_eventList->add(ev, NULL);

		for ( size_t j = 0; j < ev->originReferenceCount(); ++j ) {
			DataModel::OriginReference *ref = ev->originReference(j);
			associatedOriginIDs.insert(ref->originID());
		}
	}

	for ( size_t i = 0; i < ep->originCount(); ++i ) {
		DataModel::Origin *org = ep->origin(i);
		if ( associatedOriginIDs.find(org->publicID()) == associatedOriginIDs.end() )
			_eventList->add(NULL, org);
	}

	_eventList->selectEvent(0);
}


void MainFrame::fileSave() {
	if ( _offlineData == NULL ) {
		QMessageBox::information(this, "Error", "No data available.");
		return;
	}

	QString filename = QFileDialog::getSaveFileName(this,
		tr("Save EventParameters"), "", tr("XML files (*.xml)"));

	if ( filename.isEmpty() ) return;

	IO::XMLArchive ar;
	if ( !ar.create(filename.toStdString().c_str()) ) {
		QMessageBox::critical(this, "Error", QString("Unable to create file: %1").arg(filename));
		return;
	}

	ar.setFormattedOutput(true);
	ar << _offlineData;
	ar.close();
}


void MainFrame::setOrigin(Seiscomp::DataModel::Origin *o,
                          Seiscomp::DataModel::Event *e,
                          bool newOrigin, bool relocated) {
	_currentOrigin = o;
	_eventID = "";

	_magnitudes->setOrigin(o, e);

	if ( _magnitudeCalculationEnabled )
		_magnitudes->setReadOnly(!newOrigin);

#ifdef WITH_SMALL_SUMMARY
	_eventSmallSummary->setEvent(e);
	_eventSmallSummaryCurrent->setEvent(e, o, true);
#endif

	if ( newOrigin && relocated && _computeMagnitudesAutomatically && (o->magnitudeCount() == 0) )
		_originLocator->computeMagnitudes();

	/*
	if ( _expertMode ) {
		_eventEdit->setEvent(NULL, NULL);
	}
	*/
}


void MainFrame::updateOrigin(Seiscomp::DataModel::Origin *o, Seiscomp::DataModel::Event *e) {
#ifdef WITH_SMALL_SUMMARY
	_eventSmallSummaryCurrent->setEvent(e, o, true);
#endif

	// Switch to magnitudes tab
	//_ui.tabWidget->setCurrentWidget(_ui.tabMagnitudes);
}


void MainFrame::releaseFixedOrigin(Seiscomp::DataModel::Origin *, Seiscomp::DataModel::Event *e) {
}


void MainFrame::setArtificialOrigin(Seiscomp::DataModel::Origin *org) {
	populateOrigin(org, NULL, true);
}


void MainFrame::populateOrigin(Seiscomp::DataModel::Origin *org, Seiscomp::DataModel::Event *ev, bool local) {
	if ( _originLocator->setOrigin(org, ev, local) ) {
		_currentOrigin = org;
		if ( ev ) _eventID = ev->publicID();
		else _eventID = "";

		_magnitudes->setOrigin(org, ev);
		_magnitudes->setPreferredMagnitudeID(ev?ev->preferredMagnitudeID():"");

		_ui.tabWidget->setCurrentWidget(_ui.tabLocation);

#ifdef WITH_SMALL_SUMMARY
		_eventSmallSummary->setEvent(ev/*, org*/);
		_eventSmallSummaryCurrent->setEvent(ev, org, true);
#endif
		if ( _expertMode )
			_eventEdit->setEvent(ev, org);
	}
}

void MainFrame::setData(Seiscomp::DataModel::Origin *org, Seiscomp::DataModel::Event *ev) {
	populateOrigin(org, ev, false);
}


void MainFrame::setFMData(Seiscomp::DataModel::Event *ev) {
	populateOrigin(Origin::Find(ev->preferredOriginID()), ev, false);
	_ui.tabWidget->setCurrentWidget(_tabEventEdit);
	// Set FM tab as current
	_eventEdit->showTab(1);
}


void MainFrame::showMagnitude(const std::string &id) {
	if ( _magnitudes->showMagnitude(id) )
		_ui.tabWidget->setCurrentWidget(_ui.tabMagnitudes);
}


void MainFrame::showEventList() {
	_ui.tabWidget->setCurrentWidget(_ui.tabEventList);
}


void MainFrame::showLocator() {
	_ui.tabWidget->setCurrentWidget(_ui.tabLocation);
}


void MainFrame::toggledFullScreen(bool isFullScreen) {
	if ( menuBar() )
		menuBar()->setVisible(!isFullScreen);
}


void MainFrame::closeEvent(QCloseEvent *e) {
	_exportProcess.terminate();
	_originLocator->close();
	_magnitudes->close();

	MainWindow::closeEvent(e);
}


void MainFrame::originReferenceRemoved(const std::string &eventID, Seiscomp::DataModel::OriginReference *ref) {
	if ( _currentOrigin && _currentOrigin->publicID() == ref->originID() ) {
		if ( _originLocator->setOrigin(_currentOrigin.get(), NULL) ) {
			_magnitudes->setPreferredMagnitudeID("");

#ifdef WITH_SMALL_SUMMARY
			_eventSmallSummary->setEvent(NULL);
			_eventSmallSummaryCurrent->setEvent(NULL, _currentOrigin.get(), true);
#endif
			if ( _expertMode )
				_eventEdit->setEvent(NULL, _currentOrigin.get());

			_eventID.clear();
		}
	}
}


void MainFrame::originReferenceAdded(const std::string &eventID, Seiscomp::DataModel::OriginReference *ref) {
	if ( _currentOrigin && _currentOrigin->publicID() == ref->originID() ) {
		EventPtr evt = Event::Find(eventID);
		if ( !evt )
			evt = Event::Cast(SCApp->query()->loadObject(Event::TypeInfo(), eventID));

		if ( evt ) {
			if ( _originLocator->setOrigin(_currentOrigin.get(), evt.get()) ) {
				_magnitudes->setPreferredMagnitudeID(evt?evt->preferredMagnitudeID():"");

#ifdef WITH_SMALL_SUMMARY
				_eventSmallSummary->setEvent(evt.get()/*, org*/);
				_eventSmallSummaryCurrent->setEvent(evt.get(), _currentOrigin.get(), true);
#endif
				if ( _expertMode ) {
					_eventEdit->setEvent(evt.get(), _currentOrigin.get());
				}

				_eventID = evt->publicID();
			}
		}
	}
}

void MainFrame::objectAdded(const QString &parentID, Seiscomp::DataModel::Object* o) {
	Pick* pick = Pick::Cast(o);
	if ( pick ) {
		_originLocator->addPick(pick);
		return;
	}

	OriginReference *ref = OriginReference::Cast(o);
	if ( ref ) {
		originReferenceAdded(parentID.toStdString(), ref);
		return;
	}

	// NOTE Do not update the station state during picking
	/*
	DataModel::ConfigStation *cs = DataModel::ConfigStation::Cast(o);
	if ( cs ) {
		_originLocator->setStationEnabled(cs->networkCode(), cs->stationCode(), cs->enabled());
		return;
	}
	*/
}


void MainFrame::objectUpdated(const QString& parentID, Seiscomp::DataModel::Object* o) {
	// NOTE Do not update the station state during picking
	/*
	DataModel::ConfigStation *cs = DataModel::ConfigStation::Cast(o);
	if ( cs ) {
		_originLocator->setStationEnabled(cs->networkCode(), cs->stationCode(), cs->enabled());
		return;
	}
	*/

	Event *evt = Event::Cast(o);
	if ( evt && evt->publicID() == _eventID ) {
		if ( _currentOrigin && _currentOrigin->publicID() == evt->preferredOriginID() ) {
			_magnitudes->setPreferredMagnitudeID(evt->preferredMagnitudeID());
		}
		return;
	}
}


void MainFrame::objectRemoved(const QString& parentID, DataModel::Object* o) {
	OriginReference *ref = OriginReference::Cast(o);
	if ( ref ) {
		originReferenceRemoved(parentID.toStdString(), ref);
		return;
	}

	// NOTE Do not update the station state during picking
	/*
	DataModel::ConfigStation *cs = DataModel::ConfigStation::Cast(o);
	if ( cs ) {
		_originLocator->setStationEnabled(cs->networkCode(), cs->stationCode(), true);
		return;
	}
	*/
}


void MainFrame::originAdded() {
	if ( statusBar() )
		statusBar()->showMessage(QString("A new origin arrived at %1 (localtime)")
		                         .arg(Seiscomp::Core::Time::LocalTime().toString("%F %T").c_str()));
}


void MainFrame::showWaveforms() {
	//_ui.tabWidget->setCurrentWidget(_ui.tabWaveforms);
}


EventParametersPtr MainFrame::_createEventParametersForPublication(const Event *event) {
	EventParametersPtr ep = new EventParameters;
SEISCOMP_DEBUG("EventParametersPtr _createEventParametersForPublication(%s)",event->publicID().c_str());
	// Event
	EventPtr clonedEvent = Event::Cast( event->clone());
	clonedEvent->add(new OriginReference(clonedEvent->preferredOriginID()));
	// Copy event descriptions
	for ( size_t i = 0; i < event->eventDescriptionCount(); ++i )
		clonedEvent->add(EventDescription::Cast(event->eventDescription(i)->clone()));
	ep->add(clonedEvent.get());

	// preferred origin
	string originID = event->preferredOriginID();
	Origin *origin = Origin::Find(originID);
	if (origin == NULL) {
		// not having a preferred origin is a fatal error
		SEISCOMP_ERROR("_createEventParametersForPublication(%s): preferredOrigin not found",event->publicID().c_str());
		return NULL;
	}

	// Even though we normally have the arrivals loaded, this might not be
	// the case under certain circumstances. For instance, an event was
	// loaded and then the preferred origin was set to another origin that
	// was not yet loaded completely. In that case we would miss the
	// arrivals.
	if (origin->arrivalCount() == 0) {
		SEISCOMP_DEBUG("loading arrivals...");
		SCApp->query()->loadArrivals(origin);
		SEISCOMP_DEBUG("...done");
	}

	OriginPtr preferredOrigin = Origin::Cast(copy(origin));
	ep->add(preferredOrigin.get());

	// focal mechanism
	string focalMechanismID = event->preferredFocalMechanismID();
	FocalMechanism *focalMechanism = FocalMechanism::Find(focalMechanismID);
	// not necessarily an error if NULL
	if (focalMechanism) {
		SEISCOMP_DEBUG("Focal mechanism <%s> found", focalMechanism->publicID().c_str());
		FocalMechanismPtr preferredFocalMechanism = FocalMechanism::Cast(copy(focalMechanism));
		ep->add(preferredFocalMechanism.get());

		OriginPtr triggeringOrigin = NULL;
		if (preferredFocalMechanism->triggeringOriginID().size()) {
			if (event->preferredOriginID() == preferredFocalMechanism->triggeringOriginID())
				triggeringOrigin = preferredOrigin;
			else {
				string originID = preferredFocalMechanism->triggeringOriginID();
				Origin *origin = Origin::Find(originID);
				if (origin) {
					triggeringOrigin = Origin::Cast(copy(origin));
					if (triggeringOrigin) {
						// for a triggering origin that is not
						// the preferred origin, we don't need
						// to keep arrivals or station magnitudes
						while (triggeringOrigin->arrivalCount() > 0)
							triggeringOrigin->removeArrival(0);
						while (triggeringOrigin->stationMagnitudeCount() > 0)
							triggeringOrigin->removeStationMagnitude(0);
					}
				}
			}
			// note that triggeringOrigin may still be NULL
			if (triggeringOrigin) {
				clonedEvent->add(new OriginReference(triggeringOrigin->publicID()));
				ep->add(triggeringOrigin.get());
			}
		}

		if (preferredFocalMechanism->momentTensorCount() > 0) {
			MomentTensorPtr momentTensor =
				preferredFocalMechanism->momentTensor(0); // FIXME What if there is more than one MT?
			if (momentTensor->derivedOriginID().size() > 0) {
				string originID = momentTensor->derivedOriginID();
				Origin *origin = Origin::Find(originID);
				if (origin) {
					OriginPtr derivedOrigin = Origin::Cast(copy(origin));
					if (derivedOrigin) {
						while (triggeringOrigin->arrivalCount() > 0)
							triggeringOrigin->removeArrival(0);
						while (triggeringOrigin->stationMagnitudeCount() > 0)
							triggeringOrigin->removeStationMagnitude(0);
						clonedEvent->add(new OriginReference(derivedOrigin->publicID()));
						ep->add(derivedOrigin.get());
					}
				}
				
			}

		}
	}

	return ep;
}


void MainFrame::publishEvent() {
#if defined(WITH_SMALL_SUMMARY)

	const Event *event = NULL;
	if ( _eventSmallSummary ) event = _eventSmallSummary->currentEvent();

	if ( !event ) {
		QMessageBox::critical(this, "Error", "No event set!");
		return;
	}

	bool wasEnabled = PublicObject::IsRegistrationEnabled();
	PublicObject::SetRegistrationEnabled(false);

	EventParametersPtr ep = _createEventParametersForPublication(event);

/*
	string origID = event->preferredOriginID();
	Origin *orig = Origin::Find( origID );
	EventParameters *oldParent = orig->eventParameters();
	orig->detach();

	epShort->add( orig );

	Origin *magOrigin = NULL;
	EventParameters *oldMagParent = NULL;

	Magnitude *mag = Magnitude::Find(event->preferredMagnitudeID());
	if ( mag != NULL ) {
		// Magnitude is cross-referenced, lets add its origin as well
		if ( mag->origin() != orig && mag->origin() != NULL ) {
			magOrigin = mag->origin();
			oldMagParent = magOrigin->eventParameters();
			magOrigin->detach();
			epShort->add(magOrigin);
// XXX			clonedEvent->add(new OriginReference(magOrigin->publicID()));
		}
	}

	string tmpFileName;
	stringstream tmpss;
	tmpss << "/tmp/seiscomp_" <<   event->publicID() <<  "." << Core::pid() << "." << Core::Time::GMT().iso() << ".xml";
	tmpFileName = tmpss.str();
	IO::XMLArchive ar;
	if( !ar.create(tmpFileName.c_str()) ) {
		SEISCOMP_ERROR(" Can't open tmpFile (%s)!", tmpFileName.c_str());
		PublicObject::SetRegistrationEnabled(wasEnabled);

		if ( oldParent != NULL ) {
			orig->detach();
			oldParent->add(orig);
		}

		if ( oldMagParent != NULL ) {
			magOrigin->detach();
			oldMagParent->add(magOrigin);
		}

		return;
	}
*/
	string tmpFileName;
	stringstream tmpss;
	tmpss << "/tmp/seiscomp_" <<   event->publicID() <<  "." << Core::pid() << "." << Core::Time::GMT().iso() << ".xml";
	tmpFileName = tmpss.str();
	IO::XMLArchive ar;
	if( !ar.create(tmpFileName.c_str()) ) {
		SEISCOMP_ERROR(" Can't open tmpFile (%s)!", tmpFileName.c_str());
		PublicObject::SetRegistrationEnabled(wasEnabled);
		return;
	}
	ar.setFormattedOutput(true);
	EventParameters *p_ep = ep.get();
	ar << p_ep;
	ar.close();
	SEISCOMP_DEBUG("--> created tempFile:%s", tmpFileName.c_str());
/*
	if ( oldParent != NULL ) {
		orig->detach();
		oldParent->add(orig);
	}

	if ( oldMagParent != NULL ) {
		magOrigin->detach();
		oldMagParent->add(magOrigin);
	}
*/

	string user = Core::getLogin();
	string host = Core::getHostname();
	unsigned int pid = Core::pid();
	std::stringstream tmp;
	tmp << pid;
	std::string spid = tmp.str();
	string clientID = host + "_" + spid + "_" + user;


	//mit QProcess
	if( _exportProcess.state() != QProcess::NotRunning ) {
		if ( !_exportScriptTerminate ) {
			if ( QMessageBox::question(this, "Export event",
			                           QString("%1 is still running.\n"
			                                   "Do you want to terminate it?")
			                             .arg(_exportScript.c_str()),
			                           QMessageBox::Yes, QMessageBox::No) == QMessageBox::No ) {
				PublicObject::SetRegistrationEnabled(wasEnabled);
				return;
			}
		}

		SEISCOMP_WARNING(" ... will terminate old/other %s ...", _exportScript.c_str());
		_exportProcess.terminate();
		_exportProcess.waitForFinished();
	}

	if( _exportProcess.state() == QProcess::NotRunning ) {
		QString		prog	= _exportScript.c_str();
		//QString		spid;
		//spid.setNum( pid );
		QStringList	args;
		args << "-c";
		args << clientID.c_str();
		args << "-e";
		args << tmpFileName.c_str();
		//args << spid;
		args << "-H";
		args << SCApp->messagingHost().c_str();
		//'export' isn't the right name, hm?
		SEISCOMP_INFO("starting export-process %s ...", prog.toStdString().c_str());
		_exportProcess.start( prog, args);
		cerr << "starting: " << prog.toStdString() << " " << args.join(" ").toStdString() << " " << endl;
		if( _exportProcess.waitForStarted() ) { //can freeze gui
			SEISCOMP_INFO("... started successful ...");
			if( statusBar() ){
				statusBar()->showMessage( QString("exportprocess started."));
			}
			sleep( 1 );
			if( _exportProcess.state() != QProcess::Running ) {
				QMessageBox::warning(this, "Export Event", "process doesn't run!");
				qDebug() <<  _exportProcess.readAll();
			}
		} else  {
			QString msgToUsr("%1 couldn't be started\n");
			switch( _exportProcess.error() ){
				case QProcess::FailedToStart:
					msgToUsr += "Do you have it installed in your PATH?";
					break;
				case QProcess::Crashed:
					msgToUsr += "Process crashed right after startup!";
					break;
				case QProcess::Timedout:
					msgToUsr += "Start timed out.";
					break;
				case QProcess::WriteError:
					msgToUsr += "Can't write to the process.";
					break;
				case QProcess::ReadError:
					msgToUsr += "Can't read from the process.";
					break;
				case QProcess::UnknownError:
				default:
					msgToUsr = "An unknown problem was detected.";
			}
			QMessageBox::warning(this, "Export(publish) event",
			                     msgToUsr.arg(_exportScript.c_str()));
		}
	}

	PublicObject::SetRegistrationEnabled(wasEnabled);
#endif
}


void MainFrame::tabChanged(int tab) {
	QWidget *lastTabWidget = _currentTabWidget;
	_currentTabWidget = _ui.tabWidget->widget(tab);

	MapWidget *source = NULL, *target = NULL;
	if ( lastTabWidget == _ui.tabLocation )
		source = _originLocator->map();
	else if ( lastTabWidget == _ui.tabMagnitudes )
		source = _magnitudes->map();

	if ( _currentTabWidget == _ui.tabLocation )
		target = _originLocator->map();
	else if ( _currentTabWidget == _ui.tabMagnitudes )
		target = _magnitudes->map();

	if ( source && target )
		target->canvas().setView(source->canvas().mapCenter(), source->canvas().zoomLevel());
}


}
}
