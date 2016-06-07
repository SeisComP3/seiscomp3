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



#include <QPainter>

#define SEISCOMP_COMPONENT Gui::RecordWidget
#include <seiscomp3/core/genericrecord.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/math/math.h>
#include <seiscomp3/math/filter/butterworth.h>
#include <seiscomp3/gui/core/application.h>

using namespace std;
using namespace Seiscomp;

#include "recordwidget.h"

namespace  sc = Seiscomp::Core;

#define CHCK255(x) ((x)>255?255:((x)<0?0:(x)))

#define CHCK_RANGE                             \
	double diff = _tmax - _tmin;               \
	if ( _tmin < sc::TimeSpan::MinTime ) {     \
		_tmin = sc::TimeSpan::MinTime;         \
		_tmax = _tmin + diff;                  \
	}                                          \
                                               \
	if ( _tmax > sc::TimeSpan::MaxTime ) {     \
		_tmax = sc::TimeSpan::MaxTime;         \
		_tmin = _tmax - diff;                  \
		if ( _tmin < sc::TimeSpan::MinTime ) { \
			_tmin = sc::TimeSpan::MinTime;     \
			diff = _tmax - _tmin;              \
			_pixelPerSecond = width() / diff;  \
		}                                      \
	}                                          \

namespace {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool minmax(const ::RecordSequence *seq, const Core::TimeWindow &tw,
            float &ofs, float &min, float &max, bool globalOffset = false,
            const Core::TimeWindow &ofsTw = Core::TimeWindow()) {
	ofs = 0;
	int sampleCount = 0;
	int offsetSampleCount = 0;
	bool isFirst = true;
	RecordSequence::const_iterator it = seq->begin();
	min = max = 0;

	for (; it != seq->end(); ++it) {
		RecordCPtr rec = (*it);
		int imin = 0, imax = 0;
		int ns = rec->sampleCount();
		if ( ns == 0 || rec->data() == NULL ) continue;

		FloatArray *arr = (FloatArray*)(rec->data());

		if ( globalOffset ) {
			for ( int i = 0; i < ns; ++i )
				ofs += (*arr)[i];
			offsetSampleCount += ns;
		}

		if ( tw ) { // limit search for min/max to specified time window
			try {
				const Core::TimeWindow &rtw = rec->timeWindow();

				if ( tw.overlaps(rtw) ) {
					double fs = rec->samplingFrequency();
					double dt = tw.startTime() - rec->startTime();
					if(dt>0)
						imin = int(dt*fs);

					dt = rec->endTime() - tw.endTime();
					imax = ns;
					if(dt>0)
						imax -= int(dt*fs);
				}
				else
					continue;
			}
			catch ( ... ) {
				continue;
			}
		}
		else    // no time window specified -> search over whole record
			imax = ns;

		const float *f = (float*)arr->data();

		sampleCount += imax-imin;

		float xmin, xmax;
		::minmax(ns, f, imin, imax, &xmin, &xmax);

		if ( !globalOffset ) {
			if ( ofsTw ) {
				try {
					const Core::TimeWindow &rtw = rec->timeWindow();

					if ( ofsTw.overlaps(rtw) ) {
						double fs = rec->samplingFrequency();
						double dt = ofsTw.startTime() - rec->startTime();
						if(dt>0)
							imin = int(dt*fs);
						else
							imin = 0;

						dt = rec->endTime() - ofsTw.endTime();
						imax = ns;
						if(dt>0)
							imax -= int(dt*fs);

						for ( int i = imin; i < imax; ++i )
							ofs += (*arr)[i];
						offsetSampleCount = sampleCount;
					}
				}
				catch ( ... ) {
				}
			}
			else {
				for ( int i = imin; i < imax; ++i )
					ofs += (*arr)[i];
				offsetSampleCount = sampleCount;
			}
		}

		if( min==max && isFirst ) {
			min = xmin;
			max = xmax;
			isFirst = false;
		}
		else {
			if (xmin<min) min = xmin;
			if (xmax>max) max = xmax;
		}
	}

	ofs /= (offsetSampleCount?offsetSampleCount:1);
	return sampleCount > 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ostream &operator << (ostream &os, const Core::Time &t)
{
            os << t.toString("%F %T.%f").substr(0,23);
	            return os;
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace Seiscomp {
namespace Gui {


RecordMarker::RecordMarker(RecordWidget *parent,
                           const Seiscomp::Core::Time& pos,
                           Qt::Alignment alignment)
: _parent(NULL), _time(pos), _correctedTime(pos), _visible(true),
  _moveable(false), _moveCopy(false), _enabled(true), _id(-1), _alignment(alignment) {
	if ( parent )
		parent->addMarker(this);

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker::RecordMarker(RecordWidget *parent,
                           const Seiscomp::Core::Time& pos,
                           const QString& text,
                           Qt::Alignment alignment)
: _parent(NULL), _time(pos), _correctedTime(pos), _text(text), _visible(true),
  _moveable(false), _moveCopy(false), _enabled(true), _id(-1), _alignment(alignment) {
	if ( parent )
		parent->addMarker(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker::RecordMarker(RecordWidget *parent, const RecordMarker& m)
 : _parent(NULL),
   _color(m._color),
   _modifierColor(m._modifierColor),
   _time(m._time),
   _correctedTime(m._correctedTime),
   _text(m._text),
   _description(m._description),
   _visible(m._visible),
   _moveable(m._moveable),
   _moveCopy(m._moveCopy),
   _enabled(m._enabled),
   _id(m._id),
   _alignment(m._alignment),
   _data(m._data) {
	if ( parent )
		parent->addMarker(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker::~RecordMarker() {
	if ( _parent )
		_parent->takeMarker(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setParent(RecordWidget* p) {
	_parent = p;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget* RecordMarker::parent() const {
	return _parent;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setCorrectedTime(const Seiscomp::Core::Time& t) {
	if ( !_moveable ) return;
	_correctedTime = t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setColor(QColor c) {
	_color = c;
	_modifierColor = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QColor RecordMarker::color() const {
	return _color;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setAlignment(Qt::Alignment al) {
	_alignment = al;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Qt::Alignment RecordMarker::alignment() const {
	return _alignment;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setModifiedColor(QColor c) {
	_modifierColor = c;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QColor RecordMarker::modifiedColor() const {
	return _modifierColor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Seiscomp::Core::Time& RecordMarker::time() const {
	return _time;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Seiscomp::Core::Time& RecordMarker::correctedTime() const {
	return _correctedTime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setText(const QString &t) {
	_text = t;
	_aliases.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& RecordMarker::text() const {
	return _text;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::addAlias(const QString &alias) {
	_aliases.append(alias);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setDescription(const QString &desc) {
	_description = desc;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordMarker::matches(const QString &text) const {
	if ( _text == text ) return true;
	return _aliases.contains(text);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& RecordMarker::description() const {
	return _description;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& RecordMarker::renderText() const {
	return _description.isEmpty()?_text:_description;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setVisible(bool visible) {
	if ( _visible == visible ) return;
	_visible = visible;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordMarker::isVisible() const {
	return _visible;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordMarker::isHidden() const {
	return !_visible;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setMovable(bool enable) {
	_moveable = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordMarker::isMovable() const {
	return _moveable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setEnabled(bool enable) {
	_enabled = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordMarker::isEnabled() const {
	return _enabled && (_parent?_parent->isEnabled():true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setMoveCopy(bool enable) {
	_moveCopy = enable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordMarker::isMoveCopyEnabled() const {
	return _moveCopy;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setData(const QVariant& data) {
	_data = data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant RecordMarker::data() const {
	return _data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::setId(int id) {
	_id = id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordMarker::id() const {
	return _id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordMarker::isModified() const {
	return _time != _correctedTime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::reset() {
	_correctedTime = _time;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::apply() {
	_time = _correctedTime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::update() {
	if ( _parent )
		_parent->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker *RecordMarker::copy() {
	return new RecordMarker(NULL, *this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordMarker::draw(QPainter &painter, RecordWidget *,
                        int x, int y1, int y2,
                        QColor color, qreal lineWidth) {
	painter.setPen(QPen(color, lineWidth));
	painter.drawLine(x, y1, x, y2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString RecordMarker::toolTip() const {
	return QString();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidgetDecorator::RecordWidgetDecorator(QObject *parent)
: QObject(parent) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
static int StreamCount = 0;
static int RecordWidgetCount = 0;

RecordWidget::Stream::Stream(bool owner) {
	records[0] = records[1] = NULL;
	traces[0].dirty = traces[1].dirty = false;
	traces[0].timingQuality = traces[1].timingQuality = -1;
	traces[0].timingQualityCount = traces[1].timingQualityCount = 0;
	filter = NULL;
	pen = QPen(SCScheme.colors.records.foreground, SCScheme.records.lineWidth);
	antialiasing = SCScheme.records.antiAliasing;
	stepFunction = false;
	hasCustomBackgroundColor = false;
	scale = 1.0;
	ownRawRecords = owner;
	ownFilteredRecords = true;
	visible = true;
	filtering = false;
	optimize = SCScheme.records.optimize;

	++StreamCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget::Stream::~Stream() {
	free();

	--StreamCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::Stream::setDirty() {
	traces[Stream::Raw].reset();
	traces[Stream::Filtered].reset();

	traces[Stream::Raw].dirty = true;
	traces[Stream::Filtered].dirty = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::Stream::free() {
	if ( records[0] != NULL && ownRawRecords ) delete records[0];
	if ( records[1] != NULL && ownFilteredRecords ) delete records[1];
	if ( filter != NULL ) delete filter;

	records[0] = records[1] = NULL;
	filter = NULL;

	traces[0].poly.clear();
	traces[1].poly.clear();

	traces[0].timingQuality = -1;
	traces[0].timingQualityCount = 0;

	traces[1].timingQuality = -1;
	traces[1].timingQualityCount = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget::RecordWidget(QWidget *parent)
: QWidget(parent) {
	init();
	setScale(1, 0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget::RecordWidget(const DataModel::WaveformStreamID& streamID, QWidget *parent)
 : QWidget(parent), _streamID(streamID) {
	init();
	setScale(1, 0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::init() {
	++RecordWidgetCount;

	setAutoFillBackground(true);
	removeCustomBackgroundColor();

	_valuePrecision = SCScheme.precision.traceValues;
	_decorator = NULL;
	_shadowWidget = NULL;
	_shadowWidgetFlags = Raw;
	_markerSourceWidget = NULL;
	_filtering = false;
	_drawMode = Single;
	_clipRows = true;
	_drawOffset = true;
	_drawRecordID = true;

	_currentSlot = _requestedSlot = 0;
	_maxFilterSlot = -1;

	_useFixedAmplitudeRange = false;
	_useMinAmplitudeRange = false;

	// set the widget's background color
	QPalette pal = palette();
	pal.setColor(QPalette::WindowText, Qt::black);
	setPalette(pal);

	_amplScale = 0;
	_tmin = 0;
	_tmax = 0;
	_smin = _smax = 0;
	_active = false;
	_gridSpacing[0] = _gridSpacing[1] = 0;

	_tracePaintOffset = 0;
	_scrollBar = NULL;

	// pick/arrival times
	_alignment = Core::Time(0.);

	_drawRecords = false;
	_showAllRecords = false;
	_showScaledValues = false;
	_autoMaxScale = false;
	_useGlobalOffset = false;

	_activeMarker = NULL;
	_hoveredMarker = NULL;

	_enabled = isEnabled();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget::~RecordWidget() {
	if ( _scrollBar ) delete _scrollBar;

	// Delete all stream pointers
	clearRecords();

	for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
		if ( *it == NULL ) continue;
		delete *it;
	}

	// Clear marker
	while ( !_marker.isEmpty() )
		delete _marker[0];

	--RecordWidgetCount;

	if ( RecordWidgetCount == 0 )
		SEISCOMP_DEBUG("All RecordWidgets deleted, remaining streams = %d", StreamCount);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::clearRecords() {
	for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
		if ( *it == NULL ) continue;
		(*it)->free();
	}

	if ( _shadowWidget ) _shadowWidget->clearRecords();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setEnabled(bool enabled) {
	QWidget::setEnabled(enabled);
	_enabled = enabled;
	/*
	update();
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget::Stream *RecordWidget::getStream(int idx) {
	if ( idx < 0 ) return NULL;
	if ( idx < _streams.size() ) return _streams[idx];

	setSlotCount(idx+1);

	if ( _drawMode == InRows ) emit layoutRequest();

	return _streams[idx];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RecordWidget::Stream *RecordWidget::getStream(int idx) const {
	if ( idx < 0 ) return NULL;
	if ( idx < _streams.size() ) return _streams[idx];
	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setCustomBackgroundColor(QColor c) {
	_hasCustomBackground = true;
	_customBackgroundColor = c;

	for ( int i = 0; i < _streams.size(); ++i )
		setRecordBackgroundColor(i, c);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::removeCustomBackgroundColor() {
	_hasCustomBackground = false;

	for ( int i = 0; i < _streams.size(); ++i )
		removeRecordBackgroundColor(i);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecords(int slot, RecordSequence *s, bool owner) {
	if ( _shadowWidget ) _shadowWidget->setRecords(slot, s, false);

	Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;

	// If the same sequence is set again, make sure that it will
	// not be destroyed by free()
	if ( stream->records[Stream::Raw] == s )
		stream->ownRawRecords = false;

	// Reset filter to forget all old buffered samples
	Math::Filtering::InPlaceFilter<float> *newFilter;
	if ( stream->filter && !(_shadowWidgetFlags & Filtered) )
		newFilter = stream->filter->clone();
	else
		newFilter = NULL;

	// Delete old record sequence
	stream->free();

	stream->records[Stream::Raw] = s;
	stream->ownRawRecords = owner;
	stream->filter = newFilter;

	if ( stream->records[Stream::Raw] ) {
		float quality = -1;
		int count = 0;
		bool success = s->timingQuality(count, quality);
		stream->traces[Stream::Raw].timingQualityCount = success ? count   :  0;
		stream->traces[Stream::Raw].timingQuality      = success ? quality : -1;

		if ( stream->filtering ) createFilter(slot);
		_drawRecords = true;
	}

	changedRecords(slot, s);
	stream->setDirty();

	update();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setFilteredRecords(int slot, RecordSequence *s, bool owner) {
	if ( _shadowWidget ) {
		_shadowWidget->setFilteredRecords(slot, s, owner);
		_shadowWidget->setDirty();
	}

	Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;

	if ( stream->ownFilteredRecords && stream->records[Stream::Filtered] )
		delete stream->records[Stream::Filtered];

	stream->records[Stream::Filtered] = s;
	stream->ownFilteredRecords = owner;
	if ( s ) _drawRecords = true;

	changedRecords(slot, s);
	stream->setDirty();

	update();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::changedRecords(int slot, RecordSequence*) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
bool RecordWidget::setRecordFilter(int slot, const Filter *filter) {
	Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;
	if ( _shadowWidgetFlags & Filtered ) return true;

	if ( stream->filter )
		delete stream->filter;

	if ( filter )
		stream->filter = filter->clone();
	else {
		// Create a default filter
		stream->filter = new Math::Filtering::SelfFilter<float>();
		//SEISCOMP_DEBUG("Create default filter");
	}

	if ( stream->records[Stream::Filtered] && stream->ownFilteredRecords ) {
		delete stream->records[Stream::Filtered];
		stream->records[Stream::Filtered] = NULL;
	}

	if (stream->records[Stream::Raw] && !stream->records[Stream::Raw]->empty()) {
		const Record *rec = stream->records[Stream::Raw]->front().get();
		double fs = rec->samplingFrequency();
		stream->filter->setSamplingFrequency(fs);
		stream->filter->setStartTime(rec->startTime());
		stream->filter->setStreamID(rec->networkCode(), rec->stationCode(),
		                            rec->locationCode(), rec->channelCode());
		filterRecords(stream);
	}

	if ( _shadowWidget ) {
		if ( !(_shadowWidget->_shadowWidgetFlags & Filtered) ) {
			_shadowWidget->setRecordFilter(slot, filter);
		}
		else {
			_shadowWidget->setFilteredRecords(slot,
			                                  stream->records[Stream::Filtered], false);
		}
		_shadowWidget->setDirty();
	}

	setDirty();
	update();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordScale(int slot, double scale) {
	Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;

	stream->scale = scale;

	if ( _shadowWidget ) _shadowWidget->setRecordScale(slot, scale);

	update();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::isRecordVisible(int slot) {
	Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;

	return stream->visible;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordVisible(int slot, bool visible) {
	Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;

	stream->visible = visible;

	if ( _shadowWidget ) _shadowWidget->setRecordVisible(slot, visible);

	if ( _drawMode != Single ) setDirty();
	update();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordID(int slot, const QString &id) {
	Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;

	stream->id = id;

	if ( _shadowWidget )
		_shadowWidget->setRecordID(slot, id);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordColor(int slot, QColor c) {
	return setRecordPen(slot, c);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordPen(int slot, const QPen &pen) {
	Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;

	stream->pen = pen;
	stream->setDirty();
	update();

	if ( _shadowWidget )
		_shadowWidget->setRecordPen(slot, pen);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordAntialiasing(int slot, bool antialiasing) {
	Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;

	stream->antialiasing = antialiasing;
	update();

	if ( _shadowWidget )
		_shadowWidget->setRecordAntialiasing(slot, antialiasing);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordOptimization(int slot, bool enable) {
	Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;

	if ( stream->optimize == enable ) return true;

	stream->optimize = enable;
	stream->setDirty();
	update();

	if ( _shadowWidget )
		_shadowWidget->setRecordOptimization(slot, enable);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordStepFunction(int slot, bool enable) {
	Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;

	if ( stream->stepFunction == enable ) return true;

	stream->stepFunction = enable;
	stream->setDirty();
	update();

	if ( _shadowWidget )
		_shadowWidget->setRecordStepFunction(slot, enable);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordBackgroundColor(int slot, QColor c) {
	Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;

	stream->customBackgroundColor = c;
	stream->hasCustomBackgroundColor = true;
	update();

	if ( _shadowWidget )
		_shadowWidget->setRecordBackgroundColor(slot, c);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::removeRecordBackgroundColor(int slot) {
	Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;

	stream->hasCustomBackgroundColor = false;
	update();

	if ( _shadowWidget )
		_shadowWidget->removeRecordBackgroundColor(slot);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::setRecordUserData(int slot, QVariant data) {
	Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;

	stream->userData = data;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString RecordWidget::recordID(int slot) const {
	const Stream *stream = getStream(slot);
	if ( stream == NULL ) return QString();
	return stream->id;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::recordStepFunction(int slot) const {
	const Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;
	return stream->stepFunction;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QColor RecordWidget::recordColor(int slot) const {
	const Stream *stream = getStream(slot);
	if ( stream == NULL ) return QColor();
	return stream->pen.color();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RecordWidget::Trace *RecordWidget::traceInfo(int slot, bool filtered) const {
	const Stream *stream = getStream(slot);
	if ( stream == NULL ) return NULL;
	return &stream->traces[filtered?Stream::Filtered:Stream::Raw];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double *RecordWidget::recordScale(int slot) const {
	const Stream *stream = getStream(slot);
	if ( stream == NULL ) return NULL;
	return &stream->scale;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant RecordWidget::recordUserData(int slot) {
	Stream *stream = getStream(slot);
	if ( stream == NULL ) return QVariant();
	return stream->userData;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence *RecordWidget::createRecords(int slot, bool owner) {
	if ( _streams.empty() ) return NULL;

	for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
		Stream *s = *it;
		if ( s->records[Stream::Raw] != NULL ) {
			Stream *ns = getStream(slot);
			if ( ns == NULL ) return NULL;

			if ( ns->records[Stream::Raw] ) return NULL;

			ns->free();

			RecordSequence *seq = s->records[Stream::Raw]->clone();
			ns->records[Stream::Raw] = seq;
			ns->ownRawRecords = owner;

			setRecordFilter(slot, s->filter);

			if ( _shadowWidget )
				_shadowWidget->setRecords(slot, seq, false);

			if ( s->filtering )
				createFilter(slot);

			return seq;
		}
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RecordWidget::Filter *RecordWidget::recordFilter(int slot) const {
	if ( slot < 0 || slot >= _streams.size() ) return NULL;
	return _streams[slot]->filter;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::setCurrentRecords(int slot) {
	_currentSlot = slot;

	update();

	if ( _shadowWidget ) _shadowWidget->setCurrentRecords(slot);

	return _currentSlot;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::currentRecords() const {
	return _currentSlot;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setDrawMode(DrawMode mode) {
	if ( mode == _drawMode ) return;

	// Request a new layout when the mode changes in terms of needed
	// widget height
	if ( (_drawMode == InRows && mode != InRows) ||
	     (_drawMode != InRows && mode == InRows) ) {
		_drawMode = mode;
		emit layoutRequest();
	}
	else
		_drawMode = mode;

	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordWidget::DrawMode RecordWidget::drawMode() const {
	return _drawMode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setDrawOffset(bool f) {
	_drawOffset = f;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setDrawRecordID(bool f) {
	_drawRecordID = f;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setValuePrecision(int p) {
	_valuePrecision = p;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setClippingEnabled(bool f) {
	_clipRows = f;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::isClippingEnabled() const {
	return _clipRows;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setShadowWidget(RecordWidget *shadow, bool copyMarker, int flags) {
	if ( _shadowWidget ) _shadowWidget->_shadowWidgetFlags = 0;

	_shadowWidget = shadow;
	if ( _shadowWidget == NULL ) return;

	//_shadowWidget->clearRecords();
	if ( copyMarker )
		_shadowWidget->clearMarker();

	_shadowWidget->setSlotCount(slotCount());
	_shadowWidget->_shadowWidgetFlags = flags;

	if ( flags & Raw ) {
		for ( int i = 0; i < slotCount(); ++i ) {
			_shadowWidget->setRecords(i, _streams[i]->records[Stream::Raw], false);
		}
	}

	if ( flags & Filtered ) {
		for ( int i = 0; i < slotCount(); ++i ) {
			_shadowWidget->setFilteredRecords(i, _streams[i]->records[Stream::Filtered], false);
		}
	}

	for ( int i = 0; i < slotCount(); ++i ) {
		_shadowWidget->setRecordScale(i, _streams[i]->scale);
		_shadowWidget->setRecordPen(i, _streams[i]->pen);
		_shadowWidget->setRecordAntialiasing(i, _streams[i]->antialiasing);
		_shadowWidget->setRecordID(i, _streams[i]->id);
		_shadowWidget->setRecordVisible(i, _streams[i]->visible);

		if ( _streams[i]->hasCustomBackgroundColor )
			_shadowWidget->setRecordBackgroundColor(i, _streams[i]->customBackgroundColor);
		else
			_shadowWidget->removeRecordBackgroundColor(i);
	}

	if ( copyMarker ) {
		foreach ( RecordMarker *m, _marker )
			_shadowWidget->addMarker(m->copy());
	}

	_shadowWidget->_hoveredMarker = NULL;
	_shadowWidget->setCurrentRecords(_currentSlot);
	_shadowWidget->_streamID = _streamID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setMarkerSourceWidget(RecordWidget *source) {
	_markerSourceWidget = source;

	_hoveredMarker = markerAt(mapFromGlobal(QCursor::pos()),false,4);
	if ( _hoveredMarker ) {
		setToolTip(_hoveredMarker?_hoveredMarker->toolTip():QString());
		QToolTip::showText(QCursor::pos(), toolTip());
	}

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setDecorator(RecordWidgetDecorator *decorator) {
	_decorator = decorator;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence *RecordWidget::records() const {
	return _streams.empty()?NULL:_streams[0]->records[Stream::Raw];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence *RecordWidget::records(int slot) const {
	if ( slot < 0 || slot >= _streams.size() ) return NULL;
	return _streams[slot]->records[Stream::Raw];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence *RecordWidget::filteredRecords(int slot) const {
	if ( slot < 0 || slot >= _streams.size() ) return NULL;
	return _streams[slot]->records[Stream::Filtered];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordSequence* RecordWidget::takeRecords(int slot) {
	if ( slot < 0 || slot >= _streams.size() ) return NULL;

	RecordSequence *seq = _streams[slot]->records[Stream::Raw];
	_streams[slot]->records[Stream::Raw] = NULL;
	delete _streams[slot];
	_streams.remove(slot);

	if ( _drawMode == InRows ) emit layoutRequest();

	if ( _shadowWidget ) _shadowWidget->takeRecords(slot);

	return seq;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setTimeScale (double t) {
	_pixelPerSecond = t;
	_tmax = _tmin + (_pixelPerSecond > 0 && width()?width()/_pixelPerSecond:0);

	CHCK_RANGE
	if ( _autoMaxScale )
		setNormalizationWindow(visibleTimeWindow());
	else
		setDirty();

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setAmplScale (float a) {
	_amplScale = a;

	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::scroll(int v) {
	_tracePaintOffset = -v;
	//std::cout << "scroll::offset = " << _tracePaintOffset << std::endl;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setScale (double t, float a) {
	_pixelPerSecond = t;
	_tmax = _tmin + (_pixelPerSecond > 0 && width()?width()/_pixelPerSecond:0);

	CHCK_RANGE
	setAmplScale(a);

	if ( _autoMaxScale )
		setNormalizationWindow(visibleTimeWindow());
	else
		setDirty();

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setTimeRange (double t1, double t2) {
	_tmin = t1;
	_tmax = _tmin + (_pixelPerSecond > 0 && width()?width()/_pixelPerSecond:0);

	CHCK_RANGE
	if ( _autoMaxScale )
		setNormalizationWindow(visibleTimeWindow());

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setAmplRange (double a1, double a2) {
	_useFixedAmplitudeRange = true;
	_useMinAmplitudeRange = false;
	_amplitudeRange[0] = float(a1);
	_amplitudeRange[1] = float(a2);
	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setMinimumAmplRange( double a1, double a2) {
	_useFixedAmplitudeRange = false;
	_useMinAmplitudeRange = true;
	_amplitudeRange[0] = float(a1);
	_amplitudeRange[1] = float(a2);
	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setAmplAutoScaleEnabled(bool enabled) {
	if ( _useFixedAmplitudeRange == !enabled ) return;
	_useFixedAmplitudeRange = !enabled;
	setDirty();
	update();
	
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::showTimeRange(double t1, double t2) {
	setTimeRange(t1, t2);
	setScale(width()/(t2-t1));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::translate(double dt) {
	setTimeRange(_tmin + dt, _tmax + dt);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time RecordWidget::leftTime() const {
	return _alignment + Core::TimeSpan(_tmin);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time RecordWidget::rightTime() const {
	return _alignment + Core::TimeSpan(_tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::TimeWindow RecordWidget::visibleTimeWindow() const {
	return Core::TimeWindow(leftTime(), rightTime());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::TimeWindow RecordWidget::selectedTimeWindow() const {
	return Core::TimeWindow(_alignment + Core::TimeSpan(_smin),
	                        _alignment + Core::TimeSpan(_smax));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Seiscomp::Core::TimeWindow & RecordWidget::normalizationWindow() const {
	return _normalizationWindow;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::prepareRecords(Stream *s) {
	Trace *trace = &s->traces[Stream::Raw];
	if ( s->records[Stream::Raw] && (!s->filtering || _showAllRecords) ) {
		trace->visible = minmax(s->records[Stream::Raw], _normalizationWindow,
		                        trace->offset, trace->amplMin, trace->amplMax,
		                        _useGlobalOffset, _offsetWindow);
		trace->absMax = std::max(std::abs(trace->offset-trace->amplMin),
		                         std::abs(trace->offset-trace->amplMax));
	}
	else
		trace->visible = false;

	trace = &s->traces[Stream::Filtered];
	if ( s->records[Stream::Filtered] && (s->filtering || _showAllRecords) ) {
		trace->visible = minmax(s->records[Stream::Filtered], _normalizationWindow,
		                        trace->offset, trace->amplMin, trace->amplMax,
		                        _useGlobalOffset, _offsetWindow);
		trace->absMax = std::max(std::abs(trace->offset-trace->amplMin),
		                         std::abs(trace->offset-trace->amplMax));
	}
	else
		trace->visible = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// Draw the seismogram(s) internally into a RecordPolyline.
// to be called either by resize events or if new records are assigned
void RecordWidget::drawRecords(Stream *s, int slot, int h) {
	//Core::TimeWindow tw(leftTime(), rightTime());
	float magnify = 1; // or 0.2 etc.

	if ( _amplScale > 0 )
		magnify = 1.0/_amplScale;

	int hMargin = s->pen.width()-1;
	if ( hMargin < 0 ) hMargin = 0;

	Trace *trace = &s->traces[Stream::Raw];
	if ( s->records[Stream::Raw] && (!s->filtering || _showAllRecords) ) {
		if ( _useFixedAmplitudeRange ) {
			float offset = (_amplitudeRange[0] + _amplitudeRange[1])/2;
			trace->fyMin = magnify*(_amplitudeRange[0]-offset);
			trace->fyMax = magnify*(_amplitudeRange[1]-offset);
			createPolyline(slot, trace->poly, s->records[Stream::Raw], _pixelPerSecond,
			               trace->fyMin, trace->fyMax, offset, h-hMargin, s->optimize);
			trace->fyMin += offset;
			trace->fyMax += offset;
		}
		else if ( _useMinAmplitudeRange ) {
			double minAmpl = std::min(trace->amplMin, _amplitudeRange[0]);
			double maxAmpl = std::max(trace->amplMax, _amplitudeRange[1]);

			trace->fyMin = magnify*(minAmpl-trace->offset);
			trace->fyMax = magnify*(maxAmpl-trace->offset);
			createPolyline(slot, trace->poly, s->records[Stream::Raw], _pixelPerSecond,
			               trace->fyMin, trace->fyMax, trace->offset, h-hMargin, s->optimize);
			trace->fyMin += trace->offset;
			trace->fyMax += trace->offset;
		}
		else {
			trace->fyMin = magnify*(trace->amplMin-trace->offset);
			trace->fyMax = magnify*(trace->amplMax-trace->offset);
			createPolyline(slot, trace->poly, s->records[Stream::Raw], _pixelPerSecond,
			               trace->fyMin, trace->fyMax, trace->offset, h-hMargin, s->optimize);
			trace->fyMin += trace->offset;
			trace->fyMax += trace->offset;
		}

		trace->yMin = int(trace->poly.baseline() * (1-_amplScale));
		trace->yMax = trace->yMin + int(h * _amplScale);
		trace->dirty = false;
	}
	else {
		trace->fyMin = -1;
		trace->fyMax = 1;
		trace->poly.clear();
	}

	trace = &s->traces[Stream::Filtered];
	if ( s->records[Stream::Filtered] && (s->filtering || _showAllRecords) ) {
		if ( _useFixedAmplitudeRange ) {
			float offset = (_amplitudeRange[0] + _amplitudeRange[1])/2;
			trace->fyMin = magnify*(_amplitudeRange[0]-offset);
			trace->fyMax = magnify*(_amplitudeRange[1]-offset);
			createPolyline(slot, trace->poly, s->records[Stream::Filtered], _pixelPerSecond,
			               trace->fyMin, trace->fyMax, offset, h-hMargin, s->optimize);
			trace->fyMin += offset;
			trace->fyMax += offset;
		}
		else if ( _useMinAmplitudeRange ) {
			double minAmpl = std::min(trace->amplMin, _amplitudeRange[0]);
			double maxAmpl = std::max(trace->amplMax, _amplitudeRange[1]);

			trace->fyMin = magnify*(minAmpl-trace->offset);
			trace->fyMax = magnify*(maxAmpl-trace->offset);
			createPolyline(slot, trace->poly, s->records[Stream::Filtered], _pixelPerSecond,
			               trace->fyMin, trace->fyMax, trace->offset, h-hMargin, s->optimize);
			trace->fyMin += trace->offset;
			trace->fyMax += trace->offset;
		}
		else {
			trace->fyMin = magnify*(trace->amplMin-trace->offset);
			trace->fyMax = magnify*(trace->amplMax-trace->offset);
			createPolyline(slot, trace->poly, s->records[Stream::Filtered], _pixelPerSecond,
			               trace->fyMin, trace->fyMax, trace->offset, h-hMargin, s->optimize);
			trace->fyMin += trace->offset;
			trace->fyMax += trace->offset;
		}

		trace->yMin = int(trace->poly.baseline() * (1-_amplScale));
		trace->yMax = trace->yMin + int(h * _amplScale);
		trace->dirty = false;
	}
	else {
		trace->fyMin = -1;
		trace->fyMax = 1;
		trace->poly.clear();
	}

	if ( _amplScale > 1 ) {
		if ( !_scrollBar ) {
			_scrollBar = new QScrollBar(Qt::Vertical, this);
			connect(_scrollBar, SIGNAL(valueChanged(int)),
			        this, SLOT(scroll(int)));
			_scrollBar->setCursor(Qt::ArrowCursor);
		}
		_scrollBar->setGeometry(QRect(width()-_scrollBar->sizeHint().width(), 0, _scrollBar->sizeHint().width(), height()));
		_scrollBar->show();
	}
	else if ( _scrollBar ) {
		_scrollBar->hide();
		_tracePaintOffset = 0;
	}

	if ( _scrollBar && _scrollBar->isVisible() ) {
		int frontIndex = s->filtering?Stream::Filtered:Stream::Raw;

		int base = s->traces[frontIndex].poly.baseline();
		_scrollBar->setRange(s->traces[frontIndex].yMin, s->traces[frontIndex].yMax - h);
		_scrollBar->setSingleStep(1);
		_scrollBar->setPageStep(h);

		centerLine(base, h);
		_tracePaintOffset = -_scrollBar->value();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::createPolyline(int slot, RecordPolyline &polyline,
                                  RecordSequence const *seq, double pixelPerSecond,
                                  float amplMin, float amplMax, float amplOffset,
                                  int height, bool optimization) {
	if ( _streams[slot]->stepFunction )
		polyline.createSteps(seq, pixelPerSecond, amplMin, amplMax, amplOffset, height);
	else
		polyline.create(seq, pixelPerSecond, amplMin, amplMax, amplOffset, height, NULL, NULL, optimization);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double *RecordWidget::value(int slot, const Seiscomp::Core::Time& t) const {
	static double value;

	if ( slot < 0 || slot >= _streams.size() ) return NULL;

	Stream *s = _streams[slot];

	RecordSequence *rs = s->filtering?s->records[Stream::Filtered]:s->records[Stream::Raw];
	if ( !rs )
		return NULL;

	for ( RecordSequence::const_iterator it = rs->begin();
	      it != rs->end(); ++it ) {
		const Seiscomp::Record *rec = (*it).get();
		if ( t >= rec->startTime() && t <= rec->endTime() ) {
			if ( rec->data() == NULL ) return NULL;

			int pos = int(double(t - rec->startTime()) * rec->samplingFrequency());
			FloatArrayPtr tmp;
			const FloatArray *ar = FloatArray::ConstCast(rec->data());
			if ( !ar ) {
				tmp = (FloatArray*)rec->data()->copy(Array::FLOAT);
				ar = tmp.get();
			}

			if ( !ar ) return NULL;

			if ( ar->size() <= (int)pos )
				return NULL;

			value = (*ar)[pos];
			if ( _showScaledValues )
				value *= s->scale;

			return &value;
		}
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::centerLine(int l, int h) {
	if ( _currentSlot < 0 || _currentSlot >= _streams.size() ) return;

	Stream *s = _streams[_currentSlot];
	int frontIndex = s->filtering?Stream::Filtered:Stream::Raw,
	    logicalY = (s->traces[frontIndex].yMax-s->traces[frontIndex].yMin)*l/h
	               + s->traces[frontIndex].yMin;

	int relY = logicalY - h/2;

	//std::cout << "Center::y = " << relY << std::endl;

	_scrollBar->setValue(relY);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::showAllRecords(bool enable) {
	if ( _showAllRecords == enable ) return;
	_showAllRecords = enable;

	if ( _showAllRecords )
		createFilter();

	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::showScaledValues(bool enable) {
	if ( _showScaledValues == enable ) return;
	_showScaledValues = enable;

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setDirty() {
	_drawRecords = true;
	for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
		Stream *s = *it;
		if ( s == NULL ) continue;
		s->setDirty();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::isActive() const {
	return _active;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::isFilteringEnabled() const {
	return _filtering;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::isGlobalOffsetEnabled() const {
	return _useGlobalOffset;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::event(QEvent *event) {
	/*
	if ( event->type() == QEvent::ToolTip ) {
		QHelpEvent *helpEvent = static_cast<QHelpEvent*>(event);

		RecordMarker *activeMarker = _hoveredMarker;

		if ( activeMarker )
			QToolTip::showText(helpEvent->globalPos(), activeMarker->toolTip());
		else
			QToolTip::showText(helpEvent->globalPos(), QString());
	}
	*/

	return QWidget::event(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::paintEvent(QPaintEvent *event) {
	if ( _pixelPerSecond <= 0 ) return;
	//bool emptyTrace = _poly[frontIndex].isEmpty();

	int w = width(), h = height();

	if ( _scrollBar && _scrollBar->isVisible() )
		w -= _scrollBar->width();

	if ( h == 0 || w == 0 )
		return; // actually this must never happen

	QPainter painter(this);

	QRect rect = event->rect();
	QColor fg;
	QColor bg = palette().color(backgroundRole());
	QColor alignColor;

	/*
	if ( emptyTrace )
		bg = blend(bg, Qt::red, 90);
	*/

	if ( !_enabled ) {
		fg = QColor(160,160,160);
		alignColor = fg;
	}
	else {
		fg = palette().color(foregroundRole());
		alignColor = SCScheme.colors.records.alignment;
	}

	painter.setClipRect(rect);

	if ( hasFocus() ) {
		bg = blend(bg, palette().color(QPalette::Highlight), 90);
		painter.fillRect(rect, bg);
	}

	int sel_xmin = int((_smin-_tmin)*_pixelPerSecond),
	    sel_xmax = int((_smax-_tmin)*_pixelPerSecond),
	    sel_w = sel_xmax - sel_xmin;

	bool emitUpdated = false;

	int slot;

	switch ( _drawMode ) {
		default:
		case Single:
		{
			Stream *stream = NULL;

			if ( (_currentSlot >= _streams.size() || _currentSlot < 0) || !_streams[_currentSlot]->visible )
				stream = NULL;
			else
				stream = _streams[_currentSlot];

			for ( int i = 0; i < _streams.size(); ++i ) {
				Stream *str = _streams[i];
				if ( str == NULL ) continue;

				if ( str == stream ) {
					str->posY = 0;
					str->height = h;
				}
				else {
					str->posY = 0;
					str->height = 0;
				}
			}

			if ( stream != NULL ) {
				if ( stream->hasCustomBackgroundColor )
					painter.fillRect(0,stream->posY,w,stream->height, blend(bg, stream->customBackgroundColor));

				if ( (stream->records[Stream::Filtered] && (stream->filtering || _showAllRecords) && stream->traces[Stream::Filtered].dirty) ||
					(stream->records[Stream::Raw] && (!stream->filtering || _showAllRecords) && stream->traces[Stream::Raw].dirty) ) {
					prepareRecords(stream);
					drawRecords(stream, _currentSlot, stream->height);
					emitUpdated = true;
				}
			}

			break;
		}

		case InRows:
		{
			int visibleSlots = 0;
			int streamHeight = h;
			int streamYOffset = 0;

			for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it )
				if ( (*it)->visible ) ++visibleSlots;

			if ( visibleSlots > 0 ) streamHeight /= visibleSlots;

			slot = 0;
			for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it, ++slot ) {
				Stream *stream = *it;
				if ( !stream->visible ) {
					stream->posY = 0;
					stream->height = 0;
					continue;
				}

				stream->posY = streamYOffset;
				stream->height = streamHeight;

				if ( stream->hasCustomBackgroundColor )
					painter.fillRect(0,stream->posY,w,stream->height, blend(bg, stream->customBackgroundColor));

				if ( (stream->records[Stream::Filtered] && (stream->filtering || _showAllRecords) && stream->traces[Stream::Filtered].dirty) ||
					(stream->records[Stream::Raw] && (!stream->filtering || _showAllRecords) && stream->traces[Stream::Raw].dirty) ) {
					prepareRecords(stream);
					drawRecords(stream, slot, stream->height);
					emitUpdated = true;
				}

				streamYOffset += streamHeight;
			}

			break;
		}

		case Stacked:
		{
			bool isDirty = false;
			bool isFirst[2] = {true,true};
			float minAmpl[2] = {0,0}, maxAmpl[2] = {0,0};
			QColor customBackgroundColor;

			// Two passes: First pass fetches the amplitude range and so on and scales all records appropriate
			for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
				Stream *stream = *it;
				if ( !stream->visible ) {
					stream->posY = 0;
					stream->height = 0;
					continue;
				}

				stream->posY = 0;
				stream->height = h;

				if ( stream->hasCustomBackgroundColor &&
					 !customBackgroundColor.isValid() )
					customBackgroundColor = stream->customBackgroundColor;

				if ( (stream->records[Stream::Filtered] && (stream->filtering || _showAllRecords) && stream->traces[Stream::Filtered].dirty) ||
					(stream->records[Stream::Raw] && (!stream->filtering || _showAllRecords) && stream->traces[Stream::Raw].dirty) ) {
					isDirty = true;
					prepareRecords(stream);
				}

				for ( int i = 0; i < 2; ++i ) {
					if ( stream->traces[i].visible ) {
						if ( isFirst[i] ) {
							minAmpl[i] = stream->traces[i].amplMin;
							maxAmpl[i] = stream->traces[i].amplMax;
							isFirst[i] = false;
						}
						else {
							minAmpl[i] = std::min(minAmpl[i], stream->traces[i].amplMin);
							maxAmpl[i] = std::max(maxAmpl[i], stream->traces[i].amplMax);
						}
					}
				}
			}

			if ( customBackgroundColor.isValid() )
				painter.fillRect(0, 0, w, h, blend(bg, customBackgroundColor));

			if ( !isDirty ) break;

			// Second pass draws all records
			slot = 0;
			for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it, ++slot ) {
				Stream *stream = (*it)->visible?*it:NULL;
				if ( stream == NULL ) continue;

				stream->traces[0].amplMin = minAmpl[0];
				stream->traces[0].amplMax = maxAmpl[0];

				stream->traces[1].amplMin = minAmpl[1];
				stream->traces[1].amplMax = maxAmpl[1];

				drawRecords(stream, slot, stream->height);
				emitUpdated = true;
			}
			break;
		}

		case SameOffset:
		{
			bool isDirty = false;
			bool isFirst[2] = {true,true};
			float minAmpl[2] = {0,0}, maxAmpl[2] = {0,0};
			QColor customBackgroundColor;
			// Two passes: First pass fetches the amplitude range and so on and scales all records appropriate
			for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
				Stream *stream = *it;
				if ( !stream->visible ) {
					stream->posY = 0;
					stream->height = 0;
					continue;
				}

				stream->posY = 0;
				stream->height = h;

				if ( stream->hasCustomBackgroundColor &&
					 !customBackgroundColor.isValid() )
					customBackgroundColor = stream->customBackgroundColor;

				if ( (stream->records[Stream::Filtered] && (stream->filtering || _showAllRecords) && stream->traces[Stream::Filtered].dirty) ||
					(stream->records[Stream::Raw] && (!stream->filtering || _showAllRecords) && stream->traces[Stream::Raw].dirty) ) {
					isDirty = true;
					prepareRecords(stream);
				}

				// i == 0: foreground
				// i == 1: background
				for ( int i = 0; i < 2; ++i ) {
					int j = i ^ (stream->filtering?1:0);
					if ( !stream->traces[j].visible ) continue;

					if ( isFirst[i] ) {
						minAmpl[i] = stream->traces[j].amplMin - stream->traces[j].offset;
						maxAmpl[i] = stream->traces[j].amplMax - stream->traces[j].offset;
						isFirst[i] = false;
					}
					else {
						minAmpl[i] = std::min(minAmpl[i], stream->traces[j].amplMin - stream->traces[j].offset);
						maxAmpl[i] = std::max(maxAmpl[i], stream->traces[j].amplMax - stream->traces[j].offset);
					}
				}
			}

			if ( customBackgroundColor.isValid() )
				painter.fillRect(0,0,w,h,
				                 blend(bg, customBackgroundColor));

			if ( !isDirty ) break;

			// Second pass draws all records
			slot = 0;
			for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it, ++slot ) {
				Stream *stream = (*it)->visible?*it:NULL;
				if ( stream == NULL ) continue;

				for ( int i = 0; i < 2; ++i ) {
					int j = i ^ (stream->filtering?1:0);
					stream->traces[i].amplMin = stream->traces[i].offset + minAmpl[j];
					stream->traces[i].amplMax = stream->traces[i].offset + maxAmpl[j];
				}

				drawRecords(stream, slot, stream->height);
				emitUpdated = true;
			}
			break;
		}
	}

	_drawRecords = false;

	QColor sel = blend(bg, SCScheme.colors.recordView.selectedTraceZoom);
	painter.fillRect(sel_xmin, 0, sel_w, h, sel);

	drawCustomBackground(painter);


	// Draw gaps
	switch ( _drawMode ) {
		default:
		case Stacked:
		case Single:
		{
			Stream *stream = (_currentSlot >= 0 && _currentSlot < _streams.size() && _streams[_currentSlot]->visible) ?
			                 _streams[_currentSlot] : NULL;

			if ( stream ) {
				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				if ( !stream->traces[frontIndex].poly.empty() ) {
					double offset[2] = {0,0};
					int x_tmin[2];

					if ( stream->records[Stream::Raw] )
						offset[0] = _alignment - stream->records[Stream::Raw]->timeWindow().startTime();
					if ( stream->records[Stream::Filtered] )
						offset[1] = _alignment - stream->records[Stream::Filtered]->timeWindow().startTime();

					x_tmin[0] = int(-(offset[0] + _tmin)*_pixelPerSecond);
					x_tmin[1] = int(-(offset[1] + _tmin)*_pixelPerSecond);

					QColor gapColor(blend(bg, SCScheme.colors.records.gaps));
					QColor overlapColor(blend(bg, SCScheme.colors.records.overlaps));
					painter.setPen(fg);
					painter.translate(QPoint(x_tmin[frontIndex], _tracePaintOffset));
					stream->traces[frontIndex].poly.drawGaps(painter, 0, stream->height, gapColor, overlapColor);
					painter.translate(QPoint(-x_tmin[frontIndex], -_tracePaintOffset));
				}
			}
			break;
		}

		case SameOffset:
			break;

		case InRows:
			for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
				Stream *stream = (*it)->visible?*it:NULL;
				if ( stream == NULL ) continue;

				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				if ( !stream->traces[frontIndex].poly.empty() ) {
					double offset[2] = {0,0};
					int x_tmin[2];

					if ( stream->records[Stream::Raw] )
						offset[0] = _alignment - stream->records[Stream::Raw]->timeWindow().startTime();
					if ( stream->records[Stream::Filtered] )
						offset[1] = _alignment - stream->records[Stream::Filtered]->timeWindow().startTime();

					x_tmin[0] = int(-(offset[0] + _tmin)*_pixelPerSecond);
					x_tmin[1] = int(-(offset[1] + _tmin)*_pixelPerSecond);

					QColor gapColor(blend(bg, SCScheme.colors.records.gaps));
					QColor overlapColor(blend(bg, SCScheme.colors.records.overlaps));
					painter.setPen(fg);
					painter.translate(QPoint(x_tmin[frontIndex], _tracePaintOffset + stream->posY));
					stream->traces[frontIndex].poly.drawGaps(painter, 0, stream->height, gapColor, overlapColor);
					painter.translate(QPoint(-x_tmin[frontIndex], -_tracePaintOffset - stream->posY));
				}
			}
			break;
	}

	QColor gridColor[2] = {QColor(192,192,255), QColor(224,225,255)};

	if ( _gridSpacing[0] > 0 && !(_pixelPerSecond <= 0) && !Math::isNaN(_tmin) ) {
		double left = _tmin + _gridOffset;

		//for ( int k = 1; k >= 0; --k ) {
		for ( int k = 0; k < 1; ++k ) {
			painter.setPen(gridColor[k]);

			double correctedLeft = left - fmod(left, (double)_gridSpacing[k]);

			int x = (int)((correctedLeft-left)*_pixelPerSecond);
			while ( x < width() ) {
				painter.drawLine(x,0,x,h);
				correctedLeft += _gridSpacing[k];
				x = (int)((correctedLeft-left)*_pixelPerSecond);
			}
		}
	}

	customPaintTracesBegin(painter);

	bool isAntialiasing = painter.renderHints() & QPainter::Antialiasing;

	// Draw traces
	switch ( _drawMode ) {
		default:
		case Single:
		{
			Stream *stream = (_currentSlot >= 0 && _currentSlot < _streams.size() && _streams[_currentSlot]->visible) ?
			                 _streams[_currentSlot] : NULL;
			if ( stream ) {
				double offset[2] = {0,0};
				int x_tmin[2];

				if ( stream->records[Stream::Raw] )
					offset[0] = _alignment - stream->records[Stream::Raw]->timeWindow().startTime();
				if ( stream->records[Stream::Filtered] )
					offset[1] = _alignment - stream->records[Stream::Filtered]->timeWindow().startTime();

				x_tmin[0] = int(-(offset[0] + _tmin)*_pixelPerSecond);
				x_tmin[1] = int(-(offset[1] + _tmin)*_pixelPerSecond);

				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				if ( !stream->traces[1-frontIndex].poly.empty() && _showAllRecords ) {
					painter.setPen(gridColor[0]);
					painter.translate(QPoint(x_tmin[1-frontIndex], _tracePaintOffset));
					//_trace[1-frontIndex].poly.translate(x_tmin[1-frontIndex], _tracePaintOffset);
					stream->traces[1-frontIndex].poly.draw(painter);
					painter.translate(QPoint(-x_tmin[1-frontIndex], -_tracePaintOffset));
				}

				if ( !stream->traces[frontIndex].poly.empty() ) {
					if ( _drawOffset ) {
						painter.setPen(blend(bg, gridColor[0], 75));
						painter.drawLine(0,_tracePaintOffset+stream->traces[frontIndex].poly.baseline(), w,_tracePaintOffset+stream->traces[frontIndex].poly.baseline());
					}

					if ( stream->antialiasing != isAntialiasing )
						painter.setRenderHint(QPainter::Antialiasing, isAntialiasing = stream->antialiasing);

					int hMargin = stream->pen.width()-1;
					if ( hMargin < 0 ) hMargin = 0;

					painter.setPen(_enabled?stream->pen:fg);
					painter.translate(QPoint(x_tmin[frontIndex], _tracePaintOffset+hMargin));
					stream->traces[frontIndex].poly.draw(painter);
					painter.translate(QPoint(-x_tmin[frontIndex], -_tracePaintOffset-hMargin));
				}
			}
			break;
		}

		case InRows:
			for ( int i = 0; i < _streams.size(); ++i ) {
				Stream *stream = _streams[i]->visible?_streams[i]:NULL;
				if ( stream == NULL ) continue;

				double offset[2] = {0,0};
				int x_tmin[2];

				if ( stream->records[Stream::Raw] )
					offset[0] = _alignment - stream->records[Stream::Raw]->timeWindow().startTime();
				if ( stream->records[Stream::Filtered] )
					offset[1] = _alignment - stream->records[Stream::Filtered]->timeWindow().startTime();

				x_tmin[0] = int(-(offset[0] + _tmin)*_pixelPerSecond);
				x_tmin[1] = int(-(offset[1] + _tmin)*_pixelPerSecond);

				if ( stream->height > 0 ) {
					// Enable clipping to rows if enabled
					if ( _clipRows )
						painter.setClipRect(QRect(0, stream->posY, w, stream->height));
					else
						painter.setClipRect(QRect(0, 0, w, h));

					if ( i == _currentSlot && _streams.size() > 1 ) {
						painter.setBrush(Qt::NoBrush);
						painter.setPen(QPen(palette().color(_enabled?QPalette::Active:QPalette::Disabled, QPalette::Text), 1, Qt::DashLine));
						painter.drawRect(QRect(0, stream->posY, w-1, stream->height-1));
					}
				}

				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				if ( !stream->traces[1-frontIndex].poly.empty() && _showAllRecords ) {
					painter.setPen(gridColor[0]);
					painter.translate(QPoint(x_tmin[1-frontIndex], _tracePaintOffset + stream->posY));
					//_trace[1-frontIndex].poly.translate(x_tmin[1-frontIndex], _tracePaintOffset);
					stream->traces[1-frontIndex].poly.draw(painter);
					painter.translate(QPoint(-x_tmin[1-frontIndex], -_tracePaintOffset - stream->posY));
				}

				if ( !stream->traces[frontIndex].poly.empty() ) {
					if ( _drawOffset ) {
						painter.setPen(blend(bg, gridColor[0], 75));
						painter.drawLine(0,_tracePaintOffset+stream->traces[frontIndex].poly.baseline() + stream->posY, w,_tracePaintOffset+stream->traces[frontIndex].poly.baseline() + stream->posY);
					}

					if ( stream->antialiasing != isAntialiasing )
						painter.setRenderHint(QPainter::Antialiasing, isAntialiasing = stream->antialiasing);

					int hMargin = stream->pen.width()-1;
					if ( hMargin < 0 ) hMargin = 0;

					painter.setPen(_enabled?stream->pen:fg);
					painter.translate(QPoint(x_tmin[frontIndex], _tracePaintOffset + stream->posY + hMargin));
					stream->traces[frontIndex].poly.draw(painter);
					painter.translate(QPoint(-x_tmin[frontIndex], -_tracePaintOffset - stream->posY - hMargin));
				}
			}
			break;

		case Stacked:
		case SameOffset:
			// Draw offset
			if ( _drawOffset ) {
				for ( int i = 0; i < _streams.size(); ++i ) {
					Stream *stream = _streams[i]->visible?_streams[i]:NULL;
					if ( stream == NULL ) continue;

					int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
					if ( !stream->traces[frontIndex].poly.empty() ) {
						painter.setPen(blend(bg, gridColor[0], 75));
						painter.drawLine(0,_tracePaintOffset+stream->traces[frontIndex].poly.baseline(), w,_tracePaintOffset+stream->traces[frontIndex].poly.baseline());
					}
				}
			}

			// Draw backtraces
			if ( _showAllRecords ) {
				for ( int i = 0; i < _streams.size(); ++i ) {
					Stream *stream = _streams[i]->visible?_streams[i]:NULL;
					if ( stream == NULL ) continue;

					double offset[2] = {0,0};
					int x_tmin[2];

					if ( stream->records[Stream::Raw] )
						offset[0] = _alignment - stream->records[Stream::Raw]->timeWindow().startTime();
					if ( stream->records[Stream::Filtered] )
						offset[1] = _alignment - stream->records[Stream::Filtered]->timeWindow().startTime();

					x_tmin[0] = int(-(offset[0] + _tmin)*_pixelPerSecond);
					x_tmin[1] = int(-(offset[1] + _tmin)*_pixelPerSecond);

					int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
					if ( !stream->traces[1-frontIndex].poly.empty() ) {
						painter.setPen(gridColor[0]);
						painter.translate(QPoint(x_tmin[1-frontIndex], _tracePaintOffset));
						stream->traces[1-frontIndex].poly.draw(painter);
						painter.translate(QPoint(-x_tmin[1-frontIndex], -_tracePaintOffset));
					}
				}
			}

			// Draw records
			for ( int i = 0; i < _streams.size(); ++i ) {
				Stream *stream = _streams[i]->visible?_streams[i]:NULL;
				if ( stream == NULL ) continue;

				double offset[2] = {0,0};
				int x_tmin[2];

				if ( stream->records[Stream::Raw] )
					offset[0] = _alignment - stream->records[Stream::Raw]->timeWindow().startTime();
				if ( stream->records[Stream::Filtered] )
					offset[1] = _alignment - stream->records[Stream::Filtered]->timeWindow().startTime();

				x_tmin[0] = int(-(offset[0] + _tmin)*_pixelPerSecond);
				x_tmin[1] = int(-(offset[1] + _tmin)*_pixelPerSecond);

				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				if ( !stream->traces[frontIndex].poly.empty() ) {
					if ( stream->antialiasing != isAntialiasing )
						painter.setRenderHint(QPainter::Antialiasing, isAntialiasing = stream->antialiasing);

					int hMargin = stream->pen.width()-1;
					if ( hMargin < 0 ) hMargin = 0;

					painter.setPen(_enabled?stream->pen:fg);
					painter.translate(QPoint(x_tmin[frontIndex], _tracePaintOffset + hMargin));
					stream->traces[frontIndex].poly.draw(painter);
					painter.translate(QPoint(-x_tmin[frontIndex], -_tracePaintOffset - hMargin));
				}
			}
			break;
	}

	painter.setClipRect(this->rect());

	if ( isAntialiasing )
		painter.setRenderHint(QPainter::Antialiasing, false);

	customPaintTracesEnd(painter);

	painter.setPen(alignColor);
	int x = (int)(-_tmin*_pixelPerSecond);
	painter.drawLine(x, 0, x, h);

	// make the font a bit smaller (for the phase annotations)
	QFont font = painter.font();
	QFontInfo fi(font);
	int fontSize = std::min(h/2, fi.pixelSize());

	font.setPixelSize(fontSize);

	//font.setBold(true);
	painter.setFont(font);

	int markerCanvasOffset = 0;
	int markerCanvasHeight = h;

	QVector<RecordMarker*> *markerList = &_marker;
	RecordMarker *activeMarker = _activeMarker;

	if ( _markerSourceWidget ) {
		markerList = &_markerSourceWidget->_marker;
		activeMarker = _markerSourceWidget->_activeMarker;
	}

	foreach ( RecordMarker* m, *markerList ) {
		if ( m->isHidden() ) continue;

		int startY = markerCanvasOffset, endY = startY + markerCanvasHeight;
		int textY = markerCanvasOffset + fontSize + 1;

		switch ( m->_alignment ) {
			case Qt::AlignTop:
				endY = markerCanvasOffset + markerCanvasHeight*2/4-1;
				break;
			case Qt::AlignBottom:
				startY = markerCanvasOffset + markerCanvasHeight*2/4+1;
				textY = markerCanvasOffset + markerCanvasHeight-2;
				break;
		}

		bool enabled = _enabled && m->isEnabled();

		if ( m->isMoveCopyEnabled() ) {
			x = mapTime(m->time());

			QColor c(enabled?m->color():fg);
			c.setAlpha(64);

			painter.setPen(QPen(c, SCScheme.marker.lineWidth));
			painter.drawLine(x, startY, x, endY);
			//painter.drawRect(textRect.translated(x,textY-3));
			painter.drawText(x+2, textY, m->renderText());
		}

		x = mapTime(m->correctedTime());

		/*
		QColor c(m->color());
		c.setRed(c.red()/2);
		c.setGreen(c.green()/2);
		c.setBlue(c.blue()/2);

		painter.setPen(c);
		painter.drawLine(x+1, 0, x+1, h);
		painter.drawText(x+2+1, fontSize+1, m->renderText());
		*/

		//painter.drawRect(textRect.translated(x,textY-3));
		if ( m->isMovable() && m->isModified() ) {
			painter.setPen(enabled?m->modifiedColor():fg);
			painter.drawText(x+2, textY, m->renderText());
			m->draw(painter, this, x, markerCanvasOffset, markerCanvasOffset + markerCanvasHeight,
			        enabled?m->modifiedColor():fg, SCScheme.marker.lineWidth);
		}
		else {
			painter.setPen(enabled?m->color():fg);
			painter.drawText(x+2, textY, m->renderText());
			m->draw(painter, this, x, startY, endY,
			        enabled?m->color():fg, SCScheme.marker.lineWidth);
		}

		if ( m == _hoveredMarker ) {
			QPen pen = painter.pen();
			painter.setPen(QPen(palette().color(QPalette::Highlight), 1, Qt::DotLine));
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(x-4, startY, 8, endY-startY+1);
			painter.setPen(pen);
		}

		if ( m == activeMarker ) {
			QPen pen = painter.pen();
			painter.setPen(QPen(palette().color(QPalette::Text), 1, Qt::DashLine));
			painter.setBrush(Qt::NoBrush);
			painter.drawRect(x-4, startY, 8, endY-startY+1);
			painter.setPen(pen);
		}
	}

	if ( _active && _enabled && !_cursorText.isEmpty() )
		drawActiveCursor(painter, mapTime(_cursorPos), _currentCursorYPos);

	// Draw labels
	switch ( _drawMode ) {
		default:
		case Single:
		{
			Stream *stream = (_currentSlot >= 0 && _currentSlot < _streams.size() && _streams[_currentSlot]->visible) ?
			                 _streams[_currentSlot] : NULL;

			if ( stream ) {
				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				if ( !stream->traces[frontIndex].poly.empty() ) {
					painter.setPen(fg);
					font.setBold(false);
					painter.setFont(font);

					if ( _drawOffset ) {
						QString str;
						if ( _showScaledValues )
							str.setNum(stream->traces[frontIndex].absMax*stream->scale);
						else
							str.setNum(stream->traces[frontIndex].absMax, 'f', _valuePrecision);

						int rh = 2*painter.fontMetrics().ascent()+4;
						int y = stream->height - rh;
						if  ( y < 0 ) y = 0;

						painter.drawText(0,y, w,rh, Qt::TextSingleLine | Qt::AlignLeft | Qt::AlignTop, str);

						if ( stream->height >= y+rh ) {
							if ( _showScaledValues )
								str.setNum(stream->traces[frontIndex].offset*stream->scale);
							else
								str.setNum(stream->traces[frontIndex].offset, 'f', _valuePrecision);

							painter.drawText(0,y, w,rh, Qt::TextSingleLine | Qt::AlignLeft | Qt::AlignBottom, str);
						}
					}

					if ( !stream->id.isEmpty() && _drawRecordID ) {
						painter.setPen(fg);
						font.setBold(true);
						painter.setFont(font);
						QRect br = painter.fontMetrics().boundingRect(stream->id);
						br.adjust(0,0,4,4);
						//br.moveCenter(QPoint(br.center().x(), streamHeight/2+streamYOffset));
						br.moveTopLeft(QPoint(0,0));
						painter.fillRect(br, bg);
						painter.drawRect(br);
						painter.drawText(br, Qt::AlignCenter, stream->id);
					}

					/*
					// Draw timing quality
					if ( stream->traces[Stream::Raw].timingQuality > 0 ) {
						str.setNum((int)stream->traces[Stream::Raw].timingQuality);
						painter.drawText(0,0, w,streamHeight, Qt::TextSingleLine | Qt::AlignRight | Qt::AlignTop, str);
					}
					*/
				}
			}
			break;
		}

		case InRows:
			for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
				Stream *stream = (*it)->visible?*it:NULL;
				if ( stream == NULL ) continue;

				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				if ( !stream->traces[frontIndex].poly.empty() ) {
					painter.setPen(fg);
					font.setBold(false);
					painter.setFont(font);

					if ( _drawOffset ) {
						QString str;
						if ( _showScaledValues )
							str.setNum(stream->traces[frontIndex].absMax*stream->scale);
						else
							str.setNum(stream->traces[frontIndex].absMax, 'f', _valuePrecision);

						int rh = 2*painter.fontMetrics().ascent()+4;
						int y = stream->posY + stream->height - rh;
						if ( y < stream->posY ) y = stream->posY;

						painter.drawText(0,y, w,rh, Qt::TextSingleLine | Qt::AlignLeft | Qt::AlignTop, str);

						if ( stream->posY+stream->height >= y+rh ) {
							if ( _showScaledValues )
								str.setNum(stream->traces[frontIndex].offset*stream->scale);
							else
								str.setNum(stream->traces[frontIndex].offset, 'f', _valuePrecision);
							painter.drawText(0,y, w,rh, Qt::TextSingleLine | Qt::AlignLeft | Qt::AlignBottom, str);
						}
					}
				}

				if ( !stream->id.isEmpty() && _drawRecordID ) {
					painter.setPen(fg);
					font.setBold(true);
					painter.setFont(font);
					QRect br = painter.fontMetrics().boundingRect(stream->id);
					br.adjust(0,0,4,4);
					//br.moveCenter(QPoint(br.center().x(), streamHeight/2+streamYOffset));
					br.moveTopLeft(QPoint(0,stream->posY));
					painter.fillRect(br, bg);
					painter.drawRect(br);
					painter.drawText(br, Qt::AlignCenter, stream->id);
				}
			}
			break;

		case Stacked:
		case SameOffset:
			{
				float offset = 0;
				int cnt = 0;

				Stream *stream = NULL;
				for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
					Stream* tmpStream = (*it)->visible?*it:NULL;
					if ( tmpStream == NULL ) continue;

					int frontIndex = tmpStream->filtering?Stream::Filtered:Stream::Raw;
					if ( tmpStream->traces[frontIndex].poly.empty() ) continue;
					++cnt;
					offset += tmpStream->traces[frontIndex].offset;

					stream = tmpStream;
				}

				if ( cnt == 0 ) break;

				offset /= cnt;

				//Stream *stream = _streams[0];
				int frontIndex = stream->filtering?Stream::Filtered:Stream::Raw;
				float absMax = std::max(std::abs(stream->traces[frontIndex].offset - stream->traces[frontIndex].amplMin),
				                        std::abs(stream->traces[frontIndex].offset - stream->traces[frontIndex].amplMax));

				painter.setPen(fg);
				font.setBold(false);
				painter.setFont(font);

				if ( _drawOffset ) {
					QString str;
					if ( _showScaledValues )
						str.setNum(absMax*stream->scale);
					else
						str.setNum(absMax, 'f', _valuePrecision);

					int rh = 2*painter.fontMetrics().ascent()+4;
					int y = stream->height - rh;
					if ( y < 0 ) y = 0;

					painter.drawText(0,y, w,rh, Qt::TextSingleLine | Qt::AlignLeft | Qt::AlignTop, str);

					if ( stream->height >= y+rh ) {
						if ( _showScaledValues )
							str.setNum(offset*stream->scale);
						else
							str.setNum(offset, 'f', _valuePrecision);
						painter.drawText(0,y, w,rh, Qt::TextSingleLine | Qt::AlignLeft | Qt::AlignBottom, str);
					}
				}
			}
			break;
	}

	if ( _decorator ) _decorator->drawDecoration(&painter, this);

	if ( emitUpdated ) emit traceUpdated(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::drawActiveCursor(QPainter &painter, int x, int y) {
	int h = height();

	painter.setPen(Qt::black);
	painter.drawLine(x, 0, x, h);
	//painter.drawLine(0, _currentPos.y(), w, _currentPos.y());
	painter.drawText(0,0,x-2,h, Qt::TextSingleLine | Qt::AlignRight | Qt::AlignTop, _cursorText);

	QFont f = painter.font();
	f.setBold(false);
	painter.setFont(f);

	QString time = _cursorPos.toString("%T.%f000000").substr(0,11).c_str();

	painter.drawText(rect(), Qt::TextSingleLine | Qt::AlignRight | Qt::AlignTop, time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::mousePressEvent(QMouseEvent *event) {
	if ( event->button() == Qt::MidButton ) {
		emit clickedOnTime(unmapTime(event->x()));
		return;
	}

	if ( !_enabled ) {
		event->ignore();
		return;
	}

	if ( event->button() == Qt::LeftButton /*|| event->button() == Qt::RightButton*/ )
		setCurrentMarker(markerAt(event->pos(),false,4));

	if ( !_active ) {
		event->ignore();
		return;
	}

	if ( event->button() == Qt::LeftButton ) {
		setCursorPos(event->pos());
		_startDragPos = _cursorPos;
		event->ignore();
		return;
	}
	else if ( event->button() == Qt::RightButton ) {
		RecordMarker* m = nearestMarker(unmapTime(event->x()));
		if ( m )
			setCursorPos(m->correctedTime());

		return;
	}

	event->ignore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::mouseReleaseEvent(QMouseEvent *event) {
	if ( event->button() == Qt::LeftButton ) {
		if ( _startDragPos.valid() ) {
			if ( _startDragPos < _cursorPos )
				emit selectedTimeRange(_startDragPos, _cursorPos);
			else
				emit selectedTimeRange(_cursorPos, _startDragPos);
			_startDragPos = Core::Time();
		}
	}

	event->ignore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::mouseDoubleClickEvent(QMouseEvent *event) {
	if ( !_enabled || !_active ) {
		event->ignore();
		return;
	}

	if ( event->button() == Qt::LeftButton ) {
		float dt = event->x()/_pixelPerSecond+_tmin;
		emit selectedTime(_alignment + Core::TimeSpan(dt));
		return;
	}
	else if ( event->button() == Qt::RightButton ) {
		RecordMarker* m = nearestMarker(unmapTime(event->x()));
		if ( m )
			emit selectedTime(m->correctedTime());
		return;
	}

	event->ignore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::mouseMoveEvent(QMouseEvent *event) {
	if ( hasMouseTracking() ) {
		// Update marker hover state
		RecordMarker *hovered = markerAt(event->pos(),false,4);
		if ( _hoveredMarker != hovered ) {
			_hoveredMarker = hovered;
			setToolTip(_hoveredMarker?_hoveredMarker->toolTip():QString());
			QToolTip::showText(event->globalPos(), toolTip());
			update();
		}

		/*
		setCurrentMarker(markerAt(event->pos(),false,4));

		RecordMarker *activeMarker = _activeMarker;
		if ( _markerSourceWidget )
			activeMarker = _markerSourceWidget->_activeMarker;

		setToolTip(activeMarker?activeMarker->toolTip():QString());
		*/

		if ( _enabled && _active && event->buttons() == Qt::NoButton ) {
			_currentCursorYPos = event->pos().y();
			setCursorPos(event->pos());
		}
	}


	if ( event->buttons() == Qt::LeftButton ) {
		if ( _enabled && _active ) {
			_currentCursorYPos = event->pos().y();
			setCursorPos(event->pos());

			if ( _startDragPos.valid() ) {
				if ( _startDragPos < _cursorPos )
					emit selectedTimeRangeChanged(_startDragPos, _cursorPos);
				else
					emit selectedTimeRangeChanged(_cursorPos, _startDragPos);
			}
		}
		event->ignore();
	}

	/*
	if ( _enabled ) {
		if ( _activeMarker ) {
			Core::Time t;
			t = _alignment +
				Core::TimeSpan((float)event->x()/_pixelPerSecond+_tmin);
			_activeMarker->setCorrectedTime(t);
			update();
		}
		else {
			RecordMarker* marker = markerAt(event->x(), event->y(), true);
			if ( marker )
				setCursor(Qt::SizeHorCursor);
			else
				setCursor(Qt::ArrowCursor);
		}

		if ( _active )
			update();
	}
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::enterEvent(QEvent *) {
	_hoveredMarker = NULL;
	emit mouseOver(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::leaveEvent(QEvent *) {
	_hoveredMarker = NULL;
	emit mouseOver(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::drawCustomBackground(QPainter &) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::customPaintTracesBegin(QPainter &painter) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::customPaintTracesEnd(QPainter &painter) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::resizeEvent (QResizeEvent *event) {
	QWidget::resizeEvent(event);

	if ( _scrollBar ) {
		_scrollBar->setGeometry(QRect(width()-_scrollBar->width(), 0, _scrollBar->width(), height()));
		//_scrollBar->resize(_scrollBar->width(), height());
		//_scrollBar->move(width()-_scrollBar->width(), 0);
	}

	_tmax = _tmin + (_pixelPerSecond > 0 && event->size().width()?event->size().width()/_pixelPerSecond:0);

	if ( _autoMaxScale )
		setNormalizationWindow(visibleTimeWindow());
	else if( event->size().height() != event->oldSize().height() )
		// if the widget height has changed
		setDirty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setSelected (double t1, double t2) {
	_smin = t1;
	_smax = t2;

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setSelected(Seiscomp::Core::Time t1, Seiscomp::Core::Time t2) {
	_smin = (double)(t1 - _alignment);
	_smax = (double)(t2 - _alignment);
	if ( _smin > _smax ) std::swap(_smin, _smax);

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setAlignment(Core::Time t) {
	if ( _alignment == t ) return;

	_alignment = t;
	//setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::alignOnMarker(const QString& text) {
	for ( int i = 0; i < _marker.count(); ++i ) {
		if ( _marker[i]->text() != text ) continue;
		setAlignment(_marker[i]->correctedTime());
		break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::enableFiltering(bool enable) {
	if ( enable == _filtering ) return;

	_filtering = enable;

	if ( _filtering )
		createFilter();

	// Erase already prepared and unused data
	if ( !_showAllRecords ) {
		for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it ) {
			Stream* stream = *it;
			if ( stream->filtering != enable )
				stream->filtering = enable;
			stream->traces[_filtering?Stream::Raw:Stream::Filtered].poly = RecordPolyline();
		}
	}

	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::enableGlobalOffset(bool enable) {
	_useGlobalOffset = enable;

	setDirty();
	update();
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::enableRecordFiltering(int slot, bool enable) {
	Stream *stream = getStream(slot);
	if ( stream == NULL ) return;
	if ( stream->filtering == enable ) return;

	stream->filtering = enable;
	stream->traces[enable?Stream::Raw:Stream::Filtered].poly = RecordPolyline();

	setDirty();
	update();

	if ( _shadowWidget ) _shadowWidget->enableRecordFiltering(slot, enable);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::isRecordFilteringEnabled(int slot) {
	Stream *stream = getStream(slot);
	if ( stream == NULL ) return false;

	return stream->filtering;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setGridSpacing(double large, double _small, double ofs) {
	_gridSpacing[0] = large;
	_gridSpacing[1] = _small;
	_gridOffset = ofs;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setActive(bool a) {
	_active = a;

	/*
	RecordMarker* m = enabledMarker(_cursorText);
	if ( m )
		setCursorPos(m->correctedTime());
	else
		setCursorPos(alignment());
	*/

	setCursor(_active && !_cursorText.isEmpty()?Qt::CrossCursor:Qt::ArrowCursor);

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setAutoMaxScale(bool e) {
	if ( _autoMaxScale == e ) return;

	_autoMaxScale = e;
	if ( _autoMaxScale )
		setNormalizationWindow(visibleTimeWindow());
	else
		setNormalizationWindow(Core::TimeWindow());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setNormalizationWindow(const Seiscomp::Core::TimeWindow &tw) {
	_normalizationWindow = tw;
	_amplScale = 0.0;
	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setOffsetWindow(const Seiscomp::Core::TimeWindow &tw) {
	_offsetWindow = tw;
	setDirty();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::createFilter() {
	if ( _shadowWidgetFlags & Filtered ) return true;

	int slot = 0;
	for ( StreamMap::iterator it = _streams.begin();
	      it != _streams.end(); ++it, ++slot )
		createFilter(slot);
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::createFilter(int slot) {
	Stream* s = getStream(slot);
	if ( s == NULL ) return false;
	if ( _shadowWidgetFlags & Filtered ) return true;

	if (!s->filter) {
		setRecordFilter(slot, NULL);
		return true;
	}

	if (s->records[Stream::Raw] && !s->records[Stream::Raw]->empty()) {
		const Record *rec = s->records[Stream::Raw]->front().get();
		double fs = rec->samplingFrequency();
		if ( s->records[Stream::Filtered] && !s->records[Stream::Filtered]->empty() )
			return false;
		else {
			s->filter->setSamplingFrequency(fs);
			s->filter->setStartTime(rec->startTime());
			s->filter->setStreamID(rec->networkCode(), rec->stationCode(),
			                       rec->locationCode(), rec->channelCode());
			filterRecords(s);
			return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setFilterSlotMax(int max) {
	_maxFilterSlot = max;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setFilter(Math::Filtering::InPlaceFilter<float>* filter) {
	int slot = 0;
	for ( StreamMap::iterator it = _streams.begin(); it != _streams.end(); ++it, ++slot ) {
		if ( slot < _maxFilterSlot || _maxFilterSlot < 0 )
			setRecordFilter(slot, filter);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::filterRecords(Stream *s) {
	s->records[Stream::Filtered] = s->records[Stream::Raw]->clone();
	RecordPtr lastRec;
	for ( RecordSequence::const_iterator it = s->records[Stream::Raw]->begin();
	      it != s->records[Stream::Raw]->end(); ++it) {
		RecordPtr rec = filteredRecord(s->filter, (*it).get(), lastRec.get());
		if ( rec ) {
			s->records[Stream::Filtered]->feed(rec.get());
			s->traces[Stream::Filtered].dirty = true;
			lastRec = rec;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Record* RecordWidget::filteredRecord(Math::Filtering::InPlaceFilter<float> *filter,
                                     const Record* rec, const Record* lastRec) const {
	if ( rec->data() == NULL ) return NULL;

	FloatArrayPtr arr = (FloatArray*)rec->data()->copy(Array::FLOAT);

	GenericRecord* crec = new GenericRecord(*rec);

	// reset filter
	/*
	if ( _filteredRecords->empty() ) {
	}
	else {
		bool contiguous = _filteredRecords->back()->timeWindow().contiguous(
				rec->timeWindow(), _filteredRecords->tolerance()/rec->sampleCount());
		if ( ! contiguous && lastRec ) {
			// interpolate between records to avoid "swing in's"
			const FloatArray* lastData = (FloatArray*)lastRec->data();
			float lastSample = (*lastData)[lastData->size()-1];
			float nextSample = (*arr)[0];

			double gapLength = rec->startTime() - lastRec->endTime();

			if ( gapLength <= 4. && gapLength > 0 ) {
				double delta = nextSample - lastSample;
				int missingSamples = static_cast<int>(ceil(rec->samplingFrequency() * (double)gapLength));
				double step = 1./(double)(missingSamples+1);
				double di = step;
				for ( int i = 0; i < missingSamples; ++i, di += step ) {
					float value = lastSample + di*delta;
					_filter->apply(1, &value);
				}
			}
		}
	}
	*/

	filter->apply(arr->size(), arr->typedData());

	try {
		crec->endTime();
	}
	catch (...) {
		SEISCOMP_ERROR("Filtered record has invalid endtime -> skipping");
		delete crec;
		return NULL;
	}

	crec->setData(arr.get());

	return crec;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
float RecordWidget::timingQuality(int slot) const {
	if ( slot >= _streams.size() || slot < 0 ) return -1;
	return _streams[slot]->traces[Stream::Raw].timingQuality;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Seiscomp::Core::Time RecordWidget::centerTime() {
	return _alignment + Core::TimeSpan((_tmin+_tmax)*0.5);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::streamYPos(int slot) const {
	if ( slot >= _streams.size() || slot < 0 ) return 0;
	return _streams[slot]->posY;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::streamHeight(int slot) const {
	if ( slot >= _streams.size() || slot < 0 ) return height();
	return _streams[slot]->height;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPair<float,float> RecordWidget::amplitudeDataRange(int slot) const {
	if ( slot >= _streams.size() || slot < 0 ) return QPair<float,float>(0,0);
	if ( _showScaledValues )
		return QPair<float,float>(
			_streams[slot]->traces[_filtering?Stream::Filtered:Stream::Raw].amplMin * _streams[slot]->scale,
			_streams[slot]->traces[_filtering?Stream::Filtered:Stream::Raw].amplMax * _streams[slot]->scale
		);
	else
		return QPair<float,float>(
			_streams[slot]->traces[_filtering?Stream::Filtered:Stream::Raw].amplMin,
			_streams[slot]->traces[_filtering?Stream::Filtered:Stream::Raw].amplMax
		);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPair<float,float> RecordWidget::amplitudeRange(int slot) const {
	if ( slot >= _streams.size() || slot < 0 ) return QPair<float,float>(-1,1);
	return QPair<float,float>(
		_streams[slot]->traces[_filtering?Stream::Filtered:Stream::Raw].fyMin,
		_streams[slot]->traces[_filtering?Stream::Filtered:Stream::Raw].fyMax
	);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::ensureVisibility(const Core::Time &time,
                                    int pixelMargin) {
	Core::Time left = time - Core::TimeSpan(pixelMargin/_pixelPerSecond);
	Core::Time right = time + Core::TimeSpan(pixelMargin/_pixelPerSecond);

	double offset = 0;
	if ( right > rightTime() )
		offset = right - rightTime();
	else if ( left < leftTime() )
		offset = left - leftTime();

	if ( offset != 0 )
		setTimeRange(tmin() + offset, tmax() + offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::fed(int slot, const Seiscomp::Record *rec) {
	bool newlyCreated = false;

	if ( (slot < 0) || (slot >= _streams.size()) ) return;

	Stream *s = _streams[slot];

	s->traces[Stream::Raw].dirty = true;

	if ( rec->timingQuality() >= 0 ) {
		if ( s->traces[Stream::Raw].timingQualityCount == 0 )
			s->traces[Stream::Raw].timingQuality = rec->timingQuality();
		else
			s->traces[Stream::Raw].timingQuality =
				(s->traces[Stream::Raw].timingQuality * s->traces[Stream::Raw].timingQualityCount +
				 rec->timingQuality()) / (s->traces[Stream::Raw].timingQualityCount+1);

		++s->traces[Stream::Raw].timingQualityCount;
	}

	_drawRecords = true;

	if ( _shadowWidget  ) {
		if ( (_shadowWidget->_shadowWidgetFlags & Filtered) &&
		      _shadowWidget->_streams[slot]->records[Stream::Filtered]== NULL) {
			_shadowWidget->setFilteredRecords(slot, s->records[Stream::Filtered], false);
		}
		_shadowWidget->fed(slot, rec);
	}

	if ( !(_shadowWidgetFlags & Filtered) && (s->filtering || s->filter != NULL) )
		newlyCreated = createFilter(slot);
	else {
		s->traces[Stream::Filtered].dirty = true;
		return;
	}

	if (!s->records[Stream::Filtered] || !s->filter) return;

	if ( !newlyCreated ) {
		RecordPtr frec = filteredRecord(s->filter, rec);
		if ( frec ) {
			s->records[Stream::Filtered]->feed(frec.get());
			s->traces[Stream::Filtered].dirty = true;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::addMarker(RecordMarker* marker) {
	if ( _markerSourceWidget ) return _markerSourceWidget->addMarker(marker);

	if ( marker == NULL ) return false;

	if ( marker->parent() != this ) {
		if ( marker->parent() )
			marker->parent()->takeMarker(marker);
	}

	marker->setParent(this);
	_marker.push_back(marker);
	//setMouseTracking(true);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::insertMarker(int pos, RecordMarker* marker) {
	if ( _markerSourceWidget ) return _markerSourceWidget->insertMarker(pos, marker);

	if ( marker == NULL ) return false;

	if ( marker->parent() != this ) {
		if ( marker->parent() )
			marker->parent()->takeMarker(marker);
	}

	marker->setParent(this);

	_marker.insert(pos, marker);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::indexOfMarker(RecordMarker* marker) const {
	if ( _markerSourceWidget ) return _markerSourceWidget->indexOfMarker(marker);

	return _marker.indexOf(marker);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker *RecordWidget::takeMarker(int pos) {
	if ( _markerSourceWidget ) return _markerSourceWidget->takeMarker(pos);

	if ( pos >= 0 ) {
		RecordMarker *m = _marker[pos];
		if ( m == _activeMarker )
			_activeMarker = NULL;
		if ( m == _hoveredMarker )
			_hoveredMarker = NULL;
		m->setParent(NULL);
		_marker.remove(pos);
		return m;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker *RecordWidget::takeMarker(RecordMarker *marker) {
	if ( _markerSourceWidget ) return _markerSourceWidget->takeMarker(marker);
	return takeMarker(_marker.indexOf(marker));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::removeMarker(int pos) {
	if ( _markerSourceWidget ) return _markerSourceWidget->removeMarker(pos);
	RecordMarker *m = takeMarker(pos);
	if ( m ) {
		delete m;
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::removeMarker(RecordMarker* marker) {
	if ( _markerSourceWidget ) return _markerSourceWidget->removeMarker(marker);
	return removeMarker(_marker.indexOf(marker));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::markerCount() const {
	if ( _markerSourceWidget ) return _markerSourceWidget->markerCount();
	return _marker.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::marker(int i) const {
	if ( _markerSourceWidget ) return _markerSourceWidget->marker(i);
	return _marker[i];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::marker(const QString& txt, bool movableOnly) const {
	if ( _markerSourceWidget ) return _markerSourceWidget->marker(txt, movableOnly);
	for ( int i = 0; i < _marker.count(); ++i ) {
		if ( !_marker[i]->matches(txt) ) continue;

		if ( !movableOnly || (movableOnly && _marker[i]->isMovable()) )
			return _marker[i];
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::enabledMarker(const QString& txt) const {
	if ( _markerSourceWidget ) return _markerSourceWidget->enabledMarker(txt);
	for ( int i = 0; i < _marker.count(); ++i )
		if ( _marker[i]->matches(txt) && _marker[i]->isEnabled() )
			return _marker[i];

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::clearMarker() {
	_hoveredMarker = NULL;

	if ( _markerSourceWidget ) {
		_markerSourceWidget->clearMarker();
		return;
	}

	while ( !_marker.isEmpty() )
		delete _marker[0];
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setCurrentMarker(RecordMarker *m) {
	if ( _markerSourceWidget ) {
		if ( m != _markerSourceWidget->_activeMarker )
			update();

		_markerSourceWidget->setCurrentMarker(m);

		return;
	}

	if ( _activeMarker == m ) return;

	_activeMarker = m;
	emit currentMarkerChanged(_activeMarker);

	if ( _shadowWidget && _shadowWidget->_markerSourceWidget == this ) {
		_shadowWidget->update();
		emit _shadowWidget->currentMarkerChanged(_activeMarker);
	}

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker *RecordWidget::currentMarker() const {
	if ( _markerSourceWidget ) return _markerSourceWidget->currentMarker();
	return _activeMarker;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker *RecordWidget::hoveredMarker() const {
	return _hoveredMarker;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::markerAt(const QPoint& p, bool movableOnly, int maxDist) const {
	return markerAt(p.x(), p.y(), movableOnly, maxDist);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::markerAt(int x, int y, bool movableOnly, int maxDist) const {
	int minDist = maxDist;
	int h = height();

	RecordMarker *m = NULL;
	for ( int i = markerCount()-1; i >= 0; --i ) {
		RecordMarker *cm = marker(i);
		if ( movableOnly && !cm->isMovable() )
			continue;

		int startY = 0, endY = h;

		switch ( cm->_alignment ) {
			case Qt::AlignTop:
				endY = startY + h*2/4-1;
				break;
			case Qt::AlignBottom:
				startY = startY + h*2/4+1;
				break;
		}

		if ( y < startY || y > endY ) continue;

		int x0 = mapTime(cm->correctedTime());
		int dist = abs(x-x0);
		if ( dist < minDist ) {
			minDist = dist;
			m = cm;
		}
	}

	return m;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::lastMarker(const Seiscomp::Core::Time& t) {
	if ( _markerSourceWidget ) return _markerSourceWidget->lastMarker(t);

	int minI = -1;
	double minT = -1;
	for ( int i = 0; i < markerCount(); ++i ) {
		double delta = (double)(t - marker(i)->correctedTime());
		if ( delta > 0 && (delta < minT || minT < 0) ) {
			minT = delta;
			minI = i;
		}
	}

	if ( minI != -1 )
		return marker(minI);

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::nextMarker(const Seiscomp::Core::Time& t) {
	if ( _markerSourceWidget ) return _markerSourceWidget->nextMarker(t);

	int minI = -1;
	double minT = -1;
	for ( int i = 0; i < markerCount(); ++i ) {
		double delta = (double)(marker(i)->correctedTime() - t);
		if ( delta > 0 && (delta < minT || minT < 0) ) {
			minT = delta;
			minI = i;
		}
	}

	if ( minI != -1 )
		return marker(minI);

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* RecordWidget::nearestMarker(const Seiscomp::Core::Time& t,
                                          int maxDist) {
	int minI = -1;
	double minT = -1;
	for ( int i = 0; i < markerCount(); ++i ) {
		double delta = fabs(marker(i)->correctedTime() - t);
		if ( delta < minT || minT < 0 ) {
			minT = delta;
			minI = i;
		}
	}

	if ( maxDist >= 0 && minT*_pixelPerSecond > maxDist )
		return NULL;

	if ( minI != -1 )
		return marker(minI);

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool RecordWidget::hasMovableMarkers() const {
	if ( _markerSourceWidget ) return _markerSourceWidget->hasMovableMarkers();

	foreach(RecordMarker* m, _marker)
		if ( m->isMovable() && m->isEnabled() ) return true;

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setCursorText(const QString& text) {
	_cursorText = text;

	setCursor(_active && !_cursorText.isEmpty()?Qt::CrossCursor:Qt::ArrowCursor);

	if ( _active )
		update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString RecordWidget::cursorText() const {
	return _cursorText;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setCursorPos(const QPoint& p) {
	double dt = p.x()/_pixelPerSecond+_tmin;
	_cursorPos = _alignment + Core::TimeSpan(dt);

	if ( _enabled && _active ) {
		int slot;
		if ( _drawMode == InRows )
			slot = p.y() * _streams.size() / height();
		else
			slot = _currentSlot;

		update();
		emit cursorMoved(mapToGlobal(p));
		emit cursorUpdated(this, slot);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setCursorPos(const Seiscomp::Core::Time& t) {
	_cursorPos = t;
	if ( _enabled && _active ) {
		update();
		emit cursorMoved(mapToGlobal(QPoint(mapTime(_cursorPos), height()/2)));
		emit cursorUpdated(this, _currentSlot);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Seiscomp::Core::Time& RecordWidget::cursorPos() const {
	return _cursorPos;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const double *RecordWidget::value(const Seiscomp::Core::Time& t) const {
	return _drawMode == Single ? value(_currentSlot, t) : NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::mapTime(const Seiscomp::Core::Time& t) const {
	return (int)(((double)(t-_alignment)-_tmin)*_pixelPerSecond);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::Time RecordWidget::unmapTime(int x) const {
	return _alignment + Core::TimeSpan(x/_pixelPerSecond+_tmin);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setTracePaintOffset(int offset) {
	_tracePaintOffset = offset;
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::tracePaintOffset() const {
	return _tracePaintOffset;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::WaveformStreamID& RecordWidget::streamID() const {
	return _streamID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setSlotCount(int c) {
	if ( _shadowWidget )
		_shadowWidget->setSlotCount(c);

	int oldSize = _streams.size();
	if ( c < oldSize ) {
		// Delete ununsed slots
		for ( int i = c; i < oldSize; ++i )
			delete _streams[i];
	}

	_streams.resize(c);

	for ( int i = oldSize; i < _streams.size(); ++i ) {
		_streams[i] = new Stream(true);
		_streams[i]->hasCustomBackgroundColor = _hasCustomBackground;
		_streams[i]->customBackgroundColor = _customBackgroundColor;
		_streams[i]->filtering = _filtering;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int RecordWidget::slotCount() const {
	return _streams.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::updateRecords() {
	if ( _drawRecords ) update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordWidget::setData(const QVariant& data) {
	_data = data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant RecordWidget::data() const {
	return _data;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
