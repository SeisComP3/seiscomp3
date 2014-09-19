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


#ifndef __MAINWINDOW_H___
#define __MAINWINDOW_H___

#include "mvmapwidget.h"

#include <map>

#include <QTimer>

#include <seiscomp3/gui/core/mainwindow.h>
#include <seiscomp3/gui/core/recordstreamthread.h>
#include <seiscomp3/core/record.h>

#include <seiscomp3/datamodel/waveformquality.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/configstation.h>

#include "stationdata.h"
#include "stationdatahandler.h"
#include "eventdata.h"
#include "eventtablewidget.h"
#include "searchwidget.h"

#include "ui_mvmainwindow.h"


class QualitiyControlParameterSelector : public QObject {
	Q_OBJECT

	public slots:
		void selectDelay() {
			QCParameter::Instance()->setParameter(QCParameter::DELAY);
		}

		void selectLatency() {
			QCParameter::Instance()->setParameter(QCParameter::LATENCY);
		}

		void selectTimingQuality() {
			QCParameter::Instance()->setParameter(QCParameter::TIMING_QUALITY);
		}

		void selectGapsInterval() {
			QCParameter::Instance()->setParameter(QCParameter::GAPS_INTERVAL);
		}

		void selectGapsLength() {
			QCParameter::Instance()->setParameter(QCParameter::GAPS_LENGTH);
		}

		void selectOverlapsInterval() {
			QCParameter::Instance()->setParameter(QCParameter::OVERLAPS_INTERVAL);
		}

		void selectAvailability() {
			QCParameter::Instance()->setParameter(QCParameter::AVAILABILITY);
		}

		void selectOffset() {
			QCParameter::Instance()->setParameter(QCParameter::OFFSET);
		}

		void selectRms() {
			QCParameter::Instance()->setParameter(QCParameter::RMS);
		}
};



enum DisplayMode {
	NONE = 0x0,
	GROUND_MOTION,
	QUALITY_CONTROL,
	DisplayModeCount
};




class MvMainWindow : public Seiscomp::Gui::MainWindow {
	Q_OBJECT

	// ----------------------------------------------------------------------
	// Nested types
	// ----------------------------------------------------------------------
	private:
		typedef std::auto_ptr<Seiscomp::Gui::RecordStreamThread> RecordStreamThreadPtr;

	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		MvMainWindow(QWidget* parent = 0, Qt::WFlags = 0);
		~MvMainWindow() {}

		bool init();

	// ----------------------------------------------------------------------
	// Protected interface
	// ----------------------------------------------------------------------
	protected:
		virtual bool eventFilter(QObject* object, QEvent* event);
		virtual void closeEvent(QCloseEvent *e);

	// ----------------------------------------------------------------------
	// Private interface
	// ----------------------------------------------------------------------
	private:
		void setupStandardUi();
		void modifyUiSetupForDisplayMode();

		bool initRecordStream();

		void showMapCoordinates(const QPoint& pos);
		bool handleMapContextMenu(QContextMenuEvent* contextMenu);
		QAction* createAndConfigureContextMenuAction(const QString& title, Seiscomp::Gui::Map::Symbol* mapSymbol);
		void addStationsToContextMenu(QMenu& menu, std::vector<QAction*>& collection);
		void addOriginsToContextMenu(QMenu& menu, std::vector<QAction*>& collection);
		void sendArtificialOrigin(const QPoint& pos);

		bool readStationsFromDataBase();
		void readPicksFromDataBaseNotOlderThan(const Seiscomp::Core::TimeSpan& timeSpan);
		void readEventsFromDataBaseNotOlderThan(const Seiscomp::Core::TimeSpan& timeSpan);

		void handleNewConfigStation(const Seiscomp::DataModel::ConfigStation* configStation);
		void handleWaveformQuality(Seiscomp::DataModel::WaveformQuality* waveformQuality);
		void handleNewArrival(Seiscomp::DataModel::Arrival* arrival);
		void handleNewPick(Seiscomp::DataModel::Pick* pick);
		void handleNewAmplitude(Seiscomp::DataModel::Amplitude* amplitude);
		void handleNewMagnitude(Seiscomp::DataModel::Magnitude* magnitude);
		void handleNewOrigin(Seiscomp::DataModel::Origin* origin);
		void handleNewEvent(Seiscomp::DataModel::Event* event);
		void handleEventUpdate(Seiscomp::DataModel::Event* event, EventData* eventData);

		void updateEventData();
		void updateEventWidget();

		void updateInfoWidget(StationData* stationData);
		void updateInfoWidget(const Seiscomp::DataModel::Pick* pick);
		void updateInfoWidget(const Seiscomp::DataModel::Amplitude* amplitude);
		void updateInfoWidget(const Seiscomp::DataModel::Event* event);

		Seiscomp::Gui::OriginSymbol* createOriginSymbolFromEvent(Seiscomp::DataModel::Event* event);
		bool checkIfEventIsActive(const EventData& eventData);

		std::vector<StationData*> getAssociatedStationsForEvents(std::vector<EventData*>::const_iterator it,
		                                                         std::vector<EventData*>::const_iterator end);
		std::vector<StationData*> getAssociatedStationsForEvent(const EventData* eventData);
		std::vector<EventData*> getActiveEvents();

		bool updateEventActivityStatus();
		void updateStationsAssociatedStatus();

		void selectStations(const EventData* eventData, bool isSelected = true);
		void showOriginSymbols(bool val);

		void removeExpiredEvents();
        void removeEventData(const EventData* eventData);
        
		bool isInDisplayMode() const;

	// ----------------------------------------------------------------------
	//  slots
	// ----------------------------------------------------------------------
	private slots:
		void updateMap();
		void changeView(int index);

		void showSearchWidget();
		void markSearchWidgetResults();
		void searchWidgetClosed();

		void setStationIdVisible(bool val);

		void selectStationsForEvent(const QString& eventId);
		void deselectStations();
		void selectEvent(const QString& eventId);
		void deselectEvents();

		void handleNewRecord(Seiscomp::Record* record);
		void handleNewMessage(Seiscomp::Core::Message* message);

		void showInfoWidget();
		void updateOriginSymbolDisplay();
		void setWaveformPropagationVisible(bool val);

		void centerOriginForLatestEvent();
		void centerOriginForEvent(const QString& id);

		void resetStationData();

	// ----------------------------------------------------------------------
	// Private data members
	// ----------------------------------------------------------------------
	private:
		Ui::MvMainWindow _ui;

		MvMapWidget*      _mapWidget;
		EventTableWidget* _eventTableWidgetRef;
		SearchWidget*     _searchWidgetRef;

		DisplayMode       _displayMode;

		int    _mapUpdateInterval;
		QTimer _mapUpdateTimer;

		StationDataCollection _stationDataCollection;
		EventDataRepository   _eventDataRepository;

		RecordStreamThreadPtr _recordStreamThread;

		RecordHandler  _recordHandler;
		TriggerHandler _triggerHandler;
		QCHandler      _qcHandler;

		Seiscomp::Core::TimeSpan _configStationPickCacheLifeSpan;
		Seiscomp::Core::TimeSpan _configEventActivityLifeSpan;
		Seiscomp::Core::TimeSpan _configRemoveEventDataOlderThanTimeSpan;
		Seiscomp::Core::TimeSpan _configReadEventsNotOlderThanTimeSpan;

		std::string              _configStationRecordFilterStr;

		QualitiyControlParameterSelector _qualityControlStatusSelector;
};


#endif
