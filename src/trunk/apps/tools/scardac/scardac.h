/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *   EMail: jabe@gempa.de                                                  *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

#ifndef __SEISCOMP_APPLICATIONS_SCARDAC_H__
#define __SEISCOMP_APPLICATIONS_SCARDAC_H__

#include <seiscomp3/client/application.h>
#include <seiscomp3/client/queue.h>
#include <seiscomp3/client/queue.ipp>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/dataavailability.h>
#include <seiscomp3/datamodel/dataextent.h>
#include <seiscomp3/datamodel/dataattributeextent.h>
#include <seiscomp3/datamodel/datasegment.h>
#include <seiscomp3/datamodel/databasearchive.h>

#include <string>

namespace Seiscomp {
namespace Applications {

class SCARDAC;

class Worker {
	public:
		Worker(const SCARDAC *app, int id);

		void processExtent(DataModel::DataExtent *extent, bool foundInDB);

	private:
		typedef std::vector<DataModel::DataSegmentPtr> Segments;
		typedef std::vector<std::pair<DataModel::DataSegmentPtr,
		                              DataModel::Operation> > SegmentDBBuffer;

		bool dbConnect(DataModel::DatabaseReaderPtr &db, const char *info);

		DataModel::DatabaseIterator dbSegments();
		bool readFileSegments(Segments &segments, const std::string &file,
		                      const Core::Time &mtime);

		bool findDBSegment(DataModel::DatabaseIterator &it,
		                   const DataModel::DataSegment *segment);

		void removeSegment(DataModel::DataSegmentPtr segment);
		void addSegment(DataModel::DataSegmentPtr segment);
		void flushSegmentBuffers();

		bool writeExtent(DataModel::Operation op);
		bool syncAttributeExtents(const DataModel::DataExtent &tmpExt);

	private:
		const SCARDAC*                          _app;
		int                                     _id;
		DataModel::DatabaseReaderPtr            _dbRead;
		DataModel::DatabaseReaderPtr            _dbWrite;

		// variables valid per extent
		DataModel::DataExtent                  *_extent;
		std::string                             _sid;
		Segments                                _segmentsRemove;
		Segments                                _segmentsAdd;
		DataModel::DataSegmentPtr               _currentSegment;
};

class SCARDAC : public Seiscomp::Client::Application {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		SCARDAC(int argc, char **argv);

		//! Destructor
		~SCARDAC();

	// ----------------------------------------------------------------------
	//  Protected types
	// ----------------------------------------------------------------------
	protected:
		struct WorkQueueItem {
			WorkQueueItem() : extent(NULL), foundInDB(false) {}
			WorkQueueItem(DataModel::DataExtent *extent, bool foundInDB)
			 : extent(extent), foundInDB(foundInDB) {}

			DataModel::DataExtent  *extent;
			bool                    foundInDB;
		};

	// ----------------------------------------------------------------------
	//  Protected functions
	// ----------------------------------------------------------------------
	protected:
		void createCommandLineDescription();
		bool initConfiguration();
		bool validateParameters();
		bool run();

		void checkDirectory(int level, const std::string &dir);
		void checkFiles(const std::string &dir);

		void processExtents(int threadID);
		bool generateTestData();

	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	protected:
		// configuration parameters
		int                             _threads;
		int                             _batchSize;
		float                           _jitter;
		bool                            _deepScan;
		int                             _maxSegments;
		std::string                     _testData;

		std::string                     _archive;
		std::vector<std::string>        _archiveYears;
		DataModel::DataAvailability     _dataAvailability;

		// Map of stream ID to extents
		typedef std::map<std::string, DataModel::DataExtent*> ExtentMap;
		ExtentMap                       _extentMap;

		// Thread safe queue of extends to process
		Client::ThreadedQueue<WorkQueueItem> _workQueue;

		typedef std::vector<boost::thread*> WorkerList;
		WorkerList                      _worker;

	private:
		friend class Worker;
};

} // ns Applications
} // ns Seiscomp

#endif // __SEISCOMP_APPLICATIONS_SCARDAC_H__

