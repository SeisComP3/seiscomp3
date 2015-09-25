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



#define SEISCOMP_COMPONENT Gui::PickerView

#include "pickerview.h"
#include <seiscomp3/core/platform/platform.h>
#include <seiscomp3/gui/datamodel/selectstation.h>
#include <seiscomp3/gui/datamodel/origindialog.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/recordstreamthread.h>
#include <seiscomp3/gui/core/timescale.h>
#include <seiscomp3/gui/core/uncertainties.h>
#include <seiscomp3/gui/core/spectrogramrenderer.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/client/configdb.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/parameter.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/math/filter.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/interfacefactory.ipp>
#include <seiscomp3/seismology/ttt.h>
#include <seiscomp3/utils/misc.h>
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

#define NO_FILTER_STRING     "No filter"

#define ITEM_DISTANCE_INDEX  0
#define ITEM_RESIDUAL_INDEX  1
#define ITEM_AZIMUTH_INDEX  2
//#define ITEM_ARRIVALID_INDEX 2
#define ITEM_PRIORITY_INDEX  3

#define AUTOMATIC_POSTFIX    " "
#define THEORETICAL_POSTFIX  "  "

#define SET_PICKED_COMPONENT
//#define CENTER_SELECTION

#define COMP_NO_METADATA '\0'


using namespace Seiscomp;
using namespace Seiscomp::DataModel;
using namespace Seiscomp::Math;
using namespace Seiscomp::Util;
using namespace Seiscomp::Gui;
using namespace Seiscomp::Gui::PrivatePickerView;


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Gui::PickerMarkerActionPlugin, SC_GUI_API);


namespace {


char ZNE_COMPS[3] = {'Z', 'N', 'E'};
char ZRT_COMPS[3] = {'Z', 'R', 'T'};
char ZH_COMPS[3] = {'Z', 'H', '-'};
char Z12_COMPS[3] = {'Z', '1', '2'};


MAKEENUM(
	RotationType,
	EVALUES(
		RT_Z12,
		RT_ZNE,
		RT_ZRT,
		RT_ZH
	),
	ENAMES(
		"Z12",
		"ZNE",
		"ZRT",
		"ZH(RMS)"
	)
);


class ZoomRecordWidget : public RecordWidget {
	public:
		ZoomRecordWidget() {
			maxLower = maxUpper = 0;
			currentIndex = -1;
			crossHair = false;
			showSpectrogram = false;
			traces = NULL;

			Gradient gradient;
			gradient.setColorAt(0.0, QColor(255,   0, 255,   0));
			gradient.setColorAt(0.2, QColor(  0,   0, 255, 255));
			gradient.setColorAt(0.4, QColor(  0, 255, 255, 255));
			gradient.setColorAt(0.6, QColor(  0, 255,   0, 255));
			gradient.setColorAt(0.8, QColor(255, 255,   0, 255));
			gradient.setColorAt(1.0, QColor(255,   0,   0, 255));

			for ( int i = 0; i < 3; ++i ) {
				spectrogram[i].setOptions(spectrogram[i].options());
				spectrogram[i].setGradient(gradient);
			}
		}

		void setUncertainties(const PickerView::Config::UncertaintyList &list) {
			uncertainties = list;
			maxLower = maxUpper = 0;
			currentIndex = -1;

			for ( int i = 0; i < uncertainties.count(); ++i ) {
				if ( i == 0 ) {
					maxLower = uncertainties[i].first;
					maxUpper = uncertainties[i].second;
				}
				else {
					maxLower = std::max(maxLower, (double)uncertainties[i].first);
					maxUpper = std::max(maxUpper, (double)uncertainties[i].second);
				}
			}
		}

		void setCurrentUncertaintyIndex(int idx) {
			currentIndex = idx;
			update();
		}

		int currentUncertaintyIndex() const {
			return currentIndex;
		}

		void setCrossHairEnabled(bool enable) {
			crossHair = enable;
			update();
		}

		void setShowSpectrogram(bool enable) {
			if ( showSpectrogram == enable ) return;

			showSpectrogram = enable;

			resetSpectrogram();
			update();
		}

		void setLogSpectrogram(bool enable) {
			for ( int i = 0; i < 3; ++i )
				spectrogram[i].setLogScale(enable);
			update();
		}

		void setMinSpectrogramRange(double v) {
			for ( int i = 0; i < 3; ++i )
				spectrogram[i].setGradientRange(v, spectrogram[i].gradientUpperBound());
			update();
		}

		void setMaxSpectrogramRange(double v) {
			for ( int i = 0; i < 3; ++i )
				spectrogram[i].setGradientRange(spectrogram[i].gradientLowerBound(), v);
			update();
		}

		void setSpectrogramTimeWindow(double tw) {
			for ( int i = 0; i < 3; ++i ) {
				IO::Spectralizer::Options opts = spectrogram[i].options();
				opts.windowLength = tw;
				spectrogram[i].setOptions(opts);
			}

			if ( showSpectrogram ) {
				resetSpectrogram();
				update();
			}
		}

		void setTraces(ThreeComponentTrace::Component *t) {
			traces = t;
			resetSpectrogram();
		}

		void feedRaw(int slot, const Seiscomp::Record *rec) {
			if ( showSpectrogram && (slot >= 0) && (slot < 3))
				spectrogram[slot].feed(rec);
		}

	private:
		void resetSpectrogram() {
			if ( showSpectrogram ) {
				qApp->setOverrideCursor(Qt::WaitCursor);
				for ( int i = 0; i < 3; ++i ) {
					const double *scale = recordScale(i);
					// Scale is is nm and needs to be converted to m
					if ( scale != NULL ) spectrogram[i].setScale(*scale * 1E-9);
					spectrogram[i].setRecords(traces != NULL ? traces[i].raw : NULL);
					spectrogram[i].renderSpectrogram();
				}
				qApp->restoreOverrideCursor();
			}
		}

		void drawSpectrogram(QPainter &painter, int slot) {
			QRect r = rect();
			r.setHeight(streamHeight(slot));
			r.moveTop(streamYPos(slot));
			spectrogram[slot].setAlignment(alignment());
			spectrogram[slot].setTimeRange(tmin(), tmax());
			spectrogram[slot].render(painter, r, false, false);
		}

		void drawSpectrogramAxis(QPainter &painter, int slot) {
			QRect r = rect();
			r.setHeight(streamHeight(slot));
			r.moveTop(streamYPos(slot));
			spectrogram[slot].setAlignment(alignment());
			spectrogram[slot].setTimeRange(tmin(), tmax());
			spectrogram[slot].renderAxis(painter, r, false);
		}

	protected:
		virtual void paintEvent(QPaintEvent *p) {
			RecordWidget::paintEvent(p);

			if ( showSpectrogram ) {
				QPainter painter(this);
				painter.setBrush(palette().color(QPalette::Window));

				switch ( drawMode() ) {
					case InRows:
						for ( int i = 0; i < 3; ++i )
							drawSpectrogramAxis(painter, i);
						break;
					case Single:
						if ( (currentRecords() >= 0) && (currentRecords() < 3) )
							drawSpectrogramAxis(painter, currentRecords());
						break;
					default:
						break;
				}
			}
		}

		virtual void drawCustomBackground(QPainter &painter) {
			if ( showSpectrogram ) {
				painter.setBrush(palette().color(QPalette::Window));

				switch ( drawMode() ) {
					case InRows:
						for ( int i = 0; i < 3; ++i )
							drawSpectrogram(painter, i);
						break;
					case Single:
						if ( (currentRecords() >= 0) && (currentRecords() < 3) )
							drawSpectrogram(painter, currentRecords());
						break;
					default:
						break;
				}
			}
		}

		virtual void drawActiveCursor(QPainter &painter, int x, int y) {
			RecordWidget::drawActiveCursor(painter, x, y);

			if ( !crossHair ) return;
			if ( maxLower <= 0 && maxUpper <= 0 ) return;

			int xl = (int)(maxLower*timeScale());
			int xu = (int)(maxUpper*timeScale());
			painter.drawLine(x-xl+1,y,x+xu,y);

			painter.setPen(palette().color(QPalette::WindowText));
			for ( int i = 0; i < uncertainties.size(); ++i ) {
				double lower = uncertainties[i].first;
				double upper = uncertainties[i].second;

				if ( lower > 0 && xl > 0 ) {
					int x0 = (int)(lower*timeScale());
					int h = 12-10*x0/xl;
					painter.drawLine(x-x0, y-h, x-x0, y+h);
				}

				if ( upper > 0 && xu > 0 ) {
					int x0 = (int)(upper*timeScale());
					int h = 12-10*x0/xu;
					painter.drawLine(x+x0, y-h, x+x0, y+h);
				}
			}

			if ( currentIndex >= 0 ) {
				painter.setPen(QPen(palette().color(QPalette::Highlight), 2));
				double lower = uncertainties[currentIndex].first;
				double upper = uncertainties[currentIndex].second;

				if ( lower > 0 && xl > 0 ) {
					int x0 = (int)(lower*timeScale());
					int h = 12-10*x0/xl;
					painter.drawLine(x-x0, y-h, x-x0, y+h);
				}

				if ( upper > 0 && xu > 0 ) {
					int x0 = (int)(upper*timeScale());
					int h = 12-10*x0/xu;
					painter.drawLine(x+x0, y-h, x+x0, y+h);
				}
			}
		}

	public:
		PickerView::Config::UncertaintyList uncertainties;

	private:
		bool                            crossHair;
		double                          maxLower, maxUpper;
		int                             currentIndex;
		SpectrogramRenderer             spectrogram[3];
		bool                            showSpectrogram;
		ThreeComponentTrace::Component *traces;
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
			//return new PickerLabel(item->widget());
			return new PickerRecordLabel;
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


class TraceShadow : public RecordWidgetDecorator {
	public:
		TraceShadow(QObject *parent, TimeScale *timeScale)
		: RecordWidgetDecorator(parent), _timeScale(timeScale) {}

		void drawDecoration(QPainter *painter, RecordWidget *widget) {
			int min_sel = (int)((_timeScale->minimumSelection()-widget->tmin())*widget->timeScale());
			int max_sel = (int)((_timeScale->maximumSelection()-widget->tmin())*widget->timeScale());

			if ( min_sel > 0 )
				painter->fillRect(0,0,min_sel,widget->height(), QColor(0,0,0,128));

			if ( max_sel < widget->width() )
				painter->fillRect(max_sel,0,widget->width()-max_sel,widget->height(), QColor(0,0,0,128));
		}

	private:
		TimeScale *_timeScale;
};


class PickerMarker : public RecordMarker {
	public:
		enum Type {
			UndefinedType, /* Something undefined */
			Arrival,       /* An associated pick */
			Pick,          /* Just a pick */
			Theoretical    /* A theoretical marker */
		};

	public:
		PickerMarker(RecordWidget *parent,
		             const Seiscomp::Core::Time& pos,
		             Type type, bool newPick)
		: RecordMarker(parent, pos),
		  _type(type),
		  _slot(-1), _rot(RT_Z12) {
			setMovable(newPick);
			init();
		}

		PickerMarker(RecordWidget *parent,
		             const Seiscomp::Core::Time& pos,
		             const QString& text,
		             Type type, bool newPick)
		: RecordMarker(parent, pos, text),
		  _type(type),
		  _slot(-1), _rot(RT_Z12) {
			setMovable(newPick);
			init();
		}

		PickerMarker(RecordWidget *parent,
		             const PickerMarker& m)
		: RecordMarker(parent, m),
		  _referencedPick(m._referencedPick),
		  _polarity(m._polarity),
		  _type(m._type),
		  _slot(m._slot),
		  _rot(m._rot)
		{
			init();
			_time = m._time;
		}

		virtual ~PickerMarker() {}


	private:
		void init() {
			setMoveCopy(false);
			updateVisual();
			_drawUncertaintyValues = false;
		}


	public:
		void setPhaseCode(const QString &code) {
			if ( pick() ) {
				QString text = code;
				if ( text.isEmpty() ) {
					try {
						text = pick()->phaseHint().code().c_str();
						setText(QString("%1"AUTOMATIC_POSTFIX).arg(text));
					}
					catch (...) {}
				}
				else
					setText(text);

				if ( !pick()->methodID().empty() ) {
					setDescription(QString("%1<%2>")
					               .arg(text)
					               .arg((char)toupper(pick()->methodID()[0])));
				}
				else
					setDescription(QString());
			}
			else if ( !code.isEmpty() )
				setText(code);
		}

		void setEnabled(bool enable) {
			RecordMarker::setEnabled(enable);
			updateVisual();
		}

		void setSlot(int s) {
			_slot = s;
		}

		int slot() const {
			return _slot;
		}

		void setRotation(int r) {
			_rot = r;
		}

		int rotation() const {
			return _rot;
		}

		void setFilter(const QString &filter) {
			_filter = filter;
		}

		const QString &filter() const {
			return _filter;
		}

		void setType(Type t) {
			_type = t;
			if ( _type == Pick ) {
				setPhaseCode("");
				setEnabled(true);
			}
			updateVisual();
		}

		Type type() const {
			return _type;
		}

		bool hasBeenAssociated() const {
			return id() >= 0;
		}

		void setPick(DataModel::Pick* p) {
			_referencedPick = p;
			try { _polarity = p->polarity(); }
			catch ( ... ) { _polarity = Core::None; }

			_time = p->time();

			updateVisual();
		}

		void convertToManualPick() {
			if ( !_referencedPick ) return;
			_referencedPick = NULL;
			setMovable(true);
			setDescription("");
			updateVisual();
		}

		DataModel::Pick* pick() const {
			return _referencedPick.get();
		}

		bool equalsPick(DataModel::Pick *pick) const {
			if ( pick == NULL ) return false;

			OPT(PickPolarity) pol;
			try { pol = pick->polarity(); } catch ( ... ) {}

			// Polarities do not match: not equal
			if ( pol != _polarity ) return false;

			// Time + uncertainties do not match: not equal
			if ( _time != pick->time() ) return false;

			return true;
		}

		void setPolarity(OPT(PickPolarity) p) {
			_polarity = p;
		}

		OPT(PickPolarity) polarity() const {
			return _polarity;
		}

		void setUncertainty(double lower, double upper) {
			if ( lower == upper ) {
				if ( lower >= 0 )
					_time.setUncertainty(lower);
				else
					_time.setUncertainty(Core::None);

				_time.setLowerUncertainty(Core::None);
				_time.setUpperUncertainty(Core::None);
			}
			else {
				_time.setUncertainty(Core::None);

				if ( lower >= 0 )
					_time.setLowerUncertainty(lower);
				else
					_time.setLowerUncertainty(Core::None);

				if ( upper >= 0 )
					_time.setUpperUncertainty(upper);
				else
					_time.setUpperUncertainty(Core::None);
			}
		}

		double lowerUncertainty() const {
			try {
				return _time.lowerUncertainty();
			}
			catch ( ... ) {
				try {
					return _time.uncertainty();
				}
				catch ( ... ) {}
			}

			return -1;
		}

		double upperUncertainty() const {
			try {
				return _time.upperUncertainty();
			}
			catch ( ... ) {
				try {
					return _time.uncertainty();
				}
				catch ( ... ) {}
			}

			return -1;
		}

		bool hasUncertainty() const {
			return lowerUncertainty() >= 0 && upperUncertainty() >= 0;
		}

		void setDrawUncertaintyValues(bool e) {
			_drawUncertaintyValues = e;
		}


		bool isArrival() const {
			return _type == Arrival;
		}

		bool isPick() const {
			return _type == Pick;
		}

		bool isTheoretical() const {
			return _type == Theoretical;
		}

		RecordMarker *copy() { return new PickerMarker(NULL, *this); }

		void draw(QPainter &painter, RecordWidget *context, int x, int y1, int y2,
		          QColor color, qreal lineWidth) {
			static QPoint poly[3];

			double loUncert = lowerUncertainty();
			double hiUncert = upperUncertainty();

			if ( loUncert > 0 || hiUncert > 0 ) {
				QColor barColor(color);
				barColor.setAlpha(64);
				int l = (int)(std::max(loUncert,0.0)*context->timeScale());
				int h = (int)(std::max(hiUncert,0.0)*context->timeScale());

				painter.fillRect(x-l,0,l+h+1,context->height(), barColor);

				if ( _drawUncertaintyValues && context->markerSourceWidget() ) {
					QString str;
					QRect rct;

					QFont font = painter.font();
					font.setBold(false);
					painter.setFont(font);

					if ( loUncert >= 0 ) {
						str.setNum(loUncert, 'G', 4);
						rct = painter.fontMetrics().boundingRect(str);
						rct.adjust(0,0,4,4);
						rct.moveBottomRight(QPoint(x-l,context->height()-1));
						painter.fillRect(rct, context->palette().color(QPalette::Window));
						painter.setPen(context->palette().color(QPalette::WindowText));
						painter.drawRect(rct);
						painter.drawText(rct, Qt::AlignCenter, str);
					}

					if ( hiUncert >= 0 ) {
						str.setNum(hiUncert, 'G', 4);
						rct = painter.fontMetrics().boundingRect(str);
						rct.adjust(0,0,4,4);
						rct.moveBottomLeft(QPoint(x+h,context->height()-1));
						painter.fillRect(rct, context->palette().color(QPalette::Window));
						painter.setPen(context->palette().color(QPalette::WindowText));
						painter.drawRect(rct);
						painter.drawText(rct, Qt::AlignCenter, str);
					}
				}
			}

			painter.setPen(QPen(color, lineWidth));
			painter.drawLine(x, y1, x, y2);

			if ( _polarity ) {
				int fontSize = painter.font().pixelSize();
				y1 += fontSize+2;

				int height = y2-y1+1;

				int h = std::min(height, std::max(8,std::min(24,height*30/100)));
				int w = h*9/32;

				switch ( *_polarity ) {
					case POSITIVE:
					{
						bool hasAA = painter.renderHints() & QPainter::Antialiasing;
						painter.setRenderHint(QPainter::Antialiasing, true);
						poly[0] = QPoint(x,y1);
						poly[1] = QPoint(x+w,y1+h);
						poly[2] = QPoint(x-w,y1+h);
						painter.setBrush(color);
						painter.drawPolygon(poly, 3);
						painter.setBrush(Qt::NoBrush);
						painter.setRenderHint(QPainter::Antialiasing, hasAA);
						break;
					}
					case NEGATIVE:
					{
						bool hasAA = painter.renderHints() & QPainter::Antialiasing;
						painter.setRenderHint(QPainter::Antialiasing, true);
						poly[0] = QPoint(x,y1+h);
						poly[1] = QPoint(x-w,y1);
						poly[2] = QPoint(x+w,y1);
						painter.setBrush(color);
						painter.drawPolygon(poly, 3);
						painter.setBrush(Qt::NoBrush);
						painter.setRenderHint(QPainter::Antialiasing, hasAA);
						break;
					}
					case UNDECIDABLE:
						painter.save();
						{
							QFont f = painter.font();
							f.setPixelSize(h);
							f.setBold(true);
							painter.setFont(f);
							painter.drawText(x+2, y1+h, "X");
						}
						painter.restore();
						break;
					default:
						break;
				}
			}
		}

		QString toolTip() const {
			if ( _type != Pick && _type != Arrival )
				return RecordMarker::toolTip();

			if ( _referencedPick == NULL )
				return QString("manual %1 pick (local)\n"
				               "filter: %2\n"
				               "arrival: %3")
				       .arg(text())
				       .arg(_filter.isEmpty()?"None":_filter)
				       .arg(isArrival()?"yes":"no");

			QString text;

			try {
				switch ( _referencedPick->evaluationMode() ) {
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

			try {
				text += _referencedPick->phaseHint().code().c_str();
				text += " ";
			}
			catch ( ... ) {
				text += "?? ";
			}

			text += "pick";

			try {
				text += QString(" created by %1").arg(_referencedPick->creationInfo().author().c_str());
			}
			catch ( ... ) {}

			try {
				text += QString(" at %1").arg(_referencedPick->creationInfo().creationTime().toString("%F %T").c_str());
			}
			catch ( ... ) {}

			if ( !_referencedPick->methodID().empty() )
				text += QString("\nmethod: %1").arg(_referencedPick->methodID().c_str());
			if ( !_referencedPick->filterID().empty() )
				text += QString("\nfilter: %1").arg(_referencedPick->filterID().c_str());

			text += QString("\narrival: %1").arg(isArrival()?"yes":"no");

			return text;
		}

	private:
		void updateVisual() {
			QColor col = SCScheme.colors.picks.disabled;
			Qt::Alignment al = Qt::AlignVCenter;
			EvaluationMode state = AUTOMATIC;

			DataModel::Pick *p = pick();
			if ( p ) {
				try { state = p->evaluationMode(); } catch ( ... ) {}
			}

			if ( isMovable() )
				state = MANUAL;

			switch ( _type ) {
				case Arrival:
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

					al = isMovable()?Qt::AlignVCenter:Qt::AlignTop;
					break;

				case Pick:
					if ( isEnabled() ) {
						switch ( state ) {
							case MANUAL:
								col = SCScheme.colors.picks.manual;
								break;
							case AUTOMATIC:
							default:
								col = SCScheme.colors.picks.automatic;
								break;
						}
					}
					else
						col = SCScheme.colors.picks.disabled;

					al = Qt::AlignTop;
					break;

				case Theoretical:
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
		PickPtr           _referencedPick;
		OPT(PickPolarity) _polarity;
		TimeQuantity      _time;
		QString           _filter;
		Type              _type;
		int               _slot;
		int               _rot;
		bool              _drawUncertaintyValues;
};


bool isTraceUsed(Seiscomp::Gui::RecordWidget* w) {
	for ( int i = 0; i < w->markerCount(); ++i ) {
		PickerMarker* m = static_cast<PickerMarker*>(w->marker(i));
		if ( !m->isEnabled() ) continue;
		if ( m->type() == PickerMarker::Arrival ) return true;
	}

	return false;
}


bool isTracePicked(Seiscomp::Gui::RecordWidget* w) {
	for ( int i = 0; i < w->markerCount(); ++i ) {
		PickerMarker* m = static_cast<PickerMarker*>(w->marker(i));
		if ( m->type() == PickerMarker::Arrival ) return true;
	}

	return false;
}


bool isArrivalTrace(Seiscomp::Gui::RecordWidget* w) {
	for ( int i = 0; i < w->markerCount(); ++i ) {
		PickerMarker* m = static_cast<PickerMarker*>(w->marker(i));
		if ( m->pick() && m->id() >= 0 ) return true;
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

			//Sensor *sensor = Sensor::Find(stream->sensor());
			//if ( !sensor ) continue;

			Processing::WaveformProcessor::SignalUnit unit;

			// Unable to retrieve the unit enumeration from string
			//if ( !unit.fromString(sensor->unit().c_str()) ) continue;
			if ( !unit.fromString(stream->gainUnit().c_str()) ) continue;
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


Pick* findNonManualPick(Seiscomp::Gui::RecordWidget* w, const Seiscomp::Core::Time& t, const QString& /*phase*/) {
	for ( int i = 0; i < w->markerCount(); ++i ) {
		PickerMarker* m = static_cast<PickerMarker*>(w->marker(i));
		Pick* p = m->pick();
		if ( p && m->isPick()/* && (m->text() == phase || m->text().isEmpty())*/ ) {
			if ( p->time() == t )
				return p;
		}
	}

	return NULL;
}


Pick* findPick(Seiscomp::Gui::RecordWidget* w, const Seiscomp::Core::Time& t) {
	for ( int i = 0; i < w->markerCount(); ++i ) {
		PickerMarker* m = static_cast<PickerMarker*>(w->marker(i));
		Pick* p = m->pick();
		if ( p && p->time() == t )
			return p;
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
	return static_cast<PickerRecordLabel*>(item->label())->isLinkedItem();
}

void unlinkItem(RecordViewItem *item) {
	static_cast<PickerRecordLabel*>(item->label())->unlink();
	item->disconnect(SIGNAL(firstRecordAdded(const Seiscomp::Record*)));
}


void selectFirstVisibleItem(RecordView *view) {
	for ( int i = 0; i < view->rowCount(); ++i ) {
		if ( !static_cast<PickerRecordLabel*>(view->itemAt(i)->label())->isLinkedItem() ) {
			view->setCurrentItem(view->itemAt(i));
			view->ensureVisible(i);
			break;
		}
	}
}


#define CFG_LOAD_PICKS _ui.actionShowUnassociatedPicks->isChecked()


}


namespace Seiscomp {
namespace Gui {
namespace PrivatePickerView {


ThreeComponentTrace::ThreeComponentTrace() {
	widget = NULL;
	enableTransformation = false;
	enableL2Horizontals = false;

	for ( int i = 0; i < 3; ++i ) {
		traces[i].raw = NULL;
		traces[i].transformed = NULL;
		traces[i].thread = NULL;
		traces[i].filter = NULL;
	}
}


ThreeComponentTrace::~ThreeComponentTrace() {
	for ( int i = 0; i < 3; ++i ) {
		if ( traces[i].raw ) delete traces[i].raw;
		if ( widget ) widget->setRecords(i, NULL);
		if ( traces[i].transformed ) delete traces[i].transformed;
	}
}


void ThreeComponentTrace::reset() {
	for ( int i = 0; i < 3; ++i ) {
		traces[i].filter.reset();

		// Delete transformed traces
		if ( traces[i].transformed ) {
			delete traces[i].transformed;
			traces[i].transformed = NULL;

			if ( widget ) widget->setRecords(i, NULL);
		}
	}

	transform();
}


void ThreeComponentTrace::setFilter(RecordWidget::Filter *f) {
	for ( int i = 0; i < 3; ++i ) {
		traces[i].filter = f?f->clone():NULL;

		// Delete transformed traces
		if ( traces[i].transformed ) {
			delete traces[i].transformed;
			traces[i].transformed = NULL;

			if ( widget ) widget->setRecords(i, NULL);
		}
	}

	transform();
}


void ThreeComponentTrace::setRecordWidget(RecordWidget *w) {
	if ( widget ) {
		for ( int i = 0; i < 3; ++i ) widget->setRecords(i, NULL);
		widget->disconnect(this);
	}

	widget = w;

	if ( widget ) {
		for ( int i = 0; i < 3; ++i )
			widget->setRecords(i, traces[i].transformed, false);
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
		traces[i].filter.reset();

		// Delete transformed traces
		if ( traces[i].transformed ) {
			delete traces[i].transformed;
			traces[i].transformed = NULL;

			if ( widget ) widget->setRecords(i, NULL);
		}
	}

	transform();
}


void ThreeComponentTrace::setL2Horizontals(bool f) {
	enableL2Horizontals = f;
}


bool ThreeComponentTrace::transform(int comp, Seiscomp::Record *rec) {
	Core::Time minStartTime;
	Core::Time maxStartTime;
	Core::Time minEndTime;
	bool gotRecords = false;

	if ( enableTransformation ) {
		// Not all traces available, nothing to do
		for ( int i = 0; i < 3; ++i ) {
			if ( traces[i].transformed ) {
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

			// Trim record sizes
			for ( int i = 0; i < 3; ++i ) {
				FloatArray *data = static_cast<FloatArray*>(comps[i]->data());
				if ( data->size() > minLen ) {
					data->resize(minLen);
					comps[i]->dataUpdated();
				}
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

			if ( enableL2Horizontals ) {
				float *data1 = static_cast<FloatArray*>(comps[1]->data())->typedData();
				float *data2 = static_cast<FloatArray*>(comps[2]->data())->typedData();

				for ( int i = 0; i < minLen; ++i ) {
					float rms = sqrt(*data1 * *data1 + *data2 * *data2);
					*data1 = rms;
					*data2 = 0;

					++data1; ++data2;
				}
			}

			// And filter
			for ( int i = 0; i < 3; ++i )
				traces[i].filter.apply(comps[i].get());

			// Create record sequences
			for ( int i = 0; i < 3; ++i ) {
				// Create ring buffer without limit if needed
				if ( traces[i].transformed == NULL ) {
					traces[i].transformed = new RingBuffer(0);
					if ( widget ) widget->setRecords(i, traces[i].transformed, false);
				}

				traces[i].transformed->feed(comps[i].get());
				if ( widget ) widget->fed(i, comps[i].get());
			}

			minStartTime = minEndTime;
		}
	}
	else {
		// Record passed that needs filtering?
		if ( rec ) {
			if ( traces[comp].transformed == NULL ) {
				traces[comp].transformed = new RingBuffer(0);
				if ( widget ) widget->setRecords(comp, traces[comp].transformed, false);
			}

			RecordPtr grec = traces[comp].filter.feed(rec);

			if ( grec ) {
				traces[comp].transformed->feed(grec.get());
				if ( widget ) widget->fed(comp, grec.get());
				gotRecords = true;
			}
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

					const Record *s_rec = s_it->get();

					if ( traces[i].transformed == NULL ) {
						traces[i].transformed = new RingBuffer(0);
						if ( widget ) widget->setRecords(i, traces[i].transformed, false);
					}

					RecordPtr grec = traces[i].filter.feed(s_rec);
					if ( grec ) {
						traces[i].transformed->feed(grec.get());
						if ( widget ) widget->fed(i, grec.get());
						gotRecords = true;
					}
				}
			}
		}
	}

	return gotRecords;
}


PickerRecordLabel::PickerRecordLabel(int items, QWidget *parent, const char* name)
	: StandardRecordLabel(items, parent, name), _isLinkedItem(false), _isExpanded(false) {
	_btnExpand = NULL;
	_linkedItem = NULL;

	latitude = 999;
	longitude = 999;

	hasGotData = false;
	isEnabledByConfig = false;
}

PickerRecordLabel::~PickerRecordLabel() {}

void PickerRecordLabel::setLinkedItem(bool li) {
	_isLinkedItem = li;
}


void PickerRecordLabel::setControlledItem(RecordViewItem *controlledItem) {
	_linkedItem = controlledItem;
	static_cast<PickerRecordLabel*>(controlledItem->label())->_linkedItem = recordViewItem();
}

RecordViewItem *PickerRecordLabel::controlledItem() const {
	return _linkedItem;
}

void PickerRecordLabel::enabledExpandButton(RecordViewItem *controlledItem) {
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

void PickerRecordLabel::disableExpandButton() {
	if ( _btnExpand ) {
		delete _btnExpand;
		_btnExpand = NULL;
	}

	_linkedItem = NULL;
}

void PickerRecordLabel::unlink() {
	if ( _linkedItem ) {
		static_cast<PickerRecordLabel*>(_linkedItem->label())->disableExpandButton();
		_linkedItem = NULL;
	}
}

bool PickerRecordLabel::isLinkedItem() const {
	return _isLinkedItem;
}

bool PickerRecordLabel::isExpanded() const {
	return _isExpanded;
}


void PickerRecordLabel::visibilityChanged(bool v) {
	if ( _linkedItem && !_isLinkedItem ) {
		if ( !v )
			_linkedItem->setVisible(false);
		else if ( _isExpanded )
			_linkedItem->setVisible(true);
	}
}

void PickerRecordLabel::resizeEvent(QResizeEvent *e) {
	StandardRecordLabel::resizeEvent(e);
	if ( _btnExpand ) {
		_btnExpand->move(e->size().width() - _btnExpand->width(),
		                 e->size().height() - _btnExpand->height());
	}
}

void PickerRecordLabel::paintEvent(QPaintEvent *e) {
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

void PickerRecordLabel::enableExpandable(const Seiscomp::Record *rec) {
	enabledExpandButton(static_cast<RecordViewItem*>(sender()));
}

void PickerRecordLabel::extentButtonPressed() {
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

void PickerRecordLabel::setLabelColor(QColor c) {
	_labelColor = c;
	_hasLabelColor = true;
	update();
}


void PickerRecordLabel::removeLabelColor() {
	_hasLabelColor = false;
	update();
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickerView::Config::Config() {
	timingQualityLow = Qt::darkRed;
	timingQualityMedium = Qt::yellow;
	timingQualityHigh = Qt::darkGreen;

	defaultDepth = 10;
	alignmentPosition = 0.5;
	offsetWindowStart = 0;
	offsetWindowEnd = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::Config::getPickPhases(StringList &phases) const {
	getPickPhases(phases, phaseGroups);
	foreach ( const QString &ph, favouritePhases ) {
		if ( !phases.contains(ph) ) phases.append(ph);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::Config::getPickPhases(StringList &phases, const QList<PhaseGroup> &groups) const {
	foreach ( const PhaseGroup &g, groups ) {
		if ( g.childs.empty() ) {
			if ( !phases.contains(g.name) ) phases.append(g.name);
		}
		else
			getPickPhases(phases, g.childs);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickerView::PickerView(QWidget *parent, Qt::WFlags f)
: QMainWindow(parent,f) {
	_recordView = new TraceList();
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickerView::PickerView(const Seiscomp::Core::TimeWindow& tw,
                       QWidget *parent, Qt::WFlags f)
: QMainWindow(parent, f) {
	_recordView = new TraceList(tw);
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickerView::PickerView(const Seiscomp::Core::TimeSpan& ts,
                       QWidget *parent, Qt::WFlags f)
: QMainWindow(parent, f) {
	_recordView = new TraceList(ts);
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PickerView::~PickerView() {
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
RecordLabel* PickerView::createLabel(RecordViewItem *item) const {
	//return new PickerLabel(item->widget());
	return new PickerRecordLabel;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::init() {
	setObjectName("Picker");

#ifdef MACOSX
	Mac::addFullscreen(this);
#endif

	_ui.setupUi(this);

	_ui.labelStationCode->setFont(SCScheme.fonts.heading3);
	_ui.labelCode->setFont(SCScheme.fonts.normal);

	//setContextMenuPolicy(Qt::ActionsContextMenu);
	//_recordView->setMinimumRowHeight(70);

	//_ttTable.setBranch("P");

	_currentRotationMode = RT_Z12;
	_settingsRestored = false;
	_currentSlot = -1;
	_currentFilter = NULL;
	_autoScaleZoomTrace = true;
	_loadedPicks = false;

	_reader = NULL;

	_zoom = 1.0;
	_currentAmplScale = 1.0;

	_centerSelection = false;
	_checkVisibility = true;

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

	_connectionState = new ConnectionStateLabel(this);
	connect(_connectionState, SIGNAL(customInfoWidgetRequested(const QPoint &)),
	        this, SLOT(openConnectionInfo(const QPoint &)));

	QBoxLayout* layout = new QVBoxLayout(_ui.framePickList);
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

	_currentRecord = new ZoomRecordWidget();
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

	connect(_currentRecord, SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(openRecordContextMenu(const QPoint &)));
	connect(_currentRecord, SIGNAL(currentMarkerChanged(Seiscomp::Gui::RecordMarker*)),
	        this, SLOT(currentMarkerChanged(Seiscomp::Gui::RecordMarker*)));

	layout = new QVBoxLayout(_ui.frameCurrentRow);
	layout->setMargin(0);
	layout->setSpacing(0);

	_timeScale = new TimeScale();
	_timeScale->setSelectionEnabled(false);
	_timeScale->setSelectionHandleCount(2);
	_timeScale->setAbsoluteTimeEnabled(true);
	_timeScale->setRangeSelectionEnabled(true);

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
	connect(_timeScale, SIGNAL(selectionHandleMoved(int, double, Qt::KeyboardModifiers)),
	        this, SLOT(zoomSelectionHandleMoved(int, double, Qt::KeyboardModifiers)));
	connect(_timeScale, SIGNAL(selectionHandleMoveFinished()),
	        this, SLOT(zoomSelectionHandleMoveFinished()));

	connect(_recordView->timeWidget(), SIGNAL(dragged(double)),
	        this, SLOT(moveTraces(double)));

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
	addAction(_ui.actionSortByAzimuth);
	addAction(_ui.actionSortByResidual);

	addAction(_ui.actionShowZComponent);
	addAction(_ui.actionShowNComponent);
	addAction(_ui.actionShowEComponent);

	addAction(_ui.actionAlignOnOriginTime);
	addAction(_ui.actionAlignOnPArrival);
	addAction(_ui.actionAlignOnSArrival);

	addAction(_ui.actionToggleFilter);
	addAction(_ui.actionNextFilter);
	addAction(_ui.actionPreviousFilter);
	addAction(_ui.actionMaximizeAmplitudes);

	addAction(_ui.actionPickP);
	addAction(_ui.actionPickS);
	addAction(_ui.actionDisablePicking);

	addAction(_ui.actionCreatePick);
	addAction(_ui.actionConfirmPick);
	addAction(_ui.actionSetPick);
	addAction(_ui.actionDeletePick);

	addAction(_ui.actionShowZComponent);
	addAction(_ui.actionShowNComponent);
	addAction(_ui.actionShowEComponent);

	addAction(_ui.actionRepickAutomatically);
	addAction(_ui.actionGotoNextMarker);
	addAction(_ui.actionGotoPreviousMarker);

	addAction(_ui.actionSetPolarityPositive);
	addAction(_ui.actionSetPolarityNegative);
	addAction(_ui.actionSetPolarityUndecidable);
	addAction(_ui.actionSetPolarityUnset);

	addAction(_ui.actionRelocate);
	addAction(_ui.actionSwitchFullscreen);
	addAction(_ui.actionAddStations);
	addAction(_ui.actionSearchStation);

	addAction(_ui.actionShowSpectrogram);

	_lastFilterIndex = 0;

	_comboFilter = new QComboBox;
	//_comboFilter->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	_comboFilter->setDuplicatesEnabled(false);
	_comboFilter->addItem(NO_FILTER_STRING);

	_comboFilter->setCurrentIndex(_lastFilterIndex);
	changeFilter(_comboFilter->currentIndex());

	_ui.toolBarFilter->insertWidget(_ui.actionToggleFilter, _comboFilter);

	_comboRotation = new QComboBox;
	//_comboRotation->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred);
	_comboRotation->setDuplicatesEnabled(false);
	for ( int i = 0; i < RotationType::Quantity; ++i )
		_comboRotation->addItem(ERotationTypeNames::name(i));
	_comboRotation->setCurrentIndex(0);

	_ui.toolBarFilter->insertWidget(_ui.actionToggleFilter, _comboRotation);

	connect(_ui.actionSetPolarityPositive, SIGNAL(triggered(bool)),
	        this, SLOT(setPickPolarity()));
	connect(_ui.actionSetPolarityNegative, SIGNAL(triggered(bool)),
	        this, SLOT(setPickPolarity()));
	connect(_ui.actionSetPolarityUndecidable, SIGNAL(triggered(bool)),
	        this, SLOT(setPickPolarity()));
	connect(_ui.actionSetPolarityUnset, SIGNAL(triggered(bool)),
	        this, SLOT(setPickPolarity()));

	connect(_comboFilter, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(changeFilter(int)));
	connect(_comboRotation, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(changeRotation(int)));

	connect(_ui.actionLimitFilterToZoomTrace, SIGNAL(triggered(bool)),
	        this, SLOT(limitFilterToZoomTrace(bool)));

	connect(_ui.actionShowTheoreticalArrivals, SIGNAL(triggered(bool)),
	        this, SLOT(showTheoreticalArrivals(bool)));
	connect(_ui.actionShowUnassociatedPicks, SIGNAL(triggered(bool)),
	        this, SLOT(showUnassociatedPicks(bool)));
	connect(_ui.actionShowSpectrogram, SIGNAL(triggered(bool)),
	        this, SLOT(showSpectrogram(bool)));

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

	QCheckBox *cb = new QCheckBox;
	cb->setObjectName("spec.log");
	cb->setText(tr("Logscale"));
	cb->setChecked(false);
	connect(cb, SIGNAL(toggled(bool)), this, SLOT(specLogToggled(bool)));
	specLogToggled(cb->isChecked());

	_ui.toolBarSpectrogram->addWidget(cb);

	QDoubleSpinBox *spinLower = new QDoubleSpinBox;
	spinLower->setMinimum(-100);
	spinLower->setMaximum(100);
	spinLower->setValue(-15);
	connect(spinLower, SIGNAL(valueChanged(double)), this, SLOT(specMinValue(double)));
	specMinValue(spinLower->value());

	_ui.toolBarSpectrogram->addSeparator();
	_ui.toolBarSpectrogram->addWidget(spinLower);

	QDoubleSpinBox *spinUpper = new QDoubleSpinBox;
	spinUpper->setMinimum(-100);
	spinUpper->setMaximum(100);
	spinUpper->setValue(-5);
	connect(spinUpper, SIGNAL(valueChanged(double)), this, SLOT(specMaxValue(double)));
	specMaxValue(spinUpper->value());

	_ui.toolBarSpectrogram->addSeparator();
	_ui.toolBarSpectrogram->addWidget(spinUpper);

	QDoubleSpinBox *spinTW = new QDoubleSpinBox;
	spinTW->setMinimum(0.1);
	spinTW->setMaximum(600);
	spinTW->setValue(5);
	spinTW->setSuffix("s");
	connect(spinTW, SIGNAL(valueChanged(double)), this, SLOT(specTimeWindow(double)));
	specTimeWindow(spinTW->value());

	_ui.toolBarSpectrogram->addSeparator();
	_ui.toolBarSpectrogram->addWidget(spinTW);

	// connect actions
	connect(_ui.actionDefaultView, SIGNAL(triggered(bool)),
	        this, SLOT(setDefaultDisplay()));
	connect(_ui.actionSortAlphabetically, SIGNAL(triggered(bool)),
	        this, SLOT(sortAlphabetically()));
	connect(_ui.actionSortByDistance, SIGNAL(triggered(bool)),
	        this, SLOT(sortByDistance()));
	connect(_ui.actionSortByAzimuth, SIGNAL(triggered(bool)),
	        this, SLOT(sortByAzimuth()));
	connect(_ui.actionSortByResidual, SIGNAL(triggered(bool)),
	        this, SLOT(sortByResidual()));

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
	connect(_ui.actionAlignOnSArrival, SIGNAL(triggered(bool)),
	        this, SLOT(alignOnSArrivals()));

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
	connect(_ui.actionRepickAutomatically, SIGNAL(triggered(bool)),
	        this, SLOT(automaticRepick()));
	connect(_ui.actionGotoNextMarker, SIGNAL(triggered(bool)),
	        this, SLOT(gotoNextMarker()));
	connect(_ui.actionGotoPreviousMarker, SIGNAL(triggered(bool)),
	        this, SLOT(gotoPreviousMarker()));
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
	connect(_ui.actionNextFilter, SIGNAL(triggered(bool)),
	        this, SLOT(nextFilter()));
	connect(_ui.actionPreviousFilter, SIGNAL(triggered(bool)),
	        this, SLOT(previousFilter()));

	connect(_ui.actionMaximizeAmplitudes, SIGNAL(triggered(bool)),
	        this, SLOT(scaleVisibleAmplitudes()));

	connect(_ui.actionDisablePicking, SIGNAL(triggered(bool)),
	        this, SLOT(pickNone(bool)));
	connect(_ui.actionDisablePicking, SIGNAL(triggered(bool)),
	        this, SLOT(abortSearchStation()));
	connect(_ui.actionPickP, SIGNAL(triggered(bool)),
	        this, SLOT(pickP(bool)));
	connect(_ui.actionPickS, SIGNAL(triggered(bool)),
	        this, SLOT(pickS(bool)));

	connect(_ui.actionCreatePick, SIGNAL(triggered(bool)),
	        this, SLOT(createPick()));
	connect(_ui.actionSetPick, SIGNAL(triggered(bool)),
	        this, SLOT(setPick()));
	connect(_ui.actionConfirmPick, SIGNAL(triggered(bool)),
	        this, SLOT(confirmPick()));
	connect(_ui.actionDeletePick, SIGNAL(triggered(bool)),
	        this, SLOT(deletePick()));

	connect(_ui.actionRelocate, SIGNAL(triggered(bool)),
	        this, SLOT(relocate()));

	connect(_ui.actionModifyOrigin, SIGNAL(triggered(bool)),
	        this, SLOT(modifyOrigin()));

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
	        this, SLOT(confirmPick()));
	connect(_ui.btnRowRemove, SIGNAL(clicked(bool)),
	        this, SLOT(setCurrentRowDisabled(bool)));
	connect(_ui.btnRowRemove, SIGNAL(clicked(bool)),
	        _recordView, SLOT(selectNextRow()));
	connect(_ui.btnRowReset, SIGNAL(clicked(bool)),
	        this, SLOT(resetPick()));
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
	/*
	connect(_recordView, SIGNAL(cursorTextChanged(const QString&)),
	        _currentRecord, SLOT(setCursorText(const QString&)));
	*/

	_actionsUncertainty = NULL;
	_actionsPickGroupPhases = NULL;
	_actionsPickFavourites = NULL;
	_actionsAlignOnFavourites = NULL;
	_actionsAlignOnGroupPhases = NULL;

	_minTime = -_config.minimumTimeWindow;
	_maxTime = _config.minimumTimeWindow;

	/*
	pal = palette();
	pal.setColor(QPalette::Highlight, QColor(224,255,224));
	//pal.setColor(QPalette::Light, QColor(240,255,240));
	_recordView->setPalette(pal);
	*/

	/*
	//_wantedPhases.push_back("Pg");
	//_wantedPhases.push_back("Pn");
	_wantedPhases.push_back("P");
	//_wantedPhases.push_back("pP");
	_wantedPhases.push_back("S");
	//_wantedPhases.push_back("sS");
	*/

	/*
	_broadBandCodes.push_back("BH");
	_broadBandCodes.push_back("SH");
	_broadBandCodes.push_back("HH");

	_strongMotionCodes.push_back("BL");
	_strongMotionCodes.push_back("SL");
	_strongMotionCodes.push_back("HL");
	_strongMotionCodes.push_back("HN");
	_strongMotionCodes.push_back("LN");
	*/

	connect(_recordView, SIGNAL(selectedTime(Seiscomp::Gui::RecordWidget*, Seiscomp::Core::Time)),
	        this, SLOT(onSelectedTime(Seiscomp::Gui::RecordWidget*, Seiscomp::Core::Time)));

	connect(_currentRecord, SIGNAL(selectedTime(Seiscomp::Core::Time)),
	        this, SLOT(onSelectedTime(Seiscomp::Core::Time)));

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

	Processing::PickerFactory::ServiceNames *pickers = Processing::PickerFactory::Services();
	if ( pickers ) {
		std::sort(pickers->begin(), pickers->end());
		_comboPicker = new QComboBox(this);
		Processing::PickerFactory::ServiceNames::iterator it;
		for ( it = pickers->begin(); it != pickers->end(); ++it )
			_comboPicker->addItem(it->c_str());

		_ui.toolBarPicking->addSeparator();
		_ui.toolBarPicking->addWidget(_comboPicker);
		delete pickers;
	}
	else
		_comboPicker = NULL;

	_ui.menuPicking->addAction(_ui.actionGotoNextMarker);
	_ui.menuPicking->addAction(_ui.actionGotoPreviousMarker);
	_ui.menuPicking->addSeparator();
	_ui.menuPicking->addAction(_ui.actionDisablePicking);
	_ui.menuPicking->addAction(_ui.actionPickP);
	_ui.menuPicking->addAction(_ui.actionPickS);

	/*
	QDockWidget *dock = new QDockWidget(tr("Filter picks"), this);
	dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

	_pickInfoList = new QWidget(dock);
	QVBoxLayout *l = new QVBoxLayout;
	_pickInfoList->setLayout(l);
	dock->setWidget(_pickInfoList);
	addDockWidget(Qt::RightDockWidgetArea, dock);

	QMenu *viewWindows = _ui.menuView->addMenu(tr("Windows"));
	viewWindows->addAction(dock->toggleViewAction());
	*/

	initPhases();

	PickerMarkerActionPluginFactory::ServiceNames *plugins = PickerMarkerActionPluginFactory::Services();
	if ( plugins != NULL ) {
		PickerMarkerActionPluginFactory::ServiceNames::iterator it;
		for ( it = plugins->begin(); it != plugins->end(); ++it ) {
			PickerMarkerActionPlugin *plugin = PickerMarkerActionPluginFactory::Create(it->c_str());
			if ( plugin != NULL ) {
				plugin->setParent(this);
				_markerPlugins.append(plugin);
			}
		}

		delete plugins;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

void createPhaseMenus(QActionGroup *actionGroup, QList<QMenu*> &menus,
                      const PickerView::Config::GroupList &list,
                      QMenu *root = NULL, int depth = 0)
{
	QMenu *actionRoot = depth == 0?NULL:root;

	foreach ( const PickerView::Config::PhaseGroup &group, list ) {
		if ( group.childs.empty() ) {
			if ( actionRoot == NULL ) {
				if ( root == NULL ) {
					actionRoot = new QMenu(group.name);
					menus.append(actionRoot);
				}
				else
					actionRoot = root->addMenu("unnamed");

				// Store top-level menus
				if ( depth == 0 ) menus.append(actionRoot);
			}

			QAction *action = new QAction(group.name, actionGroup);
			actionRoot->addAction(action);
		}
		else {
			QMenu *subMenu;

			if ( root == NULL )
				subMenu = new QMenu(group.name);
			else
				subMenu = root->addMenu(group.name);

			// Store top-level menus
			if ( depth == 0 ) menus.append(subMenu);

			createPhaseMenus(actionGroup, menus, group.childs, subMenu, depth+1);
		}
	}
}


void createAlignPhaseMenus(QActionGroup *actionGroup, QList<QMenu*> &menus,
                           const PickerView::Config::GroupList &list,
                           QMenu *root = NULL, int depth = 0)
{
	QMenu *actionRoot = depth == 0?NULL:root;

	foreach ( const PickerView::Config::PhaseGroup &group, list ) {
		if ( group.childs.empty() ) {
			if ( actionRoot == NULL ) {
				if ( root == NULL ) {
					actionRoot = new QMenu(group.name);
					menus.append(actionRoot);
				}
				else
					actionRoot = root->addMenu("unnamed");

				// Store top-level menus
				if ( depth == 0 ) menus.append(actionRoot);
			}

			QAction *action = new QAction(group.name, actionGroup);
			// Flag as phase
			action->setData(QVariant(false));
			actionRoot->addAction(action);

			action = new QAction(QString("%1 (ttt)").arg(group.name), actionGroup);
			// Flag as theoretical phase
			action->setData(QVariant(true));
			actionRoot->addAction(action);
		}
		else {
			QMenu *subMenu;

			if ( root == NULL )
				subMenu = new QMenu(group.name);
			else
				subMenu = root->addMenu(group.name);

			// Store top-level menus
			if ( depth == 0 ) menus.append(subMenu);

			createAlignPhaseMenus(actionGroup, menus, group.childs, subMenu, depth+1);
		}
	}
}


}


void PickerView::initPhases() {
	_phases.clear();
	_showPhases.clear();

	// Remove pick phase group submenus
	foreach ( QMenu *menu, _menusPickGroups )
		delete menu;
	_menusPickGroups.clear();

	// Remove align phase group submenus
	foreach ( QMenu *menu, _menusAlignGroups )
		delete menu;
	_menusAlignGroups.clear();

	// Remove pick group phases
	if ( _actionsPickGroupPhases ) {
		delete _actionsPickGroupPhases;
		_actionsPickGroupPhases = NULL;
	}

	// Remove align group phases
	if ( _actionsAlignOnGroupPhases ) {
		delete _actionsAlignOnGroupPhases;
		_actionsAlignOnGroupPhases = NULL;
	}

	// Remove pick favourite phases
	if ( _actionsPickFavourites ) {
		delete _actionsPickFavourites;
		_actionsPickFavourites = NULL;
	}

	// Remove align favourite phases
	if ( _actionsAlignOnFavourites ) {
		delete _actionsAlignOnFavourites;
		_actionsAlignOnFavourites = NULL;
	}

	// Create favourite phase actions + shortcuts
	if ( !_config.favouritePhases.empty() ) {
		_ui.menuPicking->addSeparator();
		_ui.menuAlignArrival->addSeparator();

		_actionsPickFavourites = new QActionGroup(this);
		_actionsAlignOnFavourites = new QActionGroup(this);

		if ( _config.favouritePhases.count() > 9 )
			SEISCOMP_WARNING("More than 9 favourite phases defined: shortcuts are only "
			                 "assigned to the first 9 phases");

		int i = 0;
		foreach ( const QString &ph, _config.favouritePhases ) {
			QAction *pickAction = new QAction(_actionsPickFavourites);
			pickAction->setText(ph);

			QAction *alignAction = new QAction(_actionsAlignOnFavourites);
			alignAction->setText(ph);

			QAction *alignTheoreticalAction = new QAction(_actionsAlignOnFavourites);
			alignTheoreticalAction->setText(QString("%1 (ttt)").arg(ph));

			// Set flag to use pick time (theoretical == false)
			alignAction->setData(QVariant(false));
			// Set flag to use theoretical pick time (theoretical == true)
			alignTheoreticalAction->setData(QVariant(true));

			if ( i < 9 ) {
				pickAction->setShortcut(Qt::Key_1 + i);
				alignAction->setShortcut(Qt::CTRL + Qt::Key_1 + i);
				alignTheoreticalAction->setShortcut(Qt::CTRL + Qt::SHIFT + Qt::Key_1 + i);
			}

			_ui.menuPicking->addAction(pickAction);
			_ui.menuAlignArrival->addAction(alignAction);
			_ui.menuAlignArrival->addAction(alignTheoreticalAction);

			++i;
		}

		connect(_actionsPickFavourites, SIGNAL(triggered(QAction*)),
		        this, SLOT(setPickPhase(QAction*)));

		connect(_actionsAlignOnFavourites, SIGNAL(triggered(QAction*)),
		        this, SLOT(alignOnPhase(QAction*)));
	}

	if ( !_config.phaseGroups.empty() ) {
		_ui.menuPicking->addSeparator();

		_actionsPickGroupPhases = new QActionGroup(this);

		createPhaseMenus(_actionsPickGroupPhases, _menusPickGroups,
		                 _config.phaseGroups, _ui.menuPicking);

		connect(_actionsPickGroupPhases, SIGNAL(triggered(QAction*)),
		        this, SLOT(setPickPhase(QAction*)));


		_ui.menuAlignArrival->addSeparator();
		_actionsAlignOnGroupPhases = new QActionGroup(this);

		createAlignPhaseMenus(_actionsAlignOnGroupPhases, _menusAlignGroups,
		                      _config.phaseGroups, _ui.menuAlignArrival);

		connect(_actionsAlignOnGroupPhases, SIGNAL(triggered(QAction*)),
		        this, SLOT(alignOnPhase(QAction*)));
	}


	QSet<QString> phases;
	phases.insert("P");
	phases.insert("S");

	if ( _actionsPickGroupPhases ) {
		foreach ( QAction *action, _actionsPickGroupPhases->actions() )
			phases.insert(action->text());
	}

	_phases = phases.toList();

	foreach ( const QString &ph, _config.showPhases ) {
		if ( !phases.contains(ph) ) {
			_showPhases.append(ph);
			phases.insert(ph);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::setConfig(const Config &c, QString *error) {
	_config = c;

	_uncertainties.clear();

	Config::UncertaintyProfiles::iterator it;
	it = _config.uncertaintyProfiles.find(_config.uncertaintyProfile);
	if ( it != _config.uncertaintyProfiles.end() )
		_uncertainties = it.value();

	static_cast<ZoomRecordWidget*>(_currentRecord)->setUncertainties(_uncertainties);
	static_cast<ZoomRecordWidget*>(_currentRecord)->setCrossHairEnabled(_config.showCrossHair);

	if ( SCScheme.unit.distanceInKM )
		_spinDistance->setValue(Math::Geo::deg2km(_config.defaultAddStationsDistance));
	else
		_spinDistance->setValue(_config.defaultAddStationsDistance);

	if ( _comboFilter ) {
		_comboFilter->blockSignals(true);
		_comboFilter->clear();
		_comboFilter->addItem(NO_FILTER_STRING);

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

		_comboFilter->setCurrentIndex(defaultIndex != -1?defaultIndex:(_comboFilter->count() > 1?1:0));
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

	/*
	for ( int i = 0; i < _recordView->rowCount(); ++i ) {
		RecordViewItem *item = _recordView->itemAt(i);
		if ( item->value(ITEM_DISTANCE_INDEX) >= 0 ) {
			if ( _config.showAllComponents &&
				_config.allComponentsMaximumStationDistance >= item->value(ITEM_DISTANCE_INDEX) )
				item->widget()->setDrawMode(RecordWidget::InRows);
			else
				item->widget()->setDrawMode(RecordWidget::Single);
		}
		else
			item->widget()->setDrawMode(RecordWidget::Single);
	}
	*/


	if ( _actionsUncertainty ) {
		delete _actionsUncertainty;
		_actionsUncertainty = NULL;
	}

	_actionsUncertainty = new QActionGroup(this);

	QAction *action = new QAction(_actionsUncertainty);
	action->setText("unset");
	action->setShortcut(Qt::SHIFT + Qt::Key_0);
	action->setData(-1);
	addAction(action);

	connect(action, SIGNAL(triggered()), this, SLOT(setPickUncertainty()));

	if ( !_uncertainties.isEmpty() ) {
		if ( _uncertainties.count() > 8 ) {
			SEISCOMP_WARNING("more than 8 uncertainty profiles defined, shortcuts are "
			                 "assigned to the first 8 profiles only.");
		}

		for ( int i = 0; i < _uncertainties.count(); ++i ) {
			action = new QAction(_actionsUncertainty);
			QString text;

			Config::Uncertainty value = _uncertainties[i];
			if ( value.first == value.second )
				text = QString("+/-%1s").arg(value.first);
			else
				text = QString("-%1s;+%2s").arg(value.first).arg(value.second);

			action->setText(text);
			action->setData(i);

			if ( i < 9 )
				action->setShortcut(Qt::SHIFT + (Qt::Key_1 + i));

			connect(action, SIGNAL(triggered()), this, SLOT(setPickUncertainty()));

			addAction(action);
		}
	}


	if ( _config.hideStationsWithoutData ) {
		bool reselectCurrentItem = false;

		for ( int r = 0; r < _recordView->rowCount(); ++r ) {
			RecordViewItem* item = _recordView->itemAt(r);
			PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
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
			PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
			if ( isLinkedItem(item) ) continue;

			if ( !isTracePicked(item->widget()) && !label->hasGotData )
				item->forceInvisibilty(!label->isEnabledByConfig);
		}
	}

	_ui.actionShowUnassociatedPicks->setChecked(_config.loadAllPicks);

	initPhases();
	acquireStreams();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setDatabase(Seiscomp::DataModel::DatabaseQuery* reader) {
	_reader = reader;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setBroadBandCodes(const std::vector<std::string> &codes) {
	_broadBandCodes = codes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setStrongMotionCodes(const std::vector<std::string> &codes) {
	_strongMotionCodes = codes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showEvent(QShowEvent *e) {
	// avoid truncated distance labels
	int w1 = _ui.frameCurrentRowLabel->width();
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
		_ui.frameCurrentRowLabel->setFixedWidth(w2);

	if ( !_settingsRestored ) {
		QList<int> sizes;

		if ( SCApp ) {
			SCApp->settings().beginGroup(objectName());
			QVariant geometry = SCApp->settings().value("geometry");
			restoreState(SCApp->settings().value("state").toByteArray());
			restoreGeometry(geometry.toByteArray());
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

	_recordView->setLabelWidth(_ui.frameCurrentRowLabel->width() +
	                           _ui.frameCurrentRow->frameWidth() +
	                           _ui.frameCurrentRow->layout()->margin());

	QWidget::showEvent(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::onSelectedTime(Seiscomp::Core::Time time) {
	//updatePhaseMarker(_currentRecord, time);
	if ( _recordView->currentItem() ) {
		updatePhaseMarker(_recordView->currentItem()->widget(), time);
		_currentRecord->update();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::onSelectedTime(Seiscomp::Gui::RecordWidget* widget,
                                Seiscomp::Core::Time time) {
	if ( widget == _currentRecord ) return;
	updatePhaseMarker(widget, time);
	//updatePhaseMarker(_currentRecord, time);
	_currentRecord->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updatePhaseMarker(Seiscomp::Gui::RecordWidget* widget,
                                   const Seiscomp::Core::Time& time) {
	ZoomRecordWidget *zoomRecord = static_cast<ZoomRecordWidget*>(_currentRecord);
	int uncertaintyIndex = zoomRecord->currentUncertaintyIndex();

	PickerMarker *marker = (PickerMarker*)widget->marker(widget->cursorText(), true);
	// Marker found?
	if ( marker ) {
		marker->setType(PickerMarker::Arrival);

		// Set the marker time to the new picked time
		marker->setCorrectedTime(time);
		// and set its component to the currently displayed component
		marker->setSlot(_currentSlot);
		marker->setRotation(_currentRotationMode);
		marker->setFilter(_currentFilterID);

		if ( uncertaintyIndex >= 0 ) {
			marker->setUncertainty(
				_uncertainties[uncertaintyIndex].first,
				_uncertainties[uncertaintyIndex].second
			);
		}

		marker->setEnabled(true);
		widget->setCurrentMarker(marker);

		/*
		// If there exists another marker with a different phase on the same time, delete it
		for ( int i = 0; i < widget->markerCount(); ++i ) {
			if ( widget->marker(i)->text() != marker->text()
			     && ((PickerMarker*)widget->marker(i))->isArrival()
			     && widget->marker(i)->correctedTime() == marker->correctedTime() ) {
				// NOTE: Better check whether the underlaying marker is an arrival or a manual pick
				//       to decide whether to delete or to rename the pick
				if ( widget->removeMarker(i) )
					--i;
			}
		}
		*/

		// Disable all other arrivals of the same phase
		for ( int i = 0; i < widget->markerCount(); ++i ) {
			PickerMarker* m = (PickerMarker*)widget->marker(i);
			if ( marker == m ) continue;
			if ( m->text() != widget->cursorText() ) continue;
			if ( m->isArrival() ) m->setType(PickerMarker::Pick);
		}

		widget->update();
	}
	else {
		// Valid phase code?
		if ( !widget->cursorText().isEmpty() ) {
			PickerMarker *reusedMarker = NULL;

			// Look for a marker that is on the same position
			for ( int i = 0; i < widget->markerCount(); ++i ) {
				if ( /*widget->marker(i)->text() != widget->cursorText()
				     &&*/
				     widget->marker(i)->correctedTime() == time ) {
					marker = static_cast<PickerMarker*>(widget->marker(i));

#if 0
					if ( !marker->isMovable() /*&& !marker->isPick()*/ ) continue;
#endif
					if ( !marker->isMovable() && !marker->isArrival() && !marker->isPick() ) continue;
					if ( !marker->isPick() && !marker->isArrival() ) continue;

					reusedMarker = marker;
					break;
				}
			}

			if ( reusedMarker == NULL ) {
				// Create a new marker for the phase
				marker = new PickerMarker(widget, time, widget->cursorText(),
				                          PickerMarker::Arrival, true);

				if ( uncertaintyIndex >= 0 ) {
					marker->setUncertainty(
						_uncertainties[uncertaintyIndex].first,
						_uncertainties[uncertaintyIndex].second
					);
				}

				marker->setSlot(_currentSlot);
				marker->setRotation(_currentRotationMode);
				marker->setFilter(_currentFilterID);

				for ( int i = 0; i < widget->markerCount(); ++i ) {
					PickerMarker *marker2 = (PickerMarker*)widget->marker(i);
					if ( marker == marker2 ) continue;

					if ( marker2->text() == marker->text() && marker2->isArrival() ) {
						// Set type back to pick. The phase code is updated
						// automatically
						marker2->setType(PickerMarker::Pick);
					}
				}

				widget->setCurrentMarker(marker);
			}
			else {
				declareArrival(reusedMarker, widget->cursorText(), false);
				widget->setCurrentMarker(reusedMarker);
			}

			widget->update();
		}
	}

	if ( _recordView->currentItem()->widget() == widget &&
	     widget->cursorText() == "P" && marker ) {
		RecordMarker* marker2 = widget->marker("P"THEORETICAL_POSTFIX);
		if ( marker2 )
			_recordView->currentItem()->setValue(ITEM_RESIDUAL_INDEX,
				-fabs((double)(marker->correctedTime() - marker2->correctedTime())));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::declareArrival(RecordMarker *m_, const QString &phase,
                                bool updateResidual) {
	PickerMarker *m = (PickerMarker*)m_;
	if ( m->isPick() ) m->setType(PickerMarker::Arrival);
	m->setPhaseCode(phase);
	m->setEnabled(true);

	RecordWidget *w = m->parent();
	for ( int i = 0; i < w->markerCount(); ++i ) {
		PickerMarker *marker = (PickerMarker*)w->marker(i);
		if ( marker == m ) continue;
		if ( marker->text() != phase ) continue;
		if ( marker->isArrival() ) {
			if ( marker->isMovable() )
				delete marker;
			else
				marker->setType(PickerMarker::Pick);
			break;
		}
	}

	if ( !updateResidual ) return;

	if ( _recordView->currentItem()->widget() == w &&
	     w->cursorText() == "P" && m ) {
		RecordMarker* marker2 = w->marker("P"THEORETICAL_POSTFIX);
		if ( marker2 )
			_recordView->currentItem()->setValue(ITEM_RESIDUAL_INDEX,
				-fabs((double)(m->correctedTime() - marker2->correctedTime())));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::onAddedItem(const Record* rec, RecordViewItem* item) {
	// NOTE: Dynamic item insertion is not yet used
	/*
	setupItem(item);
	addTheoreticalArrivals(item, rec->networkCode(), rec->stationCode(), rec->locationCode());
	sortByDistance();
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::apply(QAction* action) {
	int id = action->data().toInt() - 1;

	if ( id < 0 ) {
		switch ( id ) {
			case -2:
				alignOnOriginTime();
				break;
			case -3:
				sortByDistance();
				break;
			case -4:
				pickNone(true);
				break;
			case -5:
				confirmPick();
				break;
			case -6:
				deletePick();
				break;
			case -7:
				relocate();
				break;
		}
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setPickPhase(QAction *action) {
	setCursorText(action->text());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::alignOnPhase(QAction *action) {
	alignOnPhase(action->text().left(action->text().indexOf(' ')), action->data().toBool());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setCursorText(const QString& text) {
	_recordView->setCursorText(text);
	_currentRecord->setCursorText(text);
	_currentRecord->setActive(text != "");

	if ( _currentRecord->isActive() ) {
#ifdef CENTER_SELECTION
		_centerSelection = true;
#endif
		RecordMarker* m = _currentRecord->marker(text, true);
		if ( !m ) m = _currentRecord->marker(text);
		if ( m )
			setCursorPos(m->correctedTime());
#ifdef CENTER_SELECTION
		else if ( _recordView->currentItem() )
			setCursorPos(_recordView->currentItem()->widget()->visibleTimeWindow().startTime() +
			             Core::TimeSpan(_recordView->currentItem()->widget()->visibleTimeWindow().length()*0.5));
#endif
	}

	updateCurrentRowState();
	componentByState();

	return;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::alignOnPhase(const QString& phase, bool theoretical) {
	int used = 0;

	_alignedOnOT = false;
	QString phaseId = phase;
	//QString shortPhaseId = QString("%1").arg(getShortPhaseName(phase.toStdString()));

	if ( theoretical ) {
		phaseId += THEORETICAL_POSTFIX;
		//shortPhaseId += THEORETICAL_POSTFIX;
	}

	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem *item = _recordView->itemAt(r);
		PickerRecordLabel *l = static_cast<PickerRecordLabel*>(item->label());

		// Is the item an linked (controlled) item, ignore it
		// The alignment is done by the controller item
		if ( l->isLinkedItem() ) continue;

		RecordViewItem *controlledItem = l->controlledItem();

		RecordWidget* w1 = item->widget();
		RecordWidget* w2 = controlledItem?controlledItem->widget():NULL;

		// Find modified arrivals for phase of controller item
		RecordMarker* m = w1->marker(phaseId, true);
		// Find arrivals for phase
		if ( !m ) m = w1->marker(phaseId);
		//if ( !m ) m = w1->marker(shortPhaseId, true);

		// No pick found on controller item?
		if ( w2 && !m ) {
			m = w2->marker(phaseId, true);
			if ( !m ) m = w2->marker(phaseId);
		}

		if ( !theoretical ) {
			// Find theoretical arrivals for phase
			if ( !m ) m = w1->marker(phase + THEORETICAL_POSTFIX);

			// Find automatic picks for phase
			if ( !m ) m = w1->marker(phase + AUTOMATIC_POSTFIX);

			if ( w2 && !m ) {
				// Find automatic picks for phase
				m = w2->marker(phase + AUTOMATIC_POSTFIX);
			}
		}

		if ( m ) {
			w1->setAlignment(m->correctedTime());
			if ( w2 ) w2->setAlignment(m->correctedTime());

			++used;
		}
	}

	if ( !used ) return;

	_checkVisibility = false;

	_recordView->setAbsoluteTimeEnabled(false);

	_recordView->setJustification(_config.alignmentPosition);

	double timeRange = _recordView->timeRangeMax() - _recordView->timeRangeMin();
	double leftTime = -timeRange*_config.alignmentPosition;
	double rightTime = timeRange*(1.0-_config.alignmentPosition);
	_recordView->setTimeRange(leftTime, rightTime);

	_checkVisibility = true;

	if ( _recordView->currentItem() ) {
		RecordWidget* w = _recordView->currentItem()->widget();
		setAlignment(w->alignment());
//#ifdef CENTER_SELECTION
		_centerSelection = true;
//#endif
		setCursorPos(w->alignment(), true);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::loadNextStations() {
	float distance = _spinDistance->value();

	if ( SCScheme.unit.distanceInKM )
		distance = Math::Geo::km2deg(distance);

	std::vector<Seiscomp::DataModel::WaveformStreamID>::iterator it;

	_recordView->setUpdatesEnabled(false);

	/*
	for ( it = _nextStations.begin(); it != _nextStations.end(); ++it ) {
		RecordViewItem* item = _recordView->item(waveformIDToString(*it));
		if ( item ) {
			item->setVisible(item->value(0) <= distance);
			//_recordView->removeItem(item);
			//_stations.remove((it->networkCode() + "." + it->stationCode()).c_str());
		}
	}
	*/

	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem* item = _recordView->itemAt(r);
		bool show = false;

		if ( !isLinkedItem(item) ) {
			if ( isArrivalTrace(item->widget()) )
			//if ( item->widget()->hasMovableMarkers() )
				show = true;
			else
				show = item->value(ITEM_DISTANCE_INDEX) <= distance;

			if ( _ui.actionShowUsedStations->isChecked() )
				show = show && isTraceUsed(item->widget());

			item->setVisible(show);
		}
	}

	loadNextStations(distance);
	fillRawPicks();

	sortByState();
	alignByState();
	componentByState();

	if ( _recordView->currentItem() == NULL ) {
		selectFirstVisibleItem(_recordView);
	}
	setCursorText(_currentRecord->cursorText());

	_recordView->setUpdatesEnabled(true);
	_recordView->setFocus();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::sortByState() {
	if ( _ui.actionSortByDistance->isChecked() )
		sortByDistance();
	else if ( _ui.actionSortByAzimuth->isChecked() )
		sortByAzimuth();
	else if ( _ui.actionSortAlphabetically->isChecked() )
		sortAlphabetically();
	else if ( _ui.actionSortByResidual->isChecked() )
		sortByResidual();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::alignByState() {
	if ( _alignedOnOT && _origin )
		_recordView->setAlignment(_origin->time());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::componentByState() {
	if ( _ui.actionShowZComponent->isChecked() )
		showComponent('Z');
	else if ( _ui.actionShowNComponent->isChecked() )
		showComponent('1');
	else if ( _ui.actionShowEComponent->isChecked() )
		showComponent('2');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::resetState() {
	if ( _comboRotation->currentIndex() > RT_Z12 )
		changeRotation(_comboRotation->currentIndex());

	showComponent('Z');
	alignOnOriginTime();
	pickNone(true);
	sortByDistance();
	_ui.actionShowUsedStations->setChecked(false);
	showUsedStations(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateLayoutFromState() {
	sortByState();
	alignByState();
	componentByState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::firstConnectionEstablished() {
	_connectionState->start();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::lastConnectionClosed() {
	_connectionState->stop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::beginWaitForRecords() {
	qApp->setOverrideCursor(Qt::WaitCursor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::doWaitForRecords(int value) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::endWaitForRecords() {
	qApp->restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showFullscreen(bool e) {
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
void PickerView::fetchComponent(char componentCode) {
	for ( WaveformStreamList::iterator it = _allStreams.begin();
	      it != _allStreams.end(); ) {
		char queuedComponent = it->component;
		if ( queuedComponent == componentCode || queuedComponent == '?' ||
		     _config.loadAllComponents ) {
			if ( _config.usePerStreamTimeWindows ) {
				// Cut the needed timewindow
				RecordViewItem* item = _recordView->item(it->streamID);
				if ( item ) {
					RecordWidget *w = item->widget();
					Core::TimeWindow tw;
					for ( int i = 0; i < w->markerCount(); ++i ) {
						PickerMarker *m = static_cast<PickerMarker*>(w->marker(i));
						if ( (m->type() == PickerMarker::Arrival || m->type() == PickerMarker::Theoretical) &&
						     getShortPhaseName(m->text().toStdString()) == 'P' &&
						     m->text().left(3) != "PcP" ) {
							Core::Time start = m->time() - Core::TimeSpan(_config.preOffset);
							Core::Time end = m->time() + Core::TimeSpan(_config.postOffset);

							if ( !tw.startTime().valid() || tw.startTime() > start )
								tw.setStartTime(start);

							if ( !tw.endTime().valid() || tw.endTime() < end )
								tw.setEndTime(end);
						}
					}

					it->timeWindow = tw;
				}
			}

			_nextStreams.push_back(*it);
			it = _allStreams.erase(it);
		}
		else
			++it;
	}

	// Sort by distance
	_nextStreams.sort();

	acquireStreams();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showComponent(char componentCode) {
	fetchComponent(componentCode);

	switch ( componentCode ) {
		default:
		case 'Z':
			_currentSlot = 0;
			break;
		case '1':
			_currentSlot = 1;
			break;
		case '2':
			_currentSlot = 2;
			break;
	}

	_recordView->showSlot(_currentSlot);
	_ui.actionShowZComponent->setChecked(componentCode == 'Z');
	_ui.actionShowNComponent->setChecked(componentCode == '1');
	_ui.actionShowEComponent->setChecked(componentCode == '2');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showUsedStations(bool usedOnly) {
	//float distance = _spinDistance->value();

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
void PickerView::loadNextStations(float distance) {
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

				// try to get the configured location and stream code
				Stream *stream = findConfiguredStream(s, _origin->time());
				if ( stream != NULL ) {
					SEISCOMP_DEBUG("Adding configured stream %s.%s.%s.%s",
					               stream->sensorLocation()->station()->network()->code().c_str(),
					               stream->sensorLocation()->station()->code().c_str(),
					               stream->sensorLocation()->code().c_str(),
					               stream->code().c_str());
				}

				// Try to get a default stream
				if ( stream == NULL ) {
					// Preferred channel code is BH. If not available use either SH or skip.
					for ( size_t c = 0; c < _broadBandCodes.size(); ++c ) {
						stream = findStream(s, _broadBandCodes[c], _origin->time());
						if ( stream ) break;
					}
				}

				if ( (stream == NULL) && !_config.ignoreUnconfiguredStations ) {
					stream = findStream(s, _origin->time(), Processing::WaveformProcessor::MeterPerSecond);
					if ( stream != NULL ) {
						SEISCOMP_DEBUG("Adding velocity stream %s.%s.%s.%s",
						               stream->sensorLocation()->station()->network()->code().c_str(),
						               stream->sensorLocation()->station()->code().c_str(),
						               stream->sensorLocation()->code().c_str(),
						               stream->code().c_str());
					}
				}

				if ( stream ) {
					WaveformStreamID streamID(n->code(), s->code(), stream->sensorLocation()->code(), stream->code().substr(0,stream->code().size()-1) + '?', "");

					RecordViewItem* item = addStream(stream->sensorLocation(), streamID, delta, streamID.stationCode().c_str(), false, true);
					if ( item ) {
						_stations.insert(code);
						item->setVisible(!_ui.actionShowUsedStations->isChecked());
						if ( _config.hideStationsWithoutData )
							item->forceInvisibilty(true);
					}
				}
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::addArrival(Seiscomp::Gui::RecordWidget* widget,
                            Seiscomp::DataModel::Arrival* arrival, int id) {
	Pick* pick = Pick::Find(arrival->pickID());
	if ( !pick ) return;

	if ( !arrival->phase().code().empty() ) {
		//NOTE: Because autoloc's associates e.g. PP phases automatically
		//      it is important to insert all arrivals here otherwise the
		//      trace wont be shown (unused)
		//if ( !_phases.contains(arrival->phase().code().c_str()) )
		//	return;

		PickerMarker *marker = new PickerMarker(widget,
		                                        pick->time(),
		                                        arrival->phase().code().c_str(),
		                                        PickerMarker::Arrival, false);

		marker->setPick(pick);
		marker->setId(id);

		if ( !pick->methodID().empty() ) {
			marker->setDescription(QString("%1<%2>")
			                       .arg(arrival->phase().code().c_str())
			                       .arg((char)toupper(pick->methodID()[0])));
		}

		try {
			if ( arrival->weight() <= 0.5 ) {
				marker->setEnabled(false);
				//marker->setMovable(true);
			}
		}
		catch ( ... ) {}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::setOrigin(Seiscomp::DataModel::Origin* origin,
                           double relTimeWindowStart,
                           double relTimeWindowEnd) {
	if ( origin == _origin ) return false;

	SEISCOMP_DEBUG("stopping record acquisition");
	stop();

	_recordView->clear();
	_recordItemLabels.clear();

	_origin = origin;

	updateOriginInformation();
	if ( _comboFilter->currentIndex() == 0 && _lastFilterIndex > 0 )
		_comboFilter->setCurrentIndex(_lastFilterIndex);

	if ( _origin == NULL )
		return false;

	setUpdatesEnabled(false);

	_stations.clear();

	Core::Time originTime = _origin->time();
	if ( !originTime ) originTime = Core::Time::GMT();

	Core::Time minTime = originTime;
	Core::Time maxTime = originTime;

	// Find the minimal and maximal pick time
	for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
		Arrival* arrival = origin->arrival(i);
		Pick* pick = Pick::Cast(PublicObject::Find(arrival->pickID()));
		if ( !pick )
			continue;

		if ( (Core::Time)pick->time() < minTime )
			minTime = pick->time();

		if ( (Core::Time)pick->time() > maxTime )
			maxTime = pick->time();
	}

	if ( minTime > maxTime )
		std::swap(minTime, maxTime);

	// Add a buffer of one minute to the time span
	minTime -= _config.preOffset;
	maxTime += _config.postOffset;

	relTimeWindowStart = minTime - originTime;
	relTimeWindowEnd = maxTime - originTime;

	double timeWindowLength = relTimeWindowEnd - relTimeWindowStart;

	_minTime = relTimeWindowStart;
	_maxTime = relTimeWindowEnd;

	SEISCOMP_DEBUG("setting time range to: [%.2f,%.2f]", _minTime, _maxTime);

	timeWindowLength = std::max(timeWindowLength, (double)_config.minimumTimeWindow);
	_timeWindow = Core::TimeWindow((Core::Time)originTime + Core::TimeSpan(relTimeWindowStart),
	                               timeWindowLength);

	//_recordView->setBufferSize(timeWindowLength + 5*60); /*safety first! */
	_recordView->setTimeWindow(_timeWindow);
	_recordView->setTimeRange(_minTime, _maxTime);

	if ( origin->arrivalCount() > 0 ) {
		for ( size_t i = 0; i < origin->arrivalCount(); ++i ) {
			Arrival* arrival = origin->arrival(i);

			PickPtr pick = Pick::Cast(PublicObject::Find(arrival->pickID()));
			if ( !pick ) {
				//std::cout << "pick not found" << std::endl;
				continue;
			}

			WaveformStreamID streamID = adjustWaveformStreamID(pick->waveformID());
			SensorLocation *loc = NULL;

			Station *sta = Client::Inventory::Instance()->getStation(
				streamID.networkCode(), streamID.stationCode(), _origin->time());

			if ( sta )
				loc = findSensorLocation(sta, streamID.locationCode(), origin->time());

			double dist;
			try { dist = arrival->distance(); }
			catch ( ... ) { dist = 0; }

			RecordViewItem* item = addStream(loc, pick->waveformID(), dist, pick->waveformID().stationCode().c_str(), true, false);

			// A new item has been inserted
			if ( item != NULL ) {
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

			addArrival(item->widget(), arrival, i);
		}
	}
	else
		loadNextStations();

	_timeWindowOfInterest.setStartTime(originTime + Core::TimeSpan(relTimeWindowStart));
	_timeWindowOfInterest.setEndTime(originTime + Core::TimeSpan(relTimeWindowStart + timeWindowLength));
	_loadedPicks = false;
	_picksInTime.clear();

	if ( CFG_LOAD_PICKS ) loadPicks();

	if ( _loadedPicks )
		_ui.actionShowUnassociatedPicks->setChecked(true);

	fillRawPicks();
	fillTheoreticalArrivals();
	_recordView->setAlignment(originTime);

	resetState();
	updateLayoutFromState();

	selectFirstVisibleItem(_recordView);

	setUpdatesEnabled(true);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int PickerView::loadPicks() {
	if ( !_timeWindowOfInterest ) return -1;

	SEISCOMP_DEBUG("Loading picks in time window: %s ~ %s",
	               _timeWindowOfInterest.startTime().iso().c_str(),
	               _timeWindowOfInterest.endTime().iso().c_str());

	std::vector<Seiscomp::DataModel::PickPtr> savePicks = _picksInTime;

	if ( !SCApp->commandline().hasOption("offline") ) {
		if ( _reader ) {
			qApp->setOverrideCursor(Qt::WaitCursor);

			DatabaseIterator it = _reader->getPicks(_timeWindowOfInterest.startTime(),
			                                        _timeWindowOfInterest.endTime());
			for ( ; *it; ++it ) {
				Pick* pick = Pick::Cast(*it);
				if ( pick )
					_picksInTime.push_back(pick);
			}

			//std::cout << "read " << _picksInTime.size() << " picks in time" << std::endl;
			_loadedPicks = true;

			qApp->restoreOverrideCursor();
		}
	}
	else {
		qApp->setOverrideCursor(Qt::WaitCursor);

		EventParameters *ep = EventParameters::Cast(PublicObject::Find("EventParameters"));
		if ( ep ) {
			for ( size_t i = 0; i < ep->pickCount(); ++i ) {
				Pick* pick = ep->pick(i);
				if ( pick && _timeWindowOfInterest.contains(pick->time().value()) )
					_picksInTime.push_back(pick);
			}
		}

		_loadedPicks = true;
		//std::cout << "read " << _picksInTime.size() << " picks in time" << std::endl;

		qApp->restoreOverrideCursor();
	}

	return (int)_picksInTime.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::setOrigin(Seiscomp::DataModel::Origin* o) {
	_origin = o;

	// Remove picks and arrivals from all traces and update the ones
	// from the new origin
	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem* item = _recordView->itemAt(r);
		item->label()->setEnabled(true);
		item->widget()->clearMarker();
	}

	Core::Time originTime = _origin->time();
	if ( !originTime ) originTime = Core::Time::GMT();

	Core::Time minTime = originTime;
	Core::Time maxTime = originTime;

	// Add a buffer of one minute to the time span
	minTime -= _config.preOffset;
	maxTime += _config.postOffset;

	double relTimeWindowStart = minTime - originTime;
	double relTimeWindowEnd = maxTime - originTime;

	double timeWindowLength = relTimeWindowEnd - relTimeWindowStart;

	_minTime = relTimeWindowStart;
	_maxTime = relTimeWindowEnd;

	SEISCOMP_DEBUG("update time range to: [%.2f,%.2f]", _minTime, _maxTime);

	timeWindowLength = std::max(timeWindowLength, (double)_config.minimumTimeWindow);
	_timeWindow = Core::TimeWindow((Core::Time)originTime + Core::TimeSpan(relTimeWindowStart),
	                               timeWindowLength);

	_timeScale->setSelectionEnabled(false);

	if ( _origin ) {
		for ( size_t i = 0; i < _origin->arrivalCount(); ++i ) {
			Pick* pick = Pick::Find(_origin->arrival(i)->pickID());
			if ( pick ) {
				RecordViewItem* item = _recordView->item(adjustWaveformStreamID(pick->waveformID()));
				if ( !item ) {
					SensorLocation *loc = NULL;

					Station *sta = Client::Inventory::Instance()->getStation(
						pick->waveformID().networkCode(),
						pick->waveformID().stationCode(),
						_origin->time());

					if ( sta )
						loc = findSensorLocation(sta, pick->waveformID().locationCode(), _origin->time());

					double dist;
					try { dist = _origin->arrival(i)->distance(); }
					catch ( ... ) { dist = 0; }

					item = addStream(loc, pick->waveformID(), dist, pick->waveformID().stationCode().c_str(), true, false);
					if ( item ) {
						_stations.insert((pick->waveformID().networkCode() + "." +
						                  pick->waveformID().stationCode()).c_str());
					}
				}

				if ( item )
					addArrival(item->widget(), _origin->arrival(i), i);
			}
		}
	}

	fillRawPicks();

	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem *item = _recordView->itemAt(r);
		PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());

		if ( _origin ) {
			double delta, az, baz;
			Geo::delazi(_origin->latitude(), _origin->longitude(),
			            label->latitude, label->longitude, &delta, &az, &baz);

			label->orientationZRT.loadRotateZ(deg2rad(baz + 180.0));
		}
		else
			label->orientationZRT.identity();
	}


	if ( _comboRotation->currentIndex() == RT_ZRT )
		changeRotation(RT_ZRT);
	else if ( _comboRotation->currentIndex() == RT_ZNE )
		changeRotation(RT_ZNE);
	else if ( _comboRotation->currentIndex() == RT_ZH )
		changeRotation(RT_ZH);

	componentByState();
	updateOriginInformation();
	updateTheoreticalArrivals();
	alignByState();
	sortByState();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateOriginInformation() {
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
const TravelTime* PickerView::findPhase(const TravelTimeList &ttt, const QString &phase, double delta) {
	TravelTimeList::const_iterator it;

	if (phase=="P" || phase=="P1")
		return firstArrivalP(&ttt);

	// First pass -> exact match
	for ( it = ttt.begin(); it != ttt.end(); ++it ) {
		if (delta>115) { // skip Pdiff et al.
			if (it->phase ==  "Pdiff") continue;
			if (it->phase == "pPdiff") continue;
			if (it->phase == "sPdiff") continue;
		}

		QString ph(it->phase.c_str());

		if ( phase == ph )
			return &(*it);

		if (phase=="P" && (it->phase == "Pn" || it->phase == "Pg" || it->phase == "Pb"))
			return &(*it);
	}

	if ( phase != "P" && phase != "S" )
		return NULL;

	// Second pass -> find first phase that represents a
	// P or S phase
	for ( it = ttt.begin(); it != ttt.end(); ++it ) {
		if (delta>115) { // skip Pdiff et al.
			if (it->phase ==  "Pdiff") continue;
			if (it->phase == "pPdiff") continue;
			if (it->phase == "sPdiff") continue;
		}

		if ( phase[0] == getShortPhaseName(it->phase) )
			return &(*it);
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::addTheoreticalArrivals(RecordViewItem* item,
                                        const std::string& netCode,
                                        const std::string& staCode,
                                        const std::string& locCode) {
	if ( _origin == NULL ) return false;

	try {
		DataModel::SensorLocation *loc =
			Client::Inventory::Instance()->getSensorLocation(
				netCode, staCode, locCode, _origin->time()
		);

		if ( loc == NULL ) {
			SEISCOMP_ERROR("SensorLocation %s.%s.%s not found",
			               netCode.c_str(), staCode.c_str(), locCode.c_str());
			return false;
		}

		double delta, az1, az2;
		double elat = _origin->latitude();
		double elon = _origin->longitude();
		double slat = loc->latitude();
		double slon = loc->longitude();
		double salt = loc->elevation();

		Geo::delazi(elat, elon, slat, slon, &delta, &az1, &az2);

		item->setValue(ITEM_DISTANCE_INDEX, delta);
		item->setValue(ITEM_AZIMUTH_INDEX, az1);

		/*
		if ( _config.showAllComponents && _config.allComponentsMaximumStationDistance >= delta )
			item->widget()->setDrawMode(RecordWidget::InRows);
		else
			item->widget()->setDrawMode(RecordWidget::Single);
		*/

		/*
		item->label()->setText(QApplication::translate("PickerView", "D: %1\302\260 A: %2\302\260", 0, QApplication::UnicodeUTF8)
		                        .arg(delta,0,'f',1).arg(az1,0,'f',1), 1);
		*/

		item->label()->setText(QString("%1").arg(netCode.c_str()), 1);
		QFontMetrics fm(item->label()->font(1));
		item->label()->setWidth(fm.boundingRect("WW  ").width(), 1);

		if ( SCScheme.unit.distanceInKM )
			item->label()->setText(QString("%1 km").arg(Math::Geo::deg2km(delta),0,'f',SCScheme.precision.distance), 2);
		else
			item->label()->setText(QString("%1%2").arg(delta,0,'f',1).arg(degrees), 2);
		item->label()->setAlignment(Qt::AlignRight, 2);
		item->label()->setColor(palette().color(QPalette::Disabled, QPalette::WindowText), 2);

		double depth;
		try {
			depth = _origin->depth();
		}
		catch ( ... ) {
			depth = 0.0;
		}

		if ( depth <= 0.0 ) depth = 1.0;
//std::cerr << staCode << std::endl;
		TravelTimeList* ttt = _ttTable.compute(elat, elon, depth, slat, slon, salt);

		if ( ttt ) {
			QMap<QString, RecordMarker*> currentPhases;

			foreach ( const QString &phase, _phases ) {
				// Find the TravelTime from the TravelTimeList that corresponds
				// the current phase
				const TravelTime *tt = findPhase(*ttt, phase, delta);
				if ( !tt ) continue;

				// If there is already a theoretical marker for the given
				// TravelTime the current item gets another alias
				if ( currentPhases.contains(tt->phase.c_str()) ) {
					currentPhases[tt->phase.c_str()]->addAlias(phase + THEORETICAL_POSTFIX);
					continue;
				}

				PickerMarker* marker = new PickerMarker(
					item->widget(),
					(Core::Time)_origin->time() + Core::TimeSpan(tt->time),
					phase + THEORETICAL_POSTFIX,
					PickerMarker::Theoretical,
					false
				);

				marker->setVisible(_ui.actionShowTheoreticalArrivals->isChecked());

				// Set the description of the marker that is used as display text
				marker->setDescription(tt->phase.c_str());

				// Remember the phase
				currentPhases[tt->phase.c_str()] = marker;
			}

			foreach ( const QString &phase, _showPhases ) {
				// Find the TravelTime from the TravelTimeList that corresponds
				// the current phase
				const TravelTime *tt = findPhase(*ttt, phase, delta);
				if ( !tt ) continue;

				// If there is already a theoretical marker for the given
				// TravelTime the current item gets another alias
				if ( currentPhases.contains(tt->phase.c_str()) ) {
					currentPhases[tt->phase.c_str()]->addAlias(phase + THEORETICAL_POSTFIX);
					continue;
				}

				PickerMarker* marker = new PickerMarker(
					item->widget(),
					(Core::Time)_origin->time() + Core::TimeSpan(tt->time),
					phase + THEORETICAL_POSTFIX,
					PickerMarker::Theoretical,
					false
				);

				marker->setVisible(_ui.actionShowTheoreticalArrivals->isChecked());

				// Set the description of the marker that is used as display text
				marker->setDescription(tt->phase.c_str());

				// Remember the phase
				currentPhases[tt->phase.c_str()] = marker;
			}

			delete ttt;
		}

		for ( int i = 0; i < item->widget()->markerCount(); ++i ) {
			PickerMarker* m = static_cast<PickerMarker*>(item->widget()->marker(i));
			if ( m->text() == "P" && m->isArrival() ) {
				RecordMarker* m2 = item->widget()->marker("P"THEORETICAL_POSTFIX);
				if ( m2 ) {
					item->setValue(ITEM_RESIDUAL_INDEX, -fabs((double)(m->correctedTime() - m2->correctedTime())));
					break;
				}
			}
		}

		return true;
	}
	catch ( std::exception& excp ) {
		SEISCOMP_ERROR("%s", excp.what());
		//item->label()->setText("unknown", 0, Qt::darkRed);
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::fillTheoreticalArrivals() {
	bool stationSet = false;

	for ( int i = 0; i < _recordView->rowCount(); ++i ) {
		WaveformStreamID stream_id = _recordView->streamID(i);

		RecordViewItem* item = _recordView->itemAt(i);
		if ( addTheoreticalArrivals(item, stream_id.networkCode(), stream_id.stationCode(), stream_id.locationCode()) )
			stationSet = true;
	}

	return stationSet;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::fillRawPicks() {
	bool pickAdded = false;
	for ( size_t i = 0; i < _picksInTime.size(); ++i ) {
		bool result = addRawPick(_picksInTime[i].get());
		pickAdded = result || pickAdded;
	}

	return pickAdded;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::addPick(Seiscomp::DataModel::Pick* pick) {
	if ( (Core::Time)pick->time() > ((Core::Time)_origin->time() + _config.minimumTimeWindow) )
		return;

	_picksInTime.push_back(pick);
	addRawPick(pick);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setStationEnabled(const std::string& networkCode,
                                   const std::string& stationCode,
                                   bool state) {
	QList<RecordViewItem*> streams = _recordView->stationStreams(networkCode, stationCode);
	foreach ( RecordViewItem* item, streams ) {
		PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
		label->isEnabledByConfig = state;

		// Force state to false if item has no data yet and should be hidden
		if ( _config.hideStationsWithoutData && !label->hasGotData && !isTracePicked(item->widget()) )
			state = false;

		item->forceInvisibilty(!state);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::addRawPick(Seiscomp::DataModel::Pick *pick) {
	RecordViewItem* item = _recordView->item(adjustWaveformStreamID(pick->waveformID()));
	if ( !item )
		return false;

	RecordWidget* widget = item->widget();

	// Do we have a marker for this pick already?
	for ( int i = 0; i < widget->markerCount(); ++i ) {
		PickerMarker* m = static_cast<PickerMarker*>(widget->marker(i));
		if ( m->pick() && m->pick()->publicID() == pick->publicID() )
			return false;
	}

	PickerMarker* marker = new PickerMarker(NULL, pick->time(), PickerMarker::Pick, false);
	widget->insertMarker(0, marker);

	try {
		marker->setText(QString("%1"AUTOMATIC_POSTFIX).arg(pick->phaseHint().code().c_str()));

		if ( !pick->methodID().empty() ) {
			marker->setDescription(QString("%1<%2>")
			                       .arg(pick->phaseHint().code().c_str())
			                       .arg((char)toupper(pick->methodID()[0])));
		}
	}
	catch ( ... ) {}

	marker->setPick(pick);
	marker->update();

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::queueStream(double dist, const DataModel::WaveformStreamID& streamID,
                             char component) {
	_allStreams.push_back(WaveformRequest(dist, Core::TimeWindow(), streamID, component));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem* PickerView::addStream(const DataModel::SensorLocation *sloc,
                                      const WaveformStreamID& streamID,
                                      double distance,
                                      const std::string& text,
                                      bool showDisabled,
                                      bool theoreticalArrivals) {
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

	RecordViewItem *item = addRawStream(sloc, streamID, distance, text, theoreticalArrivals);
	if ( item == NULL ) return NULL;

	item->setValue(ITEM_PRIORITY_INDEX, 0);

	PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
	label->isEnabledByConfig = isEnabled;
	label->hasGotData = false;

	item->forceInvisibilty(!label->isEnabledByConfig);

	if ( hasStrongMotion ) {
		// Try to find a corresponding StrongMotion stream and add
		// it to the view
		RecordViewItem *sm_item = addRawStream(smsloc, smStreamID, distance, text, theoreticalArrivals);
		if ( sm_item ) {
			label = static_cast<PickerRecordLabel*>(sm_item->label());
			label->setLinkedItem(true);
			label->isEnabledByConfig = isEnabled;
			label->hasGotData = false;
			sm_item->setValue(ITEM_PRIORITY_INDEX, 1);
			sm_item->forceInvisibilty(!label->isEnabledByConfig);
			sm_item->setVisible(false);

			// Start showing the expandable button when the first record arrives
			connect(sm_item, SIGNAL(firstRecordAdded(const Seiscomp::Record*)),
			        static_cast<PickerRecordLabel*>(item->label()), SLOT(enableExpandable(const Seiscomp::Record*)));

			static_cast<PickerRecordLabel*>(item->label())->setControlledItem(sm_item);
			//static_cast<PickerRecordLabel*>(item->label())->enabledExpandButton(sm_item);

			sm_item->label()->setBackgroundColor(QColor(192,192,255));
		}
	}

	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setPickPolarity() {
	PickerMarker *m = static_cast<PickerMarker*>(_currentRecord->currentMarker());

	if ( m == NULL ) return;

	if ( !m->isPick() && !m->isArrival() ) return;
	if ( m->pick() && !m->isEnabled() ) return;

	// Create a new marker if the existing marker uses a pick already
	// to create a new pick
	if ( m->pick() ) {
		PickerMarker *old = m;
		m = new PickerMarker(old->parent(), *old);
		m->convertToManualPick();
		old->setType(PickerMarker::Pick);
		old->parent()->setCurrentMarker(m);
	}

	if ( sender() == _ui.actionSetPolarityPositive ) {
		m->setPolarity(PickPolarity(POSITIVE));
	}
	else if ( sender() == _ui.actionSetPolarityNegative ) {
		m->setPolarity(PickPolarity(NEGATIVE));
	}
	else if ( sender() == _ui.actionSetPolarityUndecidable ) {
		m->setPolarity(PickPolarity(UNDECIDABLE));
	}
	else if ( sender() == _ui.actionSetPolarityUnset ) {
		m->setPolarity(Core::None);
	}

	_currentRecord->update();
	_recordView->currentItem()->widget()->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setPickUncertainty() {
	PickerMarker *m = static_cast<PickerMarker*>(_currentRecord->currentMarker());
	if ( !_actionsUncertainty ) return;

	/*
	if ( !_currentRecord->cursorText().isEmpty() ) {
		foreach ( QAction *action, _actionsUncertainty->actions() ) {
			if ( sender() != action ) continue;
			bool ok;
			int idx = action->data().toInt(&ok);
			if ( !ok ) {
				cerr << "triggered uncertainty action with unexpected data: "
				     << action->data().toString().toStdString() << endl;
				return;
			}

			static_cast<ZoomRecordWidget*>(_currentRecord)->setCurrentUncertaintyIndex(idx);
		}
	}
	*/

	if ( m == NULL ) return;

	if ( !m->isPick() && !m->isArrival() ) return;
	if ( m->pick() && !m->isEnabled() ) return;

	foreach ( QAction *action, _actionsUncertainty->actions() ) {
		if ( sender() != action ) continue;

		bool ok;
		int idx = action->data().toInt(&ok);
		if ( !ok ) {
			std::cerr << "triggered uncertainty action with unexpected data: "
			          << action->data().toString().toStdString() << std::endl;
			return;
		}

		if ( idx < -1 || idx >= _uncertainties.count() ) {
			std::cerr << "triggered uncertainty action out of range: "
			          << idx << " not in [0," << _uncertainties.count()-1
			          << "]" << std::endl;
			return;
		}

		// Create a new marker if the existing marker uses a pick already
		// to create a new pick
		if ( m->pick() ) {
			PickerMarker *old = m;
			m = new PickerMarker(old->parent(), *old);
			m->convertToManualPick();
			old->setType(PickerMarker::Pick);
			old->parent()->setCurrentMarker(m);
		}

		if ( idx == -1 )
			m->setUncertainty(-1,-1);
		else
			m->setUncertainty(_uncertainties[idx].first, _uncertainties[idx].second);

		updateUncertaintyHandles(m);

		// Store values since they are copied again after the context menu
		// is closed
		_tmpLowerUncertainty = m->lowerUncertainty();
		_tmpUpperUncertainty = m->upperUncertainty();

		_currentRecord->update();
		_recordView->currentItem()->widget()->update();
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::openContextMenu(const QPoint &p) {
	RecordViewItem* item = (RecordViewItem*)sender();
	if ( !item->widget()->cursorText().isEmpty() ) return;

	//std::cout << "Context menu request from " << waveformIDToStdString(item->streamID()) << std::endl;

	Client::Inventory* inv = Client::Inventory::Instance();
	if ( !inv ) return;

	QMenu menu(this);
	int entries = 0;

	QMenu *streams = menu.addMenu("Add stream");

	WaveformStreamID tmp(item->streamID());

	Station* station = inv->getStation(item->streamID().networkCode(), item->streamID().stationCode(), _origin->time());
	if ( station == NULL ) return;

	std::set<std::string> codes;

	for ( size_t i = 0; i < station->sensorLocationCount(); ++i ) {
		SensorLocation* loc = station->sensorLocation(i);

		try {
			if ( loc->end() <= _origin->time() ) continue;
		}
		catch ( Seiscomp::Core::ValueException& ) {}

		if ( loc->start() > _origin->time() ) continue;

		for ( size_t j = 0; j < loc->streamCount(); ++j ) {
			Stream* stream = loc->stream(j);

			std::string streamCode = stream->code().substr(0, stream->code().size()-1);
			std::string id = loc->code() + "." + streamCode;

			try {
				if ( stream->end() <= _origin->time() ) continue;
			}
			catch ( Seiscomp::Core::ValueException& ) {}

			if ( stream->start() > _origin->time() ) continue;

			if ( codes.find(id) != codes.end() ) continue;
			codes.insert(id);

			tmp.setLocationCode(loc->code());
			tmp.setChannelCode(streamCode + '?');

			if ( _recordView->item(tmp) != NULL ) continue;

			QAction *action = new QAction(id.c_str(), streams);
			action->setData(waveformIDToQString(tmp));
			streams->addAction(action);

			++entries;
		}
	}

	if ( !entries ) return;

	//menu.addAction(cutAct);
	//menu.addAction(copyAct);
	//menu.addAction(pasteAct);
	QAction *res = menu.exec(item->mapToGlobal(p));
	if ( !res ) return;

	QString data = res->data().toString();
	QStringList l = data.split('.');
	tmp.setNetworkCode(l[0].toStdString());
	tmp.setStationCode(l[1].toStdString());
	tmp.setLocationCode(l[2].toStdString());
	tmp.setChannelCode(l[3].toStdString());

	SensorLocation *loc = findSensorLocation(station, tmp.locationCode(), _origin->time());

	double delta, az, baz;
	if ( _origin )
		Geo::delazi(_origin->latitude(), _origin->longitude(),
		            loc->latitude(), loc->longitude(), &delta, &az, &baz);
	else
		delta = 0;

	item = addStream(loc, tmp, delta, tmp.stationCode().c_str(), false, true);

	fillRawPicks();

	sortByState();
	alignByState();
	componentByState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::openRecordContextMenu(const QPoint &p) {
	_currentRecord->setCurrentMarker(_currentRecord->hoveredMarker());
	PickerMarker *m = static_cast<PickerMarker*>(_currentRecord->currentMarker());
	if ( m == NULL ) return;

	if ( !m->isPick() && !m->isArrival() ) return;

	QMenu menu;

	// Save uncertainties to reset them again if changed
	// during preview
	_tmpLowerUncertainty = m->lowerUncertainty();
	_tmpUpperUncertainty = m->upperUncertainty();

	double lowerUncertainty = _tmpLowerUncertainty;
	double upperUncertainty = _tmpUpperUncertainty;

	QAction *defineUncertainties = NULL;
	QAction *deleteArrival = NULL;
	QAction *deleteArrivalWithRemove = NULL;
	QAction *removePick = NULL;
	QAction *createArrival = NULL;

	if ( !m->pick() || m->isEnabled() ) {
		QMenu *menuPolarity = menu.addMenu("Polarity");

		QMenu *menuUncertainty = menu.addMenu("Uncertainty");

		if ( _actionsUncertainty ) {
			connect(menuUncertainty, SIGNAL(hovered(QAction*)),
			        this, SLOT(previewUncertainty(QAction*)));

			foreach ( QAction *action, _actionsUncertainty->actions() )
				menuUncertainty->addAction(action);
			menuUncertainty->addSeparator();
		}

		defineUncertainties = menuUncertainty->addAction("Define...");

		menuPolarity->addAction(_ui.actionSetPolarityPositive);
		menuPolarity->addAction(_ui.actionSetPolarityNegative);
		menuPolarity->addAction(_ui.actionSetPolarityUndecidable);
		menuPolarity->addAction(_ui.actionSetPolarityUnset);
	}

	bool needSeparator = !menu.isEmpty();

	if ( !_currentRecord->cursorText().isEmpty() &&
	     (m->isPick() || (m->isArrival() && m->text() != _currentRecord->cursorText())) ) {
		if ( needSeparator ) { menu.addSeparator(); needSeparator = false; }
		createArrival = menu.addAction(QString("Declare %1 arrival").arg(_currentRecord->cursorText()));
	}

	if ( m->isArrival() ) {
		if ( needSeparator ) { menu.addSeparator(); needSeparator = false; }
		deleteArrival = menu.addAction("Delete arrival");
		if ( _loadedPicks )
			deleteArrivalWithRemove = menu.addAction("Delete arrival and remove pick");
	}
	else if ( m->isPick() ) {
		if ( needSeparator ) { menu.addSeparator(); needSeparator = false; }
		removePick = menu.addAction("Remove pick");
	}

	QList<QAction*> plugins;
	if ( !_markerPlugins.empty() ) {
		menu.addSeparator();
		foreach ( PickerMarkerActionPlugin *plugin, _markerPlugins ) {
			QAction *action = menu.addAction(plugin->title());
			action->setData(qVariantFromValue((void*)plugin));
			plugins.append(action);
		}
	}

	QAction *res = menu.exec(_currentRecord->mapToGlobal(p));

	if ( res == NULL ) {
		m->setUncertainty(_tmpLowerUncertainty, _tmpUpperUncertainty);
		m->update();
		_currentRecord->update();
		return;
	}

	if ( (res == deleteArrival) || (res == deleteArrivalWithRemove) ) {
		if ( _currentRecord->currentMarker() ) {
			m = static_cast<PickerMarker*>(_currentRecord->currentMarker());
			if ( m->isArrival() ) {
				if ( m->isMovable() || !_loadedPicks ) {
					delete m;
					m = NULL;
				}
				else
					m->setType(PickerMarker::Pick);
			}

			if ( m && res == deleteArrivalWithRemove ) delete m;

			_currentRecord->update();
			if ( _recordView->currentItem() ) _recordView->currentItem()->widget()->update();
		}

		return;
	}
	else if ( res == removePick ) {
		if ( _currentRecord->currentMarker() ) {
			m = static_cast<PickerMarker*>(_currentRecord->currentMarker());
			/*
			// Remove the pick from the pick list to avoid loading of
			// the marker again if fillRawPicks is called
			if ( m->pick() ) {
				std::vector<DataModel::PickPtr>::iterator it;
				it = std::find(_picksInTime.begin(), _picksInTime.end(), m->pick());
				if ( it != _picksInTime.end() ) _picksInTime.erase(it);
			}
			*/
			delete m;
			_currentRecord->update();
		}
		return;
	}
	else if ( res == createArrival ) {
		if ( _currentRecord->currentMarker() ) {
			m = static_cast<PickerMarker*>(_currentRecord->currentMarker());
			declareArrival(m, _currentRecord->cursorText(), true);
		}

		return;
	}

	// Reset uncertainties again
	m->setUncertainty(lowerUncertainty, upperUncertainty);

	// Fetch the current marker if changed in an action handler
	m = static_cast<PickerMarker*>(_currentRecord->currentMarker());

	if ( res == defineUncertainties ) {
		EditUncertainties dlg(this);

		dlg.setUncertainties(_tmpLowerUncertainty, _tmpUpperUncertainty);
		connect(&dlg, SIGNAL(uncertaintiesChanged(double, double)),
		        this, SLOT(previewUncertainty(double, double)));

		int res = dlg.exec();

		m->setUncertainty(_tmpLowerUncertainty, _tmpUpperUncertainty);
		m->update();

		if ( res == QDialog::Accepted ) {
			// Create a new pick if the current marker refers to an existing
			// pick already
			if ( m->pick() ) {
				PickerMarker *old = m;
				m = new PickerMarker(old->parent(), *old);
				m->convertToManualPick();
				old->setType(PickerMarker::Pick);
				old->parent()->setCurrentMarker(m);
			}
			m->setUncertainty(dlg.lowerUncertainty(), dlg.upperUncertainty());
		}

		updateUncertaintyHandles(m);

		_currentRecord->update();
		_recordView->currentItem()->widget()->update();
	}
	else {
		m->setUncertainty(_tmpLowerUncertainty, _tmpUpperUncertainty);
		updateUncertaintyHandles(m);
		m->update();
		_currentRecord->update();
	}

	if ( plugins.contains(res) ) {
		PickerMarkerActionPlugin *plugin;
		plugin = static_cast<PickerMarkerActionPlugin*>(res->data().value<void*>());
		if ( plugin ) {
			if ( plugin->init(_currentRecord->streamID(), m->time()) ) {
				PickerRecordLabel *label = static_cast<PickerRecordLabel*>(_recordView->currentItem()->label());
				RecordSequence *seqZ = label->data.traces[0].raw;
				RecordSequence *seq1 = label->data.traces[1].raw;
				RecordSequence *seq2 = label->data.traces[2].raw;

				plugin->setRecords(seqZ, seq1, seq2);
				plugin->finalize();
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::currentMarkerChanged(Seiscomp::Gui::RecordMarker *m) {
	updateUncertaintyHandles(m);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateUncertaintyHandles(RecordMarker *marker) {
	if ( marker == NULL ) {
		_timeScale->setSelectionEnabled(false);
		return;
	}

	PickerMarker *m = static_cast<PickerMarker*>(marker);
	bool canChangeUncertainty = true;

	if ( !m->isPick() && !m->isArrival() ) canChangeUncertainty = false;
	if ( m->pick() ) canChangeUncertainty = false;
	if ( !m->hasUncertainty() ) canChangeUncertainty = false;

	if ( canChangeUncertainty ) {
		_timeScale->setSelectionEnabled(true);
		_timeScale->setSelectionHandle(0, double(m->correctedTime()-_timeScale->alignment())-m->lowerUncertainty());
		_timeScale->setSelectionHandle(1, double(m->correctedTime()-_timeScale->alignment())+m->upperUncertainty());
	}
	else
		_timeScale->setSelectionEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::previewUncertainty(QAction *action) {
	bool ok;
	int idx = action->data().toInt(&ok);
	if ( !ok ) {
		// Reset
		previewUncertainty(_tmpLowerUncertainty, _tmpUpperUncertainty);
		return;
	}

	if ( idx == -1 )
		previewUncertainty(-1,-1);
	else if ( idx < 0 || idx >= _uncertainties.count() )
		return;
	else
		previewUncertainty(_uncertainties[idx].first, _uncertainties[idx].second);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::previewUncertainty(double lower, double upper) {
	PickerMarker *m = static_cast<PickerMarker*>(_currentRecord->currentMarker());

	m->setUncertainty(lower, upper);
	updateUncertaintyHandles(m);
	_currentRecord->update();
	_recordView->currentItem()->widget()->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::openConnectionInfo(const QPoint &p) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RecordViewItem* PickerView::addRawStream(const DataModel::SensorLocation *loc,
                                         const WaveformStreamID& sid,
                                         double distance,
                                         const std::string& text,
                                         bool theoreticalArrivals) {
	WaveformStreamID streamID(sid);

	// Lookup station channel mapping
	QList<Config::ChannelMapItem> channelMapping = _config.channelMap.values((streamID.networkCode() + "." + streamID.stationCode()).c_str());
	if ( channelMapping.isEmpty() )
		channelMapping = _config.channelMap.values((std::string("*.") + streamID.stationCode()).c_str());
	if ( channelMapping.isEmpty() )
		channelMapping = _config.channelMap.values((streamID.networkCode() + ".*").c_str());

	if ( !channelMapping.isEmpty() ) {
		QString channel = streamID.channelCode().substr(0,2).c_str();
		QString locChannel = (streamID.locationCode() + "." + streamID.channelCode().substr(0,2)).c_str();
		QListIterator<Config::ChannelMapItem> it(channelMapping);
		for ( it.toBack(); it.hasPrevious(); ) {
			const Config::ChannelMapItem &value = it.previous();
			if ( value.first == locChannel || value.first == channel ) {
				QStringList toks = value.second.split('.');
				if ( toks.size() == 1 )
					streamID.setChannelCode(toks[0].toStdString() + streamID.channelCode().substr(2));
				else if ( toks.size() == 2 ) {
					streamID.setLocationCode(toks[0].toStdString());
					streamID.setChannelCode(toks[1].toStdString() + streamID.channelCode().substr(2));
				}
				else
					SEISCOMP_WARNING("Invalid channel mapping target: %s", value.second.toStdString().c_str());

				break;
			}
		}
	}

	RecordViewItem* item = _recordView->addItem(adjustWaveformStreamID(streamID), text.c_str());
	if ( item == NULL ) return NULL;

	connect(item, SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(openContextMenu(const QPoint &)));

	if ( _currentRecord )
		item->widget()->setCursorText(_currentRecord->cursorText());

	item->label()->setText(text.c_str(), 0);
	QFont f(item->label()->font(0));
	f.setBold(true);
	item->label()->setFont(f, 0);

	QFontMetrics fm(f);
	item->label()->setWidth(fm.boundingRect("WWWW ").width(), 0);

	ThreeComponents tc;
	char comps[3] = {'Z', '1', '2'};

	PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
	label->data.setRecordWidget(item->widget());

	bool allComponents = true;

	applyFilter(item);

	if ( loc ) {
		getThreeComponents(tc, loc, streamID.channelCode().substr(0, streamID.channelCode().size()-1).c_str(), _origin->time());
		if ( tc.comps[ThreeComponents::Vertical] )
			comps[0] = *tc.comps[ThreeComponents::Vertical]->code().rbegin();
		else {
			allComponents = false;
			comps[0] = COMP_NO_METADATA;
		}

		if ( tc.comps[ThreeComponents::FirstHorizontal] )
			comps[1] = *tc.comps[ThreeComponents::FirstHorizontal]->code().rbegin();
		else {
			allComponents = false;
			comps[1] = COMP_NO_METADATA;
		}

		if ( tc.comps[ThreeComponents::SecondHorizontal] )
			comps[2] = *tc.comps[ThreeComponents::SecondHorizontal]->code().rbegin();
		else {
			allComponents = false;
			comps[2] = COMP_NO_METADATA;
		}

		label->latitude = loc->latitude();
		label->longitude = loc->longitude();

		double delta, az, baz;
		Geo::delazi(_origin->latitude(), _origin->longitude(),
		            label->latitude, label->longitude, &delta, &az, &baz);

		label->orientationZRT.loadRotateZ(deg2rad(baz + 180.0));
	}
	else {
		label->latitude = 999;
		label->longitude = 999;
		label->orientationZRT.identity();
		allComponents = false;
	}

	if ( !allComponents )
		SEISCOMP_WARNING("Unable to fetch all components of stream %s.%s.%s.%s",
		                 streamID.networkCode().c_str(), streamID.stationCode().c_str(),
		                 streamID.locationCode().c_str(), streamID.channelCode().substr(0,streamID.channelCode().size()-1).c_str());

	item->setData(QVariant(QString(text.c_str())));
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

	applyRotation(item, _comboRotation->currentIndex());

	for ( int i = 0; i < 3; ++i ) {
		WaveformStreamID componentID = setWaveformIDComponent(streamID, comps[i]);
		label->data.traces[i].channelCode = componentID.channelCode();
		// Map waveformID to recordviewitem label
		_recordItemLabels[waveformIDToStdString(componentID)] = label;
	}

	if ( theoreticalArrivals )
		addTheoreticalArrivals(item, streamID.networkCode(), streamID.stationCode(), streamID.locationCode());

	for ( int i = 0; i < 3; ++i ) {
		if ( comps[i] == COMP_NO_METADATA ) continue;
		queueStream(distance, setWaveformIDComponent(streamID, comps[i]), Z12_COMPS[i]);
	}

	return item;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
char PickerView::currentComponent() const {
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
void PickerView::setupItem(const char comps[3],
                           RecordViewItem* item) {
	connect(item->widget(), SIGNAL(cursorUpdated(RecordWidget*,int)),
	        this, SLOT(updateMainCursor(RecordWidget*,int)));

	connect(item, SIGNAL(componentChanged(RecordViewItem*, char)),
	        this, SLOT(updateItemLabel(RecordViewItem*, char)));

	connect(item, SIGNAL(firstRecordAdded(const Seiscomp::Record*)),
	        this, SLOT(updateItemRecordState(const Seiscomp::Record*)));

	item->label()->setOrientation(Qt::Horizontal);
	item->label()->setToolTip("Timing quality: undefined");

	QPalette pal = item->widget()->palette();
	pal.setColor(QPalette::WindowText, QColor(128,128,128));
	//pal.setColor(QPalette::HighlightedText, QColor(128,128,128));
	item->widget()->setPalette(pal);

	item->widget()->setCustomBackgroundColor(SCScheme.colors.records.states.unrequested);

	item->widget()->setSlotCount(3);

	for ( int i = 0; i < 3; ++i ) {
		if ( comps[i] != COMP_NO_METADATA )
			item->insertComponent(comps[i], i);
		else
			item->widget()->setRecordID(i, "No metadata");
	}

	Client::Inventory *inv = Client::Inventory::Instance();
	if ( inv ) {
		std::string channelCode = item->streamID().channelCode().substr(0,2);
		for ( int i = 0; i < 3; ++i ) {
			if ( comps[i] == COMP_NO_METADATA ) continue;
			Processing::Stream stream;
			try {
				stream.init(item->streamID().networkCode(),
				            item->streamID().stationCode(),
				            item->streamID().locationCode(),
				            channelCode + comps[i], _origin->time().value());
				if ( stream.gain > 0 )
					item->widget()->setRecordScale(i, 1E9 / stream.gain);
			}
			catch ( ... ) {}
		}
	}

	item->widget()->showScaledValues(_ui.actionShowTraceValuesInNmS->isChecked());

	// Default station distance is INFINITY to sort unknown stations
	// to the end of the view
	item->setValue(ITEM_DISTANCE_INDEX, std::numeric_limits<double>::infinity());
	// Default residual set to -INFINITY
	item->setValue(ITEM_RESIDUAL_INDEX, 0);
	// Default azimuth set to -INFINITY
	item->setValue(ITEM_AZIMUTH_INDEX, -std::numeric_limits<double>::infinity());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateMainCursor(RecordWidget* w, int s) {
	char comps[3] = {'Z', '1', '2'};
	int slot = s >= 0 && s < 3?s:-1;

	if ( slot != -1 && slot != _currentSlot )
		showComponent(comps[slot]);

	setCursorPos(w->cursorPos(), true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateSubCursor(RecordWidget* w, int s) {
	char comps[3] = {'Z', '1', '2'};
	int slot = s >= 0 && s < 3?s:-1;

	if ( slot != -1 && slot != _currentSlot )
		showComponent(comps[slot]);

	if ( _recordView->currentItem() == NULL ) return;

	_recordView->currentItem()->widget()->blockSignals(true);
	_recordView->currentItem()->widget()->setCursorPos(w->cursorPos());
	_recordView->currentItem()->widget()->blockSignals(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateRecordValue(Seiscomp::Core::Time t) {
	if ( !statusBar() ) return;

	const double *v = _currentRecord->value(t);

	if ( v == NULL )
		statusBar()->clearMessage();
	else
		statusBar()->showMessage(QString("value = %1").arg(*v, 0, 'f', 2));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showTraceScaleToggled(bool e) {
	_currentRecord->showScaledValues(e);
	for ( int i = 0; i < _recordView->rowCount(); ++i ) {
		RecordViewItem* item = _recordView->itemAt(i);
		item->widget()->showScaledValues(e);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateItemLabel(RecordViewItem* item, char component) {
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

	int slot = item->mapComponentToSlot(component);

	if ( item == _recordView->currentItem() ) {
		QString text = _ui.labelCode->text();

		int index = text.lastIndexOf(' ');
		if ( index < 0 ) return;

		char comp = component;

		if ( slot >= 0 && slot < 3 ) {
			switch ( _comboRotation->currentIndex() ) {
				case RT_Z12:
					break;
				case RT_ZNE:
					comp = ZNE_COMPS[slot];
					break;
				case RT_ZRT:
					comp = ZRT_COMPS[slot];
					break;
				case RT_ZH:
					comp = ZH_COMPS[slot];
					break;
			}
		}

		if ( text.size() - index > 2 )
			text[text.size()-1] = comp;
		else
			text += comp;

		_ui.labelCode->setText(text);
	}

	updateTraceInfo(item, NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateItemRecordState(const Seiscomp::Record *rec) {
	// Reset acquisition related coloring since the first record
	// arrived already
	RecordViewItem *item = static_cast<RecordViewItem *>(sender());
	RecordWidget *widget = item->widget();
	int slot = item->mapComponentToSlot(*rec->channelCode().rbegin());
	widget->setRecordBackgroundColor(slot, SCScheme.colors.records.states.inProgress);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setCursorPos(const Seiscomp::Core::Time& t, bool always) {
	_currentRecord->setCursorPos(t);

	if ( !always && _currentRecord->cursorText() == "" ) return;

	float offset = 0;

	if ( _centerSelection ) {
		float len = _recordView->currentItem()?
			_recordView->currentItem()->widget()->width()/_currentRecord->timeScale():
			_currentRecord->tmax() - _currentRecord->tmin();

		float pos = float(t - _currentRecord->alignment()) - len*_config.alignmentPosition;
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
void PickerView::setTimeRange(float tmin, float tmax) {
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
void PickerView::enableAutoScale() {
	_autoScaleZoomTrace = true;
	if ( _currentRecord ) {
		float amplScale = _currentRecord->amplScale();
		_currentRecord->setNormalizationWindow(_currentRecord->visibleTimeWindow());
		_currentRecord->setAmplScale(amplScale);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::disableAutoScale() {
	_autoScaleZoomTrace = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setAlignment(Seiscomp::Core::Time t) {
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
void PickerView::ensureVisibility(float& tmin, float& tmax) {
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
void PickerView::ensureVisibility(const Seiscomp::Core::Time &time, int pixelMargin) {
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
void PickerView::moveTraces(double offset) {
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
void PickerView::move(double offset) {
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
void PickerView::scroll(int offset) {
	_currentRecord->setTracePaintOffset(-offset);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::itemSelected(RecordViewItem* item, RecordViewItem* lastItem) {
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
		_currentRecord->clearRecords();
		_currentRecord->setEnabled(false);
		_currentRecord->setMarkerSourceWidget(NULL);
		static_cast<ZoomRecordWidget*>(_currentRecord)->setTraces(NULL);
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

	connect(item->label(), SIGNAL(statusChanged(bool)),
	        this, SLOT(setCurrentRowEnabled(bool)));

	double amplScale = _currentRecord->amplScale();

	_currentRecord->setNormalizationWindow(item->widget()->normalizationWindow());
	_currentRecord->setAlignment(item->widget()->alignment());
	_timeScale->setAlignment(item->widget()->alignment());

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

	if ( item->value(ITEM_DISTANCE_INDEX) >= 0 ) {
		if ( _config.showAllComponents &&
		     _config.allComponentsMaximumStationDistance >= item->value(ITEM_DISTANCE_INDEX) )
			_currentRecord->setDrawMode(RecordWidget::InRows);
		else
			_currentRecord->setDrawMode(RecordWidget::Single);
	}
	else
		_currentRecord->setDrawMode(RecordWidget::Single);

	/*
	_currentRecord->clearMarker();
	for ( int i = 0; i < item->widget()->markerCount(); ++i )
		new PickerMarker(_currentRecord, *static_cast<PickerMarker*>(item->widget()->marker(i)));
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
	int slot = item->mapComponentToSlot(component);

	if ( slot >= 0 && slot < 3 ) {
		switch ( _comboRotation->currentIndex() ) {
			case RT_Z12:
				break;
			case RT_ZNE:
				component = ZNE_COMPS[slot];
				break;
			case RT_ZRT:
				component = ZRT_COMPS[slot];
				break;
			case RT_ZH:
				component = ZH_COMPS[slot];
				break;
		}
	}

	for ( int i = 0; i < item->widget()->slotCount(); ++i ) {
		char code = _recordView->currentItem()->mapSlotToComponent(i);
		if ( code == '?' ) continue;

		switch ( _comboRotation->currentIndex() ) {
			case RT_Z12:
				_currentRecord->setRecordID(i, QString("%1").arg(code));
				break;
			case RT_ZNE:
				_currentRecord->setRecordID(i, QString("%1").arg(ZNE_COMPS[i]));
				break;
			case RT_ZRT:
				_currentRecord->setRecordID(i, QString("%1").arg(ZRT_COMPS[i]));
				break;
			case RT_ZH:
				_currentRecord->setRecordID(i, QString("%1").arg(ZH_COMPS[i]));
				break;
		}
	}

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

	PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
	static_cast<ZoomRecordWidget*>(_currentRecord)->setTraces(label->data.traces);

	currentMarkerChanged(_currentRecord->currentMarker());

#ifdef CENTER_SELECTION
	_centerSelection = true;
#endif
	//_currentRecord->enableFiltering(item->widget()->isFilteringEnabled());
	setCursorPos(cursorPos);
	_currentRecord->update();

	updateCurrentRowState();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setCurrentRowEnabled(bool enabled) {
	_currentRecord->setEnabled(enabled);
	updateCurrentRowState();

	RecordWidget* w = _recordView->currentItem()->widget();

	if ( w ) {
		for ( int i = 0; i < w->markerCount(); ++i ) {
			if ( w->marker(i)->id() >= 0 ) {
				emit arrivalChanged(w->marker(i)->id(), enabled?w->marker(i)->isEnabled():false);
				// To disable an arrival (trace) we have to keep this information
				// somewhere else not just in the picker
				emit arrivalEnableStateChanged(w->marker(i)->id(), enabled);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setCurrentRowDisabled(bool disabled) {
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
void PickerView::setMarkerState(Seiscomp::Gui::RecordWidget* w, bool enabled) {
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
				emit arrivalChanged(arid, enabled);
				//emit arrivalEnableStateChanged(arid, enabled);
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
void PickerView::updateCurrentRowState() {
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
void PickerView::updateTraceInfo(RecordViewItem* item,
                                 const Seiscomp::Record* rec) {
	float timingQuality = item->widget()->timingQuality(_currentSlot);
	if ( timingQuality >= 0 ) {
		if ( timingQuality > 100 ) timingQuality = 100;

		if ( timingQuality < 50 )
			static_cast<PickerRecordLabel*>(item->label())->setLabelColor(blend(_config.timingQualityMedium, _config.timingQualityLow, (int)(timingQuality*2)));
		else
			static_cast<PickerRecordLabel*>(item->label())->setLabelColor(blend(_config.timingQualityHigh, _config.timingQualityMedium, (int)((timingQuality-50)*2)));

		item->label()->setToolTip(QString("Timing quality: %1").arg((int)timingQuality));
	}
	else {
		static_cast<PickerRecordLabel*>(item->label())->removeLabelColor();
		item->label()->setToolTip("Timing quality: undefined");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::toggleFilter() {
	if ( _comboFilter->currentIndex() > 0 )
		_comboFilter->setCurrentIndex(0);
	else if ( _lastFilterIndex > 0 )
		_comboFilter->setCurrentIndex(_lastFilterIndex);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::nextFilter() {
	// Filtering turned off
	int idx = _comboFilter->currentIndex();
	if ( idx == 0 ) return;

	++idx;
	if ( idx >= _comboFilter->count() )
		idx = 1;

	_comboFilter->setCurrentIndex(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::previousFilter() {
	// Filtering turned off
	int idx = _comboFilter->currentIndex();
	if ( idx == 0 ) return;

	--idx;
	if ( idx < 1 )
		idx = _comboFilter->count()-1;

	_comboFilter->setCurrentIndex(idx);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::addNewFilter(const QString& filter) {
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
void PickerView::scaleVisibleAmplitudes() {
	_recordView->scaleVisibleAmplitudes();

	_currentRecord->setNormalizationWindow(_currentRecord->visibleTimeWindow());
	_currentAmplScale = 1;
	//_currentRecord->resize(_zoomTrace->width(), (int)(_zoomTrace->height()*_currentAmplScale));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::zoomSelectionHandleMoved(int handle, double pos, Qt::KeyboardModifiers mods) {
	PickerMarker *m = static_cast<PickerMarker*>(_currentRecord->currentMarker());
	if ( m == NULL ) return;

	double trigger = double(m->correctedTime() - _timeScale->alignment());
	double value = pos - trigger;

	// Clip to a hundredth of a seconds
	if ( mods & Qt::ControlModifier ) {
		value *= 1E2;
		value = Math::round(value);
		value *= 1E-2;
	}
	// Clip to a tenthousandth of a seconds
	else {
		value *= 1E4;
		value = Math::round(value);
		value *= 1E-4;
	}

	switch ( handle ) {
		case 0:
			if ( value > 0 ) value = 0;

			if ( mods & Qt::ShiftModifier )
				m->setUncertainty(-value, -value);
			else
				m->setUncertainty(-value, m->upperUncertainty());

			m->setDrawUncertaintyValues(true);
			m->update();
			_currentRecord->update();

			_timeScale->setSelectionHandle(0, trigger-m->lowerUncertainty());
			_timeScale->setSelectionHandle(1, trigger+m->upperUncertainty());
			break;
		case 1:
			if ( value < 0 ) value = 0;

			if ( mods & Qt::ShiftModifier )
				m->setUncertainty(value, value);
			else
				m->setUncertainty(m->lowerUncertainty(), value);

			m->setDrawUncertaintyValues(true);
			m->update();
			_currentRecord->update();

			_timeScale->setSelectionHandle(0, trigger-m->lowerUncertainty());
			_timeScale->setSelectionHandle(1, trigger+m->upperUncertainty());
			break;
		default:
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::zoomSelectionHandleMoveFinished() {
	PickerMarker *m = static_cast<PickerMarker*>(_currentRecord->currentMarker());
	if ( m == NULL ) return;

	m->setDrawUncertaintyValues(false);
	m->update();
	_currentRecord->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::changeScale(double, float) {
	zoom(1.0);
	/*
	std::cout << "[changeScale]" << std::endl;
	std::cout << "new TimeScale(" << _currentRecord->timeScale() << ")" << std::endl;
	std::cout << "current TimeRange = " << _currentRecord->tmin() << ", " << _currentRecord->tmax() << ")" << std::endl;
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::changeTimeRange(double, double) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::sortAlphabetically() {
	_recordView->sortByTextAndValue(0, ITEM_PRIORITY_INDEX);

	_ui.actionSortAlphabetically->setChecked(true);
	_ui.actionSortByDistance->setChecked(false);
	_ui.actionSortByAzimuth->setChecked(false);
	_ui.actionSortByResidual->setChecked(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::sortByDistance() {
	_recordView->sortByValue(ITEM_DISTANCE_INDEX, ITEM_PRIORITY_INDEX);

	_ui.actionSortAlphabetically->setChecked(false);
	_ui.actionSortByDistance->setChecked(true);
	_ui.actionSortByAzimuth->setChecked(false);
	_ui.actionSortByResidual->setChecked(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::sortByAzimuth() {
	_recordView->sortByValue(ITEM_AZIMUTH_INDEX, ITEM_PRIORITY_INDEX);

	_ui.actionSortAlphabetically->setChecked(false);
	_ui.actionSortByDistance->setChecked(false);
	_ui.actionSortByAzimuth->setChecked(true);
	_ui.actionSortByResidual->setChecked(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::sortByResidual() {
	_recordView->sortByValue(ITEM_RESIDUAL_INDEX);

	_ui.actionSortAlphabetically->setChecked(false);
	_ui.actionSortByDistance->setChecked(false);
	_ui.actionSortByAzimuth->setChecked(false);
	_ui.actionSortByResidual->setChecked(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::sortByPhase(const QString& phase) {
	_recordView->sortByMarkerTime(phase);

	_ui.actionSortAlphabetically->setChecked(false);
	_ui.actionSortByDistance->setChecked(false);
	_ui.actionSortByAzimuth->setChecked(false);
	_ui.actionSortByResidual->setChecked(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showZComponent() {
	showComponent('Z');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showNComponent() {
	showComponent('1');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showEComponent() {
	showComponent('2');
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::alignOnOriginTime() {
	_checkVisibility = false;
	_recordView->setAbsoluteTimeEnabled(true);
	_recordView->setTimeRange(_minTime, _maxTime);
	_checkVisibility = true;

	_recordView->setAlignment(_origin->time());

	_alignedOnOT = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::alignOnPArrivals() {
	alignOnPhase("P", false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::alignOnSArrivals() {
	alignOnPhase("S", false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::pickP(bool) {
	setCursorText("P");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::pickNone(bool) {
	setCursorText("");
	if ( _recordView->currentItem() )
		_recordView->currentItem()->widget()->setCurrentMarker(NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::pickS(bool) {
	setCursorText("S");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::scaleAmplUp() {
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
void PickerView::scaleAmplDown() {
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
void PickerView::scaleReset() {
	_currentRecord->setAmplScale(1.0);
	_currentAmplScale = 1.0;
	zoom(0.0);

	//_currentRecord->resize(_zoomTrace->width(), (int)(_zoomTrace->height()*_currentAmplScale));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::scaleTimeUp() {
	zoom(_recordView->zoomFactor());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::scaleTimeDown() {
	zoom(1.0/_recordView->zoomFactor());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::zoom(float factor) {
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
void PickerView::applyTimeRange(double rmin, double rmax) {
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
void PickerView::scrollLeft() {
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
void PickerView::scrollFineLeft() {
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
void PickerView::scrollRight() {
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
void PickerView::scrollFineRight() {
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
void PickerView::automaticRepick() {
	if ( _comboPicker == NULL ) {
		statusBar()->showMessage("Automatic picking: no picker available", 2000);
		return;
	}

	if ( _recordView->currentItem() == NULL ) {
		statusBar()->showMessage("Automatic picking: no active row", 2000);
		return;
	}

	if ( !_currentRecord->cursorText().isEmpty() ) {
		Core::Time cp = _currentRecord->cursorPos();

		RecordSequence *seq =
			_currentRecord->isFilteringEnabled()
				?
				_currentRecord->filteredRecords(_currentSlot)
				:
				_currentRecord->records(_currentSlot);
		if ( seq ) {
			Processing::PickerPtr picker =
				Processing::PickerFactory::Create(_comboPicker->currentText().toStdString().c_str());

			if ( picker == NULL ) {
				statusBar()->showMessage(QString("Automatic picking: unable to create picker '%1'").arg(_comboPicker->currentText()), 2000);
				return;
			}

			WaveformStreamID wid = _recordView->streamID(_recordView->currentItem()->row());

			if ( !picker->setup(Processing::Settings(SCApp->configModuleName(),
			                    wid.networkCode(), wid.stationCode(),
			                    wid.locationCode(), wid.channelCode(), &SCApp->configuration(),
			                    NULL)) ) {
				statusBar()->showMessage("Automatic picking: unable to inialize picker");
				return;
			}

			if ( _config.repickerSignalStart ) {
				picker->setSignalStart(*_config.repickerSignalStart);
				SEISCOMP_DEBUG("Set repick start to %.2f", *_config.repickerSignalStart);
			}
			if ( _config.repickerSignalEnd ) {
				picker->setSignalEnd(*_config.repickerSignalEnd);
				SEISCOMP_DEBUG("Set repick end to %.2f", *_config.repickerSignalEnd);
			}

			picker->setTrigger(cp);
			picker->setPublishFunction(boost::bind(&PickerView::emitPick, this, _1, _2));
			picker->computeTimeWindow();
			int count = picker->feedSequence(seq);
			statusBar()->showMessage(QString("Fed %1 of %2 records: state = %3(%4)")
			                         .arg(count)
			                         .arg(seq->size())
			                         .arg(picker->status().toString())
			                         .arg(picker->statusValue()), 2000);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::gotoNextMarker() {
	//if ( _currentRecord->cursorText().isEmpty() ) return;
	bool active = !_currentRecord->cursorText().isEmpty();

	RecordMarker *m = _currentRecord->nextMarker(_currentRecord->cursorPos());
	if ( m ) {
		bool oldCenter = _centerSelection;
		_centerSelection = active;
		setCursorPos(m->correctedTime());
		_currentRecord->setCurrentMarker(m);
		_centerSelection = oldCenter;

		ensureVisibility(m->correctedTime(), 5);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::gotoPreviousMarker() {
	//if ( _currentRecord->cursorText().isEmpty() ) return;
	bool active = !_currentRecord->cursorText().isEmpty();

	RecordMarker *m = _currentRecord->lastMarker(_currentRecord->cursorPos());
	if ( m ) {
		bool oldCenter = _centerSelection;
		_centerSelection = active;
		setCursorPos(m->correctedTime());
		_currentRecord->setCurrentMarker(m);
		_centerSelection = oldCenter;

		ensureVisibility(m->correctedTime(), 5);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::hasModifiedPicks() const {
	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem* rvi = _recordView->itemAt(r);
		RecordWidget* widget = rvi->widget();
		for ( int m = 0; m < widget->markerCount(); ++m ) {
			RecordMarker* marker = widget->marker(m);
			if ( marker->isModified() )
				return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::fetchManualPicks(std::vector<RecordMarker*>* markers) const {
	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem* rvi = _recordView->itemAt(r);
		RecordWidget* widget = rvi->widget();

		if ( !widget->isEnabled() ) continue;

		// The number of markers for a given phase
		QMap<QString, bool> manualMarkers;
		bool hasManualPick = false;

		// Count the number of interesting markers for a particular phase
		for ( int m = 0; m < widget->markerCount(); ++m ) {
			PickerMarker* marker = (PickerMarker*)widget->marker(m);
			//if ( (marker->isMovable() && (marker->isEnabled() || marker->pick() != NULL)) )
			if ( marker->isArrival() && (marker->pick() == NULL) )
				manualMarkers[marker->text()] = true;
		}

		for ( int m = 0; m < widget->markerCount(); ++m ) {
			PickerMarker* marker = (PickerMarker*)widget->marker(m);

			// If the marker is not an arrival do nothing
			if ( !marker->isArrival() ) continue;

			bool hasManualMarkerForPhase = manualMarkers[marker->text()];

			PickPtr pick = marker->pick();

			// Picked marker and we've got an manual replacement: do nothing
			if ( hasManualMarkerForPhase && marker->pick() ) {
				SEISCOMP_DEBUG("   - ignore pick to be replaced");
				marker->setId(-1);
				continue;
			}

			/*
			// If the marker is not enabled do nothing and reset its bound arrival id
			if ( !marker->isEnabled() ) {
				// Preset markers are ignored when there is no manual arrival
				if ( hasManualMarkerForPhase && !marker->isMovable() )
					marker->setId(-1);
				else {
					if ( markers )
						markers->push_back(marker);
					SEISCOMP_DEBUG("   - reuse existing pick");
				}

				continue;
			}
			*/

			if ( pick ) {
				SEISCOMP_DEBUG("Checking existing pick, modified = %d", marker->isModified());
				if ( !marker->isModified() ) {
					if ( markers )
						markers->push_back(marker);
					SEISCOMP_DEBUG("   - reuse existing pick");
					continue;
				}
			}

			PickPtr p = findPick(widget, marker->correctedTime());
			// If the marker did not make any changes to the pick
			// attributes (polariy, uncertainty, ...) reuse it.
			if ( p && !marker->equalsPick(p.get()) ) p = NULL;

			if ( !p ) {
				WaveformStreamID s = _recordView->streamID(r);
				p = Pick::Create();
				p->setWaveformID(WaveformStreamID(s.networkCode(), s.stationCode(),
				                                  s.locationCode(), s.channelCode().substr(0,2), ""));

#ifdef SET_PICKED_COMPONENT
				if ( marker->slot() >= 0 && marker->slot() < 3 ) {
					char comp;
					switch ( marker->rotation() ) {
						default:
						case RT_Z12:
							comp = rvi->mapSlotToComponent(marker->slot());
							break;
						case RT_ZNE:
							comp = ZNE_COMPS[marker->slot()];
							break;
						case RT_ZRT:
							comp = ZRT_COMPS[marker->slot()];
							break;
						case RT_ZH:
							comp = ZH_COMPS[marker->slot()];
							break;
					}

					p->waveformID().setChannelCode(p->waveformID().channelCode() + comp);
				}
#endif

				p->setTime(marker->correctedTime());

				if ( marker->lowerUncertainty() >= 0 )
					p->time().setLowerUncertainty(marker->lowerUncertainty());
				if ( marker->upperUncertainty() >= 0 )
					p->time().setUpperUncertainty(marker->upperUncertainty());

				if ( !marker->filter().isEmpty() )
					p->setFilterID(marker->filter().toStdString());
				p->setPhaseHint(Phase((const char*)marker->text().toAscii()));
				p->setEvaluationMode(EvaluationMode(MANUAL));
				p->setPolarity(marker->polarity());
				CreationInfo ci;
				ci.setAgencyID(SCApp->agencyID());
				ci.setAuthor(SCApp->author());
				ci.setCreationTime(Core::Time::GMT());
				p->setCreationInfo(ci);

				_changedPicks.push_back(ObjectChangeList<DataModel::Pick>::value_type(p,true));
				SEISCOMP_DEBUG("   - created new pick");

				hasManualPick = true;
			}
			else {
				SEISCOMP_DEBUG("   - reuse active pick");
			}

			if ( markers ) markers->push_back(marker);
			marker->setPick(p.get());
		}

		// Remove automatic station picks if configured
		if ( hasManualPick && _config.removeAutomaticStationPicks && markers ) {
			for ( std::vector<RecordMarker*>::iterator it = markers->begin();
			      it != markers->end(); ) {
				try {
					if ( static_cast<PickerMarker*>(*it)->pick()->evaluationMode() == MANUAL ) {
						++it;
						continue;
					}
				}
				catch ( ... ) {}

				if ( (*it)->parent() == widget )
					it = markers->erase(it);
				else
					++it;
			}
		}
	}

	// Remove all automatic picks if configured
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
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::applyPicks() {
	for ( int r = 0; r < _recordView->rowCount(); ++r ) {
		RecordViewItem* rvi = _recordView->itemAt(r);
		RecordWidget* widget = rvi->widget();

		for ( int m = 0; m < widget->markerCount(); ++m ) {
			RecordMarker* marker = widget->marker(m);
			marker->apply();
		}
	}

	_changedPicks.clear();
	_recordView->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::getChangedPicks(ObjectChangeList<DataModel::Pick> &list) const {
	list = _changedPicks;
	_changedPicks.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setDefaultDisplay() {
	//alignByState();
	alignOnOriginTime();
	selectFirstVisibleItem(_recordView);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/*
bool PickerView::start() {
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
void PickerView::stop() {
	_recordView->stop();
	closeThreads();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::selectTrace(const std::string &net, const std::string &code) {
	for ( int i = 0; i < _recordView->rowCount(); ++i ) {
		if ( _recordView->itemAt(i)->streamID().networkCode() != net ) continue;
		if ( _recordView->itemAt(i)->streamID().stationCode() != code ) continue;
		_recordView->setCurrentItem(_recordView->itemAt(i));
		_recordView->ensureVisible(i);
		break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::selectTrace(const Seiscomp::DataModel::WaveformStreamID &wid) {
	RecordViewItem *item = _recordView->item(wid);
	if ( item ) {
		_recordView->setCurrentItem(item);
		_recordView->ensureVisible(item->row());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::closeThreads() {
	foreach ( RecordStreamThread* t, _acquisitionThreads) {
		disconnect(t, SIGNAL(handleError(const QString &)),
		           this, SLOT(handleAcquisitionError(const QString &)));

		t->stop(true);
		SEISCOMP_DEBUG("removed finished thread %d from list", t->ID());
		delete t;
	}

	_acquisitionThreads.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::handleAcquisitionError(const QString &msg) {
	QMessageBox::critical(this, tr("Acquistion error"), msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::acquisitionFinished() {
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
			PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());

			for ( int i = 0; i < 3; ++i ) {
				if ( label->data.traces[i].thread != t ) continue;
				if ( label->data.traces[i].raw && !label->data.traces[i].raw->empty() )
					widget->removeRecordBackgroundColor(i);
				else
					widget->setRecordBackgroundColor(i, SCScheme.colors.records.states.notAvailable);
				// Reset the thread
				label->data.traces[i].thread = NULL;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::acquireStreams() {
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

	connect(t, SIGNAL(handleError(const QString &)),
	        this, SLOT(handleAcquisitionError(const QString &)));

	connect(t, SIGNAL(receivedRecord(Seiscomp::Record*)),
	        this, SLOT(receivedRecord(Seiscomp::Record*)));

	connect(t, SIGNAL(finished()),
	        this, SLOT(acquisitionFinished()));


	t->setTimeWindow(_timeWindow);

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
		else
			t->addStream(it->streamID.networkCode(),
			             it->streamID.stationCode(),
			             it->streamID.locationCode(),
			             it->streamID.channelCode());

		RecordViewItem *item = _recordView->item(adjustWaveformStreamID(it->streamID));
		if ( item ) {
			int slot = item->mapComponentToSlot(*it->streamID.channelCode().rbegin());
			item->widget()->setRecordBackgroundColor(slot, SCScheme.colors.records.states.requested);
			// Store the acquisition thread as user data
			static_cast<PickerRecordLabel*>(item->label())->data.traces[slot].thread = t;
			item->widget()->setRecordUserData(slot, qVariantFromValue((void*)t));
		}
	}

	_nextStreams.clear();

	_acquisitionThreads.push_back(t);
	t->start();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::receivedRecord(Seiscomp::Record *rec) {
	Seiscomp::RecordPtr tmp(rec);
	if ( !rec->data() ) return;

	std::string streamID = rec->streamID();
	RecordItemMap::iterator it = _recordItemLabels.find(streamID);
	if ( it == _recordItemLabels.end() )
		return;

	PickerRecordLabel *label = it->second;
	int i;
	for ( i = 0; i < 3; ++i ) {
		if ( label->data.traces[i].channelCode == rec->channelCode() ) {
			if ( label->data.traces[i].raw == NULL )
				label->data.traces[i].raw = new TimeWindowBuffer(_timeWindow);
			break;
		}
	}

	if ( i == 3 ) return;

	bool firstRecord = label->data.traces[i].raw->empty();
	if ( !label->data.traces[i].raw->feed(rec) ) return;

	if ( label->recordViewItem() == _recordView->currentItem() )
		static_cast<ZoomRecordWidget*>(_currentRecord)->feedRaw(i, rec);

	// Check for out-of-order records
	if ( (label->data.traces[i].filter || label->data.enableTransformation) &&
	     label->data.traces[i].raw->back() != (const Record*)rec ) {
		SEISCOMP_DEBUG("%s.%s.%s.%s: out of order record, reinitialize trace",
		               rec->networkCode().c_str(),
		               rec->stationCode().c_str(),
		               rec->locationCode().c_str(),
		               rec->channelCode().c_str());
		label->data.reset();
	}
	else
		label->data.transform(i, rec);

	RecordViewItem *item = label->recordViewItem();

	if ( firstRecord ) {
		item->widget()->setRecordBackgroundColor(i, SCScheme.colors.records.states.inProgress);
		label->hasGotData = true;

		if ( _config.hideStationsWithoutData )
			item->forceInvisibilty(!label->isEnabledByConfig);

		// If this item is linked to another item, enable the expand button of
		// the controller
		if ( label->isLinkedItem() && label->_linkedItem != NULL )
			static_cast<PickerRecordLabel*>(label->_linkedItem->label())->enabledExpandButton(item);
	}
	else {
		// Tell the widget to rebuild its traces
		//item->widget()->fed(i, rec);
		updateTraceInfo(item, rec);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::relocate() {
	std::vector<RecordMarker*> markers;
	fetchManualPicks(&markers);

	QMap<QString, PickerMarker*> pick2Marker;

	OriginPtr tmpOrigin = Origin::Create();
	CreationInfo ci;
	ci.setAgencyID(SCApp->agencyID());
	ci.setAuthor(SCApp->author());
	ci.setCreationTime(Core::Time::GMT());
	tmpOrigin->assign(_origin.get());
	tmpOrigin->setCreationInfo(ci);

	double rms = 0.0;
	size_t rmsCount = 0;

	for ( size_t i = 0; i < markers.size(); ++i ) {
		RecordMarker *m = markers[i];
		PickPtr pick = ((PickerMarker*)markers[i])->pick();
		if ( !pick ) {
			SEISCOMP_ERROR("Pick not set in marker");
			continue;
		}

		SensorLocation* sloc = Client::Inventory::Instance()->getSensorLocation(pick.get());

		// Remove pick, when no station is configured for this pick
		if ( !sloc /*&& !m->isEnabled()*/ ) continue;

		double delta, az1, az2;
		Geo::delazi(tmpOrigin->latitude(), tmpOrigin->longitude(),
		            sloc->latitude(), sloc->longitude(), &delta, &az1, &az2);

		ArrivalPtr a = new Arrival();

		RecordWidget *w = m->parent();
		RecordMarker *tm = w->marker(m->text() + THEORETICAL_POSTFIX);
		if ( tm ) {
			a->setTimeResidual(double(pick->time().value() - tm->time()));
			if (m->isEnabled()) {
				rms += a->timeResidual() * a->timeResidual();
				++rmsCount;
			}
		}

		a->setDistance(delta);
		a->setAzimuth(az1);
		a->setPickID(pick->publicID());
		a->setWeight(m->isEnabled()/* && markers[i]->isMovable()*/?1:0);
		a->setPhase(m->text().toStdString());
		tmpOrigin->add(a.get());
		pick2Marker[pick->publicID().c_str()] = static_cast<PickerMarker*>(markers[i]);
	}

	OriginQuality q;
	try { q = tmpOrigin->quality(); } catch ( Core::ValueException & ) {}

	if ( rmsCount > 0 )
		q.setStandardError(sqrt(rms / rmsCount));
	else
		q.setStandardError(Core::None);

	tmpOrigin->setQuality(q);

	try {
		OriginPtr o = tmpOrigin;

		if ( o ) {
			_origin = o;

			for ( size_t i = 0; i < o->arrivalCount(); ++i ) {
				PickerMarker* m = pick2Marker[o->arrival(i)->pickID().c_str()];
				if ( m ) {
					m->setId(i);
				}
			}

			emit originCreated(_origin.get());

			// Only clear the manual picks if the origin has been located
			// successfully. Otherwise the manual picks get lost after the
			// next successfully location
			_changedPicks.clear();

			setOrigin(_origin.get());
		}
		else {
			QMessageBox::critical(this, "Relocation error", "Relocation failed for some reason");
		}
	}
	catch ( Core::GeneralException& e ) {
		QMessageBox::critical(this, "Relocation error", e.what());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::modifyOrigin() {
	double dep = _config.defaultDepth;
	try { dep = _origin->depth(); } catch ( ... ) {}
	OriginDialog dialog(_origin->longitude().value(), _origin->latitude().value(), dep, this);
	dialog.setTime(_origin->time().value());
	dialog.setWindowTitle("Modify origin");
	dialog.setSendButtonText("Apply");
	if ( dialog.exec() == QDialog::Accepted ) {
		OriginPtr tmpOrigin = Origin::Create();
		CreationInfo ci;
		ci.setAgencyID(SCApp->agencyID());
		ci.setAuthor(SCApp->author());
		ci.setCreationTime(Core::Time::GMT());
		//tmpOrigin->assign(_origin.get());
		tmpOrigin->setLatitude(dialog.latitude());
		tmpOrigin->setLongitude(dialog.longitude());
		tmpOrigin->setTime(Core::Time(dialog.getTime_t()));
		tmpOrigin->setDepth(RealQuantity(dialog.depth()));
		tmpOrigin->setEvaluationMode(EvaluationMode(MANUAL));
		tmpOrigin->setCreationInfo(ci);
		for ( size_t i = 0; i < _origin->arrivalCount(); ++i ) {
			ArrivalPtr ar = new Arrival(*_origin->arrival(i));
			tmpOrigin->add(ar.get());
		}
		setOrigin(tmpOrigin.get());
		updateLayoutFromState();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::diffStreamState(Seiscomp::DataModel::Origin* oldOrigin,
                                 Seiscomp::DataModel::Origin* newOrigin) {
	// Do nothing for now
	return;

	QSet<QString> oldPicks, newPicks;

	for ( size_t i = 0; i < oldOrigin->arrivalCount(); ++i )
		oldPicks.insert(oldOrigin->arrival(i)->pickID().c_str());

	for ( size_t i = 0; i < newOrigin->arrivalCount(); ++i )
		newPicks.insert(newOrigin->arrival(i)->pickID().c_str());

	oldPicks -= newPicks;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::addStations() {
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

		Stream *stream = NULL;

		stream = findConfiguredStream(s, _origin->time());

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

			double delta, az1, az2;
			if ( _origin )
				Geo::delazi(_origin->latitude(), _origin->longitude(),
				            stream->sensorLocation()->latitude(),
				            stream->sensorLocation()->longitude(), &delta, &az1, &az2);
			else
				delta = 0;

			RecordViewItem* item = addStream(stream->sensorLocation(), streamID,
			                                 delta, streamID.stationCode().c_str(),
			                                 false, true);
			if ( item ) {
				_stations.insert(code);
				item->setVisible(!_ui.actionShowUsedStations->isChecked());
				if ( _config.hideStationsWithoutData )
					item->forceInvisibilty(true);
			}
		}
	}

	fillRawPicks();

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
void PickerView::searchStation() {
	_searchStation->selectAll();
	_searchStation->setVisible(true);
	_searchLabel->setVisible(true);

	//_searchStation->grabKeyboard();
	_searchStation->setFocus();
	_recordView->setFocusProxy(_searchStation);

	_ui.actionCreatePick->setEnabled(false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::searchByText(const QString &text) {
	if ( text.isEmpty() ) return;

	QRegExp rx(text + "*");
	rx.setPatternSyntax(QRegExp::Wildcard);
	rx.setCaseSensitivity(Qt::CaseInsensitive);

	while ( true ) {
		int row = _recordView->findByText(0, rx, _lastFoundRow+1);
		if ( row != -1 ) {
			_lastFoundRow = row;

			if ( !_recordView->itemAt(row)->isVisible() ) continue;

			_recordView->setCurrentItem(_recordView->itemAt(row));
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

		break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::search(const QString &text) {
	_lastFoundRow = -1;

	searchByText(text);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::nextSearch() {
	searchByText(_searchStation->text());
	if ( _lastFoundRow == -1 )
		searchByText(_searchStation->text());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::abortSearchStation() {
	_recordView->setFocusProxy(NULL);
	_searchStation->releaseKeyboard();

	_searchStation->setVisible(false);
	_searchLabel->setVisible(false);

	_ui.actionCreatePick->setEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::emitPick(const Processing::Picker *,
                          const Processing::Picker::Result &res) {
	setCursorPos(res.time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::updateTheoreticalArrivals() {
	if ( _origin == NULL ) return;

	for ( int i = 0; i < _recordView->rowCount(); ++i ) {
		RecordViewItem* item = _recordView->itemAt(i);
		RecordWidget* widget = item->widget();

		for ( int m = 0; m < widget->markerCount(); ++m ) {
			PickerMarker* marker = (PickerMarker*)widget->marker(m);
			if ( !marker->pick() ) {
				delete marker;
				--m;
			}
		}
	}

	fillTheoreticalArrivals();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::createPick() {
	RecordViewItem *item = _recordView->currentItem();
	if ( item && !item->widget()->cursorText().isEmpty() ) {
		onSelectedTime(item->widget(), item->widget()->cursorPos());
		onSelectedTime(_currentRecord, _currentRecord->cursorPos());

		_recordView->selectNextRow();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setPick() {
	RecordViewItem *item = _recordView->currentItem();
	if ( item && !item->widget()->cursorText().isEmpty() ) {
		onSelectedTime(item->widget(), item->widget()->cursorPos());
		onSelectedTime(_currentRecord, _currentRecord->cursorPos());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::confirmPick() {
	RecordViewItem *item = _recordView->currentItem();
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

		if ( item ) {
			Core::Time t = item->widget()->cursorPos();
			if ( (t < item->widget()->leftTime()) ||
			     (t > item->widget()->rightTime()) ) {
				double tmin = _recordView->timeRangeMin();
				double tmax = _recordView->timeRangeMax();

				double pos = t - _recordView->alignment();

				double offset = pos - (tmin+tmax)/2;
				_recordView->move(offset);
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::resetPick() {
	RecordViewItem *item = _recordView->currentItem();
	if ( !item ) return;

	PickerMarker *m = static_cast<PickerMarker*>(item->widget()->marker(item->widget()->cursorText(), true));
	if ( !m ) return;

	if ( m->isMoveCopyEnabled() )
		m->reset();
	else
		delete m;

	item->widget()->update();
	_currentRecord->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::deletePick() {
	RecordViewItem *item = _recordView->currentItem();
	if ( item ) {
		if ( item->widget()->currentMarker() ) {
			PickerMarker *m = static_cast<PickerMarker*>(item->widget()->currentMarker());
			if ( m->isArrival() ) {
				if ( m->isEnabled() ) {
					RecordMarker *old = item->widget()->marker(m->text());
					if ( old ) old->setEnabled(true);
				}

				if ( m->isMovable() || !_loadedPicks )
					delete m;
				else
					m->setType(PickerMarker::Pick);

				item->widget()->update();
				_currentRecord->update();
			}
		}
		else if ( !item->widget()->cursorText().isEmpty() )
			resetPick();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::addFilter(const QString& name, const QString& filter) {
	if ( _comboFilter ) {
		if ( _comboFilter->findText(name) != -1 )
			return;

		_comboFilter->addItem(name, filter);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::activateFilter(int index) {
	if ( !_comboFilter ) return;

	if ( index < 0 ) {
		_comboFilter->setCurrentIndex(0);
		return;
	}

	++index;

	if ( _comboFilter->count() > index ) {
		_comboFilter->setCurrentIndex(index);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::specLogToggled(bool e) {
	static_cast<ZoomRecordWidget*>(_currentRecord)->setLogSpectrogram(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::specMinValue(double v) {
	static_cast<ZoomRecordWidget*>(_currentRecord)->setMinSpectrogramRange(v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::specMaxValue(double v) {
	static_cast<ZoomRecordWidget*>(_currentRecord)->setMaxSpectrogramRange(v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::specTimeWindow(double tw) {
	static_cast<ZoomRecordWidget*>(_currentRecord)->setSpectrogramTimeWindow(tw);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::limitFilterToZoomTrace(bool e) {
	changeFilter(_comboFilter->currentIndex(), true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showTheoreticalArrivals(bool v) {
	for ( int i = 0; i < _currentRecord->markerCount(); ++i ) {
		PickerMarker* m = static_cast<PickerMarker*>(_currentRecord->marker(i));
		if ( m->type() == PickerMarker::Theoretical )
			m->setVisible(v);
	}

	// Since all markers are just proxies of the real traces we need
	// to update the zoom trace explicitly.
	_currentRecord->update();

	for ( int i = 0; i < _recordView->rowCount(); ++i ) {
		RecordWidget *w = _recordView->itemAt(i)->widget();

		for ( int i = 0; i < w->markerCount(); ++i ) {
			PickerMarker* m = static_cast<PickerMarker*>(w->marker(i));
			if ( m->type() == PickerMarker::Theoretical )
				m->setVisible(v);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showUnassociatedPicks(bool v) {
	if ( v && !_loadedPicks ) {
		loadPicks();
		fillRawPicks();
	}

	for ( int i = 0; i < _currentRecord->markerCount(); ++i ) {
		PickerMarker* m = static_cast<PickerMarker*>(_currentRecord->marker(i));
		if ( m->type() == PickerMarker::Pick )
			m->setVisible(v);
	}

	// Since all markers are just proxies of the real traces we need
	// to update the zoom trace explicitly.
	_currentRecord->update();

	for ( int i = 0; i < _recordView->rowCount(); ++i ) {
		RecordWidget *w = _recordView->itemAt(i)->widget();

		for ( int i = 0; i < w->markerCount(); ++i ) {
			PickerMarker* m = static_cast<PickerMarker*>(w->marker(i));
			if ( m->type() == PickerMarker::Pick )
				m->setVisible(v);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::showSpectrogram(bool v) {
	static_cast<ZoomRecordWidget*>(_currentRecord)->setShowSpectrogram(v);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::changeFilter(int index) {
	changeFilter(index, false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::changeRotation(int index) {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	_currentRotationMode = index;

	for ( int i = 0; i < _recordView->rowCount(); ++i ) {
		RecordViewItem* rvi = _recordView->itemAt(i);
		applyRotation(rvi, index);
		updateTraceInfo(rvi, NULL);
	}

	// Change icons depending on the current rotation mode
	if ( index == RT_ZRT ) {
		_ui.actionShowNComponent->setIcon(QIcon(QString::fromUtf8(":/icons/icons/channelR.png")));
		_ui.actionShowNComponent->setText(QString::fromUtf8("Radial"));
		_ui.actionShowNComponent->setToolTip(QString::fromUtf8("Show Radial Component (N)"));
		_ui.actionShowEComponent->setIcon(QIcon(QString::fromUtf8(":/icons/icons/channelT.png")));
		_ui.actionShowEComponent->setText(QString::fromUtf8("Transversal"));
		_ui.actionShowEComponent->setToolTip(QString::fromUtf8("Show Transversal Component (E)"));
	}
	else {
		_ui.actionShowNComponent->setIcon(QIcon(QString::fromUtf8(":/icons/icons/channelN.png")));
		_ui.actionShowNComponent->setText(QString::fromUtf8("North"));
		_ui.actionShowNComponent->setToolTip(QString::fromUtf8("Show North Component (N)"));
		_ui.actionShowEComponent->setIcon(QIcon(QString::fromUtf8(":/icons/icons/channelE.png")));
		_ui.actionShowEComponent->setText(QString::fromUtf8("East"));
		_ui.actionShowEComponent->setToolTip(QString::fromUtf8("Show East Component (E)"));
	}


	if ( index == RT_ZNE || index == RT_ZRT || index == RT_ZH ) {
		bool tmp = _config.loadAllComponents;
		_config.loadAllComponents = true;

		// Fetch all components if not done already
		fetchComponent('?');

		_config.loadAllComponents = tmp;
	}

	if ( _recordView->currentItem() ) {
		updateItemLabel(_recordView->currentItem(), _recordView->currentItem()->currentComponent());

		for ( int i = 0; i < _currentRecord->slotCount(); ++i ) {
			char code = _recordView->currentItem()->mapSlotToComponent(i);
			if ( code == '?' ) continue;

			switch ( index ) {
				case RT_Z12:
					_currentRecord->setRecordID(i, QString("%1").arg(code));
					break;
				case RT_ZNE:
					_currentRecord->setRecordID(i, QString("%1").arg(ZNE_COMPS[i]));
					break;
				case RT_ZRT:
					_currentRecord->setRecordID(i, QString("%1").arg(ZRT_COMPS[i]));
					break;
				case RT_ZH:
					_currentRecord->setRecordID(i, QString("%1").arg(ZH_COMPS[i]));
					break;
			}
		}

		_currentRecord->update();
	}

	QApplication::restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::applyFilter(RecordViewItem *item) {
	if ( item == NULL ) {
		for ( int i = 0; i < _recordView->rowCount(); ++i ) {
			RecordViewItem* rvi = _recordView->itemAt(i);
			PickerRecordLabel *label = static_cast<PickerRecordLabel*>(rvi->label());

			label->data.setFilter(_currentFilter);
		}
	}
	else {
		PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
		label->data.setFilter(_currentFilter);
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool PickerView::applyRotation(RecordViewItem *item, int type) {
	switch ( type ) {
		case RT_Z12:
		{
			PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
			label->data.transformation.identity();
			label->data.setL2Horizontals(false);
			label->data.setTransformationEnabled(false);
			break;
		}
		case RT_ZNE:
		{
			PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
			label->data.transformation = label->orientationZNE;
			label->data.setL2Horizontals(false);
			label->data.setTransformationEnabled(true);
			break;
		}
		case RT_ZRT:
		{
			PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
			label->data.transformation.mult(label->orientationZRT, label->orientationZNE);
			label->data.setL2Horizontals(false);
			label->data.setTransformationEnabled(true);
			break;
		}
		case RT_ZH:
		{
			PickerRecordLabel *label = static_cast<PickerRecordLabel*>(item->label());
			label->data.transformation.identity();
			label->data.setL2Horizontals(true);
			label->data.setTransformationEnabled(true);
			break;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::changeFilter(int index, bool force) {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QString name = _comboFilter->itemText(index);
	QString filter = _comboFilter->itemData(index).toString();

	if ( name == NO_FILTER_STRING ) {
		if ( _currentFilter ) delete _currentFilter;
		_currentFilter = NULL;
		_currentFilterID = QString();

		if ( !_ui.actionLimitFilterToZoomTrace->isChecked() )
			applyFilter();
		else
			applyFilter(_recordView->currentItem());

		QApplication::restoreOverrideCursor();
		return;
	}

	RecordWidget::Filter *newFilter = RecordWidget::Filter::Create(filter.toStdString());

	if ( newFilter == NULL ) {
		QMessageBox::critical(this, "Invalid filter",
		                      QString("Unable to create filter: %1\nFilter: %2").arg(name).arg(filter));

		_comboFilter->blockSignals(true);
		_comboFilter->setCurrentIndex(_lastFilterIndex);
		_comboFilter->blockSignals(false);
	}
	else
		_currentFilterID = filter;

	if ( _currentFilter ) delete _currentFilter;
	_currentFilter = newFilter;
	if ( !_ui.actionLimitFilterToZoomTrace->isChecked() )
		applyFilter();
	else
		applyFilter(_recordView->currentItem());

	_lastFilterIndex = index;
	QApplication::restoreOverrideCursor();

	/*
	if ( index == _lastFilterIndex && !force ) {
		if ( !_ui.actionLimitFilterToZoomTrace->isChecked() )
			_recordView->enableFilter(_lastFilterIndex > 0);
		else
			_currentRecord->enableFiltering(_lastFilterIndex > 0);
		return;
	}

	QString name = _comboFilter->itemText(index);
	QString filter = _comboFilter->itemData(index).toString();

	if ( name == NO_FILTER_STRING ) {
		if ( !_ui.actionLimitFilterToZoomTrace->isChecked() )
			_recordView->enableFilter(false);
		else
			_currentRecord->enableFiltering(false);
		return;
	}


	bool filterApplied = false;
	// Here one should
	if ( !_ui.actionLimitFilterToZoomTrace->isChecked() ) {
		if ( _recordView->setFilterByName(filter) ) {
			_currentRecord->setFilter(_recordView->filter());
			_lastFilterIndex = index;
			filterApplied = true;
		}
	}
	else {
		RecordWidget::Filter *f =
			Util::createFilterByName<float>(filter.toStdString());

		if ( f != NULL ) {
			_currentRecord->setFilter(f);
			_lastFilterIndex = index;
			filterApplied = true;
			delete f;
		}
	}


	if ( !filterApplied ) {
		QMessageBox::critical(this, "Invalid filter",
		                      QString("Unable to create filter: %1\nFilter: %2").arg(name).arg(filter));

		_comboFilter->blockSignals(true);
		_comboFilter->setCurrentIndex(_lastFilterIndex);
		_comboFilter->blockSignals(false);
	}

	//std::cout << "Current filter index: " << _lastFilterIndex << std::endl;
	if ( !_ui.actionLimitFilterToZoomTrace->isChecked() )
		_recordView->enableFilter(_lastFilterIndex > 0);
	else
		_currentRecord->enableFiltering(_lastFilterIndex > 0);
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void PickerView::setArrivalState(int arrivalId, bool state) {
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
bool PickerView::setArrivalState(RecordWidget* w, int arrivalId, bool state) {
	if ( !w->isEnabled() ) return false;

	// Find arrival and update state
	for ( int m = 0; m < w->markerCount(); ++m ) {
		PickerMarker *marker = (PickerMarker*)w->marker(m);
		if ( marker->id() == arrivalId && marker->isArrival() ) {
			marker->setEnabled(state);
			w->update();
			return true;
		}
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
