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



#ifndef __SEISCOMP_GUI_PICKERVIEW_H__
#define __SEISCOMP_GUI_PICKERVIEW_H__

#include <seiscomp3/gui/datamodel/ui_pickerview.h>
#include <seiscomp3/gui/core/recordview.h>
#include <seiscomp3/gui/core/connectionstatelabel.h>
#include <seiscomp3/gui/core/utils.h>
#ifndef Q_MOC_RUN
#include <seiscomp3/io/recordfilter/iirfilter.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/seismology/ttt.h>
#include <seiscomp3/math/matrix3.h>
#include <seiscomp3/processing/picker.h>
#endif
#include <QActionGroup>
#include <QComboBox>
#include <QSpinBox>
#include <QMovie>
#include <QSet>
#include <QLabel>
#include <QLineEdit>


namespace Seiscomp {

namespace DataModel {

class SensorLocation;

}

namespace Processing {

DEFINE_SMARTPOINTER(AmplitudeProcessor);

}

namespace Gui {

class TimeScale;
class PickerView;
class SpectrumWidget;


namespace PrivatePickerView {


class SC_GUI_API ThreeComponentTrace : public QObject {
	Q_OBJECT

	public:
		ThreeComponentTrace();
		~ThreeComponentTrace();

	public:
		void setTransformationEnabled(bool);
		void setL2Horizontals(bool);
		void setRecordWidget(RecordWidget *);
		void reset();
		void setFilter(RecordWidget::Filter *);
		bool transform(int comp = -1, Seiscomp::Record *rec = NULL);

	private slots:
		void widgetDestroyed(QObject *obj);

	public:
		typedef IO::RecordIIRFilter<float> Filter;

		// One component
		struct Component {
			std::string           channelCode;
			RecordSequence       *raw;
			RecordSequence       *transformed;
			Filter                filter;
			RecordStreamThread   *thread;
		};

		Math::Matrix3f  transformation;
		Component       traces[3];
		RecordWidget   *widget;
		bool            enableTransformation;
		bool            enableL2Horizontals;
};


class SC_GUI_API PickerRecordLabel : public StandardRecordLabel {
	Q_OBJECT

	public:
		PickerRecordLabel(int items=3, QWidget *parent=0, const char* name = 0);
		~PickerRecordLabel();


	public:
		void setConfigState(bool);

		void setControlledItem(RecordViewItem *controlledItem);
		RecordViewItem *controlledItem() const;

		void setLinkedItem(bool sm);

		void enabledExpandButton(RecordViewItem *controlledItem);
		void disableExpandButton();
		void unlink();

		bool isLinkedItem() const;
		bool isExpanded() const;

		void setLabelColor(QColor);
		void removeLabelColor();


	protected:
		void visibilityChanged(bool);
		void resizeEvent(QResizeEvent *e);
		void paintEvent(QPaintEvent *e);


	public slots:
		void extentButtonPressed();

	private slots:
		void enableExpandable(const Seiscomp::Record*);


	private:
		bool            _isLinkedItem;
		bool            _isExpanded;
		QPushButton    *_btnExpand;
		RecordViewItem *_linkedItem;
		bool            _hasLabelColor;
		QColor          _labelColor;

	private:
		double               latitude;
		double               longitude;
		int                  unit;
		QString              gainUnit[3];
		ThreeComponentTrace  data;
		Math::Matrix3f       orientationZNE;
		Math::Matrix3f       orientationZRT;

		bool                 hasGotData;
		bool                 isEnabledByConfig;

	friend class Gui::PickerView;
};


}


class SC_GUI_API PickerMarkerActionPlugin : public QObject {
	public:
		virtual ~PickerMarkerActionPlugin() {}

	public:
		//! Returns the action title as added to the context menu
		virtual QString title() const = 0;

		virtual bool init(const DataModel::WaveformStreamID &wid,
		                  const Core::Time &time) = 0;

		//! Feed a record of the current stream (including all available
		//! components)
		virtual void setRecords(RecordSequence *seqZ, RecordSequence *seq1, RecordSequence *seq2) = 0;

		//! Finalize the action, no more data will be fed a this point
		virtual void finalize() = 0;
};


DEFINE_INTERFACE_FACTORY(PickerMarkerActionPlugin);


class SpectrumViewBase : public QWidget {
	Q_OBJECT

	public:
		SpectrumViewBase(QWidget *parent = 0, Qt::WindowFlags f = 0)
		: QWidget(parent, f) {}

	protected slots:
		virtual void modeChanged(int) = 0;
		virtual void windowFuncChanged(int) = 0;
		virtual void windowWidthChanged(double) = 0;
};


class SC_GUI_API PickerView : public QMainWindow {
	public:
		struct SC_GUI_API Config {
			typedef QPair<QString, QString> FilterEntry;
			typedef QVector<FilterEntry> FilterList;
			typedef QList<QString> StringList;
			typedef StringList PhaseList;

			struct PhaseGroup {
				QString name;
				QList<PhaseGroup> childs;
			};

			typedef QList<PhaseGroup> GroupList;
			typedef QPair<float, float> Uncertainty;
			typedef QVector<Uncertainty> UncertaintyList;
			typedef QMap<QString, UncertaintyList> UncertaintyProfiles;
			typedef QPair<QString, QString> ChannelMapItem;
			typedef QMultiMap<QString, ChannelMapItem> ChannelMap;

			QString recordURL;
			ChannelMap channelMap;

			FilterList filters;

			QString integrationFilter;
			bool onlyApplyIntegrationFilterOnce;

			GroupList phaseGroups;
			PhaseList favouritePhases;

			PhaseList showPhases;

			UncertaintyProfiles uncertaintyProfiles;
			QString uncertaintyProfile;

			bool showCrossHair;

			bool ignoreUnconfiguredStations;
			bool ignoreDisabledStations;
			bool loadAllComponents;
			bool loadAllPicks;
			bool loadStrongMotionData;
			bool usePerStreamTimeWindows;
			bool limitStations;
			bool showAllComponents;
			bool hideStationsWithoutData;
			bool hideDisabledStations;

			int    limitStationCount;
			double allComponentsMaximumStationDistance;
			double defaultAddStationsDistance;

			double defaultDepth;

			bool removeAutomaticStationPicks;
			bool removeAutomaticPicks;

			Core::TimeSpan preOffset;
			Core::TimeSpan postOffset;
			Core::TimeSpan minimumTimeWindow;

			double alignmentPosition;
			double offsetWindowStart;
			double offsetWindowEnd;

			QColor timingQualityLow;
			QColor timingQualityMedium;
			QColor timingQualityHigh;

			OPT(double) repickerSignalStart;
			OPT(double) repickerSignalEnd;

			Config();

			void addFilter(const QString &f, const QString &n) {
				filters.push_back(QPair<QString, QString>(f, n));
			}

			void addShowPhase(const QString &ph) {
				showPhases.push_back(ph);
			}

			void getPickPhases(StringList &phases) const;
			void getPickPhases(StringList &phases, const QList<PhaseGroup> &groups) const;
		};


	Q_OBJECT

	public:
		//! Default c'tor
		//! The mode defaults to ringbuffer with a buffer
		//! size of 30 minutes
		PickerView(QWidget *parent = 0, Qt::WFlags f = 0);

		//! Creates a RecordView using a time window
		PickerView(const Seiscomp::Core::TimeWindow&,
		           QWidget *parent = 0, Qt::WFlags f = 0);

		//! Creates a RecordView using a timespan and
		//! a ringbuffer
		PickerView(const Seiscomp::Core::TimeSpan&,
		           QWidget *parent = 0, Qt::WFlags f = 0);

		~PickerView();

	public:
		bool setConfig(const Config &c, QString *error = NULL);

		void setDatabase(Seiscomp::DataModel::DatabaseQuery*);
		void activateFilter(int index);

		void setBroadBandCodes(const std::vector<std::string> &codes);
		void setStrongMotionCodes(const std::vector<std::string> &codes);

		//! Sets an origin an inserts the traces for each arrival
		//! in the view.
		bool setOrigin(Seiscomp::DataModel::Origin*,
		               double relTimeWindowStart,
		               double relTimeWindowEnd);

		//! Sets an origin and keeps all available traces
		bool setOrigin(Seiscomp::DataModel::Origin*);

		bool hasModifiedPicks() const;
		void getChangedPicks(ObjectChangeList<DataModel::Pick> &list) const;

		void stop();

		void selectTrace(const std::string &, const std::string &);
		void selectTrace(const Seiscomp::DataModel::WaveformStreamID &wid);


	signals:
		void originCreated(Seiscomp::DataModel::Origin*);
		void arrivalChanged(int id, bool state);
		void arrivalEnableStateChanged(int id, bool state);


	public slots:
		void setDefaultDisplay();
		void applyPicks();
		void changeFilter(int);
		void changeRotation(int);
		void changeUnit(int);
		void setArrivalState(int arrivalId, bool state);
		void addPick(Seiscomp::DataModel::Pick* pick);

		void setStationEnabled(const std::string& networkCode,
		                       const std::string& stationCode,
		                       bool state);


	private slots:
		void receivedRecord(Seiscomp::Record*);

		void updateTraceInfo(RecordViewItem*, const Seiscomp::Record*);
		void currentMarkerChanged(Seiscomp::Gui::RecordMarker*);
		void apply(QAction*);
		void setPickPhase(QAction*);
		void alignOnPhase(QAction*);
		void onAddedItem(const Seiscomp::Record*, Seiscomp::Gui::RecordViewItem*);
		void onSelectedTime(Seiscomp::Core::Time);
		void onSelectedTime(Seiscomp::Gui::RecordWidget*, Seiscomp::Core::Time);
		void setAlignment(Seiscomp::Core::Time);
		void acquisitionFinished();
		void handleAcquisitionError(const QString &msg);
		void relocate();
		void modifyOrigin();
		void updateTheoreticalArrivals();
		void itemSelected(RecordViewItem*, RecordViewItem*);
		void updateMainCursor(RecordWidget*,int);
		void updateSubCursor(RecordWidget*,int);
		void updateItemLabel(RecordViewItem*, char);
		void updateItemRecordState(const Seiscomp::Record*);
		void updateRecordValue(Seiscomp::Core::Time);
		void showTraceScaleToggled(bool);

		void specLogToggled(bool);
		void specSmoothToggled(bool);
		void specMinValue(double);
		void specMaxValue(double);
		void specTimeWindow(double);
		void specApply();

		void limitFilterToZoomTrace(bool);

		void showTheoreticalArrivals(bool);
		void showUnassociatedPicks(bool);
		void showSpectrogram(bool);
		void showSpectrum();

		void toggleFilter();
		void nextFilter();
		void previousFilter();
		void addNewFilter(const QString&);

		void scaleVisibleAmplitudes();

		void zoomSelectionHandleMoved(int, double, Qt::KeyboardModifiers);
		void zoomSelectionHandleMoveFinished();

		void changeScale(double, float);
		void changeTimeRange(double, double);

		void sortAlphabetically();
		void sortByDistance();
		void sortByAzimuth();
		void sortByResidual();
		void sortByPhase(const QString&);

		void showAllComponents(bool);
		void showZComponent();
		void showNComponent();
		void showEComponent();

		void alignOnOriginTime();
		void alignOnPArrivals();
		void alignOnSArrivals();

		void pickNone(bool);
		void pickP(bool);
		void pickS(bool);

		void scaleAmplUp();
		void scaleAmplDown();
		void scaleTimeUp();
		void scaleTimeDown();

		void scaleReset();

		void scrollLeft();
		void scrollFineLeft();
		void scrollRight();
		void scrollFineRight();

		void automaticRepick();
		void gotoNextMarker();
		void gotoPreviousMarker();

		void createPick();
		void setPick();
		void confirmPick();
		void resetPick();
		void deletePick();

		void setCurrentRowEnabled(bool);
		void setCurrentRowDisabled(bool);

		void loadNextStations();
		void showUsedStations(bool);

		void moveTraces(double offset);
		void move(double offset);
		void zoom(float factor);
		void applyTimeRange(double,double);

		void sortByState();
		void alignByState();
		void componentByState();
		void updateLayoutFromState();

		void firstConnectionEstablished();
		void lastConnectionClosed();

		void beginWaitForRecords();
		void doWaitForRecords(int value);
		void endWaitForRecords();

		void showFullscreen(bool);

		void enableAutoScale();
		void disableAutoScale();

		void addStations();

		void searchStation();
		void search(const QString&);
		void nextSearch();
		void abortSearchStation();

		void setPickPolarity();
		void setPickUncertainty();

		void openContextMenu(const QPoint &p);
		void openRecordContextMenu(const QPoint &p);

		void previewUncertainty(QAction *);
		void previewUncertainty(double lower, double upper);

		void openConnectionInfo(const QPoint &);
		void destroyedSpectrumWidget(QObject *);

		void ttInterfaceChanged(QString);
		void ttTableChanged(QString);


	protected:
		void showEvent(QShowEvent* event);

		RecordLabel* createLabel(RecordViewItem*) const;


	private:
		void figureOutTravelTimeTable();

		void init();
		void initPhases();
		bool fillTheoreticalArrivals();
		bool fillRawPicks();

		int loadPicks();

		const TravelTime* findPhase(const TravelTimeList &list, const QString &phase, double delta);

		RecordViewItem* addStream(const DataModel::SensorLocation *,
		                          const DataModel::WaveformStreamID& streamID,
		                          double distance,
		                          const std::string& text,
		                          bool showDisabled,
		                          bool addTheoreticalArrivals,
		                          const DataModel::Stream *base = NULL);

		RecordViewItem* addRawStream(const DataModel::SensorLocation *,
		                             const DataModel::WaveformStreamID& streamID,
		                             double distance,
		                             const std::string& text,
		                             bool addTheoreticalArrivals,
		                             const DataModel::Stream *base = NULL);

		void queueStream(double dist, const DataModel::WaveformStreamID& streamID, char component);

		void setupItem(const char comps[3], RecordViewItem*);
		bool addTheoreticalArrivals(RecordViewItem*,
		                            const std::string& netCode,
		                            const std::string& staCode,
		                            const std::string& locCode);

		bool addRawPick(Seiscomp::DataModel::Pick*);

		void resetState();
		void alignOnPhase(const QString&, bool theoretical);

		void diffStreamState(Seiscomp::DataModel::Origin* oldOrigin,
		                     Seiscomp::DataModel::Origin* newOrigin);

		void updateOriginInformation();

		void loadNextStations(float distance);

		void setCursorText(const QString&);
		void setCursorPos(const Seiscomp::Core::Time&, bool always = false);
		void setTimeRange(float, float);

		void acquireStreams();

		bool applyFilter(RecordViewItem *item = NULL);
		bool applyRotation(RecordViewItem *item, int type);
		void updateRecordAxisLabel(RecordViewItem *item);


		//! Makes sure that the time range [tmin, tmax] is visible.
		//! When the interval is larger than the visible area
		//! the time range will be left aligned.
		void ensureVisibility(double &tmin, double &tmax);
		void ensureVisibility(const Seiscomp::Core::Time &time, int pixelMargin);

		void updatePhaseMarker(Seiscomp::Gui::RecordWidget*, const Seiscomp::Core::Time&);
		void declareArrival(Seiscomp::Gui::RecordMarker *m, const QString &phase, bool);
		void updateUncertaintyHandles(RecordMarker *marker);

		void updateCurrentRowState();
		void setMarkerState(Seiscomp::Gui::RecordWidget*, bool);

		bool setArrivalState(Seiscomp::Gui::RecordWidget* w, int arrivalId, bool state);

		void fetchManualPicks(std::vector<RecordMarker*>* marker = NULL) const;

		void showComponent(char componentCode);
		void fetchComponent(char componentCode);

		void addArrival(Seiscomp::Gui::RecordWidget*, Seiscomp::DataModel::Arrival*, int id);
		void addFilter(const QString& name, const QString& filter);

		void changeFilter(int, bool force);

		void closeThreads();

		char currentComponent() const;

		void searchByText(const QString &text);

		void emitPick(const Processing::Picker *, const Processing::Picker::Result &res);


	private:
		struct WaveformRequest {
			WaveformRequest(double dist, const Core::TimeWindow &tw,
			                const DataModel::WaveformStreamID &sid,
			                char c)
			: distance(dist), timeWindow(tw), streamID(sid), component(c) {}

			bool operator<(const WaveformRequest &other) const {
				return distance < other.distance;
			}

			double                      distance;
			Core::TimeWindow            timeWindow;
			DataModel::WaveformStreamID streamID;
			char                        component;
		};

		struct SpectrogramOptions {
			double minRange;
			double maxRange;
			double tw;
		};

		typedef std::map<std::string, PrivatePickerView::PickerRecordLabel*> RecordItemMap;
		typedef std::list<WaveformRequest> WaveformStreamList;

		Seiscomp::DataModel::DatabaseQuery *_reader;
		QSet<QString>                       _stations;
		QComboBox                          *_comboFilter;
		QComboBox                          *_comboRotation;
		QComboBox                          *_comboUnit;
		QComboBox                          *_comboTTT;
		QComboBox                          *_comboTTTables;
		QDoubleSpinBox                     *_spinDistance;
		QComboBox                          *_comboPicker;

		QLineEdit                          *_searchStation;
		QLabel                             *_searchLabel;

		static QSize                        _defaultSpectrumWidgetSize;
		static QByteArray                   _spectrumWidgetGeometry;
		Config::UncertaintyList             _uncertainties;

		//QScrollArea* _zoomTrace;
		ConnectionStateLabel               *_connectionState;
		RecordView                         *_recordView;
		RecordWidget                       *_currentRecord;
		TimeScale                          *_timeScale;
		Seiscomp::DataModel::OriginPtr      _origin;

		Core::TimeWindow                    _timeWindowOfInterest;

		QActionGroup                       *_actionsUncertainty;
		QActionGroup                       *_actionsPickGroupPhases;
		QActionGroup                       *_actionsPickFavourites;

		QActionGroup                       *_actionsAlignOnFavourites;
		QActionGroup                       *_actionsAlignOnGroupPhases;

		QList<QMenu*>                       _menusPickGroups;
		QList<QMenu*>                       _menusAlignGroups;

		QList<QString>                      _phases;
		QList<QString>                      _showPhases;
		float                               _minTime, _maxTime;
		Core::TimeWindow                    _timeWindow;
		float                               _zoom;
		float                               _currentAmplScale;
		QString                             _currentPhase;
		QString                             _lastRecordURL;
		TravelTimeTableInterfacePtr         _ttTable;
		bool                                _centerSelection;
		bool                                _checkVisibility;
		bool                                _acquireNextStations;
		int                                 _lastFilterIndex;
		bool                                _autoScaleZoomTrace;
		bool                                _loadedPicks;
		int                                 _currentSlot;
		bool                                _alignedOnOT;
		RecordWidget::Filter               *_currentFilter;
		QString                             _currentFilterID;

		QWidget                            *_pickInfoList;

		double                              _tmpLowerUncertainty;
		double                              _tmpUpperUncertainty;

		int                                 _currentRotationMode;
		int                                 _currentUnitMode;
		int                                 _lastFoundRow;
		QColor                              _searchBase, _searchError;

		std::vector<std::string>            _broadBandCodes;
		std::vector<std::string>            _strongMotionCodes;

		WaveformStreamList                  _nextStreams;
		WaveformStreamList                  _allStreams;

		RecordItemMap                       _recordItemLabels;

		mutable ObjectChangeList<DataModel::Pick> _changedPicks;
		std::vector<DataModel::PickPtr>     _picksInTime;

		QVector<RecordStreamThread*>        _acquisitionThreads;
		QList<PickerMarkerActionPlugin*>    _markerPlugins;

		Config                              _config;
		SpectrogramOptions                  _specOpts;

		QWidget                            *_spectrumView;

		::Ui::PickerView                    _ui;
		bool                                _settingsRestored;

		static std::string                  _ttInterface;
		static std::string                  _ttTableName;
};


}
}


#define REGISTER_PICKER_MARKER_ACTION(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Gui::PickerMarkerActionPlugin, Class> __##Class##InterfaceFactory__(Service)


#endif
