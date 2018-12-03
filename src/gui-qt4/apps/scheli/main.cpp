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

#include "app.h"


using namespace Seiscomp;


namespace {


QString waveformIDToQString(const DataModel::WaveformStreamID &id) {
	return (id.networkCode() + "." + id.stationCode() + "." +
	        id.locationCode() + "." + id.channelCode()).c_str();
}


bool stringToWaveformID(DataModel::WaveformStreamID &id, const std::string &s) {
	std::vector<std::string> tokens;
	Core::split(tokens, s.c_str(), ".", false);
	if ( tokens.size() != 4 ) return false;

	id.setNetworkCode(tokens[0]);
	id.setStationCode(tokens[1]);
	id.setLocationCode(tokens[2]);
	id.setChannelCode(tokens[3]);

	return true;
}


}



HCApp::HCApp(int& argc, char** argv, int flags, Type type )
: Gui::Application(argc, argv, flags, type) {
	if ( type == Tty ) {
		QPalette pal = palette();
		pal.setColor(QPalette::Base, Qt::white);
		pal.setColor(QPalette::Text, Qt::black);
		setPalette(pal);
	}

	setLoadCitiesEnabled(false);
	setLoadRegionsEnabled(false);

	_gain = 1.0;
	_amplitudesRange = 0.0;
	_amplitudesMin =  -0.00001f;
	_amplitudesMax =  +0.00001f;
	_fixCurrentTimeToLastRecord = false;
	_numberOfRows = 48;
	_timeSpanPerRow = 1800;
	_antialiasing = false;
	_lineWidth = 1;
	_stationDescription = true;
	_xRes = 1024;
	_yRes = 768;
	_dpi = 300;
	_timeFormat = "%F";
	_snapshotTimeout = -1;
	_streamThread = NULL;
	_snapshotTimer = -1;
}


HCApp::~HCApp() {
	HeliStreamMap::iterator it;
	for ( it = _helis.begin(); it != _helis.end(); ++it )
		delete it.value().canvas;

	if ( _streamThread ) {
		_streamThread->stop(true);
		delete _streamThread;
	}
}


QString HCApp::findHeadline(const DataModel::WaveformStreamID &streamID,
                            const Core::Time &refTime) {
	QString headline;

	DataModel::Station *sta;
	sta = Client::Inventory::Instance()->getStation(streamID.networkCode(),
	                                                streamID.stationCode(),
	                                                refTime);

	if ( sta ) {
		QString lat = QString("%1 %2").arg(fabs(sta->latitude()), 0, 'f', 2).arg(QChar(sta->latitude() < 0 ? 'S' : 'N'));
		QString lon = QString("%1 %2").arg(fabs(sta->longitude()), 0, 'f', 2).arg(QChar(sta->longitude() < 0 ? 'W' : 'E'));

		if ( _stationDescription )
			headline = QString("%1  %2 %3")
			           .arg(sta->description().c_str())
			           .arg(lat).arg(lon);
		else
			headline = QString("%1 %2")
			           .arg(lat).arg(lon);
	}
	else
		headline = waveformIDToQString(streamID);

	return headline;
}


float HCApp::findGain(const DataModel::WaveformStreamID &streamID, const Core::Time &refTime) {
	if ( commandline().hasOption("gain") ) {
		SEISCOMP_DEBUG("Using supplied gain = %f", _gain);
		return _gain;
	}
	else {
		double gain;
		try {
			gain = Client::Inventory::Instance()->getGain(streamID.networkCode(),
			                                               streamID.stationCode(),
			                                               streamID.locationCode(),
			                                               streamID.channelCode(),
			                                               refTime);
		}
		catch ( ... ) {
			SEISCOMP_WARNING("WARNING: Unable to retrieve gain for %s.%s.%s.%s: "
			                 "using default gain",
			                 streamID.networkCode().c_str(),
			                 streamID.stationCode().c_str(),
			                 streamID.locationCode().c_str(),
			                 streamID.channelCode().c_str());
			return _gain;
		}

		SEISCOMP_DEBUG("Using gain from inventory = %f", gain);
		return gain;
	}
}


bool HCApp::initConfiguration() {
	if ( !Gui::Application::initConfiguration() )
		return false;

	try { _amplitudesMin = configGetDouble("heli.amplitudeRange.min"); }
	catch ( Config::Exception& e ) {}

	try { _amplitudesMax = configGetDouble("heli.amplitudeRange.max"); }
	catch ( Config::Exception& e ) {}

	try { _streamCodes = configGetStrings("heli.streams"); }
	catch ( Config::Exception& e ) {}

	try { _stationDescription = configGetBool("heli.stream.description"); }
	catch ( Config::Exception& e ) {}

	try { _filterString = configGetString("heli.filter"); }
	catch ( Config::Exception& e ) {}

	try { _timeFormat = configGetString("heli.timeFormat"); }
	catch ( Config::Exception& e ) {}

	try { _fixCurrentTimeToLastRecord = configGetBool("heli.recordTime"); }
	catch ( Config::Exception& e ) {}

	try {
		std::vector<std::string> colors = configGetStrings("heli.colors");
		_rowColors.clear();
		for ( size_t i = 0; i < colors.size(); ++i ) {
			QColor c;
			if ( !Gui::fromString(c, colors[i]) ) {
				std::cerr << "WARNING: '" << colors[i] << "': invalid color definition in 'colors': using defaults" << std::endl;
				_rowColors.clear();
				break;
			}
			_rowColors.append(c);
		}
	}
	catch ( ... ) {}

	try { _numberOfRows = configGetInt("heli.numberOfRows"); } catch ( ... ) {}
	try { _timeSpanPerRow = configGetInt("heli.rowTimeSpan"); } catch ( ... ) {}
	try { _antialiasing = configGetBool("heli.antialiasing"); } catch ( ... ) {}
	try { _lineWidth = configGetInt("heli.lineWidth"); } catch ( ... ) {}
	try { _xRes = configGetInt("heli.dump.xres"); } catch ( ... ) {}
	try { _yRes = configGetInt("heli.dump.yres"); } catch ( ... ) {}
	try { _dpi = configGetInt("heli.dump.dpi"); } catch ( ... ) {}
	try { _snapshotTimeout = configGetInt("heli.dump.interval"); } catch ( ... ) {}

	try {
		_imagePostProcessingScript = Seiscomp::Environment::Instance()->absolutePath(configGetString("scripts.postprocessing"));
	}
	catch ( ... ) {}

	return true;
}


void HCApp::createCommandLineDescription() {
	Gui::Application::createCommandLineDescription();

	commandline().addGroup("Mode");
	commandline().addOption("Mode", "end-time", "Set the end time of acquisition, default: 'gmt', format \"%F %T\"", (std::string*)NULL);
	commandline().addOption("Mode", "offline", "Do not connect to a messaging server and do not use the database");
	commandline().addOption("Mode", "no-messaging", "Do not connect to a messaging server but use the database");

	commandline().addGroup("Data");
	commandline().addOption("Data", "stream", "The record stream that should be displayed: stream=net.sta.loc.cha", &_streamID);
	commandline().addOption("Data", "filter", "The filter to apply", &_filterString);
	commandline().addOption("Data", "gain", "Gain applied to the data before plotting.", &_gain);
	commandline().addOption("Data", "amp-range-min", "Lower bound of amplitude range per row", &_amplitudesMin);
	commandline().addOption("Data", "amp-range-max", "Upper bound of amplitude range per row", &_amplitudesMax);
	commandline().addOption("Data", "amp-range", "Arround zero bound of amplitude range per row", &_amplitudesRange);
	commandline().addOption("Data", "record-time", "Does the last row always contain the last record received", &_fixCurrentTimeToLastRecord);

	commandline().addGroup("Output");
	commandline().addOption("Output", "desc", "Enables/disables the display of a station description", &_stationDescription);
	commandline().addOption("Output", "rows", "Configures the number of rows to display", &_numberOfRows);
	commandline().addOption("Output", "time-span", "Configures the time-span (in secs) per row", &_timeSpanPerRow);
	commandline().addOption("Output", "aa", "Sets antialiasing for rendering the traces", &_antialiasing);
	commandline().addOption("Output", "xres", "Output x resolution when generating images", &_xRes);
	commandline().addOption("Output", "yres", "Output y resolution when generating images", &_yRes);
	commandline().addOption("Output", "dpi", "Output dpi when generating postscript", &_dpi);
	commandline().addOption("Output", "output,o", "Output filename (placeholders: %N,%S,%L,%C)", &_outputFilename);
	commandline().addOption("Output", "interval", "Snapshot interval (<= 0 disabled timed snapshots)", &_snapshotTimeout);
}


bool HCApp::validateParameters() {
	if ( !Gui::Application::validateParameters() ) return false;

	if ( !_streamID.empty() ) {
		std::vector<std::string> tokens;

		Core::split(tokens, _streamID.c_str(), ".", false);
		if ( tokens.size() != 4 ) {
			std::cerr << "ERROR: Malformed streamcode found: format is: net.station.loc.channelcomponent" << std::endl;
			return false;
		}

		_streamCodes.push_back(_streamID);
	}

	if ( (type() == QApplication::Tty) && _outputFilename.empty() ) {
		std::cerr << "ERROR: Output filename empty" << std::endl;
		return false;
	}

	try {
		std::string dt = SCApp->commandline().option<std::string>("end-time");
		_endTime = Seiscomp::Core::Time::FromString(dt.c_str(), "%F %T");
		if ( !_endTime.valid() ) {
			std::cerr << "ERROR: passed endtime is not valid, expect format \"YYYY-mm-dd HH:MM:SS\"" << std::endl
			          << "       example: --end-time \"2010-01-01 12:00:00\"" << std::endl;
			return false;
		}

		std::cout << "Set defined endtime: " << _endTime.toString("%F %T") << std::endl;
	}
	catch ( ... ) {}

	if ( commandline().hasOption("amp-range") && !commandline().hasOption("amp-range-min") )
		_amplitudesMin = -1 * fabs(_amplitudesRange);

	if ( commandline().hasOption("amp-range") && !commandline().hasOption("amp-range-max") )
		_amplitudesMax = fabs(_amplitudesRange);

	if ( commandline().hasOption("no-messaging") )
		setMessagingEnabled(false);

	if ( commandline().hasOption("offline") ) {
		setMessagingEnabled(false);
		setDatabaseEnabled(false, false);
	}

	return true;
}


bool HCApp::handleInitializationError(Stage stage) {
	if ( type() == QApplication::Tty )
		return false;
	return Application::handleInitializationError(stage);
}


bool HCApp::init() {
	if ( !Gui::Application::init() ) return false;

	if ( _streamCodes.empty() ) {
		std::cerr << "ERROR: no streams given" << std::endl;
		return false;
	}

	return true;
}


bool HCApp::run() {
	if ( type() == QApplication::Tty ) {
		Core::Time endTime = _endTime.valid()?_endTime:Core::Time::GMT();

		for ( size_t i = 0; i < _streamCodes.size(); ++i ) {
			HeliCanvas *heli = new HeliCanvas();
			_helis[_streamCodes[i]] = heli;

			heli->setAntialiasingEnabled(_antialiasing);
			heli->setLineWidth(_lineWidth);
			heli->setAmplitudeRange(_amplitudesMin, _amplitudesMax);
			heli->setLayout(_numberOfRows, _timeSpanPerRow);

			if ( !_rowColors.empty() )
				heli->setRowColors(_rowColors);
			else {
				QVector<QColor> colors;
				colors.append(SCScheme.colors.records.foreground);
				colors.append(SCScheme.colors.records.alternateForeground);
				heli->setRowColors(colors);
			}

			if ( !_filterString.empty() )
				if ( !heli->setFilter(_filterString) ) {
					std::cerr << "Unable to set filter: " << _filterString << std::endl;
					return false;
				}

			DataModel::WaveformStreamID streamID;
			if ( !stringToWaveformID(streamID, _streamCodes[i]) ) {
				std::cerr << "ERROR: Malformed stream id: "
				          << _streamCodes[i] << std::endl;
				return false;
			}

			_helis[_streamCodes[i]].headline= findHeadline(streamID, endTime);

			heli->setScale(1.0 / findGain(streamID, endTime));

			if ( _snapshotTimeout > 0 ) {
				if ( !_streamThread ) {
					_streamThread = new Gui::RecordStreamThread(recordStreamURL());
					if ( !_streamThread->connect() ) {
						delete _streamThread;
						std::cerr << "ERROR: Unable to open recordstream " << recordStreamURL() << std::endl;
						return false;
					}

					connect(_streamThread, SIGNAL(receivedRecord(Seiscomp::Record*)),
					        this, SLOT(receivedRecord(Seiscomp::Record*)));

					connect(_streamThread, SIGNAL(finished()),
					        this, SLOT(acquisitionFinished()));
				}

				_streamThread->addStream(streamID.networkCode(), streamID.stationCode(), streamID.locationCode(), streamID.channelCode(),
				                         endTime - heli->recordsTimeSpan() - Core::TimeSpan(_timeSpanPerRow), Core::Time());
			}
			else {
				IO::RecordStreamPtr rs = IO::RecordStream::Open(recordStreamURL().c_str());
				if ( rs == NULL ) {
					std::cerr << "ERROR: Unable to open recordstream " << recordStreamURL() << std::endl;
					return false;
				}

				rs->addStream(streamID.networkCode(), streamID.stationCode(), streamID.locationCode(), streamID.channelCode(),
				              endTime - heli->recordsTimeSpan() - Core::TimeSpan(_timeSpanPerRow), endTime);

				try {
					IO::RecordInput ri(rs.get(), Array::FLOAT, Record::DATA_ONLY);
					for ( IO::RecordIterator it = ri.begin(); it != ri.end(); ++it )
						receivedRecord(*it);
				}
				catch ( Core::GeneralException &exc ) {
					std::cerr << "ERROR: Acquisition: " << exc.what() << std::endl;
					return false;
				}
			}
		}

		if ( _snapshotTimeout > 0 ) {
			_snapshotTimer = startTimer(_snapshotTimeout*1000);
			_streamThread->start();

			return Gui::Application::run();
		}

		_endTime = endTime;
		saveSnapshots();

		return true;
	}
	else {
		showMessage("Setup user interface");

		MainWindow* w = new MainWindow;
		setupUi(w);
		setMainWidget(w);

		if ( startFullScreen() )
			w->showFullScreen();
		else
			w->showNormal();

		return Gui::Application::run();
	}
}


void HCApp::setupUi(MainWindow *w) {
	w->setAmplitudeRange(_amplitudesMin, _amplitudesMax);
	w->fixCurrentTimeToLastRecord(_fixCurrentTimeToLastRecord);
	w->setStationDescriptionEnabled(_stationDescription);
	w->setAntialiasingEnabled(_antialiasing);
	w->setLineWidth(_lineWidth);
	w->setReferenceTime(_endTime);
	w->setTimeFormat(_timeFormat);

	DataModel::WaveformStreamID streamID;
	stringToWaveformID(streamID, _streamCodes.front());
	w->setStream(streamID);

	w->setGain(findGain(streamID, _endTime.valid()?_endTime:Core::Time::GMT()));
	w->setHeadline(findHeadline(streamID, _endTime.valid()?_endTime:Core::Time::GMT()));
	w->setPostProcessingScript(_imagePostProcessingScript);

	w->setLayout(_numberOfRows, _timeSpanPerRow);
	w->setOutputResolution(_xRes, _yRes, _dpi);
	w->setSnapshotTimeout(_snapshotTimeout);

	if ( !_rowColors.empty() )
		w->setRowColors(_rowColors);

	if ( !_filterString.empty() )
		w->setFilter(_filterString);

	w->start(_outputFilename.c_str());
}


void HCApp::timerEvent(QTimerEvent *event) {
	Gui::Application::timerEvent(event);

	if ( event->timerId() != _snapshotTimer ) return;

	saveSnapshots();
}


void HCApp::saveSnapshots() {
	HeliStreamMap::iterator it;
	for ( it = _helis.begin(); it != _helis.end(); ++it ) {
		if ( !it.value().lastSample ) {
			std::cerr << "WARNING [" << it.key() << "]: No valid records found. "
			             "will not produce output graphics." << std::endl;
			continue;
		}

		Core::Time endTime;

		if ( _endTime.valid() ) {
			endTime = _endTime;
			it.value().canvas->setCurrentTime(_endTime - Core::TimeSpan(0,1));
		}
		else {
			endTime = it.value().lastSample;
			it.value().canvas->setCurrentTime(it.value().lastSample - Core::TimeSpan(0,1));
		}

		QString from = (endTime-it.value().canvas->recordsTimeSpan()).toString(_timeFormat.c_str()).c_str();
		QString to = (endTime-Core::TimeSpan(0,1)).toString(_timeFormat.c_str()).c_str();
		QString dateline;

		if ( from != to && !from.isEmpty() && !to.isEmpty() )
			dateline = QString("%1 - %2").arg(from).arg(to);
		else
			dateline = QString("%1").arg(to);

		DataModel::WaveformStreamID streamID;
		stringToWaveformID(streamID, it.key());

		QString file = _outputFilename.c_str();
		file.replace("%N", streamID.networkCode().c_str());
		file.replace("%S", streamID.stationCode().c_str());
		file.replace("%L", streamID.locationCode().c_str());
		file.replace("%C", streamID.channelCode().c_str());

		it.value().canvas->save(it.key().c_str(), it.value().headline, dateline,
		                        file, _xRes, _yRes, _dpi);

		if ( !_imagePostProcessingScript.empty() ) {
			QProcess proc;
			proc.start(_imagePostProcessingScript.c_str(), QStringList() << file);
			if ( !proc.waitForStarted() ) {
				SEISCOMP_ERROR("Failed to start script: %s %s",
				               _imagePostProcessingScript.c_str(),
				               qPrintable(file));
			}
			else if ( !proc.waitForFinished() ) {
				SEISCOMP_ERROR("Script exited with error: %s %s",
				               _imagePostProcessingScript.c_str(),
				               qPrintable(file));
			}
		}
	}
}


void HCApp::receivedRecord(Seiscomp::Record *rec) {
	HeliStreamMap::iterator it = _helis.find(rec->streamID());
	if ( it == _helis.end() ) return;

	try {
		Core::Time endTime = rec->endTime();

		if ( !it.value().lastSample || (endTime > it.value().lastSample) ) {
			it.value().lastSample = endTime;
			if ( _fixCurrentTimeToLastRecord )
				it.value().canvas->setCurrentTime(it.value().lastSample - Core::TimeSpan(0,1));
		}

		it.value().canvas->feed(rec);
	}
	catch ( ... ) {}
}


void HCApp::acquisitionFinished() {
	if ( _streamThread ) {
		delete _streamThread;
		_streamThread = NULL;
		QApplication::quit();
	}
}



int main(int argc, char** argv) {
	int retCode;

	{
		int flags = Gui::Application::DEFAULT | Gui::Application::LOAD_STATIONS;
		Gui::Application::Type type = QApplication::GuiClient;

		if ( argc >= 2 && strcmp(argv[1], "capture") == 0 ) {
			if ( Gui::Application::minQtVersion("4.3.0") )
				// Qt 4.2.x crashes when rendering text with console
				// applications so we enable console application only if
				// at least Qt 4.3.0 is installed.
				type = QApplication::Tty;
			else
				std::cerr << "WARNING: Need Qt 4.3.0 to capture images without "
				             "a running X session." << std::endl;
		}

		HCApp app(argc, argv, flags, type);
		// app.setPrimaryMessagingGroup("LISTENER_GROUP");
		// TODO: Listen for events and picks and draw them into
		//       the trace.
		//app.addMessagingSubscription("EVENT");
		//app.addMessagingSubscription("PICK");
		// app.setMessagingUsername("");
		retCode = app();
		SEISCOMP_DEBUG("Number of remaining objects before destroying application: %d", Core::BaseObject::ObjectCount());
	}

	SEISCOMP_DEBUG("Number of remaining objects after destroying application: %d", Core::BaseObject::ObjectCount());
	return retCode;
}
