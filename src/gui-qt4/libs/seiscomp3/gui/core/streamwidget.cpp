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


#define SEISCOMP_COMPONENT StreamWidget
#include "streamwidget.h"

#include <algorithm>

#include <QStringList>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QSplitter>
#include <QScrollBar>

#include <seiscomp3/gui/core/recordview.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp
{
namespace Gui
{

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StreamWidget::StreamWidget(const std::string& recordStreamURL,
                       const std::string& waveformStreamID,
                       const double windowLength,
                       QWidget* parent) :
		QWidget(parent),
		_timer(NULL),
		_recordSequence(NULL),
		_waveformStreamID(waveformStreamID)
{
	setWindowTitle("StreamWidget");
	setWindowFlags(Qt::Window | Qt::WindowStaysOnTopHint);
	setAttribute(Qt::WA_DeleteOnClose);
	setGeometry(0, 0, 500, 180);
	_recordStreamURL = recordStreamURL;

	activateWindow();

 	setLayout(new QVBoxLayout(this));

	// Initialize permanent top level items (Order is important!)

	// Initialize trace widget
	_ringBufferSize.set((long int)windowLength);
	_recordWidget = new RecordWidget();
		
/*	Math::Filtering::InPlaceFilter<float>* filter = Util::createFilterByName<float>(parameter.filterName);
	if (filter) {
		_recordWidget->setFilter(filter);
		delete filter;
	}*/

	_recordWidget->setMinimumHeight(40);
	QPalette palette;
	palette.setColor(_recordWidget->backgroundRole(), Qt::white);
	palette.setColor(_recordWidget->foregroundRole(), Qt::darkGray);
	_recordWidget->setPalette(palette);

	QVBoxLayout* boxLayout = new QVBoxLayout();
	boxLayout->setMargin(1);
	boxLayout->addWidget(_recordWidget);

	_timeScale = new TimeScale();
	_timeScale->setMinimumHeight(15);
	_timeScale->setAbsoluteTimeEnabled(false);
	boxLayout->addWidget(_timeScale);

	_groupBox = new QGroupBox("Waveform Data");
	_groupBox->setVisible(false);
	_groupBox->setLayout(boxLayout);

	layout()->addWidget(_groupBox);

	connect(_timeScale, SIGNAL(changedInterval(double, double, double)),
	        _recordWidget, SLOT(setGridSpacing(double, double, double)));
	// Initialize thread for reading waveform data
	_thread = std::auto_ptr<RecordStreamThread>(new RecordStreamThread(_recordStreamURL));

	startWaveformDataAcquisition();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StreamWidget::~StreamWidget()
{
	stopWaveformDataAcquisition();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::startWaveformDataAcquisition()
{
	QStringList tokens = QString(_waveformStreamID.c_str()).split(".");
	if (tokens.size() < 4)
		return;
	std::string net = tokens[0].toStdString();
	std::string sta = tokens[1].toStdString();
	std::string loc = tokens[2].toStdString();
	std::string cha = tokens[3].toStdString();

	if (!_thread->connect())
	{
		QMessageBox::information(this, "thread", "Could not connect");
		return;
	}
	// _thread->addStream(station->network()->code(), station->code(), stream->locCode(), stream->code() + "Z");
	_thread->addStream(net, sta, loc, cha);
	Core::Time startTime = Core::Time::GMT() - _ringBufferSize;
	Core::Time endTime/* = Core::Time::GMT()*/;
	_thread->setTimeWindow(Core::TimeWindow(startTime, endTime));

	_recordSequence = new RingBuffer(_ringBufferSize);
	_recordWidget->setRecords(0, _recordSequence);
	connect(_thread.get(), SIGNAL(receivedRecord(Seiscomp::Record*)),
	        this, SLOT(updateRecordWidget(Seiscomp::Record*)));

	_thread->start();
	
	_groupBox->setTitle(QString("Waveform Data: %1")
	                    .arg((net + "." + sta + "." + loc + "." + cha).c_str()));
	_groupBox->setVisible(!_groupBox->isVisible());
	_recordWidget->setTimeScale(_recordWidget->width() / double(_ringBufferSize.seconds()));
	_recordWidget->setTimeRange(-1 * _ringBufferSize.seconds(), 0);
	_recordWidget->setAlignment(Core::Time::GMT());
	_timeScale->setScale(_recordWidget->width() / double(_ringBufferSize.seconds()));
	_timeScale->setTimeRange(-1 * _ringBufferSize.seconds(), 0);


	if ( !_timer )
		_timer = new QTimer(this);

	_timer->setInterval(1000);
	connect(_timer, SIGNAL(timeout()), this, SLOT(updateRecordWidgetAlignment()));

	_timer->start();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::stopWaveformDataAcquisition()
{
	_groupBox->setVisible(!_groupBox->isVisible());

	// Delete waveform data
	_thread->stop(true);

	disconnect(_thread.get(), SIGNAL(receivedRecord(Seiscomp::Record*)),
	           this, SLOT(updateRecordWidget(Seiscomp::Record*)));
	/*
	if (_recordSequence)
		delete _recordSequence;
	*/
	_recordWidget->setRecords(0, NULL);
	_recordSequence = NULL;

	if ( _timer )
		_timer->stop();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::updateRecordWidgetAlignment()
{
	if ( !_recordWidget )
		return;

	_recordWidget->setAlignment(Core::Time::GMT());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::updateRecordWidget(Seiscomp::Record* record)
{
	Seiscomp::RecordPtr recordPtr(record);

	if (_recordSequence == NULL)
		return;

	if (_recordSequence->feed(recordPtr.get()))
		_recordWidget->fed(0, recordPtr.get());

	_recordWidget->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::closeEvent(QCloseEvent*)
{
	emit StreamWidgetClosed(this);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::showEvent(QShowEvent*)
{
	// Use the same code as resizeEvent which does not depend on the
	// parameter QResizeEvent*
	// Reason: When the widget is shown for the first time the resizeEvent
	// handler is called before all child widgets have been layouted and
	// resized properly. So the record scale will be calculated based on
	// a wrong _recordWidget width. The showEvent handler is called after
	// everything has been resized and layouted and we make sure that the
	// record scale has the value we want it to be.
	resizeEvent(NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StreamWidget::resizeEvent(QResizeEvent*)
{
	_recordWidget->setTimeScale( _recordWidget->width() / double(_ringBufferSize.seconds()));
	_recordWidget->setTimeRange(-_ringBufferSize.seconds(), 0);

	_timeScale->setScale( _recordWidget->width() / double(_ringBufferSize.seconds()));
	_timeScale->setTimeRange(-1 * _ringBufferSize.seconds(), 0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Seiscomp
} // namespace Gui
