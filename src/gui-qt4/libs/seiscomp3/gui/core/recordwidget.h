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



#ifndef _SEISMOGRAMPLOT_H_
#define _SEISMOGRAMPLOT_H_

#include <QWidget>
#include <QFrame>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QHBoxLayout>

#include <seiscomp3/core/record.h>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/math/filter.h>

#include "recordpolyline.h"


namespace Seiscomp {
namespace Gui {


class RecordWidget;


//! \brief RecordMarker - marks a certain timestamp in the record
//! \brief sequence with a color and a type.
//!
//! Furthermore a marker can be moved on the stream. This class
//! is useful to correct picks visually.
class SC_GUI_API RecordMarker : public QObject {

	Q_OBJECT

	public:
		RecordMarker(RecordWidget *parent,
		             const Seiscomp::Core::Time& pos,
		             Qt::Alignment = Qt::AlignVCenter);

		RecordMarker(RecordWidget *parent,
		             const Seiscomp::Core::Time& pos,
		             const QString& text,
		             Qt::Alignment = Qt::AlignVCenter);

		RecordMarker(RecordWidget *parent,
		             const RecordMarker&);

		virtual ~RecordMarker();


	protected:
		void setParent(RecordWidget*);


	public:
		RecordWidget* parent() const;

		void setColor(QColor c);
		QColor color() const;

		void setAlignment(Qt::Alignment);
		Qt::Alignment alignment() const;

		void setModifiedColor(QColor c);
		QColor modifiedColor() const;

		const Seiscomp::Core::Time& time() const;

		void setCorrectedTime(const Seiscomp::Core::Time&);
		const Seiscomp::Core::Time& correctedTime() const;

		void setText(const QString&);
		const QString& text() const;

		void addAlias(const QString&);

		void setDescription(const QString&);
		const QString& description() const;

		bool matches(const QString &text) const;

		//! Returns the text to render
		//! When description is empty then text is used else
		//! the description itself is going to be rendered
		const QString& renderText() const;

		void setVisible(bool visible);
		bool isVisible() const;
		bool isHidden() const;

		void setMovable(bool enable);
		bool isMovable() const;

		void setMoveCopy(bool enable);
		bool isMoveCopyEnabled() const;

		bool isEnabled() const;

		//! Sets internal data to data. This can be used for
		//! user data to store anything that a QVariant can store.
		void setData(const QVariant& data);

		//! Returns the user data set in setData
		QVariant data() const;

		//! Sets an id for the marker. This id is application depended
		//! and can be chosen freely
		void setId(int id);

		//! Returns the id for the marker
		int id() const;

		bool isModified() const;

		//! Sets the corrected time to the start time
		void reset();

		//! Sets the start time to the corrected time
		void apply();

		void update();

		virtual RecordMarker *copy();

		virtual void draw(QPainter &painter, RecordWidget *context,
		                  int x, int y1, int y2,
		                  QColor color, qreal lineWidth);

		virtual QString toolTip() const;


	public slots:
		virtual void setEnabled(bool enable);


	private:
		RecordWidget *_parent;
		QColor _color;
		QColor _modifierColor;
		Seiscomp::Core::Time _time;
		Seiscomp::Core::Time _correctedTime;
		QString _text;
		QString _description;
		QVector<QString> _aliases;
		bool _visible;
		bool _moveable;
		bool _moveCopy;
		bool _enabled;
		int _id;
		Qt::Alignment _alignment;
		QVariant _data;

	friend class RecordWidget;
};


//!\brief RecordWidgetDecorator - decorates a RecordWidget
//!
//! This class can be used to draw custom decorations before
//! and after the RecordWidget paints its data.
class RecordWidgetDecorator : public QObject {
	Q_OBJECT

	public:
		RecordWidgetDecorator(QObject *parent = 0);


	public:
		virtual void drawDecoration(QPainter *painter, RecordWidget *widget) = 0;
};


//!\brief RecordWidget - draws seismograms in a rectangular widget
//!
//! This class is only responsible for drawing, it gets records assigned as
//! pointers and is (optional) responsible for their (de)allocation. Only filtered
//! recordstreams are owned by the RecordWidget.
class SC_GUI_API RecordWidget : public QWidget
{
	Q_OBJECT

	public:
		enum DrawMode {
			Single,
			InRows,
			Stacked,
			SameOffset
		};

		enum ShadowWidgetFlags {
			Raw = 0x01,
			Filtered = 0x02
		};

		typedef Math::Filtering::InPlaceFilter<float> Filter;

		struct Trace {
			Trace() : amplMin(0), amplMax(0), offset(0),
			          absMax(0), yMin(0), yMax(0), visible(false) {}
			float          amplMin;
			float          amplMax;
			float          offset;
			float          absMax;
			int            yMin;
			int            yMax;
			float          fyMin;
			float          fyMax;
			float          timingQuality;
			int            timingQualityCount;
			bool           dirty;
			bool           visible;
			RecordPolyline poly;
		};

	public:
		RecordWidget(QWidget *parent=0);
		RecordWidget(const DataModel::WaveformStreamID& streamID, QWidget *parent=0);

		~RecordWidget();

		void enableRecordFiltering(int slot, bool enable);
		bool isRecordFilteringEnabled(int slot);

		bool isRecordVisible(int slot);
		bool setRecordVisible(int slot, bool visible);

		void setCustomBackgroundColor(QColor c);
		void removeCustomBackgroundColor();

		bool setRecords(int slot, RecordSequence*, bool owner = true);
		bool setRecordID(int slot, const QString &id);
		bool setRecordColor(int slot, QColor c);
		bool setRecordPen(int slot, const QPen &pen);
		bool setRecordAntialiasing(int slot, bool antialiasing);
		bool setRecordOptimization(int slot, bool enable);
		bool setRecordBackgroundColor(int slot, QColor c);
		bool removeRecordBackgroundColor(int slot);
		bool setRecordFilter(int slot, const Filter *f);
		bool setRecordScale(int slot, double scale);
		bool setRecordUserData(int slot, QVariant data);

		RecordSequence *records() const;
		RecordSequence *records(int slot) const;
		RecordSequence *filteredRecords(int slot) const;
		RecordSequence *takeRecords(int slot);
		RecordSequence *createRecords(int slot, bool owner = true);
		const Filter *recordFilter(int slot) const;

		QString recordID(int slot) const;
		const double *recordScale(int slot) const;
		const Trace *traceInfo(int slot, bool filtered = false) const;

		QVariant recordUserData(int slot);

		int setCurrentRecords(int slot);
		int currentRecords() const;

		void setDrawMode(DrawMode mode);
		DrawMode drawMode() const;

		void setDrawOffset(bool f);
		bool drawOffset() const { return _drawOffset; }

		void setDrawRecordID(bool f);
		bool drawRecordID() const { return _drawRecordID; }

		void setValuePrecision(int p);
		int valuePrecision() const { return _valuePrecision; }

		//! Sets a shadow widget that gets informed about record changes.
		//! Available record slots are copied by reference
		//! in that way that the listener is not the owner of the
		//! data. Available marker are copied by value.
		void setShadowWidget(RecordWidget *shadow,  bool copyMarker,
		                     int flags = Raw);

		//! Returns the current shadow widget
		RecordWidget *shadowWidget() const { return _shadowWidget; }

		//! Sets the source widget for all markers.
		//! This instance does not hold any marker but displays
		//! the source widgets markers. All marker query function
		//! are forwarded to source.
		void setMarkerSourceWidget(RecordWidget *source);

		//! Returns the current marker source widget
		RecordWidget *markerSourceWidget() const { return _markerSourceWidget; }

		void setDecorator(RecordWidgetDecorator *decorator);
		RecordWidgetDecorator *decorator() const { return _decorator; }

		//! Returns the fetched timing quality average
		float timingQuality(int slot) const;

		double timeScale() const { return _pixelPerSecond; }
		float amplScale() const { return _amplScale; }
		double tmin() const { return _tmin; }
		double tmax() const { return _tmax; }
		double amplMin() const { return _amplitudeRange[0]; }
		double amplMax() const { return _amplitudeRange[1]; }

		double smin() const { return _smin; }
		double smax() const { return _smax; }
	
		Seiscomp::Core::Time alignment() { return _alignment; }

		Seiscomp::Core::Time centerTime();

		//! Returns the trace plot position of the given slot
		int streamYPos(int slot) const;
		//! Returns the trace height of the given slot
		int streamHeight(int slot) const;

		//! Returns the amplitude range of the available data in the normalization
		//! window
		QPair<float,float> amplitudeDataRange(int slot) const;

		//! Returns the amplitude range of the trace slot where first corresponds
		//! to the lower pixel row and second to the upper pixel row
		QPair<float,float> amplitudeRange(int slot) const;

		void ensureVisibility(const Seiscomp::Core::Time &time, int pixelMargin);
	
		//! Method to inform the widget about a newly inserted
		//! record.
		virtual void fed(int slot, const Seiscomp::Record *rec);
	
		//! Causes the widget to rebuild its internal data
		//! according its size and parameters
		void setDirty();

		//! Whether to show the current selected recordstream or
		//! both recordstreams
		void showAllRecords(bool enable);

		//! Whether to show scaled or raw values. The default is false.
		void showScaledValues(bool enable);
		bool areScaledValuesShown() const { return _showScaledValues; }

		//! Adds a marker to the widget. The ownership takes
		//! the widget.
		bool addMarker(RecordMarker*);

		//! Inserts a marker at pos in the marker list
		bool insertMarker(int pos, RecordMarker* marker);

		int indexOfMarker(RecordMarker* marker) const;

		//! Removes a record marker and returns the instance
		RecordMarker *takeMarker(int pos);
		RecordMarker *takeMarker(RecordMarker*);

		//! Removes and deletes a record marker.
		bool removeMarker(int pos);
		bool removeMarker(RecordMarker*);

		//! Returns the number of markers added to the widget.
		int markerCount() const;

		//! Returns the i-th marker.
		RecordMarker* marker(int i) const;

		//! Returns the marker with text set to txt
		RecordMarker* marker(const QString& txt, bool movableOnly = false) const;

		//! Returns the marker with text set to txt and which is enabled
		RecordMarker* enabledMarker(const QString& txt) const;

		//! Removes and deletes all marker inserted previously.
		void clearMarker();

		//! Sets the current marker
		void setCurrentMarker(RecordMarker *);

		//! Returns the currently focused marker
		RecordMarker *currentMarker() const;

		//! Returns the current marker under the mouse
		RecordMarker *hoveredMarker() const;

		//! Returns the marker under position p. If moveableOnly is set
		//! to true, only movable markers are checked.
		RecordMarker* markerAt(const QPoint& p, bool movableOnly = true, int maxDist = 5) const;
		RecordMarker* markerAt(int x, int y, bool movableOnly = true, int maxDist = 5) const;

		//! Returns the nearest marker left from time t
		RecordMarker* lastMarker(const Seiscomp::Core::Time& t);

		//! Returns the nearest marker right from time t
		RecordMarker* nextMarker(const Seiscomp::Core::Time& t);

		//! Returns the nearest marker at time t
		RecordMarker* nearestMarker(const Seiscomp::Core::Time& t, int maxDist = -1);

		//! Returns whether the widget has markers that are movable or not
		bool hasMovableMarkers() const;

		//! Sets the text to be displayed on the cursor
		void setCursorText(const QString& text);

		//! Returns the current cursor text
		QString cursorText() const;

		void setCursorPos(const QPoint&);
		void setCursorPos(const Seiscomp::Core::Time&);
		const Seiscomp::Core::Time& cursorPos() const;

		const double *value(const Seiscomp::Core::Time&) const;

		//! Returns the time on the left visible side of the widget
		Seiscomp::Core::Time leftTime() const;
		//! Returns the time on the right visible side of the widget
		Seiscomp::Core::Time rightTime() const;

		Seiscomp::Core::TimeWindow visibleTimeWindow() const;
		Seiscomp::Core::TimeWindow selectedTimeWindow() const;
		const Seiscomp::Core::TimeWindow & normalizationWindow() const;

		bool isActive() const;
		bool isFilteringEnabled() const;
		bool isGlobalOffsetEnabled() const;

		//! Maps a time to a position relative to the widget
		int mapTime(const Core::Time&) const;

		//! Maps a widget position to a time
		Core::Time unmapTime(int x) const;

		//bool setGain(float);
		//float gain() const;

		void setTracePaintOffset(int offset);
		int tracePaintOffset() const;

		const DataModel::WaveformStreamID& streamID() const;

		void setSlotCount(int);
		int slotCount() const;

		//! Sets internal data to data. This can be used for
		//! user data to store anything that a QVariant can store.
		void setData(const QVariant& data);

		//! Returns the user data set in setData
		QVariant data() const;


	public slots:
		void setAmplAutoScaleEnabled(bool enable);

		//! Sets clipping for drawmode InRows.
		void setClippingEnabled(bool);
		bool isClippingEnabled() const;

		void enableGlobalOffset(bool enable);

		void setSelected(double t1, double t2);
		void setSelected(Seiscomp::Core::Time t1, Seiscomp::Core::Time t2);

		void setScale(double, float=0);
		void setTimeScale(double);

		void setTimeRange(double, double);
		void setAmplRange(double, double);
		void setMinimumAmplRange(double, double);

		void showTimeRange(double, double);

		void translate(double);

		void setEnabled(bool enabled);
		void setAlignment(Seiscomp::Core::Time t);
		void alignOnMarker(const QString& text);

		void setAmplScale(float);
	
		void enableFiltering(bool enable);
		void setGridSpacing(double, double, double);

		void setActive(bool);

		void setAutoMaxScale(bool);

		void setNormalizationWindow(const Seiscomp::Core::TimeWindow&);
		void setOffsetWindow(const Seiscomp::Core::TimeWindow&);

		//! Sets the maximum slot index for which setFilter(filter) is
		//! applied. The semantics of 'any' is bound to value -1.
		void setFilterSlotMax(int max);

		//! Sets the record filter. The filter will be cloned and will not
		//! be touched by the widget. The caller is responsible to destroy
		//! the filter used as parameter.
		void setFilter(Filter* filter);

		void updateRecords();
		void clearRecords();


	private slots:
		void scroll(int);


	signals:
		//! This signal is emitted when a time (absolut time)
		//! has been selected inside the widget. It does not
		//! happen when a marker has been hit.
		void selectedTime(Seiscomp::Core::Time);

		//! This signal is emitted when a time range (absolut time)
		//! has been selected inside the widget by holding
		//! the left mouse button.
		void selectedTimeRange(Seiscomp::Core::Time, Seiscomp::Core::Time);

		//! This signal is emitted when a time range (absolut time)
		//! is selected while still holding the left mouse button.
		void selectedTimeRangeChanged(Seiscomp::Core::Time, Seiscomp::Core::Time);

		//! Whenever a current (active) marker has changed
		//! this signal will be emittet.
		void currentMarkerChanged(Seiscomp::Gui::RecordMarker*);

		void cursorMoved(QPoint p);

		//! Whenever the cursor changes this signal is emitted.
		//! The first parameter is the owner of the cursor and
		//! the second parameter is the slot where the cursor has
		//! been moved. This usually equals the current slot but
		//! when drawing all slots in rows then it might differ.
		void cursorUpdated(RecordWidget*, int);
		void mouseOver(bool);

		//! This signal is emitted when the user clicks with the
		//! middle mousebutton into the trace
		void clickedOnTime(Seiscomp::Core::Time);

		void traceUpdated(Seiscomp::Gui::RecordWidget*);

		//! This signal is emitted when the layout of the widget
		//! changes. Whenever a new slot has been added or removed
		//! and the drawingMode is InRows then a layoutRequest is made.
		void layoutRequest();


	protected:
		void init();

		bool event(QEvent *);

		void paintEvent(QPaintEvent*);

		void mousePressEvent(QMouseEvent*);
		void mouseReleaseEvent(QMouseEvent*);
		void mouseDoubleClickEvent(QMouseEvent*);
		void mouseMoveEvent(QMouseEvent*);

		void resizeEvent(QResizeEvent*);

		void enterEvent(QEvent*);
		void leaveEvent(QEvent*);

		void enabledChange(bool) { update(); }

		virtual void changedRecords(int slot, RecordSequence*);

		virtual void drawActiveCursor(QPainter &painter, int x, int y);

		virtual void drawCustomBackground(QPainter &painter);
		virtual void customPaintTracesBegin(QPainter &painter);
		virtual void customPaintTracesEnd(QPainter &painter);

		virtual void createPolyline(int slot, RecordPolyline &polyline,
		                            RecordSequence const *, double pixelPerSecond,
		                            float amplMin, float amplMax, float amplOffset,
		                            int height, bool optimization);
		virtual const double *value(int slot, const Seiscomp::Core::Time&) const;


	private:
		struct Stream {
			enum Index {
				Raw = 0,
				Filtered = 1
			};

			Stream(bool owner);
			~Stream();

			void setDirty();
			void free();

			RecordSequence *records[2];
			Trace           traces[2];
			bool            ownRawRecords;
			bool            ownFilteredRecords;
			bool            visible;
			bool            filtering;

			QString         id;
			QPen            pen;
			bool            antialiasing;
			QColor          customBackgroundColor;
			bool            hasCustomBackgroundColor;
			bool            optimize;
			double          scale;

			// Internal variables to track the current trace position
			// in widget coordinated
			int             posY;
			int             height;

			QVariant        userData;

			Filter         *filter;
		};


	private:
		Stream *getStream(int);
		const Stream *getStream(int) const;

		bool createFilter();
		bool createFilter(int slot);
		void filterRecords(Stream *s);
		bool setFilteredRecords(int slot, RecordSequence* seq, bool owner);
		Record* filteredRecord(Filter *filter,
		                       const Record*, const Record* = NULL) const;

		void prepareRecords(Stream *s);
		void drawRecords(Stream *s, int slot, int h);
		void centerLine(int l, int h);


	private:
		typedef QVector<Stream*> StreamMap;

		QVariant             _data;
		DrawMode             _drawMode;
		Seiscomp::Core::Time _alignment;
		bool                 _clipRows;
	
		double  _tmin;            // time range min
		double  _tmax;            // time range max
		double  _smin, _smax;     // selection
		double  _pixelPerSecond;
		float   _amplScale;       // pixel per amplitude unit (0=normalize)
		double  _gridSpacing[2];
		double  _gridOffset;

		float   _amplitudeRange[2];
		bool    _useFixedAmplitudeRange;
		bool    _useMinAmplitudeRange;

		bool    _active;
		bool    _filtering;
		bool    _showScaledValues;
	
		bool    _drawRecords;
		bool    _drawRecordID;
		bool    _drawOffset;
		bool    _showAllRecords;
		bool    _autoMaxScale;
		bool    _enabled;
		bool    _useGlobalOffset;

		int     _tracePaintOffset;

		StreamMap _streams;
		int       _currentSlot;
		int       _requestedSlot;
		int       _maxFilterSlot;
		int       _currentCursorYPos;
		int       _valuePrecision;

		QColor    _customBackgroundColor;
		bool      _hasCustomBackground;

		Seiscomp::DataModel::WaveformStreamID _streamID;

		QVector<RecordMarker*> _marker;
		RecordMarker* _activeMarker;
		RecordMarker* _hoveredMarker;

		QScrollBar *_scrollBar;

		QString _cursorText;
		Seiscomp::Core::Time _cursorPos;
		Seiscomp::Core::Time _startDragPos;

		Seiscomp::Core::TimeWindow _normalizationWindow;
		Seiscomp::Core::TimeWindow _offsetWindow;

		RecordWidget *_shadowWidget;
		RecordWidget *_markerSourceWidget;

		RecordWidgetDecorator *_decorator;

		int                    _shadowWidgetFlags;
};


}
}


# endif
