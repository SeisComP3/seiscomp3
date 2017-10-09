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

#ifndef __INFOWIDGET_H___
#define __INFOWIDGET_H___


#include <iostream>
#include <algorithm>
#include <functional>

#include <QMainWindow>
#include <QTreeWidget>
#include <QSplitter>
#include <QTimer>

#ifndef Q_MOC_RUN
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/recordsequence.h>
#endif
#include <seiscomp3/gui/core/recordwidget.h>
#include <seiscomp3/gui/core/timescale.h>
#include <seiscomp3/gui/core/recordstreamthread.h>

#ifndef Q_MOC_RUN
#include "types.h"
#endif


class InfoWidget : public QWidget {
	Q_OBJECT

	public:
		InfoWidget(const std::string& id,
				   QWidget* parent = 0, Qt::WindowFlags f = 0);

	public:
		const std::string& id() const;
		virtual void updateContent() = 0;
		virtual void init() {
			updateContent();
		}

	protected:
		QTreeWidget* treeWidget();
		QSplitter* splitter();

	private:
		void uiInit();

	private:
		std::string _id;
		QTreeWidget* _treeWidget;
		QSplitter*   _splitter;
};




class StationInfoWidget : public InfoWidget {
	Q_OBJECT

	private:
		enum StationInfoTags {
			STATION_LONGITUDE_TAG,
			STATION_LATITUDE_TAG,
			STATION_ELEVATION_TAG,
			STATION_DEPTH_TAG,
			STATION_PLACE_TAG,
			STATION_COUNTRY_TAG,
			STATION_DESCRIPTION_TAG,
			SationInfoTagsCount
		};
		typedef std::map<StationInfoTags, QString> StationInfo;

		enum AmplitudeInfoTags {
			AMPLITUDE_TIME_TAG,
			AMPLITUDE_MAX_VALUE_TAG,
			AMPLITUDE_MAX_VALUE_TIME_TAG,
			AplitudeInfoTagsCount
		};
		typedef std::map<AmplitudeInfoTags, QString> AmplitudeInfo;

		struct QCItemData {
			QString name;
			QString value;
			QString lowerUncertainty;
			QString upperUncertainty;
			QColor  backgroundColor;

			QCItemData()
			 : backgroundColor(Qt::white) {}
		};
		typedef std::map<QString, QCItemData> QCInfo;

		typedef std::auto_ptr<Seiscomp::Gui::RecordStreamThread> RecordStreamThreadPtr;
		typedef std::list<Seiscomp::Gui::RecordMarker*>          RecordMarkerCollection;


	public:
		StationInfoWidget(const std::string& id,
		                  QWidget* parent = 0, Qt::WindowFlags f = 0);
		~StationInfoWidget();

	public:
		virtual void init();
		virtual void updateContent();

		void addRecordMarker(const Seiscomp::Core::Time& time,
		                     const std::string& phaseHint);

		void setLongitude(const QString& longitude);
		void setLatitude(const QString& latitude);
		void setElevation(const QString& elevation);
		void setDepth(const QString& depth);
		void setPlace(const QString& place);
		void setCountry(const QString& country);
		void setDescription(const QString& description);

		void setAmplitudeTime(const QString& time);
		void setAmplitudeMaxValue(const QString& value);
		void setAmplitudeMaxValueTime(const QString& time);

		void setQCContent(const QString& name, const QString& value,
		                  const QString& lowerCertainty, const QString& upperCertainty, const QColor& color);

		void setRecordFilterString(const std::string& filterStr);
		void setRecordSequeceTimeSpan(const Seiscomp::Core::TimeSpan& timeSpan);
		void setRecordStreamUrl(const std::string& url);

	protected:
		virtual void showEvent(QShowEvent*);
		virtual void resizeEvent(QResizeEvent* evt);

	private:
		void uiInit();

		void startWaveformAcquisition();
		void stopWaveformAcquisition();

		void updateStationContent();
		void updateAmplitudeContent();
		void updateQCContent();

		void adjustRecordWidgetSize();

		Seiscomp::Gui::RecordMarker* createRecordMarkerFromTime(const Seiscomp::Core::Time& time,
		                                                        const std::string& phaseHint);
		void removeExpiredRecordMarker();

		QTreeWidgetItem* createAndAddTopLevelItem(const QString& text);

		void removeContent(QTreeWidgetItem* item);
		void resizeColumnsToContent();

	private slots:
		void updateRecordWidget(Seiscomp::Record* record);
		void updateRecordWidgetAlignment();

	private:
		StationInfo   _stationInfo;
		AmplitudeInfo _amplitudeInfo;
		QCInfo        _qcInfo;

		QTreeWidgetItem* _stationItem;
		QTreeWidgetItem* _amplitudeItem;
		QTreeWidgetItem* _qcItem;

		RecordStreamThreadPtr _recordStreamThread;

		Seiscomp::Gui::RecordWidget* _recordWidget;
		Seiscomp::Gui::TimeScale*    _recordWidgetTimeScale;

		Seiscomp::RecordSequence* _recordSequence;
		Seiscomp::Core::TimeSpan  _recordSequenceTimeSpan;

		std::string _recordFilterStr;
		std::string _recordStreamUrl;

		QTimer _recordWidgetTimer;

		RecordMarkerCollection _recordMarkerCache;
};




class OriginInfoWidget : public InfoWidget {
	Q_OBJECT

	private:
		enum OriginInfoTag {
			ORIGIN_ID_TAG,
			ORIGIN_TIME_TAG,
			ORIGIN_LONGITUDE_TAG,
			ORIGIN_LATITUDE_TAG,
			ORIGIN_DEPTH_TAG,
			ORIGIN_MAGNITUDE_TAG,
			ORIGIN_AGENCY_TAG,
			ORIGIN_MODE_TAG,
			ORIGIN_STATUS_TAG,
			ORIGIN_STATION_COUNT_TAG,
			ORIGIN_AZIMUTH_GAP_TAG,
			ORIGIN_MINIMUM_DISTANCE_TAG,
			ORIGIN_MAXIMUM_DISTANCE_TAG
		};

		typedef std::map<OriginInfoTag, QString> OriginInfo;

	public:
		OriginInfoWidget(const std::string& id,
		                 QWidget* parent = 0, Qt::WindowFlags f = 0);
		~OriginInfoWidget();

	public:
		virtual void updateContent();
		void addMarker();

		void setPreferredOriginId(const QString& id);
		void setTime(const QString& time);
		void setLongitude(const QString& longitude);
		void setLatitude(const QString& latitude);
		void setDepth(const QString& depth);
		void setMagnitude(const QString& magnitude);
		void setAgency(const QString& agency);
		void setMode(const QString& mode);
		void setStatus(const QString& status);
		void setStationCount(const QString& stationCount);
		void setAzimuthGap(const QString& azimuthGap);
		void setMinimumDistance(const QString& minimumDistance);
		void setMaximumDistance(const QString& maximumDistance);

	private slots:
		void showDetails();

	private:
		void uiInit();

	private:
		OriginInfo _originInfo;
};



extern bool __compareInfoWidgetIds(const InfoWidget* infoWidget, std::string id);

template <typename T>
class InfoWidgetRegistry {
	private:
		typedef std::list<T*> InfoWidgetRegistryImpl;

	public:
		typedef typename InfoWidgetRegistryImpl::iterator iterator;
		typedef typename InfoWidgetRegistryImpl::const_iterator const_iterator;

	public:
		size_t infoWidgetCount() const {
			return _infoWidgetRegistryImpl.size();
		}

		void add(T* infoWidget) {
			_infoWidgetRegistryImpl.push_back(infoWidget);
		}

		void remove(T* infoWidget) {
			iterator it = std::find(begin(), end(), infoWidget);
			if ( it != end() )
				_infoWidgetRegistryImpl.erase(it);
		}

		T* find(const std::string& id) {
			iterator it = std::find_if(begin(), end(),
			                           std::bind2nd(std::ptr_fun(__compareInfoWidgetIds), id));
			if ( it != end() )
				return *it;
			return NULL;
		}

		size_t count() const {
			return _infoWidgetRegistryImpl.size();
		}

		iterator begin() {
			return _infoWidgetRegistryImpl.begin();
		}

		iterator end() {
			return _infoWidgetRegistryImpl.end();
		}

		const_iterator begin() const {
			return _infoWidgetRegistryImpl.begin();
		}

		const_iterator end() const {
			return _infoWidgetRegistryImpl.end();
		}

	public:
		static InfoWidgetRegistry* Instance() {
			if ( _Registry == NULL )
				_Registry = new InfoWidgetRegistry;

			return _Registry;
		}

	private:
		InfoWidgetRegistryImpl _infoWidgetRegistryImpl;

		static InfoWidgetRegistry* _Registry;
};


typedef InfoWidgetRegistry<StationInfoWidget> StationInfoWidgetRegistry;
typedef InfoWidgetRegistry<OriginInfoWidget> EventInfoWidgetRegistry;




#endif
