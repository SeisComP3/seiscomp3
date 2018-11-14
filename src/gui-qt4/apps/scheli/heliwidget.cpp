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

#include "heliwidget.h"
#include <seiscomp3/core/genericrecord.h>
#include <seiscomp3/math/filter/butterworth.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/utils.h>


using namespace Seiscomp;


namespace {

bool minmax(const RecordSequence *seq, const Core::TimeWindow &tw,
            float &ofs, float &min, float &max) {
	ofs = 0;
	int sampleCount = 0;
	RecordSequence::const_iterator it = seq->begin();
	min = max = 0;

	for (; it != seq->end(); ++it) {
		RecordCPtr rec = (*it);
		int imin = 0, imax = 0;
		int ns = rec->sampleCount();
		if ( ns == 0 || rec->data() == NULL ) continue;

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

		FloatArray *arr = (FloatArray*)(rec->data());

		const float *f = (float*)arr->data();

		sampleCount += imax-imin+1;

		float xmin, xmax;
		::minmax(ns, f, imin, imax, &xmin, &xmax);

		for ( int i = imin; i < imax; ++i )
			ofs += (*arr)[i];

		if(min==max) {
			min = xmin;
			max = xmax;
		}
		else {
			if (xmin<min) min = xmin;
			if (xmax>max) max = xmax;
		}
	}

	ofs /= (sampleCount?sampleCount:1);
	return sampleCount > 0;
}


}


void HeliCanvas::Row::update() {
	dirty = true;
}


HeliCanvas::HeliCanvas(bool saveUnfiltered)
: _saveUnfiltered(saveUnfiltered) {
	_records = NULL;
	_filteredRecords = NULL;
	_scale = 1.0f;
	_filter = NULL;

	_gaps[0] = SCScheme.colors.records.gaps;
	_gaps[1] = SCScheme.colors.records.gaps;

	_amplitudeRange[0] = -0.00001;
	_amplitudeRange[1] = +0.00001;

	_labelMargin = 0;
	_antialiasing = false;
	_palette = qApp->palette();

	_lineWidth = 1;
}


HeliCanvas::~HeliCanvas() {
	if ( _records )
		delete _records;

	if ( _filteredRecords )
		delete _filteredRecords;

	if ( _filter )
		delete _filter;
}


void HeliCanvas::setLayout(int rows, int secondsPerRow) {
	setRowTimeSpan(secondsPerRow);
	setRecordsTimeSpan(rows*secondsPerRow);
	rebuildView();
}


void HeliCanvas::setScale(float scale) {
	_scale = scale;
}


void HeliCanvas::setAntialiasingEnabled(bool e) {
	if ( _antialiasing == e ) return;
	_antialiasing = e;
}


bool HeliCanvas::setFilter(const std::string &filterStr) {
	Filter *filter = Filter::Create(filterStr);
	if ( filter == NULL ) return false;

	if ( _filter == NULL && filter == NULL ) {
		return false;
	}

	if ( _filter )
		delete _filter;

	_filter = filter;
	applyFilter();

	return true;
}


void HeliCanvas::setAmplitudeRange(double min, double max) {
	_amplitudeRange[0] = min;
	_amplitudeRange[1] = max;

	if ( _amplitudeRange[0] > _amplitudeRange[1] )
		std::swap(_amplitudeRange[0], _amplitudeRange[1]);

	for ( int i = 0; i < _rows.size(); ++i )
		_rows[i].update();
}


void HeliCanvas::setRowColors(const QVector<QColor> &cols) {
	_rowColors = cols;
}


void HeliCanvas::setLineWidth(int lw) {
	_lineWidth = lw;
}


void HeliCanvas::applyFilter() {
	if ( _filteredRecords == NULL ) return;

	_filteredRecords->clear();
	for ( RecordSequence::iterator it = _records->begin(); it != _records->end(); ++it ) {
		FloatArrayPtr arr = (FloatArray*)(*it)->data()->copy(Array::FLOAT);
		GenericRecordPtr frec = new GenericRecord(**it);
		frec->setData(arr.get());

		if ( _filter ) {
			if ( _filteredRecords->empty() ) {
				_filter->setSamplingFrequency(frec->samplingFrequency());
				_filter->setStartTime(frec->startTime());
				_filter->setStreamID(frec->networkCode(), frec->stationCode(),
				                     frec->locationCode(), frec->channelCode());
			}
			_filter->apply(arr->size(), arr->typedData());
		}

		_filteredRecords->feed(frec.get());
	}

	for ( int i = 0; i < _rows.size(); ++i )
		_rows[i].update();
}


bool HeliCanvas::feed(Record *rec) {
	RecordPtr tmp(rec);

	if ( rec->data() == NULL ) return false;
	if ( _records == NULL ) return false;

	FloatArrayPtr arr = (FloatArray*)rec->data()->copy(Array::FLOAT);
	GenericRecordPtr crec = new GenericRecord(*rec);

	for ( int i = 0; i < arr->size(); ++i )
		arr->typedData()[i] *= _scale;

	crec->setData(arr.get());

	GenericRecordPtr frec;

	if ( _saveUnfiltered ) {
		if ( !_records->feed(crec.get()) ) return false;
		arr = (FloatArray*)crec->data()->copy(Array::FLOAT);
		frec = new GenericRecord(*crec);
		frec->setData(arr.get());
	}
	else
		frec = crec;

	if ( _filter ) {
		if ( _filteredRecords->empty() ) {
			_filter->setSamplingFrequency(frec->samplingFrequency());
			_filter->setStartTime(frec->startTime());
			_filter->setStreamID(frec->networkCode(), frec->stationCode(),
			                     frec->locationCode(), frec->channelCode());
		}
		_filter->apply(arr->size(), arr->typedData());
	}

	if ( !_filteredRecords->feed(frec.get()) ) return false;
	if ( _rows.empty() ) return false;

	Core::Time startTime = rec->startTime();
	Core::Time endTime = rec->endTime();

	Core::Time globalStart = _rows[0].time;

	int diff = (startTime - globalStart).seconds() / _rowTimeSpan;
	bool update = false;
	if ( diff >= 0 && diff < _rows.size() ) {
		_rows[diff].update();
		update = true;
	}

	diff = (endTime - globalStart).seconds() / _rowTimeSpan;
	if ( diff >= 0 && diff < _rows.size() ) {
		_rows[diff].update();
		update = true;
	}

	return update;
}


bool HeliCanvas::setCurrentTime(const Seiscomp::Core::Time &time) {
	if ( _rows.empty() ) return false;

	_currentTime = time;

	time_t newEndSeconds = (_currentTime.seconds() / _rowTimeSpan) * _rowTimeSpan;
	time_t oldEndTime = _rows.back().time;

	// Computes the row shift
	// > 0: shift rows down
	// < 0: shift rows up
	time_t shift = (newEndSeconds - oldEndTime) / _rowTimeSpan;

	if ( shift == 0 ) return false;

	if ( abs(shift) >= _rows.size() ) {
		for ( int i = 0; i < _rows.size(); ++i ) {
			_rows[_rows.size()-1-i].time = Core::Time(newEndSeconds - i*_rowTimeSpan);
			_rows[_rows.size()-1-i].update();
		}

		return true;
	}

	if ( shift > 0 ) {
		for ( int i = 0; i < _rows.size() - shift; ++i )
			_rows[i] = _rows[i+shift];

		for ( int i = _rows.size() - shift; i < _rows.size(); ++i ) {
			_rows[i].time = _rows[i-1].time + Core::TimeSpan(_rowTimeSpan);
			_rows[i].polyline = RecordPolylinePtr();
			_rows[i].update();
		}
	}
	else {
		for ( int i = _rows.size()-1; i >= shift; --i )
			_rows[i] = _rows[i-shift];

		for ( int i = shift-1; i >= 0; --i ) {
			_rows[i].time = _rows[i+1].time - Core::TimeSpan(_rowTimeSpan);
			_rows[i].polyline = RecordPolylinePtr();
			_rows[i].update();
		}
	}

	return true;
}


Core::TimeSpan HeliCanvas::recordsTimeSpan() const {
	return Core::TimeSpan(_recordsTimeSpan);
}


void HeliCanvas::setRecordsTimeSpan(int span) {
	_recordsTimeSpan = span;

	if ( _records )
		delete _records;

	_records = new RingBuffer(recordsTimeSpan());
	_filteredRecords = new RingBuffer(recordsTimeSpan());
}


void HeliCanvas::setRowTimeSpan(int seconds) {
	_rowTimeSpan = seconds;
}


void HeliCanvas::rebuildView() {
	_numberOfRows = (_recordsTimeSpan + _rowTimeSpan-1) / _rowTimeSpan;
	_rows = QVector<Row>(_numberOfRows);
}


int HeliCanvas::draw(QPainter &p, const QSize &size) {
	int tmp = _labelMargin;
	_labelMargin = p.fontMetrics().width("00:00-");
	QSize oldSize = _size;

	resize(p.fontMetrics(), size);
	render(p);

	std::swap(_labelMargin, tmp);
	resize(p.fontMetrics(), oldSize);

	return tmp;
}


void HeliCanvas::save(QString streamID, QString headline, QString date,
                      QString filename, int xres, int yres, int dpi) {
	std::cerr << "Printing..." << std::flush;

	QPainter *painter;
	QFileInfo fi(filename);
	QPrinter *printer = NULL;
	QImage *pixmap = NULL;

	if ( fi.suffix().toLower() == "ps" ) {
		printer = new QPrinter(QPrinter::HighResolution);
		printer->setOutputFileName(filename);
		printer->setResolution(dpi);
		printer->setPageSize(QPrinter::A4);
		painter = new QPainter(printer);
	}
	else {
		pixmap = new QImage(xres,yres,QImage::Format_RGB32);
		painter = new QPainter(pixmap);
		painter->fillRect(painter->window(), _palette.color(QPalette::Base));
	}

	painter->setFont(SCScheme.fonts.base);

	int fontHeight = painter->fontMetrics().height();
	int headerHeight = fontHeight*120/100;
	if ( !headline.isEmpty() )
		headerHeight += fontHeight*120/100;

	painter->translate(0, headerHeight);

	int offset = draw(
		*painter,
		QSize(painter->viewport().width(),
		      painter->viewport().height()-headerHeight)
		);

	painter->translate(0, -headerHeight);
	painter->drawText(offset, 0, painter->viewport().width()-offset, fontHeight,
	                  Qt::AlignLeft | Qt::AlignTop, streamID);
	painter->drawText(offset, 0, painter->viewport().width()-offset, fontHeight,
	                  Qt::AlignRight | Qt::AlignTop, date);

	if ( !headline.isEmpty() )
		painter->drawText(offset, fontHeight*120/100, painter->viewport().width()-offset, fontHeight,
		                  Qt::AlignLeft | Qt::AlignTop, headline);

	painter->drawLine(0, headerHeight, painter->viewport().width(), headerHeight);

	if ( pixmap )
		pixmap->save(filename);

	painter->end();

	// Clean up
	if ( printer ) delete printer;
	if ( pixmap ) delete pixmap;
	if ( painter ) delete painter;

	std::cerr << "finished" << std::endl;
}


void HeliCanvas::render(QPainter &p) {
	int fontHeight = p.fontMetrics().height();
	int h = _size.height() - fontHeight*160/100;

	if ( _rows.empty() ) {
		p.fillRect(QRect(_labelMargin,0,_size.width()-_labelMargin,h), _palette.color(QPalette::Base));
		return;
	}

	p.setClipRect(QRect(0,0,_size.width(),h));

	int rowHeight = h / (_rows.size()+2);
	int rowPos = rowHeight;
	int recordWidth = _size.width();
	Core::Time globalEnd = _records->empty()?Core::Time():_records->back()->endTime();

	int yGap = h - rowHeight*(_rows.size()+2);
	int remainingGap = yGap;
	int heightOfs = yGap > 0?1:0;
	int skipTextLines = (fontHeight*3/2) / rowHeight;
	int currentTextLine = skipTextLines;

	p.setPen(_palette.color(QPalette::Text));

	for ( int i = 0; i < _rows.size(); ++i, rowPos += rowHeight + heightOfs ) {
		if ( currentTextLine == 0 ) {
			p.drawText(QRect(0, rowPos-fontHeight, _labelMargin, rowHeight+2*fontHeight + heightOfs),
			           Qt::AlignLeft | Qt::AlignVCenter,
			           Gui::timeToString(_rows[i].time, "%H:%M"));
			currentTextLine = skipTextLines;
		}
		else
			--currentTextLine;

		--remainingGap;
		if ( remainingGap <= 0 )
			heightOfs = 0;
	}

	recordWidth -= _labelMargin;

	p.fillRect(QRect(_labelMargin, 0, recordWidth, h), _palette.color(QPalette::Base));

	heightOfs = yGap > 0?1:0;
	remainingGap = yGap;
	rowPos = rowHeight;

	for ( int i = 0; i < _rows.size(); ++i, rowPos += rowHeight + heightOfs ) {
		QBrush gapBrush = _gaps[i % 2];

		// Create new sequence
		if ( _rows[i].dirty ) {
			if ( !_rows[i].polyline )
				_rows[i].polyline = RecordPolylinePtr(new Gui::RecordPolyline);

			float ofs, min, max;
			Core::TimeWindow tw(_rows[i].time, _rows[i].time + Core::TimeSpan(_rowTimeSpan));
			minmax(_filteredRecords, tw, ofs, min, max);

			_rows[i].polyline->create(_filteredRecords,
			                          tw.startTime(), tw.endTime(),
			                          (double)recordWidth / (double)_rowTimeSpan,
			                          _amplitudeRange[0], _amplitudeRange[1], ofs, rowHeight);
			_rows[i].dirty = false;
		}

		if ( _rows[i].polyline ) {
			if ( !_rows[i].polyline->empty() ) {
				// Draw front and back gaps
				if ( _rows[i].polyline->front().first().x() > _labelMargin )
					p.fillRect(_labelMargin, rowPos,
					           _rows[i].polyline->front().first().x() - _labelMargin,
					           rowHeight, gapBrush);

				if ( (globalEnd - _rows[i].time) >= Core::TimeSpan(_rowTimeSpan) ) {
					if ( _rows[i].polyline->back().last().x() < _size.width()-2 )
						p.fillRect(_rows[i].polyline->back().last().x(),
						           rowPos, _size.width()-1 - _rows[i].polyline->back().last().x(),
						           rowHeight, gapBrush);
				}
			}
			else if ( (globalEnd - _rows[i].time) >= Core::TimeSpan(_rowTimeSpan) )
				p.fillRect(QRect(_labelMargin, rowPos, recordWidth, rowHeight), gapBrush);
		}

		--remainingGap;
		if ( remainingGap <= 0 )
			heightOfs = 0;
	}

	heightOfs = yGap > 0?1:0;
	remainingGap = yGap;
	rowPos = rowHeight;

	int rowColorIndex = 0;

	bool hadAntialiasing = (p.renderHints() & QPainter::Antialiasing) != 0;
	p.setRenderHint(QPainter::Antialiasing, _antialiasing);

	for ( int i = 0; i < _rows.size(); ++i, rowPos += rowHeight + heightOfs ) {
		QBrush gapBrush = _gaps[i & 1];
		QBrush overlapBrush = _gaps[i & 1];
		p.setPen(QPen(_rowColors[rowColorIndex], _lineWidth));

		++rowColorIndex;
		if ( rowColorIndex >= _rowColors.size() )
			rowColorIndex = 0;

		if ( !_rows[i].polyline ) continue;

		p.translate(_labelMargin, rowPos);
		_rows[i].polyline->draw(p, 0, rowHeight, gapBrush, overlapBrush);
		p.translate(-_labelMargin, -rowPos);

		--remainingGap;
		if ( remainingGap <= 0 )
			heightOfs = 0;
	}

	p.setRenderHint(QPainter::Antialiasing, hadAntialiasing);

	p.setClipping(false);
	p.setPen(_palette.color(QPalette::Text));

	// Render timescale
	double pos = 0;
	double scale = (double)recordWidth / (double)_rowTimeSpan;
	double dx[2] = {_drx[0], _drx[1]};
	
	// Adapted from gui/libs/ruler
	int tickLong = fontHeight/2;
	int tickShort = tickLong/2;
	int startY = h;
	h = _size.height()-startY;

	//p.setPen(p.palette().color(QPalette::WindowText));
	p.drawLine(_labelMargin, startY, _size.width(), startY);

	for ( int k = 0; k < 2; ++k ) {
		double cpos = pos - fmod(pos, (double)dx[k]);

		int tick = k==0 ? tickLong : tickShort;

		int x = (int)((cpos-pos)*scale);
		while ( x < recordWidth ) {
			p.drawLine(_labelMargin+x, startY, _labelMargin+x, startY+tick);

			if ( k == 0 ) {
				QString str = formatAnnotation(cpos);

				int tw = p.fontMetrics().width(str);
				p.drawText(_labelMargin + x-tw/2, startY, tw, h,
				           Qt::AlignHCenter | Qt::AlignBottom, str);
			}

			cpos += dx[k];
			x = (int)((cpos-pos)*scale);
		}
	}
}


void HeliCanvas::resize(const QFontMetrics &fm, const QSize &size) {
	struct Spacing {
		double      major;
		double      minor;
		const char *format;
	};

	const Spacing spacings[] = {
		{0.001, 0.0002, "%S.%3f"},
		{0.005, 0.001, "%S.%3f"},
		{0.01, 0.002, "%S.%2f"},
		{0.05, 0.01, "%S.%2f"},
		{0.1, 0.02, "%T.%1f"},
		{0.5, 0.1, "%T.%1f"},
		{1, 0.2, "%T"},
		{2, 0.5, "%T"},
		{5, 1, "%T"},
		{10, 2, "%T"},
		{15, 3, "%T"},
		{30, 5, "%T"},
		{60, 10, "%T"},
		{120, 20, "%T"},
		{300, 60, "%T"},
		{600, 120, "%T"},
		{900, 300, "%T"},
		{1800, 300, "%T"},
		{3600, 600, "%T"},
		{2*3600, 1200, "%T"},
		{6*3600, 3600, "%T"},
		{12*3600, 2*3600, "%T"},
		{24*3600, 6*3600, "%b, %d"},
		{2*24*3600, 12*3600, "%b, %d"},
		{7*24*3600, 24*3600, "%b, %d"},
		{14*24*3600, 2*24*3600, "%b, %d"}
	};

	_size = size;
	for ( int i = 0; i < _rows.size(); ++i )
		_rows[i].update();

	unsigned int imax = sizeof(spacings)/sizeof(Spacing);

	double scale = (double)_size.width() / (double)_rowTimeSpan;

	_drx[0] = spacings[imax-1].major;
	_drx[1] = spacings[imax-1].minor;

	int minDistance = fm.width("  XXXX-XX-XX.X  ");
	unsigned int i;
	for ( i = 0; i < imax; ++i ) {
		if ( spacings[i].major*scale >= minDistance ) {
			_drx[0] = spacings[i].major;
			_drx[1] = spacings[i].minor;
			break;
		}
	}

	if ( i == imax && scale > 0 ) {
		while ( _drx[0]*scale < minDistance ) {
			_drx[0] *= 2;
			_drx[1] *= 2;
		}
	}
}


QString HeliCanvas::formatAnnotation(const double pos){
	if ( pos == 0 ) return "0";

	int d = floor(pos / 86400);
	int h = floor((pos - d*86400) / 3600);
	int m = floor((pos - (d*86400 + h*3600)) / 60);
	int s = floor(pos - (d*86400 + h*3600 + m*60));
	double r = (pos - (d*86400 + h*3600 + m*60 + s));

	if ( d > 0 ) {
		if ( s > 0 )
			return QString("%1d %2h %3m %4s").arg(d).arg(h).arg(m).arg(s);
		else if ( m > 0 )
			return QString("%1d %2h %3m").arg(d).arg(h).arg(m);
		else if ( h > 0 )
			return QString("%1d %2h").arg(d).arg(h);
		else
			return QString("%1d").arg(d);
	}
	else if ( h > 0 ) {
		if ( s > 0 )
			return QString("%1h %2m %3s").arg(h).arg(m).arg(s);
		else if ( m > 0 )
			return QString("%1h %2m").arg(h).arg(m);
		else
			return QString("%1h").arg(h);
	}
	else if ( m > 0 ) {
		if ( s > 0 )
			return QString("%1m %2s").arg(m).arg(s);
		else
			return QString("%1m").arg(m);
	}
	else {
		if ( r == 0 ) {
			return QString("%1s").arg(s);
		}
		else {
			return QString().sprintf("%0.2fs",s+r);
		}
	}
}



HeliWidget::HeliWidget(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f) {
	_canvas.setLabelMargin(fontMetrics().width("00:00") + 6);
}


HeliWidget::~HeliWidget() {
}


void HeliWidget::feed(Record *rec) {
	if ( _canvas.feed(rec) )
		update();
}


void HeliWidget::setCurrentTime(const Seiscomp::Core::Time &time) {
	if ( _canvas.setCurrentTime(time) )
		update();
}


void HeliWidget::paintEvent(QPaintEvent *) {
	QPainter p(this);
	_canvas.render(p);
}


void HeliWidget::resizeEvent(QResizeEvent *) {
	_canvas.resize(fontMetrics(), size());
}
