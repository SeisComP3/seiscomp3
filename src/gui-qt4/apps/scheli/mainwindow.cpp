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


#define SEISCOMP_COMPONENT Helicorder
#include <seiscomp3/logging/log.h>

#include "mainwindow.h"
#include "heliwidget.h"

#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/recordstreamthread.h>


using namespace Seiscomp;


MainWindow::MainWindow() {
	_ui.setupUi(this);
	_ui.labelHeadline->setVisible(false);

	_xRes = 512;
	_yRes = 512;
	_dpi = 300;
	_rowTimeSpan = 18000;
	_timeFormat = "%F";
	_snapshotTimer = 0;

	_streamThread = NULL;
	_fixCurrentTimeToLastRecord = false;

	//_ui.labelStreamID->setFont(SCScheme.fonts.highlight);
	//_ui.labelDate->setFont(SCScheme.fonts.highlight);

	_heliWidget = new HeliWidget(_ui.frameHeli);

	_ui.frameSpacer->setFixedWidth(_heliWidget->canvas().labelWidth());

	QVBoxLayout *layout = new QVBoxLayout(_ui.frameHeli);
	layout->setMargin(0);
	_ui.frameHeli->setLayout(layout);
	layout->addWidget(_heliWidget);

	connect(_ui.actionQuit, SIGNAL(triggered(bool)), SCApp, SLOT(quit()));
	connect(&_timer, SIGNAL(timeout()), this, SLOT(advanceTime()));

	/*
	QAction *printAction = new QAction(this);
	printAction->setShortcut(QKeySequence("Ctrl+P"));
	connect(printAction, SIGNAL(triggered(bool)), this, SLOT(print()));
	addAction(printAction);
	*/

	QVector<QColor> colors;
	colors.append(SCScheme.colors.records.foreground);
	colors.append(SCScheme.colors.records.alternateForeground);

	_heliWidget->canvas().setRowColors(colors);
}


MainWindow::~MainWindow() {
	if ( _streamThread ) {
		_streamThread->stop(true);
		_streamThread->wait(2000);
		delete _streamThread;
	}
}


void MainWindow::fixCurrentTimeToLastRecord(bool e) {
	if ( _fixCurrentTimeToLastRecord == e ) return;

	_fixCurrentTimeToLastRecord = e;
	if ( _fixCurrentTimeToLastRecord ) {
		_timer.stop();
		_heliWidget->setCurrentTime(_globalEndTime - Core::TimeSpan(0,1));
		updateTimeLabel(_globalEndTime);
	}
	else {
		_timer.start(1000);
		advanceTime();
	}
}


void MainWindow::toggledFullScreen(bool val) {
	QMenuBar* pMenuBar = menuBar();
	if ( pMenuBar )
		pMenuBar->setVisible(!val);

	QStatusBar *pStatusBar = statusBar();
	if ( pStatusBar )
		pStatusBar->setVisible(!val);
}


void MainWindow::setReferenceTime(const Seiscomp::Core::Time &time) {
	_referenceTime = time;
}

void MainWindow::setGain(const float gain){
	_heliWidget->canvas().setScale(1 / gain);
}

void MainWindow::setHeadline(const QString headline){
	_ui.labelHeadline->setText(headline);
	_ui.labelHeadline->setVisible(!headline.isEmpty());
}

void MainWindow::setPostProcessingScript(const std::string &path) {
	_imagePostProcessingScript = path;
}

void MainWindow::setStream(Seiscomp::DataModel::WaveformStreamID streamID) {
	_streamID = streamID;

	_ui.labelStreamID->setText(QString("%1.%2.%3.%4")
	                           .arg(_streamID.networkCode().c_str())
	                           .arg(_streamID.stationCode().c_str())
	                           .arg(_streamID.locationCode().c_str())
	                           .arg(_streamID.channelCode().c_str()));
}

void MainWindow::setStream(const std::string &networkCode,
                           const std::string &stationCode,
                           const std::string &locationCode,
                           const std::string &channelCode) {
	_streamID.setNetworkCode(networkCode);
	_streamID.setStationCode(stationCode);
	_streamID.setLocationCode(locationCode);
	_streamID.setChannelCode(channelCode);

	_ui.labelStreamID->setText(QString("%1.%2.%3.%4")
	                           .arg(_streamID.networkCode().c_str())
	                           .arg(_streamID.stationCode().c_str())
	                           .arg(_streamID.locationCode().c_str())
	                           .arg(_streamID.channelCode().c_str()));
}


void MainWindow::setFilter(const std::string &filter) {
	if ( !_heliWidget->canvas().setFilter(filter) ) {
		if ( !filter.empty() ) {
			QMessageBox::critical(this, "Filter",
			                      QString("Unable to create filter '%1'").arg(filter.c_str()));
		}
	}
}


void MainWindow::setAmplitudeRange(double min, double max) {
	_heliWidget->canvas().setAmplitudeRange(min, max);
}


void MainWindow::setRowColors(const QVector<QColor> &cols) {
	_heliWidget->canvas().setRowColors(cols);
}


void MainWindow::setStationDescriptionEnabled(bool e) {
	_showStationDescription = e;
}


void MainWindow::setAntialiasingEnabled(bool e) {
	_heliWidget->canvas().setAntialiasingEnabled(e);
}


void MainWindow::setLineWidth(int lw) {
	_heliWidget->canvas().setLineWidth(lw);
}


void MainWindow::setLayout(int rows, int secondsPerRow) {
	_rowTimeSpan = secondsPerRow;
	_heliWidget->canvas().setLayout(rows, secondsPerRow);
	_fullTimeSpan = Core::TimeSpan(rows * secondsPerRow);
}


void MainWindow::setOutputResolution(int xres, int yres, int dpi) {
	_xRes = xres;
	_yRes = yres;
	_dpi = dpi;
}


void MainWindow::setSnapshotTimeout(int timeout) {
	if ( timeout > 0 )
		_snapshotTimer = startTimer(timeout*1000);
	else if ( _snapshotTimer > 0 ) {
		killTimer(_snapshotTimer);
		_snapshotTimer = 0;
	}
}


void MainWindow::setTimeFormat(const std::string &timeFormat) {
	_timeFormat = timeFormat;
}


void MainWindow::timerEvent(QTimerEvent *event) {
	if ( event->timerId() != _snapshotTimer ) return;
	if ( !_dumpFilename.isEmpty() ) print(_dumpFilename);
}


void MainWindow::print(QString filename) {
	_heliWidget->canvas().save(_ui.labelStreamID->text(), _ui.labelHeadline->text(),
	                           _ui.labelDate->text(),
	                           filename, _xRes, _yRes, _dpi);

	if ( !_imagePostProcessingScript.empty() ) {
		QProcess proc;
		proc.start(_imagePostProcessingScript.c_str(), QStringList() << filename);
		if ( !proc.waitForStarted() ) {
			SEISCOMP_ERROR("Failed to start script: %s %s",
			               _imagePostProcessingScript.c_str(),
			               qPrintable(filename));
		}
		else if ( !proc.waitForFinished() ) {
			SEISCOMP_ERROR("Script exited with error: %s %s",
			               _imagePostProcessingScript.c_str(),
			               qPrintable(filename));
		}
	}
}


void MainWindow::start(QString dumpFilename) {
	_dumpFilename = dumpFilename;

	if ( !_fixCurrentTimeToLastRecord ) {
		advanceTime();
		_timer.start(1000);
	}

	startAcquisition();
}


void MainWindow::startAcquisition() {
	if ( _streamThread != NULL ) {
		_streamThread->wait(2000);
		delete _streamThread;
	}

	_streamThread = new Gui::RecordStreamThread(SCApp->recordStreamURL());
	if ( !_streamThread->connect() ) {
		QMessageBox::critical(this, "Waveform acquisition",
		                      QString("Unable to open recordstream '%1'").arg(SCApp->recordStreamURL().c_str()));
		close();
	}

	connect(_streamThread, SIGNAL(receivedRecord(Seiscomp::Record*)),
	        this, SLOT(receivedRecord(Seiscomp::Record*)));

	connect(_streamThread, SIGNAL(finished()),
	        this, SLOT(acquisitionFinished()));

	_streamThread->addStream(_streamID.networkCode(),
	                         _streamID.stationCode(),
	                         _streamID.locationCode(),
	                         _streamID.channelCode(),
	                         (_referenceTime.valid()?_referenceTime:Core::Time::GMT()) - _heliWidget->canvas().recordsTimeSpan(),
	                         _referenceTime);

	_streamThread->start();
}


void MainWindow::acquisitionFinished() {
	delete _streamThread;
	_streamThread = NULL;
}


void MainWindow::receivedRecord(Record *rec) {
	try {
		Core::Time endTime = rec->endTime();

		if ( !_globalEndTime ) {
			_globalEndTime = endTime;
			if ( _fixCurrentTimeToLastRecord ) {
				_heliWidget->setCurrentTime(_globalEndTime - Core::TimeSpan(0,1));
				updateTimeLabel(_globalEndTime);
			}
		}
		else if ( endTime > _globalEndTime ) {
			_globalEndTime = endTime;
			if ( _fixCurrentTimeToLastRecord ) {
				_heliWidget->setCurrentTime(_globalEndTime - Core::TimeSpan(0,1));
				updateTimeLabel(_globalEndTime);
			}
		}

		_heliWidget->feed(rec);
	}
	catch ( ... ) {}
}


void MainWindow::updateTimeLabel(const Core::Time &time) {
	QString from = (time-_fullTimeSpan).toString(_timeFormat.c_str()).c_str();
	QString to = (time-Core::TimeSpan(0,1)).toString(_timeFormat.c_str()).c_str();

	if ( from != to && !from.isEmpty() && !to.isEmpty() )
		_ui.labelDate->setText(QString("%1 - %2").arg(from).arg(to));
	else
		_ui.labelDate->setText(QString("%1").arg(to));
}


void MainWindow::advanceTime() {
	if ( _fixCurrentTimeToLastRecord ) return;

	if ( _referenceTime.valid() ) {
		_heliWidget->setCurrentTime(_referenceTime - Core::TimeSpan(0,1));
		updateTimeLabel(_referenceTime);
	}
	else {
		Core::Time now = Core::Time::GMT();
		_heliWidget->setCurrentTime(now);
		updateTimeLabel(now);
	}
}
