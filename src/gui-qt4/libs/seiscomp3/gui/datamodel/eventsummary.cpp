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




#include <seiscomp3/gui/datamodel/eventsummary.h>

#include <seiscomp3/core/system.h>

#include <seiscomp3/datamodel/journalentry.h>
#include <seiscomp3/datamodel/momenttensor.h>

#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/datamodel/utils.h>
#include <seiscomp3/gui/datamodel/originsymbol.h>
#include <seiscomp3/gui/core/tensorrenderer.h>

#include <seiscomp3/seismology/regions.h>

using namespace Seiscomp::DataModel;

namespace Seiscomp {
namespace Gui {

namespace {

void setupFont(QWidget *w, const QFont &f) {
	w->setFont(f);
}

void setupFont(QWidget *w, const QFont &f, const QColor &c) {
	w->setFont(f);

	QPalette pal = w->palette();
	pal.setColor(QPalette::WindowText, c);
	w->setPalette(pal);
}

void setupColor(QWidget *w, const QColor &c) {
	QPalette pal = w->palette();
	pal.setColor(QPalette::WindowText, c);
	w->setPalette(pal);
}

void setText(QLabel *l, const QString &txt, int maxLength = 16) {
	/*
	if ( txt.length() > maxLength ) {
		l->setText(txt.mid(0, maxLength-3) + "...");
	}
	else*/
		l->setText(txt);
}


}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventSummaryMagnitudeRow::EventSummaryMagnitudeRow(const std::string &type,
                                                   QWidget *parent)
 : QHBoxLayout(parent) {

	label = new QLabel;
	label->setText(type.c_str());

	value = new QLabel;
	value->setText("-");

	label->installEventFilter(this);
	value->installEventFilter(this);

	addWidget(label);
	addWidget(value);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool EventSummaryMagnitudeRow::eventFilter(QObject *obj, QEvent *event) {
	if ( event->type() == QEvent::MouseButtonPress ) {
		QMouseEvent *me = static_cast<QMouseEvent*>(event);
		if ( me->button() == Qt::LeftButton && !magnitudeID.empty() ) {
			emit clicked(magnitudeID);
			return true;
		}
	}

	return QObject::eventFilter(obj, event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummaryMagnitudeRow::reset() {
	magnitudeID = "";

	label->setCursor(QCursor());
	value->setCursor(QCursor());

	value->setText("-");

	QFont f = label->font();
	f.setBold(false);
	label->setFont(f);

	f = value->font();
	f.setBold(false);
	value->setFont(f);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummaryMagnitudeRow::set(const std::string &id, double mag, int stationCount) {
	if ( magnitudeID.empty() ) {
		magnitudeID = id;
		label->setCursor(Qt::PointingHandCursor);
		value->setCursor(Qt::PointingHandCursor);
	}
	else if ( magnitudeID != id )
		return;

	setMagnitude(mag, stationCount);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummaryMagnitudeRow::setMagnitude(double mag, int stationCount) {
	char buf[32];
	if ( stationCount > 0 ) {
		sprintf(buf, "%.1f (%d)", mag, stationCount);
		value->setText(buf);
	}
	else {
		sprintf(buf, "%.1f", mag);
		value->setText(buf);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummaryMagnitudeRow::select(bool selected) {
	QFont f = label->font();
	f.setBold(selected);
	label->setFont(f);

	f = value->font();
	f.setBold(selected);
	value->setFont(f);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventSummary::EventSummary(const MapsDesc &maps,
                           DatabaseQuery* reader,
                           QWidget *parent) : QWidget(parent) {
	_reader = reader;
	_maptree = new Map::ImageTree(maps);
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventSummary::EventSummary(Map::ImageTree* mapTree,
                           DatabaseQuery* reader,
                           QWidget * parent) : QWidget(parent) {
	_reader = reader;
	_maptree = mapTree;
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventSummary::~EventSummary() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class EventSummaryMap : public MapWidget {
	public:
		EventSummaryMap(EventSummary *owner, Map::ImageTree* mapTree, QWidget *parent = 0, Qt::WFlags f = 0)
		 : MapWidget(mapTree, parent, f) {
			_owner = owner;
			QSizePolicy sp = sizePolicy();
			sp.setHeightForWidth(true);
			setSizePolicy(sp);
		}

		void mousePressEvent(QMouseEvent *event) {
			if ( event->button() == Qt::LeftButton ) {
				_owner->mapClicked();
			}
		}

	private:
		EventSummary *_owner;
};


void EventSummary::init() {
	_ui.setupUi(this);

	_showComment = true;
	_defaultEventRadius = 0.0;
	_maxMinutesSecondDisplay = -1;

	_ui.labelOpComment->setVisible(false);
	_ui.labelOpCommentSeparator->setVisible(false);

	_symbol = NULL;
	_map = new EventSummaryMap(this, _maptree.get(), _ui.map);
	QHBoxLayout* hboxLayout = new QHBoxLayout(_ui.map);
	hboxLayout->setMargin(0);
	hboxLayout->addWidget(_map);

	QObject *drawFilter = new ElideFadeDrawer(this);
	//QObject *drawFilter = new EllipsisDrawer(this);

	const QFont &locationFont = SCScheme.fonts.base;
	QFont locationFontBold = locationFont;
	locationFontBold.setBold(true);

	connect(&_timeAgo, SIGNAL(timeout()), this, SLOT(updateTimeAgo()));

	// Set the font sizes
	setupFont(_ui.originTime, SCScheme.fonts.highlight);
	_ui.originTime->setMinimumWidth(_ui.originTime->fontMetrics().width("9999-99-99 99:99:99 "));

	setupFont(_ui.timeAgo, SCScheme.fonts.base);

	//QColor highlightColor = Qt::red;

	setupFont(_ui.magnitudeText, SCScheme.fonts.highlight/*, highlightColor*/);
	setupFont(_ui.magnitude, SCScheme.fonts.highlight/*, highlightColor*/);
	setupFont(_ui.depthText, SCScheme.fonts.highlight/*, highlightColor*/);
	setupFont(_ui.depth, SCScheme.fonts.highlight/*, highlightColor*/);
	setupFont(_ui.region, SCScheme.fonts.highlight/*, highlightColor*/);
	_ui.region->installEventFilter(drawFilter);
	_ui.timeAgo->installEventFilter(drawFilter);

	setupFont(_ui.nearestCity, SCScheme.fonts.highlight);
	_ui.nearestCity->installEventFilter(drawFilter);
	_ui.nearestCity->setVisible(false);

	//setupFont(_ui.latitudeText, locationFont);
	setupFont(_ui.latitude, locationFontBold);
	//setupFont(_ui.longitudeText, locationFont);
	setupFont(_ui.longitude, locationFontBold);

	setupFont(_ui.phaseCountText, locationFont);
	setupFont(_ui.phaseCount, locationFontBold);

	setupFont(_ui.azimuthalGapText, locationFont);
	setupFont(_ui.azimuthalGap, locationFont);

	setupFont(_ui.rmsText, locationFont);
	setupFont(_ui.rms, locationFont);

	setupFont(_ui.minimumDistanceText, locationFont);
	setupFont(_ui.minimumDistance, locationFont);

	setupFont(_ui.maximumDistanceText, locationFont);
	setupFont(_ui.maximumDistance, locationFont);

	setupFont(_ui.firstLocationText, SCScheme.fonts.base);
	setupFont(_ui.firstLocation, SCScheme.fonts.base);

	setupFont(_ui.thisLocationText, SCScheme.fonts.base);
//	setupFont(_ui.thisLocation, SCScheme.fonts.base);

	setupFont(_ui.eventIDText, SCScheme.fonts.base);
	setupFont(_ui.eventID, SCScheme.fonts.base);
	_ui.eventID->installEventFilter(drawFilter);

	setupFont(_ui.originIDText, SCScheme.fonts.base);
	setupFont(_ui.originID, SCScheme.fonts.base);
	_ui.originID->installEventFilter(drawFilter);

	setupFont(_ui.agencyID, SCScheme.fonts.base);
	_ui.agencyID->installEventFilter(drawFilter);

	setupFont(_ui.state, SCScheme.fonts.base);
	setupFont(_ui.mode, SCScheme.fonts.base);

	_magnitudeRows = new QVBoxLayout(_ui.magnitudes);
	_magnitudeRows->setMargin(0);
	_magnitudeRows->setSpacing(layout()->spacing());

	_ui.exportButton->setVisible(false);

	_ui.focalMechanism->setVisible(false);

	try {
		std::vector<std::string> mags = SCApp->configGetStrings("visibleMagnitudes");
		for ( size_t i = 0; i < mags.size(); ++i )
			if ( mags[i] != "*" ) addVisibleMagnitudeType(mags[i]);
	}
	catch ( ... ) {
		addVisibleMagnitudeType("M");
		addVisibleMagnitudeType("MLv");
		addVisibleMagnitudeType("mb");
		addVisibleMagnitudeType("mB");
		addVisibleMagnitudeType("Mw(mB)");
	}

	try { _maxMinutesSecondDisplay = SCApp->configGetInt("displayAgoSecondsUpToMaximumMinutes"); }
	catch ( ... ) {}

	try { _showComment = SCApp->configGetBool("eventsummary.showComment"); }
	catch ( ... ) {}

	_ui.azimuthalGapText->setVisible(false); _ui.azimuthalGap->setVisible(false);
	_ui.firstLocationText->setVisible(false); _ui.firstLocation->setVisible(false);
	_ui.thisLocationText->setVisible(false);
	_ui.minimumDistanceText->setVisible(false); _ui.minimumDistance->setVisible(false);
	_ui.maximumDistanceText->setVisible(false); _ui.maximumDistance->setVisible(false);
	_ui.originIDText->setVisible(false); _ui.originID->setVisible(false);

	resetContent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::setTextContrast(bool f) {
	QColor c = f?
	           palette().color(QPalette::Disabled, QPalette::WindowText):
	           palette().color(QPalette::Normal, QPalette::WindowText);

	setupColor(_ui.depth, c);
	setupColor(_ui.region, c);
	setupColor(_ui.nearestCity, c);

	setupColor(_ui.latitude, c);
	setupColor(_ui.longitude, c);

	setupColor(_ui.phaseCount, c);
	setupColor(_ui.azimuthalGap, c);

	setupColor(_ui.rms, c);

	setupColor(_ui.minimumDistance, c);
	setupColor(_ui.maximumDistance, c);

	setupColor(_ui.thisLocationText, c);

	setupColor(_ui.originID, c);
	setupColor(_ui.agencyID, c);

	setupColor(_ui.state, c);
	setupColor(_ui.mode, c);

	MagnitudeList::iterator it = _magnitudes.begin();
	for ( ; it != _magnitudes.end(); ++it )
		setupColor(it->second->value, c);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPushButton *EventSummary::exportButton() const {
	return _ui.exportButton;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MapWidget *EventSummary::mapWidget() const {
	return _map;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::updateTimeAgo() {
	if ( !_currentOrigin )
		return;

	Core::TimeSpan ts;
	Core::Time ct = Core::Time::GMT();

	ts = ct - _currentOrigin->time();
	QString ago;

	int sec = ts.seconds();
	if ( sec < 0 ) {
		sec = -sec;
		ago = "in future";
	}
	else
		ago = "ago";

	int days = sec / 86400;
	int hours = (sec - days*86400) / 3600;
	int minutes = (sec - days*86400 - hours*3600) / 60;
	int seconds = sec - days*86400 - hours*3600 - 60*minutes;

	QString text;

	if ( days > 0 )
		text = QString("%1d and %2h %3").arg(days, 0, 'd', 0, ' ').arg(hours, 0, 'd', 0, ' ').arg(ago);
	else if ( ( days == 0 ) && ( hours > 0 ) )
		text = QString("%1h and %2m %3").arg(hours, 0, 'd', 0, ' ').arg(minutes, 0, 'd', 0, ' ').arg(ago);
	else if ( ( days == 0 ) && ( hours == 0 ) && ( minutes > 0 ) ) {
		if ( _maxMinutesSecondDisplay >= 0 && minutes > _maxMinutesSecondDisplay )
			text = QString("%1m %3").arg(minutes, 0, 'd', 0, ' ').arg(ago);
		else
			text = QString("%1m and %2s %3").arg(minutes, 0, 'd', 0, ' ').arg(seconds, 0, 'd', 0, ' ').arg(ago);
	}
	else if ( ( days == 0 ) && ( hours == 0 ) && ( minutes == 0 ) && ( seconds > 0 ) )
		text = QString("%1s %3").arg(seconds, 0, 'd', 0, ' ').arg(ago);

	if ( text != _ui.timeAgo->text() )
		_ui.timeAgo->setText(text);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::mapClicked() {
	if ( _currentOrigin )
		emit selected(_currentOrigin.get(), _currentEvent.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::magnitudeClicked(const std::string &magnitudeID) {
	if ( _currentOrigin ) {
		emit selected(_currentOrigin.get(), _currentEvent.get());
		emit magnitudeSelected(magnitudeID);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::addObject(const QString& parentID,
                             Object* obj) {
	Magnitude *mag = Magnitude::Cast(obj);
	if ( mag ) {
		if ( _currentOrigin && parentID.toStdString() == _currentOrigin->publicID() )
			setMagnitude(mag);
		return;
	}

	Comment *comment = Comment::Cast(obj);
	if ( comment ) {
		if ( _currentEvent && parentID == _currentEvent->publicID().c_str() &&
		     comment->id() == "Operator" )
			updateOrigin(_currentOrigin.get());
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::updateObject(const QString& parentID,
                                Object* obj) {
	Event *evt = Event::Cast(obj);
	if ( evt ) {
		if ( !_fixedView ) {
			if ( _currentEvent && evt->publicID() == _currentEvent->publicID() ) {
				setEvent(evt);
				return;
			}
		}
		else {
			setTextContrast(!_currentOrigin || evt->preferredOriginID() != _currentOrigin->publicID());
		}
	}

	Comment *comment = Comment::Cast(obj);
	if ( comment ) {
		if ( _currentEvent && parentID == _currentEvent->publicID().c_str() &&
		     comment->id() == "Operator" )
			updateOrigin(_currentOrigin.get());
		return;
	}

	updateOrigin(Origin::Cast(obj));

	Magnitude *mag = Magnitude::Cast(obj);
	if ( mag && _currentOrigin &&
	     parentID == _currentOrigin->publicID().c_str() ) {
		setMagnitude(mag);

		if ( _currentMag && mag->publicID() == _currentMag->publicID() )
			updateMagnitude();

		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::updateOrigin(Origin* org) {
	if ( org && _currentOrigin &&
	     org->publicID() == _currentOrigin->publicID() ) {
		updateOrigin();

		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::removeObject(const QString& parentID, Object* obj) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::setEvent(Event *event, Origin *org, bool fixed) {
	Event *cachedEvent = event?Event::Find(event->publicID()):NULL;
	if ( cachedEvent ) event = cachedEvent;

	_currentEvent = event;
	_currentMag = NULL;
	_fixedView = fixed;

	if ( _currentEvent ) {
		// Focal mechanism
		_currentFocalMechanism = FocalMechanism::Find(_currentEvent->preferredFocalMechanismID());
		if ( !_currentFocalMechanism && _reader )
			_currentFocalMechanism = FocalMechanism::Cast(_reader->getObject(FocalMechanism::TypeInfo(), 
			                                              _currentEvent->preferredFocalMechanismID()));
		if ( _currentFocalMechanism && (_currentFocalMechanism->momentTensorCount() == 0) && _reader) {
			_reader->loadMomentTensors(_currentFocalMechanism.get());
		}
	}
	else
		_currentFocalMechanism = NULL;

	if ( org )
		setOrigin(org);
	else
		setOrigin(_currentEvent?_currentEvent->preferredOriginID():"");

	// Displayed origin not the preferred: change colors
	if ( _currentOrigin && _currentEvent &&
	     _currentOrigin->publicID() != _currentEvent->preferredOriginID() )
		setTextContrast(true);
	else
		setTextContrast(false);

	if ( _currentEvent ) {
		if ( !_currentMag ) {
			_currentMag = Magnitude::Find(_currentEvent->preferredMagnitudeID());
			if ( !_currentMag && _reader )
				_currentMag = Magnitude::Cast(_reader->getObject(Magnitude::TypeInfo(), 
				                                         _currentEvent->preferredMagnitudeID()));
		}
	}
	else
		_currentMag = NULL;

	// Magnitude
	if ( _currentMag ) {
		updateMagnitude();
		selectMagnitude(_currentMag->type());
	}
	else {
		_ui.magnitude->setText("-");
		if ( _symbol ) {
			_symbol->setPreferredMagnitudeValue(0);
			if ( _map ) _map->update();
		}
		selectMagnitude("");
	}

}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::updateMagnitude() {
	char buf[32];
	sprintf(buf, "%.1f", _currentMag->magnitude().value());
	setText(_ui.magnitude, buf);

	if ( _symbol ) {
		_symbol->setPreferredMagnitudeValue(_currentMag->magnitude().value());
		if ( _map ) _map->update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::showOrigin(Origin *org) {
	_currentEvent = NULL;
	_currentFocalMechanism = NULL;
	setOrigin(org);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::setOrigin(Origin* org) {
	Origin *cachedOrigin = org?Origin::Find(org->publicID()):NULL;
	if ( cachedOrigin ) org = cachedOrigin;

	_currentOrigin = org;
	updateContent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::setOrigin(const std::string &originID) {
	//if ( _currentOrigin && _currentOrigin->publicID() == originID )
	//	return;

	_currentOrigin = Origin::Find(originID);
	if ( !_currentOrigin && _reader )
		_currentOrigin = Origin::Cast(_reader->getObject(Origin::TypeInfo(), originID));

	updateContent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::setMagnitude(const Magnitude *mag) {
	MagnitudeList::iterator it = _magnitudes.find(mag->type());
	if ( it == _magnitudes.end() ) return;

	int stationCount = 0;
	try {
		stationCount = mag->stationCount();
	}
	catch ( Core::ValueException& ) {}

	it->second->set(mag->publicID(), mag->magnitude().value(), stationCount);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::selectMagnitude(const std::string &type) {
	MagnitudeList::iterator it;

	for ( it = _magnitudes.begin(); it != _magnitudes.end(); ++it )
		it->second->select(it->first == type);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::updateOrigin() {
	// Origin time
	timeToLabel(_ui.originTime, _currentOrigin->time().value(), "%F %T");

	// Depth
	try {
		setText(_ui.depth, depthToString(_currentOrigin->depth().value(), SCScheme.precision.depth) + " km");
	}
	catch ( Core::ValueException& ) {
		_ui.depth->setText("-");
	}

	// Region information
	std::string region = _currentEvent?eventRegion(_currentEvent.get()):"";
	if ( _currentEvent && !region.empty() )
		setText(_ui.region, region.c_str());
	else {
		try {
			Regions regions;
			setText(_ui.region, regions.getRegionName(_currentOrigin->latitude(), _currentOrigin->longitude()).c_str());
		}
		catch ( Core::ValueException& ) {
			_ui.region->setText("-");
		}
	}

	// Operators comment
	_ui.labelOpComment->setVisible(false);
	_ui.labelOpCommentSeparator->setVisible(false);
	if ( _currentEvent && _showComment ) {
		for ( size_t i = 0; i < _currentEvent->commentCount(); ++i ) {
			if ( _currentEvent->comment(i)->id() == "Operator" ) {
				if ( !_currentEvent->comment(i)->text().empty() ) {
					_ui.labelOpComment->setVisible(true);
					_ui.labelOpCommentSeparator->setVisible(true);
					_ui.labelOpComment->setText(_currentEvent->comment(i)->text().c_str());
				}
				break;
			}
		}
	}

	// Origin information
	try { setText(_ui.latitude, latitudeToString(_currentOrigin->latitude(), true, true, SCScheme.precision.location)); }
	catch ( Core::ValueException& ) { _ui.latitude->setText("-"); }

	try { setText(_ui.longitude, longitudeToString(_currentOrigin->longitude(), true, true, SCScheme.precision.location)); }
	catch ( Core::ValueException& ) { _ui.longitude->setText("-"); }

	try { setText(_ui.phaseCount, QString("%1").arg(int(_currentOrigin->quality().usedPhaseCount()))); }
	catch ( Core::ValueException& ) { _ui.phaseCount->setText("-"); }

	try { setText(_ui.rms, QString("%1").arg(_currentOrigin->quality().standardError(), 0, 'f', 1)); }
	catch ( Core::ValueException& ) { _ui.rms->setText("-"); }

	try { setText(_ui.azimuthalGap, QString("%1%2").arg(_currentOrigin->quality().azimuthalGap(), 0, 'f', 1).arg(degrees)); }
	catch ( Core::ValueException& ) { _ui.azimuthalGap->setText("-"); }

	try { setText(_ui.minimumDistance, QString("%1%2").arg(_currentOrigin->quality().minimumDistance(), 0, 'f', 1).arg(degrees)); }
	catch ( Core::ValueException& ) { _ui.minimumDistance->setText("-"); }

	try { setText(_ui.maximumDistance, QString("%1%2").arg(_currentOrigin->quality().maximumDistance(), 0, 'f', 1).arg(degrees)); }
	catch ( Core::ValueException& ) { _ui.maximumDistance->setText("-"); }

	if ( _currentOrigin ) {
		setText(_ui.originID, _currentOrigin->publicID().c_str());
		try { setText(_ui.agencyID, _currentOrigin->creationInfo().agencyID().c_str()); }
		catch ( Core::ValueException& ) { _ui.agencyID->setText(""); }
		try { setText(_ui.state, _currentOrigin->evaluationStatus().toString()); }
		catch ( Core::ValueException& ) { _ui.state->setText("-"); }
		try { setText(_ui.mode, _currentOrigin->evaluationMode().toString()); }
		catch ( Core::ValueException& ) { _ui.mode->setText("-"); }
	}
	else {
		_ui.originID->setText("-");
		_ui.agencyID->setText("-");
	}

	// get the time of first location of an origin belonging to this Event
	QString str("-");
	if ( _currentEvent && _currentOrigin ) {
		try {
			Core::TimeSpan dt = _currentEvent->creationInfo().creationTime() - _currentOrigin->time().value();
			str = elapsedTimeString(dt);
		}
		catch ( Core::ValueException& ) {}
	}
	setText(_ui.firstLocation, str);

	str = "-";
	// get the time of the current location
	if ( _currentOrigin ) {
		try {
			Core::TimeSpan dt = _currentOrigin->creationInfo().creationTime() - _currentOrigin->time().value();
			str = elapsedTimeString(dt);
		}
		catch (...) {}
	}
//	setText(_ui.thisLocation, str);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void EventSummary::setFocalMechanism(FocalMechanism* fm) {
	if ( fm == NULL ) return;

	MomentTensorPtr mt = fm->momentTensorCount() > 0?fm->momentTensor(0):NULL;
	OriginPtr o = Origin::Find(mt?mt->derivedOriginID():fm->triggeringOriginID());

	if ( o == NULL && mt && _reader )
		o = Origin::Cast(_reader->getObject(Origin::TypeInfo(),
		                                    mt->derivedOriginID()));

	QString toolTip;
	QColor c = Qt::black;

	if ( o ) {
		try {
			OriginType type = o->type();
			toolTip  = QString("Type: %1\n").arg(type.toString());
		} catch ( ... ) {
			toolTip = "Type: -\n";
		}

		try {
			toolTip = "Epicenter: "  + latitudeToString(o->latitude(), true, false, SCScheme.precision.location)
			              + " " + latitudeToString(o->latitude(), false, true)
			              + " " + longitudeToString(o->longitude(), true, false, SCScheme.precision.location)
			              + " " + longitudeToString(o->longitude(), false, true) + "\n";
		} catch ( ... ) {
			toolTip = "Epicenter: n/a";
	
		}

		OPT(float) depth;
		try {
			depth = o->depth();
			QString text = depthToString(*depth, SCScheme.precision.depth) + " km";
			_ui.fmDepth->setText(text);
			toolTip += "Depth: " + text + "\n";
		}
		catch(...) {
			_ui.fmDepth->setText(QString("-"));
			toolTip += "Depth: n/a\n";
		}

		if ( depth ) {
			if ( *depth < 50 )
				c = Qt::red;
			else if ( *depth < 100 )
				c = QColor(255, 165, 0);
			else if ( *depth < 250 )
				c = Qt::yellow;
			else if ( *depth < 600 )
				c = Qt::green;
			else
				c = Qt::blue;
		}
	} else {
		toolTip = "Type: -\n Epicenter -\nDepth: -\n";
	}

	MagnitudePtr mag = mt?Magnitude::Find(mt->momentMagnitudeID()):NULL;
	if ( !mag && mt && _reader)
		mag = Magnitude::Cast(_reader->getObject(Magnitude::TypeInfo(), mt->momentMagnitudeID()));

	if ( mag ) {
		QString text = QString("%1").arg(mag->magnitude().value(), 0, 'f', 1);
		_ui.mw->setText(QString("Mw %1").arg(text));
		toolTip += "Mw: " + text + "\n";
	}
	else {
		_ui.mw->setText("-");
		toolTip += "Mw: n/a\n";
	}

	OPT(double) strike, dip, rake;

	try {
		strike = fm->nodalPlanes().nodalPlane1().strike().value(),
		dip = fm->nodalPlanes().nodalPlane1().dip().value(), 
		rake = fm->nodalPlanes().nodalPlane1().rake().value();
		toolTip += QString("NP1 S,D,R: %1, %2, %3\n")
		               .arg(*strike, 0, 'f', 0)
		               .arg(*dip, 0, 'f', 0)
		               .arg(*rake, 0, 'f', 0);
		toolTip += QString("NP2 S,D,R: %1, %2, %3\n")
		               .arg(fm->nodalPlanes().nodalPlane2().strike().value(), 0, 'f', 0)
		               .arg(fm->nodalPlanes().nodalPlane2().dip().value(), 0, 'f', 0)
		               .arg(fm->nodalPlanes().nodalPlane2().rake().value(), 0, 'f', 0);
		toolTip += QString("Author ID: %1\n").arg(objectAuthor(fm).c_str());
		toolTip += QString("Status: %1").arg(objectStatusToChar(fm));
	} catch ( ... ) {
		toolTip += QString("no valid focal mechanism\n");
	}

	if ( strike && dip && rake ) {
		const QSize& size = _ui.momentTensor->size();
		QImage img(size, QImage::Format_ARGB32);

		Gui::TensorRenderer renderer;
		renderer.setTColor(c);
		renderer.setShadingEnabled(true);
		renderer.render(img, *strike, *dip, *rake);
		_ui.momentTensor->setPixmap(QPixmap::fromImage(img));
	}

	_ui.focalMechanism->setToolTip(toolTip);
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

void EventSummary::updateContent() {
	if ( !_currentOrigin ) {
		resetContent();
		return;
	}

	if ( _map ) {
		if ( _symbol ) _map->canvas().symbolCollection()->remove(_symbol);

		_symbol = new OriginSymbol(_currentOrigin->latitude(), _currentOrigin->longitude());
		try { _symbol->setDepth(_currentOrigin->depth()); } catch ( Core::ValueException& ) {}

		_map->canvas().symbolCollection()->add(_symbol);

		double radius;
		if ( _defaultEventRadius > 0 )
			radius = _defaultEventRadius;
		else {
			radius = 12.5;
			try { radius = std::min(radius, _currentOrigin->quality().maximumDistance()+0.1); }
			catch ( ... ) {}
		}

		_map->canvas().displayRect(QRectF(_currentOrigin->longitude()-radius,
		                                  _currentOrigin->latitude()-radius, radius*2, radius*2));
		_map->setCursor(Qt::PointingHandCursor);
	}

	_ui.exportButton->setEnabled(true);
	_ui.exportButton->setCursor(Qt::PointingHandCursor);

	updateOrigin();

	if ( _currentEvent ) {
		setText(_ui.eventID, _currentEvent->publicID().c_str());
		_ui.eventID->setToolTip(_currentEvent->publicID().c_str());
	}
	else {
		_ui.eventID->setText("-");
		_ui.eventID->setToolTip("");
	}

	resetMagnitudes();

	if ( _currentOrigin ) {
		if ( (_currentOrigin->magnitudeCount() == 0) && _reader ) {
			_reader->loadMagnitudes(_currentOrigin.get());
		}

		for ( size_t i = 0; i < _currentOrigin->magnitudeCount(); ++i )
			setMagnitude(_currentOrigin->magnitude(i));
	}

	if ( _currentEvent ) {
		MagnitudePtr mag = Magnitude::Find(_currentEvent->preferredMagnitudeID());
		if ( !mag && _reader )
			mag = Magnitude::Cast(_reader->getObject(Magnitude::TypeInfo(),
			                                         _currentEvent->preferredMagnitudeID()));

		if ( mag )
			setMagnitude(mag.get());
	}

	if ( _currentFocalMechanism ) {
		setFocalMechanism(_currentFocalMechanism.get());
		_ui.focalMechanism->setVisible(true);
	}
	else
		_ui.focalMechanism->setVisible(false);

	_timeAgo.start(1000);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::resetMagnitudes() {
	for ( MagnitudeList::iterator it = _magnitudes.begin();
	      it != _magnitudes.end(); ++it )
		it->second->reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::resetContent() {
	if ( _map ) _map->setCursor(QCursor());

	_ui.exportButton->setEnabled(false);
	_ui.exportButton->setCursor(QCursor());

	_timeAgo.stop();

	if ( _map ) {
		if ( _symbol ) {
			_map->canvas().symbolCollection()->remove(_symbol);
			_symbol = NULL;
		}
	}

	_ui.originTime->setText("1970/01/01 - 00:00:00");
	_ui.timeAgo->setText("");
	_ui.magnitude->setText("-");
	_ui.depth->setText("-");
	_ui.region->setText("...");
	_ui.nearestCity->setText("");
	_ui.latitude->setText("-");
	_ui.longitude->setText("-");
	_ui.phaseCount->setText("-");
	_ui.azimuthalGap->setText("-");
	_ui.rms->setText("-");
	_ui.minimumDistance->setText("-");
	_ui.maximumDistance->setText("-");
	_ui.firstLocation->setText("-");
//	_ui.thisLocation->setText("-");
	_ui.eventID->setText("-");
	_ui.originID->setText("-");
	_ui.agencyID->setText("-");
	_ui.state->setText("-");
	_ui.mode->setText("-");

	_ui.mw->setText("-");
	_ui.fmDepth->setText("-");
	_ui.momentTensor->setPixmap(QPixmap());
	_ui.momentTensor->setToolTip("");
	if ( _map ) {
		_map->canvas().displayRect(_map->canvas().geoRect());
		_map->update();
	}

	resetMagnitudes();

	_ui.focalMechanism->setVisible(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Event *EventSummary::currentEvent() const {
	return _currentEvent.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Origin *EventSummary::currentOrigin() const {
	return _currentOrigin.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::addVisibleMagnitudeType(const std::string &mag) {
	if ( mag.empty() ) return;

	std::pair<MagnitudeTypes::iterator, bool> itp;
	itp = _visibleMagnitudes.insert(mag);
	// Already inserted
	if ( !itp.second ) return;

	EventSummaryMagnitudeRow* row = new EventSummaryMagnitudeRow(mag);
	_magnitudeRows->addLayout(row);

	connect(row, SIGNAL(clicked(const std::string &)),
	        this, SLOT(magnitudeClicked(const std::string &)));

	_magnitudes[mag] = row;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::setDefaultEventRadius(double radius) {
	_defaultEventRadius = radius;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EventSummary::setSecondDisplayUpToMaxMinutes(int v) {
	_maxMinutesSecondDisplay = v;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QList<QFrame*> EventSummary::separators() const {
	return QList<QFrame*>() << _ui.separator1 << _ui.separator2;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
