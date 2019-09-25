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

#define SEISCOMP_COMPONENT Gui::Utils

#include <cstdio>
#include <cmath>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/utils.h>
#include <seiscomp3/logging/log.h>

#include <boost/assign.hpp>

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

	QColor tmp(str.c_str());
	if ( tmp.isValid() ) {
		value = tmp;
		return true;
	}

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
		if ( col.size() != 3 && col.size() != 4
		 && col.size() != 6 && col.size() != 8 ) {
			colorConvertError = std::string("invalid color ") + col + ": expected 3, 4, 6 or 8 characters";
			return false;
		}

		int readItems;

		if ( col.size() > 4 )
			readItems = sscanf(col.c_str(), "%2X%2X%2X%2X",
					(unsigned int*)&red, (unsigned int*)&green,
					(unsigned int*)&blue, (unsigned int*)&alpha);
		else {
			readItems = sscanf(col.c_str(), "%1X%1X%1X%1X",
					(unsigned int*)&red, (unsigned int*)&green,
					(unsigned int*)&blue, (unsigned int*)&alpha);
			red = (red << 4) | red;
			green = (green << 4) | green;
			blue = (blue << 4) | blue;
			if ( readItems > 3 )
				alpha = (alpha << 4) | alpha;
		}

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

QColor readColor(const std::string &query, const std::string &str,
                 const QColor &base, bool *ok) {
	QColor r(base);

	if ( !fromString(r, str) ) {
		SEISCOMP_ERROR("%s: %s", query.c_str(), colorConvertError.c_str());
		if ( ok ) *ok = false;
	}
	else {
		if ( ok ) *ok = true;
	}

	return r;
}

Qt::PenStyle stringToPenStyle(const std::string &str) {
	static const std::map<std::string, Qt::PenStyle> styleNameMap =
		boost::assign::map_list_of<std::string, Qt::PenStyle>
		("customdashline", Qt::CustomDashLine)
		("dashdotdotline", Qt::DashDotDotLine)
		("dashdotline", Qt::DashDotLine)
		("dashline", Qt::DashLine)
		("dotline", Qt::DotLine)
		("nopen", Qt::NoPen)
		("solidline", Qt::SolidLine);
	std::string lower = str;
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
	return styleNameMap.at(lower);
}

Qt::PenStyle readPenStyle(const std::string &query, const std::string &str,
                          Qt::PenStyle base, bool *ok) {
	Qt::PenStyle r(base);

	try {
		r = stringToPenStyle(str);
		if ( ok ) *ok = true;
	}
	catch ( const std::out_of_range & ) {
		SEISCOMP_ERROR("%s: invalid pen style", query.c_str());
		if ( ok ) *ok = false;
	}

	return r;
}

Qt::BrushStyle stringToBrushStyle(const std::string &str) {
	static const std::map<std::string, Qt::BrushStyle> styleNameMap =
		boost::assign::map_list_of<std::string, Qt::BrushStyle>
		("solid", Qt::SolidPattern)
		("dense1", Qt::Dense1Pattern)
		("dense2", Qt::Dense2Pattern)
		("dense3", Qt::Dense3Pattern)
		("dense4", Qt::Dense4Pattern)
		("dense5", Qt::Dense5Pattern)
		("dense6", Qt::Dense6Pattern)
		("dense7", Qt::Dense7Pattern)
		("nobrush", Qt::NoBrush)
		("horizontal", Qt::HorPattern)
		("vertical", Qt::VerPattern)
		("cross", Qt::CrossPattern)
		("bdiag", Qt::BDiagPattern)
		("fdiag", Qt::FDiagPattern)
		("diagcross", Qt::DiagCrossPattern);
	std::string lower = str;
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
	return styleNameMap.at(lower);
}

Qt::BrushStyle readBrushStyle(const std::string &query, const std::string &str,
                              Qt::BrushStyle base, bool *ok) {
	Qt::BrushStyle r(base);

	try {
		r = stringToBrushStyle(str);
		if ( ok ) *ok = true;
	}
	catch ( const std::out_of_range & ) {
		SEISCOMP_ERROR("%s: invalid pen style", query.c_str());
		if ( ok ) *ok = false;
	}

	return r;
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


QString timeToString(const Core::Time &t, const char *fmt, bool addTimeZone) {
	QString s;

	if ( SCScheme.dateTime.useLocalTime ) {
		s = t.toLocalTime().toString(fmt).c_str();
		if ( addTimeZone ) {
			s += " ";
			s += Core::Time::LocalTimeZone().c_str();
		}
	}
	else {
		s = t.toString(fmt).c_str();
		if ( addTimeZone )
			s += " UTC";
	}

	return s;
}


void timeToLabel(QLabel *label, const Core::Time &t, const char *fmt, bool addTimeZone) {
	if ( SCScheme.dateTime.useLocalTime )
		label->setToolTip((t.toString(fmt) + " UTC").c_str());
	label->setText(timeToString(t, fmt, addTimeZone));
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
