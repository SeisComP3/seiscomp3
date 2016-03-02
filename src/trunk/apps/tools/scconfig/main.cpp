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


#define SEISCOMP_COMPONENT Configurator

#include <iostream>
#include <seiscomp3/utils/files.h>

#include <seiscomp3/system/environment.h>
#include <seiscomp3/system/schema.h>
#include <seiscomp3/system/model.h>
#include <seiscomp3/logging/log.h>

#include "gui.h"

#include <QtGui>
#if QT_VERSION >= 0x040400
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QCryptographicHash>
#endif


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Logging;


namespace {


const char *module_desc = "/etc/descriptions";

string filebase;
string outdir;


}


#if QT_VERSION >= 0x040400
class RunGuard {
	public:
		RunGuard(const QString &key)
		: key(key)
		, memLockKey(generateKeyHash(key, "_memLockKey"))
		, sharedmemKey(generateKeyHash(key, "_sharedmemKey"))
		, sharedMem(sharedmemKey)
		, memLock(memLockKey, 1)
		{
			memLock.acquire();
			{
				QSharedMemory fix(sharedmemKey);  // Fix for *nix: http://habrahabr.ru/post/173281/
				fix.attach();
			}
			memLock.release();
		}

		~RunGuard() {
			release();
		}

		bool isAnotherRunning() {
			if ( sharedMem.isAttached() )
				return false;

			memLock.acquire();
			const bool isRunning = sharedMem.attach();
			if ( isRunning )
				sharedMem.detach();
			memLock.release();
			return isRunning;
		}

		bool tryToRun() {
			// Extra check
			if ( isAnotherRunning() )
				return false;

			memLock.acquire();
			const bool result = sharedMem.create( sizeof( quint64 ) );
			memLock.release();
			if ( !result )
			{
				release();
				return false;
			}

			return true;
		}

		void release() {
			memLock.acquire();
			if ( sharedMem.isAttached() )
				sharedMem.detach();
			memLock.release();
		}


	private:
		Q_DISABLE_COPY(RunGuard)

		QString generateKeyHash(const QString& key, const QString& salt) {
			QByteArray data;
			data.append(key.toUtf8());
			data.append(salt.toUtf8());
			data = QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex();
			return data;
		}

		const QString    key;
		const QString    memLockKey;
		const QString    sharedmemKey;

		QSharedMemory    sharedMem;
		QSystemSemaphore memLock;
};
#endif


int main(int argc, char **argv) {
	QApplication app(argc, argv);

	filebase = Seiscomp::Environment::Instance()->installDir();

	#if QT_VERSION >= 0x040400
	RunGuard guard(QString("SeisComP3/apps/scconfig/%1").arg(filebase.c_str()));
	if ( !guard.tryToRun() ) {
		if ( QMessageBox::warning(NULL, "scconfig", app.tr("scconfig with SEISCOMP_ROOT '%1' is already running!\n"
		                                                   "Note that modifying the configuration from two instances can result in an inconsistent configuration!\n"
		                                                   "Do you want to continue?\n")
		                                            .arg(filebase.c_str()), QMessageBox::Yes, QMessageBox::No) != QMessageBox::Yes )
			return 0;
	}
	#endif

	if ( filebase.empty() )
		filebase = ".";
	else if ( *filebase.rbegin() == '/' )
		filebase.resize(filebase.size()-1);

	//outdir = filebase + ".out";
	outdir = filebase;

	// Activate error and warning logs
	enableConsoleLogging(_SCErrorChannel);
	enableConsoleLogging(_SCWarningChannel);

	for ( int i = 1; i < argc; ++i ) {
		if ( !strcmp("--debug", argv[i]) )
			Seiscomp::Logging::enableConsoleLogging(Seiscomp::Logging::getAll());
	}

	System::SchemaDefinitions defs;
	cerr << "Loading definitions from: " << filebase << module_desc << endl;
	if ( !defs.load((filebase + module_desc).c_str()) ) {
		cerr << "read error: not a directory" << endl;
		return 1;
	}

	if ( defs.moduleCount() == 0 ) {
		cerr << "read error: no module" << endl;
		return 2;
	}

	System::Model model;
	model.create(&defs);

	cerr << "Loading stations from: " << model.stationConfigDir(true) << endl;

	Configurator c(Seiscomp::Environment::CS_CONFIG_APP);
	c.resize(800,600);
	if ( !c.setModel(&model) )
		return 1;

	c.show();

	app.exec();

	return 0;
}
