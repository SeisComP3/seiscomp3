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



#include <iostream>
using namespace std;

#include "recordpolyline.h"

namespace {

std::ostream &operator << (std::ostream &os, const Seiscomp::Core::Time &t);

/*
static void optimize1(int &n, int *pt)
{
	int *tmp = new int[n+n], *pt2=tmp;

	int i=0, n2=1, xprev;
	int yin, ymin, ymax, yout;

	*(pt2++) = xprev = pt[i++];
	*(pt2++) = ymin = ymax = yin = yout = pt[i++];

	while(i<n+n) {
		int x = pt[i++];
		int y = pt[i++];

		if(x==xprev) {
			if(y<ymin)      ymin=y;
			else if(y>ymax) ymax=y;
			yout = y;
		}
		else {
			// entering new column
			*(pt2++) = x;
			*(pt2++) = yin;
			n2++;

			if(ymin<yin) {
				*(pt2++) = xprev;
				*(pt2++) = ymin;
				n2++;
			}
			if(ymax>yin) {
				*(pt2++) = xprev;
				*(pt2++) = ymax;
				n2++;
			}
			if(yout>ymin && yout<ymax) {
				*(pt2++) = xprev;
				*(pt2++) = yout;
				n2++;
			}
			xprev = x;
			yin = ymin = ymax = yout = y;
		}
	}
	// copy back
	n = n2;
	pt2 = tmp;
	for (i=0; i<n2+n2; i++) {
		pt[i] = pt2[i];
	}

	delete [] tmp;
}

static void optimize2(int &n, int *pt)
{	// eliminate points, where y[i-1]==y[i]==y[i+1]
return; // FIXME doesn't work very well yet
	int nn = n+n;
	int *tmp = new int[nn], *pt2=tmp;
	int i=0, n2=1, xprev, yprev;

	*(pt2++) = xprev = pt[i++];
	*(pt2++) = yprev = pt[i++];

	while(i<nn) {
		*(pt2)   = pt[i++];
		*(pt2+1) = pt[i++];
		if(*(pt2+1) == yprev)
			continue;

		yprev = *(pt2+1);
		pt2+=2;
		n2++;
	}
	// copy back
	n = n2;
	pt2 = tmp;
	for (i=0; i<n2+n2; i++) {
		pt[i] = pt2[i];
	}

	delete [] tmp;
}
*/

}


namespace Seiscomp {
namespace Gui {


RecordPolyline::RecordPolyline()
{
	_tx = _ty = 0;
}

void RecordPolyline::translate(int x, int y)
{
	for ( iterator it = begin(); it != end(); ++it )
		(*it).translate(x-_tx, y-_ty);

	_tx = x;  _ty = y;
}


void RecordPolyline::create(Record const *rec,
                            double pixelPerSecond,
                            float amplMin, float amplMax, float amplOffset,
                            int height, float *timingQuality,
                            bool optimization) {
	if (rec == NULL)
		return;

	// normalize peak-to-peak amplitude to height set using setHeight()
	double yscl;
	double amplHeight = amplMax - amplMin;

	if ( amplHeight == 0.0 ) {
		_baseline = height/2;
		yscl = 0;
	}
	else {
		yscl = height/amplHeight;
		_baseline = (int)(amplMax*yscl);
	}

	clear();

	QPolygon *poly = NULL;

	int nsamp = rec->sampleCount();

	if ( nsamp == 0 ) return;

	if ( timingQuality )
		*timingQuality = rec->timingQuality();

	push_back(QPolygon());
	poly = &back();

	const FloatArray *arr = (const FloatArray*)rec->data();

	float *f = (float*)arr->data();
	int x0 = 0;
	float dx = pixelPerSecond / rec->samplingFrequency();

	int x_prev = -x0;
	int y_prev = int(_baseline-yscl*(f[0]-amplOffset));
	int y_min = y_prev;
	int y_max = y_prev;

	int x_out = x_prev;
	int y_out = y_prev;

	poly->append(QPoint(x_prev, y_prev));

	if ( optimization ) {
		for (int i = 1; i<nsamp; i++) {
			int x_pos = int(i*dx) - x0;
			int y_pos = int(_baseline-yscl*(f[i]-amplOffset));

			if ( y_pos != y_out ) {
				// last output differs from the last sample?
				if ( x_out != x_prev  ) {
					x_out = x_prev;
					poly->append(QPoint(x_out, y_out));
				}

				// last output differs from the current draw position?
				if ( x_out != x_pos ) {
					if ( y_min != y_out ) {
						y_out = y_min;
						poly->append(QPoint(x_out, y_out));
					}
					if ( y_max != y_out ) {
						y_out = y_max;
						poly->append(QPoint(x_out, y_out));
					}
					if ( y_prev != y_out ) {
						y_out = y_prev;
						poly->append(QPoint(x_out, y_out));
					}

					x_out = x_pos;

					if ( y_pos != y_out ) {
						y_out = y_pos;
						poly->append(QPoint(x_out, y_pos));
					}

					y_min = y_max = y_out;
				}
				else {
					// update y min/max range
					if ( y_pos < y_min ) y_min = y_pos;
					else if ( y_pos > y_max ) y_max = y_pos;
				}
			}
			else {
				if ( y_min != y_out ) {
					y_out = y_min;
					poly->append(QPoint(x_out, y_out));
					y_min = y_out;
				}
				if ( y_max != y_out ) {
					y_out = y_max;
					poly->append(QPoint(x_out, y_out));
					y_max = y_out;
				}
				if ( y_prev != y_out ) {
					y_out = y_min = y_max = y_prev;
					poly->append(QPoint(x_out, y_out));
				}
			}

			x_prev = x_pos;
			y_prev = y_pos;
		}

		if ( x_out != x_prev )
			poly->append(QPoint(x_prev, y_prev));
	}
	else {
		for (int i = 1; i<nsamp; i++) {
			int x_pos = int(i*dx) - x0;
			int y_pos = int(_baseline-yscl*(f[i]-amplOffset));
			poly->append(QPoint(x_pos, y_pos));
		}
	}

	if ( poly->isEmpty() )
		pop_back();

	_tx = _ty = 0;
}



void RecordPolyline::create(RecordSequence const *records,
                            double pixelPerSecond,
                            float amplMin, float amplMax, float amplOffset,
                            int height, float *timingQuality,
                            QVector<QPair<int,int> >* gaps,
                            bool optimization) {
	if (records == NULL)
		return;
	if (records->size() == 0)
		return;

	// normalize peak-to-peak amplitude to height set using setHeight()
	double yscl;
	double amplHeight = amplMax - amplMin;

	if ( amplHeight == 0.0 ) {
		_baseline = height/2;
		yscl = 0;
	}
	else {
		yscl = height/amplHeight;
		_baseline = (int)(amplMax*yscl);
	}

	int skipCount = 0;
	RecordSequence::const_iterator it = records->begin();
	RecordSequence::const_iterator lastIt = it;

	/*
	if ( it != records->end() ) {
		float dx = pixelPerSecond / (*it)->samplingFrequency();
		skipCount = int((*it)->sampleCount()*dx);
	}
	*/

	clear();

	Seiscomp::Core::Time refTime = (*it)->startTime();

	QPolygon *poly = NULL;
	int timingQualityRecordCount = 0;
	if ( timingQuality ) *timingQuality = 0;

	for(; it != records->end(); ++it) {
		const Record* rec = it->get();
		const Record* lastRec = lastIt->get();

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

		if ( diff > tolerance || poly == NULL ) {
			push_back(QPolygon());
			poly = &back();
		}

		const FloatArray *arr = (const FloatArray*)rec->data();

		float *f = (float*)arr->data();
		int x0 = int(pixelPerSecond*double(/*referenceTime*/refTime-rec->startTime()));
		float dx = pixelPerSecond / rec->samplingFrequency();

		int x_prev = -x0;
		int y_prev = int(_baseline-yscl*(f[0]-amplOffset));
		int y_min = y_prev;
		int y_max = y_prev;

		int x_out = x_prev;
		int y_out = y_prev;

		poly->append(QPoint(x_prev, y_prev));

		if ( optimization ) {
			for (int i = 1; i<nsamp; i++) {
				int x_pos = int(i*dx) - x0;
				int y_pos = int(_baseline-yscl*(f[i]-amplOffset));

				if ( y_pos != y_out ) {
					// last output differs from the last sample?
					if ( x_out != x_prev  ) {
						x_out = x_prev;
						poly->append(QPoint(x_out, y_out));
					}

					// last output differs from the current draw position?
					if ( x_out != x_pos ) {
						if ( y_min != y_out ) {
							y_out = y_min;
							poly->append(QPoint(x_out, y_out));
						}
						if ( y_max != y_out ) {
							y_out = y_max;
							poly->append(QPoint(x_out, y_out));
						}
						if ( y_prev != y_out ) {
							y_out = y_prev;
							poly->append(QPoint(x_out, y_out));
						}

						x_out = x_pos;

						if ( y_pos != y_out ) {
							y_out = y_pos;
							poly->append(QPoint(x_out, y_pos));
						}

						y_min = y_max = y_out;
					}
					else {
						// update y min/max range
						if ( y_pos < y_min ) y_min = y_pos;
						else if ( y_pos > y_max ) y_max = y_pos;
					}
				}
				else {
					if ( y_min != y_out ) {
						y_out = y_min;
						poly->append(QPoint(x_out, y_out));
						y_min = y_out;
					}
					if ( y_max != y_out ) {
						y_out = y_max;
						poly->append(QPoint(x_out, y_out));
						y_max = y_out;
					}
					if ( y_prev != y_out ) {
						y_out = y_min = y_max = y_prev;
						poly->append(QPoint(x_out, y_out));
					}
				}

				x_prev = x_pos;
				y_prev = y_pos;
			}

			if ( x_out != x_prev )
				poly->append(QPoint(x_prev, y_prev));
		}
		else {
			for (int i = 1; i<nsamp; i++) {
				int x_pos = int(i*dx) - x0;
				int y_pos = int(_baseline-yscl*(f[i]-amplOffset));
				poly->append(QPoint(x_pos, y_pos));
			}
		}

		if ( poly->isEmpty() )
			pop_back();

		lastIt = it;
	}

	if ( !empty() ) {
		if ( skipCount )
			front().remove(0, skipCount);

		if ( gaps ) {
			for ( size_t i = 1; i < size(); ++i )
				gaps->append(QPair<int,int>((*this)[i-1].last().x(), (*this)[i].first().x()));
		}
	}

	_tx = _ty = 0;

	if ( timingQuality ) {
		if ( timingQualityRecordCount )
			*timingQuality /= timingQualityRecordCount;
		else
			*timingQuality = -1;
	}
}


void RecordPolyline::create(RecordSequence const *records,
                            const Core::Time &start,
                            const Core::Time &end,
                            double pixelPerSecond,
                            float amplMin, float amplMax, float amplOffset,
                            int height, float *timingQuality,
                            QVector<QPair<int,int> >* gaps,
                            bool optimization) {
	clear();

	if ( records == NULL )
		return;

	if ( records->size() == 0 )
		return;

	// normalize peak-to-peak amplitude to height set using setHeight()
	double yscl;
	double amplHeight = amplMax - amplMin;

	if ( amplHeight == 0.0 ) {
		_baseline = height/2;
		yscl = 0;
	}
	else {
		yscl = height/amplHeight;
		_baseline = (int)((amplMax-amplOffset)*yscl);
	}

	int skipCount = 0;
	RecordSequence::const_iterator it = records->begin();
	RecordSequence::const_iterator lastIt = it;

	QPolygon *poly = NULL;
	int timingQualityRecordCount = 0;
	if ( timingQuality ) *timingQuality = 0;

	for( ; it != records->end(); ++it ) {
		const Record* rec = it->get();
		const Record* lastRec = lastIt->get();

		// Skip records that are out of time window [start:end]
		try {
			if ( rec->endTime() <= start ) continue;
		}
		catch ( ... ) { continue; }

		if ( rec->startTime() >= end ) break;

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

		if ( diff > tolerance || poly == NULL ) {
			push_back(QPolygon());
			poly = &back();
		}

		const FloatArray *arr = (const FloatArray*)rec->data();

		float *f = (float*)arr->data();
		double startOfs = double(start-rec->startTime());
		double endOfs = double(rec->endTime()-end);

		// Cut front samples
		if ( startOfs > 0 ) {
			int sampleOfs = (int)(startOfs * rec->samplingFrequency());
			if ( sampleOfs >= nsamp ) continue;
			f += sampleOfs;
			nsamp -= sampleOfs;
			startOfs = 0;
		}

		// Cut back samples
		if ( endOfs > 0 ) {
			nsamp -= (int)(endOfs * rec->samplingFrequency());
			if ( nsamp <= 0 ) continue;
		}

		int x0 = int(pixelPerSecond*startOfs);
		float dx = pixelPerSecond / rec->samplingFrequency();

		int x_prev = -x0;
		int y_prev = int(_baseline-yscl*(f[0]-amplOffset));
		int y_min = y_prev;
		int y_max = y_prev;

		int x_out = x_prev;
		int y_out = y_prev;

		poly->append(QPoint(x_prev, y_prev));

		if ( optimization ) {
			for (int i = 1; i<nsamp; i++) {
				int x_pos = int(i*dx) - x0;
				int y_pos = int(_baseline-yscl*(f[i]-amplOffset));

				if ( y_pos != y_out ) {
					// last output differs from the last sample?
					if ( x_out != x_prev  ) {
						x_out = x_prev;
						poly->append(QPoint(x_out, y_out));
					}

					// last output differs from the current draw position?
					if ( x_out != x_pos ) {
						if ( y_min != y_out ) {
							y_out = y_min;
							poly->append(QPoint(x_out, y_out));
						}
						if ( y_max != y_out ) {
							y_out = y_max;
							poly->append(QPoint(x_out, y_out));
						}
						if ( y_prev != y_out ) {
							y_out = y_prev;
							poly->append(QPoint(x_out, y_out));
						}

						x_out = x_pos;

						if ( y_pos != y_out ) {
							y_out = y_pos;
							poly->append(QPoint(x_out, y_pos));
						}

						y_min = y_max = y_out;
					}
					else {
						// update y min/max range
						if ( y_pos < y_min ) y_min = y_pos;
						else if ( y_pos > y_max ) y_max = y_pos;
					}
				}
				else {
					if ( y_min != y_out ) {
						y_out = y_min;
						poly->append(QPoint(x_out, y_out));
						y_min = y_out;
					}
					if ( y_max != y_out ) {
						y_out = y_max;
						poly->append(QPoint(x_out, y_out));
						y_max = y_out;
					}
					if ( y_prev != y_out ) {
						y_out = y_min = y_max = y_prev;
						poly->append(QPoint(x_out, y_out));
					}
				}

				x_prev = x_pos;
				y_prev = y_pos;
			}

			if ( x_out != x_prev )
				poly->append(QPoint(x_prev, y_prev));
		}
		else {
			for (int i = 1; i<nsamp; i++) {
				int x_pos = int(i*dx) - x0;
				int y_pos = int(_baseline-yscl*(f[i]-amplOffset));
				poly->append(QPoint(x_pos, y_pos));
			}
		}

		if ( poly->isEmpty() )
			pop_back();

		lastIt = it;
	}

	if ( !empty() ) {
		if ( skipCount )
			front().remove(0, skipCount);

		if ( gaps ) {
			for ( size_t i = 1; i < size(); ++i )
				gaps->append(QPair<int,int>((*this)[i-1].last().x(), (*this)[i].first().x()));
		}
	}

	_tx = _ty = 0;

	if ( timingQuality ) {
		if ( timingQualityRecordCount )
			*timingQuality /= timingQualityRecordCount;
		else
			*timingQuality = -1;
	}
}


namespace {

template <typename T>
inline
void pushData(QPolygon *poly, const Record *rec, const Core::Time refTime,
              double pixelPerSecond, float multiplier, float amplOffset,
              int baseline, double yscl)
{
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


void RecordPolyline::createStepFunction(RecordSequence const *records, double pixelPerSecond,
                                        float amplMin, float amplMax, float amplOffset,
                                        int height, float multiplier) {
	clear();

	if (records == NULL) return;
	if (records->size() == 0) return;

	// normalize peak-to-peak amplitude to height set using setHeight()
	double yscl;
	double amplHeight = amplMax - amplMin;

	if ( amplHeight == 0.0 ) {
		_baseline = height/2;
		yscl = 0;
	}
	else {
		yscl = height/amplHeight;
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

	_tx = _ty = 0;
}


void RecordPolyline::draw(QPainter& p) {
	for ( iterator it = begin(); it != end(); ++it ) {
		p.drawPolyline(*it);

		/*
		printf("Points\n");
		for ( int i = 0; i < it->count(); ++i ) {
			p.drawEllipse((*it)[i].x()-2, (*it)[i].y()-2, 4, 4);
			printf(" %d / %d\n", (*it)[i].x(), (*it)[i].y());
		}
		*/
	}
}

void RecordPolyline::drawGaps(QPainter& p, int yofs, int height, const QColor& gapColor, const QColor& overlapColor) {
	for ( size_t i = 1; i < size(); ++i ) {
		int x0 = (*this)[i-1].last().x();
		int x1 = (*this)[i].first().x();
		int width = x1 - x0;
		if ( width < 0 ) {
			x0 = x1;
			width = -width;
			p.fillRect(x0, _ty + yofs, width, height, overlapColor);
		}
		else {
			if ( width < 1 ) width = 1;
			p.fillRect(x0, _ty + yofs, width, height, gapColor);
		}
	}
}

void RecordPolyline::draw(QPainter& p, int yofs, int height, const QColor& gapColor, const QColor& overlapColor) {
	draw(p);
	drawGaps(p, yofs, height, gapColor, overlapColor);
}

int RecordPolyline::baseline() const {
	return _baseline;
}


}
}
