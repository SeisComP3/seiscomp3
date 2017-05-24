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

#include "mvstationsymbol.h"

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
	_legendPos.setY(20);

	_mode = ApplicationStatus::GROUND_MOTION;

	_stationSize = 12;

	_offset = 30;
	_headingOffset = 10;

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
void Legend::Content::draw(const Seiscomp::Gui::Map::Canvas* canvas, const Legend& legend,
                           const QString& header,
                           QPainter& painter, const QPoint& pos) {
	QPoint currentPos(pos);

	int width = 0;
	for ( int i = 0; i < count(); ++i )
		width = std::max(width, painter.fontMetrics().width((*this)[i].first.first));

	int stationHeight = int(1.5*legend._stationSize);
	int rectHeight = 2*6 + stationHeight + (count()-1) * legend._offset;

	QPainter::RenderHints h = painter.renderHints();
	painter.setRenderHint(QPainter::Antialiasing, false);

	if ( !header.isEmpty() ) {
		QFont f = painter.font();
		QFont newFont(f);
		newFont.setBold(true);
		painter.setFont(newFont);
		QRect bbHeader = painter.fontMetrics().boundingRect(header);

		int headerHeight = bbHeader.height() + 2*6;

		width = std::max(width, bbHeader.width());

		painter.setPen(SCScheme.colors.legend.border);
		painter.setBrush(SCScheme.colors.legend.background);
		painter.drawRect(currentPos.x(), pos.y() - legend._stationSize,
		                 width + int(0.876*2*legend._stationSize) + 3*6,
		                 rectHeight + headerHeight);


		painter.setPen(SCScheme.colors.legend.headerText);
		painter.drawText(QRect(currentPos.x(), currentPos.y() - legend._stationSize,
		                       width + int(0.876*2*legend._stationSize) + 3*6,
		                       headerHeight),
		                 Qt::AlignHCenter | Qt::AlignVCenter, header);
		currentPos.setY(currentPos.y() + headerHeight);

		painter.setFont(f);
	}
	else {
		painter.setPen(SCScheme.colors.legend.border);
		painter.setBrush(SCScheme.colors.legend.background);
		painter.drawRect(currentPos.x(), pos.y() - legend._stationSize,
		                 width + int(0.876*2*legend._stationSize) + 3*6,
		                 rectHeight);
	}
	// Draw stations
	currentPos.setX(currentPos.x() + int(0.876*legend._stationSize) + 6);
	currentPos.setY(currentPos.y() + 6);

	painter.setRenderHints(h);

	for ( int i = 0; i < count(); ++i ) {
		legend.drawStation(canvas, painter, currentPos, (*this)[i].second,
		                   (*this)[i].first.second, (*this)[i].first.first);
		currentPos.setY(currentPos.y() + legend._offset);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
