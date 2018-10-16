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



#ifndef __SEISCOMP_GUI_CALCULATEAMPLITUDES_H__
#define __SEISCOMP_GUI_CALCULATEAMPLITUDES_H__

#include <seiscomp3/gui/core/recordstreamthread.h>
#ifndef Q_MOC_RUN
#include <seiscomp3/datamodel/timewindow.h>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/processing/amplitudeprocessor.h>
#endif
#include <seiscomp3/gui/qt4.h>

#include <QDialog>
#include <map>
#include <set>

#include <seiscomp3/gui/datamodel/ui_calculateamplitudes.h>


namespace Seiscomp {

class Record;

namespace DataModel {

DEFINE_SMARTPOINTER(Amplitude);
class Pick;
class Origin;
class DatabaseQuery;

}


namespace Gui {


class SC_GUI_API CalculateAmplitudes : public QDialog {
	Q_OBJECT

	public:
		typedef std::pair<DataModel::AmplitudePtr, bool> AmplitudeEntry;
		typedef std::multimap<std::string, AmplitudeEntry> PickAmplitudeMap;
		typedef PickAmplitudeMap::iterator iterator;
		typedef std::pair<iterator, iterator> iterator_range;
		typedef std::set<std::string> TypeSet;


	public:
		CalculateAmplitudes(DataModel::Origin *origin,
		                    QWidget * parent = 0, Qt::WindowFlags f = 0);
		~CalculateAmplitudes();

		void setOrigin(DataModel::Origin *origin);
		void setStreamURL(const std::string& streamURL);
		void setDatabase(DataModel::DatabaseQuery *);

		void setRecomputeAmplitudes(bool);
		void setAmplitudeCache(PickAmplitudeMap *cache);

		void setAmplitudeTypes(const TypeSet &types);


	public:
		int exec();
		void done(int r);

		bool process();

		//! Iterate over the computed/fetched amplitudes.
		iterator begin();
		iterator end();

		//! Returns the amplitudes belonging to a pick that has been
		//! fetched or calculated
		iterator_range pickAmplitudes(const std::string &pickID);

		iterator amplitude(const std::string &amplitudeID);

		bool isNewlyCreated(const iterator&) const;
		DataModel::AmplitudePtr amplitude(const iterator&) const;

		//! Sets the state of an amplitude that stand for 'newly created'
		void setState(iterator it, bool);

		void setSilentMode(bool f);


	private slots:
		void receivedRecord(Seiscomp::Record*);
		void finishedAcquisition();
		void filterStateChanged(int index);
		void filterTypeChanged(int index);


	private:
		void closeAcquisition();

		void addProcessor(const std::string &type,
		                  const DataModel::Pick *pick,
		                  const DataModel::SensorLocation *loc,
		                  double dist);

		void addProcessor(Processing::AmplitudeProcessor *,
		                  const DataModel::Pick *pick,
		                  int c);

		void subscribeData(Processing::AmplitudeProcessor *,
		                   const DataModel::Pick *pick,
		                   int c);

		void checkPriority(const AmplitudeEntry &newAmp);

		int addProcessingRow(const std::string &streamID, const std::string &type);

		void emitAmplitude(const Processing::AmplitudeProcessor *,
		                   const Processing::AmplitudeProcessor::Result &res);

		void setError(int row, QString text);
		void setMessage(int row, QString text);
		void setProgress(int row, int progress);
		void setValue(int row, double value);

		void filterView(int startRow = 0, int cnt = -1);


	private:
		typedef std::vector<Processing::AmplitudeProcessorPtr> ProcessorSlot;
		typedef std::map<std::string, ProcessorSlot> ProcessorMap;
		typedef std::multimap<Processing::AmplitudeProcessorCPtr, int> TableRowMap;
		typedef std::map<std::string, Seiscomp::Util::KeyValuesPtr> ParameterMap;
		typedef std::map<std::string, Seiscomp::Processing::StreamPtr> StreamMap;

		::Ui::CalculateAmplitudes _ui;

		ProcessorMap              _processors;
		ParameterMap              _parameters;
		StreamMap                 _streams;
		TableRowMap               _rows;
		PickAmplitudeMap          _amplitudes;
		PickAmplitudeMap         *_externalAmplitudeCache;
		Core::TimeWindow          _timeWindow;
		RecordStreamThread       *_thread;
		DataModel::DatabaseQuery *_query;
		DataModel::Origin        *_origin;
		TypeSet                   _amplitudeTypes;

		bool                      _recomputeAmplitudes;
		bool                      _computeSilently;
};


}

}


#endif
