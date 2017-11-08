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



#ifndef __SEISCOMP_GUI_EVENTSUMMARY_H__
#define __SEISCOMP_GUI_EVENTSUMMARY_H__

#include <QWidget>
#include <string>
#include <set>

#ifndef Q_MOC_RUN
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/focalmechanism.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/databasequery.h>
#endif
#include <seiscomp3/gui/core/gradient.h>
#include <seiscomp3/gui/map/mapwidget.h>
#include <seiscomp3/gui/qt4.h>

#include <seiscomp3/gui/datamodel/ui_eventsummary.h>

namespace Seiscomp {
namespace Gui {

class OriginSymbol;

class SC_GUI_API EventSummaryMagnitudeRow : public QHBoxLayout {
	Q_OBJECT

	public:
		EventSummaryMagnitudeRow(const std::string &type,
		                         QWidget *parent = 0);

		void reset();
		void select(bool);

		void set(const std::string &id, double value, int stationCount);

	signals:
		void clicked(const std::string &magID);

	protected:
		bool eventFilter(QObject *obj, QEvent *event);

	private:
		void setMagnitude(double value, int stationCount);

	public:
		std::string magnitudeID;
		QLabel *label;
		QLabel *value;
};

class SC_GUI_API EventSummary : public QWidget {
	Q_OBJECT

	public:
		EventSummary(const MapsDesc &maps,
		             DataModel::DatabaseQuery* reader,
		             QWidget *parent = 0);
		EventSummary(Map::ImageTree* mapTree,
		             DataModel::DatabaseQuery* reader,
		             QWidget * parent = 0);
		~EventSummary();

	public:
		DataModel::Event *currentEvent() const;
		DataModel::Origin *currentOrigin() const;

		//! Negative value switches to default behaviour
		void setDefaultEventRadius(double radius);
		void setSecondDisplayUpToMaxMinutes(int);

		void addVisibleMagnitudeType(const std::string &mag);
		QList<QFrame*> separators() const;

	signals:
		void selected(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event*);
		void magnitudeSelected(const std::string &magnitudeID);

	public slots:
		void addObject(const QString& parentID, Seiscomp::DataModel::Object* obj);
		void updateObject(const QString& parentID, Seiscomp::DataModel::Object* obj);
		void removeObject(const QString& parentID, Seiscomp::DataModel::Object* obj);

		void updateOrigin(Seiscomp::DataModel::Origin* origin);

		void setEvent(Seiscomp::DataModel::Event *event,
		              Seiscomp::DataModel::Origin* org = NULL,
		              bool fixed = false);

		void showOrigin(Seiscomp::DataModel::Origin*);

	public:
		QPushButton *exportButton() const;
		MapWidget *mapWidget() const;


	private slots:
		void updateTimeAgo();
		void magnitudeClicked(const std::string &magnitudeID);

	private:
		void init();
		void setTextContrast(bool);

		void mapClicked();

		void setFocalMechanism(DataModel::FocalMechanism*);

		void setOrigin(DataModel::Origin *origin);
		void setOrigin(const std::string &originID);

		void setMagnitude(const DataModel::Magnitude *mag);
		void selectMagnitude(const std::string &type);

		void resetContent();
		void resetMagnitudes();

		void updateContent();
		void updateMagnitude();
		void updateOrigin();
		void updateAlert();

	private:
		struct AlertSettings {
			AlertSettings() : textSize(-1) {}

			bool empty() { return commentId.empty(); }

			std::string                 commentId;
			std::vector<std::string >   commentBlacklist;

			int                         textSize;
			Gui::Gradient               gradient;
		};

	private:
		Ui::EventSummary _ui;
		Map::ImageTreePtr _maptree;
		MapWidget *_map;

		QTimer _timeAgo;

		DataModel::DatabaseQuery*        _reader;

		DataModel::EventPtr              _currentEvent;
		DataModel::OriginPtr             _currentOrigin;
		DataModel::MagnitudeCPtr         _currentMag;
		DataModel::FocalMechanismPtr     _currentFocalMechanism;

		OriginSymbol *_symbol;

		QBoxLayout *_magnitudeRows;

		typedef std::set<std::string> MagnitudeTypes;
		typedef std::map<std::string, EventSummaryMagnitudeRow*> MagnitudeList;

		MagnitudeTypes                   _visibleMagnitudes;
		MagnitudeList                    _magnitudes;
		AlertSettings                    _alertSettings;

		bool                             _alertActive;
		bool                             _fixedView;
		bool                             _showComment;

		double                           _defaultEventRadius;
		int                              _maxMinutesSecondDisplay;

	friend class EventSummaryMap;
};


}
}

#endif
