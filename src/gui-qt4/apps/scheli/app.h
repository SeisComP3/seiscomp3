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

#include <seiscomp3/gui/core/application.h>
#ifndef Q_MOC_RUN
#include <seiscomp3/core/strings.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/io/recordstream.h>
#include <seiscomp3/io/recordinput.h>
#endif
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/utils.h>
#include <seiscomp3/gui/core/recordstreamthread.h>


#include <QObject>

#include "mainwindow.h"
#include "heliwidget.h"


using namespace Seiscomp;


class HCApp : public Gui::Application {
	Q_OBJECT

	public:
		HCApp(int& argc, char** argv, int flags = DEFAULT, Type type = QApplication::GuiClient);
		~HCApp();

	protected:
		QString findHeadline(const DataModel::WaveformStreamID &streamID,
		                     const Core::Time &refTime);
		float findGain(const DataModel::WaveformStreamID &streamID,
		               const Core::Time &refTime);

		bool initConfiguration();
		void createCommandLineDescription();
		bool validateParameters();
		bool handleInitializationError(Stage stage);
		bool init();
		bool run();

		void setupUi(MainWindow *w);

	protected:
		void timerEvent(QTimerEvent *);
		void saveSnapshots();

	private slots:
		void receivedRecord(Seiscomp::Record*);
		void acquisitionFinished();


	private:
		struct HeliStream {
			HeliStream() {}
			HeliStream(HeliCanvas *hc) : canvas(hc) {}
			HeliCanvas *canvas;
			QString     headline;
			Core::Time  lastSample;
		};

		typedef QMap<std::string, HeliStream> HeliStreamMap;

		std::vector<std::string> _streamCodes;
		std::string        _streamID;
		HeliStreamMap      _helis;
		Core::Time         _endTime;
		std::string        _filterString;
		float              _gain;
		float              _amplitudesRange;
		float              _amplitudesMin;
		float              _amplitudesMax;
		QVector<QColor>    _rowColors;
		bool               _fixCurrentTimeToLastRecord;
		int                _numberOfRows;
		int                _timeSpanPerRow;
		int                _xRes;
		int                _yRes;
		int                _dpi;
		int                _snapshotTimeout;
		int                _snapshotTimer;
		bool               _antialiasing;
		int                _lineWidth;
		bool               _stationDescription;
		std::string        _timeFormat;
		std::string        _outputFilename;
		Gui::RecordStreamThread *_streamThread;
};
