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




#ifndef __QCMODEL_H__
#define __QCMODEL_H__

#include <QtGui>
#include <utility>

#ifndef Q_MOC_RUN
#include <seiscomp3/datamodel/waveformquality.h>
#endif

namespace Seiscomp {
namespace Applications {
namespace Qc {

class QcViewConfig;


typedef QVector<DataModel::WaveformQualityPtr> WfQList; // <indexRow, WfQ>

typedef struct  {
	QString streamID;
	bool enabled;
	WfQList report;
	WfQList alert;
} StreamEntry;

typedef QMap<QString, StreamEntry> StreamMap; // <StreamID, StreamEntry>


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class QcModel : public QAbstractTableModel {
	Q_OBJECT

	private:


	public:
		QcModel(const QcViewConfig* config, QObject* parent=0);
		const QcViewConfig* config() const;

		void setStreams(const std::list<std::pair<std::string, bool> >& streams);
		void setCleanUpTime(double time);
		void addColumn(const QString& qcParameterName);
		void addStream(const QString& streamID, bool enabled=true);
		void removeStream(const QString& streamID);
		void setWaveformQuality(DataModel::WaveformQuality* wfq);

		void setStationEnabled(const QString& net, const QString& sta, bool enabled);
		void setStreamEnabled(const QString& streamID, bool enabled);
		void setStreamEnabled(const QModelIndex& index, bool enabled);
		bool streamEnabled(const QModelIndex& index) const;
	
		int rowCount(const QModelIndex& parent = QModelIndex()) const;
		int columnCount(const QModelIndex& parent = QModelIndex()) const;

		QVariant data(const QModelIndex& index, int role) const;
		QVariant rawData(const QModelIndex& index, int role) const;

		QVariant headerData(int section, Qt::Orientation orientation,
		                    int role = Qt::DisplayRole) const;

		const QString& getHeader(int section) const ;
		bool hasAlerts(const QString& streamID);
		const DataModel::WaveformQuality* getAlertData(const QModelIndex& index) const;
		const DataModel::WaveformQuality* getData(const QModelIndex& index) const;
		QString getKey(const QModelIndex& index) const;
		QColor getColor(const QModelIndex& index) const;

	signals:
		void stationStateChanged(QString, bool);
		
	private slots:
		void timeout();
		void reset();
		void cleanUp();

	private:
		const QcViewConfig* _config;
		QTimer _timer;
		QStringList _columns;
		StreamMap _streamMap;
		bool _dataChanged;
		void dataChanged();
		double _cleanUpTime;
		QString wfq2str(const DataModel::WaveformQuality* wfq) const;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}
}
}
#endif
