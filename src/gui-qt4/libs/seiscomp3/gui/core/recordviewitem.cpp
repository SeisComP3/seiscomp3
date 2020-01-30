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



#define SEISCOMP_COMPONENT Gui::RecordViewItem

#include <seiscomp3/gui/core/recordviewitem.h>
#include <seiscomp3/gui/core/recordview.h>
#include <seiscomp3/logging/log.h>

#include <QApplication>
#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <QWidget>

namespace {

QString waveformIDToString(const Seiscomp::DataModel::WaveformStreamID& id) {
	return (id.networkCode() + "." + id.stationCode() + "." +
	        id.locationCode() + "." + id.channelCode()).c_str();
}


char component(const Seiscomp::Record *rec) {
	size_t len = rec->channelCode().size();
	if ( len >= 3 )
		return rec->channelCode()[len-1];

	return 'Z';
}


char component(const Seiscomp::RecordSequence *records) {
	if ( records->empty() ) return '?';
	return component(records->front().get());
}


}


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordLabel::RecordLabel(QWidget *parent, const char *)
	: QWidget(parent)
{
	_parent = NULL;
	_enabled = true;
	_interactive = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordLabel::~RecordLabel()
{
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem* RecordLabel::recordViewItem() const {
	return _parent;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordLabel::setInteractive(bool interactive) {
	_interactive = interactive;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordLabel::isInteractive() const {
	return _interactive;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordLabel::setEnabled(bool enabled) {
	if ( _enabled == enabled ) return;

	_enabled = enabled;
	// NO!  QFrame::setEnabled(_enabled); // don't do that
	emit statusChanged(_enabled);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordLabel::mouseDoubleClickEvent(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton && e->modifiers() == Qt::NoModifier ) {
		if ( _interactive )
			setEnabled(!isEnabled());
		else {
			emit doubleClicked();
			e->ignore();
		}
	}
	else
		e->ignore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordLabel::setBackgroundColor(const QColor &c) {
	QPalette p = palette();
	p.setColor(QPalette::Window, c);
	setPalette(p);
	setAutoFillBackground(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardRecordLabel::StandardRecordLabel(int items, QWidget *parent, const char*)
 : Seiscomp::Gui::RecordLabel(parent) {
	//setFrameStyle(QFrame::NoFrame);

	_items.resize(items);

	for ( int i = 0; i < _items.count(); ++i ) {
		_items[i].color = palette().color(QPalette::Text);
		_items[i].font = QWidget::font();
		_items[i].width = -1;
		_items[i].align = Qt::AlignLeft | Qt::AlignVCenter;
		_items[i].editable = false;
	}

	_vertical = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StandardRecordLabel::~StandardRecordLabel() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardRecordLabel::setText(const QString &str, int item) {
	_items[item].text = str;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardRecordLabel::setColor(QColor col, int item) {
	_items[item].color = col;
	_items[item].colorSet = true;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardRecordLabel::setFont(const QFont& f, int item) {
	_items[item].font = f;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardRecordLabel::setWidth(int width, int item) {
	_items[item].width = width;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardRecordLabel::setAlignment(Qt::Alignment al, int item) {
	_items[item].align = al;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardRecordLabel::setEditable(bool e, int item) {
	_items[item].editable = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString StandardRecordLabel::text(int item) const {
	return _items[item].text;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QFont& StandardRecordLabel::font(int item) const {
	return _items[item].font;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardRecordLabel::setOrientation(Qt::Orientation o) {
	_vertical = o == Qt::Vertical;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StandardRecordLabel::itemCount() const {
	return _items.count();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StandardRecordLabel::paintEvent(QPaintEvent *e) {
	//Seiscomp::Gui::RecordLabel::paintEvent(e);

	QPainter painter(this);

	int lw = 0,//lineWidth(),
	lw2 = 2*lw,
	w   =  width() - lw2,
	h   = height() - lw2;

	if ( _vertical ) {
		int lines = 0;
		for (int i=0; i<_items.count(); i++)
			if ( !_items[i].text.isEmpty() )
				++lines;

		if ( !lines ) return;

		int fontSize = std::min(h / lines, painter.fontMetrics().ascent());
		//font.setPixelSize(fontSize);
		//painter.setFont(font);

		int spacing = std::min(4, (h - fontSize*lines) / (lines+1));

		int posY = (h - spacing*(lines+1) - fontSize*lines)/2;
		for (int i=0; i<_items.count(); i++) {
			if ( _items[i].text.isEmpty() ) continue;
			QFont f(_items[i].font);
			f.setPixelSize(fontSize);
			painter.setFont(f);
			painter.setPen (isEnabled() ? (_items[i].colorSet?_items[i].color:palette().color(QPalette::Text)) : palette().color(QPalette::Disabled, QPalette::Text));
			painter.drawText (0,posY, w, fontSize, _items[i].align, _items[i].text);
			posY += fontSize + spacing;
		}
	}
	else {
		int posX = lw;

		for (int i=0; i<_items.count(); i++) {
			if ( _items[i].text.isEmpty() ) continue;
			painter.setFont(_items[i].font);
			painter.setPen (isEnabled() ? (_items[i].colorSet?_items[i].color:palette().color(QPalette::Text)) : palette().color(QPalette::Disabled, QPalette::Text));
			painter.drawText (posX,lw, w, h, _items[i].align, _items[i].text);

			if ( _items[i].width < 0 )
				posX += painter.fontMetrics().boundingRect(_items[i].text).width();
			else
				posX += _items[i].width;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem::RecordViewItem(RecordView *parent, bool withFrame, int frameMargin,
                               int hSpacing)
 : QWidget(), _parent(parent), _widget(NULL), _label(NULL) {
	_seqTemplate = NULL;
	_requestedComponent = _currentComponent = '?';
	setContextMenuPolicy(Qt::CustomContextMenu);
	setupUi(withFrame, frameMargin, hSpacing);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem::RecordViewItem(RecordView *parent, RecordWidget *widget,
                               RecordSequence* records,
                               bool withFrame, int frameMargin, int hSpacing)
 : QWidget(), _parent(parent), _widget(NULL), _label(NULL), _row(-1)
{
	_seqTemplate = NULL;
	_forceInvisibility = false;
	_visible = isVisible();
	_requestedComponent = _currentComponent = '?';
	setContextMenuPolicy(Qt::CustomContextMenu);
	setupUi(withFrame, frameMargin, hSpacing);
	setRecordWidget(widget);
	setRecords(records);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem::~RecordViewItem() {
	clearRecords();

	if ( _seqTemplate != NULL )
		delete _seqTemplate;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordView* RecordViewItem::recordView() const {
	return _parent;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::clearRecords() {
	if ( _widget ) _widget->clearRecords();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordViewItem::feed(const Seiscomp::Record *rec) {
	char comp = component(rec);
	int id = createComponentToSlotMapping(comp);
	RecordSequence* seq = _widget->records(id);

	if ( seq == NULL ) {
		seq = _seqTemplate;
		if ( seq != NULL ) {
			_widget->setRecords(id, seq, true);
			_widget->setRecordFilter(id, recordView()->filter());
			_seqTemplate = NULL;
		}
		else
			seq = _widget->createRecords(id);
	}

	// No target record sequence found, this should never happen
	if ( seq == NULL ) return false;

	bool firstRecord = seq->empty();

	if ( !seq->feed(rec) )
		return false;

	if ( firstRecord ) emit firstRecordAdded(rec);

	_widget->fed(id, rec);

	if ( _requestedComponent == '?' || _requestedComponent == comp ) {
		if ( _currentComponent != comp ) {
			_currentComponent = comp;
			_widget->setCurrentRecords(id);
			emit componentChanged(this, _currentComponent);
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setValue(int column, double d) {
	if ( column >= _values.size() )
		_values.resize(column+1);

	_values[column] = d;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double RecordViewItem::value(int column) const {
	return _values[column];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordViewItem::row() const {
	return _row;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::WaveformStreamID& RecordViewItem::streamID() const {
	return _widget->streamID();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordViewItem::columnCount() const {
	return _values.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setData(const QVariant& data) {
	_data = data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QVariant& RecordViewItem::data() const {
	return _data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setRecords(RecordSequence* records) {
	clearRecords();

	if ( records == NULL ) return;

	if ( records->empty() ) {
		// Clear old instance
		if ( _seqTemplate ) delete _seqTemplate;

		// Just store the sequence pointer as template to
		// create clones of that sequence when the first real
		// records arrive.
		_seqTemplate = records;
	}
	else {
		_currentComponent = component(records);
		int slot = mapComponentToSlot(_currentComponent);
		_widget->setRecords(slot, records);
		_widget->setRecordFilter(slot, recordView()->filter());
		_widget->setCurrentRecords(slot);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence* RecordViewItem::records(char componentCode) const {
	if ( _seqTemplate != NULL ) return _seqTemplate;
	if ( componentCode == '?' ) return _widget->records();
	return _widget->records(mapComponentToSlot(componentCode));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence* RecordViewItem::filteredRecords(char componentCode) const {
	if ( _seqTemplate != NULL ) return _seqTemplate;
	if ( componentCode == '?' ) return _widget->records();
	return _widget->filteredRecords(mapComponentToSlot(componentCode));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setBuffer(RecordSequence *seq) {
	if ( seq == NULL ) return;

	if ( _seqTemplate != NULL ) {
		delete _seqTemplate;
		_seqTemplate = NULL;
	}

	bool seqUsed = false;

	for ( int i = 0; i < _widget->slotCount(); ++i ) {
		const RecordSequence *old = _widget->records(i);
		if ( old == NULL ) continue;

		RecordSequence *tgt = seqUsed?seq->clone():seq;

		RecordSequence::const_iterator it;
		for ( it = old->begin(); it != old->end(); ++it )
			tgt->feed(it->get());

		_widget->setRecords(i, tgt);
		_widget->setRecordFilter(i, recordView()->filter());
		seqUsed = true;
	}

	if ( !seqUsed )
		_seqTemplate = seq;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
float RecordViewItem::timingQuality(char componentCode) const {
	return _widget->timingQuality(mapComponentToSlot(componentCode));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordViewItem::showSlot(int slot) {
	if ( slot > _widget->slotCount() ) return false;

	_requestedComponent = mapSlotToComponent(slot);
	_currentComponent = _requestedComponent;

	_widget->setCurrentRecords(slot);

	emit componentChanged(this, _currentComponent);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordViewItem::showRecords(char componentCode) {
	_requestedComponent = componentCode;

	if ( _requestedComponent == '?' ) {
		if ( _widget->slotCount() > 0 )
			_currentComponent = _widget->setCurrentRecords(0);
		else
			_currentComponent = '?';
	}
	else {
		_widget->setCurrentRecords(mapComponentToSlot(_requestedComponent));
		_currentComponent = _requestedComponent;
	}

	//if ( seq == NULL ) return false;

	emit componentChanged(this, _currentComponent);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
char RecordViewItem::currentComponent() const {
	return _currentComponent;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setLabel(RecordLabel* label) {
	if ( label->_parent != NULL && label->_parent != this ) {
		SEISCOMP_ERROR("Cannot insert a label that is already part of "
		               "another RecordViewItem");
		return;
	}

	if ( _label )
		_labelLayout->removeWidget(_label);
		//delete _label;

	_label = label;

	if ( _label ) {
		_label->_parent = this;
		_labelLayout->addWidget(_label);
		//_label->setBackgroundRole(QPalette::Window);
		if ( _widget )
			connect(_label,  SIGNAL(statusChanged(bool)),
			        _widget, SLOT(setEnabled(bool)));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setLabelText(const QString& text) {
	_label->setText("", 0);
	_label->setText(text, 1);
	_label->setText("", 2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setRecordWidget(RecordWidget *widget) {
	if ( _widget )
		delete _widget;

	_widget = widget;

	if ( _widget ) {
		_widgetLayout->addWidget(_widget);
		_widget->setAutoFillBackground(true);
		setupConnection();

		if ( _label )
			connect(_label,  SIGNAL(statusChanged(bool)),
			        _widget, SLOT(setEnabled(bool)));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::enableFiltering(bool enable) {
	if ( _filtering == enable ) return;
	_filtering = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordViewItem::isFilteringEnabled() const {
	return _filtering;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setBackgroundColor(const QColor& c) {
	QPalette pal;
	/*
	pal = _label->palette();
	pal.setColor(QPalette::Window, c);
	_label->setPalette(pal);
	*/

	pal = _widget->palette();
	pal.setColor(QPalette::Base, c);
	_widget->setPalette(pal);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QColor RecordViewItem::backgroundColor() const {
	return _widget->palette().color(_widget->backgroundRole());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordViewItem::isSelected() const {
	return _selected;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::onClickedOnTime(Seiscomp::Core::Time t) {
	emit clickedOnTime(this, t);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::handleLayoutRequest() {
	setRowHeight(_parent->rowHeight());
	_parent->layoutRows();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setSelected(bool s) {
	if ( _selected == s ) return;
	_selected = s;
	_widget->setActive(_selected);
	setColor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setHighlight(bool h) {
	_highlight = h;
	//setColor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setColor() {
	if ( false/*_highlight*/ ) {
		_widget->setBackgroundRole(QPalette::Highlight);
		//_label->setBackgroundRole(QPalette::Highlight);
	}
	else {
		_widget->setBackgroundRole(_selected?QPalette::Highlight:QPalette::Base);
		//_label->setBackgroundRole(_selected?QPalette::Highlight:QPalette::Window);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setDraggingEnabled(bool e) {
	_enableDragging = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setRowHeight(int h) {
	if ( _widget->drawMode() == RecordWidget::InRows && _widget->slotCount() > 1 )
		resize(width(), h*_widget->slotCount());
	else
		resize(width(), h);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setVisible(bool visible) {
	if ( visible == _visible ) return;
	_visible = visible;
	if ( !_forceInvisibility ) {
		QWidget::setVisible(visible);
		_label->visibilityChanged(visible);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::forceInvisibilty(bool force) {
	_forceInvisibility = force;
	if ( !_forceInvisibility ) {
		QWidget::setVisible(_visible);
		_label->visibilityChanged(_visible);
	}
	else {
		QWidget::setVisible(false);
		_label->visibilityChanged(false);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordViewItem::isInvisibilityForced() const {
	return _forceInvisibility;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordViewItem::createComponentToSlotMapping(char c) {
	SlotMap::iterator it = _slotMapping.find(c);
	if ( it == _slotMapping.end() )
		it = _slotMapping.insert(c, _slotMapping.size());

	return it.value();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordViewItem::mapComponentToSlot(char c) const {
	SlotMap::const_iterator it = _slotMapping.find(c);
	if ( it == _slotMapping.end() )
		return -1;

	return it.value();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
char RecordViewItem::mapSlotToComponent(int s) const {
	for ( SlotMap::const_iterator it = _slotMapping.begin(); it != _slotMapping.end(); ++it )
		if ( it.value() == s )
			return it.key();

	return '?';
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordViewItem::insertComponent(char c, int s) {
	for ( SlotMap::iterator it = _slotMapping.begin(); it != _slotMapping.end(); ++it )
		if ( it.value() == s ) return false;

	if ( s < 0 )
		return false;

	_slotMapping[c] = s;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setupUi(bool withFrame, int frameMargin, int hSpacing) {
	_enableDragging = false;
	_highlight = false;
	_selected = false;

	QFrame *frame;

	QHBoxLayout *lay = new QHBoxLayout(this);
	lay->setSpacing(hSpacing);
	lay->setMargin(0);

	frame = new QFrame(this);
	_labelLayout = new QVBoxLayout(frame);
	_labelLayout->setSpacing(0);
	_labelLayout->setMargin(frameMargin);
	//_labelLayout->addWidget(_label);
	frame->setLayout(_labelLayout);
	//frame->setFrameShape(withFrame?QFrame::StyledPanel:QFrame::NoFrame);
	frame->setFrameStyle(withFrame?(QFrame::StyledPanel | QFrame::Raised):(QFrame::NoFrame | QFrame::Plain));
	frame->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));

	lay->addWidget(frame);

	frame = new QFrame(this);
	_widgetLayout = new QVBoxLayout(frame);
	_widgetLayout->setSpacing(0);
	_widgetLayout->setMargin(frameMargin);
	//_widgetLayout->addWidget(_widget);
	frame->setLayout(_widgetLayout);
	//frame->setFrameShape(withFrame?QFrame::StyledPanel:QFrame::NoFrame);
	frame->setFrameStyle(withFrame?(QFrame::StyledPanel | QFrame::Raised):(QFrame::NoFrame | QFrame::Plain));

	lay->addWidget(frame);

	setLayout(lay);

	//_widget->setAttribute(Qt::WA_OpaquePaintEvent);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::setupConnection() {
	connect(_widget, SIGNAL(mouseOver(bool)),
	        this,  SLOT(setHighlight(bool)));
	connect(_widget, SIGNAL(clickedOnTime(Seiscomp::Core::Time)),
	        this, SLOT(onClickedOnTime(Seiscomp::Core::Time)));
	connect(_widget, SIGNAL(layoutRequest()),
	        this, SLOT(handleLayoutRequest()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::mousePressEvent(QMouseEvent *event) {
	if ( event->button() == Qt::LeftButton ) {
		_dragStart = event->pos();
		emit clicked(this, true, event->modifiers());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::mouseReleaseEvent(QMouseEvent *event) {
	if ( event->button() == Qt::LeftButton )
		emit clicked(this, false, event->modifiers());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordViewItem::mouseMoveEvent(QMouseEvent *event) {
	if ( !_enableDragging ) {
		event->ignore();
		return;
	}

	if ( event->buttons() & Qt::LeftButton ) {
		if ( (event->pos() - _dragStart).manhattanLength() < QApplication::startDragDistance() ) {
			event->ignore();
			return;
		}

		if ( recordView() ) {
			QDrag *drag = new QDrag(recordView());
			QMimeData *mimeData = new QMimeData;

			QList<RecordViewItem*> items = recordView()->selectedItems();

			QString streamList;
			foreach ( RecordViewItem* item, items ) {
				if ( !streamList.isEmpty() ) streamList += '\n';
				streamList += waveformIDToString(item->streamID());
			}

			mimeData->setData("stream/list", streamList.toLatin1());
			mimeData->setText(streamList);
			drag->setMimeData(mimeData);


			#if QT_VERSION >= 0x040300
				drag->exec(Qt::CopyAction | Qt::MoveAction, Qt::MoveAction);
			#else
				drag->start(Qt::MoveAction);
			#endif
		}
	}
	else
		event->ignore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
