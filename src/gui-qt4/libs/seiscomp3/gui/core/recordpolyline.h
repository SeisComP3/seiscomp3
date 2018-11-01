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



#ifndef __SEISCOMP_GUI_RECORDPOLYLINE_H__
#define __SEISCOMP_GUI_RECORDPOLYLINE_H__


#include <QPen>
#include <QPolygon>
#include <QPainter>
#include <QVector>

#ifndef Q_MOC_RUN
#include <seiscomp3/core/record.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/recordsequence.h>
#endif
#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {
namespace Gui {


DEFINE_SMARTPOINTER(AbstractRecordPolyline);
class SC_GUI_API AbstractRecordPolyline : public Seiscomp::Core::BaseObject {
	public:
		virtual void draw(QPainter &) = 0;
		virtual void drawGaps(QPainter &, int yofs, int height, const QBrush &gapBrush, const QBrush &overlapBrush) = 0;
		virtual void draw(QPainter &, int yofs, int height, const QBrush &gapBrush, const QBrush &overlapBrush) = 0;
		virtual bool isEmpty() const = 0;

		qreal baseline() const;

	protected:
		qreal _baseline;
};


DEFINE_SMARTPOINTER(RecordPolyline);
class SC_GUI_API RecordPolyline : public AbstractRecordPolyline,
                                  public QVector<QPolygon> {
	public:
		RecordPolyline();


	public:
		//! creates the record polyline and returns the virtual height
		//! of that polyline
		void create(Record const *, double pixelPerSecond,
		            float amplMin, float amplMax, float amplOffset,
		            int height, float *timingQuality = NULL,
		            bool optimization = true);

		//! creates the record polyline and returns the virtual height
		//! of that polyline
		void create(RecordSequence const *, double pixelPerSecond,
		            float amplMin, float amplMax, float amplOffset,
		            int height, float *timingQuality = NULL,
		            QVector<QPair<int,int> >* gaps = NULL,
		            bool optimization = true);

		void create(RecordSequence const *,
		            const Core::Time &start,
		            const Core::Time &end,
		            double pixelPerSecond,
		            float amplMin, float amplMax, float amplOffset,
		            int height, float *timingQuality = NULL,
		            QVector<QPair<int,int> >* gaps = NULL,
		            bool optimization = true);

		void createStepFunction(RecordSequence const *, double pixelPerSecond,
		                        float amplMin, float amplMax, float amplOffset,
		                        int height, float multiplier = 1.0);

		void createSteps(RecordSequence const *, double pixelPerSecond,
		                 float amplMin, float amplMax, float amplOffset,
		                 int height, QVector<QPair<int,int> >* gaps = NULL);

		void createSteps(RecordSequence const *,
		                 const Core::Time &start,
		                 const Core::Time &end,
		                 double pixelPerSecond,
		                 float amplMin, float amplMax, float amplOffset,
		                 int height, QVector<QPair<int,int> >* gaps = NULL);


	public:
		void draw(QPainter&);
		void drawGaps(QPainter &, int yofs, int height, const QBrush &gapBrush, const QBrush &overlapBrush);
		void draw(QPainter &, int yofs, int height, const QBrush &gapBrush, const QBrush &overlapBrush);
		bool isEmpty() const { return QVector<QPolygon>::isEmpty(); }
};


DEFINE_SMARTPOINTER(RecordPolylineF);
class SC_GUI_API RecordPolylineF : public AbstractRecordPolyline,
                                   public QVector<QPolygonF> {
	public:
		RecordPolylineF();


	public:
		//! creates the record polyline and returns the virtual height
		//! of that polyline
		void create(Record const *, double pixelPerSecond,
		            float amplMin, float amplMax, float amplOffset,
		            int height, float *timingQuality = NULL,
		            bool optimization = true);

		//! creates the record polyline and returns the virtual height
		//! of that polyline
		void create(RecordSequence const *, double pixelPerSecond,
		            float amplMin, float amplMax, float amplOffset,
		            int height, float *timingQuality = NULL,
		            QVector<QPair<qreal,qreal> >* gaps = NULL,
		            bool optimization = true);

		void create(RecordSequence const *,
		            const Core::Time &start,
		            const Core::Time &end,
		            double pixelPerSecond,
		            float amplMin, float amplMax, float amplOffset,
		            int height, float *timingQuality = NULL,
		            QVector<QPair<qreal,qreal> >* gaps = NULL,
		            bool optimization = true);


	public:
		void draw(QPainter &);
		void drawGaps(QPainter &, int yofs, int height, const QBrush &gapBrush, const QBrush &overlapBrush);
		void draw(QPainter &, int yofs, int height, const QBrush &gapBrush, const QBrush &overlapBrush);
		bool isEmpty() const { return QVector<QPolygonF>::isEmpty(); }
};


inline qreal AbstractRecordPolyline::baseline() const {
	return _baseline;
}



}
}


# endif
