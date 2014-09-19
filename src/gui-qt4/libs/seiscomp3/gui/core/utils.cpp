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


#include <cstdio>
#include <cmath>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/gui/core/utils.h>

#include <QEvent>
#include <QLabel>
#include <QPainter>


namespace Seiscomp {
namespace Gui {

QChar degrees = (ushort)0x00b0;
std::string colorConvertError;

bool fromString(QColor& value, const std::string& str) {
	if ( str.empty() ) {
		colorConvertError = "invalid color: empty string";
		return false;
	}

	std::string col = str;

	int red = 0, green = 0, blue = 0, alpha = 255;

	if ( col.substr(0,4) == "rgb(" ) {
		if ( col[col.size()-1] != ')' ) {
			colorConvertError = std::string("invalid color ") + col + ": missing closing bracket";
			return false;
		}
		col = col.substr(4, col.size()-4-1);

		std::vector<std::string> toks;
		Core::split(toks, col.c_str(), ",");
		if ( toks.size() != 3 ) {
			colorConvertError = std::string("invalid color ") + col + ": rgb() expects 3 components";
			return false;
		}

		bool ok = true;
		ok = Core::fromString(red, toks[0]) && ok;
		ok = Core::fromString(green, toks[1]) && ok;
		ok = Core::fromString(blue, toks[2]) && ok;
		alpha = 255;

		if ( !ok ) {
			colorConvertError = std::string("invalid color ") + col + ": wrong format of component inside rgb()";
			return false;
		}
	}
	else if ( col.substr(0,5) == "rgba(" ) {
		if ( col[col.size()-1] != ')' ) {
			colorConvertError = std::string("invalid color ") + col + ": missing closing bracket";
			return false;
		}
		col = col.substr(5, col.size()-5-1);

		std::vector<std::string> toks;
		Core::split(toks, col.c_str(), ",");
		if ( toks.size() != 4 ) {
			colorConvertError = std::string("invalid color ") + col + ": rgba() expects 4 components";
			return false;
		}

		bool ok = true;
		ok = Core::fromString(red, toks[0]) && ok;
		ok = Core::fromString(green, toks[1]) && ok;
		ok = Core::fromString(blue, toks[2]) && ok;
		ok = Core::fromString(alpha, toks[3]) && ok;

		if ( !ok ) {
			colorConvertError = std::string("invalid color ") + col + ": wrong format of component inside rgba()";
			return false;
		}
	}
	else {
		if ( col.size() != 6 && col.size() != 8 ) {
			colorConvertError = std::string("invalid color ") + col + ": expected 6 or 8 characters";
			return false;
		}

		int readItems = sscanf(col.c_str(), "%2X%2X%2X%2X", 
				(unsigned int*)&red, (unsigned int*)&green, 
				(unsigned int*)&blue, (unsigned int*)&alpha);
		if ( readItems < 3 ) {
			colorConvertError = std::string("invalid color ") + col + ": wrong format";
			return false;
		}
	}

	value.setRed(red);
	value.setGreen(green);
	value.setBlue(blue);
	value.setAlpha(alpha);

	return true;
}


QString latitudeToString(double lat, bool withValue, bool withUnit, int precision) {
	if ( withValue && withUnit )
		return QString("%1%2 %3")
			.arg(fabs(lat), 0, 'f', precision)
			.arg(degrees)
			.arg(lat >= 0.0?"N":"S");
	else if ( !withValue )
		return QString("%1 %2")
			.arg(degrees)
			.arg(lat >= 0.0?"N":"S");
	else if ( !withUnit )
		return QString("%1")
			.arg(fabs(lat), 0, 'f', precision);

	return "";
}

QString longitudeToString(double lon, bool withValue, bool withUnit, int precision) {
	if ( withValue && withUnit )
		return QString("%1%2 %3")
			.arg(fabs(lon), 0, 'f', precision)
			.arg(degrees)
			.arg(lon >= 0.0?"E":"W");
	else if ( !withValue )
		return QString("%1 %2")
			.arg(degrees)
			.arg(lon >= 0.0?"E":"W");
	else if ( !withUnit )
		return QString("%1")
			.arg(fabs(lon), 0, 'f', precision);

	return "";
}

QString depthToString(double depth, int precision) {
	return QString("%1")
		.arg(depth, 0, 'f', precision);
}


QString elapsedTimeString(const Core::TimeSpan &dt) {
	int d=0, h=0, m=0, s=0;
	QLatin1Char fill('0');
	dt.elapsedTime(&d, &h, &m, &s);
	if (d)
		return QString("O.T. +%1d %2h").arg(d,2).arg(h, 2, 10, fill);
	else if (h)
		return QString("O.T. +%1h %2m").arg(h,2).arg(m, 2, 10, fill);
	else
		return QString("O.T. +%1m %2s").arg(m,2).arg(s, 2, 10, fill);
}



void setMaxWidth(QWidget *w, int numCharacters) {
	QFont f = w->font();
	QFontMetrics fm(f);
	w->setMaximumWidth(fm.width("W")*numCharacters);
}


void fixWidth(QWidget *w, int numCharacters) {
	QFont f = w->font();
	QFontMetrics fm(f);
	w->setFixedWidth(fm.width("W")*numCharacters);
}


ElideFadeDrawer::ElideFadeDrawer(QObject *parent) : QObject(parent) {}

bool ElideFadeDrawer::eventFilter(QObject *obj, QEvent *event) {
	if ( event->type() == QEvent::Paint ) {
		QLabel *q = static_cast<QLabel*>(obj);
		QPainter painter(q);
		QFontMetrics fm(q->font());
		QRect rect = q->contentsRect();
		int flags = q->alignment() | Qt::TextSingleLine;

		if ( fm.width(q->text()) > rect.width() ) {
			//QPixmap pixmap(rect.size());//, QImage::Format_ARGB32);
			QImage pixmap(rect.size(), QImage::Format_ARGB32);
			pixmap.fill(Qt::transparent);

			QPainter p(&pixmap);
			p.setPen(painter.pen());
			p.setFont(painter.font());

			/*
			QLinearGradient gradient(rect.topLeft(), rect.topRight());
			QColor from = q->palette().color(QPalette::WindowText);
			QColor to = from;
			to.setAlpha(0);
			gradient.setColorAt(0.8, from);
			gradient.setColorAt(1.0, to);
			p.setPen(QPen(gradient, 0));
			*/
			p.drawText(pixmap.rect(), flags, q->text());

			QLinearGradient alphaGradient(rect.topLeft(), rect.topRight());
			alphaGradient.setColorAt(0.8, QColor(0,0,0,255));
			alphaGradient.setColorAt(1.0, QColor(0,0,0,0));
			p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
			p.fillRect(pixmap.rect(), alphaGradient);

			painter.drawImage(rect.topLeft(), pixmap);
		}
		else {
			painter.setPen(q->palette().color(QPalette::WindowText));
			painter.drawText(rect, flags, q->text());
		}
		
		return true;
	}

	// standard event processing
	return QObject::eventFilter(obj, event);
}


EllipsisDrawer::EllipsisDrawer(QObject *parent) : QObject(parent) {}

bool EllipsisDrawer::eventFilter(QObject *obj, QEvent *event) {
	if ( event->type() == QEvent::Paint ) {
		QLabel *q = static_cast<QLabel*>(obj);
		QPainter painter(q);
		QFontMetrics fm(q->font());
		QRect rect = q->contentsRect();

		if ( fm.width(q->text()) > rect.width() ) {
			int eWidth = fm.width("...");
			painter.drawText(rect.adjusted(0,0,-eWidth,0), Qt::TextSingleLine, q->text());
			painter.drawText(rect.adjusted(rect.width()-eWidth,0,0,0), Qt::TextSingleLine, "...");
		}
		else
			painter.drawText(rect, Qt::TextSingleLine, q->text());

		return true;
	}

	// standard event processing
	return QObject::eventFilter(obj, event);
}


}
}
