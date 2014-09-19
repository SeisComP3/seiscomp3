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


#define SEISCOMP_COMPONENT EventList

#include <QMutexLocker>
#include <streambuf>

#include <seiscomp3/gui/datamodel/publicobjectevaluator.h>
#include <seiscomp3/datamodel/publicobject.h>
//#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/io/archive/binarchive.h>
#include <seiscomp3/logging/log.h>


namespace {

class ByteArrayBuf : public std::streambuf {
	public:
		ByteArrayBuf(QByteArray &array) : _array(array) {}

	protected:
		int_type overflow (int_type c) {
			_array.append((char)c);
			return c;
		}

		std::streamsize xsputn(const char* s, std::streamsize n) {
			_array += QByteArray(s, n);
			return n;
		}

	private:
		QByteArray &_array;
};

}


namespace Seiscomp {
namespace Gui {


PublicObjectEvaluator *PublicObjectEvaluator::_instance = NULL;


PublicObjectEvaluator::PublicObjectEvaluator() : _reader(NULL) {
}


PublicObjectEvaluator &PublicObjectEvaluator::Instance() {
	if ( _instance == NULL ) _instance = new PublicObjectEvaluator;
	return *_instance;
}


bool PublicObjectEvaluator::setDatabaseURI(const char *uri) {
	_databaseURI = uri;

	// Protect the list
	QMutexLocker locker(&_mutexJobList);

	if ( !_jobs.isEmpty() && !isRunning() ) {
		if ( !connect() ) return false;
		start();
	}

	return true;
}


bool PublicObjectEvaluator::connect() {
	_reader.close();
	_reader.setDriver(NULL);

	IO::DatabaseInterfacePtr db = IO::DatabaseInterface::Open(_databaseURI.c_str());
	if ( db == NULL ) {
		SEISCOMP_WARNING("[obj eval] setting database %s failed", _databaseURI.c_str());
		return false;
	}

	_reader.setDriver(db.get());
	_reader.setPublicObjectCacheLookupEnabled(false);

	SEISCOMP_DEBUG("[obj eval] set database %s", _databaseURI.c_str());
	return true;
}


bool PublicObjectEvaluator::append(void *owner, const QString &publicID,
                                   const Core::RTTI& classType, const QString &script) {
	// Protect the list
	QMutexLocker locker(&_mutexJobList);

	JobIDMap::iterator it = _jobIDLookup.find(publicID);
	// publicID not yet registered
	if ( it == _jobIDLookup.end() ) {
		Job job(publicID, classType);
		job.scripts[script] = owner;
		_jobIDLookup[publicID] = _jobs.insert(_jobs.end(), job);
	}
	else {
		Scripts::iterator jit = it.value()->scripts.find(script);
		if ( jit != it.value()->scripts.end() ) {
			if ( jit.value() != owner ) jit.value() = NULL;
		}
		else
			it.value()->scripts.insert(script, owner);
	}

	if ( !_jobs.isEmpty() && !isRunning() ) {
		if ( !connect() ) return false;
		start();
	}

	return true;
}


bool PublicObjectEvaluator::append(void *owner, const QString &publicID,
                                   const Core::RTTI& classType, const QStringList &scripts) {
	// Protect the list
	QMutexLocker locker(&_mutexJobList);

	JobIDMap::iterator it = _jobIDLookup.find(publicID);
	// publicID not yet registered
	if ( it == _jobIDLookup.end() ) {
		Job job(publicID, classType);
		foreach ( const QString &script, scripts )
			job.scripts[script] = owner;
		_jobIDLookup[publicID] = _jobs.insert(_jobs.end(), job);
	}
	else {
		foreach ( const QString &script, scripts ) {
			Scripts::iterator jit = it.value()->scripts.find(script);
			if ( jit != it.value()->scripts.end() ) {
				if ( jit.value() != owner ) jit.value() = NULL;
			}
			else
				it.value()->scripts.insert(script, owner);
		}
	}

	if ( !_jobs.isEmpty() && !isRunning() ) {
		if ( !connect() ) return false;
		start();
	}

	return true;
}


bool PublicObjectEvaluator::prepend(void *owner, const QString &publicID,
                                    const Core::RTTI& classType, const QString &script) {
	// Protect the list
	QMutexLocker locker(&_mutexJobList);

	JobIDMap::iterator it = _jobIDLookup.find(publicID);
	// publicID not yet registered
	if ( it == _jobIDLookup.end() ) {
		Job job(publicID, classType);
		job.scripts[script] = owner;
		_jobIDLookup[publicID] = _jobs.insert(_jobs.begin(), job);
	}
	else {
		Scripts::iterator jit = it.value()->scripts.find(script);
		if ( jit != it.value()->scripts.end() ) {
			if ( jit.value() != owner ) jit.value() = NULL;
		}
		else
			it.value()->scripts.insert(script, owner);

		// Not the first item, make it the first
		if ( it.value() != _jobs.begin() ) {
			Job job = *it.value();
			_jobs.erase(it.value());
			it.value() = _jobs.insert(_jobs.begin(), job);
		}
	}

	if ( !_jobs.isEmpty() && !isRunning() ) {
		if ( !connect() ) return false;
		start();
	}

	return true;
}


bool PublicObjectEvaluator::prepend(void *owner, const QString &publicID,
                                    const Core::RTTI& classType, const QStringList &scripts) {
	// Protect the list
	QMutexLocker locker(&_mutexJobList);

	JobIDMap::iterator it = _jobIDLookup.find(publicID);
	// publicID not yet registered
	if ( it == _jobIDLookup.end() ) {
		Job job(publicID, classType);
		foreach ( const QString &script, scripts )
			job.scripts[script] = owner;
		_jobIDLookup[publicID] = _jobs.insert(_jobs.begin(), job);
	}
	else {
		foreach ( const QString &script, scripts ) {
			Scripts::iterator jit = it.value()->scripts.find(script);
			if ( jit != it.value()->scripts.end() ) {
				if ( jit.value() != owner ) jit.value() = NULL;
			}
			else
				it.value()->scripts.insert(script, owner);
		}

		// Not the first item, make it the first
		if ( it.value() != _jobs.begin() ) {
			Job job = *it.value();
			_jobs.erase(it.value());
			it.value() = _jobs.insert(_jobs.begin(), job);
		}
	}

	if ( !_jobs.isEmpty() && !isRunning() ) {
		if ( !connect() ) return false;
		start();
	}

	return true;
}


bool PublicObjectEvaluator::moveToFront(const QString &publicID) {
	// Protect the list
	QMutexLocker locker(&_mutexJobList);

	JobIDMap::iterator it = _jobIDLookup.find(publicID);
	// publicID not yet registered
	if ( it == _jobIDLookup.end() ) return false;

	// Not the first item, make it the first
	if ( it.value() != _jobs.begin() ) {
		Job job = *it.value();
		_jobs.erase(it.value());
		it.value() = _jobs.insert(_jobs.begin(), job);
	}

	return true;
}


bool PublicObjectEvaluator::erase(void *owner, const QString &publicID, const QString &script) {
	// Protect the list
	QMutexLocker locker(&_mutexJobList);

	JobIDMap::iterator it = _jobIDLookup.find(publicID);
	if ( it == _jobIDLookup.end() ) return false;

	Scripts::iterator jit = it.value()->scripts.find(script);
	if ( jit == it.value()->scripts.end() ) return false;

	// Owner set and script has different owner?
	if ( owner != NULL && jit.value() != owner ) return false;

	it.value()->scripts.erase(jit);
	if ( it.value()->scripts.isEmpty() ) {
		_jobs.erase(it.value());
		_jobIDLookup.erase(it);
	}

	return true;
}


bool PublicObjectEvaluator::erase(void *owner, const QString &publicID) {
	// Protect the list
	QMutexLocker locker(&_mutexJobList);

	JobIDMap::iterator it = _jobIDLookup.find(publicID);
	if ( it == _jobIDLookup.end() ) return false;

	if ( owner != NULL ) {
		Scripts::iterator jit;
		for ( jit = it.value()->scripts.begin(); jit != it.value()->scripts.end(); ) {
			if ( jit.value() != owner )
				++jit;
			else
				jit = it.value()->scripts.erase(jit);
		}

		if ( it.value()->scripts.isEmpty() ) {
			_jobs.erase(it.value());
			_jobIDLookup.erase(it);
		}
	}
	else {
		_jobs.erase(it.value());
		_jobIDLookup.erase(it);
	}

	return true;
}


int PublicObjectEvaluator::pendingJobs() const {
	// Protect the list
	QMutexLocker locker(&_mutexJobList);

	return _jobs.size();
}


void PublicObjectEvaluator::clear(void *owner) {
	// Protect the list
	_mutexJobList.lock();

	if ( owner == NULL ) {
		_jobs = JobList();
		_jobIDLookup = JobIDMap();
		_mutexJobList.unlock();
		wait();
		return;
	}

	JobIDMap::iterator it;
	for ( it = _jobIDLookup.begin(); it != _jobIDLookup.end(); ) {
		Scripts::iterator jit;
		for ( jit = it.value()->scripts.begin(); jit != it.value()->scripts.end(); ) {
			if ( jit.value() != owner )
				++jit;
			else
				jit = it.value()->scripts.erase(jit);
		}

		if ( it.value()->scripts.isEmpty() ) {
			_jobs.erase(it.value());
			it = _jobIDLookup.erase(it);
		}
		else
			++it;
	}

	_mutexJobList.unlock();
}


bool PublicObjectEvaluator::eval(DataModel::PublicObject *po, const QStringList &scripts) {
	QByteArray data;
	{
		ByteArrayBuf buf(data);
		IO::BinaryArchive ar;
		ar.create(&buf);
		ar << po;
		ar.close();
	}

	QStringList::const_iterator sit;
	for ( sit = scripts.begin(); sit != scripts.end(); ++sit ) {
		QProcess proc;
		proc.start(*sit);
		if ( !proc.waitForStarted() ) {
			SEISCOMP_ERROR("%s: failed to start", qPrintable(*sit));
			emit resultError(po->publicID().c_str(), po->typeInfo().className(), *sit, proc.error());
			continue;
		}

		proc.write(data);
		//qint64 written = proc.write(data);
		//SEISCOMP_DEBUG("... sent %d bytes to process", (int)written);
		proc.closeWriteChannel();
		proc.setReadChannel(QProcess::StandardOutput);

		if ( !proc.waitForFinished() ) {
			SEISCOMP_ERROR("%s: problem with finishing", qPrintable(*sit));
			emit resultError(po->publicID().c_str(), po->typeInfo().className(), *sit, proc.error());
			continue;
		}

		if ( proc.exitCode() != 0 ) {
			SEISCOMP_ERROR("%s: exit code: %d", qPrintable(*sit), proc.exitCode());
			emit resultError(po->publicID().c_str(), po->typeInfo().className(), *sit, -proc.exitCode());
			continue;
		}

		QByteArray result = proc.readAll();
		QString text = QString(result).trimmed();
		emit resultAvailable(po->publicID().c_str(), po->typeInfo().className(), *sit, text);
	}

	return true;
}


void PublicObjectEvaluator::run() {
	// Protect the list
	_mutexJobList.lock();

	SEISCOMP_INFO("[obj eval] started");

	// Disable object registration only in this thread
	DataModel::PublicObject::SetRegistrationEnabled(false);

	while ( !_jobs.empty() ) {
		Job job = _jobs.front();
		_jobs.pop_front();

		JobIDMap::iterator it = _jobIDLookup.find(job.publicID);
		if ( it != _jobIDLookup.end() )
			_jobIDLookup.erase(it);

		// Unlock the mutex and call the scripts
		_mutexJobList.unlock();

		// Load the entire object including childs from database
		DataModel::PublicObjectPtr o = _reader.loadObject(job.classType, job.publicID.toStdString());
		DataModel::PublicObject *po = o.get();
		if ( po == NULL ) {
			SEISCOMP_ERROR("[obj eval] %s not found in database",
			               qPrintable(job.publicID));
			_mutexJobList.lock();
			continue;
		}

		QByteArray data;
		{
			ByteArrayBuf buf(data);
			IO::BinaryArchive ar;
			ar.create(&buf);
			ar << po;
			ar.close();
		}

		//SEISCOMP_DEBUG("... generated %d bytes of XML", (int)data.size());

		Scripts::iterator sit;
		for ( sit = job.scripts.begin(); sit != job.scripts.end(); ++sit ) {
			QProcess proc;
			proc.start(sit.key());
			if ( !proc.waitForStarted() ) {
				SEISCOMP_ERROR("%s: failed to start", qPrintable(sit.key()));
				emit resultError(job.publicID, job.classType.className(), sit.key(), proc.error());
				continue;
			}

			proc.write(data);
			//qint64 written = proc.write(data);
			//SEISCOMP_DEBUG("... sent %d bytes to process", (int)written);
			proc.closeWriteChannel();
			proc.setReadChannel(QProcess::StandardOutput);

			if ( !proc.waitForFinished() ) {
				SEISCOMP_ERROR("%s: problem with finishing", qPrintable(sit.key()));
				emit resultError(job.publicID, job.classType.className(), sit.key(), proc.error());
				continue;
			}

			if ( proc.exitCode() != 0 ) {
				QByteArray errMsg = proc.readAllStandardError();
				SEISCOMP_ERROR("%s (exit code %d): %s", qPrintable(sit.key()),
				               proc.exitCode(), qPrintable(QString(errMsg).trimmed()));
				emit resultError(job.publicID, job.classType.className(), sit.key(), -proc.exitCode());
				continue;
			}

			QByteArray result = proc.readAll();
			QString text = QString(result).trimmed();
			emit resultAvailable(job.publicID, job.classType.className(), sit.key(), text);
		}

		// Lock it again
		_mutexJobList.lock();
	}

	SEISCOMP_INFO("[obj eval] finished");

	// Unlock finally
	_mutexJobList.unlock();
}


QString PublicObjectEvaluator::errorMsg(int error) const {
	QString msg;

	if ( error < 0 )
		return QString("Invalid exit code: %1: 0 expected").arg(-error);

	switch ( error ) {
		case QProcess::FailedToStart:
			msg = "The process failed to start. Either the invoked\n"
			      "program is missing, or you may have insufficient\n"
			      "permissions to invoke the program.";
			break;
		case QProcess::Crashed:
			msg = "The process crashed some time after\n"
			      "starting successfully.";
			break;
		case QProcess::Timedout:
			msg = "The last waitFor...() function timed\n"
			      "out. Process was killed.";
			break;
		case QProcess::WriteError:
			msg = "An error occurred when attempting to write\n"
			      "to the process. For example, the process may\n"
			      "not be running, or it may have closed its\n"
			      "input channel.";
			break;
		case QProcess::ReadError:
			msg = "An error occurred when attempting to read from the\n"
			      "process. For example, the process may not be running.";
			break;
		default:
			msg = "An unknown error occurred.";
			break;
	}

	return msg;
}


}
}
