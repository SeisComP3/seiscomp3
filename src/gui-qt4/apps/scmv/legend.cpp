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
Legend::Legend(QObject *parent) : Gui::Map::Legend(parent) {
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
	_currentContent = NULL;

	switch ( mode ) {
		case ApplicationStatus::GROUND_MOTION:
			_currentContent = &_gm;
			break;

		case ApplicationStatus::QC:
		{
			QCParameter::Parameter parameter = QCParameter::Instance()->parameter();
			switch ( parameter ) {
				case QCParameter::DELAY:
					_currentContent = &_qcDelay;
					break;

				case QCParameter::LATENCY:
					_currentContent = &_qcLatency;
					break;

				case QCParameter::TIMING_QUALITY:
					_currentContent = &_qcTimingInterval;
					break;

				case QCParameter::GAPS_INTERVAL:
					_currentContent = &_qcGapsInterval;
					break;

				case QCParameter::GAPS_LENGTH:
					_currentContent = &_qcGapsLength;
					break;

				case QCParameter::OVERLAPS_INTERVAL:
					_currentContent = &_qcSpikesInterval;
					break;

				case QCParameter::AVAILABILITY:
					_currentContent = &_qcSpikesAmplitude;
					break;

				case QCParameter::OFFSET:
					_currentContent = &_qcOffset;
					break;

				case QCParameter::RMS:
					_currentContent = &_qcRMS;
					break;

				default:
					break;
			}
		}

		default:
			break;
	}

	if ( _currentContent )
		_size = _currentContent->getSize(*this);
	else
		_size = QSize();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::setParameter(int p) {
	if ( _mode != ApplicationStatus::QC )
		return;

	setMode(_mode);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::draw(const QRect &rect, QPainter &painter) {
	if ( _currentContent )
		_currentContent->draw(*this, painter, rect);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::init() {
	setArea(Qt::Alignment(Qt::AlignTop | Qt::AlignLeft));

	_stationSize = 12;
	_offset = 30;

	_event.title = "";
	_event.addItem("Station is disabled", StationGlyphs::DISABLED, SCScheme.colors.stations.disabled);
	_event.addItem("Is in idle mode", SCScheme.colors.stations.idle);
	_event.addItem("Has been associated", SCScheme.colors.stations.associated);
	_event.addItem("Received a new station amplitude (blinking)", SCScheme.colors.stations.triggering);
	_event.addItem("Received an amplitude 10 min ago", SCScheme.colors.stations.triggered0);
	_event.addItem("Received an amplitude 20 min ago", SCScheme.colors.stations.triggered1);
	_event.addItem("Received an amplitude 30 min ago", SCScheme.colors.stations.triggered2);

	_gm.title = "Velocity in nm/s";
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

	_qcDelay.title = "Delay in s";
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

	_qcLatency.title = "Latency in sec.";
	_qcLatency.addItem("Warning > 60", SCScheme.colors.qc.qcWarning);
	_qcLatency.addItem("OK <= 60", SCScheme.colors.qc.qcOk);
	_qcLatency.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcLatency.addItem("General QC warning", StationGlyphs::WARNING,SCScheme.colors.qc.qcNotSet);
	_qcLatency.addItem("General QC error", StationGlyphs::ERROR,SCScheme.colors.qc.qcNotSet);

	_qcTimingInterval.title = "Timing Quality";
	_qcTimingInterval.addItem("Warning < 50", SCScheme.colors.qc.qcWarning);
	_qcTimingInterval.addItem("OK >= 50", SCScheme.colors.qc.qcOk);
	_qcTimingInterval.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcTimingInterval.addItem("General QC warning", StationGlyphs::WARNING, SCScheme.colors.qc.qcNotSet);
	_qcTimingInterval.addItem("General QC error", StationGlyphs::ERROR, SCScheme.colors.qc.qcNotSet);

	_qcGapsInterval.title = "Gaps Interval";
	_qcGapsInterval.addItem("Error", SCScheme.colors.qc.qcError);
	_qcGapsInterval.addItem("OK", SCScheme.colors.qc.qcOk);
	_qcGapsInterval.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcGapsInterval.addItem("General QC warning", StationGlyphs::WARNING, SCScheme.colors.qc.qcNotSet);
	_qcGapsInterval.addItem("General QC error", StationGlyphs::ERROR, SCScheme.colors.qc.qcNotSet);

	_qcGapsLength.title = "Gaps Length";
	_qcGapsLength.addItem("Error > 0", SCScheme.colors.qc.qcError);
	_qcGapsLength.addItem("OK <= 0", SCScheme.colors.qc.qcOk);
	_qcGapsLength.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcGapsLength.addItem("General QC warning", StationGlyphs::WARNING, SCScheme.colors.qc.qcNotSet);
	_qcGapsLength.addItem("General QC error", StationGlyphs::ERROR, SCScheme.colors.qc.qcNotSet);

	_qcSpikesInterval.title = "Overlaps Interval";
	_qcSpikesInterval.addItem("Error", SCScheme.colors.qc.qcError);
	_qcSpikesInterval.addItem("OK", SCScheme.colors.qc.qcOk);
	_qcSpikesInterval.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcSpikesInterval.addItem("General QC warning", StationGlyphs::WARNING, SCScheme.colors.qc.qcNotSet);
	_qcSpikesInterval.addItem("General QC error", StationGlyphs::ERROR, SCScheme.colors.qc.qcNotSet);

	_qcSpikesAmplitude.title = "Availability";
	_qcSpikesAmplitude.addItem("Error", SCScheme.colors.qc.qcError);
	_qcSpikesAmplitude.addItem("OK", SCScheme.colors.qc.qcOk);
	_qcSpikesAmplitude.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcSpikesAmplitude.addItem("General QC warning", StationGlyphs::WARNING, SCScheme.colors.qc.qcNotSet);
	_qcSpikesAmplitude.addItem("General QC error", StationGlyphs::ERROR, SCScheme.colors.qc.qcNotSet);

	_qcOffset.title = "Offset";
	_qcOffset.addItem("Warning > 100000", SCScheme.colors.qc.qcWarning);
	_qcOffset.addItem("OK < 100000", SCScheme.colors.qc.qcOk);
	_qcOffset.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcOffset.addItem("General QC warning", StationGlyphs::WARNING, SCScheme.colors.qc.qcNotSet);
	_qcOffset.addItem("General QC error", StationGlyphs::ERROR, SCScheme.colors.qc.qcNotSet);

	_qcRMS.title = "RMS";
	_qcRMS.addItem("Error < 10", SCScheme.colors.qc.qcError);
	_qcRMS.addItem("OK >= 10", SCScheme.colors.qc.qcOk);
	_qcRMS.addItem("Not Set", SCScheme.colors.qc.qcNotSet);
	_qcRMS.addItem("General QC warning", StationGlyphs::WARNING, SCScheme.colors.qc.qcNotSet);
	_qcRMS.addItem("General QC error", StationGlyphs::ERROR, SCScheme.colors.qc.qcNotSet);

	setMode(ApplicationStatus::GROUND_MOTION);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::drawStation(QPainter& painter, const QPoint& pos, const QColor& color,
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
	stationSymbol.draw(NULL, painter);

	painter.setPen(SCScheme.colors.legend.text);
	painter.drawText(pos.x() + _stationSize + 6, pos.y(), annotation);

	painter.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::handleQcStatus(QPainter &painter, const QRect &rect) {
	QCParameter::Parameter parameter = QCParameter::Instance()->parameter();
	switch ( parameter ) {
		case QCParameter::DELAY:
			_qcDelay.draw(*this, painter, rect);
			break;

		case QCParameter::LATENCY:
			_qcLatency.draw(*this, painter, rect);
			break;

		case QCParameter::TIMING_QUALITY:
			_qcTimingInterval.draw(*this, painter, rect);
			break;

		case QCParameter::GAPS_INTERVAL:
			_qcGapsInterval.draw(*this, painter, rect);
			break;

		case QCParameter::GAPS_LENGTH:
			_qcGapsLength.draw(*this, painter, rect);
			break;

		case QCParameter::OVERLAPS_INTERVAL:
			_qcSpikesInterval.draw(*this, painter, rect);
			break;

		case QCParameter::AVAILABILITY:
			_qcSpikesAmplitude.draw(*this, painter, rect);
			break;

		case QCParameter::OFFSET:
			_qcOffset.draw(*this, painter, rect);
			break;

		case QCParameter::RMS:
			_qcRMS.draw(*this, painter, rect);
			break;

		default:
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QSize Legend::Content::getSize(const Legend &legend) const {
	QFontMetrics fm(legend.font());
	int textHeight = fm.height();

	int width = 0;
	for ( int i = 0; i < count(); ++i )
		width = qMax(width, fm.width((*this)[i].first.first));

	int stationHeight = int(1.5*legend._stationSize);
	int height = 2*VMARGIN + stationHeight + (count()-1) * legend._offset;

	if ( !title.isEmpty() ) {
		QFont bold(legend.font());
		bold.setBold(true);
		QFontMetrics bfm(legend.font());
		QRect bbHeader = bfm.boundingRect(title);
		int headerHeight = bbHeader.height() + VSPACING;
		width = qMax(width, bbHeader.width());
		height += headerHeight;
	}

	width += int(0.876*2*legend._stationSize) + HMARGIN*2 + 6;

	return QSize(width, height);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Legend::Content::draw(const Legend &legend, QPainter &p, const QRect &rect) {
	QPoint currentPos(rect.topLeft());
	int textHeight = p.fontMetrics().height();

	int width = 0;
	for ( int i = 0; i < count(); ++i )
		width = qMax(width, p.fontMetrics().width((*this)[i].first.first));

	QPainter::RenderHints h = p.renderHints();
	p.setRenderHint(QPainter::Antialiasing, false);

	if ( !title.isEmpty() ) {
		QFont f = p.font();
		QFont bold(f);
		bold.setBold(true);
		p.setFont(bold);
		QRect bbHeader = p.fontMetrics().boundingRect(title);

		int headerHeight = bbHeader.height() + VSPACING;

		width = qMax(width, bbHeader.width());

		currentPos.setY(currentPos.y() + VMARGIN);

		p.setPen(SCScheme.colors.legend.headerText);
		p.drawText(QRect(currentPos.x(), currentPos.y(),
		                       width + int(0.876*2*legend._stationSize) + 3*6,
		                       headerHeight),
		                 Qt::AlignHCenter | Qt::AlignTop, title);
		currentPos.setY(currentPos.y() + headerHeight);

		p.setFont(f);
	}
	else
		currentPos.setY(currentPos.y() + VMARGIN);

	// Draw stations
	currentPos.setX(currentPos.x() + int(0.876*legend._stationSize) + HMARGIN);
	currentPos.setY(currentPos.y() + VSPACING);

	p.setRenderHints(h);

	for ( int i = 0; i < count(); ++i ) {
		legend.drawStation(p, currentPos, (*this)[i].second,
		                   (*this)[i].first.second, (*this)[i].first.first);
		currentPos.setY(currentPos.y() + legend._offset);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
