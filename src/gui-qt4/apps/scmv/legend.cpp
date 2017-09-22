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


#include "legend.h"

#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/datamodel/originsymbol.h>

#include "mvstationsymbol.h"


#define HMARGIN (textHeight/2)
#define VMARGIN (textHeight/2)
#define SPACING (textHeight/2)
#define VSPACING (textHeight*3/4)


using namespace Seiscomp;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Legend::Legend() {
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ApplicationStatus::Mode Legend::mode() const {
	return _mode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::setMode(ApplicationStatus::Mode mode) {
	_mode = mode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::customDraw(const Seiscomp::Gui::Map::Canvas *canvas, QPainter& painter) {
	if ( !isVisible() ) return;

	switch ( mode() ) {
		case ApplicationStatus::EVENT:
			_event.draw(canvas, *this, "", painter, _legendPos);
			break;

		case ApplicationStatus::GROUND_MOTION:
			_gm.draw(canvas, *this, "Velocity in nm/s", painter, _legendPos);
			break;

		case ApplicationStatus::QC:
			handleQcStatus(canvas, painter);
			break;

		default:
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::init() {
	setPriority(Gui::Map::Symbol::HIGH);
	setVisible(true);

	_legendPos.setX(10);
	_legendPos.setY(10);

	_mode = ApplicationStatus::GROUND_MOTION;

	_stationSize = 12;

	_offset = 30;

	_event.addItem("Station is disabled", StationGlyphs::DISABLED, SCScheme.colors.stations.disabled);
	_event.addItem("Is in idle mode", SCScheme.colors.stations.idle);
	_event.addItem("Has been associated", SCScheme.colors.stations.associated);
	_event.addItem("Received a new station amplitude (blinking)", SCScheme.colors.stations.triggering);
	_event.addItem("Received an amplitude 10 min ago", SCScheme.colors.stations.triggered0);
	_event.addItem("Received an amplitude 20 min ago", SCScheme.colors.stations.triggered1);
	_event.addItem("Received an amplitude 30 min ago", SCScheme.colors.stations.triggered2);

	_gm.addItem("Station is disabled", StationGlyphs::DISABLED, SCScheme.colors.stations.disabled);
	_gm.addItem("Not Set", SCScheme.colors.gm.gmNotSet);
	_gm.addItem("[0,200]", SCScheme.colors.gm.gm1);
	_gm.addItem("400", SCScheme.colors.gm.gm2);
	_gm.addItem("800", SCScheme.colors.gm.gm3);
	_gm.addItem("1500", SCScheme.colors.gm.gm4);
	_gm.addItem("4000", SCScheme.colors.gm.gm5);
	_gm.addItem("12000", SCScheme.colors.gm.gm6);
	_gm.addItem("30000", SCScheme.colors.gm.gm7);
	_gm.addItem("60000", SCScheme.colors.gm.gm8);
	_gm.addItem("> 150000", SCScheme.colors.gm.gm9);

	_qcDelay.addItem("Station is disabled", StationGlyphs::DISABLED, SCScheme.colors.stations.disabled);
	_qcDelay.addItem("< 20", SCScheme.colors.qc.delay0);
	_qcDelay.addItem("(20, 60]", SCScheme.colors.qc.delay1);
	_qcDelay.addItem("(60, 180]", SCScheme.colors.qc.delay2);
	_qcDelay.addItem("(180, 600]", SCScheme.colors.qc.delay3);
	_qcDelay.addItem("(600, 1800]", SCScheme.colors.qc.delay4);
	_qcDelay.addItem("(1800, 0.5d]", SCScheme.colors.qc.delay5);
	_qcDelay.addItem("(0.5d, 1d]", SCScheme.colors.qc.delay6);
	_qcDelay.addItem("> 1d", SCScheme.colors.qc.delay7);
	_qcDelay.addItem("Not set", SCScheme.colors.qc.qcNotSet);
	_qcDelay.addItem("QC warning", StationGlyphs::WARNING, SCScheme.colors.qc.qcNotSet);
	_qcDelay.addItem("QC error", StationGlyphs::ERROR, SCScheme.colors.qc.qcNotSet);

	_qcLatency.addItem("Warning > 60", SCScheme.colors.qc.qcWarning);
	_qcLatency.addItem("OK <= 60", SCScheme.colors.qc.qcOk);
	_qcLatency.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcLatency.addItem("General QC warning", StationGlyphs::WARNING,SCScheme.colors.qc.qcNotSet);
	_qcLatency.addItem("General QC error", StationGlyphs::ERROR,SCScheme.colors.qc.qcNotSet);

	_qcTimingInterval.addItem("Warning < 50", SCScheme.colors.qc.qcWarning);
	_qcTimingInterval.addItem("OK >= 50", SCScheme.colors.qc.qcOk);
	_qcTimingInterval.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcTimingInterval.addItem("General QC warning", StationGlyphs::WARNING, SCScheme.colors.qc.qcNotSet);
	_qcTimingInterval.addItem("General QC error", StationGlyphs::ERROR, SCScheme.colors.qc.qcNotSet);

	_qcGapsInterval.addItem("Error", SCScheme.colors.qc.qcError);
	_qcGapsInterval.addItem("OK", SCScheme.colors.qc.qcOk);
	_qcGapsInterval.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcGapsInterval.addItem("General QC warning", StationGlyphs::WARNING, SCScheme.colors.qc.qcNotSet);
	_qcGapsInterval.addItem("General QC error", StationGlyphs::ERROR, SCScheme.colors.qc.qcNotSet);

	_qcGapsLength.addItem("Error > 0", SCScheme.colors.qc.qcError);
	_qcGapsLength.addItem("OK <= 0", SCScheme.colors.qc.qcOk);
	_qcGapsLength.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcGapsLength.addItem("General QC warning", StationGlyphs::WARNING, SCScheme.colors.qc.qcNotSet);
	_qcGapsLength.addItem("General QC error", StationGlyphs::ERROR, SCScheme.colors.qc.qcNotSet);

	_qcSpikesInterval.addItem("Error", SCScheme.colors.qc.qcError);
	_qcSpikesInterval.addItem("OK", SCScheme.colors.qc.qcOk);
	_qcSpikesInterval.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcSpikesInterval.addItem("General QC warning", StationGlyphs::WARNING, SCScheme.colors.qc.qcNotSet);
	_qcSpikesInterval.addItem("General QC error", StationGlyphs::ERROR, SCScheme.colors.qc.qcNotSet);

	_qcSpikesAmplitude.addItem("Error", SCScheme.colors.qc.qcError);
	_qcSpikesAmplitude.addItem("OK", SCScheme.colors.qc.qcOk);
	_qcSpikesAmplitude.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcSpikesAmplitude.addItem("General QC warning", StationGlyphs::WARNING, SCScheme.colors.qc.qcNotSet);
	_qcSpikesAmplitude.addItem("General QC error", StationGlyphs::ERROR, SCScheme.colors.qc.qcNotSet);

	_qcOffset.addItem("Warning > 100000", SCScheme.colors.qc.qcWarning);
	_qcOffset.addItem("OK < 100000", SCScheme.colors.qc.qcOk);
	_qcOffset.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcOffset.addItem("General QC warning", StationGlyphs::WARNING, SCScheme.colors.qc.qcNotSet);
	_qcOffset.addItem("General QC error", StationGlyphs::ERROR, SCScheme.colors.qc.qcNotSet);

	_qcRMS.addItem("Error < 10", SCScheme.colors.qc.qcError);
	_qcRMS.addItem("OK >= 10", SCScheme.colors.qc.qcOk);
	_qcRMS.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcRMS.addItem("General QC warning", StationGlyphs::WARNING, SCScheme.colors.qc.qcNotSet);
	_qcRMS.addItem("General QC error", StationGlyphs::ERROR, SCScheme.colors.qc.qcNotSet);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::drawStation(const Seiscomp::Gui::Map::Canvas* canvas, QPainter& painter,
                         const QPoint& pos, const QColor& color,
                         const QChar& character, const QString& annotation) const {
	painter.save();

	MvStationSymbol stationSymbol;
	stationSymbol.setCharacter(character);
	stationSymbol.setCharacterDrawingColor(SCScheme.colors.stations.text);
	stationSymbol.setCharacterDrawingEnabled(true);

	stationSymbol.setRadius(_stationSize);
	stationSymbol.setFrameSize(0);
	stationSymbol.setPos(pos);
	stationSymbol.setColor(color);
	stationSymbol.setFrameColor(Qt::black);
	stationSymbol.draw(canvas, painter);


	painter.setPen(SCScheme.colors.legend.text);
	painter.drawText(pos.x() + _stationSize + 6, pos.y(), annotation);

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::handleQcStatus(const Seiscomp::Gui::Map::Canvas* canvas, QPainter& painter) {
	QCParameter::Parameter parameter = QCParameter::Instance()->parameter();
	switch ( parameter ) {
		case QCParameter::DELAY:
			_qcDelay.draw(canvas, *this, "Delay in sec.", painter, _legendPos);
			break;

		case QCParameter::LATENCY:
			_qcLatency.draw(canvas, *this, "Latency in sec.", painter, _legendPos);
			break;

		case QCParameter::TIMING_QUALITY:
			_qcTimingInterval.draw(canvas, *this, "Timing Quality", painter, _legendPos);
			break;

		case QCParameter::GAPS_INTERVAL:
			_qcGapsInterval.draw(canvas, *this, "Gaps Interval", painter, _legendPos);
			break;

		case QCParameter::GAPS_LENGTH:
			_qcGapsLength.draw(canvas, *this, "Gaps Length", painter, _legendPos);
			break;

		case QCParameter::OVERLAPS_INTERVAL:
			_qcSpikesInterval.draw(canvas, *this, "Overlaps Interval", painter, _legendPos);
			break;

		case QCParameter::AVAILABILITY:
			_qcSpikesAmplitude.draw(canvas, *this, "Availability", painter, _legendPos);
			break;

		case QCParameter::OFFSET:
			_qcOffset.draw(canvas, *this, "Offset", painter, _legendPos);
			break;

		case QCParameter::RMS:
			_qcRMS.draw(canvas, *this, "RMS", painter, _legendPos);
			break;

		default:
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::Content::draw(const Seiscomp::Gui::Map::Canvas *canvas,
                           const Legend &legend, const QString &header,
                           QPainter &p, const QPoint &pos) {
	QPoint currentPos(pos);
	int textHeight = p.fontMetrics().height();

	int width = 0;
	for ( int i = 0; i < count(); ++i )
		width = qMax(width, p.fontMetrics().width((*this)[i].first.first));

	int stationHeight = int(1.5*legend._stationSize);
	int rectHeight = 2*VMARGIN + stationHeight + (count()-1) * legend._offset;

	QPainter::RenderHints h = p.renderHints();
	p.setRenderHint(QPainter::Antialiasing, false);

	if ( !header.isEmpty() ) {
		QFont f = p.font();
		QFont bold(f);
		bold.setBold(true);
		p.setFont(bold);
		QRect bbHeader = p.fontMetrics().boundingRect(header);

		int headerHeight = bbHeader.height() + VSPACING;

		width = qMax(width, bbHeader.width());

		p.setPen(SCScheme.colors.legend.border);
		p.setBrush(SCScheme.colors.legend.background);
		p.drawRect(currentPos.x(), currentPos.y(),
		                 width + int(0.876*2*legend._stationSize) + HMARGIN*2 + 6,
		                 rectHeight + headerHeight);

		currentPos.setY(currentPos.y() + VMARGIN);

		p.setPen(SCScheme.colors.legend.headerText);
		p.drawText(QRect(currentPos.x(), currentPos.y(),
		                       width + int(0.876*2*legend._stationSize) + 3*6,
		                       headerHeight),
		                 Qt::AlignHCenter | Qt::AlignTop, header);
		currentPos.setY(currentPos.y() + headerHeight);

		p.setFont(f);
	}
	else{
		p.setPen(SCScheme.colors.legend.border);
		p.setBrush(SCScheme.colors.legend.background);
		p.drawRect(currentPos.x(), currentPos.y(),
		                 width + int(0.876*2*legend._stationSize) + HMARGIN*2 + 6,
		                 rectHeight);
		currentPos.setY(currentPos.y() + VMARGIN);
	}

	// Draw stations
	currentPos.setX(currentPos.x() + int(0.876*legend._stationSize) + HMARGIN);
	currentPos.setY(currentPos.y() + VSPACING);

	p.setRenderHints(h);

	for ( int i = 0; i < count(); ++i ) {
		legend.drawStation(canvas, p, currentPos, (*this)[i].second,
		                   (*this)[i].first.second, (*this)[i].first.first);
		currentPos.setY(currentPos.y() + legend._offset);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EQSymbolLegend::EQSymbolLegend() {
	setRelativePosition(QPoint(10,10));

	Gui::Gradient::iterator it = SCScheme.colors.originSymbol.depth.gradient.begin();
	QColor currentColor = it.value().first;
	qreal lastValue = it.key();
	++it;

	for ( ; it != SCScheme.colors.originSymbol.depth.gradient.end(); ++it ) {
		lastValue = it.key();
		_depthItems.append(DepthItem(currentColor, StringWithWidth(QString("<= %1").arg(lastValue),-1)));
		currentColor = it.value().first;
	}

	_depthItems.append(DepthItem(currentColor, StringWithWidth(QString("> %1").arg(lastValue),-1)));

	for ( int i = 1; i <= 8; ++i )
		_magItems.append(MagItem(Gui::OriginSymbol::getSize(i), StringWithWidth(QString::number(i), -1)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EQSymbolLegend::setRelativePosition(const QPoint &pos) {
	_position = pos;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void EQSymbolLegend::customDraw(const Seiscomp::Gui::Map::Canvas *canvas,
                                QPainter& p) {
	if ( !isVisible() ) return;

	p.save();

	QFont f(p.font());
	QFont bold(f);
	bold.setBold(true);

	int textHeight = p.fontMetrics().height();

	int width = 0;
	int height = textHeight*3 + 3*VSPACING;

	QString depthHeader = SCApp->tr("Depth in km");
	QString magHeader = SCApp->tr("Magnitudes");

	width = qMax(width, p.fontMetrics().boundingRect(depthHeader).width());
	width = qMax(width, p.fontMetrics().boundingRect(magHeader).width());

	int cnt = _depthItems.count();
	int minDepthWidth = 0;
	for ( int i = 0; i < cnt; ++i ) {
		_depthItems[i].second.second = p.fontMetrics().boundingRect(_depthItems[i].second.first).width();
		minDepthWidth += _depthItems[i].second.second + SPACING/2 + textHeight;
	}

	minDepthWidth += SPACING * (cnt-1);

	width = qMax(width, minDepthWidth);

	cnt = _magItems.count();
	int minMagWidth = 0;
	int minMagHeight = 0;
	for ( int i = 0; i < cnt; ++i ) {
		_magItems[i].second.second = p.fontMetrics().boundingRect(_magItems[i].second.first).width();
		minMagWidth += _magItems[i].first + SPACING/2 + _magItems[i].second.second;
		minMagHeight = qMax(minMagHeight, _magItems[i].first);
	}

	minMagWidth += SPACING * (cnt-1);

	width = qMax(width, minMagWidth);
	height += minMagHeight;

	width += HMARGIN*2;
	height += VMARGIN*2;

	int x = pos().x();
	int y = p.window().height()-pos().y()-height;

	p.setRenderHint(QPainter::Antialiasing, false);

	p.setPen(SCScheme.colors.legend.border);
	p.setBrush(SCScheme.colors.legend.background);
	p.drawRect(x, y, width, height);

	y += VMARGIN;

	p.setFont(bold);
	p.setPen(SCScheme.colors.legend.headerText);
	p.drawText(QRect(x, y, width, textHeight),
	           Qt::AlignHCenter | Qt::AlignTop, depthHeader);

	y += textHeight+VSPACING;

	// Render depth items
	cnt = _depthItems.count();

	qreal additionalItemSpacing = 0;
	if ( cnt > 1 )
		additionalItemSpacing = qreal(width - HMARGIN*2 - minDepthWidth) / qreal(cnt-1);

	p.setPen(SCScheme.colors.legend.text);
	p.setFont(f);

	qreal fX = x + HMARGIN;
	for ( int i = 0; i < cnt; ++i ) {
		p.setBrush(_depthItems[i].first);
		p.drawRect((int)fX, y, textHeight, textHeight);
		fX += textHeight + SPACING/2;

		p.drawText(QRect((int)fX, y, _depthItems[i].second.second, textHeight),
		           Qt::AlignLeft | Qt::AlignTop, _depthItems[i].second.first);
		fX += _depthItems[i].second.second + SPACING + additionalItemSpacing;
	}

	y += textHeight + VSPACING;

	// Render magnitude items

	p.setFont(bold);
	p.setPen(SCScheme.colors.legend.headerText);
	p.drawText(QRect(x, y, width, textHeight),
	           Qt::AlignHCenter | Qt::AlignTop, magHeader);

	y += textHeight + VSPACING;

	cnt = _magItems.count();

	additionalItemSpacing = 0;
	if ( cnt > 1 )
		additionalItemSpacing = qreal(width - HMARGIN*2 - minMagWidth) / qreal(cnt-1);

	p.setPen(QPen(SCScheme.colors.legend.text,2));
	p.setBrush(Qt::gray);
	p.setFont(f);

	fX = x + HMARGIN;

	p.setRenderHint(QPainter::Antialiasing, true);

	for ( int i = 0; i < cnt; ++i ) {
		p.drawEllipse((int)fX, y + (minMagHeight-_magItems[i].first)/2, _magItems[i].first, _magItems[i].first);
		fX += _magItems[i].first + SPACING/2;

		p.drawText(QRect((int)fX, y, _magItems[i].second.second, minMagHeight),
		           Qt::AlignLeft | Qt::AlignVCenter, _magItems[i].second.first);
		fX += _magItems[i].second.second + SPACING + additionalItemSpacing;
	}

	p.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
