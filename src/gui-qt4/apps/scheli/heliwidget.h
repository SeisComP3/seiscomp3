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

#ifndef __HELIWIDGET_H__
#define __HELIWIDGET_H__

#include <QtGui>

#ifndef Q_MOC_RUN
#include <boost/shared_ptr.hpp>

#include <seiscomp3/core/recordsequence.h>
#include <seiscomp3/math/filter.h>
#endif
#include <seiscomp3/gui/core/recordpolyline.h>


class HeliCanvas {
	public:
		HeliCanvas(bool saveUnfiltered = false);
		~HeliCanvas();

	public:
		bool feed(Seiscomp::Record *rec);
		bool setCurrentTime(const Seiscomp::Core::Time &time);

		void setLayout(int rows, int secondsPerRow);
		void setLabelMargin(int margin) { _labelMargin = margin; }
		void setAntialiasingEnabled(bool);

		int draw(QPainter &painter, const QSize &size);
		void save(QString streamID, QString headline, QString date,
		          QString filename, int xres, int yres, int dpi);

		void resize(const QFontMetrics &fm, const QSize &size);
		void render(QPainter &painter);

		int labelWidth() const { return _labelMargin; }
		Seiscomp::Core::TimeSpan recordsTimeSpan() const;

		// Sets the scale, usually 1/gain
		void setScale(float scale);
		bool setFilter(const std::string &filter);
		void setAmplitudeRange(double min, double max);

		void setRowColors(const QVector<QColor> &);
		void setLineWidth(int lw);


	private:
		void setRecordsTimeSpan(int seconds);
		void setRowTimeSpan(int seconds);
		QString formatAnnotation(const double pos);

		void rebuildView();
		void applyFilter();


	private:
		typedef boost::shared_ptr<Seiscomp::Gui::RecordPolyline> RecordPolylinePtr;
		typedef Seiscomp::Math::Filtering::InPlaceFilter<float> Filter;

		struct Row {
			Row() {}

			Seiscomp::Core::Time time;
			RecordPolylinePtr    polyline;
			bool                 dirty;

			void update();
		};

		QSize                     _size;
		QPalette                  _palette;
		Filter                   *_filter;
		Seiscomp::RecordSequence *_records;
		Seiscomp::RecordSequence *_filteredRecords;
		int                       _recordsTimeSpan;
		int                       _rowTimeSpan;
		QVector<Row>              _rows;

		bool                      _saveUnfiltered;
		Seiscomp::Core::Time      _currentTime;
		int                       _numberOfRows;
		float                     _scale;

		QColor                    _gaps[2];
		double                    _amplitudeRange[2];
		int                       _labelMargin;
		bool                      _antialiasing;

		QVector<QColor>           _rowColors;
		int                       _lineWidth;

		double                    _drx[2];
};



class HeliWidget : public QWidget {
	Q_OBJECT

	public:
		HeliWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
		~HeliWidget();


	public slots:
		void feed(Seiscomp::Record *rec);
		void setCurrentTime(const Seiscomp::Core::Time &time);


	public:
		HeliCanvas &canvas() { return _canvas; }


	protected:
		void paintEvent(QPaintEvent *);
		void resizeEvent(QResizeEvent *);


	private:
		HeliCanvas _canvas;
};


#endif
