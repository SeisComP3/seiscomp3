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

#define SEISCOMP_COMPONENT SCARDAC

#define MAX_THREADS 1000
#define MAX_BATCHSIZE 1000

#include <seiscomp3/core/system.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/io/database.h>
#include <seiscomp3/io/recordstream/file.h>
#include <seiscomp3/io/recordinput.h>
#include <seiscomp3/io/records/mseedrecord.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/utils/files.h>

#include <boost/filesystem/convenience.hpp>

#include <ctime>
#include <time.h>
#include <vector>

#include "scardac.h"


using namespace std;
using namespace Seiscomp::DataModel;

namespace fs = boost::filesystem;

namespace Seiscomp {

namespace {

inline
string streamID(const WaveformStreamID &wfid) {
	return wfid.networkCode() + "." + wfid.stationCode() + "." +
	       wfid.locationCode() + "." + wfid.channelCode();
}

inline
bool wfID(WaveformStreamID &wfid, const string &id) {
	vector<string> toks;
	if ( Seiscomp::Core::split(toks, id.c_str(), ".", false) != 4 )
		return false;
	wfid.setNetworkCode(toks[0]);
	wfid.setStationCode(toks[1]);
	wfid.setLocationCode(toks[2]);
	wfid.setChannelCode(toks[3]);
	return true;
}

inline
string fileStreamID(string filename) {
	size_t pos = -1;
	// NET.STA.LOC.CHA.D.YEAR.DAY
	for ( int i = 0; i < 4; ++i ) {
		pos = filename.find('.', pos+1);
		if ( pos == string::npos ) return "";
	}
	return filename.substr(0, pos);
}

inline
Core::Time fileMTime(const string &fileName) {
	Core::Time t;
	try {
		// TODO: resolve softlinks
		std::time_t mtime = fs::last_write_time(SC_FS_PATH(fileName));
		if ( mtime >= 0 ) {
			t = mtime;
			t = t.toGMT();
		}
		else {
			SEISCOMP_WARNING("could not read mtime of file: %s", fileName.c_str());
		}
	}
	catch ( ... ) {
		SEISCOMP_WARNING("file does not exist: %s", fileName.c_str());
	}
	return t;
}

inline
Core::Time fileDate(const string &fileName) {
	// NET.STA.LOC.CHA.D.YEAR.DAY
	vector<string> toks;
	int year, doy;
	Core::split(toks, fileName.c_str(), ".", false);
	if ( toks.size() == 7 &&
	     toks[5].length() == 4 && Core::fromString(year, toks[5]) &&
	     toks[6].length() == 3 && Core::fromString(doy, toks[6]) ) {
		long epochDays = (year - 1970) * 365 + doy;
		return Core::Time(epochDays * 86400, 0);
	}
	return Core::Time();
}

inline
bool equalsNoUpdated(const DataSegment *s1,
                     const DataSegment *s2) {
	return s1->start()      == s2->start() &&
	       s1->end()        == s2->end() &&
	       s1->sampleRate() == s2->sampleRate() &&
	       s1->quality()    == s2->quality() &&
	       s1->outOfOrder() == s2->outOfOrder();
}

inline
bool compareSegmentStart(DataSegmentPtr a,
                         DataSegmentPtr b) {
	return a->start() <= b->start();
}

inline
void updateExtent(DataModel::DataExtent &ext, const DataModel::DataSegment *seg) {
	// first segment: update extent start time
	if ( !ext.start() )
		ext.setStart(seg->start());

	// check for last end time which is not necessarily to be found
	// in last segment of last file
	if ( seg->end() > ext.end() )
		ext.setEnd(seg->end());

	DataAttributeExtent *attExt = ext.dataAttributeExtent(
	    DataModel::DataAttributeExtentIndex(seg->sampleRate(), seg->quality()));
	if ( attExt == NULL ) {
		attExt = new DataAttributeExtent();
		attExt->setSampleRate(seg->sampleRate());
		attExt->setQuality(seg->quality());
		attExt->setStart(seg->start());
		attExt->setEnd(seg->end());
		attExt->setUpdated(seg->updated());
		attExt->setSegmentCount(1);
		ext.add(attExt);
	}
	else {
		// update of start time not necessary since segments are process in
		// sequential order in respect to their start time
		if ( seg->end() > attExt->end() )
			attExt->setEnd(seg->end());
		if ( seg->updated() > attExt->updated() )
			attExt->setUpdated(seg->updated());
		attExt->setSegmentCount(attExt->segmentCount() + 1);
	}
}

} // ns anonymous


namespace Applications {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Worker::Worker(const SCARDAC *app, int id) : _app(app), _id(id) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Worker::processExtent(DataExtent *extent, bool foundInDB) {
	if ( extent == NULL ) return;

	const WaveformStreamID &wid = extent->waveformID();

	_extent = extent;
	_sid = streamID(wid);
	_segmentsRemove.clear();
	_segmentsAdd.clear();
	_currentSegment = NULL;

	vector<DataModel::DataAttributeExtentIndex> attributeIndices;

	SEISCOMP_INFO("[%i] %s: start processing", _id, _sid.c_str());

	// Year/NET/STA/CHA.D/NET.STA.LOC.CHA.D.YEAR.DAY
	string constStreamDir = "/" + wid.networkCode() + "/" + wid.stationCode() +
	                        "/" + wid.channelCode() + ".D/";

	string constStreamFile = wid.networkCode() + "." + wid.stationCode() + "." +
	                         wid.locationCode() + "." + wid.channelCode() + ".D.";

	vector<string> streamFiles;
	for ( vector<string>::const_iterator it = _app->_archiveYears.begin();
	      it != _app->_archiveYears.end() && !_app->_exitRequested; ++it ) {
		string streamDir = *it + constStreamDir;
		string streamFile = constStreamFile + *it + ".";
		size_t streamFileLen = streamFile.length();
		fs::directory_iterator end_itr;
		try {
			for ( fs::directory_iterator itr(_app->_archive + streamDir);
			      itr != end_itr && !_app->_exitRequested; ++itr ) {
				std::string name = SC_FS_FILE_NAME(SC_FS_DE_PATH(itr));
				if ( name.length() == streamFileLen + 3 &&
				     name.substr(0, streamFileLen) == streamFile ) {
					streamFiles.push_back(streamDir + name);
				}
			}
		}
		catch ( ... ) {}
	}

	if ( _app->_exitRequested ) return;

	// sort files by name
	sort(streamFiles.begin(), streamFiles.end());
	if ( streamFiles.empty() ) {
		SEISCOMP_INFO("[%i] %s: found no data files", _id, _sid.c_str());
	}
	else {
		SEISCOMP_INFO("[%i] %s: found %lu data files", _id, _sid.c_str(),
		              (unsigned long) streamFiles.size());
		SEISCOMP_DEBUG("[%i] %s: first: %s", _id, _sid.c_str(),
		               streamFiles.front().c_str());
		SEISCOMP_DEBUG("[%i] %s: last : %s", _id, _sid.c_str(),
		               streamFiles.back().c_str());
	}

	DatabaseIterator db_seg_it;
	// check if extent exists
	if ( foundInDB ) {
		// query existing segments
		if ( ! dbConnect(_dbRead, "read") ) {
			SEISCOMP_ERROR("[%i] %s: could not query existing attribute "
			               "extents and data segments", _id, _sid.c_str());
			return;
		}

		// load existing data attribute extents and segments
		_dbRead->loadDataAttributeExtents(_extent);
		db_seg_it = dbSegments();
	}
	else if ( ! writeExtent(OP_ADD) )
		return;

	Core::Time now = Core::Time::GMT();
	DataModel::DataExtent scanExt("tmp_" + _sid);

	// iterate over all stream files
	string fileName, absFileName;
	Segments fileSegments;
	size_t segCount = 0;
	for ( vector<string>::const_iterator f_it = streamFiles.begin();
	      f_it != streamFiles.end() && !_app->_exitRequested &&
	      !scanExt.segmentOverflow(); ++f_it ) {
		fileName = SC_FS_PATH(*f_it).filename().string();
		absFileName = _app->_archive + *f_it;

		Core::Time fileStart = fileDate(fileName);
		if ( !fileStart.valid() ) {
			SEISCOMP_WARNING("[%i] %s: invalid file name, skipping: %s",
			                 _id, _sid.c_str(), f_it->c_str() );
			continue;
		}

		Core::Time mtime = fileMTime(absFileName);
		if ( !mtime ) mtime = now;
		if ( mtime > scanExt.updated() ) scanExt.setUpdated(mtime);

		// check if file was modified since last scan
		if ( false ) { /*!_app->_deepScan && _extent->lastScan() && mtime <= _extent->lastScan() ) {
			// file not modified since last scan, advance db iterator to segment
			// containing end time of file
			Core::Time fileEnd = fileStart + TimeSpan(86400, 0);
			for ( ; *seg_it && !_app->_exitRequested; ++seg_it ) {
				DataSegment s = DataSegment::Cast(*dbSegIt);
				if ( ! s )
					continue;
				if ( s->start() <= fileEnd ) {
					dbSegment = s;
				}
				else
					break;
			}*/
		}
		else {
			if ( ! readFileSegments(fileSegments, absFileName, mtime) )
				continue;

			// process file segments with the exception of the last element
			// which might be extented later on by records of the next data file
			for ( Segments::const_iterator it = fileSegments.begin(),
			      last = --fileSegments.end(); it != fileSegments.end(); ++it ) {
				if ( _app->_exitRequested ) return;

				// check for segment overflow
				if ( _app->_maxSegments >= 0 &&
				     !scanExt.segmentOverflow() &&
				     segCount >= (unsigned long)_app->_maxSegments ) {
					scanExt.setSegmentOverflow(true);
					SEISCOMP_WARNING("[%i] %s: segment overflow detected",
					                 _id, _sid.c_str());
				}

				_currentSegment = *it;

				if ( it == last ) break;

				++segCount;

				// update extent and attribute extent boundaries
				updateExtent(scanExt, _currentSegment.get());

				// remove database segments no longer found in file
				if ( !scanExt.segmentOverflow() &&
				     !findDBSegment(db_seg_it, _currentSegment.get() ) ) {
					addSegment(_currentSegment);
				}
			}
		}
	}

	// process last segment
	if ( _currentSegment && !scanExt.segmentOverflow() ) {
		// update extent and attribute extent boundaries
		updateExtent(scanExt, _currentSegment.get());

		// remove database segments no longer found in file
		if ( !scanExt.segmentOverflow() &&
		     !findDBSegment(db_seg_it, _currentSegment.get()) ) {
			addSegment(_currentSegment);
		}
	}

	// remove trailing database segments
	for ( ; !_app->_exitRequested && *db_seg_it; ++db_seg_it ) {
		DataSegmentPtr dbSeg = DataSegment::Cast(*db_seg_it);
		dbSeg->setParent(_extent);
		removeSegment(dbSeg);
	}
	flushSegmentBuffers();

	// sync attribute extents with database
	syncAttributeExtents(scanExt);

	// update extent
	if ( _currentSegment ) {
		_extent->setLastScan(now);
		bool extentModified = false;
		if ( _extent->start() != scanExt.start() || _extent->end() != scanExt.end() ||
		     _extent->updated() != scanExt.updated() ||
		     _extent->segmentOverflow() != _extent->segmentOverflow() ) {
			_extent->setStart(scanExt.start());
			_extent->setEnd(scanExt.end());
			_extent->setUpdated(scanExt.updated());
			_extent->setSegmentOverflow(scanExt.segmentOverflow());
			extentModified = true;
		}

		if ( writeExtent(OP_UPDATE) ) {
			SEISCOMP_INFO("[%i] %s: extent %s: %s ~ %s", _id, _sid.c_str(),
			              string(extentModified?"modified":"unchanged").c_str(),
			              _extent->start().iso().c_str(),
			              _extent->end().iso().c_str());
		}
	}
	else {
		// no segments found, remove entire extent and all attribute extents
		if ( writeExtent(OP_REMOVE) ) {
			SEISCOMP_INFO("[%i] %s: extent removed", _id, _sid.c_str());
		}
	}

	db_seg_it.close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Worker::dbConnect(DatabaseReaderPtr &db, const char *info) {
	while ( ! _app->_exitRequested && ( !db || !db->driver()->isConnected() ) ) {
		SEISCOMP_DEBUG("[%i] initializing database %s connection", _id, info);
		IO::DatabaseInterfacePtr dbInterface =
		        IO::DatabaseInterface::Open(_app->databaseURI().c_str());
		if ( dbInterface ) {
			db = new DatabaseReader(dbInterface.get());
		}
		else {
			SEISCOMP_DEBUG("[%i] trying to reconnect in 5s", _id);
			for ( int i = 0; i < 5 && ! _app->_exitRequested; ++i )
				sleep(1);
		}
	}
	if ( db && db->driver()->isConnected() )
		return true;

	SEISCOMP_ERROR("[%i] could not initializing database %s connection", _id, info);
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define _T(name) _dbRead->driver()->convertColumnName(name)
inline
DatabaseIterator Worker::dbSegments() {
	std::ostringstream oss;
	oss << "SELECT DataSegment.* "
	       "FROM DataSegment, PublicObject AS PDataExtent "
	    << "WHERE PDataExtent." << _T("publicID") << "='"
	    <<     _dbRead->toString(_extent->publicID()) << "' AND "
	           "DataSegment._parent_oid=PDataExtent._oid "
	       "ORDER BY DataSegment." << _T("start") << " ASC, "
	                "DataSegment." << _T("start_ms") << " ASC";

	return _dbRead->getObjectIterator(oss.str(), DataSegment::TypeInfo());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Worker::readFileSegments(Segments &segments, const std::string &file,
                              const Core::Time &mtime) {
	segments.clear();
	RecordStream::File stream;
	const WaveformStreamID &wid = _extent->waveformID();

	if ( !stream.setSource(file) ) {
		SEISCOMP_WARNING("[%i] %s: could not open record file: %s",
		                 _id, _sid.c_str(), file.c_str());
	}
	stream.addStream(wid.networkCode(), wid.stationCode(), wid.locationCode(),
	                 wid.channelCode());

	DataSegmentPtr segment = _currentSegment;

	IO::RecordInput input(&stream, Array::DOUBLE, Record::DATA_ONLY);
	RecordPtr rec;
	IO::MSeedRecord *msRec;
	string quality;
	int records         = 0;
	int samples         = 0;
	int gaps            = 0;
	int overlaps        = 0;
	int outOfOrder      = 0;
	int rateChanges     = 0;
	int qualityChanges  = 0;
	double availability = 0;
	for ( IO::RecordIterator it = input.begin();
	      it != input.end() && !_app->_exitRequested; ++it ) {
		rec = *it;

		if ( ! rec ) {
			SEISCOMP_WARNING("[%i] %s: received invalid record while reading "
			                 "file: %s", _id, _sid.c_str(), file.c_str());
			continue;
		}

		if ( rec->streamID() != _sid ) {
			SEISCOMP_WARNING("[%i] %s: received record with invalid stream id "
			                 "while reading file: %s",
			                 _id, _sid.c_str(), file.c_str());
			continue;
		}

//		SEISCOMP_DEBUG("[%i] %s: received record: %i samples, %.1fHz, %s ~ %s",
//		               _id, _sid.c_str(), rec->sampleCount(),
//		               rec->samplingFrequency(), rec->startTime().iso().c_str(),
//		               rec->endTime().iso().c_str());

		// set time jitter to half of sample time
		double jitter = _app->_jitter / rec->samplingFrequency();

		// try cast to MSeedRecord to read data quality
		msRec = IO::MSeedRecord::Cast(rec.get());
		quality = msRec ? string(1, msRec->dataQuality()) : "";

		// check if record can be merged with current segment
		bool merge = false;
		if ( segment ) {
			// gap
			if ( (rec->startTime() - segment->end()).length() > jitter ) {
				++gaps;
				SEISCOMP_DEBUG("[%i] %s: detected gap: %s ~ %s", _id, _sid.c_str(),
				               segment->end().iso().c_str(),
				               rec->startTime().iso().c_str());
			}
			// overlap
			else if ( (segment->end() - rec->startTime()).length() > jitter ) {
				++overlaps;
				SEISCOMP_DEBUG("[%i] %s: detected overlap: %s ~ %s",
				               _id, _sid.c_str(), rec->startTime().iso().c_str(),
				               segment->end().iso().c_str());
			}
			else {
				merge = true;
			}

			// sampling rate change
			if ( segment->sampleRate() != rec->samplingFrequency() ) {
				++rateChanges;
				SEISCOMP_DEBUG("[%i] %s: detected change of sampling rate at "
				               "%s: %.1f -> %.1f", _id, _sid.c_str(),
				               rec->startTime().iso().c_str(),
				               segment->sampleRate(), rec->samplingFrequency());
				merge = false;
			}

			// quality change
			if ( segment->quality() != quality ) {
				++qualityChanges;
				SEISCOMP_DEBUG("[%i] %s: detected change of quality at %s "
				               "%s -> %s", _id, _sid.c_str(),
				               rec->startTime().iso().c_str(),
				               segment->quality().c_str(), quality.c_str());
				merge = false;
			}
		}

		if ( merge ) {
			// check if first record is merged with segment of previous file:
			// update time if this file's mtime is greater the segment mtime
			if ( records == 0 && mtime > segment->updated() )
				segment->setUpdated(mtime);
			segment->setEnd(rec->endTime());
		}
		else {
			bool ooo = false;
			if ( segment ) {
				segments.push_back(segment.get());
				if ( rec->startTime() < segment->start() ) {
					ooo = true;
					++outOfOrder;
				}
			}
			segment = new DataSegment();
			segment->setStart(rec->startTime());
			segment->setEnd(rec->endTime());
			segment->setUpdated(mtime);
			segment->setSampleRate(rec->samplingFrequency());
			segment->setQuality(quality);
			segment->setOutOfOrder(ooo);
			segment->setParent(_extent);
		}

		records += 1;
		samples += rec->sampleCount();
		availability += ((double)rec->sampleCount()) / rec->samplingFrequency();
	}
	stream.close();

	// save last segment
	if ( segment ) {
		segments.push_back(segment.get());
		SEISCOMP_DEBUG("[%i] %s: %s, \n"
		               "  segments             : %lu\n"
		               "  gaps                 : %i\n"
		               "  overlaps             : %i\n"
		               "  out of order segments: %i\n"
		               "  sampling rate changes: %i\n"
		               "  quality changes      : %i\n"
		               "  records              : %i\n"
		               "  samples              : %i\n"
		               "  availability         : %.2f%% (%.1fs)\n"
		               "  modification time    : %s", _id, _sid.c_str(),
		               file.c_str(), (unsigned long)segments.size(), gaps,
		               overlaps, outOfOrder, rateChanges, qualityChanges,
		               records, samples, availability/864.0, availability,
		               mtime.iso().c_str());

		// sort segment vector according start time if out of order data
		// was detected
		if ( outOfOrder > 0 ) {
			sort(segments.begin(), segments.end(), compareSegmentStart);
		}
	}
	else {
		SEISCOMP_WARNING("[%i] %s: found no data in file: %s ", _id,
		                 _sid.c_str(), file.c_str());
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Worker::findDBSegment(DataModel::DatabaseIterator &it,
                           const DataModel::DataSegment *segment) {
	if ( !segment || !*it ) return false;

	for ( ; !_app->_exitRequested && *it; ++it ) {
		DataSegmentPtr dbSeg = DataSegment::Cast(*it);
		if ( dbSeg->start() > segment->start() )
			break;
		else if ( equalsNoUpdated(dbSeg.get(), segment) ) {
			++it;
			return true;
		}

		dbSeg->setParent(_extent);
		removeSegment(dbSeg);
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Worker::removeSegment(DataSegmentPtr segment){
	_segmentsRemove.push_back(segment);
	if ( _segmentsRemove.size() + _segmentsAdd.size() >=
	     (unsigned long) _app->_batchSize ) {
		flushSegmentBuffers();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Worker::addSegment(DataSegmentPtr segment) {
	_segmentsAdd.push_back(segment);
	if ( _segmentsRemove.size() + _segmentsAdd.size() >=
	     (unsigned long) _app->_batchSize ) {
		flushSegmentBuffers();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Worker::flushSegmentBuffers() {
	if ( _segmentsRemove.empty() && _segmentsAdd.empty() ) return;

	if ( dbConnect(_dbWrite, "write") ) {
		_dbWrite->driver()->start();

		for ( Segments::iterator it = _segmentsRemove.begin();
		      it != _segmentsRemove.end(); ) {
			if ( _dbWrite->remove(it->get()) )
				it = _segmentsRemove.erase(it);
			else
				++it;
		}
		for ( Segments::iterator it = _segmentsAdd.begin();
		      it != _segmentsAdd.end(); ) {
			if ( _dbWrite->write(it->get()) )
				it = _segmentsAdd.erase(it);
			else
				++it;
		}
		_dbWrite->driver()->commit();
	}

	if ( !_segmentsRemove.empty() ) {
		SEISCOMP_ERROR("[%i] %s: failed to add %lu segments",
		               _id, _sid.c_str(), (long unsigned) _segmentsRemove.size());
		_segmentsRemove.clear();
	}

	if ( !_segmentsAdd.empty() ) {
		SEISCOMP_ERROR("[%i] %s: failed to add %lu segments",
		               _id, _sid.c_str(), (long unsigned) _segmentsAdd.size());
		_segmentsAdd.clear();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Worker::writeExtent(Operation op) {
	if ( op == OP_UNDEFINED || ! dbConnect(_dbWrite, "write") )
		return false;

	_dbWrite->driver()->start();
	if ( ( op == OP_ADD    && ! _dbWrite->write(_extent)  ) ||
	     ( op == OP_UPDATE && ! _dbWrite->update(_extent) ) ||
	     ( op == OP_REMOVE && ! _dbWrite->remove(_extent) ) ) {
		_dbWrite->driver()->rollback();
		SEISCOMP_ERROR("[%i] %s: could not %s extent: %s", _id, _sid.c_str(),
		               op.toString(), _extent->publicID().c_str());
		return false;
	}

	_dbWrite->driver()->commit();
	SEISCOMP_DEBUG("[%i] %s: %s extent: %s", _id, _sid.c_str(),
	               string(op == OP_ADD   ?"added":
	                      op == OP_UPDATE?"updated":"removed").c_str(),
	               _extent->publicID().c_str());
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Worker::syncAttributeExtents(const DataModel::DataExtent &tmpExt) {
	if ( ! dbConnect(_dbWrite, "write") )
		return false;

	_dbWrite->driver()->start();
	// remove attribute extents no longer existing, update those who have changed
	for ( size_t i = 0; i < _extent->dataAttributeExtentCount(); ) {
		DataAttributeExtent *attExt = _extent->dataAttributeExtent(i);
		DataAttributeExtent *tmpAttExt = tmpExt.dataAttributeExtent(attExt->index());
		if ( tmpAttExt == NULL ) {
			_dbWrite->remove(attExt);
			_extent->removeDataAttributeExtent(i);
			SEISCOMP_DEBUG("[%i] %s: removed attribute extent with index %f,%s",
			                _id, _sid.c_str(), attExt->sampleRate(),
			               attExt->quality().c_str());
			continue;
		}
		else if ( *attExt != *tmpAttExt ) {
			*attExt = *tmpAttExt;
			_dbWrite->update(attExt);
			SEISCOMP_DEBUG("[%i] %s: updated attribute extent with index %f,%s",
			                _id, _sid.c_str(), attExt->sampleRate(),
			               attExt->quality().c_str());
		}
		++i;
	}
	// add new attribute extents
	for ( size_t i = 0; i < tmpExt.dataAttributeExtentCount(); ++i ) {
		DataAttributeExtent *tmpAttExt = tmpExt.dataAttributeExtent(i);
		DataAttributeExtent *attExt = _extent->dataAttributeExtent(tmpAttExt->index());
		if ( attExt == NULL ) {
			attExt = new DataAttributeExtent(*tmpAttExt);
			_extent->add(attExt);
			_dbWrite->write(attExt);
			SEISCOMP_DEBUG("[%i] %s: added attribute extent with index %f,%s",
			                _id, _sid.c_str(), attExt->sampleRate(),
			               attExt->quality().c_str());
		}
	}
	_dbWrite->driver()->commit();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SCARDAC::SCARDAC(int argc, char **argv)
: Seiscomp::Client::Application(argc, argv) {
	setMessagingEnabled(true);
	setDatabaseEnabled(true, true);

	_archive = "@ROOTDIR@/var/lib/archive";
	_threads = 1;
	_batchSize = 100;
	_jitter = 0.5;
	_maxSegments = 1000000;
	_deepScan = false;
	_workQueue.resize(MAX_THREADS);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SCARDAC::~SCARDAC() {
	for ( WorkerList::iterator it = _worker.begin(); it != _worker.end(); ++it ) {
		delete *it;
		*it = NULL;
	}
	_worker.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SCARDAC::createCommandLineDescription() {
	commandline().addGroup("Collector");
	commandline().addOption("Collector", "archive,a",
	                        "Location of the waveform archive",
	                        &_archive);
	commandline().addOption("Collector", "threads",
	                        "Number of threads scanning the archive in parallel",
	                        &_threads);
	commandline().addOption("Collector", "batch-size",
	                        "Batch size of database transactions used when "
	                        "updating data availability segments, allowed "
	                        "range: [1,1000]",
	                        &_batchSize);
	commandline().addOption("Collector", "jitter",
	                        "Acceptable derivation of end time and start time "
	                        "of successive records in multiples of sample time",
	                        &_jitter);
	commandline().addOption("Collector", "deep-scan",
	                        "Process all data files independ of their file "
	                        "modification time");
	commandline().addOption("Collector", "generate-test-data",
	                        "For each stream in inventory generate test data. "
	                        "Format: days,gaps,gapseconds,overlaps,"
	                        "overlapseconds",
	                        &_testData);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SCARDAC::initConfiguration() {
	if ( !Client::Application::initConfiguration() ) return false;

	try {
		_archive = Environment::Instance()->absolutePath(
		               SCCoreApp->configGetString("archive"));
	}
	catch (...) {}

	try {
		_threads = SCCoreApp->configGetInt("threads");
	}
	catch (...) {}

	try {
		_batchSize = SCCoreApp->configGetInt("batchSize");
	}
	catch (...) {}

	try {
		_jitter = SCCoreApp->configGetDouble("jitter");
	}
	catch (...) {}

	try {
		_maxSegments = SCCoreApp->configGetInt("maxSegments");
	}
	catch (...) {}


	try {
		_deepScan = SCCoreApp->commandline().hasOption("deep-scan") ||
		            SCCoreApp->configGetBool("deepScan");
	}
	catch (...) {}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SCARDAC::validateParameters() {
	SEISCOMP_DEBUG("validating parameters");
	if ( !Client::Application::validateParameters() ) return false;

	// database connection configured, no need to fetch parameters
	if ( !_db.empty() ) {
		setMessagingEnabled(false);
		setDatabaseEnabled(true, false);
	}

	// archive location
	if ( !_archive.empty() && _archive[_archive.size()-1] != '/' )
		_archive += '/';

	if ( _testData.empty() && !Util::pathExists(_archive)) {
		SEISCOMP_ERROR("archive directory does not exist: %s", _archive.c_str());
		return false;
	}

	// thread count
	if ( _threads < 1 || _threads > MAX_THREADS ) {
		SEISCOMP_ERROR("invalid number of threads, allowed range: [1,%i]",
		               MAX_THREADS);
		return false;
	}

	// batch size
	if ( _batchSize < 1 || _batchSize > MAX_BATCHSIZE ) {
		SEISCOMP_ERROR("invalid batch size, allowed range: [1,%i]",
		               MAX_BATCHSIZE);
		return false;
	}

	// jitter samples
	if ( _jitter < 0 ) {
		SEISCOMP_ERROR("invalid jitter value, minimum value: 0");
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SCARDAC::run() {
	if ( !_testData.empty() )
		return generateTestData();

	SEISCOMP_INFO("Configuration:\n"
	              "  archive     : %s\n"
	              "  threads     : %i\n"
	              "  batch size  : %i\n"
	              "  jitter      : %f\n"
	              "  max segments: %i\n"
	              "  deep scan   : %s", _archive.c_str(), _threads, _batchSize,
	              _jitter, _maxSegments, Core::toString(_deepScan).c_str());
	// disable public object cache
	PublicObject::SetRegistrationEnabled(false);
	Notifier::Disable();

	// query all extents stored in database so far and add them to extent map
	int count = query()->loadDataExtents(&_dataAvailability);
	SEISCOMP_INFO("loaded %i extents (streams) from database", count);

	// build vector of archive years heavily used later on
	fs::directory_iterator end_itr;
	try {
		for ( fs::directory_iterator itr(_archive); itr != end_itr; ++itr ) {
			if ( _exitRequested ) return true;

			fs::path dir = SC_FS_DE_PATH(itr);
			if ( !fs::is_directory(dir) ) continue;
			std::string name = SC_FS_FILE_NAME(dir);
			int year;
			if ( Core::fromString(year, name) && name.length() == 4 )
				_archiveYears.push_back(name);
			else {
				SEISCOMP_WARNING("invalid archive directory: %s/%s",
				                 _archive.c_str(), name.c_str());
				continue;
			}
		}
	}
	catch ( ... ) {
		SEISCOMP_ERROR("could not read archive years");
		return false;
	}

	// sort years
	sort(_archiveYears.begin(), _archiveYears.end());

	// add existing entents to extent map
	for ( size_t i = 0; i < _dataAvailability.dataExtentCount(); ++i) {
		DataExtent *extent = _dataAvailability.dataExtent(i);
		_extentMap[streamID(extent->waveformID())] = extent;
	}

	// stop here if archive is empty and no extents have been found in database
	if ( _archiveYears.empty() && _extentMap.empty() ) {
		SEISCOMP_INFO("archive is empty and no extents found in database");
		return true;
	}

	// create N worker threads
	SEISCOMP_INFO("creating %i worker threads", _threads);
	for ( int i = 0; i < _threads; ++i ) {
		_worker.push_back(new boost::thread(
		                  boost::bind(&SCARDAC::processExtents, this, i+1)));
	}

	// add extents to work queue, push may block if queue size is exceeded
	for ( ExtentMap::const_iterator it = _extentMap.begin();
	      it != _extentMap.end(); ++it ) {
		_workQueue.push(WorkQueueItem(it->second, true));
	}

	// search for new streams and create new extents
	size_t oldSize = _extentMap.size();
	SEISCOMP_INFO("scanning archive '%s' for new streams", _archive.c_str());
	for ( vector<string>::const_iterator it = _archiveYears.begin();
	      it != _archiveYears.end() && !_exitRequested; ++it ) {
		checkDirectory(1, _archive + *it + "/");
	}
	SEISCOMP_INFO("found %lu new streams in archive",
	              (unsigned long) _extentMap.size() - oldSize);

	// a NULL object is used to signal end of queue
	_workQueue.push(WorkQueueItem());
	SEISCOMP_INFO("last stream pushed, waiting for worker to terminate");

	// wait for all workers to terminate
	for ( WorkerList::const_iterator it = _worker.begin();
	      it != _worker.end(); ++it ) {
		(*it)->join();
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SCARDAC::checkDirectory(int level, const std::string &dir) {
	// Year/NET/STA/CHA.D/NET.STA.LOC.CHA.D.YEAR.DAY
	fs::directory_iterator end_itr;
	try {
		for ( fs::directory_iterator itr(dir);
		      itr != end_itr && !_exitRequested; ++itr ) {

			std::string name = SC_FS_FILE_NAME(SC_FS_DE_PATH(itr));
			if ( level < 3 ) {
				checkDirectory(level+1, dir + name + "/");
			}
			else {
				//SEISCOMP_DEBUG(msg[3], name.c_str());
				checkFiles(dir + name + "/");
			}
		}
	}
	catch ( ... ) {}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SCARDAC::checkFiles(const std::string &dir) {
	fs::directory_iterator end_itr;
	try {
		for ( fs::directory_iterator itr(dir); itr != end_itr; ++itr ) {
			if ( _exitRequested ) return;

			fs::path path = SC_FS_DE_PATH(itr);
			if ( !SC_FS_IS_REGULAR_FILE(path) )
				continue;

			std::string name = SC_FS_FILE_NAME(path);
			string streamID = fileStreamID(name);
			if ( streamID.empty() )
				continue;

			if ( _extentMap.find(streamID) == _extentMap.end() ) {
				WaveformStreamID wfid;
				if ( !wfID(wfid, streamID) ) {
					SEISCOMP_WARNING("          invalid file name: %s/%s",
					                 dir.c_str(), name.c_str());
					continue;
				}
				DataExtent *extent = DataExtent::Create();
				extent->setWaveformID(wfid);
				_dataAvailability.add(extent);
				_extentMap[streamID] = extent;
				_workQueue.push(WorkQueueItem(extent, false));
				SEISCOMP_INFO("           new extent for stream %s",
				              streamID.c_str());
			}
		}
	}
	catch ( ... ) {}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SCARDAC::processExtents(int threadID) {
	Worker worker(this, threadID);

	WorkQueueItem item;
	while ( !_exitRequested ) {
		try {
			item = _workQueue.pop();
		}
		catch ( Client::QueueClosedException & ) {
			SEISCOMP_DEBUG("[%i] work queue closed", threadID);
			return;
		}

		if ( item.extent == NULL ) {
			SEISCOMP_INFO("[%i] read last extent, closing queue", threadID);
			_workQueue.close();
			return;
		}

		worker.processExtent(item.extent, item.foundInDB);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SCARDAC::generateTestData() {
	if ( !query()->driver()->isConnected() ) {
		SEISCOMP_ERROR("database connection not available");
		return false;
	}

	setLoadInventoryEnabled(true);
	if ( !reloadInventory() ) {
		SEISCOMP_ERROR("could not load inventory");
		return false;
	}

	DatabaseObjectWriter dbWriter(*query(), true, _batchSize);

	vector<string> toks;
	Core::split(toks, _testData.c_str(), ",");

	int gaps, overlaps, segments;
	double days, gaplen, overlaplen;
	if ( toks.size() != 5 ||
	     !Core::fromString(days, toks[0]) ||
	     !Core::fromString(gaps, toks[1]) ||
	     !Core::fromString(gaplen, toks[2]) ||
	     !Core::fromString(overlaps, toks[3]) ||
	     !Core::fromString(overlaplen, toks[4]) ||
	     gaps < 0 || overlaps < 0 || (gaps > 0 && gaplen <= 0) ||
	     (overlaps > 0 && overlaplen <= 0 ) ) {
		SEISCOMP_ERROR("invalid format in parameter 'generate-test-data'");
		return false;
	}

	Core::Time end = Core::Time::GMT();
	Core::Time start = end - Core::TimeSpan(days * 86400.0, 0);
	segments = gaps + overlaps + 1;
	Core::TimeSpan segStep(days * 86400.0 / segments);
	Core::TimeSpan gapLen(gaplen);
	Core::TimeSpan overlapLen(overlaplen);

	Inventory *inv = Client::Inventory::Instance()->inventory();
	for ( size_t iNet = 0; iNet < inv->networkCount(); ++iNet ) {
		Network *net = inv->network(iNet);
		for ( size_t iSta = 0; iSta < net->stationCount(); ++iSta ) {
			Station *sta = net->station(iSta);
			for ( size_t iLoc = 0; iLoc < sta->sensorLocationCount(); ++iLoc ) {
				SensorLocation *loc = sta->sensorLocation(iLoc);
				for ( size_t iCha = 0; iCha < loc->streamCount(); ++iCha ) {
					Stream *cha = loc->stream(iCha);
					WaveformStreamID wid(net->code(), sta->code(),
					                     loc->code(), cha->code(), "");
					double sr = cha->sampleRateNumerator() /
					            cha->sampleRateDenominator();
					DataExtentPtr ext = DataExtent::Create();
					ext->setParent(&_dataAvailability);
					ext->setWaveformID(wid);
					ext->setStart(start);
					ext->setEnd(end);
					ext->setUpdated(end);
					ext->setLastScan(end);

					DataAttributeExtent *attExt = new DataAttributeExtent();
					attExt->setStart(start);
					attExt->setEnd(end);
					attExt->setUpdated(end);
					attExt->setQuality("M");
					attExt->setSampleRate(sr);
					attExt->setSegmentCount(gaps + overlaps);
					ext->add(attExt);

					Core::Time t = start;
					DataSegment *seg;
					for ( int i = 0; i < segments; ++i ) {
						seg = new DataSegment();
						if ( i <= gaps ) {
							seg->setStart(t);
							t += segStep;
							seg->setEnd(t - gapLen);
						}
						else {
							seg->setStart(t - overlapLen);
							t += segStep;
							seg->setEnd(t);
						}
						seg->setQuality("M");
						seg->setSampleRate(sr);
						seg->setOutOfOrder(false);
						seg->setUpdated(end);
						ext->add(seg);
					}

					if ( dbWriter(ext.get()) ) {
						SEISCOMP_INFO("wrote extent for stream: %s.%s.%s.%s",
						               net->code().c_str(),
						               sta->code().c_str(),
						               loc->code().c_str(),
						               cha->code().c_str());
					}
					else {
						SEISCOMP_WARNING("could not write extent for stream: "
						                 "%s.%s.%s.%s",
						                 net->code().c_str(),
						                 sta->code().c_str(),
						                 loc->code().c_str(),
						                 cha->code().c_str());
					}
				}
			}
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // ns Applications
} // ns Seiscomp
