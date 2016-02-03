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

#ifndef __MAINWINDOW_H__
#define __MAINWINDOW_H__

#include <QtGui>
#ifndef Q_MOC_RUN
#include <seiscomp3/datamodel/waveformstreamid.h>
#endif
#include <seiscomp3/gui/core/mainwindow.h>


#include "ui_mainwindow.h"


class HeliWidget;

namespace Seiscomp {

class Record;

namespace Gui {

class RecordStreamThread;

}
}


class MainWindow : public Seiscomp::Gui::MainWindow {
	Q_OBJECT

	public:
		MainWindow();
		~MainWindow();


	public:
		void setGain(const float gain);
		void setHeadline(const QString headline);

		void setReferenceTime(const Seiscomp::Core::Time &time);
		void setStream(const std::string &networkCode,
		const std::string &stationCode,
		const std::string &locationCode,
		const std::string &channelCode);

		void setStream(Seiscomp::DataModel::WaveformStreamID streamID);

		void setFilter(const std::string &filter);
		void setAmplitudeRange(double min, double max);
		void setRowColors(const QVector<QColor> &);

		void setStationDescriptionEnabled(bool);
		void setAntialiasingEnabled(bool);
		void setLineWidth(int);
		void setLayout(int rows, int secondsPerRow);
		void setOutputResolution(int xres, int yres, int dpi);
		void setSnapshotTimeout(int timeout);

		void setTimeFormat(const std::string &timeFormat);

		void fixCurrentTimeToLastRecord(bool);

		void start(QString dumpFilename);
		void updateTimeLabel(const Seiscomp::Core::Time &time);

		void timerEvent(QTimerEvent *event);


	public slots:
		void print(QString filename);


	protected:
		void toggledFullScreen(bool val);


	private:
		void startAcquisition();


	private slots:
		void acquisitionFinished();
		void receivedRecord(Seiscomp::Record*);
		void advanceTime();


	private:
		Ui::MainWindow                        _ui;
		HeliWidget                           *_heliWidget;
		Seiscomp::Gui::RecordStreamThread    *_streamThread;
		Seiscomp::DataModel::WaveformStreamID _streamID;
		Seiscomp::Core::Time                  _realTimeStart;
		Seiscomp::Core::Time                  _globalEndTime;
		Seiscomp::Core::Time                  _referenceTime;
		Seiscomp::Core::TimeSpan              _fullTimeSpan;
		std::string                           _timeFormat;
		QTimer                                _timer;
		QString                               _dumpFilename;
		bool                                  _fixCurrentTimeToLastRecord;
		bool                                  _showStationDescription;
		int                                   _rowTimeSpan;
		int                                   _xRes;
		int                                   _yRes;
		int                                   _dpi;
		int                                   _snapshotTimer;
};


#endif
