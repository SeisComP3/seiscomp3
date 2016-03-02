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


#ifndef __SEISCOMP_GUI_CORE_STREAMWIDGET_H__
#define __SEISCOMP_GUI_CORE_STREAMWIDGET_H__

#include <vector>
#include <map>
#include <list>

#include <QWidget>
#include <QGroupBox>

#ifndef Q_MOC_RUN
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/recordsequence.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/timewindow.h>
#include <seiscomp3/gui/core/recordstreamthread.h>
#include <seiscomp3/gui/core/recordwidget.h>
#include <seiscomp3/gui/core/timescale.h>
#include <seiscomp3/gui/qt4.h>
#endif


namespace Seiscomp { 
namespace Gui {


class SC_GUI_API StreamWidget : public QWidget {

	Q_OBJECT


public:
	StreamWidget(const std::string& recordStreamURL,
                 const std::string& waveformStreamID,
                 const double windowLength=600,
                 QWidget* parent=0);

	~StreamWidget();
	

protected:
	virtual void closeEvent(QCloseEvent*);
	virtual void showEvent(QShowEvent*);
	virtual void resizeEvent(QResizeEvent* evt);
	
	
signals:
	void StreamWidgetClosed(StreamWidget* widget);
	
	
private slots:
	void updateRecordWidget(Seiscomp::Record* record);
	void updateRecordWidgetAlignment();

	
private:
	void startWaveformDataAcquisition();
	void stopWaveformDataAcquisition();

	
private:
		
	QTimer* _timer;
	
	QGroupBox*                        _groupBox;
	std::auto_ptr<RecordStreamThread> _thread;
	RecordWidget*                     _recordWidget;
	RecordSequence*                   _recordSequence;
	Core::TimeSpan                    _ringBufferSize;
	TimeScale*                        _timeScale;
	std::string                       _recordStreamURL;
	std::string                       _waveformStreamID;
};


} // namespace Gui
} // namespace Seiscomp

#endif
