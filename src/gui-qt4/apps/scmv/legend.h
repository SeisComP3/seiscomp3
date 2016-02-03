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


#ifndef __LEGEND_H__
#define __LEGEND_H__

#ifndef Q_MOC_RUN
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/rtti.h>
#endif
#include <seiscomp3/gui/map/mapsymbol.h>
#include <seiscomp3/gui/map/mapwidget.h>

#ifndef Q_MOC_RUN
#include "types.h"
#endif


class Legend : public Seiscomp::Gui::Map::Symbol {
	DECLARE_RTTI;

	private:
		typedef QPair<QString, QChar> Annotation;
		typedef QPair<Annotation, QColor> ContentItem;

	private:
		class Content : public QVector<ContentItem> {
			public:
				void addItem(const QString& text, QColor c) {
					append(ContentItem(Annotation(text, ' '), c));
				}
				void addItem(const QString& text, const QChar& character, QColor c) {
					append(ContentItem(Annotation(text, character), c));
				}

				void draw(const Seiscomp::Gui::Map::Canvas* canvas, const Legend& legend,
				          const QString& header, QPainter& painter, const QPoint& pos);
		};

	public:
		Legend();

	public:
		ApplicationStatus::Mode mode() const;
		void setMode(ApplicationStatus::Mode mode);

	private:
		virtual void customDraw(const Seiscomp::Gui::Map::Canvas *canvas, QPainter& painter);

	private:
		void init();
		void drawStation(const Seiscomp::Gui::Map::Canvas *canvas, QPainter& painter,
		                 const QPoint& pos, const QColor& color,
		                 const QChar& character, const QString& annotation) const;

		void handleQcStatus(const Seiscomp::Gui::Map::Canvas *canvas, QPainter& painter);

	private:
		ApplicationStatus::Mode   _mode;
		int    _stationSize;
		int    _offset;
		int    _headingOffset;
		QPoint _legendPos;

		Content _qcDelay;
		Content _qcLatency;
		Content _qcTimingInterval;
		Content _qcGapsInterval;
		Content _qcGapsLength;
		Content _qcSpikesInterval;
		Content _qcSpikesAmplitude;
		Content _qcOffset;
		Content _qcRMS;
		Content _event;
		Content _gm;
};

#endif
