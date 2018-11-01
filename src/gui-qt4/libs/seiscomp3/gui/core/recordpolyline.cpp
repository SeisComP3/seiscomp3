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



#include <seiscomp3/gui/core/recordpolyline.h>
#include <iostream>


using namespace std;


//#define DEBUG_POINTS


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordPolyline::RecordPolyline() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordPolyline::create(Record const *rec,
                            double pixelPerSecond,
                            float amplMin, float amplMax, float amplOffset,
                            int height, float *timingQuality,
                            bool optimization) {
	clear();

	if ( rec == NULL )
		return;

	// normalize peak-to-peak amplitude to height set using setHeight()
	double yscl;
	double amplHeight = amplMax - amplMin;

	if ( amplHeight == 0.0 ) {
		_baseline = height/2;
		yscl = 0;
	}
	else {
		yscl = (height-1)/amplHeight;
		_baseline = (int)(amplMax*yscl);
	}

	QPolygon *poly = NULL;

	int nsamp = rec->sampleCount();

	if ( nsamp == 0 ) return;

	if ( timingQuality )
		*timingQuality = rec->timingQuality();

	push_back(QPolygon());
	poly = &back();

	const FloatArray *arr = (const FloatArray*)rec->data();

	float *f = (float*)arr->data();
	float dx = pixelPerSecond / rec->samplingFrequency();

	int x_pos, y_pos;

	if ( optimization ) {
		x_pos = 0;
		y_pos = int(_baseline-yscl*(f[0]-amplOffset));

		int y_min = y_pos;
		int y_max = y_pos;

		int x_out = x_pos;
		int y_out = y_pos;

		poly->append(QPoint(x_pos, y_pos));

		int collapsedSamples = 0;

		for ( int i = 1; i < nsamp; ++i ) {
			x_pos = i*dx;
			y_pos = _baseline-yscl*(f[i]-amplOffset);

			if ( (int)x_out == (int)x_pos ) {
				// Points share the same pixel, just collect min/max
				if ( y_pos < y_min ) y_min = y_pos;
				else if ( y_pos > y_max ) y_max = y_pos;
				++collapsedSamples;
			}
			else {
				if ( collapsedSamples ) {
					// We want to draw from y_out to y_pos and taking
					// y_min/y_max into account
					if ( !(y_out <= y_min && y_pos >= y_max) &&
					     !(y_out >= y_max && y_pos <= y_min) ) {
						if ( y_out < y_pos ) {
							poly->append(QPoint(x_out, y_min));
							poly->append(QPoint(x_out, y_max));
						}
						else {
							poly->append(QPoint(x_out, y_max));
							poly->append(QPoint(x_out, y_min));
						}
					}
				}

				poly->append(QPoint(x_pos, y_pos));
				x_out = x_pos;
				y_out = y_min = y_max = y_pos;

				collapsedSamples = 0;
			}
		}

		if ( collapsedSamples ) {
			// We want to draw from y_out to y_pos and taking
			// y_min/y_max into account
			if ( !(y_out <= y_min && y_pos >= y_max) &&
			     !(y_out >= y_max && y_pos <= y_min) ) {
				if ( y_out < y_pos ) {
					poly->append(QPoint(x_out, y_min));
					poly->append(QPoint(x_out, y_max));
				}
				else {
					poly->append(QPoint(x_out, y_max));
					poly->append(QPoint(x_out, y_min));
				}
			}
		}

		if ( x_pos != x_out || y_pos != y_out )
			poly->append(QPoint(x_pos, y_pos));
	}
	else {
		for ( int i = 0; i < nsamp; ++i ) {
			x_pos = i*dx;
			y_pos = _baseline-yscl*(f[i]-amplOffset);
			poly->append(QPoint(x_pos, y_pos));
		}
	}

	if ( poly->isEmpty() )
		pop_back();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordPolyline::create(RecordSequence const *records,
                            double pixelPerSecond,
                            float amplMin, float amplMax, float amplOffset,
                            int height, float *timingQuality,
                            QVector<QPair<int,int> >* gaps,
                            bool optimization) {
	static Core::Time invalidTime;
	create(records, invalidTime, invalidTime, pixelPerSecond,
	       amplMin, amplMax, amplOffset, height, timingQuality,
	       gaps, optimization);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordPolyline::create(RecordSequence const *records,
                            const Core::Time &start,
                            const Core::Time &end,
                            double pixelPerSecond,
                            float amplMin, float amplMax, float amplOffset,
                            int height, float *timingQuality,
                            QVector<QPair<int,int> >* gaps,
                            bool optimization) {
	clear();

	if ( records == NULL ) return;
	if ( records->size() == 0 ) return;

	// normalize peak-to-peak amplitude to height set using setHeight()
	double yscl;
	double amplHeight = amplMax - amplMin;

	if ( amplHeight == 0.0 ) {
		_baseline = height/2;
		yscl = 0;
	}
	else {
		yscl = (height-1)/amplHeight;
		_baseline = (int)(amplMax*yscl);
	}

	int skipCount = 0;
	RecordSequence::const_iterator it = records->begin();
	RecordSequence::const_iterator lastIt = it;

	QPolygon *poly = NULL;
	int timingQualityRecordCount = 0;
	if ( timingQuality ) *timingQuality = 0;

	int y_min = 0;
	int y_max = 0;

	int x_out = 0;
	int y_out = 0;

	int x_pos = 0;
	int y_pos = 0;

	int i;
	int collapsedSamples = 0;
	double startOfs = 0;
	double endOfs = 0;

	for( ; it != records->end(); ++it ) {
		const Record *rec = it->get();
		const Record *lastRec = lastIt->get();

		// Skip records that are out of time window [start:end]
		if ( start.valid() ) {
			try {
				if ( rec->endTime() <= start ) continue;
			}
			catch ( ... ) { continue; }
			startOfs = double(start-rec->startTime());
		}
		else
			startOfs = double(records->front()->startTime()-rec->startTime());

		double dt = 1.0/rec->samplingFrequency();
		if ( end.valid() ) {
			if ( rec->startTime() >= end ) break;
			endOfs = double(rec->endTime()-end) - dt;
		}

		if ( timingQuality && rec->timingQuality() >= 0 ) {
			*timingQuality += rec->timingQuality();
			++timingQualityRecordCount;
		}

		int nsamp = rec->sampleCount();
		double tolerance = records->tolerance()/rec->samplingFrequency();
		double diff;

		if ( nsamp == 0 ) continue;

		try {
			diff = abs(double(rec->startTime() - lastRec->endTime()));
		}
		catch ( ... ) {
			diff = tolerance*2;
		}

		const FloatArray *arr = (const FloatArray*)rec->data();
		const float *f = arr->typedData();

		// Cut front samples
		if ( startOfs > 0 ) {
			int sampleOfs = (int)(startOfs * rec->samplingFrequency());
			if ( sampleOfs >= nsamp ) continue;
			f += sampleOfs;
			nsamp -= sampleOfs;
			startOfs -= (sampleOfs * dt);
		}

		// Cut back samples
		if ( endOfs > 0 ) {
			nsamp -= (int)(endOfs * rec->samplingFrequency());
			if ( nsamp <= 0 ) continue;
		}

		int x0 = int(pixelPerSecond*startOfs);
		float dx = pixelPerSecond * dt;

		if ( diff > tolerance || poly == NULL ) {
			push_back(QPolygon());
			poly = &back();

			x_pos = -x0;
			y_pos = _baseline-yscl*(f[0]-amplOffset);
			y_min = y_pos;
			y_max = y_pos;

			x_out = x_pos;
			y_out = y_pos;

			poly->append(QPoint(x_pos, y_pos));
			i = 1;
		}
		else
			i = 0;

		if ( optimization ) {
			for ( ; i < nsamp; ++i ) {
				x_pos = (i*dx) - x0;
				y_pos = _baseline-yscl*(f[i]-amplOffset);

				if ( (int)x_out == (int)x_pos ) {
					// Points share the same pixel, just collect min/max
					if ( y_pos < y_min ) y_min = y_pos;
					else if ( y_pos > y_max ) y_max = y_pos;
					++collapsedSamples;
				}
				else {
					if ( collapsedSamples ) {
						// We want to draw from y_out to y_pos and taking
						// y_min/y_max into account
						if ( !(y_out <= y_min && y_pos >= y_max) &&
						     !(y_out >= y_max && y_pos <= y_min) ) {
							if ( y_out < y_pos ) {
								poly->append(QPoint(x_out, y_min));
								poly->append(QPoint(x_out, y_max));
							}
							else {
								poly->append(QPoint(x_out, y_max));
								poly->append(QPoint(x_out, y_min));
							}
						}
					}

					poly->append(QPoint(x_pos, y_pos));
					x_out = x_pos;
					y_out = y_min = y_max = y_pos;

					collapsedSamples = 0;
				}
			}
		}
		else {
			for ( ; i < nsamp; ++i ) {
				x_pos = (i*dx) - x0;
				y_pos = _baseline-yscl*(f[i]-amplOffset);
				poly->append(QPoint(x_pos, y_pos));
			}
		}

		if ( poly->isEmpty() )
			pop_back();

		lastIt = it;
	}

	if ( optimization && poly ) {
		if ( collapsedSamples ) {
			// We want to draw from y_out to y_pos and taking
			// y_min/y_max into account
			if ( !(y_out <= y_min && y_pos >= y_max) &&
			     !(y_out >= y_max && y_pos <= y_min) ) {
				if ( y_out < y_pos ) {
					poly->append(QPoint(x_out, y_min));
					poly->append(QPoint(x_out, y_max));
				}
				else {
					poly->append(QPoint(x_out, y_max));
					poly->append(QPoint(x_out, y_min));
				}
			}
		}

		if ( x_pos != x_out || y_pos != y_out )
			poly->append(QPoint(x_pos, y_pos));
	}

	if ( !empty() ) {
		if ( skipCount )
			front().remove(0, skipCount);

		if ( gaps ) {
			for ( int i = 1; i < size(); ++i )
				gaps->append(QPair<int,int>((*this)[i-1].last().x(), (*this)[i].first().x()));
		}
	}

	if ( timingQuality ) {
		if ( timingQualityRecordCount )
			*timingQuality /= timingQualityRecordCount;
		else
			*timingQuality = -1;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

template <typename T>
inline
void pushData(QPolygon *poly, const Record *rec, const Core::Time refTime,
              double pixelPerSecond, float multiplier, float amplOffset,
              int baseline, double yscl) {
	const TypedArray<T> *arr = (const TypedArray<T>*)rec->data();
	T *f = (T*)arr->data();
	int x0 = int(pixelPerSecond*double(refTime-rec->startTime()));
	float dx = pixelPerSecond / rec->samplingFrequency();
	int nsamp = arr->size();

	int x_prev = -x0;
	int y_prev = int(baseline-yscl*(f[0]*multiplier-amplOffset));

	if ( !poly->isEmpty() ) {
		poly->append(QPoint(x_prev, poly->back().y()));
	}

	poly->append(QPoint(x_prev, y_prev));

	for ( int i = 1; i<nsamp; ++i ) {
		// horizontal line
		int x_pos = int(i*dx) - x0;
		int y_pos = int(baseline-yscl*(f[i]*multiplier-amplOffset));
		poly->append(QPoint(x_pos, y_prev));
		poly->append(QPoint(x_pos, y_pos));

		x_prev = x_pos;
		y_prev = y_pos;
	}
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordPolyline::createStepFunction(RecordSequence const *records, double pixelPerSecond,
                                        float amplMin, float amplMax, float amplOffset,
                                        int height, float multiplier) {
	clear();

	if ( records == NULL ) return;
	if ( records->size() == 0 ) return;

	// normalize peak-to-peak amplitude to height set using setHeight()
	double yscl;
	double amplHeight = amplMax - amplMin;

	if ( amplHeight == 0.0 ) {
		_baseline = height/2;
		yscl = 0;
	}
	else {
		yscl = (height-1)/amplHeight;
		_baseline = (int)(amplMax*yscl);
	}

	int skipCount = 0;
	RecordSequence::const_iterator it = records->begin();

	Seiscomp::Core::Time refTime = (*it)->startTime();

	QPolygon *poly = NULL;

	for(; it != records->end(); ++it) {
		const Record* rec = it->get();

		int nsamp = rec->sampleCount();

		if ( nsamp == 0 ) continue;

		if ( poly == NULL ) {
			push_back(QPolygon());
			poly = &back();
		}

		Array::DataType datatype = rec->dataType();
		switch ( datatype ) {
			case Array::FLOAT:
				pushData<float>(poly, rec, refTime, pixelPerSecond, multiplier, amplOffset, _baseline, yscl);
				break;
			case Array::DOUBLE:
				pushData<double>(poly, rec, refTime, pixelPerSecond, multiplier, amplOffset, _baseline, yscl);
				break;
			case Array::INT:
				pushData<int>(poly, rec, refTime, pixelPerSecond, multiplier, amplOffset, _baseline, yscl);
				break;
			default:
				break;
		}
	}

	if ( poly->isEmpty() )
		pop_back();

	if ( !empty() ) {
		if ( skipCount )
			front().remove(0, skipCount);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordPolyline::createSteps(RecordSequence const *records, double pixelPerSecond,
                                 float amplMin, float amplMax, float amplOffset,
                                 int height, QVector<QPair<int,int> >* gaps) {
	clear();

	if ( records == NULL ) return;
	if ( records->size() == 0 ) return;

	// normalize peak-to-peak amplitude to height set using setHeight()
	double yscl;
	double amplHeight = amplMax - amplMin;

	if ( amplHeight == 0.0 ) {
		_baseline = height/2;
		yscl = 0;
	}
	else {
		yscl = (height-1)/amplHeight;
		_baseline = (int)(amplMax*yscl);
	}

	int skipCount = 0;
	RecordSequence::const_iterator it = records->begin();
	RecordSequence::const_iterator lastIt = it;

	Seiscomp::Core::Time refTime = (*it)->startTime();

	QPolygon *poly = NULL;

	for(; it != records->end(); ++it) {
		const Record* rec = it->get();
		const Record* lastRec = lastIt->get();

		int nsamp = rec->sampleCount();
		if ( nsamp == 0 ) continue;

		double tolerance = records->tolerance()/rec->samplingFrequency();
		double diff;

		try {
			diff = abs(double(rec->startTime() - lastRec->endTime()));
		}
		catch ( ... ) {
			diff = tolerance*2;
		}

		if ( diff > tolerance || poly == NULL ) {
			push_back(QPolygon());
			poly = &back();
		}

		Array::DataType datatype = rec->dataType();
		switch ( datatype ) {
			case Array::FLOAT:
				pushData<float>(poly, rec, refTime, pixelPerSecond, 1.0, amplOffset, _baseline, yscl);
				break;
			case Array::DOUBLE:
				pushData<double>(poly, rec, refTime, pixelPerSecond, 1.0, amplOffset, _baseline, yscl);
				break;
			case Array::INT:
				pushData<int>(poly, rec, refTime, pixelPerSecond, 1.0, amplOffset, _baseline, yscl);
				break;
			default:
				break;
		}

		lastIt = it;
	}

	if ( poly->isEmpty() )
		pop_back();

	if ( !empty() ) {
		if ( skipCount )
			front().remove(0, skipCount);

		if ( gaps ) {
			for ( int i = 1; i < size(); ++i )
				gaps->append(QPair<int,int>((*this)[i-1].last().x(), (*this)[i].first().x()));
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordPolyline::createSteps(RecordSequence const *records,
                                 const Core::Time &start, const Core::Time &end,
                                 double pixelPerSecond,
                                 float amplMin, float amplMax, float amplOffset,
                                 int height, QVector<QPair<int,int> >* gaps) {
	clear();

	if ( records == NULL ) return;
	if ( records->size() == 0 ) return;

	// normalize peak-to-peak amplitude to height set using setHeight()
	double yscl;
	double amplHeight = amplMax - amplMin;

	if ( amplHeight == 0.0 ) {
		_baseline = height/2;
		yscl = 0;
	}
	else {
		yscl = (height-1)/amplHeight;
		_baseline = (int)(amplMax*yscl);
	}

	int skipCount = 0;
	RecordSequence::const_iterator it = records->begin();
	RecordSequence::const_iterator lastIt = it;

	Seiscomp::Core::Time refTime = start;

	QPolygon *poly = NULL;

	for(; it != records->end(); ++it) {
		const Record* rec = it->get();
		const Record* lastRec = lastIt->get();

		// Skip records that are out of time window [start:end]
		try {
			if ( rec->endTime() <= start ) continue;
		}
		catch ( ... ) { continue; }

		if ( rec->startTime() >= end ) break;

		int nsamp = rec->sampleCount();
		if ( nsamp == 0 ) continue;

		double tolerance = records->tolerance()/rec->samplingFrequency();
		double diff;

		try {
			diff = abs(double(rec->startTime() - lastRec->endTime()));
		}
		catch ( ... ) {
			diff = tolerance*2;
		}

		if ( diff > tolerance || poly == NULL ) {
			push_back(QPolygon());
			poly = &back();
		}

		Array::DataType datatype = rec->dataType();
		switch ( datatype ) {
			case Array::FLOAT:
				pushData<float>(poly, rec, refTime, pixelPerSecond, 1.0, amplOffset, _baseline, yscl);
				break;
			case Array::DOUBLE:
				pushData<double>(poly, rec, refTime, pixelPerSecond, 1.0, amplOffset, _baseline, yscl);
				break;
			case Array::INT:
				pushData<int>(poly, rec, refTime, pixelPerSecond, 1.0, amplOffset, _baseline, yscl);
				break;
			default:
				break;
		}

		lastIt = it;
	}

	if ( poly->isEmpty() )
		pop_back();

	if ( !empty() ) {
		if ( skipCount )
			front().remove(0, skipCount);

		if ( gaps ) {
			for ( int i = 1; i < size(); ++i )
				gaps->append(QPair<int,int>((*this)[i-1].last().x(), (*this)[i].first().x()));
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordPolyline::draw(QPainter& p) {
#ifdef DEBUG_POINTS
	int cnt = 0;
#endif
	for ( iterator it = begin(); it != end(); ++it ) {
#ifdef DEBUG_POINTS
		cnt += (*it).size();
#endif
		p.drawPolyline(*it);
	}
#ifdef DEBUG_POINTS
	std::cerr << cnt << " pnt" << std::endl;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordPolyline::drawGaps(QPainter& p, int yofs, int height, const QBrush& gapBrush, const QBrush& overlapBrush) {
	for ( int i = 1; i < size(); ++i ) {
		int x0 = (*this)[i-1].last().x();
		int x1 = (*this)[i].first().x();
		int width = x1 - x0;
		if ( width < 0 ) {
			x0 = x1;
			width = -width;
			p.fillRect(x0, yofs, width, height, overlapBrush);
		}
		else {
			if ( width < 1 ) width = 1;
			p.fillRect(x0, yofs, width, height, gapBrush);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordPolyline::draw(QPainter& p, int yofs, int height, const QBrush& gapBrush, const QBrush& overlapBrush) {
	draw(p);
	drawGaps(p, yofs, height, gapBrush, overlapBrush);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordPolylineF::RecordPolylineF() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordPolylineF::create(Record const *rec, double pixelPerSecond,
                             float amplMin, float amplMax, float amplOffset,
                             int height, float *timingQuality, bool optimization) {
	clear();

	if ( rec == NULL )
		return;

	// normalize peak-to-peak amplitude to height set using setHeight()
	double yscl;
	double amplHeight = amplMax - amplMin;

	if ( amplHeight == 0.0 ) {
		_baseline = height*0.5+0.5;
		yscl = 0;
	}
	else {
		yscl = (height-1)/amplHeight;
		_baseline = amplMax*yscl+0.5;
	}

	QPolygonF *poly = NULL;

	int nsamp = rec->sampleCount();

	if ( nsamp == 0 ) return;

	if ( timingQuality )
		*timingQuality = rec->timingQuality();

	push_back(QPolygonF());
	poly = &back();

	const FloatArray *arr = (const FloatArray*)rec->data();

	float *f = (float*)arr->data();
	float dx = pixelPerSecond / rec->samplingFrequency();

	qreal x_pos, y_pos;

	if ( optimization ) {
		x_pos = 0;
		y_pos = _baseline-yscl*(f[0]-amplOffset);

		qreal y_min = y_pos;
		qreal y_max = y_pos;

		qreal x_out = x_pos;
		qreal y_out = y_pos;

		poly->append(QPointF(x_pos, y_pos));

		int collapsedSamples = 0;

		for ( int i = 1; i < nsamp; ++i ) {
			x_pos = i*dx;
			y_pos = _baseline-yscl*(f[i]-amplOffset);

			if ( (int)x_out == (int)x_pos ) {
				// Points share the same pixel, just collect min/max
				if ( y_pos < y_min ) y_min = y_pos;
				else if ( y_pos > y_max ) y_max = y_pos;
				++collapsedSamples;
			}
			else {
				if ( collapsedSamples ) {
					// We want to draw from y_out to y_pos and taking
					// y_min/y_max into account
					if ( !(y_out <= y_min && y_pos >= y_max) &&
					     !(y_out >= y_max && y_pos <= y_min) ) {
						if ( y_out < y_pos ) {
							poly->append(QPointF(x_out, y_min));
							poly->append(QPointF(x_out, y_max));
						}
						else {
							poly->append(QPointF(x_out, y_max));
							poly->append(QPointF(x_out, y_min));
						}
					}
				}

				poly->append(QPointF(x_pos, y_pos));
				x_out = x_pos;
				y_out = y_min = y_max = y_pos;

				collapsedSamples = 0;
			}
		}

		if ( collapsedSamples ) {
			// We want to draw from y_out to y_pos and taking
			// y_min/y_max into account
			if ( !(y_out <= y_min && y_pos >= y_max) &&
			     !(y_out >= y_max && y_pos <= y_min) ) {
				if ( y_out < y_pos ) {
					poly->append(QPointF(x_out, y_min));
					poly->append(QPointF(x_out, y_max));
				}
				else {
					poly->append(QPointF(x_out, y_max));
					poly->append(QPointF(x_out, y_min));
				}
			}
		}

		if ( x_pos != x_out || y_pos != y_out )
			poly->append(QPointF(x_pos, y_pos));
	}
	else {
		for ( int i = 0; i < nsamp; ++i ) {
			x_pos = i*dx;
			y_pos = _baseline-yscl*(f[i]-amplOffset);
			poly->append(QPointF(x_pos, y_pos));
		}
	}

	if ( poly->isEmpty() )
		pop_back();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordPolylineF::create(RecordSequence const *records, double pixelPerSecond,
                             float amplMin, float amplMax, float amplOffset,
                             int height, float *timingQuality,
                             QVector<QPair<qreal,qreal> >* gaps, bool optimization) {
	static Core::Time invalidTime;
	create(records, invalidTime, invalidTime, pixelPerSecond,
	       amplMin, amplMax, amplOffset, height, timingQuality,
	       gaps, optimization);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordPolylineF::create(RecordSequence const *records,
                             const Core::Time &start, const Core::Time &end,
                             double pixelPerSecond,
                             float amplMin, float amplMax, float amplOffset,
                             int height, float *timingQuality,
                             QVector<QPair<qreal,qreal> >* gaps,
                             bool optimization) {
	clear();

	if ( records == NULL ) return;
	if ( records->size() == 0 ) return;

	// normalize peak-to-peak amplitude to height set using setHeight()
	double yscl;
	double amplHeight = amplMax - amplMin;

	if ( amplHeight == 0.0 ) {
		_baseline = height*0.5+0.5;
		yscl = 0;
	}
	else {
		yscl = (height-1)/amplHeight;
		_baseline = amplMax*yscl+0.5;
	}

	int skipCount = 0;
	RecordSequence::const_iterator it = records->begin();
	RecordSequence::const_iterator lastIt = it;

	QPolygonF *poly = NULL;
	int timingQualityRecordCount = 0;
	if ( timingQuality ) *timingQuality = 0;

	qreal y_min = 0;
	qreal y_max = 0;

	qreal x_out = 0;
	qreal y_out = 0;

	qreal x_pos = 0;
	qreal y_pos = 0;

	int i;
	int collapsedSamples = 0;
	double startOfs = 0;
	double endOfs = 0;

	for( ; it != records->end(); ++it ) {
		const Record *rec = it->get();
		const Record *lastRec = lastIt->get();

		// Skip records that are out of time window [start:end]
		if ( start.valid() ) {
			try {
				if ( rec->endTime() <= start ) continue;
			}
			catch ( ... ) { continue; }
			startOfs = double(start-rec->startTime());
		}
		else
			startOfs = double(records->front()->startTime()-rec->startTime());

		double dt = 1.0/rec->samplingFrequency();
		if ( end.valid() ) {
			if ( rec->startTime() >= end ) break;
			endOfs = double(rec->endTime()-end) - dt;
		}

		if ( timingQuality && rec->timingQuality() >= 0 ) {
			*timingQuality += rec->timingQuality();
			++timingQualityRecordCount;
		}

		int nsamp = rec->sampleCount();
		double tolerance = records->tolerance()/rec->samplingFrequency();
		double diff;

		if ( nsamp == 0 ) continue;

		try {
			diff = abs(double(rec->startTime() - lastRec->endTime()));
		}
		catch ( ... ) {
			diff = tolerance*2;
		}

		const FloatArray *arr = (const FloatArray*)rec->data();
		const float *f = arr->typedData();

		// Cut front samples
		if ( startOfs > 0 ) {
			int sampleOfs = (int)(startOfs * rec->samplingFrequency());
			if ( sampleOfs >= nsamp ) continue;
			f += sampleOfs;
			nsamp -= sampleOfs;
			startOfs -= (sampleOfs * dt);
		}

		// Cut back samples
		if ( endOfs > 0 ) {
			nsamp -= (int)(endOfs * rec->samplingFrequency());
			if ( nsamp <= 0 ) continue;
		}

		qreal x0 = pixelPerSecond*startOfs;
		float dx = pixelPerSecond * dt;

		if ( diff > tolerance || poly == NULL ) {
			push_back(QPolygonF());
			poly = &back();

			x_pos = -x0;
			y_pos = _baseline-yscl*(f[0]-amplOffset);
			y_min = y_pos;
			y_max = y_pos;

			x_out = x_pos;
			y_out = y_pos;

			poly->append(QPointF(x_pos, y_pos));
			i = 1;
		}
		else
			i = 0;

		if ( optimization ) {
			for ( ; i < nsamp; ++i ) {
				x_pos = (i*dx) - x0;
				y_pos = _baseline-yscl*(f[i]-amplOffset);

				if ( (int)x_out == (int)x_pos ) {
					// Points share the same pixel, just collect min/max
					if ( y_pos < y_min ) y_min = y_pos;
					else if ( y_pos > y_max ) y_max = y_pos;
					++collapsedSamples;
				}
				else {
					if ( collapsedSamples ) {
						qreal dist = x_pos - x_out;
						// We want to draw from y_out to y_pos and taking
						// y_min/y_max into account
						if ( !(y_out <= y_min && y_pos >= y_max) &&
						     !(y_out >= y_max && y_pos <= y_min) ) {
							if ( y_out < y_pos ) {
								poly->append(QPointF(x_out+dist*0.33333, y_min));
								poly->append(QPointF(x_out+dist*0.66666, y_max));
							}
							else {
								poly->append(QPointF(x_out+dist*0.33333, y_max));
								poly->append(QPointF(x_out+dist*0.66666, y_min));
							}
						}
					}

					poly->append(QPointF(x_pos, y_pos));

					x_out = x_pos;
					y_out = y_min = y_max = y_pos;

					collapsedSamples = 0;
				}
			}
		}
		else {
			for ( ; i < nsamp; ++i ) {
				x_pos = (i*dx) - x0;
				y_pos = _baseline-yscl*(f[i]-amplOffset);
				poly->append(QPointF(x_pos, y_pos));
			}
		}

		if ( poly->isEmpty() )
			pop_back();

		lastIt = it;
	}

	if ( optimization && poly ) {
		if ( collapsedSamples ) {
			qreal dist = x_pos - x_out;
			// We want to draw from y_out to y_pos and taking
			// y_min/y_max into account
			if ( !(y_out <= y_min && y_pos >= y_max) &&
			     !(y_out >= y_max && y_pos <= y_min) ) {
				if ( y_out < y_pos ) {
					poly->append(QPointF(x_out+dist*0.33333, y_min));
					poly->append(QPointF(x_out+dist*0.66666, y_max));
				}
				else {
					poly->append(QPointF(x_out+dist*0.33333, y_max));
					poly->append(QPointF(x_out+dist*0.66666, y_min));
				}
			}
		}

		if ( x_pos != x_out || y_pos != y_out )
			poly->append(QPointF(x_pos, y_pos));
	}

	if ( !empty() ) {
		if ( skipCount )
			front().remove(0, skipCount);

		if ( gaps ) {
			for ( int i = 1; i < size(); ++i )
				gaps->append(QPair<qreal,qreal>((*this)[i-1].last().x(), (*this)[i].first().x()));
		}
	}

	if ( timingQuality ) {
		if ( timingQualityRecordCount )
			*timingQuality /= timingQualityRecordCount;
		else
			*timingQuality = -1;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordPolylineF::draw(QPainter &p) {
#ifdef DEBUG_POINTS
	int cnt = 0;
#endif
	for ( iterator it = begin(); it != end(); ++it ) {
#ifdef DEBUG_POINTS
		cnt += (*it).size();
#endif
		p.drawPolyline(*it);
	}
#ifdef DEBUG_POINTS
	std::cerr << cnt << " pnt" << std::endl;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordPolylineF::drawGaps(QPainter &p, int yofs, int height,
                               const QBrush &gapBrush,
                               const QBrush &overlapBrush) {
	for ( int i = 1; i < size(); ++i ) {
		qreal x0 = (*this)[i-1].last().x();
		qreal x1 = (*this)[i].first().x();
		qreal width = x1 - x0;
		if ( width < 0 ) {
			x0 = x1;
			width = -width;
			p.fillRect(x0, yofs, width, height, overlapBrush);
		}
		else {
			if ( width < 1 ) width = 1;
			p.fillRect(x0, yofs, width, height, gapBrush);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void RecordPolylineF::draw(QPainter &p, int yofs, int height,
                           const QBrush &gapBrush,
                           const QBrush &overlapBrush) {
	draw(p);
	drawGaps(p, yofs, height, gapBrush, overlapBrush);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
