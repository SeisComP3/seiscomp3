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

#include "mvmainwindow.h"

#include <QGridLayout>

#define SEISCOMP_COMPONENT MapView
#include <seiscomp3/logging/log.h>

#include <seiscomp3/client/inventory.h>

#include <seiscomp3/core/datamessage.h>

#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/messages.h>
#include <seiscomp3/gui/datamodel/stationsymbol.h>
#include <seiscomp3/gui/datamodel/origindialog.h>
#include <seiscomp3/gui/datamodel/ttdecorator.h>
#include <seiscomp3/gui/map/projection.h>

#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/parameterset.h>
#include <seiscomp3/datamodel/parameter.h>
#include <seiscomp3/datamodel/inventory.h>
#include <seiscomp3/datamodel/utils.h>

#include <seiscomp3/utils/keyvalues.h>

#include <seiscomp3/math/math.h>
#include <seiscomp3/math/filter.h>

#include "types.h"
#include "infowidget.h"

#include "debug.h"

using namespace Seiscomp;

namespace {


DataModel::Amplitude* debugGetAmplitudeForPickFromDataBase(const std::string& publicID) {
	return SCApp->query()->getAmplitude(publicID, "snr");
}




inline std::string getStationId(const DataModel::WaveformStreamID* waveformID) {
	return waveformID->networkCode() + "."
	       + waveformID->stationCode() + "."
	       + waveformID->locationCode() + "."
	       + waveformID->channelCode();
}




inline std::string getStationId(const DataModel::Pick* pick) {
	return getStationId(&pick->waveformID());
}




inline std::string getStationId(const DataModel::Amplitude* amplitude) {
	return getStationId(&amplitude->waveformID());
}




inline std::string getStationId(const Record* record) {
	return record->networkCode() + "."
	       + record->stationCode() + "."
	       + record->locationCode() + "."
	       + record->channelCode();
}




inline std::string getStationId(DataModel::WaveformQuality* waveformQuality) {
	return getStationId(&waveformQuality->index().waveformID);
}




inline bool areOriginSymbolsVisible() {
	if ( ApplicationStatus::Instance()->mode() == ApplicationStatus::QC )
		return false;
	return true;
}




bool isFakeEvent(const DataModel::Event* event) {
    bool isFakeEvent = false;
    try {
        isFakeEvent = event->type() == DataModel::NOT_EXISTING ||
                      event->type() == DataModel::OTHER_EVENT;
        } catch ( Core::ValueException& ) {}

	return isFakeEvent;
}




bool hasEventCountChanged(const EventDataRepository& repository) {
	static int preceededEventCount = 0;
	if ( repository.eventCount() == preceededEventCount )
		return false;

	preceededEventCount = repository.eventCount();
	return true;
}




void addEventWidgetRowData(EventTableWidget::RowData& rowData,
                           const EventData& eventData,
                           const DataModel::Origin* origin) {
	QString eventId = QString(eventData.id().c_str());
	QString originTime(origin->time().value().toString("%d.%m.%y %T").c_str());

	QString eventRegion(DataModel::eventRegion(eventData.object()).c_str());

	double latitude = origin->latitude();
	QString latitudeValue = QString("%1").arg((latitude >= 0) ? latitude : latitude * (-1.0));
	QString latitudeOrientation = QString("%1").arg((latitude >= 0) ? "N" : "");

	double longitude = origin->longitude();
	QString longitudeValue = QString("%1").arg((longitude <= 0) ? longitude * (-1.0) : longitude);
	QString longitudeOrientation = QString("%1").arg((longitude <= 0) ? "W" : "E");

	QString depth = QString("%1 km").arg(static_cast<int> (Math::round(origin->depth())));

	rowData[EventTableWidget::EVENT_ID]     = eventId;
	rowData[EventTableWidget::ORIGIN_TIME]  = originTime;
	rowData[EventTableWidget::EVENT_REGION] = eventRegion;
	rowData[EventTableWidget::LATITUDE]     = latitudeValue + latitudeOrientation;
	rowData[EventTableWidget::LONGITUDE]    = longitudeValue + longitudeOrientation;
	rowData[EventTableWidget::DEPTH]        = depth;
}




void addEventWidgetRowData(EventTableWidget::RowData& rowData,
                           const DataModel::Magnitude* magnitude) {
	QString magnitudeValue = QString("%1").arg(magnitude->magnitude(), 0, 'f', 2);
	QString magnitudeType(magnitude->type().c_str());

	rowData[EventTableWidget::MAGNITUDE]      = magnitudeValue;
	rowData[EventTableWidget::MAGNITUDE_TYPE] = magnitudeType;
}




void showInfoWidget(InfoWidget* infoWidget) {
	infoWidget->init();
	infoWidget->move(QCursor::pos());
	infoWidget->show();
}




void configureInfoWidgetForRecordAcquisition(StationInfoWidget* infoWidget,
                                             const Core::TimeSpan& timeSpan,
                                             const std::string& filterStr) {
	infoWidget->setRecordSequeceTimeSpan(timeSpan);
	infoWidget->setRecordStreamUrl(SCApp->recordStreamURL());
	infoWidget->setRecordFilterString(filterStr);
}




void setInfoWidgetContent(StationInfoWidget* infoWidget, const DataModel::Amplitude* amplitude) {
	try	{
		QString time = amplitude->timeWindow().reference().toString("%F-%T").c_str();
		infoWidget->setAmplitudeTime(time);
	}
	catch ( Core::ValueException& ) {}

	QString amplitudeValue = QString("%1").arg(amplitude->amplitude().value());
	infoWidget->setAmplitudeMaxValue(amplitudeValue);

	try {
		Core::Time amplTime(amplitude->timeWindow().reference()
							+ Core::TimeSpan(amplitude->timeWindow().end()));
		QString amplitudeValueTime = amplTime.toString("%F-%T").c_str();
		infoWidget->setAmplitudeMaxValueTime(amplitudeValueTime);
	}
	catch ( Core::ValueException& )	{}
}




void setInfoWidgetContent(StationInfoWidget* infoWidget, DataModel::Station* station) {
	infoWidget->setLongitude(QString("%1").arg(station->longitude()));
	infoWidget->setLatitude(QString("%1").arg(station->latitude()));
	infoWidget->setElevation(QString("%1").arg(station->elevation()));
	/* TODO: Find a replacement
	infoWidget->setDepth(QString("%1").arg(station->depth()));
	*/
	infoWidget->setPlace(station->place().c_str());
	infoWidget->setCountry(station->country().c_str());
	infoWidget->setDescription(station->description().c_str());
}




void setInfoWidgetContent(StationInfoWidget* infoWidget, StationData* stationData) {
	StationData::QcDataMap::iterator it = stationData->qcDataMap.begin();
	for ( ; it != stationData->qcDataMap.end(); it++ ) {
		StationData::QCData& data = it->second;

		QString name = it->first.toString();
        if ( data.status == QCStatus::NOT_SET ) {
		    infoWidget->setQCContent(name, "", "", "", Qt::white);
            continue;
        }

        QString value = QString("%1").arg(data.value, 0, 'f', 2);
		QString lowerUncertainty = "NaN";
		QString upperUncertainty = "NaN";

		if ( std::numeric_limits<double>::quiet_NaN() != data.lowerUncertainty )
			lowerUncertainty = QString("%1").arg(data.lowerUncertainty, 0, 'f', 2);

		if ( std::numeric_limits<double>::quiet_NaN() != data.upperUncertainty )
			upperUncertainty = QString("%1").arg(data.upperUncertainty, 0, 'f', 2);

		QColor color = Qt::white;
		if ( data.status == QCStatus::OK )
			color = SCScheme.colors.qc.qcOk;
		else if ( data.status == QCStatus::WARNING )
			color = SCScheme.colors.qc.qcWarning;
		else if ( data.status == QCStatus::ERROR )
			color = SCScheme.colors.qc.qcError;

		infoWidget->setQCContent(name, value, lowerUncertainty, upperUncertainty, color);
	}
}




void setInfoWidgetContent(OriginInfoWidget* infoWidget, const DataModel::Origin* origin,
                          const std::string& preferredMagnitudeId) {
	infoWidget->setPreferredOriginId(origin->publicID().c_str());

	QString time = origin->time().value().toString("%F - %T").c_str();
	infoWidget->setTime(time);

	QString latitude = QString("%1").arg(origin->latitude());
	infoWidget->setLatitude(latitude);

	QString longitude = QString("%1").arg(origin->longitude());
	infoWidget->setLongitude(longitude);

	QString depth = QString("%1").arg(origin->depth());
	infoWidget->setDepth(depth);

	QString magnitude;
	for ( size_t i = 0; i < origin->magnitudeCount(); i++ ) {
		DataModel::Magnitude* magnitudeRef = origin->magnitude(i);
		if ( magnitudeRef->publicID() == preferredMagnitudeId ) {
			magnitude = QString("%1").arg(magnitudeRef->magnitude(), 0, 'f', 2);
			break;
		}
	}
	infoWidget->setMagnitude(magnitude);

	QString agency = QString("%1").fromStdString(objectAgencyID(origin).c_str());
	infoWidget->setAgency(agency);

	QString mode;
	try { mode = QString("%1").fromStdString(origin->evaluationMode().toString()); }
	catch ( Core::ValueException& ) {}
	infoWidget->setMode(mode);

	QString status;
	try { status = QString("%1").fromStdString(origin->evaluationStatus().toString()); }
	catch ( Core::ValueException& ) {}
	infoWidget->setStatus(status);

	QString stationCount = "0";
	try { stationCount = QString("%1").arg(origin->quality().usedStationCount()); }
	catch (Core::ValueException& e) {}
	infoWidget->setStationCount(stationCount);

	QString azimuthGap;
	try { azimuthGap = QString("%1").arg(origin->quality().azimuthalGap()); }
	catch (Core::ValueException& e) {}
	infoWidget->setAzimuthGap(azimuthGap);

	QString minimumDistance;
	try { minimumDistance = QString("%1").arg(origin->quality().minimumDistance()); }
	catch ( Core::ValueException& ) {}
	infoWidget->setMinimumDistance(minimumDistance);

	QString maximumDistance;
	try { maximumDistance = QString("%1").arg(origin->quality().maximumDistance()); }
	catch ( Core::ValueException ) {}
	infoWidget->setMaximumDistance(maximumDistance);
}




class StationRenderParameter {
	public:
		virtual ~StationRenderParameter() {}

	public:
		void setStationData(StationData* stationData) {
			_stationData = stationData;
		}

		virtual QColor color() const = 0;

		virtual QColor frameColor() const = 0;

		virtual int frameSize() const = 0;

		virtual Gui::Map::Symbol::Priority priority() const = 0;

		virtual char annotation() const {
			return StationGlyphs::DEFAULT;
		}

		virtual bool hasAnnotation() const {
			return false;
		}

	protected:
		StationData* stationData() const {
			return _stationData;
		}

	private:
		StationData* _stationData;
};




class DisabledStationRenderParameter : public StationRenderParameter {
	public:
		virtual QColor color() const {
			return SCScheme.colors.stations.disabled;
		}

		virtual QColor frameColor() const {
			return SCScheme.colors.stations.disabled;
		}

		virtual int frameSize() const {
			return TriggerHandler::STATION_DEFAULT_FRAME_SIZE;
		}

		virtual Gui::Map::Symbol::Priority priority() const {
			return Gui::Map::Symbol::NONE;
		}

		virtual char annotation() const {
			return StationGlyphs::DISABLED;
		}

		virtual bool hasAnnotation() const {
			return true;
		}
};




class GMStationRenderParameter : public StationRenderParameter {
	public:
		virtual QColor color() const {
			return stationData()->gmColor;
		}

		virtual QColor frameColor() const {
			if ( stationData()->isAssociated )
				return SCScheme.colors.stations.associated;

			if ( stationData()->isSelected )
				return SCScheme.colors.stations.selected;

			if ( stationData()->isTriggering )
				return SCScheme.colors.stations.triggering;

			return Qt::black;
		}

		virtual int frameSize() const {
			if ( !ApplicationStatus::Instance()->isTriggering() )
				return TriggerHandler::STATION_DEFAULT_FRAME_SIZE;

			if ( stationData()->isTriggering )
				return stationData()->frameSize;

			if ( stationData()->isAssociated )
				return TriggerHandler::STATION_DEFAULT_PICK_TRIGGER_FRAME_SIZE;

			if ( stationData()->isSelected )
				return TriggerHandler::STATION_DEFAULT_PICK_TRIGGER_FRAME_SIZE;

			return TriggerHandler::STATION_DEFAULT_FRAME_SIZE;
		}

		virtual Gui::Map::Symbol::Priority priority() const {
			bool emphasize = stationData()->isTriggering ||
			                 stationData()->isAssociated ||
			                 stationData()->isSelected;

			if ( emphasize )
				return Gui::Map::Symbol::HIGH;

			if ( stationData()->gmColor != SCScheme.colors.gm.gmNotSet )
				return Gui::Map::Symbol::MEDIUM;

			return Gui::Map::Symbol::NONE;
		}
};




class QCStationRenderParameter : public StationRenderParameter {
	public:
		virtual QColor color() const {
			if ( QCParameter::Instance()->parameter() == QCParameter::DELAY )
				return stationData()->qcColor;

			QCParameter::Parameter parameter = QCParameter::Instance()->parameter();
			StationData::QCData data = stationData()->qcDataMap[parameter];
			QCStatus::Status status = data.status;

			if ( status == QCStatus::OK )
				return SCScheme.colors.qc.qcOk;

			if ( status == QCStatus::WARNING )
				return SCScheme.colors.qc.qcWarning;

			if ( status == QCStatus::ERROR )
				return SCScheme.colors.qc.qcError;

			return SCScheme.colors.qc.qcNotSet;
		}

		virtual QColor frameColor() const {
			return Qt::black;
		}

		virtual int frameSize() const {
			return TriggerHandler::STATION_DEFAULT_FRAME_SIZE;
		}

		virtual Gui::Map::Symbol::Priority priority() const {
			bool emphasize = stationData()->qcGlobalStatus == QCStatus::WARNING ||
			                 stationData()->qcGlobalStatus == QCStatus::ERROR ||
			                 stationData()->isSelected;
			if ( emphasize )
				return Gui::Map::Symbol::HIGH;

			if ( stationData()->qcGlobalStatus == QCStatus::OK )
				return Gui::Map::Symbol::LOW;

			return Gui::Map::Symbol::NONE;
		}

		virtual char annotation() const {
			if ( stationData()->qcGlobalStatus == QCStatus::WARNING )
				return StationGlyphs::WARNING;

			if ( stationData()->qcGlobalStatus == QCStatus::ERROR )
				return StationGlyphs::ERROR;

			return StationGlyphs::DEFAULT;
		}

		virtual bool hasAnnotation() const {
			if ( stationData()->qcGlobalStatus > QCStatus::OK )
				return true;
			return false;
		}

};



StationRenderParameter* selectRenderParameterInstance(bool isStationEnabled) {
	if ( !isStationEnabled ) {
		static DisabledStationRenderParameter disabledRenderParameter;
		return &disabledRenderParameter;
	}

	StationRenderParameter* parameter = NULL;
	if ( ApplicationStatus::Instance()->mode() == ApplicationStatus::GROUND_MOTION ) {
		static GMStationRenderParameter gmParameter;
		parameter = &gmParameter;
	}
	else if ( ApplicationStatus::Instance()->mode() == ApplicationStatus::EVENT ) {
		static GMStationRenderParameter eventParameter;
		return &eventParameter;
	}
	else if ( ApplicationStatus::Instance()->mode() == ApplicationStatus::QC ) {
		static QCStationRenderParameter qcParameter;
		parameter = &qcParameter;
	}

	return parameter;
}




DisplayMode selectDisplayModeFromString(const std::string& mode) {
	if ( mode == "groundmotion")
		return GROUND_MOTION;

	if ( mode == "qualitycontrol" )
		return QUALITY_CONTROL;

	return NONE;
}




QWidget* selectTabFromDisplayMode(DisplayMode mode, const Ui::MvMainWindow& ui) {
	if ( mode == GROUND_MOTION )
		return ui.gmTab;

	if ( mode == QUALITY_CONTROL )
		return ui.qcTab;

	return NULL;
}


} // namespace




MvMainWindow::MvMainWindow(QWidget* parent, Qt::WFlags flags)
 : Gui::MainWindow(parent, flags),
   _mapWidget(NULL),
   _displayMode(NONE),
   _mapUpdateInterval(1E3),
   _configStationPickCacheLifeSpan(15 * 60),
   _configEventActivityLifeSpan(15 * 60),
   _configRemoveEventDataOlderThanTimeSpan(static_cast<double>(12 * 3600)),
   _configReadEventsNotOlderThanTimeSpan(0.0),
   _configStationRecordFilterStr("RMHP(50)->ITAPER(20)->BW(2,0.04,2)") {

	setupStandardUi();
}




bool MvMainWindow::init() {
	if ( !SCApp->query() ) {
		SEISCOMP_ERROR("No database is configured, abort");
		return false;
	}

	_qcHandler.init(SCApp->configuration());

	// Overwrite default menu settings
	try {
		_ui.showMapLegendAction->setChecked(SCApp->configGetBool("legend"));
	}
	catch ( ... ) {}

	try {
		_ui.showStationIdAction->setChecked(SCApp->configGetBool("annotations"));
	}
	catch ( ... ) {}

	try {
		_ui.actionShowStationChannelCodes->setChecked(SCApp->configGetBool("annotationsWithChannels"));
	}
	catch ( ... ) {}

	std::string displayMode;
	try {
		displayMode = SCApp->configGetString("displaymode");
	} catch ( Config::Exception& ) {}

	if ( SCApp->commandline().hasOption("displaymode") ) {
		displayMode = SCApp->commandline().option<std::string>("displaymode");
	}

	_displayMode = selectDisplayModeFromString(displayMode);
	bool isDisplayModeStringInvalid = _displayMode == NONE && !displayMode.empty();
	if ( isDisplayModeStringInvalid ) {
		SEISCOMP_ERROR("Invalid displaymode found: %s", displayMode.c_str());
		return false;
	}

	if ( isInDisplayMode() )
		modifyUiSetupForDisplayMode();

	try {
		int recordLifeSpan = SCApp->configGetInt("groundMotionRecordLifeSpan");
		_recordHandler.setRecordLifeSpan(recordLifeSpan);
	} catch ( Config::Exception& ) {}

	_triggerHandler.setPickLifeSpan(_configStationPickCacheLifeSpan);

	if ( !readStationsFromDataBase() ) {
		SEISCOMP_ERROR("Could not read stations from database");
		return false;
	}

	try {
		_configStationRecordFilterStr = SCApp->configGetString("stations.groundMotionFilter");
	} catch ( Config::Exception& ) {}

	StationDataCollection::iterator stationIt = _stationDataCollection.begin();
	for ( ; stationIt != _stationDataCollection.end(); ++stationIt )
		stationIt->gmFilter = StationData::GmFilterPtr(Math::Filtering::InPlaceFilter<double>::Create(_configStationRecordFilterStr));

	if ( !initRecordStream() ) {
		SEISCOMP_ERROR("Could not initialize record stream");
	}

	try {
		_configRemoveEventDataOlderThanTimeSpan = SCApp->configGetDouble("removeEventDataOlderThan");
	} catch ( Config::Exception& ) {}
	_eventDataRepository.setEventDataLifeSpan(_configRemoveEventDataOlderThanTimeSpan);
	_eventDataRepository.setDatabaseArchive(SCApp->query());

	try {
		bool centerOrigins = SCApp->configGetBool("centerOrigins");
		if ( centerOrigins )
			_ui.centerOriginForLatestEventAction->setChecked(true);
	} catch ( Config::Exception& ) {}

	try {
		_configReadEventsNotOlderThanTimeSpan = SCApp->configGetDouble("readEventsNotOlderThan");
	} catch ( Config::Exception& ) {}
	readEventsFromDataBaseNotOlderThan(_configReadEventsNotOlderThanTimeSpan);
	updateEventWidget();
	if ( areOriginSymbolsVisible() )
		updateOriginSymbolDisplay();
	if ( _configReadEventsNotOlderThanTimeSpan.seconds() > 0 )
		readPicksFromDataBaseNotOlderThan(_configStationPickCacheLifeSpan);
	if ( _ui.centerOriginForLatestEventAction->isChecked() )
		centerOriginForLatestEvent();

#ifdef DEBUG_EVENTS
	std::cerr << "= Total read events from database = " << std::endl;
	std::cerr << " * Number of events: " << _eventDataRepository.eventCount() << std::endl;

	EventDataRepository::const_event_iterator eventIt = _eventDataRepository.eventsBegin();
	for ( ; eventIt != _eventDataRepository.eventsEnd(); eventIt++ )
		std::cout << eventIt->id() << std::endl;
#endif

	try {
		_configEventActivityLifeSpan = SCApp->configGetDouble("eventActivityLifeSpan");
	} catch ( Config::Exception& ) {}

	return true;
}




bool MvMainWindow::eventFilter(QObject* object, QEvent* event) {
	if ( object == _mapWidget ) {
		if ( event->type() == QEvent::MouseButtonPress ) {
			QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);

			if ( mouseEvent->button() == Qt::LeftButton	&& mouseEvent->modifiers() == Qt::ShiftModifier ) {
				QPoint pos = QPoint(mouseEvent->x(), mouseEvent->y());
				showMapCoordinates(pos);
				return true;
			}
			else if ( mouseEvent->button() == Qt::MidButton ) {
				QPoint pos = QPoint(mouseEvent->x(), mouseEvent->y());
				sendArtificialOrigin(pos);
				return true;
			}
		}
		else if ( event->type() == QEvent::ContextMenu ) {
			QContextMenuEvent* contextMenuEvent = static_cast<QContextMenuEvent*>(event);
			return handleMapContextMenu(contextMenuEvent);
		}
	}

	return Seiscomp::Gui::MainWindow::eventFilter(object, event);
}



void MvMainWindow::closeEvent(QCloseEvent *e) {
	if ( _recordStreamThread.get() )
		_recordStreamThread->stop(true);
}


void MvMainWindow::setupStandardUi() {
	_ui.setupUi(this);

	addAction(_ui.showMapLegendAction);
	addAction(_ui.showWaveformPropagationAction);
	addAction(_ui.showHistoricOriginsAction);

	_ui.splitter->setStretchFactor(0, 1);

	// Setup menu
	_ui.showEventTableWidgetAction->setChecked(false);
	_ui.showWaveformPropagationAction->setChecked(true);
	_ui.showMapLegendAction->setChecked(true);
	_ui.showHistoricOriginsAction->setChecked(true);

	_eventTableWidgetRef = new EventTableWidget;
	_eventTableWidgetRef->setVisible(_ui.showEventTableWidgetAction->isChecked());
	_ui.splitter->addWidget(_eventTableWidgetRef);

	// Setup map
	_ui.gmTab->setLayout(new QGridLayout);
	_ui.gmTab->layout()->setMargin(0);
	_ui.gmTab->layout()->setSpacing(0);

	_ui.eventTab->setLayout(new QGridLayout);
	_ui.eventTab->layout()->setMargin(0);
	_ui.eventTab->layout()->setSpacing(0);

	int index = _ui.tabWidget->indexOf(_ui.eventTab);
	_ui.tabWidget->removeTab(index);

	_ui.qcTab->setLayout(new QGridLayout);
	_ui.qcTab->layout()->setMargin(0);
	_ui.qcTab->layout()->setSpacing(0);

	QWidget* currentTab = _ui.gmTab;
	_ui.tabWidget->setCurrentWidget(currentTab);

	_mapWidget = new MvMapWidget(SCApp->mapsDesc());
	_mapWidget->installEventFilter(this);

	double lonmin = -180, lonmax = 180, latmin = -90, latmax = 90;
	try { lonmin = SCApp->configGetDouble("display.lonmin"); } catch (Config::Exception) {}
	try { lonmax = SCApp->configGetDouble("display.lonmax"); } catch (Config::Exception) {}
	try { latmin = SCApp->configGetDouble("display.latmin"); } catch (Config::Exception) {}
	try { latmax = SCApp->configGetDouble("display.latmax"); } catch (Config::Exception) {}

	QRectF displayRect;
	displayRect.setRect(lonmin, latmin, lonmax-lonmin, latmax-latmin);
	_mapWidget->canvas().displayRect(displayRect);

	_ui.gmTab->layout()->addWidget(_mapWidget);

	connect(_ui.tabWidget, SIGNAL(currentChanged(int)), this, SLOT(changeView(int)));

	connect(_ui.showWaveformPropagationAction, SIGNAL(toggled(bool)), this, SLOT(setWaveformPropagationVisible(bool)));
	connect(_ui.showEventTableWidgetAction, SIGNAL(toggled(bool)), _eventTableWidgetRef, SLOT(setVisible(bool)));
	connect(_ui.showStationIdAction, SIGNAL(toggled(bool)), this, SLOT(setStationIdVisible(bool)));
	connect(_ui.actionShowStationChannelCodes, SIGNAL(toggled(bool)), this, SLOT(setStationChannelCodesVisible(bool)));
	connect(_ui.showMapLegendAction, SIGNAL(toggled(bool)), _mapWidget, SLOT(showMapLegend(bool)));
	connect(_ui.showHistoricOriginsAction, SIGNAL(toggled(bool)), this, SLOT(updateOriginSymbolDisplay()));
	connect(_ui.searchStationAction, SIGNAL(triggered(bool)), this, SLOT(showSearchWidget()));
	connect(_ui.centerOriginForLatestEventAction, SIGNAL(triggered(bool)), this, SLOT(centerOriginForLatestEvent()));
	connect(_ui.resetAction, SIGNAL(triggered()), this, SLOT(resetStationData()));
	connect(_ui.quitAction, SIGNAL(triggered()), SCApp, SLOT(quit()));

	connect(_ui.qcLatencyAction, SIGNAL(triggered()), &_qualityControlStatusSelector, SLOT(selectLatency()));
	connect(_ui.qcDelayAction, SIGNAL(triggered()), &_qualityControlStatusSelector, SLOT(selectDelay()));
	connect(_ui.qcTimingQualityAction, SIGNAL(triggered()), &_qualityControlStatusSelector, SLOT(selectTimingQuality()));
	connect(_ui.qcGapsIntervalAction, SIGNAL(triggered()), &_qualityControlStatusSelector, SLOT(selectGapsInterval()));
	connect(_ui.qcGapsLengthAction, SIGNAL(triggered()), &_qualityControlStatusSelector, SLOT(selectGapsLength()));
	connect(_ui.qcOverlapsIntervalAction, SIGNAL(triggered()), &_qualityControlStatusSelector, SLOT(selectOverlapsInterval()));
	connect(_ui.qcAvailabilityAction, SIGNAL(triggered()), &_qualityControlStatusSelector, SLOT(selectAvailability()));
	connect(_ui.qcOffsetAction, SIGNAL(triggered()), &_qualityControlStatusSelector, SLOT(selectOffset()));
	connect(_ui.qcRmsAction, SIGNAL(triggered()), &_qualityControlStatusSelector, SLOT(selectRms()));

	connect(_eventTableWidgetRef, SIGNAL(eventSelected(const QString&)), this, SLOT(selectStationsForEvent(const QString&)));
	connect(_eventTableWidgetRef, SIGNAL(eventSelected(const QString&)), this, SLOT(selectEvent(const QString&)));

	connect(_eventTableWidgetRef, SIGNAL(eventDoubleClicked(const QString&)), this, SLOT(centerOriginForEvent(const QString&)));
	connect(_eventTableWidgetRef, SIGNAL(eventDoubleClicked(const QString&)), this, SLOT(selectStationsForEvent(const QString&)));
	connect(_eventTableWidgetRef, SIGNAL(eventDoubleClicked(const QString&)), this, SLOT(selectEvent(const QString&)));

	connect(_eventTableWidgetRef, SIGNAL(eventDeselected(const QString&)), this, SLOT(deselectEvents()));
	connect(_eventTableWidgetRef, SIGNAL(eventDeselected(const QString&)), this, SLOT(deselectStations()));

	connect(SCApp, SIGNAL(messageAvailable(Seiscomp::Core::Message*, Seiscomp::Communication::NetworkMessage*)),
	        this, SLOT(handleNewMessage(Seiscomp::Core::Message*)));

	connect(&_mapUpdateTimer, SIGNAL(timeout()), this, SLOT(updateMap()));

	_mapUpdateTimer.start(_mapUpdateInterval);
}




void MvMainWindow::modifyUiSetupForDisplayMode() {
	// Overwrite default menu settings
	bool showLegend = false;

	try { showLegend = SCApp->configGetBool("legend"); } catch ( ... ) {}
	if ( SCApp->commandline().hasOption("with-legend") ) showLegend = true;

	_ui.showMapLegendAction->setChecked(showLegend);
	_ui.showHistoricOriginsAction->setChecked(false);

	QWidget* currentTab = _ui.tabWidget->currentWidget();
	currentTab->layout()->removeWidget(_mapWidget);

	this->setCentralWidget(_mapWidget);
	_eventTableWidgetRef = NULL;

	menuBar()->setVisible(false);
	statusBar()->setVisible(false);

	if ( _displayMode == GROUND_MOTION )
		ApplicationStatus::Instance()->setMode(ApplicationStatus::GROUND_MOTION);
	else if ( _displayMode == QUALITY_CONTROL )
		ApplicationStatus::Instance()->setMode(ApplicationStatus::QC);
}




bool MvMainWindow::initRecordStream() {
	if ( !SCApp->isRecordStreamEnabled() ) {
		SEISCOMP_DEBUG("No record stream source specified");
		return false;
	}

	_recordStreamThread = RecordStreamThreadPtr(new Gui::RecordStreamThread(SCApp->recordStreamURL()));

	if ( !_recordStreamThread->connect() ) {
		SEISCOMP_ERROR("Could not open recordstream %s", SCApp->recordStreamURL().c_str());
		return false;
	}

	connect(_recordStreamThread.get(), SIGNAL(receivedRecord(Seiscomp::Record*)),
			this, SLOT(handleNewRecord(Seiscomp::Record*)));

	StationDataCollection::iterator it = _stationDataCollection.begin();
	for ( ; it != _stationDataCollection.end(); ++it ) {
		std::vector<std::string> tokens;
		Core::split(tokens, it->id.c_str(), ".", false);

		_recordStreamThread->addStream(tokens[0],tokens[1],tokens[2], tokens[3]);

		OPT(double) gain = SCApp->query()->getComponentGain(tokens[0], tokens[1],
		                                                    tokens[2], tokens[3],
		                                                    Core::Time::GMT());
		if ( gain )
			it->gmGain = gain.get();
	}

	_recordStreamThread->start();

	return true;
}




void MvMainWindow::showMapCoordinates(const QPoint& pos) {
	QPointF mapPos;
	if ( !_mapWidget->canvas().projection()->unproject(mapPos, pos) ) return;

	statusBar()->showMessage(QString("Latitude: %1 Longitude: %2") .arg(mapPos.y()).arg(mapPos.x()), int(2E3));
}




bool MvMainWindow::handleMapContextMenu(QContextMenuEvent* contextMenuEvent) {
	typedef std::vector<QAction*> ActionCollection;
	ActionCollection stationActionCollection;
	ActionCollection originActionCollection;

	Gui::Map::SymbolCollection::const_iterator it = _mapWidget->canvas().symbolCollection()->begin();
	for ( ; it != _mapWidget->canvas().symbolCollection()->end(); it++ ) {
		Gui::Map::Symbol* mapSymbol = *it;

		if ( !mapSymbol->isInside(contextMenuEvent->x(), contextMenuEvent->y()) )
			continue;

		if ( mapSymbol->typeInfo() == Gui::OriginSymbol::TypeInfo() ) {
			QString title = QString("Event %1").arg(mapSymbol->id().c_str());
			QAction* action = createAndConfigureContextMenuAction(title, mapSymbol);

			originActionCollection.push_back(action);
		}
		else if ( mapSymbol->typeInfo() == MvStationSymbol::TypeInfo() ) {
			QString title = QString("Station: %1").arg(mapSymbol->id().c_str());
			QAction* action = createAndConfigureContextMenuAction(title, mapSymbol);

			StationData* stationData = _stationDataCollection.find(mapSymbol->id());
			if ( stationData->tAmplitudeTime != Core::Time::Null ) {
				QFont font;
				font.setBold(true);
				action->setFont(font);
			}

			stationActionCollection.push_back(action);
		}
	}

	QMenu menu("Select Mapsymbol", this);
	addOriginsToContextMenu(menu, originActionCollection);
	addStationsToContextMenu(menu, stationActionCollection);

	if ( !menu.isEmpty()) {
		menu.exec(contextMenuEvent->globalPos());
		return true;
	}
	return false;
}




QAction* MvMainWindow::createAndConfigureContextMenuAction(const QString& title, Gui::Map::Symbol* mapSymbol) {
	QAction* action = new QAction(title, NULL);

	QVariant variant = qVariantFromValue(static_cast<void*>(mapSymbol));
	action->setData(variant);

	connect(action, SIGNAL(triggered()), this, SLOT(showInfoWidget()));

	return action;
}




void MvMainWindow::addStationsToContextMenu(QMenu& menu, std::vector<QAction*>& collection) {
	bool hasActiveArrival = false;
	std::vector<QAction*>::iterator it = collection.begin();
	for ( ; it != collection.end(); it++ ) {
		bool hasBoldFont = (*it)->font().bold();
		if ( hasBoldFont ) {
			menu.addAction(*it);
			hasActiveArrival = true;
		}
	}

	if ( hasActiveArrival )
		menu.addSeparator();

	it = collection.begin();
	for ( ; it != collection.end(); it++ ) {
		bool hasBoldFont = (*it)->font().bold();
		if ( !hasBoldFont )
			menu.addAction(*it);
	}
}




void MvMainWindow::addOriginsToContextMenu(QMenu& menu, std::vector<QAction*>& collection) {
	std::vector<QAction*>::iterator it = collection.begin();
	for ( ; it != collection.end(); it++ ) {
		menu.addAction(*it);
	}

	if ( !collection.empty() )
		menu.addSeparator();
}




void MvMainWindow::sendArtificialOrigin(const QPoint& pos) {
	QPointF mapPos;
	if ( !_mapWidget->canvas().projection()->unproject(mapPos, pos) ) return;

	Gui::OriginDialog dialog(mapPos.x(), mapPos.y(), this);
	dialog.move(pos);
	if ( dialog.exec() != QDialog::Accepted ) return;

	DataModel::Origin* origin = DataModel::Origin::Create();
	DataModel::CreationInfo ci;

	ci.setAgencyID(SCApp->agencyID());
	ci.setAuthor(SCApp->author());
	ci.setCreationTime(Core::Time::GMT());

	origin->setCreationInfo(ci);
	origin->setLongitude(dialog.longitude());
	origin->setLatitude(dialog.latitude());
	origin->setDepth(DataModel::RealQuantity(dialog.depth()));
	origin->setTime(Core::Time(dialog.getTime_t()));

	SCApp->sendCommand(Gui::CM_OBSERVE_LOCATION, "", origin);
}




void MvMainWindow::updateMap() {
	ApplicationStatus::Instance()->setTriggering(!ApplicationStatus::Instance()->isTriggering());

	updateEventData();

	StationDataCollection::iterator it = _stationDataCollection.begin();
	for ( ; it != _stationDataCollection.end(); ++it ) {
		_recordHandler.update(&(*it));
		_triggerHandler.update(&(*it));
		_qcHandler.update(&(*it));

		StationRenderParameter* renderTraits = selectRenderParameterInstance(it->isEnabled);
		renderTraits->setStationData(&(*it));

		it->stationSymbolRef->setColor(renderTraits->color());
		it->stationSymbolRef->setFrameColor(renderTraits->frameColor());
		it->stationSymbolRef->setFrameSize(renderTraits->frameSize());
		it->stationSymbolRef->setPriority(renderTraits->priority());

		it->stationSymbolRef->setCharacterDrawingEnabled(false);
		if ( renderTraits->hasAnnotation() ) {
			it->stationSymbolRef->setCharacterDrawingEnabled(true);
			it->stationSymbolRef->setCharacterDrawingColor(SCScheme.colors.stations.text);
			it->stationSymbolRef->setCharacter(renderTraits->annotation());
		}
	}

	// FIX: Is this the only way to update the mapWidget? A simple
	// update didn't suffice
	_mapWidget->update();
}




void MvMainWindow::changeView(int index) {
	QWidget* precedingTab = _ui.tabWidget->currentWidget();
	precedingTab->layout()->removeWidget(_mapWidget);

	QWidget* w = _ui.tabWidget->widget(index);
	w->layout()->addWidget(_mapWidget);

	bool isQcTabSelected = precedingTab != _ui.qcTab &&
	                       _ui.tabWidget->currentWidget() == _ui.qcTab;

	bool isQcTabDeselected = precedingTab == _ui.qcTab &&
	                         _ui.tabWidget->currentWidget() != _ui.qcTab;

	if ( isQcTabSelected ) {
		showOriginSymbols(false);
		disconnect(_ui.showHistoricOriginsAction, 0, 0, 0);
	}
	else if ( isQcTabDeselected ) {
		updateOriginSymbolDisplay();
		connect(_ui.showHistoricOriginsAction, SIGNAL(toggled(bool)),
		        this, SLOT(updateOriginSymbolDisplay()));
	}

	if ( _ui.tabWidget->currentWidget() == _ui.gmTab )
		ApplicationStatus::Instance()->setMode(ApplicationStatus::GROUND_MOTION);
	else if ( _ui.tabWidget->currentWidget() == _ui.eventTab )
		ApplicationStatus::Instance()->setMode(ApplicationStatus::EVENT);
	else if ( _ui.tabWidget->currentWidget() == _ui.qcTab )
		ApplicationStatus::Instance()->setMode(ApplicationStatus::QC);

	_mapWidget->setMode(ApplicationStatus::Instance()->mode());
}




bool MvMainWindow::readStationsFromDataBase() {
	// Read configured streams
	DataModel::ConfigModule *module = SCApp->configModule();
	if ( !module ) return false;

	std::map<std::string, StationData> stations;

	for ( size_t i = 0; i < module->configStationCount(); ++i ) {
		DataModel::ConfigStation *cs = module->configStation(i);
		DataModel::Setup *setup = findSetup(cs, SCApp->name());
		if ( setup ) {
			DataModel::ParameterSet* ps = DataModel::ParameterSet::Find(setup->parameterSetID());
			if ( !ps ) {
				SEISCOMP_ERROR("Cannot find parameter set %s", setup->parameterSetID().c_str());
				continue;
			}

			std::string net = cs->networkCode();
			std::string sta = cs->stationCode();

			Util::KeyValues params;
			params.init(ps);

			std::string location;
			std::string channel;

			params.getString(location, "detecLocid");
			params.getString(channel, "detecStream");

			if ( channel.empty() ) continue;

			if ( channel.size() < 3 ) {
				char compCode = 'Z';
				DataModel::SensorLocation *sloc = Client::Inventory::Instance()->getSensorLocation(net, sta, location, Core::Time::GMT());
				if ( sloc ) {
					DataModel::Stream *stream = DataModel::getVerticalComponent(sloc, channel.c_str(), Core::Time::GMT());
					if ( stream )
						channel = stream->code();
					else
						channel += compCode;
				}
				else
					channel += compCode;
			}

			StationData stationData;
			stationData.id = net + "." + sta + "." + location + "." + channel;
			stationData.isEnabled = cs->enabled();
			stations.insert(make_pair(net + "." + sta, stationData));
			SEISCOMP_DEBUG("Adding station id: %s", stationData.id.data());
		}
	}

	// Read configured stations
	DataModel::Inventory* inventory = Client::Inventory::Instance()->inventory();
	if ( !inventory ) return false;

	Core::Time now = Core::Time::GMT();

	for ( size_t i = 0; i < inventory->networkCount(); ++i ) {
		DataModel::NetworkPtr network = inventory->network(i);
		for ( size_t j = 0; j < network->stationCount(); ++j ) {
			DataModel::StationPtr station = network->station(j);
			if ( station->start() > now ) continue;

			try {
				if ( station->end() < now ) continue;
			}
			catch ( Core::ValueException& ) {}

			if ( _mapWidget->canvas().isInside(station->longitude(), station->latitude()) ) {
				std::string key = station->network()->code() + "." + station->code();
				std::map<std::string, StationData>::iterator it = stations.find(key);
				if ( it == stations.end() ) continue;

				it->second.stationRef = station;
				it->second.stationSymbolRef = new MvStationSymbol(station->latitude(),
				                                                  station->longitude());
				it->second.stationSymbolRef->setID(it->second.id);
				it->second.stationSymbolRef->setNetworkCode(station->network()->code());
				it->second.stationSymbolRef->setStationCode(station->code());
				it->second.stationSymbolRef->setIdDrawingColor(SCScheme.colors.map.stationAnnotations);
				it->second.stationSymbolRef->setIdDrawingEnabled(_ui.showStationIdAction->isChecked());
				it->second.stationSymbolRef->setDrawFullID(_ui.actionShowStationChannelCodes->isChecked());

				_stationDataCollection.add(it->second);
				_mapWidget->canvas().symbolCollection()->add(it->second.stationSymbolRef);
				SEISCOMP_DEBUG("Adding station symbol: %s", key.data());
			}
		}
	}

	return true;
}




void MvMainWindow::readPicksFromDataBaseNotOlderThan(const Core::TimeSpan& timeSpan) {
	Core::Time begin = Core::Time::GMT() - timeSpan;
	Core::Time end = Core::Time::GMT();

#ifdef DEBUG_AMPLITUDES
	std::vector<std::string> pickIds;
#endif

	DataModel::DatabaseIterator it = SCApp->query()->getPicks(begin, end);
	for ( ; *it != NULL; ++it ) {
		DataModel::Pick* pick = DataModel::Pick::Cast(*it);
		if ( !pick ) continue;

		handleNewPick(pick);

#ifdef DEBUG_AMPLITUDES
		pickIds.push_back(pick->publicID());
#endif

	}

#ifdef DEBUG_AMPLITUDES
	size_t count = 0;
	for ( std::vector<std::string>::const_iterator it = pickIds.begin();
			it != pickIds.end(); it++ ) {
		DataModel::Amplitude* amplitude = debugGetAmplitudeForPickFromDataBase(*it);
		if ( amplitude ) {
			handleNewAmplitude(amplitude);
		}
		else {
			std::cout << "Could not get amplitude for pick " << *it << " from database" << std::endl;
			count++;
		}
	}
	std::cout << "A total of " << count << " could not be retrieved from database" << std::endl;
#endif

}




void MvMainWindow::readEventsFromDataBaseNotOlderThan(const Seiscomp::Core::TimeSpan& timeSpan) {
	Core::Time begin = Core::Time::GMT() - timeSpan;
	Core::Time end = Core::Time::GMT();

	typedef std::vector<DataModel::EventPtr> EventCollection;
	EventCollection events;

	DataModel::DatabaseIterator it = SCApp->query()->getEvents(begin, end);
	for ( ; *it; ++it ) {
		DataModel::Event* event = DataModel::Event::Cast(*it);
		if ( !event ) continue;
		if ( isFakeEvent(event) ) {
			SEISCOMP_DEBUG("Skipping fake event %s", event->publicID().c_str());
			continue;
		}

		events.push_back(event);
	}

	EventCollection::iterator eventIt = events.begin();
	for ( ; eventIt != events.end(); eventIt++ ) {
		std::string eventId = (*eventIt)->publicID();
		std::string preferredOriginId = (*eventIt)->preferredOriginID();

		DataModel::Origin* origin = DataModel::Origin::Cast(SCApp->query()->getObject(DataModel::Origin::TypeInfo(),
		                                                                              preferredOriginId));
		if ( !origin ) {
			SEISCOMP_DEBUG("Preferred origin not found, skipping event %s", eventId.c_str());
			continue;
		}

		SCApp->query()->load(origin);
		handleNewOrigin(origin);

		std::string preferredMagnitudeId = (*eventIt)->preferredMagnitudeID();
		DataModel::Magnitude* magnitude =
				DataModel::Magnitude::Cast(SCApp->query()->getObject(DataModel::Magnitude::TypeInfo(),
				                                                     preferredMagnitudeId));
		if ( !magnitude ) {
			SEISCOMP_DEBUG("Preferred magnitude not found, skipping event %s", eventId.c_str());
			continue;
		}

		handleNewMagnitude(magnitude);

		SCApp->query()->load((*eventIt).get());
		handleNewEvent((*eventIt).get());
	}
}




void MvMainWindow::handleNewConfigStation(const DataModel::ConfigStation* configStation) {
	StationDataCollection::iterator it = _stationDataCollection.begin();
	for ( ; it != _stationDataCollection.end(); it++ ) {
		std::vector<std::string> tokens;
		Core::split(tokens, it->id.c_str(), ".", false);

		bool match = tokens[0] == configStation->networkCode() &&
					 tokens[1] == configStation->stationCode();
		if ( match )
			it->isEnabled = configStation->enabled();
	}
}




void MvMainWindow::handleWaveformQuality(DataModel::WaveformQuality* waveformQuality) {
	std::string stationId = getStationId(waveformQuality);
	StationData* stationData = _stationDataCollection.find(stationId);
	if ( !stationData ) return;

	_qcHandler.handle(stationData, waveformQuality);
}




void MvMainWindow::handleNewArrival(DataModel::Arrival* arrival) {
	_eventDataRepository.addArrival(arrival);
}




void MvMainWindow::handleNewPick(DataModel::Pick* pick) {
	std::string stationId = getStationId(pick);
	StationData* stationData = _stationDataCollection.find(stationId);
	if ( stationData ) {
		_triggerHandler.handle(stationData, pick);
		_eventDataRepository.addPick(pick);
	}
}




void MvMainWindow::handleNewAmplitude(DataModel::Amplitude* amplitude) {
	if ( amplitude->type() != "snr" )
		return;

	std::string stationId = getStationId(amplitude);
	StationData* stationData = _stationDataCollection.find(stationId);
#ifdef DEBUG_AMPLITUDES
	std::cout << "Delivering Amplitude " << amplitude->publicID() << " to station " << stationId << std::endl;
#endif
	if ( stationData ) {
		_triggerHandler.handle(stationData, amplitude);
		_eventDataRepository.addAmplitude(amplitude);
		updateInfoWidget(amplitude);
	}
}




void MvMainWindow::handleNewOrigin(Seiscomp::DataModel::Origin* origin) {
	_eventDataRepository.addOrigin(origin);
}




void MvMainWindow::handleNewMagnitude(Seiscomp::DataModel::Magnitude* magnitude) {
	_eventDataRepository.addMagnitude(magnitude);
}




void MvMainWindow::handleNewEvent(DataModel::Event* event) {
	Gui::OriginSymbol* originSymbol = createOriginSymbolFromEvent(event);
	if ( !originSymbol ) return;

	if ( !_eventDataRepository.addEvent(event, originSymbol) ) {
		delete originSymbol;
		return;
	}

	_mapWidget->canvas().symbolCollection()->add(originSymbol);
}




void MvMainWindow::handleEventUpdate(DataModel::Event* event, EventData* eventData) {
	//std::string eventId = event->publicID();
	//_eventDataRepository.removeEvent(eventId);

	//Gui::OriginSymbol* originSymbol = eventData->originSymbol();
	//_mapWidget->canvas().symbolCollection()->remove(originSymbol);
    removeEventData(eventData);
	handleNewEvent(event);
}




bool MvMainWindow::checkIfEventIsActive(const EventData& eventData) {
	std::string originId = eventData.preferredOriginId();
	DataModel::Origin* origin = _eventDataRepository.findOrigin(originId);

	Core::Time referenceTime = Core::Time::GMT() - _configEventActivityLifeSpan;
	if ( origin->time().value() < referenceTime )
		return false;
	return true;
}




std::vector<StationData*> MvMainWindow::getAssociatedStationsForEvents(std::vector<EventData*>::const_iterator it,
                                                                       std::vector<EventData*>::const_iterator end) {
	std::vector<StationData*> associatedStations;
	for ( ; it != end; it++ ) {
		std::vector<StationData*> tmpAssociatedStations = getAssociatedStationsForEvent(*it);
		associatedStations.insert(associatedStations.end(),
								  tmpAssociatedStations.begin(), tmpAssociatedStations.end());
	}

	return associatedStations;
}




std::vector<StationData*> MvMainWindow::getAssociatedStationsForEvent(const EventData* eventData) {
	std::string preferredOriginId = eventData->preferredOriginId();
	DataModel::Origin* origin = _eventDataRepository.findOrigin(preferredOriginId);

	typedef std::map<std::string, DataModel::PickPtr> PickMap;
	PickMap pickMap;

	DataModel::DatabaseIterator it = SCApp->query()->getPicks(preferredOriginId);
	for ( ; *it; it++ ) {
		DataModel::Pick* pick = DataModel::Pick::Cast(*it);
		pickMap.insert(std::make_pair(pick->publicID(), pick));
	}

	std::vector<StationData*> associatedStations;
	for ( size_t i = 0; i < origin->arrivalCount(); i++ ) {
		DataModel::Arrival* arrival = origin->arrival(i);
		if ( !arrival ) {
			continue;
		}

		std::string arrivalPickId = arrival->pickID();
		PickMap::iterator it = pickMap.find(arrivalPickId);
		if ( it == pickMap.end() ) continue;

		DataModel::Pick* pick = it->second.get();
		std::string stationId = getStationId(pick);
		StationData* stationData = _stationDataCollection.find(stationId);
		if ( stationData )
			associatedStations.push_back(stationData);
	}

	return associatedStations;
}




std::vector<EventData*> MvMainWindow::getActiveEvents() {
	std::vector<EventData*> activeEvents;
	EventDataRepository::event_iterator it = _eventDataRepository.eventsBegin();
	for ( ; it != _eventDataRepository.eventsEnd(); it++ ) {
		if ( it->isActive() )
			activeEvents.push_back(&(*it));
	}

	return activeEvents;
}




bool MvMainWindow::updateEventActivityStatus() {
	bool hasEventStatusChanged = false;

	EventDataRepository::event_iterator it = _eventDataRepository.eventsBegin();
	for ( ; it != _eventDataRepository.eventsEnd(); it++ ) {
		bool wasEventActive = it->isActive();
		bool isEventActive = checkIfEventIsActive(*it);
		it->setActive(isEventActive);

		if ( wasEventActive != isEventActive )
			hasEventStatusChanged = true;
	}

	return hasEventStatusChanged;
}




void MvMainWindow::updateStationsAssociatedStatus() {
	std::vector<StationData*> associatedStations;
	StationDataCollection::iterator stationDataIt = _stationDataCollection.begin();
	for ( ; stationDataIt != _stationDataCollection.end(); stationDataIt++ ) {
		if ( stationDataIt->isAssociated )
			associatedStations.push_back(&(*stationDataIt));
	}

	std::vector<EventData*> activeEvents = getActiveEvents();
	std::vector<StationData*> currentAssociatedStations = getAssociatedStationsForEvents(activeEvents.begin(),
	                                                                                     activeEvents.end());

	std::sort(associatedStations.begin(), associatedStations.end());
	std::sort(currentAssociatedStations.begin(), currentAssociatedStations.end());

	std::vector<StationData*> stationsToBeDeassociated;
	std::set_difference(associatedStations.begin(),
	                    associatedStations.end(),
	                    currentAssociatedStations.begin(),
	                    currentAssociatedStations.end(),
	                    std::back_inserter(stationsToBeDeassociated));

	std::vector<StationData*>::iterator it = stationsToBeDeassociated.begin();
	for ( ; it != stationsToBeDeassociated.end(); it++ )
		(*it)->resetTriggerRelatedData();

	it = currentAssociatedStations.begin();
	for ( ; it != currentAssociatedStations.end(); it++ ) {
		(*it)->isAssociated = true;
	}
}




void MvMainWindow::selectStations(const EventData* eventData, bool isSelected) {
	std::vector<StationData*> associatedStations = getAssociatedStationsForEvent(eventData);
	std::vector<StationData*>::iterator it = associatedStations.begin();
	for ( ; it != associatedStations.end(); it++ )
		(*it)->isSelected = isSelected;
}




void MvMainWindow::showOriginSymbols(bool val) {
	EventDataRepository::event_iterator it = _eventDataRepository.eventsBegin();
	for ( ; it != _eventDataRepository.eventsEnd(); it++ ) {
		if ( it->originSymbol()->TypeInfo() == Gui::OriginSymbol::TypeInfo() )
			it->originSymbol()->setVisible(val);
	}
}




void MvMainWindow::removeExpiredEvents() {
	while ( true ) {
		EventData* expiredEvent = _eventDataRepository.findNextExpiredEvent();
		if ( !expiredEvent ) break;

        removeEventData(expiredEvent);

		//Gui::OriginSymbol* originSymbol = expiredEvent->originSymbol();
		//_mapWidget->canvas().symbolCollection()->remove(originSymbol);

		//std::string id = expiredEvent->id();
		//_eventDataRepository.removeEvent(id);
	}
}




void MvMainWindow::removeEventData(const EventData* eventData) {
    Gui::OriginSymbol* originSymbol = eventData->originSymbol();
    _mapWidget->canvas().symbolCollection()->remove(originSymbol);

    std::string eventId = eventData->id();
    _eventDataRepository.removeEvent(eventId);
}




bool MvMainWindow::isInDisplayMode() const {
	return _displayMode != NONE && _displayMode != DisplayModeCount;
}




void MvMainWindow::updateEventData() {
	bool updateNeeded = hasEventCountChanged(_eventDataRepository) ||
	                    updateEventActivityStatus();

	if ( updateNeeded ) {
		updateEventWidget();
		updateStationsAssociatedStatus();
	}

	if ( areOriginSymbolsVisible() )
		updateOriginSymbolDisplay();
}




void MvMainWindow::updateEventWidget() {
	if ( !_eventTableWidgetRef ) return;

	_eventTableWidgetRef->removeEventTableEntries();

	std::vector<std::string> eventInfo;
	EventDataRepository::event_iterator eventDataIt = _eventDataRepository.eventsBegin();
	for ( ; eventDataIt != _eventDataRepository.eventsEnd(); eventDataIt++ ) {
		std::string preferredOriginId = eventDataIt->object()->preferredOriginID();
		DataModel::Origin* origin = _eventDataRepository.findOrigin(preferredOriginId);
		if ( !origin ) {
			SEISCOMP_ERROR("Could not get preferred origin %s for event %s from database",
			               preferredOriginId.c_str(), eventDataIt->id().c_str());
			continue;
		}

		EventTableWidget::RowData rowData;
		rowData.setActive(eventDataIt->isActive());
		rowData.setSelected(eventDataIt->isSelected());

		std::string preferredMagnitudeId = eventDataIt->object()->preferredMagnitudeID();
		DataModel::Magnitude* magnitude= _eventDataRepository.findMagnitude(preferredMagnitudeId);
		if ( magnitude )
			addEventWidgetRowData(rowData, magnitude);
		addEventWidgetRowData(rowData, *eventDataIt, origin);

		_eventTableWidgetRef->addRow(rowData);
	}
}




void MvMainWindow::updateInfoWidget(StationData* stationData) {
	if ( StationInfoWidgetRegistry::Instance()->count() == 0 ) return;

	std::string stationId = stationData->id;
	StationInfoWidget* infoWidget = StationInfoWidgetRegistry::Instance()->find(stationId);
	if ( !infoWidget ) return;

	setInfoWidgetContent(infoWidget, stationData);
	infoWidget->updateContent();
}




void MvMainWindow::updateInfoWidget(const DataModel::Pick* pick) {
	if ( StationInfoWidgetRegistry::Instance()->count() == 0 ) return;

	std::string stationId = getStationId(pick);
	StationInfoWidget* infoWidget = StationInfoWidgetRegistry::Instance()->find(stationId);
	if ( !infoWidget ) return;

	Core::Time time = pick->time();
	std::string phaseHint = pick->phaseHint().code();
	infoWidget->addRecordMarker(time, phaseHint);

}




void MvMainWindow::updateInfoWidget(const DataModel::Amplitude* amplitude) {
	if ( StationInfoWidgetRegistry::Instance()->count() == 0 ) return;

	std::string stationId = getStationId(amplitude);
	StationInfoWidget* infoWidget = StationInfoWidgetRegistry::Instance()->find(stationId);
	if ( !infoWidget ) return;

	setInfoWidgetContent(infoWidget, amplitude);
	infoWidget->updateContent();
}




void MvMainWindow::updateInfoWidget(const DataModel::Event* event) {
	if ( EventInfoWidgetRegistry::Instance()->count() == 0 ) return;

	std::string eventId = event->publicID();
	OriginInfoWidget* infoWidget = EventInfoWidgetRegistry::Instance()->find(eventId);
	if ( !infoWidget ) return;

	std::string originId = event->preferredOriginID();
	DataModel::Origin* origin = _eventDataRepository.findOrigin(originId);

	setInfoWidgetContent(infoWidget, origin, event->preferredMagnitudeID());
	infoWidget->updateContent();
}




Gui::OriginSymbol* MvMainWindow::createOriginSymbolFromEvent(DataModel::Event* event) {
	DataModel::Origin* origin = _eventDataRepository.findOrigin(event->preferredOriginID());
	if ( !origin ) {
		SEISCOMP_ERROR("Could not get preferred origin %s for event %s",
					   event->preferredOriginID().c_str(),
		               event->publicID().c_str());
		return NULL;
	}

	Gui::TTDecorator* ttDecorator = new Gui::TTDecorator(&_mapWidget->canvas());
	ttDecorator->setDepth(origin->depth());
	ttDecorator->setOriginTime(origin->time());
	ttDecorator->setLatitude(origin->latitude());
	ttDecorator->setLongitude(origin->longitude());

	double magnitudeValue = 0.0;
	DataModel::Magnitude* magnitude= _eventDataRepository.findMagnitude(event->preferredMagnitudeID());
	if ( magnitude ) {
		magnitudeValue = magnitude->magnitude().value();
	}
	ttDecorator->setPreferredMagnitudeValue(magnitudeValue);
	ttDecorator->setVisible(_ui.showWaveformPropagationAction->isChecked());

	Gui::OriginSymbol* originSymbol = new Gui::OriginSymbol(ttDecorator);
	originSymbol->setLatitude(origin->latitude());
	originSymbol->setLongitude(origin->longitude());
	originSymbol->setID(event->publicID());
	originSymbol->setDepth(origin->depth());
	originSymbol->setPreferredMagnitudeValue(magnitudeValue);

	return originSymbol;
}



void MvMainWindow::showSearchWidget() {
	SearchWidget* tmp = SearchWidget::Create();
	if ( !tmp ) return;

	_searchWidgetRef = tmp;

	connect(_searchWidgetRef, SIGNAL(stationSelected()), this, SLOT(markSearchWidgetResults()));
	connect(_searchWidgetRef, SIGNAL(destroyed()), this, SLOT(searchWidgetClosed()));

	StationDataCollection::iterator it = _stationDataCollection.begin();
	for ( ; it != _stationDataCollection.end(); it ++ )
		_searchWidgetRef->addStationName(it->id);

	int offset = 20;
	_searchWidgetRef->move(pos().x() + offset, pos().y() + offset);
	_searchWidgetRef->show();
}




void MvMainWindow::markSearchWidgetResults() {
	if ( _searchWidgetRef->matches().empty() )
		return;

	std::string idOfFirstMatch = _searchWidgetRef->matches().front();

	Gui::Map::SymbolCollection::iterator it = _mapWidget->canvas().symbolCollection()->begin();
	for ( ; it != _mapWidget->canvas().symbolCollection()->end(); it++ ) {
		Gui::Map::Symbol* mapSymbol = *it;

		if ( mapSymbol->typeInfo() == MvStationSymbol::TypeInfo() ) {
			SearchWidget::Matches::const_iterator found = std::find(_searchWidgetRef->matches().begin(),
			                                                        _searchWidgetRef->matches().end(),
			                                                        mapSymbol->id());
			if ( found != _searchWidgetRef->matches().end() ) {
				mapSymbol->setVisible(true);
				if ( idOfFirstMatch == mapSymbol->id() )
					_mapWidget->canvas().centerMap(QPoint(static_cast<MvStationSymbol*>(mapSymbol)->x(),
					                             static_cast<MvStationSymbol*>(mapSymbol)->y()));
			}
			else {
				mapSymbol->setVisible(false);
			}
		}
		else if ( mapSymbol->typeInfo() == Gui::OriginSymbol::TypeInfo() ) {
			mapSymbol->setVisible(false);
		}
	}
}




void MvMainWindow::searchWidgetClosed() {
	Gui::Map::SymbolCollection::iterator it = _mapWidget->canvas().symbolCollection()->begin();
	for ( ; it != _mapWidget->canvas().symbolCollection()->end(); it++ ) {
		Gui::Map::Symbol* mapSymbol = *it;

		if ( mapSymbol->typeInfo() == MvStationSymbol::TypeInfo() )
			mapSymbol->setVisible(true);
	}

	if ( areOriginSymbolsVisible() )
		updateOriginSymbolDisplay();
}




void MvMainWindow::handleNewRecord(Seiscomp::Record* record) {
	Seiscomp::RecordPtr recordPtr(record);

	std::string id = getStationId(record);

	StationData* stationData = _stationDataCollection.find(id);
	if ( stationData )
		_recordHandler.handle(stationData, record);
}




void MvMainWindow::handleNewMessage(Core::Message* message) {
	Core::DataMessage* dataMessage = Core::DataMessage::Cast(message);
	if ( dataMessage ) {
		Core::DataMessage::iterator it = dataMessage->begin();
		for ( ; it != dataMessage->end(); ++it ) {
			DataModel::WaveformQuality* waveformQuality = DataModel::WaveformQuality::Cast(*it);
			if ( waveformQuality ) {
				handleWaveformQuality(waveformQuality);

				std::string stationId = getStationId(waveformQuality);
				StationData* stationData = _stationDataCollection.find(stationId);
				if ( stationData )
					updateInfoWidget(stationData);
			}
		}
	}

	DataModel::NotifierMessage* notifierMessage = DataModel::NotifierMessage::Cast(message);
	if ( !notifierMessage ) return;

	DataModel::NotifierMessage::iterator notifierIt = notifierMessage->begin();
	for ( ; notifierIt != notifierMessage->end(); ++notifierIt ) {
		DataModel::Object* object = (*notifierIt)->object();

		DataModel::ConfigStation* configStation = DataModel::ConfigStation::Cast((*notifierIt)->object());
		if ( configStation ) {
			bool processData = SCApp->configModule() &&
			                   SCApp->configModule()->publicID() == (*notifierIt)->parentID();
			if ( processData ) {
				handleNewConfigStation(configStation);
				continue;
			}
		}

		DataModel::Arrival* arrival = DataModel::Arrival::Cast(object);
		if ( arrival ) {
			handleNewArrival(arrival);
			continue;
		}

		DataModel::Pick* pick = DataModel::Pick::Cast(object);
		if ( pick ) {
			handleNewPick(pick);
			updateInfoWidget(pick);
			continue;
		}

		DataModel::Amplitude* amplitude = DataModel::Amplitude::Cast(object);
		if ( amplitude ) {
			handleNewAmplitude(amplitude);
			updateInfoWidget(amplitude);
			continue;
		}

		DataModel::Magnitude* magnitude = DataModel::Magnitude::Cast(object);
		if ( magnitude ) {
			handleNewMagnitude(magnitude);
			continue;
		}

		DataModel::Origin* origin = DataModel::Origin::Cast(object);
		if ( origin ) {
			handleNewOrigin(origin);
			continue;
		}

		DataModel::Event* event = DataModel::Event::Cast(object);
		if ( event ) {
			std::string eventId = event->publicID();
			EventData* eventData = _eventDataRepository.findEvent(eventId);

			if ( isFakeEvent(event) ||
			     (*notifierIt)->operation() == DataModel::OP_REMOVE ) {
                if ( eventData )  {
                    removeEventData(eventData);
                    updateEventWidget();
                }
                continue;
            }

			SCApp->query()->loadEventDescriptions(event);

			bool handleAsEventUpdate = eventData;
			if ( handleAsEventUpdate ) {
				std::cout << "Event update for:" << event->publicID()  << std::endl;

				handleEventUpdate(event, eventData);
				updateInfoWidget(event);
			}
			else {
				std::cout << "New event: " << event->publicID() << std::endl;
				handleNewEvent(event);
			}

			if ( _ui.centerOriginForLatestEventAction->isChecked() )
				centerOriginForLatestEvent();

			removeExpiredEvents();
		}
	}
}




void MvMainWindow::showInfoWidget() {
	QAction* contextMenuAction = static_cast<QAction*>(sender());

	QVariant variant = contextMenuAction->data();
	void* data = variant.value<void*>();
	Gui::Map::Symbol* mapSymbol = static_cast<Gui::Map::Symbol*>(data);

	std::string mapSymbolId = mapSymbol->id();

	if ( mapSymbol->typeInfo() == MvStationSymbol::TypeInfo() ) {
		StationInfoWidget* infoWidget = new StationInfoWidget(mapSymbolId, this);

		StationData* stationData = _stationDataCollection.find(mapSymbolId);

		configureInfoWidgetForRecordAcquisition(infoWidget,
  		                                        _configStationPickCacheLifeSpan,
		                                        _configStationRecordFilterStr);

		setInfoWidgetContent(infoWidget, stationData->stationRef.get());

		if ( stationData->qcGlobalStatus != QCStatus::NOT_SET )
			setInfoWidgetContent(infoWidget, stationData);

		StationData::TPickCollection::iterator it = stationData->tPickCache.begin();
		for ( ; it != stationData->tPickCache.end(); it++ ) {
			Core::Time time = (*it)->time();
			std::string phaseHint = (*it)->phaseHint().code();
			infoWidget->addRecordMarker(time, phaseHint);
		}

		if ( stationData->tAmplitudeTime != Core::Time::Null )
			setInfoWidgetContent(infoWidget, stationData->tAmplitude.get());

		::showInfoWidget(infoWidget);
	}
	else if ( mapSymbol->typeInfo() == Gui::OriginSymbol::TypeInfo() ) {
		OriginInfoWidget* infoWidget = new OriginInfoWidget(mapSymbolId, this);

		std::string eventId = mapSymbol->id();
		EventData* eventData = _eventDataRepository.findEvent(eventId);

		std::string preferredOriginId = eventData->object()->preferredOriginID();
		DataModel::Origin* origin = _eventDataRepository.findOrigin(preferredOriginId);

		setInfoWidgetContent(infoWidget, origin, eventData->object()->preferredMagnitudeID());
		::showInfoWidget(infoWidget);
	}
}




void MvMainWindow::updateOriginSymbolDisplay() {
	bool areHistoricOriginsVisible = _ui.showHistoricOriginsAction->isChecked();
	EventData* latestEventData = _eventDataRepository.findLatestEvent();

	EventDataRepository::event_iterator it = _eventDataRepository.eventsBegin();
	for ( ; it != _eventDataRepository.eventsEnd(); it++ ) {
		EventData& eventData = *it;
		Gui::OriginSymbol* originSymbol = eventData.originSymbol();

		bool eventIsActive = eventData.isActive();
		bool eventIsSelected = eventData.isSelected();

		bool isLatestEventAndHistoricOriginsAreVisible = areHistoricOriginsVisible &&
		                                                 eventData.id() == latestEventData->id();
		bool isLatestEventAndHistoricOriginsAreNotVisible = !areHistoricOriginsVisible &&
		                                                    eventData.id() == latestEventData->id();

		if ( eventIsActive || isLatestEventAndHistoricOriginsAreVisible ) {
			originSymbol->setVisible(true);
			originSymbol->setFilled(false);
			if ( ApplicationStatus::Instance()->isTriggering() )
				originSymbol->setFilled(true);
		}
		else if ( eventIsSelected || isLatestEventAndHistoricOriginsAreNotVisible ) {
			originSymbol->setVisible(true);
			originSymbol->setFilled(false);
		}
		else {
			originSymbol->setVisible(areHistoricOriginsVisible);
			originSymbol->setFilled(false);
		}
	}
}




void MvMainWindow::setWaveformPropagationVisible(bool val) {
	Gui::Map::SymbolCollection::iterator it = _mapWidget->canvas().symbolCollection()->begin();
	for ( ; it != _mapWidget->canvas().symbolCollection()->end(); it++ ) {
		Gui::Map::Symbol* mapSymbol = *it;
		if ( Gui::OriginSymbol::TypeInfo() == mapSymbol->typeInfo() ) {
			Gui::Map::Decorator* decorator = mapSymbol->decorator();
			if ( decorator )
				decorator->setVisible(val);
		}
	}
}




void MvMainWindow::centerOriginForLatestEvent() {
	EventData* eventData = _eventDataRepository.findLatestEvent();
	if ( !eventData ) return;

	std::string eventId = eventData->id();
	centerOriginForEvent(eventId.c_str());
}




void MvMainWindow::centerOriginForEvent(const QString& id) {
	EventData* eventData = _eventDataRepository.findEvent(id.toStdString());

	std::string preferredOriginId = eventData->object()->preferredOriginID();
	DataModel::Origin* origin = _eventDataRepository.findOrigin(preferredOriginId);

	_mapWidget->canvas().setMapCenter(QPointF(origin->longitude(), origin->latitude()));
}




void MvMainWindow::resetStationData() {
	StationDataCollection::iterator it = _stationDataCollection.begin();
	for ( ; it != _stationDataCollection.end(); it++ )
		it->reset();
}



void MvMainWindow::selectStationsForEvent(const QString& eventId) {
	deselectStations();

	EventData* eventData = _eventDataRepository.findEvent(eventId.toStdString());
	selectStations(eventData);
}




void MvMainWindow::deselectStations() {
	StationDataCollection::iterator it = _stationDataCollection.begin();
	for ( ; it != _stationDataCollection.end(); it++ )
		it->isSelected = false;
}




void MvMainWindow::selectEvent(const QString& eventId) {
	deselectEvents();

	EventData* eventData = _eventDataRepository.findEvent(eventId.toStdString());
	eventData->setSelected(true);
}




void MvMainWindow::deselectEvents() {
	EventDataRepository::event_iterator eventDataIt = _eventDataRepository.eventsBegin();
	for ( ; eventDataIt != _eventDataRepository.eventsEnd(); eventDataIt++ )
		eventDataIt->setSelected(false);
}




void MvMainWindow::setStationIdVisible(bool val) {
	for ( StationDataCollection::iterator it = _stationDataCollection.begin();
			it	!= _stationDataCollection.end(); it++ ) {
		it->stationSymbolRef->setIdDrawingEnabled(val);
	}

	_mapWidget->update();
}




void MvMainWindow::setStationChannelCodesVisible(bool val) {
	for ( StationDataCollection::iterator it = _stationDataCollection.begin();
			it	!= _stationDataCollection.end(); it++ ) {
		it->stationSymbolRef->setDrawFullID(val);
	}

	_mapWidget->update();
}
