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



#ifndef __SEISCOMP_GUI_CONNECTIONSTATELABEL_H__
#define __SEISCOMP_GUI_CONNECTIONSTATELABEL_H__


#include <seiscomp3/gui/qt4.h>

#include <QLabel>
#include <QPixmap>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API ConnectionStateLabel : public QLabel {
	Q_OBJECT

	public:
		ConnectionStateLabel(QWidget *parent = 0, Qt::WindowFlags f = 0);

	public slots:
		void start();
		void stop();


	signals:
		void customInfoWidgetRequested(const QPoint &pos);


	protected:
		void mousePressEvent(QMouseEvent *event);


	private:
		QPixmap _connected;
		QPixmap _disconnected;

};


}
}


#endif
