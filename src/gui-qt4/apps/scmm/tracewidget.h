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


#ifndef __TRACEWIDGET_H__
#define __TRACEWIDGET_H__

#include <QtGui>
#include <QFrame>

namespace Seiscomp {
namespace Gui {

class TraceWidget : public QFrame {
	Q_OBJECT

	public:
		TraceWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);

	public:
		void setValueCount(int count);
		int valueCount() const;

		void addValue(int value);
		int value(int index);

		void clear();

	protected:
		void paintEvent(QPaintEvent *event);

	private:
		QVector<int> _values;
		int _max;
		int _offset;
		int _count;
		int _sum;
		int _collectedValues;
};


}
}

#endif
