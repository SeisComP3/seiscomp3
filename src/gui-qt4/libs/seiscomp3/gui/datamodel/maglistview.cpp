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



#define SEISCOMP_COMPONENT OriginLocatorView
#include "maglistview.h"
#include <seiscomp3/gui/core/connectiondialog.h>
#include <seiscomp3/gui/core/messages.h>
#include <seiscomp3/gui/datamodel/utils.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/originquality.h>
#include <seiscomp3/datamodel/originreference.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/databasequery.h>

#include <QProgressDialog>

#include <iostream>


#define LOOKUP(reader, Type, o, what) o = Type::Cast(PublicObject::Find(what)); \
                                      if (!o && reader) \
                                      o = Type::Cast(_reader->getObject(Type::TypeInfo(), what));

using namespace Seiscomp::Client;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::IO;


namespace Seiscomp {
namespace Gui {


namespace {

//! HACK
static bool _showAll;

class SchemeTreeItem : public QTreeWidgetItem {
	protected:
		SchemeTreeItem(PublicObject* object, QTreeWidgetItem * parent = 0)
		: QTreeWidgetItem(parent), _object(object) {}

	public:
		virtual void update() = 0;

		PublicObject* object() const { return _object.get(); }

	private:
		PublicObjectPtr _object;
};


class EventTreeItem : public SchemeTreeItem {
	public:
		EventTreeItem(Event* event, QTreeWidgetItem * parent = 0)
		  : SchemeTreeItem(event, parent) {
			update();
		}

		void update() {
			Event* event = static_cast<Event*>(object());
			if ( event ) {

				setText(0, QString("%1").arg(event->publicID().c_str()));
				setText(1, QString("%1").arg(eventRegion(event).c_str()));

				Magnitude* nm = Magnitude::Cast(PublicObject::Find(event->preferredMagnitudeID()));
				if ( nm ){
					setText(2, QString("%1").arg(nm->magnitude(), 0, 'f', 1));
					setText(3, QString(nm->type().c_str()));

					//! display the station Count of a magnitude
					try {
						int staCount = nm->stationCount();
						setText(4, QString("%1").arg(staCount, 0, 'd', 0, ' '));
					}
					catch(...){
						setText(4, QString("?"));
					}
					//! -----------------------------------------------------

				}
				else{
					setText(2, "-");
					setText(3, "-");
					setText(4, "-"); // stationCount
				}

				//! this lines are for displaying defining Phase Count of an origin
				//
				Origin* origin = Origin::Cast(PublicObject::Find(event->preferredOriginID()));
				if (origin){
					try{
						OriginQuality quality = origin->quality();
						setText(5, QString("%1").arg(quality.associatedPhaseCount(), 0, 'd', 0, ' '));
					}
					catch(...){
						setText(5, "-");
					}
				}
				//! --------------------------------------------------------------

				//! HACK display 'other' fake events in different style ---------
				std::string et;
				try{et = event->type().toString();}
				catch(...){}
				if (et == "other"){
					setTextColor(0, Qt::lightGray);
					setTextColor(1, Qt::lightGray);
					setTextColor(2, Qt::lightGray);
					setTextColor(3, Qt::lightGray);
					setTextColor(4, Qt::lightGray);
					setTextColor(5, Qt::lightGray);
				}
				//! HACK ---------------------------------------------------------

			}
			else {
				setText(0, "<>");
				setText(1, "Unassociated");
			}
		}
};


class OriginTreeItem : public SchemeTreeItem {
	public:
		OriginTreeItem(Origin* origin, QTreeWidgetItem * parent = 0)
		  : SchemeTreeItem(origin, parent) {
			update();
		}

		void update() {
			Origin* origin = static_cast<Origin*>(object());
			setText(0, QString("%1").arg(origin->publicID().c_str()));
			setText(1, QString("%1").arg(timeToString(origin->time().value(), "%F %T")));
			//setText(2, QString("%1").arg(origin->arrivalCount()));
		}

		// highlight this item and move it on top of list
		void setHighlight(bool highlight) {
			QFont f = font(0);
			f.setBold(highlight);
			setFont(0,f);

			QTreeWidgetItem * p = parent();
			if ( p ) {
				bool expanded = false;
				QTreeWidget* tree = treeWidget();
				if ( tree )
					expanded = tree->isItemExpanded(p);
				p->insertChild(0, p->takeChild(p->indexOfChild(this)));
				if ( !expanded && tree )
					tree->collapseItem(p);
			}
		}
};

class NetMagTreeItem : public SchemeTreeItem {
	public:
		NetMagTreeItem(Magnitude* netMag, QTreeWidgetItem * parent = 0)
		  : SchemeTreeItem(netMag, parent) {
			update();
		}

		void update() {
			Magnitude* netMag = static_cast<Magnitude*>(object());
			setText(0, QString("%1").arg(netMag->publicID().c_str()));

			if ( netMag ){
					setText(2, QString("%1").arg(netMag->magnitude(), 0, 'f', 1));
					setText(3, QString(netMag->type().c_str()));

					//! display the station Count of a magnitude
					try {
						int staCount = netMag->stationCount();
						setText(4, QString("%1").arg(staCount, 0, 'd', 0, ' '));
					}
					catch(...){
						setText(4, QString("?"));
					}
					//! -----------------------------------------------------
				}
				else{
					setText(2, "-");
					setText(3, "-");
					setText(4, "-"); // stationCount
				}



		}

		// highlight this item and move it on top of list
		void setHighlight(bool highlight) {
			QFont f = font(0);
			f.setBold(highlight);
			setFont(0,f);

			QTreeWidgetItem * p = parent();
			if ( p ) {
				bool expanded = false;
				QTreeWidget* tree = treeWidget();
				if ( tree )
					expanded = tree->isItemExpanded(p);
				p->insertChild(0, p->takeChild(p->indexOfChild(this)));
				if ( !expanded && tree )
					tree->collapseItem(p);
			}
		}
};


} // of namespace


MagListView::MagListView(Seiscomp::DataModel::DatabaseQuery* reader, bool withOrigins,
                             QWidget * parent, Qt::WindowFlags f)
 : QWidget(parent, f), _reader(reader), _withOrigins(withOrigins), _blockSelection(false) {
	_ui.setupUi(this);

	_ui.treeWidget->setHeaderLabels(QStringList() << "PublicID" << "Desc/Time" << "Mag" << "MagType" << "MagStaCount" << "DefPhaseCount");
	_ui.treeWidget->setAlternatingRowColors(true);
	_ui.btnDbRead->setEnabled(_reader != NULL);

	initTree();

	_autoSelect = false;
	_readLock = false;

	//! HACK show all events in list, even with type 'other'
	QCheckBox* cbShowAll = new QCheckBox(this);
	cbShowAll->setToolTip("hide other events");
	cbShowAll->setTristate(false);
	cbShowAll->setCheckState(Qt::Checked);
	_ui.hboxLayout->addWidget(cbShowAll);
	connect(cbShowAll, SIGNAL(stateChanged(int)), this,  SLOT(onShowAll()));
	_showAll = false;
	//! HACK ------------1--------

	connect(_ui.btnDbRead, SIGNAL(clicked()), this, SLOT(readFromDatabase()));
	connect(_ui.btnClear, SIGNAL(clicked()), this, SLOT(clear()));
	connect(_ui.treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(itemSelected(QTreeWidgetItem*,int)));
}


//! HACK toggle show/showNot all events in list, even with type 'other'
void MagListView::onShowAll() {

	if (_showAll)
		_showAll = false;
	else
		_showAll = true;

	readFromDatabase();

}
//! HACK show all events in list, even with type 'other'


MagListView::~MagListView() {
}


void MagListView::initTree() {
	_ui.treeWidget->clear();
	if ( _withOrigins )
		_unassociatedEventItem = addEvent(NULL);
	else
		_unassociatedEventItem = NULL;
}


void MagListView::clear() {
	initTree();
}


void MagListView::readFromDatabase() {
	if ( _reader == NULL ) return;

	// prevent updates from messaging during database read
	_readLock = true;

	initTree();
	SEISCOMP_WARNING("readFromDB");
	EventParameters ep;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	bool saveAutoSelect = _autoSelect;
	_autoSelect = false;
	_blockSelection = true;

	EventPtr event;
	size_t numberOfEvents = _reader->getObjectCount(&ep, Event::TypeInfo());
	size_t numberOfSteps = numberOfEvents*2;
	size_t currentStep = 0;

	QProgressDialog progress(this);
	progress.setWindowTitle(tr("Please wait..."));
	progress.setRange(0, numberOfSteps);

	DatabaseIterator it = _reader->getObjects(&ep, Event::TypeInfo());
	progress.setLabelText(tr("Reading events..."));
	while ( (event = static_cast<Event*>(*it)) != NULL ) {
		if ( progress.wasCanceled() )
			break;
		ep.add(event.get());
		++it;
		progress.setValue(++currentStep);
		qApp->processEvents();
	}
	it.close();

	_ui.treeWidget->setUpdatesEnabled (false);

	for ( size_t i = 0; i < ep.eventCount(); ++i ) {
		Event* event = ep.event(i);
		addEvent(event);
	}

	_ui.treeWidget->setUpdatesEnabled (true);

	QApplication::restoreOverrideCursor();
	_autoSelect = saveAutoSelect;
	_blockSelection = false;
	_readLock = false;
}


// void MagListView::readFromDatabase() {
// 	if ( _reader == NULL ) return;
// 
// 	initTree();
// SEISCOMP_WARNING("readFromDB");
// 	EventParameters ep;
// 
// 	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
// 	bool saveAutoSelect = _autoSelect;
// 	_autoSelect = false;
// 	_blockSelection = true;
// 
// 	EventPtr event;
// 	size_t numberOfEvents = _reader->getObjectCount(&ep, Event::TypeInfo());
// 	size_t numberOfOrigins = _reader->getObjectCount(&ep, Origin::TypeInfo());
// 	size_t numberOfSteps = numberOfEvents*2;
// 	size_t currentStep = 0;
// 
// 	if ( _withOrigins )
// 		numberOfSteps += numberOfEvents*10 + numberOfOrigins;
// 
// 	QProgressDialog progress(this);
// 	progress.setWindowTitle(tr("Please wait..."));
// 	progress.setRange(0, numberOfSteps);
// 
// 	DatabaseIterator it = _reader->getObjects(&ep, Event::TypeInfo());
// 	progress.setLabelText(tr("Reading events..."));
// 	while ( event = static_cast<Event*>(*it) ) {
// 		if ( progress.wasCanceled() )
// 			break;
// 		ep.add(event.get());
// 		++it;
// 		progress.setValue(++currentStep);
// 		qApp->processEvents();
// 	}
// 	it.close();
// 
// 	if ( _withOrigins ) {
// 		it = _reader->getObjects(&ep, Origin::TypeInfo());
// 		progress.setLabelText(tr("Reading origins..."));
// 		OriginPtr origin;
// 		while ( origin = static_cast<Origin*>(*it) ) {
// 			if ( progress.wasCanceled() )
// 				break;
// 	
// 			ep.add(origin.get());
// 			++it;
// 			progress.setValue(++currentStep);
// 			qApp->processEvents();
// 		}
// 		it.close();
// 	
// 		progress.setLabelText(tr("Reading origin references..."));
// 		for ( size_t i = 0; i < ep.eventCount(); ++i ) {
// 			if ( progress.wasCanceled() )
// 				break;
// 	
// 			_reader->loadOriginReferences(ep.event(i));
// 			currentStep += 10;
// 			progress.setValue(currentStep);
// 			qApp->processEvents();
// 		}
// 	}
// 
// 	/*
// 	progess.setLabelText(tr("Reading arrivals..."));
// 	for ( size_t i = 0; i < ep.originCount(); ++i ) {
// 		if ( progess.wasCanceled() )
// 			break;
// 
// 		ar.loadArrivals(ep.origin(i));
// 		progess.setValue(++currentStep);
// 		qApp->processEvents();
// 	}
// 	*/
// 
// 	QSet<void*> associatedOrigins;
// 
// 	_ui.treeWidget->setUpdatesEnabled (false);
// 
// 	progress.setLabelText(tr("Reading magnitudes..."));
// 
// 	for ( size_t i = 0; i < ep.eventCount(); ++i ) {
// 		Event* event = ep.event(i);
// 
// 		MagnitudePtr nm;
// 		if ( !progress.wasCanceled() ){
// 			LOOKUP(_reader, Magnitude, nm, event->preferredMagnitudeID());
// 		}
// 
// 		OriginPtr preferredOrigin;
// 		//! for displaying definingPhaseCount
// 		if ( !_withOrigins ) {
// 			if (!progress.wasCanceled()){
// 				LOOKUP(_reader, Origin, preferredOrigin, event->preferredOriginID());
// 			}
// 		}
// 		//! ---------------------------------
// 	
// 		QTreeWidgetItem* eventItem = addEvent(event);
// 
// 		OriginTreeItem* preferredOriginItem = NULL;
// 
// 		for ( size_t j = 0; j < event->originReferenceCount(); ++j ){
// 			OriginReference* ref = event->originReference(j);
// 			Origin* o = Origin::Cast(PublicObject::Find(ref->originID()));
// 
// 			if ( o ) {
// 				OriginTreeItem* item = (OriginTreeItem*)addOrigin(o, false, eventItem);
// 
// 				if ( o->publicID() == event->preferredOriginID() )
// 					preferredOriginItem = item;
// 
// 				associatedOrigins.insert(o);
// 					
// 				//! insert Magnitudes
// 				if ( !o->magnitudeCount() && _reader )
// 					_reader->loadMagnitudes(o);
// 				for ( size_t ii = 0; ii < o->magnitudeCount(); ++ii ) {
// 					if (o->magnitude(ii)->publicID() == event->preferredMagnitudeID())
// 						addNetMag(o->magnitude(ii), true, item);
// 					else
// 						addNetMag(o->magnitude(ii), false, item);
// 				}
// 
// 			}
// 		}
// 
// 		if ( preferredOriginItem )
// 			preferredOriginItem->setHighlight(true);
// 
// 		progress.setValue(++currentStep);
// 		qApp->processEvents();
// 	}
// 
// 	for ( size_t i = 0; i < ep.originCount(); ++i ) {
// 		if ( !associatedOrigins.contains(ep.origin(i)) )
// 			addOrigin(ep.origin(i), false);
// 	}
// 
// 	_ui.treeWidget->setUpdatesEnabled (true);
// 
// 	QApplication::restoreOverrideCursor();
// 	_autoSelect = saveAutoSelect;
// 	_blockSelection = false;
// }


void MagListView::setAutoSelect(bool s) {
	_autoSelect = s;
}


void MagListView::expandEventItem(QTreeWidgetItem* eventItem, int col){
	
	SchemeTreeItem* item = static_cast<SchemeTreeItem*>(eventItem);
	if (item->childCount())
		return;

	Event* event = Event::Cast(item->object());	

	if(!event->originReferenceCount() && _reader)
		_reader->loadOriginReferences(event);
	
	for ( size_t j = 0; j < event->originReferenceCount(); ++j ){
		OriginReference* ref = event->originReference(j);

		Origin* o = Origin::Cast(PublicObject::Find(ref->originID()));
		if(!o && _reader)
			o = Origin::Cast(_reader->getObject(Origin::TypeInfo(), ref->originID()));

		if (o) {
			if ( o->publicID() == event->preferredOriginID() )
				addOrigin(o, true, eventItem);
			else
				addOrigin(o, false, eventItem);
		}
	}
}


void MagListView::expandOriginItem(QTreeWidgetItem* originItem, int col){
	
	SchemeTreeItem* item = static_cast<SchemeTreeItem*>(originItem);
	if (item->childCount())
		return;

	SchemeTreeItem* parentItem = static_cast<SchemeTreeItem*>(originItem->parent());

	Origin* origin = Origin::Cast(item->object());	
	EventPtr event = Event::Cast(parentItem->object());
	if (!event) event = new Event("dummy");

	if(!origin->magnitudeCount() && _reader)
		_reader->loadMagnitudes(origin);

	for ( size_t ii = 0; ii < origin->magnitudeCount(); ++ii ) {
		if (origin->magnitude(ii)->publicID() == event->preferredMagnitudeID())
			addNetMag(origin->magnitude(ii), true, item);
		else
			addNetMag(origin->magnitude(ii), false, item);
	}
	

}


// add event to list
QTreeWidgetItem* MagListView::addEvent(Seiscomp::DataModel::Event* event) {
	if ( event != NULL ) {
		//! HACK display 'other' fake events in different style ---------
		std::string et;
		try{et = event->type().toString();}
		catch(...){}
		if (et == "other" && !_showAll){
			return NULL;
		}
		//! HACK ---------------------------------------------------------
	}

	QTreeWidgetItem* item = new EventTreeItem(event);
	_ui.treeWidget->insertTopLevelItem(0, item);
	_ui.btnClear->setEnabled(true);
	return item;
}


// add origin to list
QTreeWidgetItem* MagListView::addOrigin(Seiscomp::DataModel::Origin* origin, bool bold, QTreeWidgetItem* parent) {
	OriginTreeItem* item = new OriginTreeItem(origin);
	(parent?parent:_unassociatedEventItem)->insertChild(0,item);

	if ( bold )
		item->setHighlight(bold);

	_ui.btnClear->setEnabled(true);

	emit originAdded();
	return item;
}


// add netMag to list
QTreeWidgetItem* MagListView::addNetMag(Seiscomp::DataModel::Magnitude* netMag, bool bold, QTreeWidgetItem* parent) {
	NetMagTreeItem* item = new NetMagTreeItem(netMag);
	(parent?parent:_unassociatedEventItem)->insertChild(0,item);

	if ( bold )
		item->setHighlight(bold);

	_ui.btnClear->setEnabled(true);

	SEISCOMP_DEBUG("------> addNetMag");

	emit netMagAdded();
	return item;
}


// receive GUI command message e.g. from eventSummaryView
void MagListView::messageAvailable(Seiscomp::Core::Message* msg) {

	if (_readLock) {
		SEISCOMP_DEBUG("processing messages deferred");
		return;
	}

	CommandMessage* cmsg = CommandMessage::Cast(msg);

	if ( cmsg != NULL ) {
		if ( cmsg->command() == CM_SHOW_MAGNITUDE ) {
			
			std::cerr << " ~~~ ---GUImsg---> " << cmsg->parameter() << std::endl;

			QTreeWidgetItem* item = findNetMag(cmsg->parameter());
			
			if ( item ) {
				itemSelected(item, 0);
				return;
			}
		
			Magnitude* nm = Magnitude::Cast(PublicObject::Find(cmsg->parameter()));
			if (!nm && _reader)
				nm = Magnitude::Cast(_reader->getObject(Magnitude::TypeInfo(), cmsg->parameter()));
	
			if (nm) {
	
				_reader->load(nm);

				Event* e = NULL;
				Origin* o = NULL;
				
				// try to find parent objects: netMag->origin->event
				try{
					Origin* po = Origin::Cast(nm->parent());
					if (po){
						SEISCOMP_DEBUG("nm->parent(): %s", po->publicID().c_str());
						o = Origin::Cast(PublicObject::Find(po->publicID()));
							if (!o && _reader)
								o = Origin::Cast(_reader->getObject(Origin::TypeInfo(), po->publicID()));
					}
				} catch(...){
					SEISCOMP_DEBUG("nm->parent(): no parent object");
				}
	
				// parent origin			
				if (o){
					e = Event::Cast(PublicObject::Find( o->parent()->publicID() ));
						if (!e && _reader)
							e = Event::Cast(_reader->getObject(Event::TypeInfo(), o->parent()->publicID() ));
				}
				// parent event
				if (e){
					QTreeWidgetItem* eventItem = addEvent(e);
					addOrigin(o, false, eventItem);
				}

				//readPicks(o);
				//emit originSelected(o, NULL);
				QTreeWidgetItem* item;
				if(o)
					item = addNetMag(nm, false, findOrigin(o->publicID()));
				else
					item = addNetMag(nm, false, NULL);

				itemSelected(item, 0);
			}
		}
		return;
	}
}

// receive notifier message from *
void MagListView::notifierAvailable(Seiscomp::DataModel::Notifier* n) {
	
	if (_readLock) {
		SEISCOMP_DEBUG("processing messages deferred");
		return;
	}
	
	if ( _withOrigins ) {
		Origin* o = Origin::Cast(n->object());
		if ( o != NULL ) {
			switch ( n->operation() ) {
				case OP_ADD:
					{
						QTreeWidgetItem* item = addOrigin(o, false);
						if ( _autoSelect )
							//_ui.treeWidget->setItemSelected(item, true);
							itemSelected(item, 0);
					}
					break;
				case OP_UPDATE:
					{
						SchemeTreeItem* item = (SchemeTreeItem*)findOrigin(o->publicID());
						if ( item ) {
							item->update();
							emit originUpdated(static_cast<Origin*>(item->object()));
						}
					}
					break;
				default:
					break;
			}
			return;
		}
	}

	Event* e = Event::Cast(n->object());
	if ( e != NULL ) {
		switch ( n->operation() ) {
			case OP_ADD:
				addEvent(e);
				break;
			case OP_UPDATE:
				{
					EventTreeItem* item = (EventTreeItem*)findEvent(e->publicID());
					if ( item ) {
						Event* event = static_cast<Event*>(item->object());
						OriginTreeItem* originItem = (OriginTreeItem*)findOrigin(event->preferredOriginID());
						if ( originItem ) originItem->setHighlight(false);
						event->assign(e);
						originItem = (OriginTreeItem*)findOrigin(event->preferredOriginID());
						if ( originItem ) originItem->setHighlight(true);

						MagnitudePtr nm = Magnitude::Cast(_reader->getObject(Magnitude::TypeInfo(), event->preferredMagnitudeID()));
						item->update();
					}
				}
				break;
			default:
				break;
		}
		return;
	}

	OriginReference* ref = OriginReference::Cast(n->object());
	if ( ref != NULL ) {
		switch ( n->operation() ) {
			case OP_ADD:
				{
					SchemeTreeItem* eventItem = (SchemeTreeItem*)findEvent(n->parentID());
					if ( eventItem ) {
						SEISCOMP_INFO("found eventitem with publicID '%s', registered(%d)", eventItem->object()->publicID().c_str(), eventItem->object()->registered());
						SchemeTreeItem* originItem = (SchemeTreeItem*)findOrigin(ref->originID());
						if ( originItem && originItem->parent() ) {
							QTreeWidgetItem *taken = originItem->parent()->takeChild(originItem->parent()->indexOfChild(originItem));
							if ( taken ) {
								eventItem->addChild(taken);
								eventItem->update();
							}
						}
					}
				}
				break;
			default:
				break;
		};
		return;
	}
}

// return Item, if event found in list
QTreeWidgetItem* MagListView::findEvent(const std::string& publicID) {
	for ( int i = 0; i < _ui.treeWidget->topLevelItemCount(); ++i ) {
		SchemeTreeItem* item = (SchemeTreeItem*)_ui.treeWidget->topLevelItem(i);
		if ( item->object() && item->object()->publicID() == publicID ) {
			return item;
		}
	}

	return NULL;
}

// return Item, if origin found in list
QTreeWidgetItem* MagListView::findOrigin(const std::string& publicID) {
	for ( int i = 0; i < _ui.treeWidget->topLevelItemCount(); ++i ) {
		QTreeWidgetItem* item = _ui.treeWidget->topLevelItem(i);
		for ( int j = 0; j < item->childCount(); ++j ) {
			SchemeTreeItem* schemeItem = (SchemeTreeItem*)item->child(j);
			if ( schemeItem->object() && schemeItem->object()->publicID() == publicID ) {
				return schemeItem;
			}
		}
	}

	return NULL;
}

// return Item, if netMag found in list
QTreeWidgetItem* MagListView::findNetMag(const std::string& publicID) {
	for ( int i = 0; i < _ui.treeWidget->topLevelItemCount(); ++i ) {
		QTreeWidgetItem* item = _ui.treeWidget->topLevelItem(i);
		for ( int j = 0; j < item->childCount(); ++j ) {
			QTreeWidgetItem* item2 = (QTreeWidgetItem*)item->child(j);
			for ( int k = 0; k < item2->childCount(); ++k ) {
				SchemeTreeItem* item3 = (SchemeTreeItem*)item2->child(k);
				if ( item3->object() && item3->object()->publicID() == publicID ) {
					return item3;
				}
			}
		}
	}

	return NULL;
}


void MagListView::readPicks(Origin* o) {
	bool saveAutoSelect = _autoSelect;
	_autoSelect = false;

	_associatedPicks.clear();
	//_associatedStations.clear();

	if ( _reader ) {
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		if ( o->arrivalCount() == 0 )
			_reader->loadArrivals(o);

		if ( o->magnitudeCount() == 0 )
			_reader->loadMagnitudes(o);

		if ( o->stationMagnitudeCount() == 0 )
			_reader->loadStationMagnitudes(o);

		QProgressDialog progress(this);
		progress.setWindowTitle(tr("Please wait..."));
		progress.setRange(0, o->arrivalCount());
		progress.setLabelText(tr("Loading picks..."));
		progress.setCancelButton(NULL);

		for ( size_t i = 0; i < o->arrivalCount(); ++i ) {

			PickPtr pick = Pick::Cast(PublicObject::Find(o->arrival(i)->pickID()));
			if (!pick && _reader)			
				Pick::Cast(_reader->getObject(Pick::TypeInfo(), o->arrival(i)->pickID()));

			if ( pick )
				_associatedPicks.push_back(pick);

			progress.setValue(progress.value()+1);
			qApp->processEvents();
		}
		QApplication::restoreOverrideCursor();
	}

	_autoSelect = saveAutoSelect;
}


void MagListView::itemSelected(QTreeWidgetItem* item, int) {

	if ( _blockSelection ) return;
	_blockSelection = true;

	SchemeTreeItem* schemeItem = static_cast<SchemeTreeItem*>(item);

	// emit selected Magnitude plus parent Origin and parent Event
	Magnitude* nm = Magnitude::Cast(schemeItem->object());
	if (nm) {
		Origin* origin = NULL;
		Event* event = NULL;

		SchemeTreeItem* parentItem = (SchemeTreeItem*)schemeItem->parent();
		if (parentItem) 
			origin = Origin::Cast(parentItem->object());

		if (origin) {
			readPicks(origin);
			event = NULL;
			SchemeTreeItem* parentParentItem = (SchemeTreeItem*)parentItem->parent();
			if ( parentParentItem ) 
				event = Event::Cast(parentParentItem->object());	
		}

		emit netMagSelected(nm, origin, event);

		_blockSelection = false;
		return;
	}

	// emit selected Origin plus parent Event
	Origin* o = Origin::Cast(schemeItem->object());
	if ( o ) {
		Event* event = NULL;
		SchemeTreeItem* parentItem = (SchemeTreeItem*)schemeItem->parent();

		if ( parentItem ) 
			event = Event::Cast(parentItem->object());

		readPicks(o);

		emit originSelected(o, event);

		// insert netMags in listView
		expandOriginItem(schemeItem, 0);

		_blockSelection = false;
		return;
	}


	// emit selected Event plus preferredOrigin
	Event* e = Event::Cast(schemeItem->object());
	if (e) {
		Origin* o = Origin::Cast(PublicObject::Find(e->preferredOriginID()));
	
		if (o) {
			readPicks(o); 
			emit originSelected(o, e);
		}
		else
			emit eventSelected(e);

		// insert origins in listView
		expandEventItem(schemeItem, 0);

		_blockSelection = false;
		return;
	}
}


}
}
