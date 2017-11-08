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



#ifndef __SEISCOMP_GUI_UTIL_H__
#define __SEISCOMP_GUI_UTIL_H__


#include <seiscomp3/gui/qt4.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/defs.h>
#include <QWidget>
#include <vector>


class QLabel;


namespace Seiscomp {
namespace Gui {


SC_GUI_API extern QChar degrees;

SC_GUI_API extern std::string colorConvertError;

SC_GUI_API bool fromString(QColor &value, const std::string &str);
SC_GUI_API QColor readColor(const std::string &query, const std::string &str,
                            const QColor &base, bool *ok = NULL);

SC_GUI_API Qt::PenStyle stringToPenStyle(const std::string &str);
SC_GUI_API Qt::PenStyle readPenStyle(const std::string &query, const std::string &str,
                                     Qt::PenStyle base, bool *ok = NULL);

SC_GUI_API Qt::BrushStyle stringToBrushStyle(const std::string &str);
SC_GUI_API Qt::BrushStyle readBrushStyle(const std::string &query, const std::string &str,
                                         Qt::BrushStyle base, bool *ok = NULL);

SC_GUI_API QString latitudeToString(double lat, bool withValue = true, bool withUnit = true, int precision = 2);
SC_GUI_API QString longitudeToString(double lon, bool withValue = true, bool withUnit = true, int precision = 2);
SC_GUI_API QString depthToString(double depth, int precision = 0);
SC_GUI_API QString timeToString(const Core::Time &t, const char *fmt, bool addTimeZone = false);
SC_GUI_API void timeToLabel(QLabel *label, const Core::Time &t, const char *fmt, bool addTimeZone = false);
SC_GUI_API QString elapsedTimeString(const Core::TimeSpan &dt);

SC_GUI_API void setMaxWidth(QWidget *w, int numCharacters);
SC_GUI_API void fixWidth(QWidget *w, int numCharacters);


class SC_GUI_API ElideFadeDrawer : public QObject {
	public:
		ElideFadeDrawer(QObject *parent = 0);

	protected:
		bool eventFilter(QObject *obj, QEvent *event);
};


class SC_GUI_API EllipsisDrawer : public QObject {
	public:
		EllipsisDrawer(QObject *parent = 0);

	protected:
		bool eventFilter(QObject *obj, QEvent *event);
};


template <typename T>
class ObjectChangeList : public std::vector<std::pair<typename Core::SmartPointer<T>::Impl, bool> > {
};


}
}


#endif
