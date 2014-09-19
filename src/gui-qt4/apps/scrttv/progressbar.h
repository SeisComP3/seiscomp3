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




#ifndef __SEISCOMP_GUI_PROGRESSBAR_H__
#define __SEISCOMP_GUI_PROGRESSBAR_H__

#include <QPainter>
#include <QPaintEvent>
#include <QFrame>

class QWidget;


namespace Seiscomp {
namespace Gui {


class ProgressBar : public QFrame
{
  Q_OBJECT

	public:
		ProgressBar(QWidget *parent=0)
		: QFrame(parent)
		{
			setMinimumSize(50,10);
			_value = 0;
		}

	public slots:
		void reset()
		{
			_value = 0;
			update();
		}

		void setValue(int val)
		{
			_value = val;
			if (_value>100)
				_value = 100;
			repaint();
		}

	protected:
		void paintEvent(QPaintEvent *)
		{
			int w=width(), h=height();
			QPainter paint(this);
			paint.fillRect(0, 0, int(w*_value / 100), h, QColor(0,0,128));
		}

	private:
		int _value;
};


}
}


#endif
