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



#define SEISCOMP_COMPONENT Application

#include <seiscomp3/core/system.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/connectiondialog.h>
#include <seiscomp3/gui/core/aboutwidget.h>
#include <seiscomp3/gui/core/utils.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/communication/servicemessage.h>
#include <seiscomp3/client/pluginregistry.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <license.h>
#include <seiscomp3/utils/files.h>
#include <seiscomp3/utils/misc.h>

#include <QSplashScreen>
#include <QMessageBox>
#include <set>
#include <iostream>
#include <string>
#include <algorithm>
#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#endif


using namespace std;
using namespace Seiscomp::Communication;
using namespace Seiscomp::DataModel;


namespace {


QString splashDefaultImage = ":/images/images/splash-default.png";


// Don't catch signal on windows since that path hasn't tested.
#ifdef WIN32
struct Initializer {
	Initializer() {
		Seiscomp::Client::Application::HandleSignals(false, false);
	}
};

Initializer __init;
#endif


class ShowPlugins : public QDialog {
	public:
		ShowPlugins(QWidget *parent = NULL) : QDialog(parent) {
			_ui.setupUi(this);
			_ui.labelHeadline->setFont(SCScheme.fonts.heading3);
			_ui.labelHeadline->setText(QString("Plugins for %1").arg(SCApp->name().c_str()));

			QString content;

			Seiscomp::Client::PluginRegistry::iterator it;
			for ( it = Seiscomp::Client::PluginRegistry::Instance()->begin();
			      it != Seiscomp::Client::PluginRegistry::Instance()->end(); ++it ) {
				QFileInfo info(it->filename.c_str());

				content += QString("<p><b>%1</b><br/>"
				                   "<i>%2</i><br/>"
				                   "File: <u>%7</u><br/>"
				                   "Author: %6<br/>"
				                   "Version: %3.%4.%5</p>")
				           .arg(info.baseName())
				           .arg((*it)->description().description.c_str())
				           .arg((*it)->description().version.major)
				           .arg((*it)->description().version.minor)
				           .arg((*it)->description().version.revision)
				           .arg((*it)->description().author.c_str())
				           .arg(info.absoluteFilePath());
			}

			_ui.content->setHtml(content);
		}

	private:
		Ui::ShowPlugins _ui;
};


void blurImage(QImage &img, int radius) {
	if ( radius < 1 ) return;

	QImage out = QImage(img.size(), QImage::Format_ARGB32);
	int div = (radius*2+1)*(radius*2+1);

	int w = out.width();
	int h = out.height();

	const QRgb *bits = (const QRgb*)img.bits();
	QRgb *out_bits = (QRgb*)out.bits();

	for ( int y = 0; y < h; ++y ) {
		for ( int x = 0; x < w; ++x, ++bits, ++out_bits ) {
			int r = 0; int g = 0; int b = 0; int a = 0;
			div = 0;
			for ( int ry = -radius; ry <= radius; ++ry ) {
				int ny = y+ry;
				if ( ny < 0 || ny >= h ) continue;
				for ( int rx = -radius; rx <= radius; ++rx ) {
					int nx = x+rx;
					if ( nx < 0 || nx >= w ) continue;
					QRgb c = *(bits + ry*w + rx);
					r += qRed(c);
					g += qGreen(c);
					b += qBlue(c);
					a += qAlpha(c);
					++div;
				}
			}

			r /= div;
			g /= div;
			b /= div;
			a /= div;

			*out_bits = qRgba(r,g,b,a);
		}
	}

	img = out;
}


void drawText(QPainter &p, const QPoint &hotspot, int align, const QString &s) {
	QRect r(hotspot, hotspot);

	QRect tr = p.fontMetrics().boundingRect(s);
	int radius = 2;
	QImage blur(tr.width()+radius*2+2, tr.height()+radius*2+2, QImage::Format_ARGB32);
	blur.fill(0);

	QPoint blurpos = hotspot - QPoint(radius+1, radius+1);
	QPainter pi(&blur);
	pi.setFont(p.font());
	QPen pen = p.pen();
	QColor c = pen.color();
	c.setAlpha(112);
	pen.setColor(c);
	pi.setPen(pen);
	pi.drawText(pi.window().adjusted(radius+1,radius+1,-radius-1,-radius-1),
	            align, s);
	pi.end();
	blurImage(blur, radius);

	if ( align & Qt::AlignLeft ) r.setRight(p.window().right());
	else if ( align & Qt::AlignRight ) {
		r.setLeft(p.window().left());
		blurpos.setX(blurpos.x()-tr.width());
	}
	else if ( align & Qt::AlignHCenter ) {
		r.setLeft(hotspot.x()-p.window().width());
		r.setRight(hotspot.x()+p.window().width());
		blurpos.setX(blurpos.x()-tr.width()/2);
	}

	if ( align & Qt::AlignTop ) r.setBottom(p.window().bottom());
	else if ( align & Qt::AlignBottom ) {
		r.setTop(p.window().top());
		blurpos.setY(blurpos.y()-tr.height());
	}
	else if ( align & Qt::AlignVCenter ) {
		r.setTop(hotspot.y()-p.window().height());
		r.setBottom(hotspot.y()+p.window().height());
		blurpos.setY(blurpos.y()-tr.height()/2);
	}

	p.drawImage(blurpos+QPoint(radius, radius),blur);
	p.drawText(r, align, s);
}


class SplashScreen : public QSplashScreen {
	public:
		SplashScreen(const QPixmap & pixmap = QPixmap(), Qt::WindowFlags f = 0)
		: QSplashScreen(pixmap, f) {}

		void setMessage(const QString &str) {
			message = str;
			repaint();
		}

		void drawContents(QPainter *painter) {
			painter->setPen(SCScheme.colors.splash.message);
			drawText(*painter, SCScheme.splash.message.pos,
			         SCScheme.splash.message.align, message);
		}

		QString message;
};


}


namespace Seiscomp {
namespace Gui {


Application* Application::_instance = NULL;
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::Application(int& argc, char **argv, int flags, Type type)
: QApplication(argc, argv, type)
, Client::Application(argc, argv)
, _settings(NULL)
, _intervalSOH(60)
, _readOnlyMessaging(false)
, _mainWidget(NULL)
, _splash(NULL)
, _dlgConnection(NULL)
, _settingsOpened(false)
, _flags(flags) {

	if ( type == QApplication::Tty )
		_flags &= ~SHOW_SPLASH;

	setDaemonEnabled(false);

	if ( _instance != this && _instance != NULL ) {
		SEISCOMP_WARNING("Another GUI application object exists already. "
		                 "This usage is not intended. "
		                 "The Application::Instance() method will return "
		                 "the last created application.");
	}

	_instance = this;

	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
	QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

	_guiGroup = "GUI";
	_thread = NULL;
	_startFullScreen = false;
	_nonInteractive = false;
	_filterCommands = true;
	_mapsDesc.isMercatorProjected = false;

	// argc and argv may be modified by QApplication. It removes the
	// commandline options it recognizes so we can go on without an
	// "unknown command" error when parsing the commandline with our own
	// class Client::CommandLine

	setDaemonEnabled(false);
	setRecordStreamEnabled(true);

	setDatabaseEnabled(_flags & WANT_DATABASE, _flags & FETCH_DATABASE);
	setMessagingEnabled(_flags & WANT_MESSAGING);

	setAutoApplyNotifierEnabled(_flags & AUTO_APPLY_NOTIFIER);
	setInterpretNotifierEnabled(_flags & INTERPRETE_NOTIFIER);

	/*
	setAutoApplyNotifierEnabled(false);
	setInterpretNotifierEnabled(false);
	*/

	setLoadInventoryEnabled(_flags & LOAD_INVENTORY);
	setLoadStationsEnabled(_flags & LOAD_STATIONS);
	setLoadConfigModuleEnabled(_flags & LOAD_CONFIGMODULE);
	setLoadCitiesEnabled(true);

	setConnectionRetries(0);

#ifndef WIN32
	if ( ::socketpair(AF_UNIX, SOCK_STREAM, 0, _signalSocketFd) )
		qFatal("Couldn't create HUP socketpair");

	_signalNotifier = new QSocketNotifier(_signalSocketFd[1], QSocketNotifier::Read, this);
	connect(_signalNotifier, SIGNAL(activated(int)), this, SLOT(handleSignalNotification()));
#else
	_signalNotifier = NULL;
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application::~Application() {
	if ( _dlgConnection ) delete _dlgConnection;
	if ( _settings ) delete _settings;
#ifndef WIN32
	close(_signalSocketFd[0]);
	close(_signalSocketFd[1]);
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Application* Application::Instance() {
	return _instance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::minQtVersion(const char *ver) {
	QString s = QString::fromLatin1(ver);
	QString sq = qVersion();
	return ((sq.section('.',0,0).toInt()<<16)+(sq.section('.',1,1).toInt()<<8)+sq.section('.',2,2).toInt()>=
	       (s.section('.',0,0).toInt()<<16)+(s.section('.',1,1).toInt()<<8)+s.section('.',2,2).toInt());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::copyToClipboard(const QAbstractItemView *view,
                                  const QHeaderView *header) {
	QAbstractItemModel *model = view->model();
	QModelIndexList items = view->selectionModel()->selectedRows();
	QString csv;
	int previousRow = -1;
	int columns = model->columnCount();

	for ( QModelIndexList::const_iterator it = items.constBegin();
	      it != items.constEnd(); ++it ) {
		if ( previousRow >= 0 )
			csv += '\n';

		int c = 0;
		for ( int i = 0; i < columns; ++i ) {
			if ( header && header->isSectionHidden(i) ) continue;
			if ( c++ > 0 ) csv += ';';
			csv += model->data(it->sibling(it->row(), i)).toString();
		}

		previousRow = it->row();
	}

	QClipboard *cb = QApplication::clipboard();
	if ( cb ) cb->setText(csv);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setMainWidget(QWidget* w) {
	_mainWidget = w;

	QMainWindow *mw = dynamic_cast<QMainWindow*>(w);
	if ( mw ) {
		QMenu *helpMenu = mw->menuBar()->findChild<QMenu*>("menuHelp");
		if ( helpMenu == NULL ) {
			helpMenu = new QMenu(mw->menuBar());
			helpMenu->setObjectName("menuHelp");
			helpMenu->setTitle("&Help");
			mw->menuBar()->addAction(helpMenu->menuAction());
		}

		QAction *a = helpMenu->addAction("&About SeisComP3");
		connect(a, SIGNAL(triggered()), this, SLOT(showAbout()));

		a = helpMenu->addAction("&Documentation index");
		a->setShortcut(QKeySequence("F1"));
		connect(a, SIGNAL(triggered()), this, SLOT(showHelpIndex()));

		a = helpMenu->addAction(QString("Documentation for %1").arg(name().c_str()));
		a->setShortcut(QKeySequence("Shift+F1"));
		connect(a, SIGNAL(triggered()), this, SLOT(showAppHelp()));

		a = helpMenu->addAction("&Loaded Plugins");
		connect(a, SIGNAL(triggered()), this, SLOT(showPlugins()));
	}

	if ( _splash )
		_splash->finish(w);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showAbout() {
	AboutWidget *w = new AboutWidget(NULL);
	w->setAttribute(Qt::WA_DeleteOnClose);
	w->setWindowModality(Qt::ApplicationModal);

	if ( _mainWidget ) {
		QPoint p = _mainWidget->geometry().center();
		QRect g = w->geometry();
		g.moveCenter(p);
		w->setGeometry(g);
	}

	w->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showHelpIndex() {
	QString indexFile = QString("%1/doc/seiscomp3/html/index.html")
	                    .arg(Environment::Instance()->shareDir().c_str());

	if ( !QFile::exists(indexFile) ) {
		QMessageBox::information(NULL, "Help index",
		                         tr("The help package has not been found (not installed?)."));
		return;
	}

	QDesktopServices::openUrl(QString("file://%1").arg(indexFile));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showAppHelp() {
	QString indexFile = QString("%1/doc/seiscomp3/html/apps/%2.html")
	                    .arg(Environment::Instance()->shareDir().c_str())
	                    .arg(name().c_str());

	if ( !QFile::exists(indexFile) ) {
		QMessageBox::information(NULL, QString("%1 help").arg(name().c_str()),
		                         tr("Help for %1 is not available.").arg(name().c_str()));
		return;
	}

	QDesktopServices::openUrl(QString("file://%1").arg(indexFile));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showPlugins() {
	ShowPlugins dlg;
	dlg.exec();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::timerSOH() {
	// Save current time
	Core::Time now = Core::Time::LocalTime();

	int factor = int(double(now - _lastSOH)*1000) / _timerSOH.interval();

	// Latency of factor 10 or higher
	if ( factor >= 10 )
		SEISCOMP_ERROR("Application latency level %d", factor);
	else if ( factor >= 2 )
		SEISCOMP_WARNING("Application latency level %d", factor);

	_lastSOH = now;

	if ( database() ) {
		if ( !database()->beginQuery("select 1") )
			SEISCOMP_ERROR("DB ping failed");
		else
			database()->endQuery();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleSignalNotification() {
	int signal;
	_signalNotifier->setEnabled(false);
	ssize_t bytesRead = ::read(_signalSocketFd[1], &signal, sizeof(signal));
	if ( bytesRead != sizeof(signal) )
		qWarning() << "Failed to read int from pipe";
	QApplication::quit();
	_signalNotifier->setEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setDatabaseSOHInterval(int secs) {
	_intervalSOH = secs;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Scheme& Application::scheme() {
	return _scheme;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QSettings &Application::settings() {
	if ( !_settings ) _settings = new QSettings;
	return *_settings;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const QSettings &Application::settings() const {
	if ( !_settings ) _settings = new QSettings;
	return *_settings;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::startFullScreen() const {
	return _startFullScreen;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::nonInteractive() const {
	return _nonInteractive;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const MapsDesc &Application::mapsDesc() const {
	return _mapsDesc;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const MessageGroups &Application::messageGroups() const {
	return _messageGroups;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::TimeSpan Application::maxEventAge() const {
	return _eventTimeAgo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QColor Application::configGetColor(const std::string& query,
                                   const QColor& base) const {
	try {
		std::string col = configGetString(query);
		return readColor(query, col, base);
	}
	catch ( ... ) {}

	return base;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Gradient Application::configGetColorGradient(const std::string& query,
                                             const Gradient& base) const {
	const Seiscomp::Config::Config &config = configuration();
	bool error = false;

	std::vector<std::string> colors = config.getStrings(query, &error);
	if ( error ) return base;

	Gradient grad;
	for ( size_t i = 0; i < colors.size(); ++i ) {
		QColor color;
		qreal value;

		std::vector<std::string> toks;
		size_t size = Core::split(toks, colors[i].c_str(), ":");
		if ( size < 2 || size > 3 ) {
			SEISCOMP_ERROR("Wrong format of color entry %lu in '%s'",
			               (unsigned long)i, query.c_str());
			return base;
		}

		if ( !Core::fromString(value, toks[0]) ) {
			SEISCOMP_ERROR("Wrong value format of color entry %lu in '%s'",
			               (unsigned long)i, query.c_str());
			return base;
		}

		bool ok;
		color = readColor("", toks[1], color, &ok);
		if ( !ok ) return base;

		QString text;
		if ( size == 3 ) text = QString::fromStdString(toks[2]);

		grad.setColorAt(value, color, text);
	}

	return grad;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QFont Application::configGetFont(const std::string& query, const QFont& base) const {
	QFont f(base);

	try {
		f.setFamily(configGetString(query + ".family").c_str());
	}
	catch ( ... ) {}

	try {
		f.setPointSize(configGetInt(query + ".size"));
	}
	catch ( ... ) {}

	try {
		f.setBold(configGetBool(query + ".bold"));
	}
	catch ( ... ) {}

	try {
		f.setItalic(configGetBool(query + ".italic"));
	}
	catch ( ... ) {}

	try {
		f.setUnderline(configGetBool(query + ".underline"));
	}
	catch ( ... ) {}

	try {
		f.setOverline(configGetBool(query + ".overline"));
	}
	catch ( ... ) {}

	return f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QPen Application::configGetPen(const std::string& query, const QPen& base) const {
	QPen p(base);

	// Color
	try {
		const std::string& colQuery = query + ".color";
		p.setColor(readColor(colQuery, configGetString(colQuery), base.color()));
	}
	catch ( ... ) {}

	// Style
	try {
		const std::string& styleQuery = query + ".style";
		p.setStyle(readPenStyle(styleQuery, configGetString(styleQuery), base.style()));
	}
	catch ( ... ) {}

	// Width
	try {
		p.setWidth(configGetDouble(query + ".width"));
	}
	catch ( ... ) {}

	return p;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QBrush Application::configGetBrush(const std::string& query, const QBrush& base) const {
	QBrush b(base);

	// Color
	try {
		const std::string& colQuery = query + ".color";
		b.setColor(readColor(colQuery, configGetString(colQuery), base.color()));
	}
	catch ( ... ) {}

	// Style
	try {
		const std::string& styleQuery = query + ".style";
		b.setStyle(readBrushStyle(styleQuery, configGetString(styleQuery), base.style()));
	}
	catch ( ... ) {}

	return b;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::configSetColorGradient(const std::string& query, const Gradient &gradient) {
	std::vector<std::string> colors;
	Gradient::const_iterator it;
	for ( it = gradient.begin(); it != gradient.end(); ++it ) {
		string c = Core::toString(it.key());
		c += ":";

		Util::toHex(c, (unsigned char)it.value().first.red());
		Util::toHex(c, (unsigned char)it.value().first.green());
		Util::toHex(c, (unsigned char)it.value().first.blue());
		if ( it.value().first.alpha() != 255 )
			Util::toHex(c, (unsigned char)it.value().first.alpha());

		if ( !it.value().second.isEmpty() ) {
			c += ":";
			c += it.value().second.toStdString();
		}
		colors.push_back(c);
	}

	_configuration.setStrings(query, colors);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::createCommandLineDescription() {
	commandline().addGroup("User interface");
	commandline().addOption("User interface", "full-screen,F", "starts the application in fullscreen");
	commandline().addOption("User interface", "non-interactive,N", "use non interactive presentation mode");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initConfiguration() {
	if ( !Client::Application::initConfiguration() )
		return false;

	QPalette pal;
	_scheme.colors.background = pal.color(QPalette::Window);
	_scheme.fetch();

	pal.setColor(QPalette::Window, _scheme.colors.background);
#if QT_VERSION >= 0x040300
	// Keep original Qt settings for buttons. This can be achieved by
	// using a custom StyleSheet:
	//   appname -stylesheet=mystyle.qss
	// mystyle.qcc
	//  QPushButton { background-color: red }
	//pal.setColor(QPalette::Button, _scheme.colors.background.lighter(110));
#endif
	setPalette(pal);

	try { _mapsDesc.location = configGetString("map.location").c_str(); }
	catch (...) { _mapsDesc.location = "@DATADIR@/maps/world%s.png"; }

	_mapsDesc.type = QString();
	try { _mapsDesc.type = configGetString("map.type").c_str(); }
	catch (...) {}

	_mapsDesc.isMercatorProjected = false;

	try {
		string proj = configGetString("map.format");
		if ( proj == "mercator" )
			_mapsDesc.isMercatorProjected = true;
		else if ( proj == "rectangular" )
			_mapsDesc.isMercatorProjected = false;
		else {
			cerr << "Unknown map format: " << proj << endl;
			return false;
		}
	}
	catch (...) {}

	_mapsDesc.cacheSize = 0;
	try { _mapsDesc.cacheSize = configGetInt("map.cacheSize"); }
	catch ( ... ) {}

	_mapsDesc.location = Environment::Instance()->absolutePath(_mapsDesc.location.toStdString()).c_str();

	_eventTimeAgo = 0.0;
	bool setTimeAgo = false;
	try {
		_eventTimeAgo += double(configGetInt("events.timeAgo.days")*24*60*60);
		setTimeAgo = true;
	}
	catch (...) {}
	try {
		_eventTimeAgo += double(configGetInt("events.timeAgo.hours")*60*60);
		setTimeAgo = true;
	}
	catch (...) {}
	try {
		_eventTimeAgo += double(configGetInt("events.timeAgo.minutes")*60);
		setTimeAgo = true;
	}
	catch (...) {}
	try {
		_eventTimeAgo += double(configGetInt("events.timeAgo.seconds"));
		setTimeAgo = true;
	}
	catch (...) {}
	try {
		_nonInteractive = configGetBool("mode.interactive") == false;
	}
	catch (...) {}
	try {
		_startFullScreen = configGetBool("mode.fullscreen");
	}
	catch (...) {}

	// Default is: display events from 1 day ago until 'now'
	if ( !setTimeAgo )
		_eventTimeAgo = double(24*60*60);

	_commandTargetClient = "";
	try {
		_commandTargetClient = configGetString("commands.target");
	}
	catch (...) {}

	setOrganizationName(agencyID().c_str());
	setApplicationName(name().c_str());

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::validateParameters() {
	if ( commandline().hasOption("full-screen") ) _startFullScreen = true;
	if ( commandline().hasOption("non-interactive") ) _nonInteractive = true;

	// There is nothing to validate. It is just the best place to show up
	// the splash screen before the time consuming initialization starts
	// and after the possible early exit because of the "--help" flag.
	if ( _flags & SHOW_SPLASH ) {
		QPixmap pmSplash;
		const Seiscomp::Environment *env = Seiscomp::Environment::Instance();

		try {
			std::string splashFile = env->absolutePath(configGetString("scheme.splash.image"));
			if ( Util::fileExists(splashFile) )
				pmSplash = QPixmap(splashFile.c_str());
		}
		catch ( ... ) {}

		if ( pmSplash.isNull() ) {
			if ( env && Util::fileExists(env->shareDir() + "/splash.png") )
				pmSplash = QPixmap((env->shareDir() + "/splash.png").c_str());
			else
				pmSplash = QPixmap(splashImagePath());

			QPainter p(&pmSplash);

			const char *appVersion = version();
			if ( appVersion == NULL )
				appVersion = frameworkVersion();

			p.setFont(SCScheme.fonts.splashVersion);
			p.setPen(SCScheme.colors.splash.version);
			drawText(p, SCScheme.splash.version.pos,
			         SCScheme.splash.version.align, QString("Release %1").arg(appVersion));
		}

		// Reset LC_ALL locale to "C" since it is overwritten during
		// first usage of QPixmap
		setlocale(LC_ALL, "C");

		_splash = new SplashScreen(pmSplash);
		_splash->setFont(SCScheme.fonts.splashMessage);
		_splash->setContentsMargins(20,20,20,100);

		_splash->setAttribute(Qt::WA_DeleteOnClose);
		connect(_splash, SIGNAL(destroyed(QObject*)),
		        this, SLOT(objectDestroyed(QObject*)));

		if ( _mainWidget )
			_splash->finish(_mainWidget);

		_splash->show();
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initSubscriptions() {
	if ( type() == QApplication::Tty )
		return Client::Application::initSubscriptions();
	else
		return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::schemaValidationNames(std::vector<std::string> &modules,
                                        std::vector<std::string> &plugins) const {
	Client::Application::schemaValidationNames(modules, plugins);
	plugins.push_back("GUI");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::initLicense() {
	if ( !License::isValid() ) {
		std::cout << std::endl;
		std::cout << "<WARNING>" << std::endl << std::endl;
		License::printWarning(std::cout);
		std::cout << std::endl << "Exiting..." << std::endl;

		if ( type() != QApplication::Tty ) {
			std::stringstream ss;
			License::printWarning(ss);
			QMessageBox::critical(NULL, "License error",
			                      ss.str().c_str());
		}

		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::init() {
	if ( !initLicense() ) return false;

	bool result = Client::Application::init();

	// Check author read-only
	try {
		vector<string> blacklistedAuthors = configGetStrings("blacklist.authors");
		if ( find(blacklistedAuthors.begin(), blacklistedAuthors.end(), author()) != blacklistedAuthors.end() )
			_readOnlyMessaging = true;
	}
	catch ( ... ) {}

	try {
		vector<string> blacklistedUsers = configGetStrings("blacklist.users");
		SEISCOMP_DEBUG("Check if user %s is blacklisted", Seiscomp::Core::getLogin().c_str());
		if ( find(blacklistedUsers.begin(), blacklistedUsers.end(), Seiscomp::Core::getLogin()) != blacklistedUsers.end() ) {
			SEISCOMP_DEBUG("User %s is blacklisted, setup read-only connection", Seiscomp::Core::getLogin().c_str());
			_readOnlyMessaging = true;
		}
	}
	catch ( ... ) {}

	_messageGroups.pick = "PICK";
	_messageGroups.amplitude = "AMPLITUDE";
	_messageGroups.magnitude = "MAGNITUDE";
	_messageGroups.location = "LOCATION";
	_messageGroups.focalMechanism = "FOCMECH";
	_messageGroups.event = "EVENT";

	try { _messageGroups.pick = configGetString("groups.pick"); }
	catch ( ... ) {}
	try { _messageGroups.amplitude = configGetString("groups.amplitude"); }
	catch ( ... ) {}
	try { _messageGroups.magnitude = configGetString("groups.magnitude"); }
	catch ( ... ) {}
	try { _messageGroups.location = configGetString("groups.location"); }
	catch ( ... ) {}
	try { _messageGroups.focalMechanism = configGetString("groups.focalMechanism"); }
	catch ( ... ) {}
	try { _messageGroups.event = configGetString("groups.event"); }
	catch ( ... ) {}

	try { _intervalSOH = configGetInt("IntervalSOH"); }
	catch ( ... ) {}

	if ( _intervalSOH > 0 ) _timerSOH.setInterval(_intervalSOH*1000);

	if ( !result && (_exitRequested || (type() == QApplication::Tty)) )
		return false;

	if ( isMessagingEnabled() && (type() != QApplication::Tty) ) {
		if ( !cdlg()->hasConnectionChanged() ) {
			const set<string>& subscriptions = subscribedGroups();
			QStringList groups;
			for ( set<string>::const_iterator it = subscriptions.begin();
			      it != subscriptions.end(); ++it )
				groups << (*it).c_str();

			cdlg()->setClientParameters(_messagingHost.c_str(),
			                            _messagingUser.c_str(),
			                            _messagingPrimaryGroup.c_str(),
			                            groups, _messagingTimeout);
		}
	}

	if ( isDatabaseEnabled() && (type() != QApplication::Tty) ) {
		cdlg()->setDefaultDatabaseParameters(_db.c_str());

		if ( !cdlg()->hasDatabaseChanged() )
			cdlg()->setDatabaseParameters(_db.c_str());

		cdlg()->connectToDatabase();
	}

	if ( !_settingsOpened && isMessagingEnabled() && (type() != QApplication::Tty) )
		cdlg()->connectToMessaging();

	/*
	if ( _flags & OPEN_CONNECTION_DIALOG ) {
		bool ok = true;

		if ( (_flags & WANT_MESSAGING) && !messaging )
			ok = false;

		if ( (_flags & WANT_DATABASE) && !database )
			ok = false;

		if ( !ok )
			showSettings();

		result = true;
	}
	*/

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QString Application::splashImagePath() const {
	return splashDefaultImage;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ConnectionDialog *Application::cdlg() {
	createSettingsDialog();
	return _dlgConnection;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::createSettingsDialog() {
	if ( _dlgConnection ) return;
	if ( type() == QApplication::Tty )
		return;

	_dlgConnection = new ConnectionDialog(&_connection, &_database);
	_dlgConnection->setMessagingEnabled(isMessagingEnabled());

	connect(_dlgConnection, SIGNAL(aboutToConnect(QString, QString, QString, int)),
	        this, SLOT(createConnection(QString, QString, QString, int)));

	connect(_dlgConnection, SIGNAL(aboutToDisconnect()),
	        this, SLOT(destroyConnection()));

	connect(_dlgConnection, SIGNAL(databaseChanged()),
	        this, SLOT(databaseChanged()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::handleInitializationError(Stage stage) {
	if ( (type() == QApplication::Tty) || (stage != MESSAGING && stage != DATABASE) ) {
		if ( stage == PLUGINS ) {
			std::cerr << "Failed to load plugins: check the log for more details" << std::endl;
			this->exit(1);
		}
		else if ( stage == LOGGING ) {
			std::cerr << "Failed to initialize logging: check the log for more details" << std::endl;
			this->exit(1);
		}

		return false;
	}

	if ( (_flags & OPEN_CONNECTION_DIALOG) && !_settingsOpened ) {
		const set<string>& subscriptions = subscribedGroups();
		QStringList groups;
		for ( set<string>::const_iterator it = subscriptions.begin();
		      it != subscriptions.end(); ++it )
			groups << (*it).c_str();

		cdlg()->setClientParameters(_messagingHost.c_str(),
		                            _messagingUser.c_str(),
		                            _messagingPrimaryGroup.c_str(),
		                            groups, _messagingTimeout);

		cdlg()->setDatabaseParameters(_db.c_str());

		cdlg()->connectToMessaging();
		cdlg()->connectToDatabase();

		_settingsOpened = true;
		showSettings();
		setDatabase(database());
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::handleInterrupt(int signal) throw() {
	ssize_t bytesWritten = ::write(_signalSocketFd[0], &signal, sizeof(signal));
	if ( bytesWritten != sizeof(signal) )
		qWarning() << "Failed to write int to pipe";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::run() {
	startMessageThread();
	if ( _thread ) _thread->setReconnectOnErrorEnabled(true);

	connect(this, SIGNAL(lastWindowClosed()),
	        this, SLOT(closedLastWindow()));

	connect(&_timerSOH, SIGNAL(timeout()), this, SLOT(timerSOH()));
	_lastSOH = Core::Time::LocalTime();
	_timerSOH.start();

	Client::Application::exit(QApplication::exec());
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::done() {
	if ( _thread ) destroyConnection();
	if ( _mainWidget ) {
		QWidget *mainWidget = _mainWidget;
		_mainWidget = NULL;
		delete mainWidget;
	}

	Client::Application::done();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showMessage(const char* msg) {
	if ( _splash )
		static_cast<SplashScreen*>(_splash)->setMessage(msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showWarning(const char* msg) {
	if ( type() != QApplication::Tty )
		QMessageBox::warning(NULL, "Warning", msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::notify(QObject *receiver, QEvent *e) {
	try {
		return QApplication::notify(receiver, e);
	}
	catch ( std::exception &e ) {
		SEISCOMP_ERROR("An exception occurred while calling an event handler: %s", e.what());
		::exit(-1);
	}
	catch ( ... ) {
		SEISCOMP_ERROR("An unknown exception occurred while calling an event handler");
		::exit(-1);
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::sendMessage(Seiscomp::Core::Message* msg) {
	return sendMessage(NULL, msg);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Application::sendMessage(const char* group, Seiscomp::Core::Message* msg) {
	bool result = false;

	if ( _readOnlyMessaging ) {
		QMessageBox::critical(activeWindow(), tr("Read-only connection"),
		                      tr("This is a read-only session. No message has been sent."));
		return false;
	}

	if ( connection() )
		result =
			group?
				connection()->send(group, msg)
				:
				connection()->send(msg);


	if ( result ) return true;

	QMessageBox msgBox(activeWindow());
	QPushButton *settingsButton = msgBox.addButton(tr("Setup connection"), QMessageBox::ActionRole);
	QPushButton *retryButton = msgBox.addButton(tr("Retry"), QMessageBox::ActionRole);
	QPushButton *abortButton = msgBox.addButton(QMessageBox::Abort);

	msgBox.setWindowTitle("Error");
	msgBox.setText("Sending the message failed!\nAre you connected?");
	msgBox.setIcon(QMessageBox::Critical);

	while ( !result ) {
		msgBox.exec();

		if ( msgBox.clickedButton() == retryButton ) {
			if ( connection() )
				result =
					group?
						connection()->send(group, msg)
					:
						connection()->send(msg);
		}
		else if ( msgBox.clickedButton() == settingsButton ) {
			showSettings();
		}
		else if (msgBox.clickedButton() == abortButton) {
			break;
		}
	}

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::showSettings() {
	if ( !isMessagingEnabled() && !isDatabaseEnabled() ) return;

	if ( _thread ) _thread->setReconnectOnErrorEnabled(false);

	cdlg()->exec();

	if ( cdlg()->hasDatabaseChanged() ) {
		/*
		if ( query() )
			query()->setDriver(_database.get());
		*/
		emit changedDatabase();
	}

	if ( _thread )
		_thread->setReconnectOnErrorEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::createConnection(QString host, QString user,
                                   QString group, int timeout) {
	int status = 0;

	SEISCOMP_DEBUG("createConnection(%s, %s, %s, %d)",
	               (const char*)host.data(),
	               (const char*)user.data(),
	               (const char*)group.data(),
	               timeout);

	_connection = Connection::Create(host.toStdString(), user.toStdString(), group.toStdString(),
	                                 Protocol::PRIORITY_DEFAULT, timeout, &status);

	if ( _connection == NULL ) {
		QMessageBox::warning(NULL, "ConnectionError",
		                     QString("Could not establish connection for:\n"
		                     "  Host: %1\n"
		                     "  User: %2\n"
		                     "  Group: %3\n"
		                     "  Timeout: %4\n"
		                     "\n"
		                     "  ERROR: %5")
		                     .arg(host).arg(user).arg(group)
		                     .arg(timeout).arg(Core::Status::StatusToStr(status)));
	}
	else {
		MessageEncoding enc;
		if ( enc.fromString(_messagingEncoding.c_str()) ) {
			SEISCOMP_INFO("Setting message encoding to %s", _messagingEncoding.c_str());
			_connection->setEncoding(enc);
		}
	}

	_messagingUser = user.toStdString();
	_messagingHost = host.toStdString();
	_messagingPrimaryGroup = group.toStdString();
	_messagingTimeout = timeout;

	startMessageThread();
	if ( _thread )
		_thread->setReconnectOnErrorEnabled(false);

	emit changedConnection();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::destroyConnection() {
	if ( _thread )
		_thread->setReconnectOnErrorEnabled(false);

	if ( _connection )
		_connection->disconnect();

	closeMessageThread();

	ConnectionDialog *dlg = cdlg();
	if ( dlg != NULL )
		dlg->setDefaultDatabaseParameters("","");

	_connection = NULL;
	emit changedConnection();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::databaseChanged() {
	if ( query() ) {
		query()->setDriver(_database.get());
		_db = cdlg()->databaseURI();
		if ( query()->hasError() ) {
			_database->disconnect();
			QMessageBox::critical(NULL, "Database Error",
			                      query()->errorMsg().c_str());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::startMessageThread() {
	if ( _thread ) {
		if ( _thread->connection() != _connection )
			closeMessageThread();
		else
			return;
	}

	if ( _connection ) {
		_thread = new MessageThread(_connection.get());
		//connect(_thread, SIGNAL(finished()), _thread, SLOT(deleteLater()));
		connect(_thread, SIGNAL(messagesAvailable()), this, SLOT(messagesAvailable()));
		connect(_thread, SIGNAL(connectionError(int)), this, SLOT(connectionError(int)));
		connect(_thread, SIGNAL(connectionEstablished()), this, SLOT(onConnectionEstablished()));
		connect(_thread, SIGNAL(connectionLost()), this, SLOT(onConnectionLost()));
		connect(_thread, SIGNAL(connectionEstablished()), this, SIGNAL(connectionEstablished()));
		connect(_thread, SIGNAL(connectionLost()), this, SIGNAL(connectionLost()));
		_thread->setReconnectOnErrorEnabled(true);
		_thread->start();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::closeMessageThread() {
	if ( _thread ) {
		_thread->setReconnectOnErrorEnabled(false);
		_thread->wait();
		delete _thread;
		_thread = NULL;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::messagesAvailable() {
	if ( !_connection ) return;

	Seiscomp::Core::MessagePtr msg;
	NetworkMessagePtr nmsg;
	while ( true ) {
		NetworkMessage *tmp_msg = NULL;
		msg = _connection->readQueuedMessage(Connection::READ_ALL, &tmp_msg);
		nmsg = tmp_msg;
		if ( !msg ) {
			// end of traffic?
			if ( !nmsg )
				break;

			emit messageSkipped(nmsg.get());
			continue;
		}

		if ( isDatabaseEnabled() ) {
			Communication::DatabaseProvideMessage* dbmsg = Communication::DatabaseProvideMessage::Cast(msg);
			if ( dbmsg && cdlg() ) {
				cdlg()->setDefaultDatabaseParameters(dbmsg->service(), dbmsg->parameters());
				if ( database() == NULL ) {
					cdlg()->setDatabaseParameters(dbmsg->service(), dbmsg->parameters());
					cdlg()->connectToDatabase();
					if ( cdlg()->hasDatabaseChanged() ) {
						_db = cdlg()->databaseURI();
						setDatabase(database());
					}
				}
			}
		}

		CommandMessage* cmd = CommandMessage::Cast(msg);
		if ( cmd && _filterCommands ) {
			QRegExp re(cmd->client().c_str());
			if ( re.exactMatch(_messagingUser.c_str()) )
				emit messageAvailable(cmd, nmsg.get());
			else {
				SEISCOMP_DEBUG("Ignoring command message for client: %s, user is: %s",
				               cmd->client().c_str(), _messagingUser.c_str());
			}

			continue;
		}

		emit messageAvailable(msg.get(), nmsg.get());

		NotifierMessage* nm = NotifierMessage::Cast(msg);

		if ( isAutoApplyNotifierEnabled() ) {
			if ( !nm ) {
				for ( Core::MessageIterator it = msg->iter(); *it; ++it ) {
					DataModel::Notifier* n = DataModel::Notifier::Cast(*it);
					if ( n ) {
						SEISCOMP_DEBUG("Non persistent notifier for '%s'", n->parentID().c_str());
						n->apply();
					}
				}
			}
			else {
				for ( NotifierMessage::iterator it = nm->begin(); it != nm->end(); ++it ) {
					SEISCOMP_DEBUG("Notifier for '%s'", (*it)->parentID().c_str());
					(*it)->apply();
				}
			}
		}

		if ( !nm ) {
			for ( Core::MessageIterator it = msg->iter(); *it; ++it ) {
				DataModel::Notifier* n = DataModel::Notifier::Cast(*it);
				if ( n ) {
					emitNotifier(n);
				}
			}
		}
		else {
			for ( NotifierMessage::iterator it = nm->begin(); it != nm->end(); ++it )
				emitNotifier(it->get());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::emitNotifier(Notifier* n) {
	emit notifierAvailable(n);
	if ( isInterpretNotifierEnabled() ) {
		switch ( n->operation() ) {
			case OP_ADD:
				emit addObject(n->parentID().c_str(), n->object());
				break;
			case OP_REMOVE:
				emit removeObject(n->parentID().c_str(), n->object());
				break;
			case OP_UPDATE:
				emit updateObject(n->parentID().c_str(), n->object());
				break;
			default:
				break;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::onConnectionEstablished() {
	if ( type() != QApplication::Tty)
		cdlg()->connectToMessaging();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::onConnectionLost() {
	if ( type() != QApplication::Tty)
		cdlg()->onConnectionError(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::connectionError(int code) {
	if ( type() == QApplication::Tty) return;

	if ( _connection && !_connection->isConnected() ) {
		SEISCOMP_ERROR("Connection went away...");
		closeMessageThread();
		cdlg()->onConnectionError(code);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::objectDestroyed(QObject* o) {
	if ( o == _splash )
		_splash = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::closedLastWindow() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::exit(int returnCode) {
	if ( _thread )
		_thread->setReconnectOnErrorEnabled(false);

	if ( qApp ) qApp->quit();

	Client::Application::exit(returnCode);

	closeMessageThread();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::setFilterCommandsEnabled(bool e) {
	_filterCommands = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Application::commandTarget() const {
	return _commandTargetClient;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::sendCommand(Command command, const std::string& parameter) {
	sendCommand(command, parameter, NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Application::sendCommand(Command command, const std::string& parameter, Core::BaseObject *obj) {
	if ( commandTarget().empty() ) {
		QMessageBox::critical(NULL,
		            "Commands",
		            "Variable <commands.target> is not set. To disable sending commands "
		            "to all connected clients, set a proper target. You can use "
		            "regular expressions to specify a group of clients (HINT: all = '.*$').");
		return;
	}

	CommandMessagePtr cmsg = new CommandMessage(commandTarget(), command);
	cmsg->setParameter(parameter);
	cmsg->setObject(obj);

	sendMessage(_guiGroup.c_str(), cmsg.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
