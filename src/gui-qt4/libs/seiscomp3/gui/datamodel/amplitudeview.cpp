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



#define SEISCOMP_COMPONENT Gui::AmplitudeView

#include "amplitudeview.h"
#include <seiscomp3/core/platform/platform.h>
#include <seiscomp3/core/genericrecord.h>
#include <seiscomp3/gui/datamodel/selectstation.h>
#include <seiscomp3/gui/datamodel/origindialog.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/recordstreamthread.h>
#include <seiscomp3/gui/core/timescale.h>
#include <seiscomp3/gui/core/uncertainties.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/client/configdb.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/parameter.h>
#include <seiscomp3/datamodel/stationmagnitudecontribution.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/math/filter.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/utils/misc.h>
#include <seiscomp3/math/mean.h>
#include <seiscomp3/logging/log.h>

#include <QMessageBox>
#include <numeric>
#include <fstream>
#include <limits>
#include <set>

#include <boost/bind.hpp>

#ifdef MACOSX
#include <seiscomp3/gui/core/osx.h>
#endif

#define NO_FILTER_STRING       "Raw"
#define DEFAULT_FILTER_STRING  "Default"

#define ITEM_DISTANCE_INDEX  0
#define ITEM_AZIMUTH_INDEX  1
#define ITEM_PRIORITY_INDEX  2
//#define ITEM_ARRIVALID_INDEX 2


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Math;
using namespace Seiscomp::Util;
using namespace Seiscomp::Gui;
using namespace Seiscomp::Gui::PrivateAmplitudeView;


namespace {

char COMPS[3] = {'Z', '1', '2'};

struct StationItem {
	AmplitudePtr amp;
	PickPtr      pick;
	bool         isTrigger;
};


class TraceList : public RecordView {
	public:
		TraceList(QWidget *parent = 0, Qt::WFlags f = 0)
		 : RecordView(parent, f) {}

		TraceList(const Seiscomp::Core::TimeWindow& tw,
		          QWidget *parent = 0, Qt::WFlags f = 0)
		 : RecordView(tw, parent, f) {}

		TraceList(const Seiscomp::Core::TimeSpan& ts,
		          QWidget *parent = 0, Qt::WFlags f = 0)
		 : RecordView(ts, parent, f) {}

	protected:
		RecordLabel* createLabel(RecordViewItem *item) const {
			return new AmplitudeRecordLabel;
		}

		void dropEvent(QDropEvent *event) {
			if ( event->mimeData()->hasFormat("text/plain") ) {
				Math::Filtering::InPlaceFilter<float> *f =
					Math::Filtering::InPlaceFilter<float>::Create(event->mimeData()->text().toStdString());

				if ( !f ) {
					QMessageBox::critical(this, "Create filter",
					QString("Invalid filter: %1").arg(event->mimeData()->text()));
					return;
				}

				delete f;
				emit filterChanged(event->mimeData()->text());
			}
		}
};


class TraceDecorator : public RecordWidgetDecorator {
	public:
		TraceDecorator(QObject *parent, AmplitudeRecordLabel *itemLabel)
		: RecordWidgetDecorator(parent), _itemLabel(itemLabel) {}

		AmplitudeRecordLabel *label() { return _itemLabel; }

		void drawDecoration(QPainter *painter, RecordWidget *widget) {
			if ( _itemLabel->processor ) {
				int nbegin = widget->mapTime(_itemLabel->processor->trigger()+Core::TimeSpan(_itemLabel->processor->config().noiseBegin));
				int nend = widget->mapTime(_itemLabel->processor->trigger()+Core::TimeSpan(_itemLabel->processor->config().noiseEnd));
				int sbegin = widget->mapTime(_itemLabel->processor->trigger()+Core::TimeSpan(_itemLabel->processor->config().signalBegin));
				int send = widget->mapTime(_itemLabel->processor->trigger()+Core::TimeSpan(_itemLabel->processor->config().signalEnd));

				// Draw unused area before noise
				painter->fillRect(0,0,nbegin,widget->height(), QColor(0,0,0,92));
				// Draw noise area
				painter->fillRect(nbegin,0,nend-nbegin,widget->height(), QColor(0,0,0,64));
				// Draw unused area between noise and signal
				painter->fillRect(nend,0,sbegin-nend,widget->height(), QColor(0,0,0,92));
				// Draw signal area
				painter->fillRect(send,0,widget->width()-send,widget->height(), QColor(0,0,0,92));

				if ( !_itemLabel->infoText.isEmpty() ) {
					QRect boundingRect =
						widget->fontMetrics().boundingRect(painter->window(),
						                                   Qt::AlignRight|Qt::AlignTop,
						                                   _itemLabel->infoText);
					boundingRect.adjust(-9,0,-1,8);

					if ( _itemLabel->isError ) {
						painter->setPen(Qt::white);
						painter->setBrush(QColor(128,0,0,192));
					}
					else {
						painter->setPen(qApp->palette().color(QPalette::WindowText));
						painter->setBrush(QColor(255,255,255,192));
					}

					painter->drawRect(boundingRect);

					if ( _itemLabel->processor->status() > Processing::WaveformProcessor::Finished )
						painter->setPen(Qt::white);

					painter->drawText(boundingRect, Qt::AlignHCenter|Qt::AlignVCenter, _itemLabel->infoText);
				}
			}
		}

	private:
		AmplitudeRecordLabel *_itemLabel;
};


class MyRecordWidget : public RecordWidget {
	public:
		MyRecordWidget() {}

		void setSelected(const Core::Time &t1, const Core::Time &t2) {
			_t1 = t1;
			_t2 = t2;
			update();
		}

	protected:
		void drawCustomBackground(QPainter &painter) {
			if ( !_t1 || !_t2 ) return;

			int x1 = mapTime(_t1);
			int x2 = mapTime(_t2);

			//int y1 = streamYPos(currentRecords());
			//int h = streamHeight(currentRecords());
			int y1 = 0;
			int h = height();

			QColor col = palette().color(QPalette::Highlight);
			col.setAlpha(64);
			painter.fillRect(x1,y1,x2-x1+1,h, col);
		}

	private:
		Core::Time _t1, _t2;
};


class AmplitudeViewMarker : public RecordMarker {
	public:
		enum Type {
			UndefinedType, /* Something undefined */
			Reference,     /* The amplitude reference marker (first P arrival) */
			Amplitude      /* An amplitude */
		};

	public:
		AmplitudeViewMarker(RecordWidget *parent,
		                    const Seiscomp::Core::Time& pos,
		                    Type type, bool newAmplitude)
		: RecordMarker(parent, pos),
		  _type(type),
		  _slot(-1) {
			setMovable(newAmplitude);
			init();
		}

		AmplitudeViewMarker(RecordWidget *parent,
		                    const Seiscomp::Core::Time& pos,
		                    const QString& text,
		                    Type type, bool newAmplitude)
		: RecordMarker(parent, pos, text),
		  _type(type),
		  _slot(-1) {
			setMovable(newAmplitude);
			init();
		}

		AmplitudeViewMarker(RecordWidget *parent,
		                    const AmplitudeViewMarker& m)
		: RecordMarker(parent, m),
		  _referencedAmplitude(m._referencedAmplitude),
		  _type(m._type),
		  _slot(m._slot),
		  _newResult(m._newResult)
		{
			init();
			_time = m._time;
		}

		virtual ~AmplitudeViewMarker() {}


	private:
		void init() {
			_twBegin = _twEnd = 0;
			_newResult = false;
			setMoveCopy(false);
			updateVisual();
		}


	public:
		void setEnabled(bool enable) {
			RecordMarker::setEnabled(enable);
			updateVisual();
		}

		void setSlot(int s) {
			_slot = s;
		}

		char slot() const {
			return _slot;
		}

		void setType(Type t) {
			_type = t;
			updateVisual();
		}

		Type type() const {
			return _type;
		}

		void setTimeWindow(float begin, float end) {
			_twBegin = begin;
			_twEnd = end;
		}

		float timeWindowBegin() const {
			return _twBegin;
		}

		float timeWindowEnd() const {
			return _twEnd;
		}

		void setMagnitude(OPT(double) mag, const QString &error) {
			_magnitude = mag;
			_magnitudeError = error;

			if ( _magnitude )
				setDescription(QString("%1: %2").arg(text()).arg(*_magnitude, 0, 'f', 2));
			else
				setDescription("");
		}

		void setAmplitude(DataModel::Amplitude *a) {
			_referencedAmplitude = a;
			_time = a->timeWindow().reference();
			try {
				setTimeWindow(a->timeWindow().begin(), a->timeWindow().end());
			}
			catch ( ... ) {
				_twBegin = _twEnd = 0;
			}

			if ( _referencedAmplitude )
				_newResult = false;

			updateVisual();
		}

		void setAmplitudeResult(const Processing::AmplitudeProcessor::Result &res) {
			_newResult = true;
			_newAmplitude = res;
			_newAmplitude.record = NULL;
		}

		void setFilterID(const std::string &filterID) {
			_filterID = filterID;
		}

		const std::string &filterID() const {
			return _filterID;
		}

		const Processing::AmplitudeProcessor::Result &amplitudeResult() const {
			return _newAmplitude;
		}

		void setPick(DataModel::Pick *p) {
			_pick = p;
		}

		void convertToManualAmplitude() {
			if ( !_referencedAmplitude ) return;
			_referencedAmplitude = NULL;
			setMovable(true);
			setDescription("");
			updateVisual();
		}

		DataModel::Amplitude *amplitude() const {
			return _referencedAmplitude.get();
		}

		bool equalsAmplitude(DataModel::Amplitude *amp) const {
			if ( amp == NULL ) return false;

			// Time + uncertainties do not match: not equal
			if ( correctedTime() != amp->timeWindow().reference() ) return false;
			try {
				if ( _newAmplitude.amplitude.value != amp->amplitude().value() ) return false;
			}
			catch ( ... ) {
				return false;
			}

			return true;
		}

		bool isAmplitude() const {
			return _type == Amplitude;
		}

		bool isNewAmplitude() const {
			return _type == Amplitude && _newResult;
		}

		bool isReference() const {
			return _type == Reference;
		}

		RecordMarker *copy() { return new AmplitudeViewMarker(NULL, *this); }

		void draw(QPainter &painter, RecordWidget *context, int x, int y1, int y2,
		          QColor color, qreal lineWidth) {
			// Adjust vertical position to current slot
			if ( _slot >= 0 ) {
				y1 = context->streamYPos(_slot);
				y2 = y1 + context->streamHeight(_slot);
			}
			else {
				y1 = 0;
				y2 = context->height();
			}

			int twb = (int)(_twBegin * context->timeScale());
			int twe = (int)(_twEnd * context->timeScale());

			if ( twe-twb > 0 )
				painter.fillRect(x+twb,y2-10,twe-twb+1,10, QColor(color.red(), color.green(), color.blue(), 64));

			if ( _slot >= 0 ) {
				painter.setPen(QPen(color, lineWidth, Qt::DashLine));
				painter.drawLine(x, 0, x, y1);
				painter.drawLine(x, y2, x, context->height());
			}

			RecordMarker::draw(painter, context, x, y1, y2, color, lineWidth);

			if ( !_magnitudeError.isEmpty() ) {
				painter.save();

				static QPoint marker[3] = {QPoint(-1,2), QPoint(1,2), QPoint(0,0)};

				int fh = 24;
				x += fh/4;
				y1 += 2;

				painter.setRenderHint(QPainter::Antialiasing);
				painter.setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin));
				painter.setBrush(Qt::yellow);

				marker[0] = QPoint(x-fh/2, y1);
				marker[1] = QPoint(x+fh/2, y1);
				marker[2] = QPoint(x,y1+fh);

				painter.drawPolygon(marker, 3);

				y2 = y1+fh*3/4;
				painter.drawLine(x,y1+4,x,y2-8);
				painter.drawLine(x,y2-6,x,y2-5);

				painter.restore();
			}
		}

		QString toolTip() const {
			QString text;

			if ( (_referencedAmplitude == NULL) && !isAmplitude() )
				return text;

			if ( _magnitude )
				text += QString("magnitude: %1").arg(*_magnitude,0,'f',2);
			else
				text += "magnitude: -";

			if ( !_magnitudeError.isEmpty() )
				text += QString(" (%1)").arg(_magnitudeError);

			if ( !text.isEmpty() )
				text += "\n\n";

			if ( _referencedAmplitude ) {
				try {
					switch ( _referencedAmplitude->evaluationMode() ) {
						case MANUAL:
							text += "manual ";
							break;
						case AUTOMATIC:
							text += "automatic ";
							break;
						default:
							break;
					}
				}
				catch ( ... ) {}

				text += "amplitude";

				try {
					text += QString(" created by %1").arg(_referencedAmplitude->creationInfo().author().c_str());
				}
				catch ( ... ) {}

				try {
					text += QString(" at %1").arg(_referencedAmplitude->creationInfo().creationTime().toString("%F %T").c_str());
				}
				catch ( ... ) {}

				try {
					text += QString("\nvalue: %1").arg(_referencedAmplitude->amplitude().value());
				}
				catch ( ... ) {}

				try {
					text += QString("\nperiod: %1").arg(_referencedAmplitude->period());
				}
				catch ( ... ) {}

				try {
					text += QString("\nsnr: %1").arg(_referencedAmplitude->snr());
				}
				catch ( ... ) {}

				if ( !_referencedAmplitude->filterID().empty() )
					text += QString("\nfilter: %1").arg(_referencedAmplitude->filterID().c_str());

				if ( !_referencedAmplitude->methodID().empty() )
					text += QString("\nmethod: %1").arg(_referencedAmplitude->methodID().c_str());
			}
			else {
				text += "amplitude\n";
				text += QString("value: %1").arg(_newAmplitude.amplitude.value);

				if ( _newAmplitude.period > 0 )
					text += QString("\nperiod: %1").arg(_newAmplitude.period);

				if ( _newAmplitude.snr >= 0 )
					text += QString("\nsnr: %1").arg(_newAmplitude.snr);

				if ( !_filterID.empty() )
					text += QString("\nfilter: %1").arg(_filterID.c_str());
			}

			return text;
		}

	private:
		void updateVisual() {
			QColor col = SCScheme.colors.picks.disabled;
			Qt::Alignment al = Qt::AlignVCenter;
			EvaluationMode state = AUTOMATIC;

			DataModel::Amplitude *a = amplitude();
			if ( a ) {
				try { state = a->evaluationMode(); } catch ( ... ) {}
			}
			
			if ( isMovable() )
				state = MANUAL;

			switch ( _type ) {
				case Amplitude:
					if ( isEnabled() ) {
						switch ( state ) {
							case MANUAL:
								col = SCScheme.colors.arrivals.manual;
								break;
							case AUTOMATIC:
							default:
								col = SCScheme.colors.arrivals.automatic;
								break;
						}
					}
					else
						col = SCScheme.colors.arrivals.disabled;

					al = Qt::AlignVCenter;
					break;

				case Reference:
					col = SCScheme.colors.arrivals.theoretical;
					al = Qt::AlignBottom;
					break;

				default:
					break;
			}

			setColor(col);
			setAlignment(al);
		}


	private:
		Processing::AmplitudeProcessor::Result _newAmplitude;
		AmplitudePtr                           _referencedAmplitude;
		PickPtr                                _pick;
		TimeQuantity                           _time;
		Type                                   _type;
		int                                    _slot;
		float                                  _twBegin;
		float                                  _twEnd;
		bool                                   _newResult;
		OPT(double)                            _magnitude;
		QString                                _magnitudeError;
		std::string                            _filterID;
};


bool isTraceUsed(Seiscomp::Gui::RecordWidget *w) {
	for ( int i = 0; i < w->markerCount(); ++i ) {
		AmplitudeViewMarker *m = static_cast<AmplitudeViewMarker*>(w->marker(i));
		if ( !m->isEnabled() ) continue;
		if ( m->type() == AmplitudeViewMarker::Amplitude ) return true;
	}

	return false;
}


bool isTracePicked(Seiscomp::Gui::RecordWidget* w) {
	for ( int i = 0; i < w->markerCount(); ++i ) {
		AmplitudeViewMarker *m = static_cast<AmplitudeViewMarker*>(w->marker(i));
		if ( m->type() == AmplitudeViewMarker::Amplitude ) return true;
	}

	return false;
}


bool isArrivalTrace(Seiscomp::Gui::RecordWidget *w) {
	for ( int i = 0; i < w->markerCount(); ++i ) {
		AmplitudeViewMarker *m = static_cast<AmplitudeViewMarker*>(w->marker(i));
		if ( m->amplitude() && m->id() >= 0 ) return true;
	}

	return false;
}


SensorLocation *findSensorLocation(Station *station, const std::string &code,
                                   const Seiscomp::Core::Time &time) {
	for ( size_t i = 0; i < station->sensorLocationCount(); ++i ) {
		SensorLocation *loc = station->sensorLocation(i);

		try {
			if ( loc->end() <= time ) continue;
		}
		catch ( Seiscomp::Core::ValueException& ) {}

		if ( loc->start() > time ) continue;

		if ( loc->code() == code )
			return loc;
	}

	return NULL;
}


Stream* findStream(Station *station, const std::string &code,
                   const Seiscomp::Core::Time &time) {
	for ( size_t i = 0; i < station->sensorLocationCount(); ++i ) {
		SensorLocation *loc = station->sensorLocation(i);

		try {
			if ( loc->end() <= time ) continue;
		}
		catch ( Seiscomp::Core::ValueException& ) {}

		if ( loc->start() > time ) continue;

		for ( size_t j = 0; j < loc->streamCount(); ++j ) {
			Stream *stream = loc->stream(j);

			try {
				if ( stream->end() <= time ) continue;
			}
			catch ( Seiscomp::Core::ValueException& ) {}

			if ( stream->start() > time ) continue;

			if ( stream->code().substr(0, code.size()) != code ) continue;

			return stream;
		}
	}

	return NULL;
}


Stream* findStream(Station *station, const std::string &code, const std::string &locCode,
                   const Seiscomp::Core::Time &time) {
	for ( size_t i = 0; i < station->sensorLocationCount(); ++i ) {
		SensorLocation *loc = station->sensorLocation(i);

		try {
			if ( loc->end() <= time ) continue;
		}
		catch ( Seiscomp::Core::ValueException& ) {}

		if ( loc->start() > time ) continue;

		if ( loc->code() != locCode ) continue;

		for ( size_t j = 0; j < loc->streamCount(); ++j ) {
			Stream *stream = loc->stream(j);

			try {
				if ( stream->end() <= time ) continue;
			}
			catch ( Seiscomp::Core::ValueException& ) {}

			if ( stream->start() > time ) continue;

			if ( stream->code().substr(0, code.size()) != code ) continue;

			return stream;
		}
	}

	return NULL;
}


Stream* findStream(Station *station, const Seiscomp::Core::Time &time,
                   Processing::WaveformProcessor::SignalUnit requestedUnit) {
	for ( size_t i = 0; i < station->sensorLocationCount(); ++i ) {
		SensorLocation *loc = station->sensorLocation(i);

		try {
			if ( loc->end() <= time ) continue;
		}
		catch ( Seiscomp::Core::ValueException& ) {}

		if ( loc->start() > time ) continue;

		for ( size_t j = 0; j < loc->streamCount(); ++j ) {
			Stream *stream = loc->stream(j);

			try {
				if ( stream->end() <= time ) continue;
			}
			catch ( Seiscomp::Core::ValueException& ) {}

			if ( stream->start() > time ) continue;

			Sensor *sensor = Sensor::Find(stream->sensor());
			if ( !sensor ) continue;

			Processing::WaveformProcessor::SignalUnit unit;

			// Unable to retrieve the unit enumeration from string
			if ( !unit.fromString(sensor->unit().c_str()) ) continue;
			if ( unit != requestedUnit ) continue;

			return stream;
		}
	}

	return NULL;
}


Stream* findConfiguredStream(Station *station, const Seiscomp::Core::Time &time) {
	DataModel::Stream *stream = NULL;
	DataModel::ConfigModule *module = SCApp->configModule();
	if ( module ) {
		for ( size_t ci = 0; ci < module->configStationCount(); ++ci ) {
			DataModel::ConfigStation* cs = module->configStation(ci);
			if ( cs->networkCode() == station->network()->code() &&
			     cs->stationCode() == station->code() ) {

				for ( size_t si = 0; si < cs->setupCount(); ++si ) {
					DataModel::Setup* setup = cs->setup(si);

					DataModel::ParameterSet* ps = NULL;
					try {
						ps = DataModel::ParameterSet::Find(setup->parameterSetID());
					}
					catch ( Core::ValueException ) {
						continue;
					}

					if ( !ps ) {
						SEISCOMP_ERROR("Cannot find parameter set %s", setup->parameterSetID().c_str());
						continue;
					}

					std::string net, sta, loc, cha;
					net = cs->networkCode();
					sta = cs->stationCode();
					for ( size_t pi = 0; pi < ps->parameterCount(); ++pi ) {
						DataModel::Parameter* par = ps->parameter(pi);
						if ( par->name() == "detecLocid" )
							loc = par->value();
						else if ( par->name() == "detecStream" )
							cha = par->value();
					}

					// No channel defined
					if ( cha.empty() ) continue;
					stream = findStream(station, cha, loc, time);
					if ( stream ) return stream;
				}
			}
		}
	}

	return stream;
}


Util::KeyValuesPtr getParams(const string &net, const string &sta) {
	ConfigModule *module = SCApp->configModule();
	if ( module == NULL ) return NULL;

	for ( size_t ci = 0; ci < module->configStationCount(); ++ci ) {
		ConfigStation* cs = module->configStation(ci);
		if ( cs->networkCode() != net || cs->stationCode() != sta ) continue;
		Setup *setup = findSetup(cs, SCApp->name());
		if ( setup == NULL ) continue;
		if ( !setup->enabled() ) continue;

		DataModel::ParameterSet *ps = DataModel::ParameterSet::Find(setup->parameterSetID());
		if ( ps == NULL ) {
			SEISCOMP_WARNING("Cannot find parameter set %s for station %s.%s",
			                 setup->parameterSetID().data(),
			                 net.data(), sta.data());
			continue;
		}

		Util::KeyValuesPtr keys = new Util::KeyValues;
		keys->init(ps);
		return keys;
	}

	return NULL;
}


Amplitude* findAmplitude(Seiscomp::Gui::RecordWidget* w, const Seiscomp::Core::Time& t) {
	for ( int i = 0; i < w->markerCount(); ++i ) {
		AmplitudeViewMarker *m = static_cast<AmplitudeViewMarker*>(w->marker(i));
		Amplitude *a = m->amplitude();
		if ( a && a->timeWindow().reference() == t )
			return a;
	}

	return NULL;
}


std::string adjustChannelCode(const std::string& channelCode, bool allComponents) {
	if ( channelCode.size() < 3 )
		return channelCode + (allComponents?'?':'Z');
	else
		return allComponents?channelCode.substr(0,2) + '?':channelCode;
}


WaveformStreamID setWaveformIDComponent(const WaveformStreamID& id, char component) {
	return WaveformStreamID(id.networkCode(), id.stationCode(), id.locationCode(),
	                        id.channelCode().substr(0,2) + component, id.resourceURI());
}


WaveformStreamID adjustWaveformStreamID(const WaveformStreamID& id) {
	return WaveformStreamID(id.networkCode(), id.stationCode(), id.locationCode(),
	                        adjustChannelCode(id.channelCode(), true), id.resourceURI());
}

QString waveformIDToQString(const WaveformStreamID& id) {
	return (id.networkCode() + "." + id.stationCode() + "." +
	        id.locationCode() + "." + id.channelCode()).c_str();
}


std::string waveformIDToStdString(const WaveformStreamID& id) {
	return (id.networkCode() + "." + id.stationCode() + "." +
	        id.locationCode() + "." + id.channelCode());
}


char waveformIDComponent(const WaveformStreamID& id) {
	if ( id.channelCode().size() > 2 )
		return id.channelCode()[2];

	return '\0';
}


bool isLinkedItem(RecordViewItem *item) {
	return static_cast<AmplitudeRecordLabel*>(item->label())->isLinkedItem();
}

void unlinkItem(RecordViewItem *item) {
	static_cast<AmplitudeRecordLabel*>(item->label())->unlink();
	item->disconnect(SIGNAL(firstRecordAdded(const Seiscomp::Record*)));
}


void selectFirstVisibleItem(RecordView *view) {
	for ( int i = 0; i < view->rowCount(); ++i ) {
		view->setCurrentItem(view->itemAt(i));
		view->ensureVisible(i);
		break;
	}
}



/*
bool calcAmpTime(RecordSequence *seq, const Core::Time &begin, const Core::Time &end,
                 double offset, Processing::AmplitudeProcessor::AmplitudeMeasureType type,
                 double &ampValue, Core::Time &ampTime, double &ampWidth) {
	if ( seq == NULL ) return false;

	Core::Time minTime, maxTime;
	OPT(double) minValue;
	OPT(double) maxValue;

	for ( RecordSequence::iterator it = seq->begin(); it != seq->end(); ++it) {
		RecordCPtr rec = *it;
		Core::Time startTime = rec->startTime();
		Core::Time endTime = rec->endTime();

		if ( startTime >= end ) continue;
		if ( endTime <= begin ) continue;

		const FloatArray *data = static_cast<const FloatArray*>(rec->data());
		int from = double(begin - startTime) * rec->samplingFrequency();
		int to   = double(end - startTime) * rec->samplingFrequency();

		if ( from == to ) to += 2;

		if ( from < 0 ) from = 0;
		if ( to > data->size() ) to = data->size();

		int imin = -1;
		int imax = -1;

		switch ( type ) {
			case Processing::AmplitudeProcessor::AbsMax:
				for ( int i = from; i < to; ++i ) {
					double v = fabs((*data)[i]-offset);
					if ( !maxValue || (v > *maxValue) ) {
						maxValue = v;
						imax = i;
					}
				}
				break;
			case Processing::AmplitudeProcessor::MinMax:
				for ( int i = from; i < to; ++i ) {
					double v = (*data)[i]-offset;
					if ( !maxValue || (v > *maxValue)) {
						maxValue = v;
						imax = i;
					}

					if ( !minValue || (v < *minValue) ) {
						minValue = v;
						imin = i;
					}
				}
				break;
			default:
				return false;
		}

		if ( imin != -1 )
			minTime = startTime + Core::TimeSpan(double(imin) / rec->samplingFrequency());

		if ( imax != -1 )
			maxTime = startTime + Core::TimeSpan(double(imax) / rec->samplingFrequency());
	}

	switch ( type ) {
		case Processing::AmplitudeProcessor::AbsMax:
			if ( !maxValue ) return false;
			ampValue = *maxValue;
			ampWidth = -1;
			ampTime = maxTime;
			return true;
		case Processing::AmplitudeProcessor::MinMax:
		{
			if ( !minValue || !maxValue ) return false;

			ampValue = *maxValue - *minValue;

			double dMinTime = (double)minTime;
			double dMaxTime = (double)maxTime;
			double center = (dMinTime + dMaxTime) * 0.5;
			ampWidth = fabs(center-dMinTime);
			ampTime = Core::Time(center);
			return true;
		}
		default:
			break;
	};

	return false;
}


bool calcAmpTime(RecordSequence *seq, const Core::Time &time,
                 double timeWidth, double offset,
                 Processing::AmplitudeProcessor::AmplitudeMeasureType type,
                 double &ampValue, Core::Time &ampTime, double &ampWidth) {
	Core::Time begin = time - Core::TimeSpan(timeWidth);
	Core::Time end = time + Core::TimeSpan(timeWidth);

	return calcAmpTime(seq, begin, end, offset, type, ampValue, ampTime, ampWidth);
}
*/


}


namespace Seiscomp {
namespace Gui {
namespace PrivateAmplitudeView {


ThreeComponentTrace::ThreeComponentTrace() {
	widget = NULL;
	enableTransformation = false;
	showProcessed = false;

	for ( int i = 0; i < 3; ++i ) {
		traces[i].raw = NULL;
		traces[i].transformed = NULL;
		traces[i].processed = NULL;
		traces[i].thread = NULL;
		traces[i].filter = NULL;
	}
}


ThreeComponentTrace::~ThreeComponentTrace() {
	for ( int i = 0; i < 3; ++i ) {
		if ( traces[i].raw ) delete traces[i].raw;
		if ( widget ) widget->setRecords(traces[i].recordSlot, NULL);
		if ( traces[i].transformed ) delete traces[i].transformed;
		if ( traces[i].processed ) delete traces[i].processed;
		if ( traces[i].filter ) delete traces[i].filter;
	}
}


void ThreeComponentTrace::setFilter(RecordWidget::Filter *f, const std::string &filterID_) {
	if ( f )
		filterID = filterID_;
	else
		filterID.clear();

	for ( int i = 0; i < 3; ++i ) {
		if ( traces[i].filter ) delete traces[i].filter;
		traces[i].filter = f?f->clone():NULL;

		// Delete transformed traces
		if ( traces[i].transformed ) {
			delete traces[i].transformed;
			traces[i].transformed = NULL;

			if ( widget && !showProcessed ) widget->setRecords(traces[i].recordSlot, NULL);
		}

		removeProcessedData(i);
	}

	transform();
}


void ThreeComponentTrace::setRecordWidget(RecordWidget *w) {
	if ( widget ) {
		for ( int i = 0; i < 3; ++i )
			widget->setRecords(traces[i].recordSlot, NULL);
		widget->disconnect(this);
	}

	widget = w;

	if ( widget ) {
		for ( int i = 0; i < 3; ++i )
			widget->setRecords(traces[i].recordSlot, showProcessed?traces[i].processed:traces[i].transformed, false);
		connect(widget, SIGNAL(destroyed(QObject*)), this, SLOT(widgetDestroyed(QObject*)));
	}
}


void ThreeComponentTrace::widgetDestroyed(QObject *obj) {
	if ( obj == widget )
		widget = NULL;
}


void ThreeComponentTrace::setTransformationEnabled(bool f) {
	enableTransformation = f;

	for ( int i = 0; i < 3; ++i ) {
		// Delete transformed traces
		if ( traces[i].transformed ) {
			delete traces[i].transformed;
			traces[i].transformed = NULL;

			if ( widget && !showProcessed ) widget->setRecords(traces[i].recordSlot, NULL);
		}
	}

	transform();
}


void ThreeComponentTrace::showProcessedData(bool e) {
	showProcessed = e;
	if ( !widget ) return;
	for ( int i = 0; i < 3; ++i )
		widget->setRecords(traces[i].recordSlot, showProcessed?traces[i].processed:traces[i].transformed, false);
}


bool ThreeComponentTrace::setProcessedData(int comp,
                                           const std::string &networkCode,
                                           const std::string &stationCode,
                                           const std::string &locationCode,
                                           const Core::Time &startTime,
                                           double samplingFrequency,
                                           FloatArrayPtr data) {
	GenericRecordPtr prec = new GenericRecord(networkCode,
	                                          stationCode,
	                                          locationCode,
	                                          traces[comp].channelCode,
	                                          startTime, samplingFrequency);
	prec->setData(data.get());
	prec->dataUpdated();

	try {
		prec->endTime();
	}
	catch ( ... ) {
		return false;
	}

	if ( traces[comp].processed == NULL )
		traces[comp].processed = new RingBuffer(0);
	else
		traces[comp].processed->clear();

	traces[comp].processed->feed(prec.get());

	if ( widget && showProcessed )
		widget->setRecords(traces[comp].recordSlot, traces[comp].processed, false);

	return true;
}


void ThreeComponentTrace::removeProcessedData(int comp) {
	if ( traces[comp].processed ) {
		delete traces[comp].processed;
		traces[comp].processed = NULL;
	}

	if ( widget && showProcessed )
		widget->setRecords(traces[comp].recordSlot, traces[comp].processed, false);
}


bool ThreeComponentTrace::transform(int comp, Record *rec) {
	Core::Time minStartTime;
	Core::Time maxStartTime;
	Core::Time minEndTime;
	bool gotRecords = false;

	if ( enableTransformation ) {
		// Not all traces available, nothing to do
		for ( int i = 0; i < 3; ++i ) {
			// Delete current transformed records
			if ( traces[i].transformed ) {
				//delete traces[i].transformed;
				//traces[i].transformed = NULL;
				Core::Time endTime = traces[i].transformed->back()->endTime();
				if ( endTime > minStartTime )
					minStartTime = endTime;
			}

			if ( !traces[i].raw || traces[i].raw->empty() )
				return false;
		}

		// Find common start time for all three components
		RecordSequence::iterator it[3];
		RecordSequence::iterator it_end[3];
		int maxStartComponent;
		int skips;
		double samplingFrequency, timeTolerance;

		// Initialize iterators for each component
		for ( int i = 0; i < 3; ++i )
			it[i] = traces[i].raw->begin();

		// Store sampling frequency of first record of first component
		// All records must match this sampling frequency
		samplingFrequency = (*it[0])->samplingFrequency();
		timeTolerance = 0.5 / samplingFrequency;

		while ( true ) {
			if ( minStartTime ) {
				for ( int i = 0; i < 3; ++i ) {
					while ( it[i] != traces[i].raw->end() ) {
						if ( (*it[i])->endTime() <= minStartTime ) {
							++it[i];
						}
						else
							break;
					}

					// End of stream?
					if ( it[i] == traces[i].raw->end() ) {
						return gotRecords;
					}
				}
			}

			// Advance all other components to first record matching
			// the first sampling frequency found
			for ( int i = 0; i < 3; ++i ) {
				while ( ((*it[i])->samplingFrequency() != samplingFrequency) ) {
					++it[i];
					// No matching sampling frequency found?
					if ( it[i] == traces[i].raw->end() )
						return gotRecords;
				}
			}

			// Find maximum start time of all three records
			skips = 1;
			while ( skips ) {
				for ( int i = 0; i < 3; ++i ) {
					if ( !i || maxStartTime < (*it[i])->startTime() ) {
						maxStartTime = (*it[i])->startTime();
						maxStartComponent = i;
					}
				}

				skips = 0;

				// Check all other components against maxStartTime
				for ( int i = 0; i < 3; ++i ) {
					if ( i == maxStartComponent ) continue;
					while ( (*it[i])->samplingFrequency() != samplingFrequency ||
					        (*it[i])->endTime() <= maxStartTime ) {
						++it[i];

						// End of sequence? Nothing can be done anymore
						if ( it[i] == traces[i].raw->end() )
							return gotRecords;

						// Increase skip counter
						++skips;
					}
				}
			}

			// Advance all iterators to last non-gappy record
			for ( int i = 0; i < 3; ++i ) {
				RecordSequence::iterator tmp = it[i];
				it_end[i] = it[i];
				++it_end[i];
				while ( it_end[i] != traces[i].raw->end() ) {
					const Record *rec = it_end[i]->get();

					// Skip records with wrong sampling frequency
					if ( rec->samplingFrequency() != samplingFrequency ) break;

					double diff = (double)(rec->startTime()-(*tmp)->endTime());
					if ( fabs(diff) > timeTolerance ) break;

					tmp = it_end[i];
					++it_end[i];
				}

				it_end[i] = tmp;
			}

			// Find minimum end time of all three records
			for ( int i = 0; i < 3; ++i ) {
				if ( !i || minEndTime > (*it_end[i])->endTime() )
					minEndTime = (*it_end[i])->endTime();
			}


			GenericRecordPtr comps[3];
			int minLen = 0;

			// Clip maxStartTime to minStartTime
			if ( maxStartTime < minStartTime )
				maxStartTime = minStartTime;

			// Rotate records
			for ( int i = 0; i < 3; ++i ) {
				float tq = 0;
				int tqCount = 0;
				GenericRecordPtr rec = new GenericRecord((*it[i])->networkCode(),
				                                         (*it[i])->stationCode(),
				                                         (*it[i])->locationCode(),
				                                         (*it[i])->channelCode(),
				                                         maxStartTime, samplingFrequency);

				FloatArrayPtr data = new FloatArray;
				RecordSequence::iterator seq_end = it_end[i];
				++seq_end;

				for ( RecordSequence::iterator rec_it = it[i]; rec_it != seq_end; ++rec_it ) {
					const Array *rec_data = (*rec_it)->data();
					if ( rec_data == NULL ) {
						SEISCOMP_ERROR("%s: no data for record", (*rec_it)->streamID().c_str());
						return gotRecords;
					}

					if ( (*rec_it)->startTime() > minEndTime )
						break;

					++it[i];

					const FloatArray *srcData = FloatArray::ConstCast(rec_data);
					FloatArrayPtr tmp;
					if ( srcData == NULL ) {
						tmp = (FloatArray*)data->copy(Array::FLOAT);
						srcData = tmp.get();
					}

					int startIndex = 0;
					int endIndex = srcData->size();

					if ( (*rec_it)->startTime() < maxStartTime )
						startIndex += (int)(double(maxStartTime-(*rec_it)->startTime())*(*rec_it)->samplingFrequency());

					if ( (*rec_it)->endTime() > minEndTime )
						endIndex -= (int)(double((*rec_it)->endTime()-minEndTime)*(*rec_it)->samplingFrequency());

					int len = endIndex-startIndex;
					// Skip empty records
					if ( len <= 0 ) continue;

					if ( (*rec_it)->timingQuality() >= 0 ) {
						tq += (*rec_it)->timingQuality();
						++tqCount;
					}

					data->append(len, srcData->typedData()+startIndex);
				}

				if ( tqCount > 0 )
					rec->setTimingQuality((int)(tq / tqCount));

				minLen = i==0?data->size():std::min(minLen, data->size());

				rec->setData(data.get());

				comps[i] = rec;
			}

			// Create record sequences
			for ( int i = 0; i < 3; ++i ) {
				FloatArray *data = static_cast<FloatArray*>(comps[i]->data());
				if ( data->size() > minLen ) {
					data->resize(minLen);
					comps[i]->dataUpdated();
				}

				// Create ring buffer without limit if needed
				if ( traces[i].transformed == NULL ) {
					traces[i].transformed = new RingBuffer(0);
					if ( widget && !showProcessed )
						widget->setRecords(traces[i].recordSlot, traces[i].transformed, false);
					if ( traces[i].filter )
						traces[i].filter->setSamplingFrequency(comps[i]->samplingFrequency());
				}

				transformedRecord(i, comps[i].get());
			}

			gotRecords = true;

			float *dataZ = static_cast<FloatArray*>(comps[0]->data())->typedData();
			float *data1 = static_cast<FloatArray*>(comps[1]->data())->typedData();
			float *data2 = static_cast<FloatArray*>(comps[2]->data())->typedData();

			// Rotate finally
			for ( int i = 0; i < minLen; ++i ) {
				Math::Vector3f v = transformation*Math::Vector3f(*data2, *data1, *dataZ);
				*dataZ = v.z;
				*data1 = v.y;
				*data2 = v.x;

				++dataZ; ++data1; ++data2;
			}

			// And filter
			for ( int i = 0; i < 3; ++i ) {
				if ( traces[i].filter )
					traces[i].filter->apply(*static_cast<FloatArray*>(comps[i]->data()));
			}

			minStartTime = minEndTime;
		}
	}
	else {
		// Record passed that needs filtering?
		if ( rec ) {
			if ( traces[comp].transformed == NULL ) {
				traces[comp].transformed = new RingBuffer(0);
				if ( widget && !showProcessed )
					widget->setRecords(traces[comp].recordSlot, traces[comp].transformed, false);
				if ( traces[comp].filter )
					traces[comp].filter->setSamplingFrequency(rec->samplingFrequency());
			}

			RecordPtr toFeed;
			if ( traces[comp].filter ) {
				GenericRecordPtr grec = new GenericRecord(rec->networkCode(),
				                                          rec->stationCode(),
				                                          rec->locationCode(),
				                                          rec->channelCode(),
				                                          rec->startTime(),
				                                          rec->samplingFrequency(),
				                                          rec->timingQuality());

				FloatArrayPtr data = (FloatArray*)rec->data()->copy(Array::FLOAT);
				traces[comp].filter->apply(*data);
				grec->setData(data.get());
				toFeed = grec;
			}
			else
				toFeed = rec;

			transformedRecord(comp, toFeed.get());
			gotRecords = true;
		}
		else {
			// Just copy the records and filter them if activated
			for ( int i = 0; i < 3; ++i ) {
				if ( traces[i].raw == NULL || traces[i].raw->empty() ) continue;

				RecordSequence::iterator it;
				if ( traces[i].transformed == NULL )
					it = traces[i].raw->begin();
				else {
					Core::Time endTime = traces[i].transformed->back()->endTime();
					for ( RecordSequence::iterator s_it = traces[i].raw->begin();
					      s_it != traces[i].raw->end(); ++s_it ) {
						if ( (*s_it)->startTime() >= endTime ) {
							it = s_it;
							break;
						}
					}
				}

				for ( RecordSequence::iterator s_it = it;
				      s_it != traces[i].raw->end(); ++s_it ) {

					RecordCPtr s_rec = s_it->get();

					if ( traces[i].transformed == NULL ) {
						traces[i].transformed = new RingBuffer(0);
						if ( widget && !showProcessed )
							widget->setRecords(traces[i].recordSlot, traces[i].transformed, false);
						if ( traces[i].filter )
							traces[i].filter->setSamplingFrequency(s_rec->samplingFrequency());
					}

					if ( traces[i].filter ) {
						GenericRecordPtr grec = new GenericRecord(s_rec->networkCode(),
						                                          s_rec->stationCode(),
						                                          s_rec->locationCode(),
						                                          s_rec->channelCode(),
						                                          s_rec->startTime(),
						                                          s_rec->samplingFrequency(),
						                                          s_rec->timingQuality());

						FloatArrayPtr data = (FloatArray*)s_rec->data()->copy(Array::FLOAT);
						traces[i].filter->apply(*data);
						grec->setData(data.get());
						s_rec = grec;
					}

					transformedRecord(i, s_rec.get());
					gotRecords = true;
				}
			}
		}
	}

	return gotRecords;
}


void ThreeComponentTrace::transformedRecord(int comp, const Record *rec) {
	traces[comp].transformed->feed(rec);
	if ( widget && !showProcessed ) widget->fed(traces[comp].recordSlot, rec);

	if ( label->processor ) {
		Processing::WaveformProcessor::StreamComponent c = label->processor->usedComponent();
		switch ( c ) {
			case Processing::WaveformProcessor::Vertical:
				if ( comp == 0 ) {
					label->processor->feed(rec);
					label->updateProcessingInfo();
				}
				break;
			case Processing::WaveformProcessor::FirstHorizontal:
				if ( comp == 1 ) {
					label->processor->feed(rec);
					label->updateProcessingInfo();
				}
				break;
			case Processing::WaveformProcessor::SecondHorizontal:
				if ( comp == 2 ) {
					label->processor->feed(rec);
					label->updateProcessingInfo();
				}
				break;
			case Processing::WaveformProcessor::Horizontal:
				if ( comp == 1 || comp == 2 ) {
					label->processor->feed(rec);
					label->updateProcessingInfo();
				}
				break;
			case Processing::WaveformProcessor::Any:
				label->processor->feed(rec);
				label->updateProcessingInfo();
				break;
			default:
				break;
		}

		if ( label->processor->isFinished() ) {
			for ( int i = 0; i < 3; ++i ) {
				const Processing::AmplitudeProcessor *compProc = label->processor->componentProcessor((Processing::WaveformProcessor::Component)i);
				if ( compProc == NULL ) continue;
				const DoubleArray *processedData = compProc->processedData((Processing::WaveformProcessor::Component)i);
				if ( traces[i].processed == NULL && processedData )
					setProcessedData(
						i, rec->networkCode(),
						rec->stationCode(),
						rec->locationCode(),
						compProc->dataTimeWindow().startTime(),
						compProc->samplingFrequency(),
						FloatArray::Cast(processedData->copy(Array::FLOAT))
					);
			}
		}
	}
}


AmplitudeRecordLabel::AmplitudeRecordLabel(int items, QWidget *parent, const char* name)
	: StandardRecordLabel(items, parent, name), _isLinkedItem(false), _isExpanded(false) {
	_btnExpand = NULL;
	_linkedItem = NULL;

	latitude = 999;
	longitude = 999;
	isError = false;
	data.label = this;

	hasGotData = false;
	isEnabledByConfig = false;
}

AmplitudeRecordLabel::~AmplitudeRecordLabel() {}

void AmplitudeRecordLabel::setLinkedItem(bool li) {
	_isLinkedItem = li;
}

void AmplitudeRecordLabel::setControlledItem(RecordViewItem *controlledItem) {
	_linkedItem = controlledItem;
	static_cast<AmplitudeRecordLabel*>(controlledItem->label())->_linkedItem = recordViewItem();
}

RecordViewItem *AmplitudeRecordLabel::controlledItem() const {
	return _linkedItem;
}

void AmplitudeRecordLabel::enabledExpandButton(RecordViewItem *controlledItem) {
	if ( _btnExpand ) return;

	_btnExpand = new QPushButton(this);
	_btnExpand->resize(16,16);
	_btnExpand->move(width() - _btnExpand->width(), height() - _btnExpand->height());
	_btnExpand->setIcon(QIcon(QString::fromUtf8(":/icons/icons/arrow_down.png")));
	_btnExpand->setFlat(true);
	_btnExpand->show();

	connect(_btnExpand, SIGNAL(clicked()),
	        this, SLOT(extentButtonPressed()));

	if ( !_linkedItem )
		setControlledItem(controlledItem);

	_isExpanded = false;
}

void AmplitudeRecordLabel::disableExpandButton() {
	if ( _btnExpand ) {
		delete _btnExpand;
		_btnExpand = NULL;
	}

	_linkedItem = NULL;
}

void AmplitudeRecordLabel::unlink() {
	if ( _linkedItem ) {
		static_cast<AmplitudeRecordLabel*>(_linkedItem->label())->disableExpandButton();
		_linkedItem = NULL;
	}
}

bool AmplitudeRecordLabel::isLinkedItem() const {
	return _isLinkedItem;
}

bool AmplitudeRecordLabel::isExpanded() const {
	return _isExpanded;
}


void AmplitudeRecordLabel::visibilityChanged(bool v) {
	if ( _linkedItem && !_isLinkedItem ) {
		if ( !v )
			_linkedItem->setVisible(false);
		else if ( _isExpanded )
			_linkedItem->setVisible(true);
	}
}

void AmplitudeRecordLabel::resizeEvent(QResizeEvent *e) {
	StandardRecordLabel::resizeEvent(e);
	if ( _btnExpand ) {
		_btnExpand->move(e->size().width() - _btnExpand->width(),
		                 e->size().height() - _btnExpand->height());
	}
}

void AmplitudeRecordLabel::enableExpandable(const Seiscomp::Record *rec) {
	enabledExpandButton(static_cast<RecordViewItem*>(sender()));
}

void AmplitudeRecordLabel::extentButtonPressed() {
	_isExpanded = !_isExpanded;
	_btnExpand->setIcon(QIcon(QString::fromUtf8(_isExpanded?":/icons/icons/arrow_up.png":":/icons/icons/arrow_down.png")));
	if ( _linkedItem ) {
		if ( !_isExpanded ) {
			recordViewItem()->recordView()->setCurrentItem(recordViewItem());
			_linkedItem->recordView()->setItemSelected(_linkedItem, false);
		}
		_linkedItem->setVisible(_isExpanded);
	}
}

void AmplitudeRecordLabel::paintEvent(QPaintEvent *e) {
	QPainter p(this);

	if ( _hasLabelColor ) {
		QRect r(rect());

		r.setLeft(r.right()-16);

		QColor bg = palette().color(QPalette::Window);
		QLinearGradient gradient(r.left(), 0, r.right(), 0);
		gradient.setColorAt(0, bg);
		gradient.setColorAt(1, _labelColor);

		p.fillRect(r, gradient);
	}

	if ( _items.count() == 0 ) return;

	int w = width();
	int h = height();

	int posX = 0;

	int fontSize = p.fontMetrics().ascent();
	int posY = (h - fontSize*2 - 4)/2;

	for ( int i = 0; i < _items.count()-1; ++i ) {
		if ( _items[i].text.isEmpty() ) continue;
		p.setFont(_items[i].font);
		p.setPen(isEnabled() ? _items[i].color : palette().color(QPalette::Disabled, QPalette::WindowText));
		p.drawText(posX,posY, w, fontSize, _items[i].align, _items[i].text);

		if ( _items[i].width < 0 )
			posX += p.fontMetrics().boundingRect(_items[i].text).width();
		else
			posX += _items[i].width;
	}

	posY += fontSize + 4;
	p.setPen(isEnabled() ? _items.last().color : palette().color(QPalette::Disabled, QPalette::WindowText));
	p.drawText(0,posY, _items.last().width < 0?w-18:std::min(_items.last().width,w-18), fontSize, _items.last().align, _items.last().text);
}

void AmplitudeRecordLabel::setLabelColor(QColor c) {
	_labelColor = c;
	_hasLabelColor = true;
	update();
}

void AmplitudeRecordLabel::removeLabelColor() {
	_hasLabelColor = false;
	update();
}


void AmplitudeRecordLabel::updateProcessingInfo() {
	if ( !processor ) {
		infoText = QString();
		return;
	}

	switch ( processor->status() ) {
		case Processing::WaveformProcessor::WaitingForData:
			infoText = processor->status().toString();
			break;
		case Processing::WaveformProcessor::Finished:
			infoText = QString();
			break;
		case Processing::WaveformProcessor::Terminated:
			infoText = processor->status().toString();
			break;
		case Processing::WaveformProcessor::InProgress:
			infoText = QString("%1: %2%")
			           .arg(processor->status().toString())
			           .arg(processor->statusValue(),0,'f',1);
			break;
		case Processing::WaveformProcessor::LowSNR:
			infoText = QString("%1: %2 < %3")
			           .arg(processor->status().toString())
			           .arg(processor->statusValue(),0,'f',1)
			           .arg(processor->config().snrMin,0,'f',1);
			break;
		default:
			infoText = QString("%1(%2)")
			           .arg(processor->status().toString())
			           .arg(processor->statusValue(),0,'f',1);
			break;
	}

	isError = processor->status() > Processing::WaveformProcessor::Finished;
}



}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeView::Config::Config() {
	timingQualityLow = Qt::darkRed;
	timingQualityMedium = Qt::yellow;
	timingQualityHigh = Qt::darkGreen;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeView::AmplitudeView(QWidget *parent, Qt::WFlags f)
: QMainWindow(parent,f) {
	_recordView = new TraceList();
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
AmplitudeView::~AmplitudeView() {
	for ( int i = 0; i < _recordView->rowCount(); ++i )
		_recordView->itemAt(i)->widget()->setShadowWidget(NULL, false);

	if ( _currentFilter ) delete _currentFilter;

	closeThreads();

	QList<int> sizes = _ui.splitter->sizes();

	if ( SCApp ) {
		SCApp->settings().beginGroup(objectName());

		SCApp->settings().setValue("geometry", saveGeometry());
		SCApp->settings().setValue("state", saveState());

		if ( sizes.count() >= 2 ) {
			SCApp->settings().setValue("splitter/upper", sizes[0]);
			SCApp->settings().setValue("splitter/lower", sizes[1]);
		}

		SCApp->settings().endGroup();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordLabel* AmplitudeView::createLabel(RecordViewItem *item) const {
	return new AmplitudeRecordLabel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::init() {
	setObjectName("Amplitudes");

#ifdef MACOSX
	Mac::addFullscreen(this);
#endif

	_ui.setupUi(this);

	_ui.labelStationCode->setFont(SCScheme.fonts.heading3);
	_ui.labelCode->setFont(SCScheme.fonts.normal);

	_settingsRestored = false;
	_currentSlot = -1;
	_currentFilter = NULL;
	_currentFilterStr = "";
	_autoScaleZoomTrace = true;
	_showProcessedData = true;

	_reader = NULL;

	_zoom = 1.0;
	_currentAmplScale = 1.0;

	_centerSelection = false;
	_checkVisibility = true;

	insertToolBarBreak(_ui.toolBarFilter);

	_recordView->setSelectionMode(RecordView::SingleSelection);
	_recordView->setMinimumRowHeight(fontMetrics().ascent()*2+6);
	_recordView->setDefaultRowHeight(fontMetrics().ascent()*2+6);
	_recordView->setSelectionEnabled(false);
	_recordView->setRecordUpdateInterval(1000);

	connect(_recordView, SIGNAL(currentItemChanged(RecordViewItem*, RecordViewItem*)),
	        this, SLOT(itemSelected(RecordViewItem*, RecordViewItem*)));

	connect(_recordView, SIGNAL(fedRecord(RecordViewItem*, const Seiscomp::Record*)),
	        this, SLOT(updateTraceInfo(RecordViewItem*, const Seiscomp::Record*)));

	connect(_recordView, SIGNAL(filterChanged(const QString&)),
	        this, SLOT(addNewFilter(const QString&)));

	connect(_recordView, SIGNAL(progressStarted()),
	        this, SLOT(beginWaitForRecords()));

	connect(_recordView, SIGNAL(progressChanged(int)),
	        this, SLOT(doWaitForRecords(int)));

	connect(_recordView, SIGNAL(progressFinished()),
	        this, SLOT(endWaitForRecords()));

	_recordView->setAlternatingRowColors(true);
	_recordView->setAutoInsertItem(false);
	_recordView->setAutoScale(true);
	_recordView->setRowSpacing(2);
	_recordView->setHorizontalSpacing(6);
	_recordView->setFramesEnabled(false);
	//_recordView->setDefaultActions();

	_recordView->timeWidget()->setSelectionHandleCount(3);

	_connectionState = new ConnectionStateLabel(this);
	connect(_connectionState, SIGNAL(customInfoWidgetRequested(const QPoint &)),
	        this, SLOT(openConnectionInfo(const QPoint &)));

	QBoxLayout* layout = new QVBoxLayout(_ui.frameTraces);
	layout->setMargin(2);
	layout->setSpacing(0);
	layout->addWidget(_recordView);

	_searchStation = new QLineEdit();
	_searchStation->setVisible(false);

	_searchBase = _searchStation->palette().color(QPalette::Base);
	_searchError = blend(Qt::red, _searchBase, 50);

	_searchLabel = new QLabel();
	_searchLabel->setVisible(false);
	_searchLabel->setText(tr("Type the station code to search for"));

	connect(_searchStation, SIGNAL(textChanged(const QString&)),
	        this, SLOT(search(const QString&)));

	connect(_searchStation, SIGNAL(returnPressed()),
	        this, SLOT(nextSearch()));

	statusBar()->addPermanentWidget(_searchStation, 1);
	statusBar()->addPermanentWidget(_searchLabel, 5);
	statusBar()->addPermanentWidget(_connectionState);

	_currentRecord = new MyRecordWidget();
	_currentRecord->showScaledValues(_ui.actionShowTraceValuesInNmS->isChecked());
	_currentRecord->setClippingEnabled(_ui.actionClipComponentsToViewport->isChecked());
	_currentRecord->setMouseTracking(true);
	_currentRecord->setContextMenuPolicy(Qt::CustomContextMenu);

	//_currentRecord->setFocusPolicy(Qt::StrongFocus);

	//_currentRecord->setDrawMode(RecordWidget::Single);
	//_currentRecord->setDrawMode(RecordWidget::InRows);
	/*
	_currentRecord->setRecordColor(0, Qt::red);
	_currentRecord->setRecordColor(1, Qt::green);
	_currentRecord->setRecordColor(2, Qt::blue);
	*/

	/*
	connect(_currentRecord, SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(openRecordContextMenu(const QPoint &)));
	*/

	layout = new QVBoxLayout(_ui.frameCurrentTrace);
	layout->setMargin(0);
	layout->setSpacing(0);

	_timeScale = new TimeScale();
	_timeScale->setSelectionEnabled(true);
	_timeScale->setRangeSelectionEnabled(true);
	_timeScale->setAbsoluteTimeEnabled(true);
	_timeScale->setSelectionHandleCount(3);

	layout->addWidget(_currentRecord);
	layout->addWidget(_timeScale);

	connect(_timeScale, SIGNAL(dragged(double)),
	        this, SLOT(move(double)));
	connect(_timeScale, SIGNAL(dragStarted()),
	        this, SLOT(disableAutoScale()));
	connect(_timeScale, SIGNAL(dragFinished()),
	        this, SLOT(enableAutoScale()));
	connect(_timeScale, SIGNAL(rangeChangeRequested(double,double)),
	        this, SLOT(applyTimeRange(double,double)));
	connect(_timeScale, SIGNAL(selectionHandleMoved(int,double,Qt::KeyboardModifiers)),
	        this, SLOT(zoomSelectionHandleMoved(int,double,Qt::KeyboardModifiers)));
	connect(_timeScale, SIGNAL(selectionHandleMoveFinished()),
	        this, SLOT(zoomSelectionHandleMoveFinished()));

	connect(_recordView->timeWidget(), SIGNAL(dragged(double)),
	        this, SLOT(moveTraces(double)));
	connect(_recordView->timeWidget(), SIGNAL(selectionHandleMoved(int,double,Qt::KeyboardModifiers)),
	        this, SLOT(selectionHandleMoved(int,double,Qt::KeyboardModifiers)));
	connect(_recordView->timeWidget(), SIGNAL(selectionHandleMoveFinished()),
	        this, SLOT(selectionHandleMoveFinished()));

	connect(_recordView, SIGNAL(updatedRecords()),
	        _currentRecord, SLOT(updateRecords()));

	QPalette pal = _currentRecord->palette();
	pal.setColor(_currentRecord->backgroundRole(), Qt::white);
	pal.setColor(_currentRecord->foregroundRole(), Qt::black);
	_currentRecord->setPalette(pal);

	// add actions
	addAction(_ui.actionIncreaseAmplitudeScale);
	addAction(_ui.actionDecreaseAmplitudeScale);
	addAction(_ui.actionTimeScaleUp);
	addAction(_ui.actionTimeScaleDown);
	addAction(_ui.actionClipComponentsToViewport);

	addAction(_ui.actionIncreaseRowHeight);
	addAction(_ui.actionDecreaseRowHeight);
	addAction(_ui.actionIncreaseRowTimescale);
	addAction(_ui.actionDecreaseRowTimescale);

	addAction(_ui.actionScrollLeft);
	addAction(_ui.actionScrollFineLeft);
	addAction(_ui.actionScrollRight);
	addAction(_ui.actionScrollFineRight);
	addAction(_ui.actionSelectNextTrace);
	addAction(_ui.actionSelectPreviousTrace);
	addAction(_ui.actionSelectFirstRow);
	addAction(_ui.actionSelectLastRow);

	addAction(_ui.actionDefaultView);

	addAction(_ui.actionSortAlphabetically);
	addAction(_ui.actionSortByDistance);

	addAction(_ui.actionShowZComponent);
	addAction(_ui.actionShowNComponent);
	addAction(_ui.actionShowEComponent);
	
	addAction(_ui.actionAlignOnOriginTime);
	addAction(_ui.actionAlignOnPArrival);

	addAction(_ui.actionToggleFilter);
	addAction(_ui.actionMaximizeAmplitudes);

	addAction(_ui.actionCreateAmplitude);
	addAction(_ui.actionSetAmplitude);
	addAction(_ui.actionConfirmAmplitude);
	addAction(_ui.actionDeleteAmplitude);

	addAction(_ui.actionShowZComponent);
	addAction(_ui.actionShowNComponent);
	addAction(_ui.actionShowEComponent);

	addAction(_ui.actionGotoNextMarker);
	addAction(_ui.actionGotoPreviousMarker);

	addAction(_ui.actionComputeMagnitudes);
	addAction(_ui.actionSwitchFullscreen);
	addAction(_ui.actionAddStations);
	addAction(_ui.actionSearchStation);

	addAction(_ui.actionRecalculateAmplitude);
	addAction(_ui.actionRecalculateAmplitudes);

	_lastFilterIndex = -1;

	_comboFilter = new QComboBox;
	//_comboFilter->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	_comboFilter->setDuplicatesEnabled(false);
	_comboFilter->addItem(NO_FILTER_STRING);
	_comboFilter->addItem(DEFAULT_FILTER_STRING);

	_comboFilter->setCurrentIndex(1);
	changeFilter(_comboFilter->currentIndex());

	_spinSNR = new QDoubleSpinBox;
	_spinSNR->setRange(0, 10000);
	_spinSNR->setDecimals(2);
	_spinSNR->setSingleStep(1);
	//_spinSNR->setPrefix("Min. SNR ");
	_spinSNR->setSpecialValueText("Disabled");

	_comboAmpType = new QComboBox;
	_comboAmpType->setEnabled(false);

	_comboAmpCombiner = new QComboBox;
	_comboAmpCombiner->setEnabled(false);

	connect(_ui.actionRecalculateAmplitude, SIGNAL(triggered()),
	        this, SLOT(recalculateAmplitude()));
	connect(_ui.actionRecalculateAmplitudes, SIGNAL(triggered()),
	        this, SLOT(recalculateAmplitudes()));

	_ui.toolBarFilter->insertWidget(_ui.actionToggleFilter, _comboFilter);
	_ui.toolBarSetup->insertWidget(_ui.actionRecalculateAmplitude, new QLabel("Min SNR:"));
	_ui.toolBarSetup->insertWidget(_ui.actionRecalculateAmplitude, _spinSNR);
	_ui.toolBarSetup->insertSeparator(_ui.actionRecalculateAmplitude);
	_ui.toolBarSetup->insertWidget(_ui.actionRecalculateAmplitude, _labelAmpType = new QLabel("Amp.type:"));
	_ui.toolBarSetup->insertWidget(_ui.actionRecalculateAmplitude, _comboAmpType);
	_ui.toolBarSetup->insertSeparator(_ui.actionRecalculateAmplitude);
	_ui.toolBarSetup->insertWidget(_ui.actionRecalculateAmplitude, _labelAmpCombiner = new QLabel("Amp.combiner:"));
	_ui.toolBarSetup->insertWidget(_ui.actionRecalculateAmplitude, _comboAmpCombiner);

	_labelAmpType->setEnabled(false);
	_labelAmpCombiner->setEnabled(false);

	connect(_comboFilter, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(changeFilter(int)));

	connect(_ui.actionLimitFilterToZoomTrace, SIGNAL(triggered(bool)),
	        this, SLOT(limitFilterToZoomTrace(bool)));

	_spinDistance = new QDoubleSpinBox;
	_spinDistance->setValue(15);

	if ( SCScheme.unit.distanceInKM ) {
		_spinDistance->setRange(0, 25000);
		_spinDistance->setDecimals(0);
		_spinDistance->setSuffix("km");
	}
	else {
		_spinDistance->setRange(0, 180);
		_spinDistance->setDecimals(1);
		_spinDistance->setSuffix(degrees);
	}

	_ui.toolBarStations->insertWidget(_ui.actionShowAllStations, _spinDistance);

	/*
	connect(_spinDistance, SIGNAL(editingFinished()),
	        this, SLOT(loadNextStations()));
	*/

	// connect actions
	connect(_ui.actionDefaultView, SIGNAL(triggered(bool)),
	        this, SLOT(setDefaultDisplay()));
	connect(_ui.actionSortAlphabetically, SIGNAL(triggered(bool)),
	        this, SLOT(sortAlphabetically()));
	connect(_ui.actionSortByDistance, SIGNAL(triggered(bool)),
	        this, SLOT(sortByDistance()));

	connect(_ui.actionShowZComponent, SIGNAL(triggered(bool)),
	        this, SLOT(showZComponent()));
	connect(_ui.actionShowNComponent, SIGNAL(triggered(bool)),
	        this, SLOT(showNComponent()));
	connect(_ui.actionShowEComponent, SIGNAL(triggered(bool)),
	        this, SLOT(showEComponent()));
	
	connect(_ui.actionAlignOnOriginTime, SIGNAL(triggered(bool)),
	        this, SLOT(alignOnOriginTime()));
	connect(_ui.actionAlignOnPArrival, SIGNAL(triggered(bool)),
	        this, SLOT(alignOnPArrivals()));

	connect(_ui.actionIncreaseAmplitudeScale, SIGNAL(triggered(bool)),
	        this, SLOT(scaleAmplUp()));
	connect(_ui.actionDecreaseAmplitudeScale, SIGNAL(triggered(bool)),
	        this, SLOT(scaleAmplDown()));
	connect(_ui.actionTimeScaleUp, SIGNAL(triggered(bool)),
	        this, SLOT(scaleTimeUp()));
	connect(_ui.actionTimeScaleDown, SIGNAL(triggered(bool)),
	        this, SLOT(scaleTimeDown()));
	connect(_ui.actionClipComponentsToViewport, SIGNAL(triggered(bool)),
	        _currentRecord, SLOT(setClippingEnabled(bool)));
	connect(_ui.actionScrollLeft, SIGNAL(triggered(bool)),
	        this, SLOT(scrollLeft()));
	connect(_ui.actionScrollFineLeft, SIGNAL(triggered(bool)),
	        this, SLOT(scrollFineLeft()));
	connect(_ui.actionScrollRight, SIGNAL(triggered(bool)),
	        this, SLOT(scrollRight()));
	connect(_ui.actionScrollFineRight, SIGNAL(triggered(bool)),
	        this, SLOT(scrollFineRight()));
	/*
	connect(_ui.actionGotoNextMarker, SIGNAL(triggered(bool)),
	        this, SLOT(gotoNextMarker()));
	connect(_ui.actionGotoPreviousMarker, SIGNAL(triggered(bool)),
	        this, SLOT(gotoPreviousMarker()));
	*/
	connect(_ui.actionSelectNextTrace, SIGNAL(triggered(bool)),
	        _recordView, SLOT(selectNextRow()));
	connect(_ui.actionSelectPreviousTrace, SIGNAL(triggered(bool)),
	        _recordView, SLOT(selectPreviousRow()));
	connect(_ui.actionSelectFirstRow, SIGNAL(triggered(bool)),
	        _recordView, SLOT(selectFirstRow()));
	connect(_ui.actionSelectLastRow, SIGNAL(triggered(bool)),
	        _recordView, SLOT(selectLastRow()));
	connect(_ui.actionIncreaseRowHeight, SIGNAL(triggered(bool)),
	        _recordView, SLOT(verticalZoomIn()));
	connect(_ui.actionDecreaseRowHeight, SIGNAL(triggered(bool)),
	        _recordView, SLOT(verticalZoomOut()));
	connect(_ui.actionIncreaseRowTimescale, SIGNAL(triggered(bool)),
	        _recordView, SLOT(horizontalZoomIn()));
	connect(_ui.actionDecreaseRowTimescale, SIGNAL(triggered(bool)),
	        _recordView, SLOT(horizontalZoomOut()));
	connect(_ui.actionShowTraceValuesInNmS, SIGNAL(triggered(bool)),
	        this, SLOT(showTraceScaleToggled(bool)));

	connect(_ui.actionToggleFilter, SIGNAL(triggered(bool)),
	        this, SLOT(toggleFilter()));

	connect(_ui.actionMaximizeAmplitudes, SIGNAL(triggered(bool)),
	        this, SLOT(scaleVisibleAmplitudes()));

	connect(_ui.actionPickAmplitude, SIGNAL(triggered(bool)),
	        this, SLOT(pickAmplitudes(bool)));
	connect(_ui.actionDisablePicking, SIGNAL(triggered(bool)),
	        this, SLOT(pickNone(bool)));
	connect(_ui.actionDisablePicking, SIGNAL(triggered(bool)),
	        this, SLOT(abortSearchStation()));

	connect(_ui.actionCreateAmplitude, SIGNAL(triggered(bool)),
	        this, SLOT(createAmplitude()));
	connect(_ui.actionSetAmplitude, SIGNAL(triggered(bool)),
	        this, SLOT(setAmplitude()));
	connect(_ui.actionConfirmAmplitude, SIGNAL(triggered(bool)),
	        this, SLOT(confirmAmplitude()));
	connect(_ui.actionDeleteAmplitude, SIGNAL(triggered(bool)),
	        this, SLOT(deleteAmplitude()));

	connect(_ui.actionComputeMagnitudes, SIGNAL(triggered(bool)),
	        this, SLOT(commit()));

	connect(_ui.actionShowAllStations, SIGNAL(triggered(bool)),
	        this, SLOT(loadNextStations()));

	connect(_ui.actionShowUsedStations, SIGNAL(triggered(bool)),
	        this, SLOT(showUsedStations(bool)));

	connect(_ui.btnAmplScaleUp, SIGNAL(clicked()),
	        this, SLOT(scaleAmplUp()));
	connect(_ui.btnAmplScaleDown, SIGNAL(clicked()),
	        this, SLOT(scaleAmplDown()));
	connect(_ui.btnTimeScaleUp, SIGNAL(clicked()),
	        this, SLOT(scaleTimeUp()));
	connect(_ui.btnTimeScaleDown, SIGNAL(clicked()),
	        this, SLOT(scaleTimeDown()));
	connect(_ui.btnScaleReset, SIGNAL(clicked()),
	        this, SLOT(scaleReset()));

	connect(_ui.btnRowAccept, SIGNAL(clicked()),
	        this, SLOT(confirmAmplitude()));
	connect(_ui.btnRowRemove, SIGNAL(clicked(bool)),
	        this, SLOT(setCurrentRowDisabled(bool)));
	connect(_ui.btnRowRemove, SIGNAL(clicked(bool)),
	        _recordView, SLOT(selectNextRow()));
	connect(_ui.btnRowReset, SIGNAL(clicked(bool)),
	        this, SLOT(deleteAmplitude()));
	connect(_ui.btnRowReset, SIGNAL(clicked(bool)),
	        _recordView, SLOT(selectNextRow()));

	connect(_currentRecord, SIGNAL(cursorUpdated(RecordWidget*,int)),
	        this, SLOT(updateSubCursor(RecordWidget*,int)));

	connect(_currentRecord, SIGNAL(clickedOnTime(Seiscomp::Core::Time)),
	        this, SLOT(updateRecordValue(Seiscomp::Core::Time)));

	connect(_ui.frameZoom, SIGNAL(lineDown()),
	        _recordView, SLOT(selectNextRow()));
	connect(_ui.frameZoom, SIGNAL(lineUp()),
	        _recordView, SLOT(selectPreviousRow()));

	connect(_ui.frameZoom, SIGNAL(verticalZoomIn()),
	        this, SLOT(scaleAmplUp()));
	connect(_ui.frameZoom, SIGNAL(verticalZoomOut()),
	        this, SLOT(scaleAmplDown()));

	connect(_ui.frameZoom, SIGNAL(horizontalZoomIn()),
	        this, SLOT(scaleTimeUp()));
	connect(_ui.frameZoom, SIGNAL(horizontalZoomOut()),
	        this, SLOT(scaleTimeDown()));

	connect(_ui.actionSwitchFullscreen, SIGNAL(triggered(bool)),
	        this, SLOT(showFullscreen(bool)));

	connect(_timeScale, SIGNAL(changedInterval(double, double, double)),
	        _currentRecord, SLOT(setGridSpacing(double, double, double)));
	connect(_recordView, SIGNAL(toggledFilter(bool)),
	        _currentRecord, SLOT(enableFiltering(bool)));
	connect(_recordView, SIGNAL(scaleChanged(double, float)),
	        this, SLOT(changeScale(double, float)));
	connect(_recordView, SIGNAL(timeRangeChanged(double, double)),
	        this, SLOT(changeTimeRange(double, double)));
	connect(_recordView, SIGNAL(selectionChanged(double, double)),
	        _currentRecord, SLOT(setSelected(double, double)));
	connect(_recordView, SIGNAL(alignmentChanged(const Seiscomp::Core::Time&)),
	        this, SLOT(setAlignment(Seiscomp::Core::Time)));
	connect(_recordView, SIGNAL(amplScaleChanged(float)),
	        _currentRecord, SLOT(setAmplScale(float)));

	connect(_ui.actionAddStations, SIGNAL(triggered(bool)),
	        this, SLOT(addStations()));

	connect(_ui.actionSearchStation, SIGNAL(triggered(bool)),
	        this, SLOT(searchStation()));

	connect(_recordView, SIGNAL(selectedTime(Seiscomp::Gui::RecordWidget*, Seiscomp::Core::Time)),
	        this, SLOT(onSelectedTime(Seiscomp::Gui::RecordWidget*, Seiscomp::Core::Time)));

	connect(_currentRecord, SIGNAL(selectedTime(Seiscomp::Core::Time)),
	        this, SLOT(onSelectedTime(Seiscomp::Core::Time)));

	connect(_currentRecord, SIGNAL(selectedTimeRangeChanged(Seiscomp::Core::Time, Seiscomp::Core::Time)),
	        this, SLOT(onChangingTimeRange(Seiscomp::Core::Time, Seiscomp::Core::Time)));
	connect(_currentRecord, SIGNAL(selectedTimeRange(Seiscomp::Core::Time, Seiscomp::Core::Time)),
	        this, SLOT(onSelectedTimeRange(Seiscomp::Core::Time, Seiscomp::Core::Time)));

	connect(_recordView, SIGNAL(addedItem(const Seiscomp::Record*, Seiscomp::Gui::RecordViewItem*)),
	        this, SLOT(onAddedItem(const Seiscomp::Record*, Seiscomp::Gui::RecordViewItem*)));

	connect(&RecordStreamState::Instance(), SIGNAL(firstConnectionEstablished()),
	        this, SLOT(firstConnectionEstablished()));
	connect(&RecordStreamState::Instance(), SIGNAL(lastConnectionClosed()),
	        this, SLOT(lastConnectionClosed()));

	if ( RecordStreamState::Instance().connectionCount() )
		firstConnectionEstablished();
	else
		lastConnectionClosed();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeView::setConfig(const Config &c, QString *error) {
	_config = c;

	if ( SCScheme.unit.distanceInKM )
		_spinDistance->setValue(Math::Geo::deg2km(_config.defaultAddStationsDistance));
	else
		_spinDistance->setValue(_config.defaultAddStationsDistance);

	//_config.filters.append(Config::FilterEntry("4 pole HP @2s", "BW_HP(4,0.5)"));

	if ( _comboFilter ) {
		_comboFilter->blockSignals(true);
		_comboFilter->clear();
		_comboFilter->addItem(NO_FILTER_STRING);
		_comboFilter->addItem(DEFAULT_FILTER_STRING);

		_lastFilterIndex = -1;

		int defaultIndex = -1;
		for ( int i = 0; i < _config.filters.count(); ++i ) {
			if ( _config.filters[i].first.isEmpty() ) continue;

			if ( _config.filters[i].first[0] == '@' ) {
				if ( defaultIndex == -1 ) defaultIndex = _comboFilter->count();
				addFilter(_config.filters[i].first.mid(1), _config.filters[i].second);
			}
			else
				addFilter(_config.filters[i].first, _config.filters[i].second);
		}

		_comboFilter->blockSignals(false);

		_comboFilter->setCurrentIndex(defaultIndex != -1?defaultIndex:1);
	}

	RecordViewItem *item = _recordView->currentItem();
	if ( item && _currentRecord ) {
		if ( item->value(ITEM_DISTANCE_INDEX) >= 0 ) {
			if ( _config.showAllComponents &&
				_config.allComponentsMaximumStationDistance >= item->value(ITEM_DISTANCE_INDEX) )
				_currentRecord->setDrawMode(RecordWidget::InRows);
			else
				_currentRecord->setDrawMode(RecordWidget::Single);
		}
		else
			_currentRecord->setDrawMode(RecordWidget::Single);
	}

	if ( _config.hideStationsWithoutData ) {
		bool reselectCurrentItem = false;

		for ( int r = 0; r < _recordView->rowCount(); ++r ) {
			RecordViewItem* item = _recordView->itemAt(r);
			AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
			if ( isLinkedItem(item) ) continue;

			if ( !isTracePicked(item->widget()) && !label->hasGotData ) {
				item->forceInvisibilty(true);
				if ( item == _recordView->currentItem() )
					reselectCurrentItem = true;
			}
		}

		if ( _recordView->currentItem() == NULL ) reselectCurrentItem = true;

		if ( reselectCurrentItem )
			selectFirstVisibleItem(_recordView);
	}
	else {
		for ( int r = 0; r < _recordView->rowCount(); ++r ) {
			RecordViewItem* item = _recordView->itemAt(r);
			AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
			if ( isLinkedItem(item) ) continue;

			if ( !isTracePicked(item->widget()) && !label->hasGotData )
				item->forceInvisibilty(!label->isEnabledByConfig);
		}
	}

	acquireStreams();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setDatabase(Seiscomp::DataModel::DatabaseQuery* reader) {
	_reader = reader;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setStrongMotionCodes(const std::vector<std::string> &codes) {
	_strongMotionCodes = codes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showEvent(QShowEvent *e) {
	// avoid truncated distance labels
	int w1 = _ui.frameCurrentTraceLabel->width();
	int w2 = 0;
	QFont f(_ui.labelDistance->font()); // hack to get default font size
	QFontMetrics fm(f);
	w2 += fm.boundingRect("WW ").width();
	f.setBold(true);
	w2 += fm.boundingRect("WWWWW 100").width();

	if ( SCScheme.unit.distanceInKM )
		w2 = std::max(w2, fm.boundingRect(QString("%1 km").arg(299999.0/3.0, 0, 'f', SCScheme.precision.distance)).width());
	else
		w2 = std::max(w2, fm.boundingRect(QString("155.5%1").arg(degrees)).width());

	if (w2>w1)
		_ui.frameCurrentTraceLabel->setFixedWidth(w2);

	if ( !_settingsRestored ) {
		QList<int> sizes;

		if ( SCApp ) {
			SCApp->settings().beginGroup(objectName());
			restoreGeometry(SCApp->settings().value("geometry").toByteArray());
			restoreState(SCApp->settings().value("state").toByteArray());

#ifdef MACOSX
			Mac::addFullscreen(this);
#endif

			QVariant splitterUpperSize = SCApp->settings().value("splitter/upper");
			QVariant splitterLowerSize = SCApp->settings().value("splitter/lower");

			if ( !splitterUpperSize.isValid() || !splitterLowerSize.isValid() ) {
				sizes.append(200);
				sizes.append(400);
			}
			else {
				sizes.append(splitterUpperSize.toInt());
				sizes.append(splitterLowerSize.toInt());
			}

			SCApp->settings().endGroup();
		}
		else {
			sizes.append(200);
			sizes.append(400);
		}

		_ui.splitter->setSizes(sizes);

		_settingsRestored = true;
	}

	_recordView->setLabelWidth(_ui.frameCurrentTraceLabel->width() +
	                           _ui.frameCurrentTrace->frameWidth() +
	                           _ui.frameCurrentTrace->layout()->margin());

	QWidget::showEvent(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::onSelectedTime(Seiscomp::Core::Time time) {
	//setPhaseMarker(_currentRecord, time);
	if ( _recordView->currentItem() ) {
		setPhaseMarker(_recordView->currentItem()->widget(), time);
		_currentRecord->update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::onChangingTimeRange(Seiscomp::Core::Time t1, Seiscomp::Core::Time t2) {
	static_cast<MyRecordWidget*>(_currentRecord)->setSelected(t1, t2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::onSelectedTimeRange(Seiscomp::Core::Time t1, Seiscomp::Core::Time t2) {
	static_cast<MyRecordWidget*>(_currentRecord)->setSelected(Core::Time(), Core::Time());

	RecordViewItem *item = _recordView->currentItem();
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());

	if ( t1 == t2 ) return;

	if ( _currentSlot < 0 ) return;
	if ( label->processor == NULL ) return;

	double smin = t1-label->processor->trigger();
	double smax = t2-label->processor->trigger();

	label->processor->setMinSNR(_spinSNR->value());

	if ( _comboAmpType->isEnabled() )
		label->processor->setParameter(Processing::AmplitudeProcessor::MeasureType, _comboAmpType->currentText().toStdString());
	if ( _comboAmpCombiner->isEnabled() )
		label->processor->setParameter(Processing::AmplitudeProcessor::Combiner, _comboAmpCombiner->currentText().toStdString());

	label->processor->setPublishFunction(boost::bind(&AmplitudeView::newAmplitudeAvailable, this, _1, _2));
	label->processor->reprocess(smin, smax);
	label->processor->setPublishFunction(Processing::AmplitudeProcessor::PublishFunc());
	label->updateProcessingInfo();

	QString statusText = label->processor->status().toString();

	switch ( label->processor->status() ) {
		case Processing::WaveformProcessor::WaitingForData:
		case Processing::WaveformProcessor::Finished:
		case Processing::WaveformProcessor::Terminated:
			break;
		case Processing::WaveformProcessor::InProgress:
			statusText += QString(": %1%")
			              .arg(label->processor->statusValue(),0,'f',1);
			break;
		case Processing::WaveformProcessor::LowSNR:
			statusText += QString(": %1 < %2")
			              .arg(label->processor->statusValue(),0,'f',1)
			              .arg(label->processor->config().snrMin,0,'f',1);
			break;
		default:
			statusText += QString("(%1)")
			              .arg(label->processor->statusValue(),0,'f',1);
			break;
	}

	statusBar()->showMessage(statusText);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::onSelectedTime(Seiscomp::Gui::RecordWidget* widget,
                                   Seiscomp::Core::Time time) {
	if ( widget == _currentRecord ) return;
	setPhaseMarker(widget, time);
	//setPhaseMarker(_currentRecord, time);
	_currentRecord->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setPhaseMarker(Seiscomp::Gui::RecordWidget* widget,
                                   const Seiscomp::Core::Time& time) {
	if ( widget != _recordView->currentItem()->widget() ) return;
	if ( widget->cursorText().isEmpty() ) return;

	RecordViewItem *item = _recordView->currentItem();
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());

	if ( _currentSlot < 0 ) return;
	if ( label->processor == NULL ) return;

	double smin = double(time-label->processor->trigger())-0.5;
	double smax = smin+1.0;

	label->processor->setMinSNR(_spinSNR->value());

	if ( _comboAmpType->isEnabled() )
		label->processor->setParameter(Processing::AmplitudeProcessor::MeasureType, _comboAmpType->currentText().toStdString());
	if ( _comboAmpCombiner->isEnabled() )
		label->processor->setParameter(Processing::AmplitudeProcessor::Combiner, _comboAmpCombiner->currentText().toStdString());

	label->processor->setPublishFunction(boost::bind(&AmplitudeView::newAmplitudeAvailable, this, _1, _2));
	label->processor->reprocess(smin, smax);
	label->processor->setPublishFunction(Processing::AmplitudeProcessor::PublishFunc());
	label->updateProcessingInfo();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordMarker* AmplitudeView::updatePhaseMarker(Seiscomp::Gui::RecordViewItem *item,
                                               const Processing::AmplitudeProcessor::Result &res,
                                               const QString &text) {
	RecordWidget *widget = item->widget();
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());

	OPT(double) mag;
	QString magError;

	if ( label->magnitudeProcessor ) {
		double m;
		Processing::MagnitudeProcessor::Status status;
		status = label->magnitudeProcessor->computeMagnitude(
		         res.amplitude.value, res.period, item->value(ITEM_DISTANCE_INDEX),
		         _origin->depth(), m);
		if ( status == Processing::MagnitudeProcessor::OK )
			mag = m;
		else
			magError = status.toString();
	}

	int slot = -1;
	if ( res.component < Processing::WaveformProcessor::Horizontal )
		slot = _componentMap[res.component];

	AmplitudeViewMarker *marker = (AmplitudeViewMarker*)widget->marker(text, true);
	// Marker found?
	if ( marker ) {
		// Set the marker time to the new picked time
		marker->setCorrectedTime(res.time.reference);
		// and set its component to the currently displayed component
		marker->setSlot(slot);
		//marker->setWidth(res.amplitudeLeftWidth, res.amplitudeRightWidth);
		marker->setTimeWindow(res.time.begin, res.time.end);
		marker->setMagnitude(mag, magError);
		marker->setAmplitudeResult(res);
		marker->setFilterID(label->data.filterID);

		widget->update();
	}
	else {
		// Valid phase code?
		if ( !text.isEmpty() ) {

			// Create a new marker for the phase
			marker = new AmplitudeViewMarker(widget, res.time.reference, text,
			                          AmplitudeViewMarker::Amplitude, true);
			marker->setSlot(slot);
			//marker->setWidth(res.amplitudeLeftWidth, res.amplitudeRightWidth);
			marker->setTimeWindow(res.time.begin, res.time.end);
			marker->setMagnitude(mag, magError);
			marker->setAmplitudeResult(res);
			marker->setFilterID(label->data.filterID);
			marker->setEnabled(true);
	
			for ( int i = 0; i < widget->markerCount(); ++i ) {
				RecordMarker* marker2 = widget->marker(i);
				if ( marker == marker2 ) continue;
					if ( marker2->text() == marker->text() && !marker2->isMovable() )
						marker2->setEnabled(false);
			}
		}

		widget->update();
	}

	return marker;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::onAddedItem(const Record* rec, RecordViewItem* item) {
	// NOTE: Dynamic item insertion is not yet used
	/*
	setupItem(item);
	addTheoreticalArrivals(item, rec->networkCode(), rec->stationCode());
	sortByDistance();
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setCursorText(const QString& text) {
	_recordView->setCursorText(text);
	_currentRecord->setCursorText(text);
	_currentRecord->setActive(text != "");

	if ( _currentRecord->isActive() ) {
		//_centerSelection = true;
		RecordMarker* m = _currentRecord->marker(text);
		if ( m )
			setCursorPos(m->correctedTime());
		else if ( _recordView->currentItem() )
			setCursorPos(_recordView->currentItem()->widget()->visibleTimeWindow().startTime() +
			             Core::TimeSpan(_recordView->currentItem()->widget()->visibleTimeWindow().length()*0.5));
	}

	updateCurrentRowState();
	componentByState();

	return;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::loadNextStations() {
	float distance = _spinDistance->value();

	if ( SCScheme.unit.distanceInKM )
		distance = Math::Geo::km2deg(distance);

	std::vector<Seiscomp::DataModel::WaveformStreamID>::iterator it;

	_recordView->setUpdatesEnabled(false);

	loadNextStations(distance);

	sortByState();
	alignByState();
	componentByState();

	// Load all required components
	for ( int i = 0; i < 3; ++i )
		if ( _componentMap[i] >= 0 )
			fetchComponent(COMPS[i]);

	if ( _recordView->currentItem() == NULL ) {
		selectFirstVisibleItem(_recordView);
	}
	setCursorText(_currentRecord->cursorText());

	_recordView->setUpdatesEnabled(true);
	_recordView->setFocus();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::sortByState() {
	if ( _ui.actionSortByDistance->isChecked() )
		sortByDistance();
	else if ( _ui.actionSortAlphabetically->isChecked() )
		sortAlphabetically();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::alignByState() {
	if ( _ui.actionAlignOnPArrival->isChecked() )
		alignOnPArrivals();
	else if ( _ui.actionAlignOnOriginTime->isChecked() )
		alignOnOriginTime();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::componentByState() {
	if ( _ui.actionShowZComponent->isChecked() )
		showComponent('Z');
	else if ( _ui.actionShowNComponent->isChecked() )
		showComponent('1');
	else if ( _ui.actionShowEComponent->isChecked() )
		showComponent('2');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::resetState() {
	for ( int i = 0; i < 3; ++i )
		if ( _componentMap[i] >= 0 )
			showComponent(COMPS[i]);
	//showComponent('Z');
	//alignOnOriginTime();
	alignOnPArrivals();
	pickNone(true);
	sortByDistance();
	_ui.actionShowUsedStations->setChecked(false);
	showUsedStations(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateLayoutFromState() {
	sortByState();
	alignByState();
	componentByState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::firstConnectionEstablished() {
	_connectionState->start();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::lastConnectionClosed() {
	_connectionState->stop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::beginWaitForRecords() {
	qApp->setOverrideCursor(Qt::WaitCursor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::doWaitForRecords(int value) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::endWaitForRecords() {
	qApp->restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showFullscreen(bool e) {
	if ( e )
		showFullScreen();
	else {
		showNormal();
#ifdef MACOSX
		Mac::addFullscreen(this);
#endif
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::recalculateAmplitude() {
	RecordViewItem *item = _recordView->currentItem();
	if ( item == NULL ) return;

	AmplitudeRecordLabel *l = static_cast<AmplitudeRecordLabel*>(item->label());
	if ( !l->processor ) return;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	l->processor->setMinSNR(_spinSNR->value());
	resetAmplitude(item, _amplitudeType.c_str(), false);

	if ( _comboAmpType->isEnabled() )
		l->processor->setParameter(Processing::AmplitudeProcessor::MeasureType, _comboAmpType->currentText().toStdString());
	if ( _comboAmpCombiner->isEnabled() )
		l->processor->setParameter(Processing::AmplitudeProcessor::Combiner, _comboAmpCombiner->currentText().toStdString());

	if ( l->processor->isFinished() ) {
		l->processor->setPublishFunction(boost::bind(&AmplitudeView::newAmplitudeAvailable, this, _1, _2));
		l->processor->reprocess();

		for ( int i = 0; i < 3; ++i ) {
			const Processing::AmplitudeProcessor *compProc = l->processor->componentProcessor((Processing::WaveformProcessor::Component)i);
			if ( compProc == NULL ) continue;

			const DoubleArray *processedData = compProc->processedData((Processing::WaveformProcessor::Component)i);

			if ( processedData ) {
				l->data.setProcessedData(
					i, item->streamID().networkCode(),
					item->streamID().stationCode(),
					item->streamID().locationCode(),
					compProc->dataTimeWindow().startTime(),
					compProc->samplingFrequency(),
					FloatArray::Cast(processedData->copy(Array::FLOAT))
				);

				//l->processor->writeData();
			}
			else
				l->data.removeProcessedData(i);
		}

		l->processor->setPublishFunction(Processing::AmplitudeProcessor::PublishFunc());
	}

	l->updateProcessingInfo();
	//l->setEnabled(!l->isError);

	_currentRecord->update();
	QApplication::restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::recalculateAmplitudes() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem *item = _recordView->itemAt(r);
		AmplitudeRecordLabel *l = static_cast<AmplitudeRecordLabel*>(item->label());
		if ( !l->processor ) continue;

		// NOTE: Ignore items that are children of other items and not
		//       expanded (eg SM channels)
		if ( l->isLinkedItem() ) {
			AmplitudeRecordLabel *controllerLabel = static_cast<AmplitudeRecordLabel*>(l->controlledItem()->label());
			if ( !controllerLabel->isExpanded() ) continue;
		}

		l->processor->setMinSNR(_spinSNR->value());
		resetAmplitude(item, _amplitudeType.c_str(), false);

		if ( _comboAmpType->isEnabled() )
			l->processor->setParameter(Processing::AmplitudeProcessor::MeasureType, _comboAmpType->currentText().toStdString());
		if ( _comboAmpCombiner->isEnabled() )
			l->processor->setParameter(Processing::AmplitudeProcessor::Combiner, _comboAmpCombiner->currentText().toStdString());

		if ( l->processor->isFinished() ) {
			l->processor->setPublishFunction(boost::bind(&AmplitudeView::newAmplitudeAvailable, this, _1, _2));
			l->processor->reprocess();

			for ( int i = 0; i < 3; ++i ) {
				const Processing::AmplitudeProcessor *compProc = l->processor->componentProcessor((Processing::WaveformProcessor::Component)i);
				if ( compProc == NULL ) continue;

				const DoubleArray *processedData = compProc->processedData((Processing::WaveformProcessor::Component)i);

				if ( processedData ) {
					l->data.setProcessedData(
						i, item->streamID().networkCode(),
						item->streamID().stationCode(),
						item->streamID().locationCode(),
						compProc->dataTimeWindow().startTime(),
						compProc->samplingFrequency(),
						FloatArray::Cast(processedData->copy(Array::FLOAT))
					);

					//l->processor->writeData();
				}
				else
					l->data.removeProcessedData(i);
			}

			l->processor->setPublishFunction(Processing::AmplitudeProcessor::PublishFunc());
		}

		l->updateProcessingInfo();
		//l->setEnabled(!l->isError);
	}

	_currentRecord->update();
	QApplication::restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::newAmplitudeAvailable(const Processing::AmplitudeProcessor *proc,
                                          const Processing::AmplitudeProcessor::Result &res) {
	std::string streamID = res.record->streamID();
	RecordItemMap::iterator it = _recordItemLabels.find(streamID);
	if ( it == _recordItemLabels.end() )
		return;

	AmplitudeRecordLabel *label = it->second;
	RecordViewItem *item = label->recordViewItem();

	if ( label->processor.get() != proc ) return;
	if ( proc->type() != _amplitudeType ) return;

	updatePhaseMarker(item, res, proc->type().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::fetchComponent(char componentCode) {
	for ( WaveformStreamList::iterator it = _allStreams.begin();
	      it != _allStreams.end(); ) {
		char queuedComponent = COMPS[it->component];
		if ( queuedComponent == componentCode || queuedComponent == '?' ) {
			// Cut the needed timewindow
			RecordViewItem* item = _recordView->item(it->streamID);
			if ( item ) {
				AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
				it->timeWindow = label->timeWindow;
			}

			_nextStreams.push_back(*it);
			it = _allStreams.erase(it);
		}
		else
			++it;
	}

	acquireStreams();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showComponent(char componentCode) {
	int newSlot;

	switch ( componentCode ) {
		default:
		case 'Z':
			newSlot = 0;
			break;
		case '1':
			newSlot = 1;
			break;
		case '2':
			newSlot = 2;
			break;
	}

	if ( _componentMap[newSlot] >= 0 ) {
		fetchComponent(componentCode);
		_currentSlot = newSlot;
	}

	_recordView->showSlot(_componentMap[_currentSlot]);
	_ui.actionShowZComponent->setChecked(_currentSlot == 0);
	_ui.actionShowNComponent->setChecked(_currentSlot == 1);
	_ui.actionShowEComponent->setChecked(_currentSlot == 2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showUsedStations(bool usedOnly) {
	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem* item = _recordView->itemAt(r);
		if ( !isLinkedItem(item) ) {
			if ( usedOnly )
				item->setVisible(isTraceUsed(item->widget()));
			else
				item->setVisible(true);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::loadNextStations(float distance) {
	DataModel::Inventory* inv = Client::Inventory::Instance()->inventory();

	if ( inv != NULL ) {

		for ( size_t i = 0; i < inv->networkCount(); ++i ) {
			Network* n = inv->network(i);
			for ( size_t j = 0; j < n->stationCount(); ++j ) {
				Station* s = n->station(j);
	
				QString code = (n->code() + "." + s->code()).c_str();
	
				if ( _stations.contains(code) ) continue;
	
				try {
					if ( s->end() <= _origin->time() )
						continue;
				}
				catch ( Core::ValueException& ) {}
	
				double lat = s->latitude();
				double lon = s->longitude();
				double delta, az1, az2;
	
				Geo::delazi(_origin->latitude(), _origin->longitude(),
				            lat, lon, &delta, &az1, &az2);
	
				if ( delta > distance ) continue;

				// Skip stations out of amplitude processors range
				if ( delta < _minDist || delta > _maxDist ) continue;
	
				// try to get the configured location and stream code
				Stream *stream = findConfiguredStream(s, _origin->time());

				// Try to get a default stream
				if ( stream == NULL ) {
					// Preferred channel code is BH. If not available use either SH or skip.
					for ( size_t c = 0; c < _broadBandCodes.size(); ++c ) {
						stream = findStream(s, _broadBandCodes[c], _origin->time());
						if ( stream ) break;
					}
				}

				if ( stream == NULL )
					stream = findStream(s, _origin->time(), Processing::WaveformProcessor::MeterPerSecond);
	
				if ( stream ) {
					WaveformStreamID streamID(n->code(), s->code(), stream->sensorLocation()->code(), stream->code().substr(0,stream->code().size()-1) + '?', "");

					try {
						TravelTime ttime =
							_ttTable.computeFirst(_origin->latitude(), _origin->longitude(),
							                      _origin->depth(), lat, lon);

						Core::Time referenceTime = _origin->time().value() + Core::TimeSpan(ttime.time);

						RecordViewItem* item = addStream(stream->sensorLocation(), streamID, referenceTime, false);
						if ( item ) {
							_stations.insert(code);
							item->setVisible(!_ui.actionShowUsedStations->isChecked());
							if ( _config.hideStationsWithoutData )
								item->forceInvisibilty(true);
						}
					}
					catch ( ... ) {}
				}
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::addAmplitude(Gui::RecordViewItem *item,
                                 DataModel::Amplitude *amp,
                                 DataModel::Pick *pick, Core::Time reference,
                                 int id) {
	RecordWidget *widget = item->widget();
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());

	// Store referencing pickID
	if ( label->processor && pick )
		label->processor->setReferencingPickID(pick->publicID());

	if ( amp ) {
		AmplitudeViewMarker *marker;
		marker = new AmplitudeViewMarker(widget, amp->timeWindow().reference(), AmplitudeViewMarker::Amplitude, false);
		marker->setAmplitude(amp);
		marker->setText(_magnitudeType.c_str());
		marker->setId(id);

		if ( amp->waveformID().channelCode().size() > 2 )
			marker->setSlot(item->mapComponentToSlot(*amp->waveformID().channelCode().rbegin()));

		if ( label->magnitudeProcessor ) {
			double m, per = -1;

			try { per = amp->period().value(); }
			catch ( ... ) {}

			Processing::MagnitudeProcessor::Status stat;
			stat = label->magnitudeProcessor->computeMagnitude(
			         amp->amplitude().value(), per, item->value(ITEM_DISTANCE_INDEX),
			         _origin->depth(), m);
			if ( stat == Processing::MagnitudeProcessor::OK )
				marker->setMagnitude(m, QString());
			else
				marker->setMagnitude(Core::None, stat.toString());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeView::setOrigin(Seiscomp::DataModel::Origin* origin,
                              const std::string &magType) {
	if ( (origin == _origin) && (_magnitudeType == magType) ) return true;

	SEISCOMP_DEBUG("stopping record acquisition");
	stop();

	_recordView->clear();
	_recordItemLabels.clear();

	_labelAmpType->setEnabled(false);
	_comboAmpType->clear();
	_comboAmpType->setEnabled(false);

	_labelAmpCombiner->setEnabled(false);
	_comboAmpCombiner->clear();
	_comboAmpCombiner->setEnabled(false);

	_origin = origin;
	_magnitude = NULL;

	updateOriginInformation();
	if ( _comboFilter->currentIndex() == 0 )
		_comboFilter->setCurrentIndex(1);

	if ( _origin == NULL )
		return false;

	Processing::MagnitudeProcessorPtr procMag = Processing::MagnitudeProcessorFactory::Create(magType.c_str());
	if ( !procMag ) {
		QMessageBox::critical(this, "Amplitude waveform review",
		                      QString("Failed to create magnitude processor with type %1").arg(magType.c_str()));
		return false;
	}

	_magnitudeType = magType;
	_amplitudeType = procMag->amplitudeType();
	Processing::AmplitudeProcessorPtr procAmp = Processing::AmplitudeProcessorFactory::Create(_amplitudeType.c_str());
	if ( !procAmp ) {
		QMessageBox::critical(this, "Amplitude waveform review",
		                      QString("Failed to create amplitude processor with type %1").arg(_amplitudeType.c_str()));
		return false;
	}

	if ( procAmp->supports(Processing::AmplitudeProcessor::MeasureType) ) {
		Processing::AmplitudeProcessor::IDList params = procAmp->capabilityParameters(Processing::AmplitudeProcessor::MeasureType);
		for ( size_t i = 0; i < params.size(); ++i )
			_comboAmpType->addItem(params[i].c_str());
	}
	_comboAmpType->setEnabled(_comboAmpType->count() > 0);
	_labelAmpType->setEnabled(_comboAmpType->isEnabled());

	if ( procAmp->supports(Processing::AmplitudeProcessor::Combiner) ) {
		_comboAmpCombiner->setEnabled(true);
		Processing::AmplitudeProcessor::IDList params = procAmp->capabilityParameters(Processing::AmplitudeProcessor::Combiner);
		for ( size_t i = 0; i < params.size(); ++i )
			_comboAmpCombiner->addItem(params[i].c_str());
	}
	_comboAmpCombiner->setEnabled(_comboAmpCombiner->count() > 0);
	_labelAmpCombiner->setEnabled(_comboAmpCombiner->isEnabled());

	// Default map from component to trace slot in RecordWidget
	_componentMap[0] = -1;
	_componentMap[1] = -1;
	_componentMap[2] = -1;
	_slotCount = 0;

	Processing::WaveformProcessor::StreamComponent c = procAmp->usedComponent();
	switch ( c ) {
		case Processing::WaveformProcessor::Vertical:
			_componentMap[0] = 0;
			_slotCount = 1;
			break;
		case Processing::WaveformProcessor::FirstHorizontal:
			_componentMap[1] = 0;
			_slotCount = 1;
			break;
		case Processing::WaveformProcessor::SecondHorizontal:
			_componentMap[2] = 0;
			_slotCount = 1;
			break;
		case Processing::WaveformProcessor::Horizontal:
			_componentMap[1] = 0;
			_componentMap[2] = 1;
			_slotCount = 2;
			break;
		case Processing::WaveformProcessor::Any:
			_componentMap[0] = 0;
			_componentMap[1] = 1;
			_componentMap[2] = 2;
			_slotCount = 3;
			break;
		default:
			return false;
	}

	_spinSNR->setValue(procAmp->config().snrMin);

	_minDist = procAmp->config().minimumDistance;
	_maxDist = procAmp->config().maximumDistance;

	if ( SCScheme.unit.distanceInKM ) {
		_spinDistance->setMinimum(Math::Geo::deg2km(_minDist));
		_spinDistance->setMaximum(Math::Geo::deg2km(_maxDist));
	}
	else {
		_spinDistance->setMinimum(_minDist);
		_spinDistance->setMaximum(_maxDist);
	}

	setUpdatesEnabled(false);

	_stations.clear();

	Core::Time originTime = _origin->time();
	if ( !originTime ) originTime = Core::Time::GMT();	

	// Default is 1h travel time for 180deg
	double maxTravelTime = 3600.0;

	try {
		TravelTime tt = _ttTable.computeFirst(0.0,0.0, _origin->depth(), 180.0,0.0);
		maxTravelTime = tt.time;
	}
	catch ( ... ) {}

	SEISCOMP_DEBUG("MaxTravelTime = %.2f", maxTravelTime);

	_minTime = procAmp->config().noiseBegin;
	_maxTime = maxTravelTime+procAmp->config().signalEnd;
	Core::TimeWindow timeWindow(originTime+Core::TimeSpan(_minTime),
	                            originTime+Core::TimeSpan(_maxTime));
	_recordView->setTimeWindow(timeWindow);
	_recordView->setTimeRange(_minTime, _maxTime);

	for ( size_t i = 0; i < _origin->magnitudeCount(); ++i ) {
		if ( _origin->magnitude(i)->type() == magType ) {
			_magnitude = _origin->magnitude(i);
			break;
		}
	}

	map<string, StationItem> items;

	if ( _magnitude ) {
		for ( size_t i = 0; i < _magnitude->stationMagnitudeContributionCount(); ++i ) {
			StationMagnitudeContribution *staMagRef = _magnitude->stationMagnitudeContribution(i);
			StationMagnitude *staMag = StationMagnitude::Find(staMagRef->stationMagnitudeID());
			if ( !staMag ) continue;
			StationItem item;

			item.amp = Amplitude::Find(staMag->amplitudeID());
			if ( !item.amp && _reader )
				item.amp = Amplitude::Cast(_reader->getObject(Amplitude::TypeInfo(), staMag->amplitudeID()));

			if ( !item.amp ) continue;

			item.pick = Pick::Cast(PublicObject::Find(item.amp->pickID()));
			item.isTrigger = true;

			if ( !item.amp->pickID().empty() && !item.pick && _reader )
				item.pick = Pick::Cast(_reader->getObject(Pick::TypeInfo(), item.amp->pickID()));

			string itemID = item.amp->waveformID().networkCode() + "." + item.amp->waveformID().stationCode();
			items[itemID] = item;
		}
	}

	for ( size_t i = 0; i < _origin->arrivalCount(); ++i ) {
		Arrival* arrival = origin->arrival(i);

		StationItem item;

		item.pick = Pick::Cast(PublicObject::Find(arrival->pickID()));
		if ( !item.pick ) {
			//std::cout << "pick not found" << std::endl;
			continue;
		}

		item.isTrigger = getShortPhaseName(arrival->phase().code()) == 'P';

		string itemID = item.pick->waveformID().networkCode() + "." + item.pick->waveformID().stationCode();
		map<string, StationItem>::iterator it = items.find(itemID);

		if ( it != items.end() ) {
			// Entry with amplitude already registered?
			if ( it->second.amp ) continue;
			if ( !it->second.pick ) continue;

			// If this pick is earlier than the already registered one
			// use it
			if ( it->second.pick->time().value() > item.pick->time().value() ) {
				it->second.pick = item.pick;
				it->second.isTrigger = item.isTrigger;
			}
		}
		else {
			items[itemID] = item;
		}
	}

	for ( map<string, StationItem>::iterator it = items.begin();
	      it != items.end(); ++it ) {
		WaveformStreamID streamID;
		Core::Time reference;

		if ( it->second.amp )
			streamID = adjustWaveformStreamID(it->second.amp->waveformID());
		else if ( it->second.pick )
			streamID = adjustWaveformStreamID(it->second.pick->waveformID());
		else
			continue;

		SensorLocation *loc = NULL;

		Station *sta = Client::Inventory::Instance()->getStation(
		               streamID.networkCode(), streamID.stationCode(), _origin->time());

		if ( sta )
			loc = findSensorLocation(sta, streamID.locationCode(), _origin->time());
		if ( loc == NULL ) {
			SEISCOMP_ERROR("skipping station %s.%s: sensor location %s not found",
			               streamID.networkCode().c_str(),
			               streamID.stationCode().c_str(),
			               streamID.locationCode().c_str());
			continue;
		}

		if ( it->second.pick && it->second.isTrigger )
			reference = it->second.pick->time().value();
		else /*if ( it->second.amp )*/ {
			try {
				TravelTime ttime =
					_ttTable.computeFirst(_origin->latitude(), _origin->longitude(),
					                      _origin->depth(), loc->latitude(), loc->longitude());

				reference = _origin->time().value() + Core::TimeSpan(ttime.time);
			}
			catch ( ... ) {
				SEISCOMP_ERROR("skipping station %s.%s: unable to compute trigger time",
				               streamID.networkCode().c_str(),
				               streamID.stationCode().c_str());
				continue;
			}
		}

		if ( it->second.amp == NULL ) {
			double delta, az, baz;
			Geo::delazi(_origin->latitude(), _origin->longitude(),
			            loc->latitude(), loc->longitude(), &delta, &az, &baz);

			if ( delta < _minDist || delta > _maxDist ) {
				SEISCOMP_INFO("skipping station %s.%s: out of range",
				              streamID.networkCode().c_str(), streamID.stationCode().c_str());
				continue;
			}
		}

		RecordViewItem *item = addStream(loc, streamID, reference, true);
		// A new item has been inserted
		if ( item != NULL ) {
			addAmplitude(item, it->second.amp.get(), it->second.pick.get(), reference, -1);
			_stations.insert((streamID.networkCode() + "." + streamID.stationCode()).c_str());
		}
		else {
			// Try to find the existing item for the stream
			item = _recordView->item(streamID);

			// If not found ignore this stream, we can't do anything else
			if ( item == NULL ) continue;

			// If the stream is a strong motion stream, we need to unlink
			// it from its broadband stream (disconnect the "expand button" feature)
			if ( isLinkedItem(item) )
				unlinkItem(item);
		}
	}

	if ( _recordView->rowCount() == 0 )
		loadNextStations();

	_recordView->setAlignment(originTime);

	_ui.actionShowZComponent->setEnabled(_componentMap[0] >= 0);
	_ui.actionShowZComponent->setVisible(_componentMap[0] >= 0);
	_ui.actionShowNComponent->setEnabled(_componentMap[1] >= 0);
	_ui.actionShowNComponent->setVisible(_componentMap[1] >= 0);
	_ui.actionShowEComponent->setEnabled(_componentMap[2] >= 0);
	_ui.actionShowEComponent->setVisible(_componentMap[2] >= 0);

	resetState();
	updateLayoutFromState();

	selectFirstVisibleItem(_recordView);

	setUpdatesEnabled(true);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateOriginInformation() {
	QString title;

	if ( _origin ) {
		QString depth;
		try {
			depth = QString("%1").arg((int)_origin->depth());
		}
		catch ( Core::ValueException& ) {
			depth = "NULL";
		}

		title = QString("ID: %1, Lat/Lon: %2 | %3, Depth: %4 km")
		                 .arg(_origin->publicID().c_str())
		                 .arg(_origin->latitude(), 0, 'f', 2)
		                 .arg(_origin->longitude(), 0, 'f', 2)
		                 .arg(depth);
	}
	else {
		title = "";
	}

	setWindowTitle(title);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setStationEnabled(const std::string& networkCode,
                                      const std::string& stationCode,
                                      bool state) {
	QList<RecordViewItem*> streams = _recordView->stationStreams(networkCode, stationCode);
	foreach ( RecordViewItem* item, streams ) {
		AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());

		label->isEnabledByConfig = state;

		// Force state to false if item has no data yet and should be hidden
		if ( _config.hideStationsWithoutData && !label->hasGotData && !isTracePicked(item->widget()) )
			state = false;

		item->forceInvisibilty(!state);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setCurrentStation(const std::string& networkCode,
                                      const std::string& stationCode) {
	QList<RecordViewItem*> streams = _recordView->stationStreams(networkCode, stationCode);
	if ( streams.isEmpty() ) return;
	_recordView->setCurrentItem(streams.front());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::queueStream(const DataModel::WaveformStreamID& streamID,
                                int component) {
	_allStreams.push_back(WaveformRequest(Core::TimeWindow(), streamID, component));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem* AmplitudeView::addStream(const DataModel::SensorLocation *sloc,
                                         const WaveformStreamID& streamID,
                                         const Core::Time &referenceTime,
                                         bool showDisabled) {
	bool isEnabled = true;
	if ( !showDisabled ) {
		isEnabled = SCApp->isStationEnabled(streamID.networkCode(), streamID.stationCode());
		if ( !isEnabled )
			return NULL;
	}

	// HACK: Add strong motion
	WaveformStreamID smStreamID(streamID);
	SensorLocation *smsloc = NULL;
	bool hasStrongMotion = false;

	if ( _config.loadStrongMotionData ) {

		Station *sta = Client::Inventory::Instance()->getStation(
			streamID.networkCode(),
			streamID.stationCode(),
			_origin->time());

		if ( sta ) {
			// Find the stream with given code priorities
			Stream *stream = NULL;
			for ( size_t c = 0; c < _strongMotionCodes.size(); ++c ) {
				stream = findStream(sta, _strongMotionCodes[c], _origin->time());
				if ( stream ) break;
			}

			if ( stream == NULL )
				stream = findStream(sta, _origin->time(), Processing::WaveformProcessor::MeterPerSecondSquared);

			if ( stream ) {
				smsloc = stream->sensorLocation();
				smStreamID.setLocationCode(smsloc->code());

				smStreamID.setChannelCode(stream->code());
				smStreamID = adjustWaveformStreamID(smStreamID);

				hasStrongMotion = true;
			}
		}

	}

	RecordViewItem *item = addRawStream(sloc, streamID, referenceTime);
	if ( item == NULL ) return NULL;

	item->setValue(ITEM_PRIORITY_INDEX, 0);
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
	label->isEnabledByConfig = isEnabled;

	item->forceInvisibilty(!label->isEnabledByConfig);

	if ( hasStrongMotion ) {
		// Try to find a corresponding StrongMotion stream and add
		// it to the view
		RecordViewItem *sm_item = addRawStream(smsloc, smStreamID, referenceTime);
		if ( sm_item ) {
			label = static_cast<AmplitudeRecordLabel*>(sm_item->label());
			label->setLinkedItem(true);
			label->isEnabledByConfig = isEnabled;
			label->hasGotData = false;
			sm_item->setValue(ITEM_PRIORITY_INDEX, 1);
			sm_item->forceInvisibilty(!label->isEnabledByConfig);
			sm_item->setVisible(false);

			// Start showing the expandable button when the first record arrives
			connect(sm_item, SIGNAL(firstRecordAdded(const Seiscomp::Record*)),
			        static_cast<AmplitudeRecordLabel*>(item->label()), SLOT(enableExpandable(const Seiscomp::Record*)));

			static_cast<AmplitudeRecordLabel*>(item->label())->setControlledItem(sm_item);

			sm_item->label()->setBackgroundColor(QColor(192,192,255));
		}
	}

	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::openConnectionInfo(const QPoint &p) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem* AmplitudeView::addRawStream(const DataModel::SensorLocation *loc,
                                            const WaveformStreamID& sid,
                                            const Core::Time &referenceTime) {
	WaveformStreamID streamID(sid);

	if ( loc == NULL ) return NULL;

	double delta, az, baz;
	Geo::delazi(_origin->latitude(), _origin->longitude(),
	            loc->latitude(), loc->longitude(), &delta, &az, &baz);

	// Skip stations out of range
	//if ( delta < _minDist || delta > _maxDist ) return NULL;

	Processing::AmplitudeProcessorPtr proc = Processing::AmplitudeProcessorFactory::Create(_amplitudeType.c_str());
	if ( proc == NULL ) {
		cerr << sid.networkCode() << "." << sid.stationCode() << ": unable to create processor "
		     << _amplitudeType << ": ignoring station" << endl;
		return NULL;
	}

	bool allComponents = true;
	ThreeComponents tc;
	char comps[3] = {COMPS[0],COMPS[1],COMPS[2]};

	// Fetch all three components
	getThreeComponents(tc, loc, streamID.channelCode().substr(0, streamID.channelCode().size()-1).c_str(), _origin->time());
	if ( tc.comps[ThreeComponents::Vertical] ) {
		comps[0] = *tc.comps[ThreeComponents::Vertical]->code().rbegin();
		Processing::Stream stream;
		stream.init(sid.networkCode(),
		            sid.stationCode(),
		            sid.locationCode(),
		            tc.comps[ThreeComponents::Vertical]->code(),
		            referenceTime);
		proc->streamConfig(Processing::WaveformProcessor::VerticalComponent) = stream;
	}
	else
		allComponents = false;

	if ( tc.comps[ThreeComponents::FirstHorizontal] ) {
		comps[1] = *tc.comps[ThreeComponents::FirstHorizontal]->code().rbegin();
		Processing::Stream stream;
		stream.init(sid.networkCode(),
		            sid.stationCode(),
		            sid.locationCode(),
		            tc.comps[ThreeComponents::FirstHorizontal]->code(),
		            referenceTime);
		proc->streamConfig(Processing::WaveformProcessor::FirstHorizontalComponent) = stream;
	}
	else
		allComponents = false;

	if ( tc.comps[ThreeComponents::SecondHorizontal] ) {
		comps[2] = *tc.comps[ThreeComponents::SecondHorizontal]->code().rbegin();
		Processing::Stream stream;
		stream.init(sid.networkCode(),
		            sid.stationCode(),
		            sid.locationCode(),
		            tc.comps[ThreeComponents::SecondHorizontal]->code(),
		            referenceTime);
		proc->streamConfig(Processing::WaveformProcessor::SecondHorizontalComponent) = stream;
	}
	else
		allComponents = false;


	Util::KeyValuesPtr keys = getParams(sid.networkCode(), sid.stationCode());

	if ( !proc->setup(
		Processing::Settings(
			SCApp->configModuleName(),
			sid.networkCode(), sid.stationCode(),
			sid.locationCode(), sid.channelCode().substr(0,2),
			&SCCoreApp->configuration(), keys.get())) ) {
		cerr << sid.networkCode() << "." << sid.stationCode() << ": setup processor failed"
		     << ": ignoring station" << endl;
		return NULL;
	}

	Processing::MagnitudeProcessorPtr magProc = Processing::MagnitudeProcessorFactory::Create(_magnitudeType.c_str());
	if ( proc == NULL ) {
		cerr << sid.networkCode() << "." << sid.stationCode() << ": unable to create magnitude processor "
		     << _magnitudeType << ": ignoring station" << endl;
		return NULL;
	}

	if ( !magProc->setup(
		Processing::Settings(
			SCApp->configModuleName(),
			sid.networkCode(), sid.stationCode(),
			sid.locationCode(), sid.channelCode().substr(0,2),
			&SCCoreApp->configuration(), keys.get())) ) {
		cerr << sid.networkCode() << "." << sid.stationCode() << ": setup magnitude processor failed"
		     << ": ignoring station" << endl;
		return NULL;
	}

	RecordViewItem* item = _recordView->addItem(adjustWaveformStreamID(streamID), sid.stationCode().c_str());
	if ( item == NULL ) return NULL;

	if ( _currentRecord )
		item->widget()->setCursorText(_currentRecord->cursorText());

	item->label()->setText(sid.stationCode().c_str(), 0);
	QFont f(item->label()->font(0));
	f.setBold(true);
	item->label()->setFont(f, 0);

	QFontMetrics fm(f);
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
	label->setWidth(fm.boundingRect("WWWW ").width(), 0);

	label->setText(QString("%1").arg(sid.networkCode().c_str()), 1);
	label->processor = proc;
	label->processor->setTrigger(referenceTime);
	label->magnitudeProcessor = magProc;

	label->latitude = loc->latitude();
	label->longitude = loc->longitude();

	label->orientationZRT.loadRotateZ(deg2rad(baz + 180.0));

	item->setValue(ITEM_DISTANCE_INDEX, delta);
	item->setValue(ITEM_AZIMUTH_INDEX, az);

	if ( SCScheme.unit.distanceInKM )
		label->setText(QString("%1 km").arg(Math::Geo::deg2km(delta),0,'f',SCScheme.precision.distance), 2);
	else
		label->setText(QString("%1%2").arg(delta,0,'f',1).arg(degrees), 2);

	label->setAlignment(Qt::AlignRight, 2);
	label->setColor(palette().color(QPalette::Disabled, QPalette::WindowText), 2);

	try {
		label->processor->setHint(Processing::WaveformProcessor::Depth, _origin->depth());
	}
	catch ( ... ) {}

	label->processor->setHint(Processing::WaveformProcessor::Distance, delta);

	label->processor->computeTimeWindow();

	label->timeWindow.set(referenceTime+Core::TimeSpan(label->processor->config().noiseBegin-_config.preOffset),
	                      referenceTime+Core::TimeSpan(label->processor->config().signalEnd+_config.postOffset));
	//label->timeWindow = label->processor->safetyTimeWindow();

	if ( !allComponents )
		SEISCOMP_WARNING("Unable to fetch all components of stream %s.%s.%s.%s",
		                 streamID.networkCode().c_str(), streamID.stationCode().c_str(),
		                 streamID.locationCode().c_str(), streamID.channelCode().substr(0,streamID.channelCode().size()-1).c_str());

	item->setData(QVariant(QString(sid.stationCode().c_str())));
	setupItem(comps, item);

	// Compute and set rotation matrix
	if ( allComponents ) {
		//cout << "[" << streamID.stationCode() << "]" << endl;
		try {
			Math::Vector3f n;
			n.fromAngles(+deg2rad(tc.comps[ThreeComponents::Vertical]->azimuth()),
			             -deg2rad(tc.comps[ThreeComponents::Vertical]->dip())).normalize();
			label->orientationZNE.setColumn(2, n);
			//cout << tc.comps[ThreeComponents::Vertical]->code() << ": dip=" << tc.comps[ThreeComponents::Vertical]->dip()
			//     << ", az=" << tc.comps[ThreeComponents::Vertical]->azimuth() << endl;

			n.fromAngles(+deg2rad(tc.comps[ThreeComponents::FirstHorizontal]->azimuth()),
			             -deg2rad(tc.comps[ThreeComponents::FirstHorizontal]->dip())).normalize();
			label->orientationZNE.setColumn(1, n);
			//cout << tc.comps[ThreeComponents::FirstHorizontal]->code() << ": dip=" << tc.comps[ThreeComponents::FirstHorizontal]->dip()
			//     << ", az=" << tc.comps[ThreeComponents::FirstHorizontal]->azimuth() << endl;

			n.fromAngles(+deg2rad(tc.comps[ThreeComponents::SecondHorizontal]->azimuth()),
			             -deg2rad(tc.comps[ThreeComponents::SecondHorizontal]->dip())).normalize();
			label->orientationZNE.setColumn(0, n);
			//cout << tc.comps[ThreeComponents::SecondHorizontal]->code() << ": dip=" << tc.comps[ThreeComponents::SecondHorizontal]->dip()
			//     << ", az=" << tc.comps[ThreeComponents::SecondHorizontal]->azimuth() << endl;
		}
		catch ( ... ) {
			SEISCOMP_WARNING("Unable to fetch orientation of stream %s.%s.%s.%s",
		                 streamID.networkCode().c_str(), streamID.stationCode().c_str(),
		                 streamID.locationCode().c_str(), streamID.channelCode().substr(0,streamID.channelCode().size()-1).c_str());
			allComponents = false;
		}
	}

	if ( !allComponents )
		// Set identity matrix
		label->orientationZNE.identity();

	for ( int i = 0; i < 3; ++i ) {
		WaveformStreamID componentID = setWaveformIDComponent(streamID, comps[i]);
		label->data.traces[i].channelCode = componentID.channelCode();
		label->data.traces[i].recordSlot = _componentMap[i];
		// Map waveformID to recordviewitem label
		_recordItemLabels[waveformIDToStdString(componentID)] = label;

		if ( label->data.traces[i].recordSlot >= 0 )
			queueStream(setWaveformIDComponent(streamID, comps[i]), i);
	}

	label->data.setRecordWidget(item->widget());
	label->updateProcessingInfo();
	label->hasGotData = false;

	applyFilter(item);

	AmplitudeViewMarker *marker =
		new AmplitudeViewMarker(item->widget(), referenceTime, AmplitudeViewMarker::Reference, false);
	marker->setText("P");

	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
char AmplitudeView::currentComponent() const {
	if ( _ui.actionShowZComponent->isChecked() )
		return 'Z';
	else if ( _ui.actionShowNComponent->isChecked() )
		return '1';
	else if ( _ui.actionShowEComponent->isChecked() )
		return '2';

	return '\0';
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setupItem(const char comps[3],
                              RecordViewItem* item) {
	connect(item->widget(), SIGNAL(cursorUpdated(RecordWidget*,int)),
	        this, SLOT(updateMainCursor(RecordWidget*,int)));

	connect(item, SIGNAL(componentChanged(RecordViewItem*, char)),
	        this, SLOT(updateItemLabel(RecordViewItem*, char)));

	connect(item, SIGNAL(firstRecordAdded(const Seiscomp::Record*)),
	        this, SLOT(updateItemRecordState(const Seiscomp::Record*)));

	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
	item->widget()->setDecorator(new TraceDecorator(item->widget(), label));
	item->widget()->setSelected(0,0);

	label->setOrientation(Qt::Horizontal);
	label->setToolTip("Timing quality: undefined");

	QPalette pal = item->widget()->palette();
	pal.setColor(QPalette::WindowText, QColor(128,128,128));
	//pal.setColor(QPalette::HighlightedText, QColor(128,128,128));
	item->widget()->setPalette(pal);

	item->widget()->setCustomBackgroundColor(SCScheme.colors.records.states.unrequested);

	item->widget()->setSlotCount(_slotCount);

	for ( int i = 0; i < 3; ++i ) {
		if ( _componentMap[i] < 0 ) continue;
		item->insertComponent(comps[i], _componentMap[i]);
	}

	Client::Inventory *inv = Client::Inventory::Instance();
	if ( inv ) {
		std::string channelCode = item->streamID().channelCode().substr(0,2);
		double gain;

		for ( int i = 0; i < 3; ++i ) {
			if ( _componentMap[i] < 0 ) continue;
			try {
				gain = inv->getGain(item->streamID().networkCode(), item->streamID().stationCode(),
				                    item->streamID().locationCode(), channelCode + comps[i],
				                    _origin->time().value());
				item->widget()->setRecordScale(_componentMap[i], 1E9 / gain);
			}
			catch ( ... ) {}
		}
	}

	item->widget()->showScaledValues(_ui.actionShowTraceValuesInNmS->isChecked());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateMainCursor(RecordWidget* w, int s) {
	char comps[3] = {'Z', '1', '2'};
	int slot = s >= 0 && s < 3?s:-1;

	if ( slot != -1 && slot != _currentSlot )
		showComponent(comps[slot]);

	setCursorPos(w->cursorPos(), true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateSubCursor(RecordWidget* w, int s) {
	char comps[3] = {'Z', '1', '2'};
	int slot = s >= 0 && s < 3?s:-1;

	if ( slot != -1 ) {
		if ( _componentMap[0] == slot ) slot = 0;
		else if ( _componentMap[1] == slot ) slot = 1;
		else if ( _componentMap[2] == slot ) slot = 2;
		else slot = -1;
	}

	if ( slot != -1 && slot != _currentSlot )
		showComponent(comps[slot]);

	_recordView->currentItem()->widget()->blockSignals(true);
	_recordView->currentItem()->widget()->setCursorPos(w->cursorPos());
	_recordView->currentItem()->widget()->blockSignals(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateRecordValue(Seiscomp::Core::Time t) {
	if ( !statusBar() ) return;

	const double *v = _currentRecord->value(t);

	if ( v == NULL )
		statusBar()->clearMessage();
	else
		statusBar()->showMessage(QString("value = %1").arg(*v, 0, 'f', 2));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showTraceScaleToggled(bool e) {
	_currentRecord->showScaledValues(e);
	for ( int i = 0; i < _recordView->rowCount(); ++i ) {
		RecordViewItem* item = _recordView->itemAt(i);
		item->widget()->showScaledValues(e);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateItemLabel(RecordViewItem* item, char component) {
	/*
	if ( item == _recordView->currentItem() )
		_currentRecord->setRecords(item->widget()->records(component), component, false);
	*/

	/*
	QString text = item->label()->text(0);
	int index = text.lastIndexOf('.');
	if ( index < 0 ) return;

	if ( text.size() - index > 2 )
		text[text.size()-1] = component;
	else
		text += component;
	item->label()->setText(text, 0);
	*/

	if ( item == _recordView->currentItem() ) {
		QString text = _ui.labelCode->text();

		int index = text.lastIndexOf(' ');
		if ( index < 0 ) return;
	
		if ( text.size() - index > 2 )
			text[text.size()-1] = component;
		else
			text += component;

		_ui.labelCode->setText(text);
	}

	updateTraceInfo(item, NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateItemRecordState(const Seiscomp::Record *rec) {
	// Reset acquisition related coloring since the first record
	// arrived already
	RecordViewItem *item = static_cast<RecordViewItem *>(sender());
	RecordWidget *widget = item->widget();
	int slot = item->mapComponentToSlot(*rec->channelCode().rbegin());
	widget->setRecordBackgroundColor(slot, SCScheme.colors.records.states.inProgress);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setCursorPos(const Seiscomp::Core::Time& t, bool always) {
	_currentRecord->setCursorPos(t);

	if ( !always && _currentRecord->cursorText() == "" ) return;

	float offset = 0;

	if ( _centerSelection ) {
		float len = _recordView->currentItem()?
			_recordView->currentItem()->widget()->width()/_currentRecord->timeScale():
			_currentRecord->tmax() - _currentRecord->tmin();

		float pos = float(t - _currentRecord->alignment()) - len/2;
		offset = pos - _currentRecord->tmin();
	}
	else {
		if ( t > _currentRecord->rightTime() )
			offset = t - _currentRecord->rightTime();
		else if ( t < _currentRecord->leftTime() )
			offset = t - _currentRecord->leftTime();
	}

	move(offset);

	_centerSelection = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setTimeRange(float tmin, float tmax) {
	float amplScale = _currentRecord->amplScale();
	_currentRecord->setTimeRange(tmin, tmax);

	if ( _autoScaleZoomTrace )
		_currentRecord->setNormalizationWindow(_currentRecord->visibleTimeWindow());

	/*
	std::cout << "ScaleWindow: " << Core::toString(_currentRecord->visibleTimeWindow().startTime()) << ", "
	          << Core::toString(_currentRecord->visibleTimeWindow().endTime()) << std::endl;
	*/

	_currentRecord->setAmplScale(amplScale);
	_timeScale->setTimeRange(tmin, tmax);

	/*
	std::cout << "[setTimeRange]" << std::endl;
	std::cout << "new TimeRange(" << _currentRecord->tmin() << ", " << _currentRecord->tmax() << ")" << std::endl;
	std::cout << "current TimeScale = " << _currentRecord->timeScale() << std::endl;
	*/

	if ( _recordView->currentItem() )
		_recordView->currentItem()->widget()->setSelected(_currentRecord->tmin(), _currentRecord->tmax());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::enableAutoScale() {
	_autoScaleZoomTrace = true;
	if ( _currentRecord ) {
		float amplScale = _currentRecord->amplScale();
		_currentRecord->setNormalizationWindow(_currentRecord->visibleTimeWindow());
		_currentRecord->setAmplScale(amplScale);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::disableAutoScale() {
	_autoScaleZoomTrace = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::zoomSelectionHandleMoved(int idx, double v, Qt::KeyboardModifiers) {
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(_recordView->currentItem()->label());
	if ( label->processor ) {
		double relTime = v - (double)(label->processor->trigger()-_timeScale->alignment());

		switch ( idx ) {
			case 0:
				label->processor->setNoiseStart(relTime);
				_recordView->timeWidget()->setSelectionHandle(0, v);
				break;
			case 1:
				label->processor->setNoiseEnd(relTime);
				label->processor->setSignalStart(relTime);
				_recordView->timeWidget()->setSelectionHandle(1, v);
				break;
			case 2:
				label->processor->setSignalEnd(relTime);
				_recordView->timeWidget()->setSelectionHandle(2, v);
				break;
			default:
				return;
		}

		_currentRecord->update();
		_recordView->currentItem()->widget()->update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::zoomSelectionHandleMoveFinished() {
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(_recordView->currentItem()->label());
	if ( label->processor ) label->processor->computeTimeWindow();

	applyFilter(_recordView->currentItem());
	_recordView->currentItem()->widget()->update();
	_currentRecord->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::selectionHandleMoved(int idx, double v, Qt::KeyboardModifiers) {
	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem* item = _recordView->itemAt(r);
		AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
		if ( label->processor ) {
			double relTime = v;

			switch ( idx ) {
				case 0:
					label->processor->setNoiseStart(relTime);
					break;
				case 1:
					label->processor->setNoiseEnd(relTime);
					label->processor->setSignalStart(relTime);
					break;
				case 2:
					label->processor->setSignalEnd(relTime);
					break;
				default:
					return;
			}

			item->widget()->update();
		}
	}

	_currentRecord->update();

	if ( _recordView->currentItem() == NULL ) return;

	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(_recordView->currentItem()->label());
	if ( label->processor ) {
		_timeScale->setSelectionEnabled(true);
		_timeScale->setSelectionHandle(0, double(label->processor->trigger()-_timeScale->alignment())+label->processor->config().noiseBegin);
		_timeScale->setSelectionHandle(1, double(label->processor->trigger()-_timeScale->alignment())+label->processor->config().signalBegin);
		_timeScale->setSelectionHandle(2, double(label->processor->trigger()-_timeScale->alignment())+label->processor->config().signalEnd);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::selectionHandleMoveFinished() {
	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem* item = _recordView->itemAt(r);
		AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
		if ( label->processor ) label->processor->computeTimeWindow();
	}

	applyFilter();
	_currentRecord->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setAlignment(Seiscomp::Core::Time t) {
	double offset = _currentRecord->alignment() - t;
	_currentRecord->setAlignment(t);

	// Because selection handle position are relative to the alignment
	// move them
	if ( _timeScale->isSelectionEnabled() ) {
		for ( int i = 0; i < _timeScale->selectionHandleCount(); ++i )
			_timeScale->setSelectionHandle(i, _timeScale->selectionHandlePos(i)+offset);
	}

	_timeScale->setAlignment(t);

	float tmin = _currentRecord->tmin()+offset;
	float tmax = _currentRecord->tmax()+offset;

	if ( _checkVisibility ) ensureVisibility(tmin, tmax);
	setTimeRange(tmin, tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::ensureVisibility(float& tmin, float& tmax) {
	if ( _recordView->currentItem() ) {
		RecordWidget* w = _recordView->currentItem()->widget();
		float leftOffset = tmin - w->tmin();
		float rightOffset = tmax - w->tmax();
		if ( leftOffset < 0 ) {
			tmin = w->tmin();
			tmax -= leftOffset;
		}
		else if ( rightOffset > 0 ) {
			float usedOffset = std::min(leftOffset, rightOffset);
			tmin -= usedOffset;
			tmax -= usedOffset;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::ensureVisibility(const Seiscomp::Core::Time &time, int pixelMargin) {
	Core::Time left = time - Core::TimeSpan(pixelMargin/_currentRecord->timeScale());
	Core::Time right = time + Core::TimeSpan(pixelMargin/_currentRecord->timeScale());

	double offset = 0;
	if ( right > _currentRecord->rightTime() )
		offset = right - _currentRecord->rightTime();
	else if ( left < _currentRecord->leftTime() )
		offset = left - _currentRecord->leftTime();

	if ( offset != 0 )
		setTimeRange(_currentRecord->tmin() + offset, _currentRecord->tmax() + offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::moveTraces(double offset) {
	if ( fabs(offset) < 0.001 ) return;

	_recordView->move(offset);

	float tmin = _recordView->timeRangeMin();
	float tmax = _recordView->timeRangeMax();

	if ( tmin > _currentRecord->tmin() ) {
		offset = tmin - _currentRecord->tmin();
	}
	else if ( tmax < _currentRecord->tmax() ) {
		float length = tmax - tmin;
		float cr_length = _currentRecord->tmax() - _currentRecord->tmin();

		offset = tmax - _currentRecord->tmax();

		if ( cr_length > length )
			offset += cr_length - length;
	}
	else
		offset = 0;

	setTimeRange(_currentRecord->tmin() + offset,
	             _currentRecord->tmax() + offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::move(double offset) {
	if ( fabs(offset) < 0.001 ) return;

	float tmin = _currentRecord->tmin() + offset;
	float tmax = _currentRecord->tmax() + offset;

	if ( tmin < _recordView->timeRangeMin() ) {
		offset = tmin - _recordView->timeRangeMin();
	}
	else if ( tmax > _recordView->timeRangeMax() ) {
		float length = tmax - tmin;
		float rv_length = _recordView->timeRangeMax() - _recordView->timeRangeMin();

		offset = tmax - _recordView->timeRangeMax();

		if ( length > rv_length )
			offset -= length - rv_length;
	}
	else
		offset = 0;

	_recordView->move(offset);
	setTimeRange(tmin, tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scroll(int offset) {
	_currentRecord->setTracePaintOffset(-offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::itemSelected(RecordViewItem* item, RecordViewItem* lastItem) {
	float smin = 0;
	float smax = 0;

	Core::TimeSpan relSelectedTime;

	if ( lastItem ) {
		smin = lastItem->widget()->smin();
		smax = lastItem->widget()->smax();
		lastItem->widget()->setSelected(0,0);
		lastItem->widget()->setShadowWidget(NULL, false);
		lastItem->widget()->setCurrentMarker(NULL);

		disconnect(lastItem->label(), SIGNAL(statusChanged(bool)),
		           this, SLOT(setCurrentRowEnabled(bool)));

		relSelectedTime = lastItem->widget()->cursorPos() - lastItem->widget()->alignment();
	}

	if ( !item ) {
		_currentRecord->setDecorator(NULL);
		_currentRecord->clearRecords();
		_currentRecord->setEnabled(false);
		_currentRecord->setMarkerSourceWidget(NULL);
		_timeScale->setSelectionEnabled(false);
		return;
	}

	//_centerSelection = true;

	Core::Time cursorPos;
	RecordMarker* m = item->widget()->enabledMarker(item->widget()->cursorText());
	if ( m )
		cursorPos = m->correctedTime();
	else
		cursorPos = item->widget()->alignment() + relSelectedTime;

	//item->widget()->setCursorPos(cursorPos);
	//_currentRecord->setCursorPos(cursorPos);
	_currentRecord->setEnabled(item->widget()->isEnabled());
	_currentRecord->setDecorator(item->widget()->decorator());

	connect(item->label(), SIGNAL(statusChanged(bool)),
	        this, SLOT(setCurrentRowEnabled(bool)));

	double amplScale = _currentRecord->amplScale();

	_currentRecord->setNormalizationWindow(item->widget()->normalizationWindow());
	_currentRecord->setAlignment(item->widget()->alignment());

	_timeScale->setAlignment(item->widget()->alignment());

	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
	if ( label->processor ) {
		_timeScale->setSelectionEnabled(true);
		_timeScale->setSelectionHandle(0, double(label->processor->trigger()-_timeScale->alignment())+label->processor->config().noiseBegin);
		_timeScale->setSelectionHandle(1, double(label->processor->trigger()-_timeScale->alignment())+label->processor->config().signalBegin);
		_timeScale->setSelectionHandle(2, double(label->processor->trigger()-_timeScale->alignment())+label->processor->config().signalEnd);

		_recordView->timeWidget()->setSelectionHandle(0, label->processor->config().noiseBegin);
		_recordView->timeWidget()->setSelectionHandle(1, label->processor->config().signalBegin);
		_recordView->timeWidget()->setSelectionHandle(2, label->processor->config().signalEnd);
	}
	else
		_timeScale->setSelectionEnabled(false);

	if ( smax - smin > 0 )
		setTimeRange(smin, smax);
	else
		setTimeRange(_recordView->timeRangeMin(),_recordView->timeRangeMax());

	//_currentRecord->setAmplScale(item->widget()->amplScale());
	_currentRecord->setAmplScale(amplScale);

	item->widget()->setShadowWidget(_currentRecord, false);
	_currentRecord->setMarkerSourceWidget(item->widget());

	if ( _ui.actionLimitFilterToZoomTrace->isChecked() )
		applyFilter(item);

	/*
	if ( item->value(ITEM_DISTANCE_INDEX) >= 0 ) {
		if ( _config.showAllComponents &&
		     _config.allComponentsMaximumStationDistance >= item->value(ITEM_DISTANCE_INDEX) )
			_currentRecord->setDrawMode(RecordWidget::InRows);
		else
			_currentRecord->setDrawMode(RecordWidget::Single);
	}
	else
		_currentRecord->setDrawMode(RecordWidget::Single);
	*/
	_currentRecord->setDrawMode(RecordWidget::InRows);

	/*
	_currentRecord->clearMarker();
	for ( int i = 0; i < item->widget()->markerCount(); ++i )
		new AmplitudeViewMarker(_currentRecord, *static_cast<AmplitudeViewMarker*>(item->widget()->marker(i)));
	*/

	//_ui.labelCode->setText(item->label()->text(0));
	//_ui.labelInfo->setText(item->label()->text(1));

	if ( item->value(ITEM_DISTANCE_INDEX) >= 0 ) {
		if ( SCScheme.unit.distanceInKM )
			_ui.labelDistance->setText(QString("%1 km").arg(Math::Geo::deg2km(item->value(ITEM_DISTANCE_INDEX)),0,'f',SCScheme.precision.distance));
		else
			_ui.labelDistance->setText(QString("%1%2").arg(item->value(ITEM_DISTANCE_INDEX),0,'f',1).arg(degrees));
		_ui.labelAzimuth->setText(QString("%1%2").arg(item->value(ITEM_AZIMUTH_INDEX),0,'f',1).arg(degrees));
	}

	WaveformStreamID streamID = _recordView->streamID(item->row());
	std::string cha = streamID.channelCode();
	char component = item->currentComponent();

	for ( int i = 0; i < _currentRecord->slotCount(); ++i )
		_currentRecord->setRecordID(i, QString("%1").arg(item->mapSlotToComponent(i)));

	if ( cha.size() > 2 )
		cha[cha.size()-1] = component;
	else
		cha += component;

	_ui.labelStationCode->setText(streamID.stationCode().c_str());
	_ui.labelCode->setText(QString("%1  %2%3")
	                        .arg(streamID.networkCode().c_str())
	                        .arg(streamID.locationCode().c_str())
	                        .arg(cha.c_str()));
	/*
	const RecordSequence* seq = _currentRecord->records();
	if ( seq && !seq->empty() )
		_ui.labelCode->setText((*seq->begin())->streamID().c_str());
	else
		_ui.labelCode->setText("NO DATA");
	*/

	//_centerSelection = true;
	//_currentRecord->enableFiltering(item->widget()->isFilteringEnabled());
	setCursorPos(cursorPos);
	_currentRecord->update();

	updateCurrentRowState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setCurrentRowEnabled(bool enabled) {
	_currentRecord->setEnabled(enabled);
	updateCurrentRowState();

	RecordWidget* w = _recordView->currentItem()->widget();

	if ( w ) {
		for ( int i = 0; i < w->markerCount(); ++i ) {
			if ( w->marker(i)->id() >= 0 ) {
				/*
				emit arrivalChanged(w->marker(i)->id(), enabled?w->marker(i)->isEnabled():false);
				// To disable an arrival (trace) we have to keep this information
				// somewhere else not just in the picker
				emit arrivalEnableStateChanged(w->marker(i)->id(), enabled);
				*/
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setCurrentRowDisabled(bool disabled) {
	//setCurrentRowEnabled(!disabled);
	if ( _currentRecord->cursorText().isEmpty() ||
	     (!disabled && !_currentRecord->isEnabled()) ) {
		_currentRecord->setEnabled(!disabled);
		if ( _recordView->currentItem() )
			_recordView->currentItem()->label()->setEnabled(!disabled);
	}
	else {
		setMarkerState(_currentRecord, !disabled);
		if ( _recordView->currentItem() )
			setMarkerState(_recordView->currentItem()->widget(), !disabled);
	}

	updateCurrentRowState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setMarkerState(Seiscomp::Gui::RecordWidget* w, bool enabled) {
	bool foundManual = false;
	int arid = -1;

	for ( int m = 0; m < w->markerCount(); ++m ) {
		RecordMarker* marker = w->marker(m);
		if ( marker->text() == w->cursorText() ) {
			if ( marker->isMovable() ) foundManual = true;
			if ( marker->id() >= 0 ) arid = marker->id();
		}
	}

	for ( int m = 0; m < w->markerCount(); ++m ) {
		RecordMarker* marker = w->marker(m);
		if ( marker->text() == w->cursorText() ) {
			if ( marker->isEnabled() != enabled && arid >= 0 ) {
				//emit arrivalChanged(arid, enabled);
				arid = -1;
			}

			if ( marker->isMovable() || !foundManual ) {
				marker->setEnabled(enabled);
				marker->update();
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateCurrentRowState() {
	//setMarkerState(_currentRecord, enabled);
	//_currentRecord->setEnabled(enabled);

	bool enabled = true;

	if ( !_currentRecord->isEnabled() )
		enabled = false;
	else if ( !_currentRecord->cursorText().isEmpty() ) {
		RecordMarker* m = _currentRecord->marker(_currentRecord->cursorText(), true);
		if ( !m ) m = _currentRecord->marker(_currentRecord->cursorText(), false);
		enabled = m?m->isEnabled():true;
	}

	_ui.btnRowAccept->setChecked(false);
	_ui.btnRowAccept->setEnabled(enabled);
	_ui.btnRowReset->setEnabled(enabled);

	_ui.btnRowRemove->setChecked(!enabled);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::updateTraceInfo(RecordViewItem* item,
                                 const Seiscomp::Record* rec) {
	float timingQuality = item->widget()->timingQuality(_componentMap[_currentSlot]);
	if ( timingQuality >= 0 ) {
		if ( timingQuality > 100 ) timingQuality = 100;

		if ( timingQuality < 50 )
			static_cast<AmplitudeRecordLabel*>(item->label())->setLabelColor(blend(_config.timingQualityMedium, _config.timingQualityLow, (int)(timingQuality*2)));
		else
			static_cast<AmplitudeRecordLabel*>(item->label())->setLabelColor(blend(_config.timingQualityHigh, _config.timingQualityMedium, (int)((timingQuality-50)*2)));

		item->label()->setToolTip(QString("Timing quality: %1").arg((int)timingQuality));
	}
	else {
		static_cast<AmplitudeRecordLabel*>(item->label())->removeLabelColor();
		item->label()->setToolTip("Timing quality: undefined");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::toggleFilter() {
	if ( _comboFilter->currentIndex() > 1 )
		_comboFilter->setCurrentIndex(1);
	else {
		if ( _lastFilterIndex < 0 )
			_lastFilterIndex = std::min(_comboFilter->count()-1,2);

		_comboFilter->setCurrentIndex(_lastFilterIndex);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::addNewFilter(const QString& filter) {
	_lastFilterIndex = _comboFilter->findData(filter);

	if ( _lastFilterIndex == -1 ) {
		_comboFilter->addItem(filter, filter);
		_lastFilterIndex = _comboFilter->count()-1;
	}

	_comboFilter->setCurrentIndex(_lastFilterIndex);
	_currentRecord->setFilter(_recordView->filter());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scaleVisibleAmplitudes() {
	_recordView->scaleVisibleAmplitudes();

	_currentRecord->setNormalizationWindow(_currentRecord->visibleTimeWindow());
	_currentAmplScale = 1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::changeScale(double, float) {
	zoom(1.0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::changeTimeRange(double, double) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::sortAlphabetically() {
	_recordView->sortByTextAndValue(0, ITEM_PRIORITY_INDEX);

	_ui.actionSortAlphabetically->setChecked(true);
	_ui.actionSortByDistance->setChecked(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::sortByDistance() {
	_recordView->sortByValue(ITEM_DISTANCE_INDEX, ITEM_PRIORITY_INDEX);

	_ui.actionSortAlphabetically->setChecked(false);
	_ui.actionSortByDistance->setChecked(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showZComponent() {
	showComponent('Z');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showNComponent() {
	showComponent('1');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::showEComponent() {
	showComponent('2');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::alignOnOriginTime() {
	_checkVisibility = false;
	_recordView->setAbsoluteTimeEnabled(true);
	_recordView->setTimeRange(_minTime, _maxTime);
	_recordView->setSelectionEnabled(false);
	_checkVisibility = true;

	_recordView->setAlignment(_origin->time());

	_ui.actionAlignOnPArrival->setChecked(false);
	_ui.actionAlignOnOriginTime->setChecked(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::alignOnPArrivals() {
	int used = 0;
	double minTime = -10;
	double maxTime = 60;

	_ui.actionAlignOnPArrival->setChecked(true);
	_ui.actionAlignOnOriginTime->setChecked(false);

	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem *item = _recordView->itemAt(r);
		AmplitudeRecordLabel *l = static_cast<AmplitudeRecordLabel*>(item->label());

		RecordWidget* w = item->widget();

		// Find modified arrivals for phase of controller item
		RecordMarker* m = w->marker("P");

		if ( m ) {
			w->setAlignment(m->correctedTime());
			++used;
		}

		if ( l->processor ) {
			if ( l->processor->config().noiseBegin < minTime )
				minTime = l->processor->config().noiseBegin;
			if ( l->processor->config().signalEnd > maxTime )
				maxTime = l->processor->config().signalEnd;
		}
	}

	if ( !used ) return;

	_recordView->setAbsoluteTimeEnabled(false);
	_recordView->setTimeRange(minTime-5, maxTime+5);
	_recordView->setSelectionEnabled(true);

	if ( _recordView->currentItem() ) {
		RecordWidget* w = _recordView->currentItem()->widget();
		setAlignment(w->alignment());
		setCursorPos(w->alignment(), true);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::pickAmplitudes(bool) {
	setCursorText(_amplitudeType.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::pickNone(bool) {
	setCursorText("");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scaleAmplUp() {
	float scale = _currentRecord->amplScale();
	//if ( scale >= 1 ) scale = _currentAmplScale;
	float value = (scale == 0?1.0:scale)*_recordView->zoomFactor();
	if ( value > 1000 ) value = 1000;
	if ( /*value < 1*/true ) {
		_currentRecord->setAmplScale(value);
		_currentAmplScale = 1;
	}
	else {
		_currentRecord->setAmplScale(1);
		_currentAmplScale = value;
	}

	//_currentRecord->resize(_zoomTrace->width(), (int)(_zoomTrace->height()*_currentAmplScale));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scaleAmplDown() {
	float scale = _currentRecord->amplScale();
	//if ( scale >= 1 ) scale = _currentAmplScale;
	float value = (scale == 0?1.0:scale)/_recordView->zoomFactor();
	//if ( value < 1 ) value = 1;
	if ( value < 0.001 ) value = 0.001;

	//_currentRecord->setAmplScale(value);
	if ( /*value < 1*/true ) {
		_currentRecord->setAmplScale(value);
		_currentAmplScale = 1;
	}
	else {
		_currentRecord->setAmplScale(1);
		_currentAmplScale = value;
	}

	//_currentRecord->resize(_zoomTrace->width(), (int)(_zoomTrace->height()*_currentAmplScale));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scaleReset() {
	_currentRecord->setAmplScale(1.0);
	_currentAmplScale = 1.0;
	zoom(0.0);

	//_currentRecord->resize(_zoomTrace->width(), (int)(_zoomTrace->height()*_currentAmplScale));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scaleTimeUp() {
	zoom(_recordView->zoomFactor());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scaleTimeDown() {
	zoom(1.0/_recordView->zoomFactor());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::zoom(float factor) {
	_zoom *= factor;
	if ( _zoom < 1.0 )
		_zoom = 1.0;

	if ( _zoom > 100 )
		_zoom = 100;

	float currentScale = _currentRecord->timeScale();
	float newScale = _recordView->timeScale()*_zoom;

	factor = newScale/currentScale;

	float tmin = _currentRecord->tmin();
	float tmax = _recordView->currentItem()?
		tmin + _recordView->currentItem()->widget()->width()/_currentRecord->timeScale():
		_currentRecord->tmax();
	float tcen = tmin + (tmax-tmin)*0.5;

	tmin = tcen - (tcen-tmin)/factor;
	tmax = tcen + (tmax-tcen)/factor;

	_currentRecord->setTimeScale(newScale);
	_timeScale->setScale(newScale);
	
	if ( _checkVisibility ) ensureVisibility(tmin, tmax);
	setTimeRange(tmin, tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::applyTimeRange(double rmin, double rmax) {
	float tmin = (float)rmin;
	float tmax = (float)rmax;

	float newScale = _currentRecord->width() / (tmax-tmin);
	if ( newScale < _recordView->timeScale() )
		newScale = _recordView->timeScale();

	if ( tmin < _recordView->currentItem()->widget()->tmin() )
		tmin = _recordView->currentItem()->widget()->tmin();

	_currentRecord->setTimeScale(newScale);
	_timeScale->setScale(newScale);

	// Calculate zoom
	_zoom = newScale / _recordView->timeScale();

	if ( _checkVisibility ) ensureVisibility(tmin, tmax);
	setTimeRange(tmin, tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scrollLeft() {
	if ( !_currentRecord->cursorText().isEmpty() ) {
		Core::Time cp = _currentRecord->cursorPos();
		cp -= Core::TimeSpan((float)width()/(20*_currentRecord->timeScale()));
		setCursorPos(cp);
		return;
	}
	
	float offset = -(float)width()/(8*_currentRecord->timeScale());
	move(offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scrollFineLeft() {
if ( !_currentRecord->cursorText().isEmpty() ) {
		Core::Time cp = _currentRecord->cursorPos();
		cp -= Core::TimeSpan(1.0/_currentRecord->timeScale());
		setCursorPos(cp);
		return;
	}
	
	float offset = -1.0/_currentRecord->timeScale();
	move(offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scrollRight() {
	if ( !_currentRecord->cursorText().isEmpty() ) {
		Core::Time cp = _currentRecord->cursorPos();
		cp += Core::TimeSpan((float)width()/(20*_currentRecord->timeScale()));
		setCursorPos(cp);
		return;
	}

	float offset = (float)width()/(8*_currentRecord->timeScale());
	move(offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::scrollFineRight() {
	if ( !_currentRecord->cursorText().isEmpty() ) {
		Core::Time cp = _currentRecord->cursorPos();
		cp += Core::TimeSpan(1.0/_currentRecord->timeScale());
		setCursorPos(cp);
		return;
	}

	float offset = 1.0/_currentRecord->timeScale();
	move(offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::applyAmplitudes() {
	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem *rvi = _recordView->itemAt(r);
		RecordWidget *widget = rvi->widget();

		for ( int m = 0; m < widget->markerCount(); ++m ) {
			RecordMarker *marker = widget->marker(m);
			marker->apply();
		}
	}

	_changedAmplitudes.clear();
	_recordView->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeView::hasModifiedAmplitudes() const {
	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem *rvi = _recordView->itemAt(r);
		RecordWidget *widget = rvi->widget();
		for ( int m = 0; m < widget->markerCount(); ++m ) {
			RecordMarker *marker = widget->marker(m);
			if ( marker->isModified() )
				return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::getChangedAmplitudes(ObjectChangeList<DataModel::Amplitude> &list) const {
	list = _changedAmplitudes;
	_changedAmplitudes.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::fetchManualAmplitudes(std::vector<RecordMarker*>* markers) const {
	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem* rvi = _recordView->itemAt(r);
		AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(rvi->label());
		RecordWidget* widget = rvi->widget();

		if ( !widget->isEnabled() ) continue;

		// NOTE: Ignore items that are children of other items and not
		//       expanded (eg SM channels)
		if ( label->isLinkedItem() ) {
			AmplitudeRecordLabel *controllerLabel = static_cast<AmplitudeRecordLabel*>(label->controlledItem()->label());
			if ( !controllerLabel->isExpanded() ) continue;
		}

		bool hasManualMarker = false;

		// Count the number of interesting markers for a particular phase
		for ( int m = 0; m < widget->markerCount(); ++m ) {
			AmplitudeViewMarker *marker = (AmplitudeViewMarker*)widget->marker(m);
			if ( !marker->isEnabled() ) continue;
			if ( marker->isNewAmplitude() ) {
				hasManualMarker = true;
				break;
			}
		}

		for ( int m = 0; m < widget->markerCount(); ++m ) {
			AmplitudeViewMarker *marker = (AmplitudeViewMarker*)widget->marker(m);

			// If the marker is not an amplitude do nothing
			if ( !marker->isAmplitude() ) continue;
			if ( !marker->isEnabled() ) continue;

			AmplitudePtr amp = marker->amplitude();

			// Picked marker and we've got an manual replacement: do nothing
			if ( hasManualMarker && !marker->isNewAmplitude() ) {
				//SEISCOMP_DEBUG("   - ignore amplitude to be replaced");
				marker->setId(-1);
				continue;
			}

			if ( amp ) {
				if ( !marker->isModified() ) {
					if ( markers )
						markers->push_back(marker);
					SEISCOMP_DEBUG("   - reuse existing amplitude");
					continue;
				}
			}

			AmplitudePtr a = findAmplitude(widget, marker->correctedTime());

			// If the marker did not make any changes to the amplitude
			// attributes, reuse it.
			if ( a && !marker->equalsAmplitude(a.get()) ) a = NULL;

			if ( !a ) {
				const Processing::AmplitudeProcessor::Result &res = marker->amplitudeResult();
				WaveformStreamID s = _recordView->streamID(r);
				a = Amplitude::Create();

				if ( res.component <= Processing::WaveformProcessor::SecondHorizontal )
					a->setWaveformID(
						WaveformStreamID(
							s.networkCode(), s.stationCode(),
							s.locationCode(), label->data.traces[res.component].channelCode, ""
						)
					);
				else
					a->setWaveformID(
						WaveformStreamID(
							s.networkCode(), s.stationCode(),
							s.locationCode(), s.channelCode().substr(0,2), ""
						)
					);

				a->setAmplitude(
					RealQuantity(res.amplitude.value, Core::None,
					             res.amplitude.lowerUncertainty,
					             res.amplitude.upperUncertainty, Core::None)
				);

				if ( res.period > 0 ) a->setPeriod(RealQuantity(res.period));
				if ( res.snr >= 0 ) a->setSnr(res.snr);
				a->setType(label->processor->type());
				a->setUnit(label->processor->unit());
				a->setTimeWindow(
					TimeWindow(res.time.reference, res.time.begin, res.time.end)
				);
				a->setPickID(label->processor->referencingPickID());
				a->setFilterID(marker->filterID());

				a->setEvaluationMode(EvaluationMode(MANUAL));

				CreationInfo ci;
				ci.setAgencyID(SCApp->agencyID());
				ci.setAuthor(SCApp->author());
				ci.setCreationTime(Core::Time::GMT());
				a->setCreationInfo(ci);

				_changedAmplitudes.push_back(ObjectChangeList<DataModel::Amplitude>::value_type(a,true));
				SEISCOMP_DEBUG("   - created new amplitude");
			}
			else {
				SEISCOMP_DEBUG("   - reuse active amplitude");
			}

			if ( markers ) markers->push_back(marker);
			marker->setAmplitude(a.get());
		}
	}

	/*
	// Remove all automatic amplitudes if configured
	if ( _config.removeAutomaticPicks ) {
		for ( std::vector<RecordMarker*>::iterator it = markers->begin();
		      it != markers->end(); ) {
			try {
				if ( static_cast<PickerMarker*>(*it)->pick()->evaluationMode() == MANUAL ) {
					++it;
					continue;
				}
			}
			catch ( ... ) {}

			it = markers->erase(it);
		}
	}
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::commit() {
	std::vector<RecordMarker*> markers;
	fetchManualAmplitudes(&markers);

	QList<AmplitudePtr> amps;

	for ( size_t i = 0; i < markers.size(); ++i ) {
		AmplitudeViewMarker *m = static_cast<AmplitudeViewMarker*>(markers[i]);
		if ( !m->isEnabled() ) continue;
		AmplitudePtr amp = m->amplitude();
		if ( amp == NULL ) {
			SEISCOMP_ERROR("Amplitude not set in marker");
			continue;
		}

		amps.append(amp);
	}

	if ( !amps.isEmpty() )
		emit amplitudesConfirmed(_origin.get(), amps);

	return;


	SEISCOMP_DEBUG("Origin.stationMags before: %d", (int)_origin->stationMagnitudeCount());

	// Remove all station magnitudes of origin with requested type
	for ( size_t i = 0; i < _origin->stationMagnitudeCount(); ) {
		if ( _origin->stationMagnitude(i)->type() == _magnitudeType )
			_origin->removeStationMagnitude(i);
		else
			++i;
	}

	SEISCOMP_DEBUG("Origin.stationMags after: %d", (int)_origin->stationMagnitudeCount());

	if ( !_magnitude )
		_magnitude = Magnitude::Create();
	else {
		// Remove all stationmagnitude references from magnitude
		while ( _magnitude->stationMagnitudeContributionCount() > 0 )
			_magnitude->removeStationMagnitudeContribution(0);

		SEISCOMP_DEBUG("Mag.stationMagRefs after: %d",
		               (int)_magnitude->stationMagnitudeContributionCount());
	}

	CreationInfo ci;
	ci.setAgencyID(SCApp->agencyID());
	ci.setAuthor(SCApp->author());
	ci.setCreationTime(Core::Time::GMT());

	_magnitude->setCreationInfo(ci);
	_magnitude->setType(_magnitudeType);
	_magnitude->setEvaluationStatus(EvaluationStatus(CONFIRMED));
	_magnitude->setOriginID("");

	vector<double> mags;
	vector<double> azimuths;
	set<string> stations;

	for ( size_t i = 0; i < markers.size(); ++i ) {
		AmplitudeViewMarker *m = static_cast<AmplitudeViewMarker*>(markers[i]);
		//if ( !m->isEnabled() ) continue;

		AmplitudePtr amp = m->amplitude();
		if ( !amp ) {
			SEISCOMP_ERROR("Amplitude not set in marker");
			continue;
		}

		RecordWidget *w = m->parent();
		AmplitudeRecordLabel *label = static_cast<TraceDecorator*>(w->decorator())->label();
		RecordViewItem *item = label->recordViewItem();

		if ( !label->magnitudeProcessor ) {
			SEISCOMP_ERROR("No magnitude processor attached for station %s",
			               item->data().toString().toStdString().c_str());
			continue;
		}

		double magValue;
		double period;

		try { period = amp->period().value(); } catch ( ... ) { period = 0; }

		Processing::MagnitudeProcessor::Status stat =
			label->magnitudeProcessor->computeMagnitude(
				amp->amplitude().value(), period,
				item->value(ITEM_DISTANCE_INDEX), _origin->depth(), magValue
			);

		if ( stat != Processing::MagnitudeProcessor::OK ) {
			SEISCOMP_ERROR("Failed to compute magnitude for station %s: %s",
			               item->data().toString().toStdString().c_str(),
			               stat.toString());
			continue;
		}

		mags.push_back(magValue);
		azimuths.push_back(item->value(ITEM_AZIMUTH_INDEX));

		StationMagnitudePtr staMag = StationMagnitude::Create();
		CreationInfo ci;
		ci.setAgencyID(SCApp->agencyID());
		ci.setAuthor(SCApp->author());
		ci.setCreationTime(Core::Time::GMT());
		staMag->setType(_magnitude->type());
		staMag->setCreationInfo(ci);
		staMag->setWaveformID(amp->waveformID());
		staMag->setMagnitude(magValue);
		staMag->setAmplitudeID(amp->publicID());

		_origin->add(staMag.get());

		StationMagnitudeContributionPtr ref = new StationMagnitudeContribution;
		ref->setStationMagnitudeID(staMag->publicID());
		ref->setWeight(m->isEnabled()?1.0:0.0);

		_magnitude->add(ref.get());

		stations.insert(amp->waveformID().networkCode() + "." + amp->waveformID().stationCode());
	}

	// Magnitudes are averaged by the handler of this signal
	emit magnitudeCreated(_magnitude.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setDefaultDisplay() {
	//alignByState();
	alignOnPArrivals();
	selectFirstVisibleItem(_recordView);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/*
bool AmplitudeView::start() {
	stop();

	if ( _recordView->start() ) {
		connect(_recordView->recordStreamThread(), SIGNAL(finished()),
		        this, SLOT(acquisitionFinished()));
		return true;
	}

	return false;
}
*/
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::stop() {
	_recordView->stop();
	closeThreads();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::selectTrace(const Seiscomp::DataModel::WaveformStreamID &wid) {
	RecordViewItem *item = _recordView->item(wid);
	if ( item ) {
		_recordView->setCurrentItem(item);
		_recordView->ensureVisible(item->row());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::selectTrace(const std::string &code) {
	for ( int i = 0; i < _recordView->rowCount(); ++i ) {
		if ( _recordView->itemAt(i)->streamID().stationCode() == code ) {
			_recordView->setCurrentItem(_recordView->itemAt(i));
			_recordView->ensureVisible(i);
			return;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::closeThreads() {
	foreach ( RecordStreamThread* t, _acquisitionThreads) {
		t->stop(true);
		SEISCOMP_DEBUG("removed finished thread %d from list", t->ID());
		delete t;
	}

	_acquisitionThreads.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::acquisitionFinished() {
	QObject* s = sender();
	if ( s ) {
		RecordStreamThread* t = static_cast<RecordStreamThread*>(s);
		int index = _acquisitionThreads.indexOf(t);
		if ( index != -1 ) {
			_acquisitionThreads.remove(index);
			SEISCOMP_DEBUG("removed finished thread %d from list", t->ID());
			delete t;
		}

		// Update color states
		for ( int i = 0; i < _recordView->rowCount(); ++i ) {
			RecordViewItem *item = _recordView->itemAt(i);
			RecordWidget *widget = item->widget();

			AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());

			for ( int i = 0; i < 3; ++i ) {
				if ( label->data.traces[i].thread != t ) continue;
				if ( label->data.traces[i].raw && !label->data.traces[i].raw->empty() )
					widget->removeRecordBackgroundColor(label->data.traces[i].recordSlot);
				else
					widget->setRecordBackgroundColor(label->data.traces[i].recordSlot, SCScheme.colors.records.states.notAvailable);
				// Reset the thread
				label->data.traces[i].thread = NULL;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::acquireStreams() {
	if ( _nextStreams.empty() ) return;

	RecordStreamThread *t = new RecordStreamThread(_config.recordURL.toStdString());

	if ( !t->connect() ) {
		if ( _config.recordURL != _lastRecordURL ) {
			QMessageBox::critical(this, "Waveform acquisition",
			                      QString("Unable to open recordstream '%1'").arg(_config.recordURL));
		}

		_lastRecordURL = _config.recordURL;
		delete t;
		return;
	}

	connect(t, SIGNAL(receivedRecord(Seiscomp::Record*)),
	        this, SLOT(receivedRecord(Seiscomp::Record*)));

	connect(t, SIGNAL(finished()),
	        this, SLOT(acquisitionFinished()));

	for ( WaveformStreamList::const_iterator it = _nextStreams.begin();
	      it != _nextStreams.end(); ++it ) {
		if ( it->timeWindow ) {
			if ( !t->addStream(it->streamID.networkCode(),
				               it->streamID.stationCode(),
				               it->streamID.locationCode(),
				               it->streamID.channelCode(),
				               it->timeWindow.startTime(), it->timeWindow.endTime()) )
				t->addStream(it->streamID.networkCode(),
				             it->streamID.stationCode(),
				             it->streamID.locationCode(),
				             it->streamID.channelCode());
		}
		else {
			SEISCOMP_WARNING("Not time window for stream %s set: ignoring", waveformIDToStdString(it->streamID).c_str());
			continue;
		}

		RecordViewItem *item = _recordView->item(adjustWaveformStreamID(it->streamID));
		if ( item ) {
			int slot = item->mapComponentToSlot(*it->streamID.channelCode().rbegin());
			item->widget()->setRecordBackgroundColor(slot, SCScheme.colors.records.states.requested);
			item->widget()->setRecordUserData(slot, qVariantFromValue((void*)t));
			// Store the acquisition thread as user data
			static_cast<AmplitudeRecordLabel*>(item->label())->data.traces[it->component].thread = t;
		}
	}

	_nextStreams.clear();

	_acquisitionThreads.push_back(t);
	t->start();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::receivedRecord(Seiscomp::Record *rec) {
	Seiscomp::RecordPtr tmp(rec);
	if ( !rec->data() ) return;

	std::string streamID = rec->streamID();
	RecordItemMap::iterator it = _recordItemLabels.find(streamID);
	if ( it == _recordItemLabels.end() )
		return;

	AmplitudeRecordLabel *label = it->second;
	RecordViewItem *item = label->recordViewItem();

	int i;
	for ( i = 0; i < 3; ++i ) {
		if ( label->data.traces[i].channelCode == rec->channelCode() ) {
			if ( label->data.traces[i].raw == NULL )
				label->data.traces[i].raw = new TimeWindowBuffer(label->timeWindow);
			break;
		}
	}

	if ( i == 3 ) return;

	bool firstRecord = label->data.traces[i].raw->empty();
	if ( !label->data.traces[i].raw->feed(rec) ) return;

	// Check for out-of-order records
	if ( (label->data.traces[i].filter || label->data.enableTransformation) &&
	     label->data.traces[i].raw->back() != (const Record*)rec ) {
		SEISCOMP_DEBUG("%s.%s.%s.%s: out of order record, reinitialize trace",
		               rec->networkCode().c_str(),
		               rec->stationCode().c_str(),
		               rec->locationCode().c_str(),
		               rec->channelCode().c_str());
		RecordWidget::Filter *f = label->data.traces[i].filter->clone();
		label->data.setFilter(f, label->data.filterID);
		delete f;
	}
	else
		label->data.transform(i, rec);

	if ( firstRecord ) {
		item->widget()->setRecordBackgroundColor(_componentMap[i], SCScheme.colors.records.states.inProgress);
		label->hasGotData = true;

		if ( _config.hideStationsWithoutData )
			item->forceInvisibilty(!label->isEnabledByConfig);

		//item->widget()->setRecords(i, label->traces[i].raw, false);

		// If this item is linked to another item, enable the expand button of
		// the controller
		if ( label->isLinkedItem() && label->_linkedItem != NULL )
			static_cast<AmplitudeRecordLabel*>(label->_linkedItem->label())->enabledExpandButton(item);
	}
	else {
		// Tell the widget to rebuild its traces
		//item->widget()->fed(i, rec);
		updateTraceInfo(item, rec);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::addStations() {
	if ( !_origin ) return;

	SelectStation dlg(_origin->time(), _stations, this);
	dlg.setReferenceLocation(_origin->latitude(), _origin->longitude());
	if ( dlg.exec() != QDialog::Accepted ) return;

	QList<DataModel::Station *> stations = dlg.selectedStations();
	if ( stations.isEmpty() ) return;

	_recordView->setUpdatesEnabled(false);

	foreach ( DataModel::Station *s, stations ) {
		DataModel::Network *n = s->network();

		QString code = (n->code() + "." + s->code()).c_str();

		if ( _stations.contains(code) ) continue;

		double delta, az1, az2;
		Geo::delazi(_origin->latitude(), _origin->longitude(),
		            s->latitude(), s->longitude(), &delta, &az1, &az2);

		// Skip stations out of amplitude processors range
		if ( delta < _minDist || delta > _maxDist ) continue;

		Stream *stream = NULL;
		// Preferred channel code is BH. If not available use either SH or skip.
		for ( size_t c = 0; c < _broadBandCodes.size(); ++c ) {
			stream = findStream(s, _broadBandCodes[c], _origin->time());
			if ( stream ) break;
		}

		if ( stream == NULL )
			stream = findStream(s, _origin->time(), Processing::WaveformProcessor::MeterPerSecond);

		if ( stream ) {
			WaveformStreamID streamID(n->code(), s->code(), stream->sensorLocation()->code(), stream->code().substr(0,stream->code().size()-1) + '?', "");

			try {
				TravelTime ttime =
					_ttTable.computeFirst(_origin->latitude(), _origin->longitude(),
					                      _origin->depth(), s->latitude(), s->longitude());

				Core::Time referenceTime = _origin->time().value() + Core::TimeSpan(ttime.time);

				RecordViewItem* item = addStream(stream->sensorLocation(), streamID, referenceTime, false);
				if ( item ) {
					_stations.insert(code);
					item->setVisible(!_ui.actionShowUsedStations->isChecked());
					if ( _config.hideStationsWithoutData )
						item->forceInvisibilty(true);
				}
			}
			catch ( ... ) {}
		}
	}

	sortByState();
	alignByState();
	componentByState();

	if ( _recordView->currentItem() == NULL )
		selectFirstVisibleItem(_recordView);

	setCursorText(_currentRecord->cursorText());

	_recordView->setUpdatesEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::searchStation() {
	_searchStation->selectAll();
	_searchStation->setVisible(true);
	_searchLabel->setVisible(true);

	//_searchStation->grabKeyboard();
	_searchStation->setFocus();
	_recordView->setFocusProxy(_searchStation);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::searchByText(const QString &text) {
	if ( text.isEmpty() ) return;

	QRegExp rx(text + "*");
	rx.setPatternSyntax(QRegExp::Wildcard);
	rx.setCaseSensitivity(Qt::CaseInsensitive);

	int row = _recordView->findByText(0, rx, _lastFoundRow+1);
	if ( row != -1 ) {
		_recordView->setCurrentItem(_recordView->itemAt(row));
		_lastFoundRow = row;

		QPalette pal = _searchStation->palette();
		pal.setColor(QPalette::Base, _searchBase);
		_searchStation->setPalette(pal);

		_recordView->ensureVisible(_lastFoundRow);
	}
	else {
		QPalette pal = _searchStation->palette();
		pal.setColor(QPalette::Base, _searchError);
		_searchStation->setPalette(pal);
		_lastFoundRow = -1;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::search(const QString &text) {
	_lastFoundRow = -1;

	searchByText(text);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::nextSearch() {
	searchByText(_searchStation->text());
	if ( _lastFoundRow == -1 )
		searchByText(_searchStation->text());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::abortSearchStation() {
	_recordView->setFocusProxy(NULL);
	_searchStation->releaseKeyboard();

	_searchStation->setVisible(false);
	_searchLabel->setVisible(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/*
void AmplitudeView::emitPick(const Processing::Picker *,
                             const Processing::Picker::Result &res) {
	setCursorPos(res.time);
}
*/
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::createAmplitude() {
	RecordViewItem* item = _recordView->currentItem();
	if ( item && !item->widget()->cursorText().isEmpty() ) {
		onSelectedTime(item->widget(), item->widget()->cursorPos());
		onSelectedTime(_currentRecord, _currentRecord->cursorPos());

		_recordView->selectNextRow();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setAmplitude() {
	RecordViewItem* item = _recordView->currentItem();
	if ( item && !item->widget()->cursorText().isEmpty() ) {
		onSelectedTime(item->widget(), item->widget()->cursorPos());
		onSelectedTime(_currentRecord, _currentRecord->cursorPos());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::confirmAmplitude() {
	RecordViewItem* item = _recordView->currentItem();
	if ( item && !item->widget()->cursorText().isEmpty() ) {
		onSelectedTime(item->widget(), item->widget()->cursorPos());
		onSelectedTime(_currentRecord, _currentRecord->cursorPos());

		int row = item->row() + 1;

		item = NULL;

		for ( int i = 0; i < _recordView->rowCount(); ++i, ++row ) {
			if ( row >= _recordView->rowCount() ) row -= _recordView->rowCount();

			RecordViewItem* nextItem = _recordView->itemAt(row);

			// ignore disabled rows
			if ( !nextItem->widget()->isEnabled() ) continue;

			RecordMarker* m = nextItem->widget()->marker(nextItem->widget()->cursorText());
			if ( m ) {
				item = nextItem;
				_recordView->setCurrentItem(nextItem);
				_recordView->ensureVisible(row);
				break;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::deleteAmplitude() {
	RecordViewItem* item = _recordView->currentItem();
	AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
	if ( item ) {
		label->isError = false;
		label->infoText = QString();
		if ( item->widget()->cursorText().isEmpty() )
			resetAmplitude(item, _amplitudeType.c_str(), true);
		else
			resetAmplitude(item, item->widget()->cursorText(), true);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::resetAmplitude(RecordViewItem *item, const QString &text, bool enable) {
	AmplitudeViewMarker* m = static_cast<AmplitudeViewMarker*>(item->widget()->marker(text, true));
	if ( m ) {
		if ( m->isMoveCopyEnabled() ) {
			m->reset();
			m->setEnabled(enable);
		}
		else {
			delete m;
			m = static_cast<AmplitudeViewMarker*>(item->widget()->marker(text));
			if ( m )
				m->setEnabled(enable);
		}
	}
	else {
		m = static_cast<AmplitudeViewMarker*>(item->widget()->marker(text));
		if ( m )
			m->setEnabled(enable);
	}

	item->widget()->update();
	_currentRecord->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::addFilter(const QString& name, const QString& filter) {
	if ( _comboFilter ) {
		if ( _comboFilter->findText(name) != -1 )
			return;

		_comboFilter->addItem(name, filter);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::limitFilterToZoomTrace(bool e) {
	changeFilter(_comboFilter->currentIndex(), true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::changeFilter(int index) {
	changeFilter(index, false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeView::applyFilter(RecordViewItem *item) {
	if ( item == NULL ) {
		for ( int i = 0; i < _recordView->rowCount(); ++i ) {
			RecordViewItem* rvi = _recordView->itemAt(i);
			AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(rvi->label());
			if ( label->processor ) {
				Core::Time t = label->processor->trigger();
				label->processor->reset();
				label->processor->setTrigger(t);
			}
			label->data.setFilter(_currentFilter, _currentFilterStr);
			label->data.showProcessedData(_showProcessedData);
		}
	}
	else {
		for ( int i = 0; i < _recordView->rowCount(); ++i ) {
			RecordViewItem* rvi = _recordView->itemAt(i);
			AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(rvi->label());
			label->data.showProcessedData(_showProcessedData);
		}

		AmplitudeRecordLabel *label = static_cast<AmplitudeRecordLabel*>(item->label());
		if ( label->processor ) {
			Core::Time t = label->processor->trigger();
			label->processor->reset();
			label->processor->setTrigger(t);
		}
		label->data.setFilter(_currentFilter, _currentFilterStr);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::changeFilter(int index, bool force) {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QString name = _comboFilter->itemText(index);
	QString filter = _comboFilter->itemData(index).toString();

	_showProcessedData = false;

	if ( name == NO_FILTER_STRING ) {
		if ( _currentFilter ) delete _currentFilter;
		_currentFilter = NULL;
		_currentFilterStr = "";

		if ( !_ui.actionLimitFilterToZoomTrace->isChecked() )
			applyFilter();
		else
			applyFilter(_recordView->currentItem());

		QApplication::restoreOverrideCursor();
		return;
	}
	else if ( name == DEFAULT_FILTER_STRING ) {
		if ( _currentFilter ) delete _currentFilter;
		_currentFilter = NULL;
		_currentFilterStr = "";

		_showProcessedData = true;
		applyFilter();

		QApplication::restoreOverrideCursor();
		return;
	}

	_showProcessedData = true;
	RecordWidget::Filter *newFilter = RecordWidget::Filter::Create(filter.toStdString());

	if ( newFilter == NULL ) {
		QMessageBox::critical(this, "Invalid filter",
		                      QString("Unable to create filter: %1\nFilter: %2").arg(name).arg(filter));

		_comboFilter->blockSignals(true);
		_comboFilter->setCurrentIndex(_lastFilterIndex);
		_comboFilter->blockSignals(false);

		QApplication::restoreOverrideCursor();
		return;
	}

	if ( _currentFilter ) delete _currentFilter;
	_currentFilter = newFilter;
	_currentFilterStr = filter.toStdString();

	if ( !_ui.actionLimitFilterToZoomTrace->isChecked() )
		applyFilter();
	else
		applyFilter(_recordView->currentItem());

	_lastFilterIndex = index;
	QApplication::restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void AmplitudeView::setArrivalState(int arrivalId, bool state) {
	setArrivalState(_currentRecord, arrivalId, state);

	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem* item = _recordView->itemAt(r);
		if ( setArrivalState(item->widget(), arrivalId, state) ) {
			item->setVisible(!(_ui.actionShowUsedStations->isChecked() &&
			                   item->widget()->hasMovableMarkers()));
			if ( state )
				item->label()->setEnabled(true);
			break;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool AmplitudeView::setArrivalState(RecordWidget* w, int arrivalId, bool state) {
	if ( !w->isEnabled() ) return false;

	bool foundManual = false;
	QString phase;

	// Find phase for arrival
	for ( int m = 0; m < w->markerCount(); ++m ) {
		RecordMarker* marker = w->marker(m);
		if ( marker->id() == arrivalId )
			phase = marker->text();
	}

	// Find manual marker for arrival
	for ( int m = 0; m < w->markerCount(); ++m ) {
		RecordMarker* marker = w->marker(m);
		if ( marker->text() == phase && marker->isMovable() )
			foundManual = true;
	}

	// Update state
	for ( int m = 0; m < w->markerCount(); ++m ) {
		RecordMarker* marker = w->marker(m);

		if ( marker->text() == phase ) {
			if ( marker->isMovable() || !foundManual ) {
				marker->setEnabled(state);
				marker->update();
				return true;
			}
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
