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




#define SEISCOMP_COMPONENT Gui::QcView
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/gui/core/utils.h>

#include "qcmodel.h"
#include "qcviewconfig.h"


namespace Seiscomp {
namespace Applications {
namespace Qc {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString getStreamID(const DataModel::WaveformStreamID& wfid) {

	std::string streamID = wfid.networkCode()+"."+wfid.stationCode()+"."+
	                       wfid.locationCode()+"."+wfid.channelCode();
	
	return QString(streamID.c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QcModel::QcModel(const QcViewConfig* config, QObject* parent)
	: QAbstractTableModel(parent),
	  _config(config) {
	
	// delete alertMsg after x sec time
	_cleanUpTime = 300.0;
	_dataChanged = false;

	addColumn("streamID");
	addColumn("enabled");
	// retrieve a list of available qcParameters and
	// create colums for them
	QStringList qcp = _config->parameter();
	QStringList::iterator it;
	for (it = qcp.begin(); it != qcp.end(); it++) {
		addColumn(*it);
	}

// 	_timer.setSingleShot(true);
	_timer.start(1000);
	connect(&_timer, SIGNAL(timeout()), this, SLOT(timeout()));

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QcViewConfig* QcModel::config() const {

	return _config;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcModel::addColumn(const QString& qcParameterName) {

	_columns.append(qcParameterName);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcModel::timeout() {
	if (_dataChanged) {
		reset();
		_dataChanged = false;
	}

	cleanUp();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcModel::dataChanged() {

	_dataChanged = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcModel::reset() {

	QAbstractItemModel::reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcModel::setCleanUpTime(double time) {

	_cleanUpTime = time;

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QcModel::hasAlerts(const QString& streamID) {

	if (_streamMap.empty())
		return false;

	if (_streamMap.contains(streamID))
		return  _streamMap.value(streamID).alert.count(NULL) != _columns.size();

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcModel::setStreams(const std::list<std::pair<std::string, bool> >& streams) {

	for (std::list<std::pair<std::string, bool> >::const_iterator it = streams.begin(); it != streams.end(); ++it) {
		addStream(QString(it->first.c_str()), it->second);
	}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcModel::setStationEnabled(const QString& network, const QString& station, bool enabled) {

	QRegExp netSta("^"+network+"\\."+station+".*$");

	foreach (QString streamID, _streamMap.keys()) {
		if (streamID.contains(netSta))
			setStreamEnabled(streamID, enabled);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcModel::setStreamEnabled(const QString& streamID, bool enabled) {

	if (_streamMap.contains(streamID))
		_streamMap[streamID].enabled = enabled;

	dataChanged();
	//reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcModel::setStreamEnabled(const QModelIndex& index, bool enabled) {

	(_streamMap.begin()+index.row()).value().enabled = enabled;

	// trigger: send configStation message
	emit stationStateChanged((_streamMap.begin()+index.row()).key(), enabled);

	reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QcModel::streamEnabled(const QModelIndex& index) const {

	return (_streamMap.begin()+index.row()).value().enabled;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcModel::addStream(const QString& streamID, bool enabled) {

	StreamEntry se;
	se.streamID = streamID;
	se.enabled = enabled;
	se.report.fill(NULL, _columns.size());
	se.alert.fill(NULL, _columns.size());

	if (_streamMap.contains(streamID))
		_streamMap[streamID].enabled = enabled;
	else
		_streamMap.insert(streamID, se);

	dataChanged();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcModel::removeStream(const QString& streamID) {

	_streamMap.remove(streamID);

	reset();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcModel::setWaveformQuality(DataModel::WaveformQuality* wfq) {

	if (!wfq) return;
	if (_columns.isEmpty()) return;

	int column = _columns.indexOf(QString(wfq->parameter().c_str()));

	// ignore unknown (not/wrong specified in cfg file) parameters
	if (column == -1) {
// 		SEISCOMP_DEBUG("setWaveformQuality: unknown parameter received: %s", wfq->parameter().c_str());
		return;
	}
	
	QString streamID = getStreamID(wfq->waveformID());

	if (!_streamMap.contains(streamID)) {
		if (_config->cumulative())
			addStream(streamID);
		else
			return;
	}	
	
	WfQList* wfqList;

	if (wfq->type() == "report")
		wfqList = &_streamMap[streamID].report;
	else if (wfq->type() == "alert")
		wfqList = &_streamMap[streamID].alert;
	else return;

	
	(*wfqList)[column] = wfq;

	dataChanged();

/*	QModelIndex inx = createIndex(_streamMap.keys().indexOf(streamID), column);
	emit QAbstractItemModel::dataChanged(inx, inx);*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcModel::cleanUp() {
	
	if (_cleanUpTime == 0.0)
		return;

	for (StreamMap::iterator it = _streamMap.begin(); it != _streamMap.end(); ++it) {
		WfQList wfqList = it.value().alert;

		for (WfQList::iterator it2 = wfqList.begin(); it2 != wfqList.end(); ++it2) {
			const DataModel::WaveformQuality* wfq = it2->get();
			if (!wfq) continue;
			try {
				double dt = (double)(Core::Time::GMT() - wfq->end());
				SEISCOMP_DEBUG("[%f s] cleaning up alert entries for: %s", dt, getStreamID(wfq->waveformID()).toAscii().data());
				if (dt > _cleanUpTime) {
					QString streamID = getStreamID(wfq->waveformID());
 					SEISCOMP_WARNING("[%f s] cleaning up alert entries for: %s", dt, getStreamID(wfq->waveformID()).toAscii().data());
					*it2 = NULL;
				}
			}
			catch(...){SEISCOMP_ERROR("cleaning up alert entries FAILED!");}
		}
	
	}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// void QcModel::dataChanged() {

// }
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int QcModel::rowCount(const QModelIndex &) const {

	return _streamMap.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int QcModel::columnCount(const QModelIndex &) const {

	return _columns.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QString& QcModel::getHeader(int section) const {

	return _columns.at(section);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString QcModel::getKey(const QModelIndex& index) const {

	try {return (_streamMap.begin()+index.row()).key();}
	catch (...){};

	return "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::WaveformQuality* QcModel::getAlertData(const QModelIndex& index) const {

	if (_streamMap.empty())
		return NULL;

	const DataModel::WaveformQuality* wfq = NULL;

	QString streamID = getKey(index);

	if (_streamMap.contains(streamID)) {
		wfq = _streamMap.value(streamID).alert.value(index.column()).get();
	}

	return wfq;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::WaveformQuality* QcModel::getData(const QModelIndex& index) const {
	
	if (_streamMap.empty())
		return NULL;

	DataModel::WaveformQuality* wfq = NULL;
	
	try {
		wfq = (_streamMap.begin()+index.row()).value().report.value(index.column()).get();
	}
	catch(...){}

	if (!wfq) return NULL;

	try {
		if (wfq->windowLength() == -1.0) // HACK
			return NULL;
	}
	catch(...) {;;}
	
	try {
		if (_config->expired(_columns.at(index.column()), (double)(Core::Time::GMT() - wfq->end())))
			return NULL;
		else
			return wfq;
	}
	catch (std::exception& e) {
 		qDebug() << e.what();
		return NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QColor QcModel::getColor(const QModelIndex& index) const {

	if (data(index, Qt::UserRole).isValid() ) {
		return _config->color(_columns.at(index.column()), data(index, Qt::UserRole).toDouble());
	}

	return _config->color("default", 1.0);

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant QcModel::data(const QModelIndex &index, int role) const {

	if ( !index.isValid() )
		return QVariant();

	if ( index.row() >= _streamMap.size() || index.column() >= _columns.size() )
		return QVariant();


	//!---------------------------------------------------------------------------------
	if ( role == Qt::DisplayRole ) {
		if (index.column() == 0) {
			return getKey(index);
		}
		if (index.column() == 1) {
			return streamEnabled(index)?"on":"off";
		}

		const DataModel::WaveformQuality* wfq = getData(index);
		if (wfq)
			return _config->format(_columns.at(index.column()), wfq->value());
		else
			return QVariant();
	}
	//!---------------------------------------------------------------------------------


	//!---------------------------------------------------------------------------------
	// for correct sorting and coloring: return 'raw' data
	if ( role == Qt::UserRole ) {
		if (index.column() == 0) {
			return getKey(index);
		}
		if (index.column() == 1) {
			return streamEnabled(index);
		}

		const DataModel::WaveformQuality* wfq = getData(index);
		if (wfq) {
			if (_config->useAbsoluteValue(_columns.at(index.column())))
				return fabs(wfq->value());
			else
				return wfq->value();
		}
	 	else
			return QVariant();
	}
	//!---------------------------------------------------------------------------------


	//!---------------------------------------------------------------------------------
	else if ( role == Qt::TextAlignmentRole ) {
		// stream enabled column
		if (index.column() == 1) {
			return int(Qt::AlignCenter | Qt::AlignVCenter);
		}
		return int(Qt::AlignRight | Qt::AlignVCenter);
	}
	//!---------------------------------------------------------------------------------


	//!---------------------------------------------------------------------------------
	else if ( role == Qt::BackgroundColorRole ) {

		if (index.column() == 1) {
			return streamEnabled(index)?QColor(0,255,0,255):QColor(255,0,0,255);
		}


		return getColor(index);
	}
	//!---------------------------------------------------------------------------------


	//!---------------------------------------------------------------------------------
	else if ( role == Qt::ToolTipRole ) {

		const DataModel::WaveformQuality* wfq = getData(index);
		QString text1, text2;

		if (wfq)
			text1 = "Report Message:\n" + wfq2str(wfq);

			
		if ( (wfq = getAlertData(index)) )
			text2 = "\nAlert Message:\n" + wfq2str(wfq);
		
		return text1 + text2;
}
	//!---------------------------------------------------------------------------------

	return QVariant();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString QcModel::wfq2str(const DataModel::WaveformQuality* wfq) const {

	QString text = QString();

	text.append(QString("value: %1 (+/- %2)\n").arg(wfq->value()).arg(wfq->lowerUncertainty()));
	text.append(QString("start: %1\n").arg(Gui::timeToString(wfq->start(), "%Y-%m-%d %H:%M:%S UTC")));
	try { text.append(QString("end : %1\n").arg(Gui::timeToString(wfq->end(), "%Y-%m-%d %H:%M:%S UTC"))); }
	catch(...) {};
	try { text.append(QString("window length: %1 s").arg(wfq->windowLength())); }
	catch(...) {};

	return text+"\n";

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant QcModel::headerData(int section, Qt::Orientation orientation, int role) const {

	if ( orientation == Qt::Horizontal ) {
		switch ( role ) {
			case Qt::DisplayRole:
				if ( section >= _columns.size() )
					return QString("%1").arg(section);
				else
					return getHeader(section);
				break;
			case Qt::TextAlignmentRole:
				return int(Qt::AlignCenter | Qt::AlignVCenter);
		}
	}
	else {
		switch ( role ) {
			case Qt::DisplayRole:
				if ( section >= _streamMap.size() )
					return QString("%1").arg(section);
				else {
					return (_streamMap.begin()+section).key();
				}
				break;
			case Qt::TextAlignmentRole:
				return int(Qt::AlignLeft | Qt::AlignVCenter);
		}
	}

	return QVariant();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





}
}
}
