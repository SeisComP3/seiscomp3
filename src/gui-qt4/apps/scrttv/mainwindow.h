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




#ifndef __MAINWINDOW__
#define __MAINWINDOW__

#include <QtGui>
#ifndef Q_MOC_RUN
#include <seiscomp3/core/record.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/datamodel/configstation.h>
#endif
#include <seiscomp3/gui/core/mainwindow.h>
#include <seiscomp3/gui/core/questionbox.h>
#include <seiscomp3/gui/core/recordview.h>
#include <seiscomp3/gui/core/recordstreamthread.h>
#include "progressbar.h"
#include "ui_mainwindow.h"

namespace Seiscomp {
namespace DataModel {

class Pick;

}

// 30 minutes of records per stream
#define RECORD_TIMESPAN   30*60


namespace Applications {
namespace TraceView {


struct TraceState {
	Seiscomp::Client::StationLocation location;
};

class TraceView : public Seiscomp::Gui::RecordView {
	Q_OBJECT

	public:
		TraceView(const Seiscomp::Core::TimeSpan &span,
		          QWidget *parent = 0, Qt::WFlags f = 0);

		~TraceView();

	public:
		void setTimeSpan(const Seiscomp::Core::TimeSpan &span) {
			_timeSpan = span;
		}

	public slots:
		void setDefaultDisplay() {
			setUpdatesEnabled(false);
			Seiscomp::Gui::RecordView::setDefaultDisplay();
			setJustification(1.0);
			setTimeRange(-_timeSpan,0);
			setUpdatesEnabled(true);
		}

	private:
		double _timeSpan;
};


class TraceViewTabBar : public QTabBar {
	Q_OBJECT

	public:
		TraceViewTabBar(QWidget *parent = 0);

	public:
		int findIndex(const QPoint& p);

	protected:
		void mousePressEvent(QMouseEvent *e);

	private slots:
		void textChanged();
};


class TraceTabWidget : public QTabWidget {
	Q_OBJECT

	//! Construction
	public:
		TraceTabWidget(QWidget* parent = NULL);

	signals:
		void tabRemovalRequested(int index);
		void moveSelectionRequested(Seiscomp::Gui::RecordView *target,
		                            Seiscomp::Gui::RecordView *source);

	//! Private slots
	private slots:
		void showContextMenu(const QPoint&);
		void closeTab();

	protected:
		void checkActiveTab(const QPoint& p);
		bool checkDraging(QDropEvent *event);

	//! Event handler
	protected:
		void dragEnterEvent(QDragEnterEvent *event);
		void dragMoveEvent(QDragMoveEvent *event);
		void dropEvent(QDropEvent *event);
		void showEvent(QShowEvent *);

	private:
		TraceViewTabBar* _tabBar;
		QList<QAction*> _tabActions;
		int _tabToClose;
		bool _nonInteractive;
};



class MainWindow : public Seiscomp::Gui::MainWindow {
	Q_OBJECT

	public:
		MainWindow();
		~MainWindow();

	public:
		void setFiltersByName(const std::vector<std::string> &filters);

		void setStartTime(const Seiscomp::Core::Time &t);
		void setEndTime(const Seiscomp::Core::Time &t);

		void setBufferSize(Seiscomp::Core::TimeSpan bs);

		void setAllowTimeWindowExtraction(bool);
		void setMaximumDelay(int d);
		void setShowPicks(bool);
		void setAutomaticSortEnabled(bool);
		void setInventoryEnabled(bool);

		void start();


	private slots:
		void openFile();
		void openAcquisition();
		void openXML();

		void selectStreams();
		void addTabulator();

		void cycleFilters(bool);
		void cycleFiltersReverse(bool);
		void showScaledValues(bool enable);
		void changeTraceState();

		void receivedRecord(Seiscomp::Record* r);
		void selectedTime(Seiscomp::Gui::RecordWidget*, Seiscomp::Core::Time);

		void scrollLineUp();
		void scrollLineDown();
		void scrollPageUp();
		void scrollPageDown();
		void scrollToTop();
		void scrollToBottom();
		void scrollLeft();
		void scrollRight();

		void alignOriginTime();
		void sortByStationCode();
		void sortByNetworkStationCode();
		void sortByDistance();
		void sortByConfig();

		void alignLeft();
		void alignRight();

		void jumpToLastRecord();
		void clearPickMarkers();

		void step();
		void switchToNormalState();

		void listHiddenStreams();

		void removeTab(int);

		void setupItem(const Seiscomp::Record*, Seiscomp::Gui::RecordViewItem*);
		void itemCustomContextMenuRequested(const QPoint &);
		void createOrigin(Seiscomp::Gui::RecordViewItem*, Seiscomp::Core::Time);

		void messageArrived(Seiscomp::Core::Message*, Seiscomp::Communication::NetworkMessage*);
		void objectAdded(const QString& parentID, Seiscomp::DataModel::Object*);
		void objectUpdated(const QString& parentID, Seiscomp::DataModel::Object*);

		bool addPick(Seiscomp::DataModel::Pick* pick);

		void moveSelection(Seiscomp::Gui::RecordView* target,
		                   Seiscomp::Gui::RecordView* source);

		void filterChanged(const QString&);

		void enableSearch();
		void search(const QString &text);
		void nextSearch();
		void abortSearch();

		void checkTraceDelay();


	protected:
		void toggledFullScreen(bool);
		void applyFilter();


	private:
		void openFile(const std::vector<std::string> &files);

		DataModel::ConfigStation* configStation(const std::string& networkCode,
		                                        const std::string& stationCode) const;

		bool isStationEnabled(const std::string& networkCode,
		                      const std::string& stationCode) const;

		void setStationEnabled(const std::string& networkCode,
		                       const std::string& stationCode,
		                       bool enable);

		void updateTraces(const std::string& networkCode,
		                  const std::string& stationCode,
		                  bool enable);

		TraceView* createTraceView();

		void sortByOrigin(double lat, double lon);

		void searchByText(const QString &text);


	private:
		Ui::MainWindow _ui;
		QVector<TraceView*> _traceViews;

		Seiscomp::Gui::RecordStreamThread* _recordStreamThread;

		QLabel* _statusBarFile;
		QLabel* _statusBarFilter;
		QLineEdit *_statusBarSearch;
		Seiscomp::Gui::ProgressBar* _statusBarProg;
		Seiscomp::Core::Time _endTime;
		Seiscomp::Core::Time _originTime;
		Seiscomp::Core::Time _lastRecordTime;
		Seiscomp::Core::Time _startTime;
		Core::TimeSpan _bufferSize;

		bool _autoApplyFilter;
		bool _automaticSortEnabled;
		bool _inventoryEnabled;
		int _maxDelay;

		QMap<DataModel::WaveformStreamID, double> _scaleMap;
		QColor _searchBase, _searchError;

		TraceTabWidget* _tabWidget;

		QTimer* _timer;
		QTimer* _switchBack;

		bool _allowTimeWindowExtraction;
		int  _lastFoundRow;
		bool _showPicks;
		int  _rowSpacing;
		bool _withFrames;
		int  _frameMargin;
		int  _rowHeight;
		int  _numberOfRows;

		std::vector<std::string> _filters;
		int _currentFilterIdx;

		Seiscomp::Gui::QuestionBox _questionApplyChanges;

		struct WaveformStreamEntry {
			WaveformStreamEntry(const Seiscomp::DataModel::WaveformStreamID& id, int idx, double s = 1.0)
			: streamID(id), index(idx), scale(s) {}
		
			bool operator==(const WaveformStreamEntry& other) const {
				return streamID == other.streamID;
			}

			Seiscomp::DataModel::WaveformStreamID streamID;
			int                                   index;
			double                                scale;
		};

		struct ltWaveformStreamID {
			bool operator()(const WaveformStreamEntry& left, const WaveformStreamEntry& right) const {
				return left.streamID < right.streamID;
			}
		};

		struct DecorationDesc {
			std::string matchID;
			OPT(double) minValue;
			OPT(double) maxValue;
			bool        fixedScale;
			OPT(double) gain;
			OPT(double) minMaxMargin;
			QPen        minPen;
			QBrush      minBrush;
			QPen        maxPen;
			QBrush      maxBrush;
			QString     description;
			QString     unit;
		};

		typedef std::set<WaveformStreamEntry, ltWaveformStreamID> WaveformStreamSet;
		WaveformStreamSet _waveformStreams;

		typedef std::vector<DecorationDesc> DecorationDescs;
		DecorationDescs _decorationDescs;

	friend class TraceDecorator;
};

}
}
}


#endif
