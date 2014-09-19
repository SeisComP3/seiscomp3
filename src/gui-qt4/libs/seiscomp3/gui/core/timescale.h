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



#ifndef _TIMESCALE_H_
#define _TIMESCALE_H_

#include "ruler.h"
#include <seiscomp3/core/datetime.h>

#define  REPAINT_WITHOUT_ERASE   FALSE
#define  REPAINT_AFTER_ERASE     TRUE

namespace Seiscomp {
namespace Gui {

class SC_GUI_API TimeScale : public Ruler {
	Q_OBJECT

	public:
		TimeScale(QWidget *parent = 0, Qt::WindowFlags f = 0, Position pos = Bottom);
		~TimeScale(){}

		void setTimeRange(double tmin, double tmax) {
			setRange(tmin, tmax);
		}

		double tmin() const { return _min; }
		double tmax() const { return _max; }

		void setAlignment(const Core::Time& t);
		const Core::Time &alignment() const { return _alignment; }


	public slots:
		void setAbsoluteTimeEnabled(bool absoluteTime, bool absoluteDate = true);

	protected:
		bool getTickText(double pos, double lastPos,
		                 int line, QString &str) const;
		void updateIntervals();

	private:
		Core::Time  _alignment;
		bool        _showAbsoluteValues;
		bool        _showAbsoluteDate;
		const char *_primaryTimeFormat;
		const char *_secondaryTimeFormat;
		const char *_relativeTimeFormat;
};


}
}

# endif
