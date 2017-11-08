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


#ifndef __SEISCOMP_GUI_EVENTEDIT_H__
#define __SEISCOMP_GUI_EVENTEDIT_H__

#include <QWidget>
#include <string>
#include <list>

#ifndef Q_MOC_RUN
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/focalmechanism.h>
#include <seiscomp3/datamodel/momenttensor.h>
#include <seiscomp3/datamodel/journalentry.h>
#include <seiscomp3/datamodel/databasequery.h>
#endif
#include <seiscomp3/gui/qt4.h>
#include <seiscomp3/gui/datamodel/originsymbol.h>
#include <seiscomp3/gui/datamodel/tensorsymbol.h>
#include <seiscomp3/gui/map/mapwidget.h>

#include <seiscomp3/gui/datamodel/ui_eventedit.h>

namespace Seiscomp {
namespace Gui {

// Extends tensor symbol by label and reference position
class SC_GUI_API ExtTensorSymbol  : public TensorSymbol {
	public:
		ExtTensorSymbol(const Math::Tensor2Sd &t,
		                const DataModel::FocalMechanism *fm,
		                Map::Decorator* decorator = NULL);
		~ExtTensorSymbol() {};

	public:
		void setSelected(bool selected) { _selected = selected; }
		void setDrawAgency(bool enabled) { _drawAgency = enabled; }
		void setDrawMagnitude(bool enabled) { _drawMagnitude = enabled; }
		void setDrawDepth(bool enabled) { _drawDepth = enabled; }
		void setReferencePositionEnabled(bool enabled) { _refPosEnabled = enabled; }
		void setReferencePosition(const QPointF &refPos) { _refPos = refPos; }

		const QString& agencyID() const { return _agency; }
		const Core::Time& created() const { return _created; }

	protected:
		virtual void customDraw(const Map::Canvas *canvas, QPainter &painter);

	private:
		bool            _selected;
		bool            _refPosEnabled;
		QPointF         _refPos;

		QString         _agency;
		QString         _magnitude;
		QString         _depth;
		Core::Time      _created;

		bool            _drawAgency;
		bool            _drawMagnitude;
		bool            _drawDepth;
};



// Adds context menu entries to control drawing of focal mechanism symbols
class SC_GUI_API FMMap : public MapWidget {
	Q_OBJECT

	public:
		FMMap(const MapsDesc &maps, QWidget *parent = 0, Qt::WFlags f = 0)
		 : MapWidget(maps, parent, f) { init(); }
		FMMap(Map::ImageTree* mapTree, QWidget *parent = 0, Qt::WFlags f = 0)
		 : MapWidget(mapTree, parent, f) { init(); }

		virtual ~FMMap();

		virtual void draw(QPainter&);

		void addFM(const DataModel::FocalMechanism *fm);
		void clear();
		void setCurrentFM(const std::string &id);
		void setEvent(const DataModel::Event *event);


	protected:
		void contextMenuEvent(QContextMenuEvent *e);

	private:
		void init();
		void updateSmartLayout();

	private:
		typedef std::map<std::string, ExtTensorSymbol*> FMSymbols;

		FMSymbols                   _fmSymbols;
		OriginSymbol               *_originSymbol;

		bool                        _drawAgency;
		bool                        _drawMagnitude;
		bool                        _drawDepth;
		bool                        _smartLayout;
		bool                        _groupByAgency;

		bool                        _smartLayoutDirty;
		QRectF                      _fmBoundings;
};


// Derived from Observer to receive local object modifications, because
// messages sent by other controls won't be received by the client
// application again.
class SC_GUI_API EventEdit : public QWidget, public DataModel::Observer {
	Q_OBJECT

	public:
		EventEdit(DataModel::DatabaseQuery* reader,
		          Map::ImageTree *mapTreeOrigin = NULL,
		          QWidget *parent = 0);
		~EventEdit();


	public:
		//! Sets the usage of messaging to notify about object changes.
		//! When disabled a local EventParameters instance is searched for and
		//! all updates and adds are applied to this instance.
		void setMessagingEnabled(bool);


	signals:
		void originSelected(Seiscomp::DataModel::Origin*, Seiscomp::DataModel::Event* = NULL);
		void originMergeRequested(QList<Seiscomp::DataModel::Origin*>);
		void fmSelected(Seiscomp::DataModel::FocalMechanism*, Seiscomp::DataModel::Event* = NULL);


	public slots:
		void addObject(const QString& parentID, Seiscomp::DataModel::Object* obj);
		void updateObject(const QString& parentID, Seiscomp::DataModel::Object* obj);
		void removeObject(const QString& parentID, Seiscomp::DataModel::Object* obj);

		void setEvent(Seiscomp::DataModel::Event *event, Seiscomp::DataModel::Origin *origin);
		void updateOrigin(Seiscomp::DataModel::Origin*);
		void updateFM(Seiscomp::DataModel::FocalMechanism*);

		void showTab(int);


	private slots:
		void sortOriginItems(int);
		void sortFMItems(int);
		void originSelected(QTreeWidgetItem *item, int);
		void fmSelected(QTreeWidgetItem *item, int);

		void sortMagnitudeItems(int);

		void currentTypeChanged(int);
		void currentTypeCertaintyChanged(int);
		void currentOriginChanged(QTreeWidgetItem*, QTreeWidgetItem*);
		void currentFMChanged(QTreeWidgetItem*, QTreeWidgetItem*);
		void currentMagnitudeChanged(QTreeWidgetItem*, QTreeWidgetItem*);

		void originTreeCustomContextMenu(const QPoint &);
		void magnitudeTreeCustomContextMenu(const QPoint &);

		void fixOrigin();
		void fixFM();
		void releaseOrigin();
		void releaseFM();

		void fixMagnitudeType();
		void releaseMagnitudeType();

		void fixMw();
		void releaseMw();
		void triggerMw();

		void evalResultAvailable(const QString &originID,
		                         const QString &className,
		                         const QString &script,
		                         const QString &result);

		void evalResultError(const QString &originID,
		                     const QString &className,
		                     const QString &script,
		                     int error);


	public:
		const DataModel::Event *currentEvent() const;
		void handleOrigins(const QList<DataModel::Origin*> &origins);


	private:
		void init();

		void updateContent();
		void updateEvent();
		void updateOrigin();
		void updateMagnitude();
		void updateFM();
		void updateMT();
		void updateJournal();

		void resetContent();
		void resetOrigin();
		void resetMagnitude();
		void resetFM();
		void resetMT(bool resetCurrent = false);
		bool sendJournal(const std::string &action,
		                 const std::string &params);

		void addMagnitude(DataModel::Magnitude *mag);
		void addJournal(DataModel::JournalEntry *entry);

		void updatePreferredOriginIndex();
		void updatePreferredMagnitudeIndex();
		void updatePreferredFMIndex();

		void onObjectAdded(DataModel::Object* parent, DataModel::Object* newChild);
		void onObjectRemoved(DataModel::Object* parent, DataModel::Object* oldChild);
		void onObjectModified(DataModel::Object* object);

		void insertOriginRow(DataModel::Origin *);
		void updateOriginRow(int row, DataModel::Origin *);
		void updateMagnitudeRow(int row, DataModel::Magnitude *);
		void insertFMRow(DataModel::FocalMechanism *);
		void updateFMRow(int row, DataModel::FocalMechanism *);

		void storeOrigin(DataModel::Origin *);
		void storeFM(DataModel::FocalMechanism *);
		void storeDerivedOrigin(DataModel::Origin *);
		void clearOrigins();
		void clearFMs();

		void mergeOrigins(const QList<DataModel::Origin*> &origins);

		void setFMActivity(bool);


	private:
		typedef std::list<DataModel::OriginPtr> OriginList;
		typedef std::list<DataModel::FocalMechanismPtr> FMList;
		struct ProcessColumn {
			int     pos;
			QString script;
		};

		Ui::EventEdit _ui;

		DataModel::DatabaseQuery* _reader;
		Map::ImageTreePtr         _mapTreeOrigin;
		Map::ImageTreePtr         _mapTreeFM;
		QLabel                   *_fmActivity;
		QMovie                   *_fmActivityMovie;

		DataModel::EventPtr       _currentEvent;
		bool                      _updateLocalEPInstance;
		bool                      _blockObserver;

		// origin tab
		OriginList                _origins;
		MapWidget                *_originMap;
		QRectF                    _originBoundings;
		DataModel::OriginPtr      _currentOrigin;
		DataModel::MagnitudePtr   _currentMagnitude;
		int                       _fixOriginDefaultActionCount;
		int                       _preferredOriginIdx;
		int                       _preferredMagnitudeIdx;
		QStringList               _originTableHeader;
		QVector<int>              _originColumnMap;
		int                       _customColumn;
		QString                   _customColumnLabel;
		std::string               _commentID;
		QString                   _customDefaultText;
		QMap<std::string, QColor> _customColorMap;
		QVector<ProcessColumn>    _scriptColumns;
		QHash<QString, int>       _scriptColumnMap;

		QTreeWidget              *_originTree;

		// focal mechanism tab
		FMList                       _fms;
		OriginList                   _derivedOrigins;
		FMMap                       *_fmMap;
		DataModel::FocalMechanismPtr _currentFM;
		DataModel::MomentTensorPtr   _currentMT;
		int                          _fixFMDefaultActionCount;
		int                          _preferredFMIdx;
		QStringList                  _fmTableHeader;
		QVector<int>                 _fmColumnMap;
		OriginSymbol                *_fmPrefOriginSymbol;
};


}
}

#endif
