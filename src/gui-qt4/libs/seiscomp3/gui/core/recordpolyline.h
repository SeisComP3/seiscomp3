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



#ifndef _RECORDPOLYLINE_H_
#define _RECORDPOLYLINE_H_

#include <vector>

#include <QPen>
#include <QPolygon>
#include <QPainter>

#ifndef Q_MOC_RUN
#include <seiscomp3/core/record.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/recordsequence.h>
#endif
#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {
namespace Gui {


#define SPLOT_NORMALIZE  1
#define SPLOT_WIGGLE     2 // FIXME probably obsolete

class SC_GUI_API RecordPolyline : public std::vector<QPolygon>
{
  public:
	RecordPolyline();

	//! translate the polyline w.r.t. the *original* coordinates
	void translate(int x, int y);

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

	void draw(QPainter&);
	void drawGaps(QPainter&, int yofs, int height, const QColor& gapColor, const QColor& overlapColor);
	void draw(QPainter&, int yofs, int height, const QColor& gapColor, const QColor& overlapColor);

	int baseline() const;

  private:
	int _tx, _ty;
	int _baseline;
};


}
}


# endif
