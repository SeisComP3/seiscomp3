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



#define SEISCOMP_COMPONENT Gui::RecordView
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/recordview.h>
#include <seiscomp3/gui/core/timescale.h>
#include <seiscomp3/core/recordsequence.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/gui/core/recordstreamthread.h>
#include <seiscomp3/math/filter.h>

#include <limits>

#include <QApplication>
#include <QScrollBar>
#include <QLabel>
#include <QDateTime>
#include <QPainter>
#include <QAction>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;


#define DEFAULT_LABEL_WIDTH    70
#define CHCK255(x)             ((x)>255?255:((x)<0?0:(x)))


namespace Seiscomp {
namespace DataModel {


bool operator<(const WaveformStreamID& left, const WaveformStreamID& right) {
	if ( left.networkCode() < right.networkCode() ) return true;
	if ( left.networkCode() > right.networkCode() ) return false;

	if ( left.stationCode() < right.stationCode() ) return true;
	if ( left.stationCode() > right.stationCode() ) return false;

	if ( left.locationCode() < right.locationCode() ) return true;
	if ( left.locationCode() > right.locationCode() ) return false;

	return left.channelCode() < right.channelCode();
}


}
}


namespace {


class RecordScrollArea : public QScrollArea {
	public:
		RecordScrollArea(Seiscomp::Gui::RecordView* parent = 0)
		 : QScrollArea(parent) {
			_isZooming = false;
			_overlay = new QWidget(this);
			_overlay->installEventFilter(this);
			_overlay->setVisible(false);
			_overlay->setGeometry(viewport()->geometry());
		 }

		void enableZooming(bool e) {
			setUpdatesEnabled(false);
			_overlay->setVisible(e);
			if ( !e ) _isZooming = false;
			setUpdatesEnabled(true);
		}

	protected:
		void resizeEvent(QResizeEvent *e) {
			_overlay->setGeometry(viewport()->geometry());
			QScrollArea::resizeEvent(e);
		}

		bool eventFilter(QObject *obj, QEvent *e) {
			if ( obj != _overlay )
				return QScrollArea::eventFilter(obj, e);

			Seiscomp::Gui::RecordView* view = (Seiscomp::Gui::RecordView*)parent();

			if ( e->type() == QEvent::Paint ) {
				QPainter p((QWidget*)obj);

				QRect viewport(p.window());
				viewport.setLeft(view->labelWidth() + view->horizontalSpacing());

				if ( _isZooming ) {
					p.setPen(QColor(32,64,96));
					p.setBrush(QColor(192,224,255,160));

					QRect rect = QRect(_startPoint, _endPoint).normalized();
					rect &= viewport;
					p.drawRect(rect);
				}

				return true;
			}
			else if ( e->type() == QEvent::MouseButtonPress ) {
				QMouseEvent *ev = static_cast<QMouseEvent*>(e);

				if ( ev->button() == Qt::LeftButton ) {
					_startPoint = ev->pos();
					_endPoint = _startPoint;
					_isZooming = true;

					return true;
				}
				else {
					return QScrollArea::eventFilter(obj, e);
				}
			}
			else if ( e->type() == QEvent::MouseButtonRelease ) {
				QMouseEvent *ev = static_cast<QMouseEvent*>(e);

				if ( ev->button() == Qt::LeftButton ) {
					_isZooming = false;

					QPoint p0 = mapToGlobal(_startPoint);
					QPoint p1 = mapToGlobal(_endPoint);

					view->setZoomRectFromGlobal(QRect(p0, p1).normalized());
					_overlay->update();
					return true;
				}
				else
					return QScrollArea::eventFilter(obj, e);
			}
			else if ( e->type() == QEvent::MouseMove ) {
				QMouseEvent *ev = static_cast<QMouseEvent*>(e);

				if ( _isZooming ) {
					_endPoint = ev->pos();
					_overlay->update();
					return true;
				}
			}
			else if ( e->type() == QEvent::Wheel ) {
				QWheelEvent *ev = static_cast<QWheelEvent*>(e);
				wheelEvent(ev);
				return true;
			}

			return false;
		}

		void wheelEvent(QWheelEvent *event) {
			Seiscomp::Gui::RecordView* p = (Seiscomp::Gui::RecordView*)parent();
			if ( event->modifiers() & Qt::ControlModifier ) {
				QPointF tmp(p->zoomSpot());

				p->setZoomSpotFromGlobal(event->globalPos());

				if ( event->delta() < 0 )
					p->zoomOut();
				else
					p->zoomIn();

				p->setZoomSpot(tmp);

				return;
			}
			QScrollArea::wheelEvent(event);
		}

		void finishZooming() {
			_isZooming = false;
			_overlay->update();
		}

	private:
		QPoint _startPoint;
		QPoint _endPoint;
		bool _isZooming;
		QWidget *_overlay;
};


class ItemWidget : public QWidget {
	public:
		ItemWidget(Seiscomp::Gui::RecordView *owner) : QWidget(), _owner(owner) {}

	protected:
		bool event(QEvent* event) {
			if ( event->type() == QEvent::LayoutRequest )
				QApplication::sendEvent(_owner, event);

			return QWidget::event(event);
		}

		void resizeEvent(QResizeEvent *event) {
			for ( int i = 0; i < _owner->rowCount(); ++i )
				_owner->itemAt(i)->setFixedWidth(event->size().width());
		}

	private:
		Seiscomp::Gui::RecordView* _owner;
};


}

namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordView::RecordView(QWidget * parent, Qt::WFlags f, TimeScale *timeScale)
 : QWidget(parent, f)
, _timeScaleWidget(timeScale) {
	setupUi();
	setBufferSize(Seiscomp::Core::TimeSpan(30*60, 0));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordView::RecordView(const Seiscomp::Core::TimeWindow& tw,
                       QWidget * parent, Qt::WFlags f, TimeScale *timeScale)
 : QWidget(parent, f)
, _timeScaleWidget(timeScale) {
	setupUi();
	setTimeWindow(tw);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordView::RecordView(const Seiscomp::Core::TimeSpan& ts,
                       QWidget * parent, Qt::WFlags f, TimeScale *timeScale)
 : QWidget(parent, f)
, _timeScaleWidget(timeScale) {
	setupUi();
	setBufferSize(ts);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordView::~RecordView() {
	if ( _filter )
		delete _filter;

	closeThread();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::closeEvent(QCloseEvent *e) {
	closeThread();
	QWidget::closeEvent(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::sliderAction(int action) {
	if ( action == QAbstractSlider::SliderMove ) {
		int step = rowHeight() + _rowSpacing;
		int pos = _scrollArea->verticalScrollBar()->sliderPosition();
		if ( pos < _scrollArea->verticalScrollBar()->maximum() )
			_scrollArea->verticalScrollBar()->setSliderPosition((pos/step)*step);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setHorizontalSpacing(int hs) {
	if ( rowCount() > 0 ) {
		std::cerr << "RecordView::setHorizontalSpacing(): spacing can only be "
		             "set when rows are empty" << std::endl;
		return;
	}

	_horizontalSpacing = hs;

	if ( _timeScaleAuxLayout )
		_timeScaleAuxLayout->setSpacing(hs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordView::horizontalSpacing() const {
	return _horizontalSpacing;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setFramesEnabled(bool e) {
	if ( rowCount() > 0 ) {
		std::cerr << "RecordView::setFramesEnabled(): frames can only be "
		             "set when rows are empty" << std::endl;
		return;
	}

	_frames = e;

	if ( _timeScaleAuxLayout ) {
		for ( int i = 0; i < _timeScaleAuxLayout->count(); ++i ) {
			QLayoutItem *item = _timeScaleAuxLayout->itemAt(i);
			QFrame *frame = qobject_cast<QFrame*>(item->widget());
			if ( frame ) {
				//frame->setFrameShape(_frames?QFrame::StyledPanel:QFrame::NoFrame);
				frame->setFrameStyle(_frames?(QFrame::StyledPanel | QFrame::Raised):(QFrame::NoFrame | QFrame::Plain));
				frame->layout()->setMargin(_frameMargin);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setFrameMargin(int margin) {
	_frameMargin = margin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setupUi() {
	qRegisterMetaType<Seiscomp::RecordPtr>("Seiscomp::RecordPtr");

	_labelColumns = 3;

	_selectionMode = NoSelection;
	_currentItem = NULL;

	_filter = NULL;
	_filterAction = NULL;
	_filtering = false;
	_absTimeAction = NULL;
	_labelWidth = DEFAULT_LABEL_WIDTH;

	setFilter(NULL);
	setZoomFactor(2.0f);

	_zoomSpot = QPointF(0.5,0.5);

	_defaultRowHeight = 16;
	_rowHeight = _minRowHeight = _defaultRowHeight;
	_maxRowHeight = -1;
	_numberOfRows = -1;

	_minTimeScale = 0.0;
	_timeScale = 1.0/3.0;
	_amplScale = 0.;

	_tmin = 0;
	_tmax = 0;
	_rowSpacing = 0;
	_horizontalSpacing = 0;
	_frames = false;
	_frameMargin = 0;

	_scrollArea = new RecordScrollArea;
	_scrollArea->setWidgetResizable(true);

	/*
	connect(_scrollArea->verticalScrollBar(), SIGNAL(actionTriggered(int)),
	        this, SLOT(sliderAction(int)));
	*/

	QWidget* scrollWidget = new ItemWidget(this);

	// Setting up timescale widget
	if ( !_timeScaleWidget )
		_timeScaleWidget = new TimeScale;
	_timeScaleWidget->setRange(_tmin, 0);
	_timeScaleWidget->setScale(_timeScale);
	_timeScaleWidget->setSelected(0, 0);
	_timeScaleWidget->setAbsoluteTimeEnabled(true);
	_timeScaleWidget->setRangeSelectionEnabled(true);
	//_timeScaleWidget->setAutoFillBackground(true);

	_timeScaleInfo = new QWidget;
	_timeScaleInfo->setFixedWidth(_labelWidth);

	_timeScaleAuxLayout = new QHBoxLayout;

	// Left spacer for timescale widget
	QFrame *frame = new QFrame;
	QVBoxLayout *frameLayout = new QVBoxLayout(frame);
	frameLayout->setSpacing(0);
	frameLayout->setMargin(_frameMargin);
	frameLayout->addWidget(_timeScaleInfo);
	frame->setLayout(frameLayout);
	frame->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
	//frame->setFrameShape(_frames?QFrame::StyledPanel:QFrame::NoFrame);
	frame->setFrameStyle(_frames?(QFrame::StyledPanel | QFrame::Raised):(QFrame::NoFrame | QFrame::Plain));

	QWidget *timeScaleAuxWidget = new QWidget;
	_timeScaleAuxLayout->setSpacing(_horizontalSpacing);
	_timeScaleAuxLayout->setMargin(0);

	// Add left info frame
	_timeScaleAuxLayout->addWidget(frame);

	frame = new QFrame;
	//frame->setFrameShape(_frames?QFrame::StyledPanel:QFrame::NoFrame);
	frame->setFrameStyle(_frames?(QFrame::StyledPanel | QFrame::Raised):(QFrame::NoFrame | QFrame::Plain));
	frameLayout = new QVBoxLayout(frame);
	frameLayout->setSpacing(0);
	frameLayout->setMargin(_frameMargin);
	frameLayout->addWidget(_timeScaleWidget);
	frame->setLayout(frameLayout);

	_timeScaleAuxLayout->addWidget(frame);
	timeScaleAuxWidget->setLayout(_timeScaleAuxLayout);

	connect(_timeScaleWidget, SIGNAL(changedSelection(double, double)),
	        this, SLOT(setSelection(double, double)));

	connect(_timeScaleWidget, SIGNAL(changedInterval(double, double, double)),
	        this, SIGNAL(updatedInterval(double, double, double)));

	connect(_timeScaleWidget, SIGNAL(rangeChangeRequested(double,double)),
	        this, SLOT(setTimeRange(double,double)));

	QLayout* centralLayout = new QVBoxLayout;
	centralLayout->setSpacing(_rowSpacing);
	centralLayout->setMargin(0);
	setLayout(centralLayout);

	centralLayout->addWidget(_scrollArea);
	centralLayout->addWidget(timeScaleAuxWidget);

	_scrollArea->setWidget(scrollWidget);
	_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	_scrollArea->setFrameShape(QFrame::NoFrame);

	scrollWidget->setAutoFillBackground(false);
	_scrollArea->viewport()->setAutoFillBackground(false);

	_autoInsertItems = true;
	_alternatingColors = false;
	_showAllRecords = false;
	_autoScale = false;
	_autoMaxScale = false;

	_thread = NULL;

	connect(&_recordUpdateTimer, SIGNAL(timeout()),
	        this, SLOT(updateRecords()));
	connect(&_recordUpdateTimer, SIGNAL(timeout()),
	        this, SIGNAL(updatedRecords()));

	setAcceptDrops(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setMinimumRowHeight(int h) {
	_minRowHeight = h;
	if ( (_maxRowHeight > 0) && (_maxRowHeight < _minRowHeight) )
		_maxRowHeight = _minRowHeight;

	if ( _minRowHeight > rowHeight() )
		setRowHeight(_minRowHeight);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setMaximumRowHeight(int h) {
	_maxRowHeight = h;
	if ( (_maxRowHeight > 0) && (_minRowHeight > _maxRowHeight) )
		_minRowHeight = _maxRowHeight;

	if ( rowHeight() > _maxRowHeight )
		setRowHeight(_maxRowHeight);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setRelativeRowHeight(int desiredNumberOfTraces) {
	_numberOfRows = desiredNumberOfTraces;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setSelectionMode(SelectionMode mode) {
	if ( mode == NoSelection )
		clearSelection();

	_selectionMode = mode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::clearSelection() {
	if ( !_selectedItems.empty() ) {
		foreach ( RecordViewItem* item, _selectedItems )
			item->setSelected(false);

		_selectedItems.clear();
		emit selectionChanged();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setTimeWindow(const Seiscomp::Core::TimeWindow& tw) {
	_mode = TIME_WINDOW;
	_timeStart = tw.startTime();
	_timeSpan = tw.endTime() - _timeStart;
	applyBufferChange();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setBufferSize(const Seiscomp::Core::TimeSpan& ts) {
	_mode = RING_BUFFER;
	_timeSpan = ts;
	applyBufferChange();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::applyBufferChange() {
	foreach (RecordViewItem* item, _rows) {
		RecordSequence* seq = NULL;

		switch ( _mode ) {
			case TIME_WINDOW:
				seq = new TimeWindowBuffer(TimeWindow(_timeStart, _timeStart + _timeSpan));
				break;
			case RING_BUFFER:
				seq = new RingBuffer(_timeSpan);
				break;
		}

		item->setBuffer(seq);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setRowHeight(int h, bool allowStretch) {
	if ( _rowHeight == h ) return;
	_rowHeight = h;

	int stretch = 0;
	int budget = 0;
	int budgetRows = 0;

	if ( allowStretch ) {
		// Stretch the first items to match the window height
		int targetHeight = _scrollArea->viewport()->height();
		budgetRows = (targetHeight + _rowSpacing) / (_rowHeight + _rowSpacing);
		if ( budgetRows > 1 ) {
			int layoutHeight = budgetRows * _rowHeight + (budgetRows-1)*_rowSpacing;
			budget = targetHeight - layoutHeight;
			stretch = (budget + budgetRows - 1) / budgetRows;
		}
	}

	foreach (RecordViewItem* item, _rows) {
		item->widget()->setDirty();
		item->setRowHeight(rowHeight() + stretch);

		if ( budgetRows > 1 ) {
			budget -= stretch;
			--budgetRows;
			stretch = (budget + budgetRows - 1) / budgetRows;
		}
		else
			stretch = 0;
	}

	_scrollArea->verticalScrollBar()->setSingleStep(rowHeight() + _rowSpacing);

	layoutRows();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setDefaultRowHeight(int height) {
	_defaultRowHeight = height;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setDefaultItemColumns(int numCols) {
	_labelColumns = numCols;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setRowSpacing(int spacing) {
	if ( _rowSpacing == spacing || spacing < 0 ) return;
	_rowSpacing = spacing;

	if ( layout() )
		layout()->setSpacing(_rowSpacing);

	layoutRows();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setLabelWidth(int width) {
	if ( _labelWidth == width || width <= 0 ) return;
	_labelWidth = width;
	_timeScaleInfo->setFixedWidth(_labelWidth);

	foreach (RecordViewItem* item, _rows)
		item->label()->setFixedWidth(_labelWidth);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordView::labelWidth() const {
	return _labelWidth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::layoutRows() {
	int pos = 0, row = 0;

	setUpdatesEnabled(false);

	foreach (RecordViewItem* item, _rows) {
		if ( item->isHidden() ) continue;

		if ( item->pos().y() != pos )
			item->move(0, pos);

		colorItem(item, row);

		pos += item->height() + _rowSpacing;
		++row;
	}

	int newHeight = pos - _rowSpacing;

	if ( newHeight < 0 ) newHeight = 0;

	if ( _scrollArea->widget()->height() != newHeight )
		_scrollArea->widget()->setFixedHeight(newHeight);

	if ( isVisible() && _currentItem && !_currentItem->isVisible() ) {
		setCurrentItem(NULL);
	}

	setUpdatesEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordView::rowHeight() const {
	return _rowHeight;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time RecordView::alignment() const {
	return _alignment;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem* RecordView::currentItem() const {
	return _currentItem;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QList<RecordViewItem*> RecordView::selectedItems() const {
	QList<RecordViewItem*> items;

	foreach ( RecordViewItem *item, _selectedItems )
		items << item;

	return items;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double RecordView::timeRangeMin() const {
	return _tmin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double RecordView::timeRangeMax() const {
	return (float)(_scrollArea->viewport()->width() - _labelWidth - _horizontalSpacing)/_timeScale + _tmin;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setZoomFactor(float factor) {
	_zoomFactor = factor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
float RecordView::zoomFactor() const {
	return _zoomFactor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setAlternatingRowColors(bool enable) {
	if ( _alternatingColors == enable ) return;
	_alternatingColors = enable;

	foreach (RecordViewItem* item, _rows)
		colorItem(item);

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setAutoInsertItem(bool enable) {
	_autoInsertItems = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::showAllRecords(bool enable) {
	if ( _showAllRecords == enable ) return;

	_showAllRecords = enable;

	int step = 0;
	emit progressStarted();
	foreach (RecordViewItem* item, _items) {
		item->widget()->showAllRecords(_showAllRecords);
		emit progressChanged(++step * 100 / rowCount());
	}
	emit progressFinished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::showScrollBar(bool show) {
	if ( show )
		_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	else
	        _scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setAutoScale(bool enable) {
	if ( _autoScale == enable ) return;

	_autoScale = enable;

	if ( _autoScale )
		scaleContent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setAutoMaxScale(bool enable) {
	if ( _autoMaxScale == enable ) return;
	_autoMaxScale = enable;
	foreach(RecordViewItem* item, _items)
		item->widget()->setAutoMaxScale(_autoMaxScale);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::feed(const Seiscomp::Record *rec) {
	RecordCPtr saver(rec);

	if ( rec == NULL )
		return false;

	try {
		rec->endTime();
	}
	catch ( ... ) {
		return false;
	}

	DataModel::WaveformStreamID streamID(rec->networkCode(), rec->stationCode(),
	                                     rec->locationCode(), rec->channelCode(), "");
	QString stationCode = rec->stationCode().c_str();

	RecordViewItem* child = item(streamID);
	if ( !child ) {
		if ( _autoInsertItems ) {
			child = addItem(streamID, stationCode);
			if ( !child ) return false;
			emit addedItem(rec, child);
		}
		else
			return false;
	}

	if ( child->feed(rec) ) {
		emit fedRecord(child, rec);

		if ( !_recordUpdateTimer.isActive() ) {
			RecordWidget *w = child->widget();
			if ( updatesEnabled() )
				w->update();
		}

		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::feed(const Seiscomp::RecordPtr rec) {
	return feed(rec.get());
}
bool RecordView::feed(Seiscomp::Record *rec) {
	return feed((const Seiscomp::Record*)rec);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setRecordUpdateInterval(int ms) {
	_recordUpdateTimer.stop();
	if ( ms > 0 )
		_recordUpdateTimer.start(ms);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::updateRecords() {
	foreach(RecordViewItem* item, _items)
		item->widget()->updateRecords();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget *RecordView::createRecordWidget(const DataModel::WaveformStreamID&) const {
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordLabel *RecordView::createLabel(RecordViewItem*) const {
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem* RecordView::addItem(const DataModel::WaveformStreamID& streamID,
                                    const QString& /*stationCode*/, int slotCount) {
	if ( item(streamID) != NULL ) return NULL;

	RecordSequence* seq = NULL;

	switch ( _mode ) {
		case TIME_WINDOW:
			seq = new TimeWindowBuffer(TimeWindow(_timeStart, _timeStart + _timeSpan));
			break;
		case RING_BUFFER:
			seq = new RingBuffer(_timeSpan);
			break;
	}

	if ( !seq ) return NULL;

	/*
	std::cout << "insert row for stream '" << (const char*)streamID.toAscii() << "'"
	          << std::endl;
	*/

	RecordWidget *widget = createRecordWidget(streamID);
	if ( widget == NULL ) widget = new RecordWidget(streamID);

	RecordViewItem *item = new RecordViewItem(this, widget, seq, _frames, _frameMargin, _horizontalSpacing);
	item->widget()->setSlotCount(slotCount);

	RecordLabel* label = createLabel(item);
	if ( label == NULL )
		label = new StandardRecordLabel(_labelColumns);

	item->setLabel(label);

	if ( !addItem(item) ) {
		delete item;
		item = NULL;
	}

	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::colorItem(RecordViewItem* item, int row) {
	QPalette pal;
	if ( (row % 2) && _alternatingColors )
		item->setBackgroundColor(SCScheme.colors.records.alternateBackground);
	else
		item->setBackgroundColor(SCScheme.colors.records.background);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::colorItem(RecordViewItem* item) {
	colorItem(item, item->row());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::onItemClicked(RecordViewItem* item, bool buttonDown, Qt::KeyboardModifiers m) {
	if ( _selectionMode == NoSelection ) return;

	if ( m == Qt::NoModifier ) {
		if ( (!buttonDown && item->isSelected()) || !item->isSelected() ) {
			if ( !item->isSelected() || _selectedItems.size() > 1 ) {
				blockSignals(true);
				clearSelection();
				blockSignals(false);
				setItemSelected(item, true);
			}
		}
	}
	else if ( _selectionMode == ExtendedSelection ) {
		if ( m == Qt::ShiftModifier ) {
			if ( _currentItem && buttonDown ) {
				blockSignals(true);
				int fromRow = _currentItem->row();
				int toRow = item->row();

				if ( fromRow > toRow ) std::swap(fromRow, toRow);

				bool changed = false;
				for ( int i = fromRow; i <= toRow; ++i ) {
					RecordViewItem* it = itemAt(i);
					if ( !it->isSelected() ) {
						changed = true;
						setItemSelected(it, true);
					}
				}
				blockSignals(false);

				if ( changed )
					emit selectionChanged();
			}
		}
		else if ( m == Qt::ControlModifier ) {
			if ( buttonDown )
				setItemSelected(item, !item->isSelected());
		}
		else
			return;
	}

	if ( buttonDown )
		setCurrentItem(item);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::selectedTime(Seiscomp::Core::Time time) {
	emit selectedTime((RecordWidget*)sender(), time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordView::rowCount() const {
	return _rows.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordView::visibleRowCount() const {
	int rows = 0;
	foreach(RecordViewItem* item, _items) {
		if ( item->isVisible() ) ++rows;
	}

	return rows;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::WaveformStreamID RecordView::streamID(int row) const {
	if ( row >= rowCount() ) return DataModel::WaveformStreamID();
	return _items.key(_rows[row]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::isEnabled(int row) const {
	return _rows[row]->label()->isEnabled();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem* RecordView::itemAt(int row) const {
	if ( row >= _rows.count() ) {
		SEISCOMP_ERROR("Row %d out of range [0..%d]", row, _rows.count()-1);
		//return NULL;
	}

	return _rows[row];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::removeItem(int row) {
	RecordViewItem* item = takeItem(row);
	if ( item == NULL ) return false;

	delete item;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem* RecordView::takeItem(int row) {
	RecordViewItem* item = _items.value(streamID(row));
	takeItem(item);
	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::takeItem(RecordViewItem* item) {
	if ( !item || item->recordView() != this ) return false;

	if ( item == _currentItem )
		setCurrentItem(NULL);

	if ( item->isSelected() )
		setItemSelected(item, false);

	int row = item->row();
	int offset = 0;
	if ( row < _rows.size()-1 )
		offset = _rows[row+1]->pos().y() - item->pos().y();

	if ( !_items.remove(item->streamID()) ) {
		SEISCOMP_ERROR("Could not remove item '%s.%s.%s.%s' from ItemMap", item->streamID().networkCode().c_str(),
		                                                                   item->streamID().stationCode().c_str(),
		                                                                   item->streamID().locationCode().c_str(),
		                                                                   item->streamID().channelCode().c_str());
		return false;
	}

	_rows.remove(row);

	for ( int r = row; r < _rows.size(); ++r ) {
		--_rows[r]->_row;
		colorItem(_rows[r]);
		_rows[r]->move(0, _rows[r]->pos().y() - offset);
	}

	if ( _autoScale )
		scaleContent();
	else
		_scrollArea->widget()->setFixedHeight(_scrollArea->widget()->height() - offset);

	item->_row = -1;
	item->_parent = NULL;
	item->setParent(NULL);

	// Disconnect our item
	item->disconnect(this);
	item->label()->disconnect(this);
	item->widget()->disconnect(this);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::cycleToRow(int row) {
	if ( row >= rowCount() || row < 0 || rowCount() < 1 ) return;

	QVector<RecordViewItem*> oldRows = _rows;

	for ( int i = 0; i < rowCount(); ++i ) {
		int newRow = i - row;
		if ( newRow < 0 ) newRow += rowCount();
		_rows[newRow] = oldRows[i];
	}

	layoutRows();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::addItem(RecordViewItem* item) {
	if ( !item || (item->recordView() && item->recordView() != this) ) return false;

	item->setParent(_scrollArea->widget());
	item->setSelected(false);

	item->widget()->setFilter(_filter);
	item->widget()->setGridSpacing(_timeScaleWidget->dA(), _timeScaleWidget->dT(), _timeScaleWidget->dOfs());
	item->widget()->setAlignment(_alignment);
	item->widget()->setTimeRange(_tmin, 0);
	item->widget()->setScale(_timeScale);
	item->widget()->enableFiltering(isFilterEnabled());
	item->widget()->showAllRecords(_showAllRecords);
	item->widget()->setAutoMaxScale(_autoMaxScale);
	item->label()->setFixedWidth(_labelWidth);

	if ( _timeScaleWidget )
		item->widget()->setSelected(_timeScaleWidget->minimumSelection(),
		                            _timeScaleWidget->maximumSelection());

	item->_visible = false;
	item->resize(_scrollArea->widget()->width(), 0);
	item->setRowHeight(rowHeight());
	item->show();
	item->_row = rowCount();
	_items[item->_widget->streamID()] = item;
	_rows.push_back(item);
	item->_parent = this;

	layoutRows();
	//_scrollArea->widget()->setFixedHeight(_scrollArea->widget()->height() + _rowSpacing + item->height());
	//item->move(0, _scrollArea->widget()->height()-item->height());

	connect(item, SIGNAL(clicked(RecordViewItem*, bool, Qt::KeyboardModifiers)),
	        this, SLOT(onItemClicked(RecordViewItem*, bool, Qt::KeyboardModifiers)));

	connect(item->widget(), SIGNAL(selectedTime(Seiscomp::Core::Time)),
	        this, SLOT(selectedTime(Seiscomp::Core::Time)));

	connect(this, SIGNAL(updatedInterval(double, double, double)),
	        item->widget(), SLOT(setGridSpacing(double, double, double)));

	connect(item->widget(), SIGNAL(cursorMoved(QPoint)),
	        this, SLOT(setZoomSpotFromGlobal(const QPoint&)));

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::removeItem(RecordViewItem* item) {
	if ( item == NULL ) return false;
	return removeItem(item->row());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::clear() {
	setCurrentItem(NULL);
	clearSelection();

	foreach (RecordViewItem* item, _items)
		delete item;

	_items.clear();
	_rows.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::clearRecords() {
	foreach ( RecordViewItem* item, _items ) {
		RecordSequence* seq = NULL;

		switch ( _mode ) {
			case TIME_WINDOW:
				seq = new TimeWindowBuffer(TimeWindow(_timeStart, _timeStart + _timeSpan));
				break;
			case RING_BUFFER:
				seq = new RingBuffer(_timeSpan);
				break;
		}

		item->setRecords(seq);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem* RecordView::item(const DataModel::WaveformStreamID& streamID) const {
	Items::const_iterator it = _items.find(streamID);
	if ( it == _items.end() ) {
		std::string chaCode = streamID.channelCode();
		if ( chaCode.size() < 3 )
			chaCode += '?';
		else
			chaCode[chaCode.size()-1] = '?';
		DataModel::WaveformStreamID tmp(streamID);
		tmp.setChannelCode(chaCode);
		it = _items.find(tmp);
		if ( it == _items.end() )
			return NULL;
	}
	return *it;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::scrollLeft() {
	float offset = 0;

	if ( _currentItem && !_currentItem->widget()->cursorText().isEmpty() ) {
		Core::Time cp = _currentItem->widget()->cursorPos();
		cp -= Core::TimeSpan((float)width()/(20*_timeScale));
		_currentItem->widget()->setCursorPos(cp);
		if ( cp < _currentItem->widget()->leftTime() )
			offset = _currentItem->widget()->leftTime() - cp;
		else
			return;
	}
	else
		offset = (float)width()/(8*_timeScale);

	setTimeRange(_tmin - offset, _tmax - offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::scrollLeftSlowly() {
	float offset = 0;

	if ( _currentItem && !_currentItem->widget()->cursorText().isEmpty() ) {
		Core::Time cp = _currentItem->widget()->cursorPos();
		cp -= Core::TimeSpan(1.0f/_timeScale);
		_currentItem->widget()->setCursorPos(cp);
		if ( cp < _currentItem->widget()->leftTime() )
			offset = _currentItem->widget()->leftTime() - cp;
		else
			return;
	}
	else
		offset = 1.0/_timeScale;

	setTimeRange(_tmin - offset, _tmax - offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::scrollRight() {
	float offset = 0;

	if ( _currentItem && !_currentItem->widget()->cursorText().isEmpty() ) {
		Core::Time cp = _currentItem->widget()->cursorPos();
		cp += Core::TimeSpan((float)width()/(20*_timeScale));
		_currentItem->widget()->setCursorPos(cp);
		if ( cp > _currentItem->widget()->rightTime() )
			offset = cp - _currentItem->widget()->rightTime();
		else
			return;
	}
	else
		offset = (float)width()/(8*_timeScale);

	setTimeRange(_tmin + offset, _tmax + offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::scrollRightSlowly() {
	float offset = 0;

	if ( _currentItem && !_currentItem->widget()->cursorText().isEmpty() ) {
		Core::Time cp = _currentItem->widget()->cursorPos();
		cp += Core::TimeSpan(1.0f/_timeScale);
		_currentItem->widget()->setCursorPos(cp);
		if ( cp > _currentItem->widget()->rightTime() )
			offset = cp - _currentItem->widget()->rightTime();
		else
			return;
	}
	else
		offset = (float)1.0/_timeScale;

	setTimeRange(_tmin + offset, _tmax + offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::scrollLineUp() {
	/*if ( !_scrollArea->verticalScrollBar()->isVisible() )
		cycleToRow(rowCount()-1);
	else*/ {
		int dy = rowHeight() + _rowSpacing;
		int  y = _scrollArea->verticalScrollBar()->sliderPosition();
		_scrollArea->verticalScrollBar()->setSliderPosition(y-dy);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::scrollLineDown() {
	/*if ( !_scrollArea->verticalScrollBar()->isVisible() )
		cycleToRow(1);
	else*/ {
		int dy = rowHeight() + _rowSpacing;
		int  y = _scrollArea->verticalScrollBar()->sliderPosition();
		_scrollArea->verticalScrollBar()->setSliderPosition(y+dy);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::scrollPageUp() {
	int dy = _scrollArea->verticalScrollBar()->pageStep();
	int  y = _scrollArea->verticalScrollBar()->sliderPosition();
	_scrollArea->verticalScrollBar()->setSliderPosition(y-dy);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::scrollPageDown() {
	int dy = _scrollArea->verticalScrollBar()->pageStep();
	int  y = _scrollArea->verticalScrollBar()->sliderPosition();
	_scrollArea->verticalScrollBar()->setSliderPosition(y+dy);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::scrollToTop() {
	_scrollArea->verticalScrollBar()->setSliderPosition(-1000000);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::scrollToBottom() {
	_scrollArea->verticalScrollBar()->setSliderPosition(1000000);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::selectPreviousRow() {
	if ( _currentItem ) {
		int previousRow = _currentItem->row();
		int hops = rowCount();
		do {
			--previousRow;
			if ( previousRow < 0 ) previousRow += rowCount();
			RecordViewItem* item = itemAt(previousRow);
			//if ( !item->label()->isEnabled() ) continue;
			if ( !item->label()->isVisible() ) continue;
			onItemClicked(item, true);
			onItemClicked(item, false);
			ensureVisible(previousRow);
			break;
		}
		while ( hops-- );
	}
	else
		scrollLineUp();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::selectNextRow() {
	if ( _currentItem ) {
		int nextRow = _currentItem->row();
		int hops = rowCount();
		do {
			++nextRow;
			if ( nextRow >= rowCount() ) nextRow -= rowCount();
			RecordViewItem* item = itemAt(nextRow);
			//if ( !item->label()->isEnabled() ) continue;
			if ( !item->label()->isVisible() ) continue;
			onItemClicked(item, true);
			onItemClicked(item, false);
			ensureVisible(nextRow);
			break;
		}
		while ( hops-- );
	}
	else
		scrollLineDown();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::selectFirstRow() {
	if ( _currentItem ) {
		int r = 0;
		RecordViewItem* item;
		while ( r < rowCount() ) {
			item = itemAt(r);
			if ( item->isVisible() ) {
				onItemClicked(item, true);
				onItemClicked(item, false);
				ensureVisible(r);
				break;
			}
			++r;
		}
	}
	else
		scrollToTop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::selectLastRow() {
	if ( _currentItem ) {
		int r = rowCount()-1;
		RecordViewItem* item;
		while ( r >= 0 ) {
			item = itemAt(r);
			if ( item->isVisible() ) {
				onItemClicked(item, true);
				onItemClicked(item, false);
				ensureVisible(r);
				break;
			}
			--r;
		}
	}
	else
		scrollToBottom();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::ensureVisible(int row) {
	RecordViewItem* item = itemAt(row);
	if ( item == NULL ) return;

	QPoint pos = item->pos();
	QSize size = item->size();
	_scrollArea->ensureVisible(pos.x() + size.width()/2,
		                       pos.y() + size.height()/2,
		                       size.width()/2, size.height()/2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::showSlot(int slot) {
	int step = 0;
	emit progressStarted();
	foreach (RecordViewItem* item, _items) {
		item->showSlot(slot);
		emit progressChanged(++step * 100 / rowCount());
	}
	emit progressFinished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::showComponent(char componentCode) {
	int step = 0;
	emit progressStarted();
	foreach (RecordViewItem* item, _items) {
		item->showRecords(componentCode);
		emit progressChanged(++step * 100 / rowCount());
	}
	emit progressFinished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::enableFilter(bool enable) {
	if ( _filtering == enable ) return;
	_filtering = enable;

	if ( _filterAction)
		_filterAction->setChecked(_filtering);

	bool wasActive = _recordUpdateTimer.isActive();
	_recordUpdateTimer.stop();

	int step = 0;
	emit progressStarted();
	foreach (RecordViewItem* item, _items) {
		item->widget()->enableFiltering(enable);
		emit progressChanged(++step * 100 / rowCount());
	}
	emit progressFinished();
	emit toggledFilter(_filtering);

	if ( wasActive )
		_recordUpdateTimer.start();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::enableFilter(int slot, bool enable) {
	bool wasActive = _recordUpdateTimer.isActive();
	_recordUpdateTimer.stop();

	int step = 0;
	emit progressStarted();
	foreach (RecordViewItem* item, _items) {
		item->widget()->enableRecordFiltering(slot, enable);
		emit progressChanged(++step * 100 / rowCount());
	}
	emit progressFinished();
	emit toggledFilter(_filtering);

	if ( wasActive )
		_recordUpdateTimer.start();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::isFilterEnabled() const {
	return _filtering;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::scaleContent() {
	int w = _scrollArea->viewport()->width() - _labelWidth - _horizontalSpacing;
	int h = _scrollArea->viewport()->height();

	if ( w <= 0 ) return;

	float timeWindowLength = _tmax - _tmin;

	if ( timeWindowLength < 0 ) {
		timeWindowLength = 60;
		_tmax = _tmin + timeWindowLength;
	}

	float scale;
	
	if ( timeWindowLength == 0 )
		scale = 0;
	else
		scale = (float)w/timeWindowLength;
	
	/*
	if ( scale < _minTimeScale )
		scale = _minTimeScale;
	*/

	if ( scale != _timeScale )
		setScale(scale, _amplScale);

	int visibleRows = visibleRowCount();

	if ( visibleRows > 0 ) {
		if ( _numberOfRows > 0 )
			visibleRows = _numberOfRows;

		int rowHeight = ((h + _rowSpacing) / visibleRows) - _rowSpacing;
		if ( rowHeight < _minRowHeight )
			rowHeight = _minRowHeight;

		if ( (_maxRowHeight > 0) && (rowHeight > _maxRowHeight) )
			rowHeight = _maxRowHeight;

		setRowHeight(rowHeight, true);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setScale(double t, float a) {
	_timeScale = t;
	_amplScale = a;

	if (_timeScaleWidget)
		_timeScaleWidget->setScale(_timeScale);

	foreach (RecordViewItem* item, _items)
		item->widget()->setScale(_timeScale, _amplScale);

	emit scaleChanged(t, a);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double RecordView::timeScale() const {
	return _timeScale;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QList<RecordViewItem*> RecordView::stationStreams(const std::string& networkCode,
                                                  const std::string& stationCode) const {
	QList<RecordViewItem*> items;
	foreach (RecordViewItem* item, _items) {
		if ( item->streamID().networkCode() == networkCode &&
		     item->streamID().stationCode() == stationCode )
			items << item;
	}

	return items;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::copyState(RecordView* from) {
	setUpdatesEnabled(false);
	blockSignals(true);

	setAlignment(from->alignment());
	_zoomSpot = from->_zoomSpot;

	setMinimumRowHeight(from->_minRowHeight);
	setMaximumRowHeight(from->_maxRowHeight);
	setRowHeight(from->rowHeight());
	setZoomFactor(from->zoomFactor());

	setTimeRange(from->timeRangeMin(), from->timeRangeMax());

	_minTimeScale = from->_minTimeScale;

	setScale(from->timeScale(), from->_amplScale);
	enableFilter(from->isFilterEnabled());

	setAlternatingRowColors(from->_alternatingColors);
	showAllRecords(from->_showAllRecords);
	setAutoInsertItem(from->_autoInsertItems);
	setAutoScale(from->_autoScale);

	setFramesEnabled(from->_frames);
	setHorizontalSpacing(from->_horizontalSpacing);
	setRowSpacing(from->_rowSpacing);
	setLabelWidth(from->_labelWidth);

	setFilter(from->_filter);

	setUpdatesEnabled(true);
	blockSignals(false);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::moveItemsTo(RecordView *to) {
	while ( rowCount() ) {
		RecordViewItem *item = takeItem(0);
		if ( item )
			if ( !to->addItem(item) )
				return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::moveSelectionTo(RecordView *to) {
	if ( this == to ) return false;

	QList<Seiscomp::Gui::RecordViewItem*> items = selectedItems();

	foreach ( RecordViewItem* item, items ) {
		if ( takeItem(item) )
			to->addItem(item);
		else
			return false;
	}

	if ( _autoScale ) scaleContent();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setTimeRange(double t1, double t2) {
	_tmin = t1;
	_tmax = t2;

	if (_timeScaleWidget)
		_timeScaleWidget->setTimeRange(_tmin, 0);

	foreach (RecordViewItem* item, _items)
		item->widget()->setTimeRange(_tmin, 0);

	if ( _autoScale )
		scaleContent();

	emit timeRangeChanged(_tmin, _tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::move(double offset) {
	bool saveAutoScale = _autoScale;
	_autoScale = false;
	setTimeRange(_tmin+offset, _tmax+offset);
	_autoScale = saveAutoScale;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setSelection(double t1, double t2) {
	foreach (RecordViewItem* item, _items) {
		item->widget()->setSelected(t1, t2);
		if (item->widget()->visibleRegion().isEmpty())
			continue;
		item->widget()->update();
	}

	if (_timeScaleWidget)
		_timeScaleWidget->setSelected(t1, t2);

	emit selectionChanged(t1, t2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setSelectionEnabled(bool enabled) {
	if ( _timeScaleWidget )
		_timeScaleWidget->setSelectionEnabled(enabled);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setJustification(float justification) {
	_zoomSpot.setX(justification);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::align() {
	int w = _scrollArea->viewport()->width() - _labelWidth - _horizontalSpacing;

	float length = w/_timeScale;
	float tcen = - length*_zoomSpot.x();

	setTimeRange(tcen, tcen + length);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::horizontalZoom(float factor) {
	int w = _scrollArea->viewport()->width() - _labelWidth - _horizontalSpacing;

	//float tcen = (_zoomSpot.x() * (float)w) / _timeScale + _tmin;

	float pps = _timeScale;
	float tmax = _tmin+w/pps;
	float tcen = _tmin + (tmax-_tmin)*_zoomSpot.x();

	/*
	std::cout << "nullpx = " << -_tmin * pps << std::endl;

	std::cout << "tmin = " << _tmin << std::endl;
	std::cout << "tmax = " << tmax << std::endl;
	std::cout << "TCEN = " << tcen << std::endl;
	std::cout << "zoomspot (px) = " << _zoomSpot.x() * w << std::endl;
	std::cout << "tcen (px) = " << (tcen - _tmin) * pps << std::endl;
	*/

	pps *= factor;
	if ( pps > 100000 )
		pps = 100000;
	else if ( pps < 0.00001 )
		pps = 0.00001;

	_minTimeScale = pps;

	factor = _timeScale/pps;

	setTimeRange(tcen - (tcen-_tmin)*factor, tcen + (tmax-tcen)*factor);
	setScale(pps);

	tmax = _tmin+w/_timeScale;
	tcen = _tmin + (tmax-_tmin)*_zoomSpot.x();

	/*
	std::cout << "new nullpx = " << -_tmin * _timeScale << std::endl;
	std::cout << "new tmin = " << _tmin << std::endl;
	std::cout << "new tmax = " << tmax << std::endl;
	std::cout << "new TCEN = " << tcen << std::endl;
	std::cout << "new zoomspot (px) = " << (tcen - _tmin) * _timeScale << std::endl;
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::verticalZoom(float factor) {
	int h  = rowHeight();
	int h2 = (int)ceil(h*factor);

	if ( h2 < _defaultRowHeight ) h2 = _defaultRowHeight;
	if ( h2 > _scrollArea->viewport()->height() )
		h2 = _scrollArea->viewport()->height();

	_minRowHeight = h2;
	if ( (_maxRowHeight > 0) && _maxRowHeight < h2 )
		_maxRowHeight = -1;

	int oldHeight = _scrollArea->widget()->height();
	if( oldHeight == 0 ) return;
	int posY = (int)(_zoomSpot.y()*_scrollArea->viewport()->height());
	int absolutePos = posY + _scrollArea->verticalScrollBar()->sliderPosition();

	if ( _autoScale )
		scaleContent();
	else
		setRowHeight(_minRowHeight);

	int newAbsolutePos = (int)(((double)absolutePos / (double)oldHeight) * _scrollArea->widget()->height());

	_scrollArea->verticalScrollBar()->setSliderPosition(newAbsolutePos - posY);

	if ( _currentItem )
		ensureVisible(_currentItem->row());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setZoomSpotFromGlobal(const QPoint& p) {
	QPoint pos = _scrollArea->mapFromGlobal(p);

	setZoomSpot(QPointF((float)(pos.x() - _labelWidth - _horizontalSpacing) /
	                    (float)(_scrollArea->viewport()->width() - _labelWidth - _horizontalSpacing),
	                    (float)pos.y() / (float)_scrollArea->height()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setZoomSpot(const QPointF& p) {
	_zoomSpot = p;

	if ( _zoomSpot.x() < 0 ) _zoomSpot.setX(0);
	if ( _zoomSpot.x() > 1 ) _zoomSpot.setX(1);

	if ( _zoomSpot.y() < 0 ) _zoomSpot.setY(0);
	if ( _zoomSpot.y() > 1 ) _zoomSpot.setY(1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPointF RecordView::zoomSpot() const {
	return _zoomSpot;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setZoomRectFromGlobal(const QRect &rect) {
	QPoint p0 = _scrollArea->mapFromGlobal(rect.topLeft());
	QPoint p1 = _scrollArea->mapFromGlobal(rect.bottomRight());

	QPointF p0f = QPointF((float)(p0.x() - _labelWidth - _horizontalSpacing) /
	                      (float)(_scrollArea->viewport()->width() - _labelWidth - _horizontalSpacing),
	                      (float)p0.y() / (float)_scrollArea->height());

	QPointF p1f = QPointF((float)(p1.x() - _labelWidth - _horizontalSpacing) /
	                      (float)(_scrollArea->viewport()->width() - _labelWidth - _horizontalSpacing),
	                      (float)p1.y() / (float)_scrollArea->height());

	setZoomRect(QRectF(p0f.x(), p0f.y(), p1f.x()-p0f.x(),p1f.y()-p0f.y()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setZoomRect(const QRectF &rect) {
	QRectF r = rect & QRectF(0,0,1,1);

	if ( !r.isValid() ) return;

	setUpdatesEnabled(false);

	/*
	QPointF tmpZS(_zoomSpot);

	setZoomSpot(r.center());
	horizontalZoom(1.0f / r.width());
	verticalZoom(1.0f / r.height());
	*/

	int w = _scrollArea->viewport()->width() - _labelWidth - _horizontalSpacing;

	float timeLength = w/_timeScale;
	float newTimeLength = r.width() * timeLength;

	float newTMin = _tmin + timeLength * r.left();
	float pps = w / newTimeLength;

	if ( pps > 100000)
		pps = 100000;
	else if ( pps < 0.00001 )
		pps = 0.00001;

	setTimeRange(newTMin, newTMin + newTimeLength);
	setScale(pps);

	int h  = rowHeight();
	int h2 = (int)ceil(h / r.height());

	if ( h2 < _defaultRowHeight ) h2=_defaultRowHeight;
	if ( h2 > _scrollArea->viewport()->height() )
		h2 = _scrollArea->viewport()->height();

	int oldHeight = _scrollArea->widget()->height();
	int posY = (int)(r.center().y()*_scrollArea->viewport()->height());
	int absolutePos = posY + _scrollArea->verticalScrollBar()->sliderPosition();

	_minRowHeight = h2;
	if ( (_maxRowHeight > 0) && _maxRowHeight < h2 )
		_maxRowHeight = -1;

	setRowHeight(h2);

	double newAbsolutePos = ((double)absolutePos / (double)oldHeight) * _scrollArea->widget()->height();

	_scrollArea->verticalScrollBar()->setSliderPosition((int)(newAbsolutePos - _scrollArea->viewport()->height()/2));

	setUpdatesEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::zoom(float factor) {
	setUpdatesEnabled(false);
	int h = rowHeight();
	verticalZoom(factor);
	horizontalZoom((float)rowHeight()/(float)h);
	setUpdatesEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::scaleAmplitudesUp() {
	if ( !currentItem() ) return;

	float amplScale = currentItem()->widget()->amplScale();
	if ( amplScale == 0.0 )
		amplScale = 1.0;

	currentItem()->widget()->setAmplScale(amplScale*_zoomFactor);

	emit amplScaleChanged(amplScale*_zoomFactor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::scaleAmplitudesDown() {
	if ( !currentItem() ) return;

	float amplScale = currentItem()->widget()->amplScale();
	if ( amplScale == 0.0 )
		amplScale = 1.0;

	currentItem()->widget()->setAmplScale(amplScale/_zoomFactor);

	emit amplScaleChanged(amplScale/_zoomFactor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::scaleVisibleAmplitudes() {
	foreach (RecordViewItem* item, _items)
		item->widget()->setNormalizationWindow(item->widget()->visibleTimeWindow());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::scaleAllRecords() {
	Core::TimeWindow tw;

	foreach (RecordViewItem* item, _items)
		item->widget()->setNormalizationWindow(tw);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::sortByText(int item) {
	list< pair<QString, RecordViewItem*> > distlist;

	foreach (RecordViewItem* rvItem, _items) {
		if (rvItem->label()->itemCount() <= item)
			return;

		distlist.push_back(
			pair<QString, RecordViewItem*>(rvItem->label()->text(item), rvItem) );
	}

	sortRows(distlist);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::sortByText(int row1, int row2) {
	list< pair<pair<QString, QString>, RecordViewItem*> > distlist;

	foreach (RecordViewItem* rvItem, _items) {
		if (rvItem->label()->itemCount() <= row1 || rvItem->label()->itemCount() <= row2)
			return;

		distlist.push_back(
			pair<pair<QString, QString>, RecordViewItem*>(
				pair<QString, QString>(rvItem->label()->text(row1), rvItem->label()->text(row2)), rvItem));
	}

	sortRows(distlist);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::sortByValue(int column) {
	list< pair<double, RecordViewItem*> > distlist;

	foreach (RecordViewItem* item, _items) {
		if (item->columnCount() <= column)
			return;

		distlist.push_back(
			pair<double, RecordViewItem*>(item->value(column), item) );
	}

	sortRows(distlist);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::sortByValue(int column1, int column2) {
	list< pair<pair<double, double>, RecordViewItem*> > distlist;

	foreach (RecordViewItem* item, _items) {
		if ( item->columnCount() <= column1 || item->columnCount() <= column2 )
			return;

		distlist.push_back(
			pair<pair<double, double>, RecordViewItem*>(pair<double, double>(item->value(column1), item->value(column2)), item) );
	}

	sortRows(distlist);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::sortByTextAndValue(int item, int column) {
	list< pair<pair<QString, double>, RecordViewItem*> > distlist;

	foreach (RecordViewItem* rvItem, _items) {
		if (rvItem->label()->itemCount() <= item)
			return;

		distlist.push_back(
			pair<pair<QString, double>, RecordViewItem*>(pair<QString, double>(rvItem->label()->text(item), rvItem->value(column)), rvItem));
	}

	sortRows(distlist);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::sortByData() {
	list< pair<string, RecordViewItem*> > distlist;

	foreach (RecordViewItem* item, _items) {
		distlist.push_back(
			pair<string, RecordViewItem*>(item->data().toString().toStdString(), item) );
	}

	sortRows(distlist);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordView::findByText(int row, QRegExp &regexp, int startRow) const {
	for ( int i = startRow; i < _rows.size(); ++i ) {
		RecordViewItem* rvItem = _rows[i];

		if (rvItem->label()->itemCount() <= row)
			continue;

		if ( regexp.exactMatch(rvItem->label()->text(row)) )
			return i;
	}

	return -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::sortByMarkerTime(const QString& markerText) {
	list< pair<double, RecordViewItem*> > distlist;

	foreach (RecordViewItem* item, _items) {
		RecordWidget* w = item->widget();
		RecordMarker* m = w->marker(markerText);
		if ( m == NULL )
			distlist.push_back(
				pair<double, RecordViewItem*>(std::numeric_limits<double>::infinity(), item));
		else
			distlist.push_back(
				pair<double, RecordViewItem*>((double)m->correctedTime(), item));
	}

	sortRows(distlist);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename T>
void RecordView::sortRows(list< pair<T, RecordViewItem*> >& l) {
	l.sort();

	int row = 0;
	typename list< pair<T, RecordViewItem*> >::iterator it;
	for (it = l.begin(); it != l.end(); ++it)
	{
		RecordViewItem *item = (*it).second;

		item->_row = row;
		_rows[row] = item;
		++row;
	}

	layoutRows();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setAlignment(const Seiscomp::Core::Time& time) {
	_alignment = time;
	foreach (RecordViewItem* item, _items) {
		RecordWidget* w = item->widget();
		w->setAlignment(time);
	}
	_timeScaleWidget->setAlignment(time);
	_timeScaleWidget->update();

	emit alignmentChanged(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::alignOnMarker(const QString& text) {
	foreach (RecordViewItem* item, _items) {
		RecordWidget* w = item->widget();
		w->alignOnMarker(text);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setCursorText(const QString& text) {
	foreach (RecordViewItem* item, _items) {
		RecordWidget* w = item->widget();
		w->setCursorText(text);
	}

	emit cursorTextChanged(text);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setItemSelected(RecordViewItem* item, bool select) {
	if ( _selectionMode == NoSelection ) return;

	if ( item && (item->parent() != _scrollArea->widget()) ) return;

	if ( select ) {
		if ( _selectedItems.contains(item) ) {
			//std::cerr << "selectItem: already selected" << std::endl;
			return;
		}

		if ( _selectionMode == SingleSelection ) {
			foreach ( RecordViewItem* item, _selectedItems )
				item->setSelected(false);

			_selectedItems.clear();
		}

		item->setSelected(true);
		_selectedItems.insert(item);
	}
	else {
		if ( !_selectedItems.contains(item) ) return;

		item->setSelected(false);
		_selectedItems.remove(item);
	}

	emit selectionChanged();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::deselectAllItems() {
	if ( _selectedItems.isEmpty() ) return;

	foreach (RecordViewItem* item, _selectedItems)
		item->setSelected(false);

	_selectedItems.clear();
	_currentItem = NULL;
	emit selectionChanged();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setCurrentItem(RecordViewItem* item) {
	if ( item == _currentItem ) return;

	RecordViewItem *last = _currentItem;
	_currentItem = item;

	emit currentItemChanged(_currentItem, last);

	if ( item && _selectionMode == SingleSelection )
		setItemSelected(item, true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setDefaultDisplay() {
	if ( rowCount() == 0 ) return;

	int w = _scrollArea->viewport()->width();
	int h = _scrollArea->viewport()->height();

	double pos = 0;

	if ( _scrollArea->widget()->height() > 0 )
		pos = double(_scrollArea->verticalScrollBar()->sliderPosition() + h/2) / (double)_scrollArea->widget()->height();

	_minRowHeight = _defaultRowHeight;
	_maxRowHeight = -1;

	scaleAllRecords();

	setTimeRange(-900., 0);
	setScale(w/900.);

	setRowHeight(std::max(h/rowCount(), _minRowHeight));

	h = _scrollArea->viewport()->height();
	_scrollArea->verticalScrollBar()->setSliderPosition((int)(pos * _scrollArea->widget()->height() - h/2));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setZoomEnabled(bool e) {
	static_cast<RecordScrollArea*>(_scrollArea)->enableZooming(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setFilter(Seiscomp::Math::Filtering::InPlaceFilter<float>* filter) {
	if ( _filter )
		delete _filter;

	_filter = filter;

	bool wasActive = _recordUpdateTimer.isActive();
	_recordUpdateTimer.stop();

	if ( _filtering ) {
		int step = 0;
		emit progressStarted();
		foreach (RecordViewItem* item, _items) {
			//item->widget()->setFilterParameters(order, lowerFreq, upperFreq);
			item->widget()->setFilter(_filter);
			emit progressChanged(++step * 100 / rowCount());
		}
		emit progressFinished();
	}
	else {
		foreach (RecordViewItem* item, _items)
			//item->widget()->setFilterParameters(order, lowerFreq, upperFreq);
			item->widget()->setFilter(_filter);
	}

	if ( wasActive )
		_recordUpdateTimer.start();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::setFilterByName(const QString& strFilter) {
	Math::Filtering::InPlaceFilter<float> *f =
		Math::Filtering::InPlaceFilter<float>::Create(strFilter.toStdString());
	
	if ( !f )
		return false;
	
	setFilter(f);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Math::Filtering::InPlaceFilter<float>* RecordView::filter() const {
	return _filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setAbsoluteTimeEnabled(bool enable) {
	if ( _absTimeAction )
		_absTimeAction->setChecked(enable);
	else
		_timeScaleWidget->setAbsoluteTimeEnabled(enable);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::event(QEvent* event) {
	if ( event->type() == QEvent::LayoutRequest ) {
		layoutRows();
		if ( _autoScale )
			scaleContent();
	}

	return QWidget::event(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::showEvent(QShowEvent *) {
	resizeEvent(NULL);
	layoutRows();

	/*
	if ( currentItem() )
		setZoomSpot(currentItem()->widget()->mapToGlobal(currentItem()->widget()->rect().center()));
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::resizeEvent(QResizeEvent *) {
	/*
	foreach (RecordViewItem* item, _items) {
		item->resize(_scrollArea->widget()->width(), rowHeight());
	}
	//_scrollArea->widget()->setFixedWidth(_scrollArea->viewport()->width());
	*/

	if ( _autoScale )
		scaleContent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::dragEnterEvent(QDragEnterEvent *event) {
	if ( event->mimeData()->hasFormat("text/plain") ) {
		event->acceptProposedAction();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::dropEvent(QDropEvent *event) {
	if ( event->mimeData()->hasFormat("text/plain") ) {
		if ( setFilterByName(event->mimeData()->text()) ) {
			enableFilter(true);
			event->acceptProposedAction();
			emit filterChanged(event->mimeData()->text());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::setDefaultActions() {
	QAction* left = new QAction(this);
	left->setText("Scroll left");
	left->setShortcut(Qt::Key_Left);
	addAction(left);
	connect(left, SIGNAL(triggered()), this, SLOT(scrollLeft()));

	QAction* smartLeft = new QAction(this);
	smartLeft->setText("Scroll left (slowly)");
	smartLeft->setShortcut(Qt::Key_Left + Qt::SHIFT);
	addAction(smartLeft);
	connect(smartLeft, SIGNAL(triggered()), this, SLOT(scrollLeftSlowly()));

	QAction* right = new QAction(this);
	right->setText("Scroll right");
	right->setShortcut(Qt::Key_Right);
	addAction(right);
	connect(right, SIGNAL(triggered()), this, SLOT(scrollRight()));

	QAction* smartRight = new QAction(this);
	smartRight->setText("Scroll right (slowly)");
	smartRight->setShortcut(Qt::Key_Right + Qt::SHIFT);
	addAction(smartRight);
	connect(smartRight, SIGNAL(triggered()), this, SLOT(scrollRightSlowly()));

	QAction* up = new QAction(this);
	up->setText("Scroll up");
	up->setShortcut(Qt::Key_Up);
	addAction(up);
	connect(up, SIGNAL(triggered()), this, SLOT(selectPreviousRow()));

	QAction* down = new QAction(this);
	down->setText("Scroll down");
	down->setShortcut(Qt::Key_Down);
	addAction(down);
	connect(down, SIGNAL(triggered()), this, SLOT(selectNextRow()));

	QAction* home = new QAction(this);
	home->setText("Scroll to top");
	home->setShortcut(Qt::Key_Home);
	addAction(home);
	connect(home, SIGNAL(triggered()), this, SLOT(selectFirstRow()));

	QAction* end = new QAction(this);
	end->setText("Scroll to bottom");
	end->setShortcut(Qt::Key_End);
	addAction(end);
	connect(end, SIGNAL(triggered()), this, SLOT(selectLastRow()));

	QAction* separator = new QAction(this);
	separator->setSeparator(true);
	addAction(separator);

	QAction* zoomIn = new QAction(this);
	zoomIn->setText("Zoom in");
	zoomIn->setShortcut(Qt::Key_Plus);
	addAction(zoomIn);
	connect(zoomIn, SIGNAL(triggered()), this, SLOT(zoomIn()));

	QAction* zoomOut = new QAction(this);
	zoomOut->setText("Zoom out");
	zoomOut->setShortcut(Qt::Key_Minus);
	addAction(zoomOut);
	connect(zoomOut, SIGNAL(triggered()), this, SLOT(zoomOut()));

	QAction* hZoomIn = new QAction(this);
	hZoomIn->setText("Horizontal zoom in");
	hZoomIn->setShortcut(QString(">"));
	addAction(hZoomIn);
	connect(hZoomIn, SIGNAL(triggered()), this, SLOT(horizontalZoomIn()));

	QAction* hZoomOut = new QAction(this);
	hZoomOut->setText("Horizontal zoom out");
	hZoomOut->setShortcut(QString("<"));
	addAction(hZoomOut);
	connect(hZoomOut, SIGNAL(triggered()), this, SLOT(horizontalZoomOut()));

	QAction* vZoomIn = new QAction(this);
	vZoomIn->setText("Vertical zoom in");
	vZoomIn->setShortcut(Qt::Key_Y + Qt::SHIFT);
	addAction(vZoomIn);
	connect(vZoomIn, SIGNAL(triggered()), this, SLOT(verticalZoomIn()));

	QAction* vZoomOut = new QAction(this);
	vZoomOut->setText("Vertical zoom out");
	vZoomOut->setShortcut(Qt::Key_Y);
	addAction(vZoomOut);
	connect(vZoomOut, SIGNAL(triggered()), this, SLOT(verticalZoomOut()));

	QAction* amplScaleUp = new QAction(this);
	amplScaleUp->setText("Scale amplitudes up");
	amplScaleUp->setShortcut(Qt::Key_Up + Qt::SHIFT);
	addAction(amplScaleUp);
	connect(amplScaleUp, SIGNAL(triggered()), this, SLOT(scaleAmplitudesUp()));

	QAction* amplScaleDown = new QAction(this);
	amplScaleDown->setText("Scale amplitudes down");
	amplScaleDown->setShortcut(Qt::Key_Down + Qt::SHIFT);
	addAction(amplScaleDown);
	connect(amplScaleDown, SIGNAL(triggered()), this, SLOT(scaleAmplitudesDown()));

	QAction* amplNormalize = new QAction(this);
	amplNormalize->setText("Normalize visible/selected amplitudes");
	amplNormalize->setShortcut(Qt::Key_S);
	addAction(amplNormalize);
	connect(amplNormalize, SIGNAL(triggered()), this, SLOT(scaleVisibleAmplitudes()));

	QAction* defaultView = new QAction(this);
	defaultView->setText("Default view");
	defaultView->setShortcut(Qt::Key_N);
	addAction(defaultView);
	connect(defaultView, SIGNAL(triggered()), this, SLOT(setDefaultDisplay()));

	separator = new QAction(this);
	separator->setSeparator(true);
	addAction(separator);

	_filterAction = new QAction(this);
	_filterAction->setText("Filter");
	_filterAction->setShortcut(Qt::Key_F);
	_filterAction->setCheckable(true);
	addAction(_filterAction);
	connect(_filterAction, SIGNAL(toggled(bool)), this, SLOT(enableFilter(bool)));

	QAction* showBoth = new QAction(this);
	showBoth->setText("Show all");
	showBoth->setShortcut(Qt::Key_R);
	showBoth->setCheckable(true);
	addAction(showBoth);
	connect(showBoth, SIGNAL(toggled(bool)), this, SLOT(showAllRecords(bool)));

	_absTimeAction = new QAction(this);
	_absTimeAction->setText("Show absolute time");
	_absTimeAction->setShortcut(Qt::Key_A);
	_absTimeAction->setCheckable(true);
	_absTimeAction->setChecked(true);
	addAction(_absTimeAction);
	connect(_absTimeAction, SIGNAL(toggled(bool)), _timeScaleWidget, SLOT(setAbsoluteTimeEnabled(bool)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::setDataSource(const QString& streamURL) {
	closeThread();
	_thread = new RecordStreamThread((const char*)streamURL.toAscii());

	connect(_thread, SIGNAL(receivedRecord(Seiscomp::Record*)),
	        this, SLOT(feed(Seiscomp::Record*)));

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::connectToDataSource() {
	if ( _thread ) {
		if ( _thread->isRunning() ) return true;

		if ( !_thread->connect() )
			return false;

		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordView::start() {
	if ( _thread ) {
		_thread->start();
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::stop() {
	if ( _thread )
		_thread->stop(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordStreamThread* RecordView::recordStreamThread() const {
	return _thread;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QWidget* RecordView::infoWidget() const {
	return _timeScaleInfo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TimeScale* RecordView::timeWidget() const {
	return _timeScaleWidget;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::TimeWindow RecordView::coveredTimeRange() const {
	Core::TimeWindow tw;
	foreach ( RecordViewItem *trace, _rows ) {
		int slotCount = trace->widget()->slotCount();
		for ( int i = 0; i < slotCount; ++i ) {
			Seiscomp::RecordSequence *seq = trace->widget()->records(i);
			if ( seq == NULL ) continue;
			tw = tw | seq->timeWindow();
		}
	}

	return tw;
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordView::closeThread() {
	stop();
	if ( _thread ) {
		delete _thread;
		_thread = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
