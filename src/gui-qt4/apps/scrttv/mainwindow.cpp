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

#define SEISCOMP_COMPONENT Gui::TraceView

#include "mainwindow.h"
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/record.h>
#include <seiscomp3/core/datamessage.h>
#include <seiscomp3/io/recordinput.h>
#include <seiscomp3/io/recordstream/file.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/datamodel/inventory_package.h>
#include <seiscomp3/datamodel/config_package.h>
#include <seiscomp3/datamodel/eventparameters_package.h>
#include <seiscomp3/datamodel/messages.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/utils/keyvalues.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/utils/timer.h>
#include <seiscomp3/gui/core/recordviewitem.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/scheme.h>
#include <seiscomp3/gui/core/infotext.h>
#include <seiscomp3/gui/datamodel/origindialog.h>
#include <seiscomp3/gui/datamodel/inventorylistview.h>
#include <set>
#include <fstream>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::IO;
using namespace Seiscomp::DataModel;


namespace {

QString waveformIDToString(const WaveformStreamID& id) {
	return (id.networkCode() + "." + id.stationCode() + "." +
	        id.locationCode() + "." + id.channelCode()).c_str();
}

string waveformIDToStdString(const WaveformStreamID& id) {
	return (id.networkCode() + "." + id.stationCode() + "." +
	        id.locationCode() + "." + id.channelCode());
}


QString prettyPrint(long seconds) {
	long days = seconds / 86400;
	long secondsPerDay = seconds % 86400;

	long hours = secondsPerDay / 3600;
	long secondsPerHour = secondsPerDay % 3600;

	long minutes = secondsPerHour / 60;
	seconds = secondsPerHour % 60;

	/*
	if ( days > 0 )
		return QString("%1 days and %2 hours").arg(days).arg(hours);
	else if ( hours > 0 )
		return QString("%1 hours and %2 minutes").arg(hours).arg(minutes);
	else if ( minutes > 0 )
		return QString("%1 minutes and %2 seconds").arg(minutes).arg(seconds);
	else
		return QString("%1 seconds").arg(seconds);
	*/

	return QString("%1:%2:%3:%4")
	        .arg(days,4,10,QChar(' '))
	        .arg(hours,2,10,QChar('0'))
	        .arg(minutes,2,10,QChar('0'))
	        .arg(seconds,2,10,QChar('0'));
}


}


Q_DECLARE_METATYPE(Seiscomp::Applications::TraceView::TraceState)


namespace Seiscomp {
namespace Applications {
namespace TraceView {


namespace {


class TabEditWidget : public QLineEdit {
	public:
		TabEditWidget(int index, QWidget *parent = 0) : QLineEdit(parent), _index(index) {
			setAttribute(Qt::WA_DeleteOnClose);
		}

		int index() const { return _index; }

	protected:
		void focusOutEvent(QFocusEvent *e) {
			close();
		}

	private:
		int _index;
};


}


class TraceDecorator : public Gui::RecordWidgetDecorator {
	public:
		TraceDecorator(QObject *parent, const MainWindow::DecorationDesc *desc)
		: Gui::RecordWidgetDecorator(parent), _desc(desc) {}

		void drawLine(QPainter *painter, Gui::RecordWidget *widget, float y, bool fillAbove,
		              const QBrush &brush) {
			QPair<float,float> range = widget->amplitudeRange(0);
			range.first *= *widget->recordScale(0);
			range.second *= *widget->recordScale(0);
			float amplRange = range.second-range.first;
			if ( amplRange > 0 ) {
				int y0 = widget->streamYPos(0);
				int h = widget->streamHeight(0);
				int y1 = y0+h;
				int py = y1-(y-range.first)*h / (range.second-range.first);
				painter->drawLine(0,py,painter->window().width(),py);

				if ( brush != Qt::NoBrush ) {
					if ( fillAbove ) {
						if ( py > y1) py = y1;
						if ( py > y0 )
							painter->fillRect(0,y0,painter->window().width(),py-y0,brush);
					}
					else {
						if ( py <= y0) py = y0;
						if ( py <= y1 )
							painter->fillRect(0,py,painter->window().width(),y1-py,brush);
					}
				}
			}
		}

		void drawDecoration(QPainter *painter, Gui::RecordWidget *widget) {
			if ( _desc->minValue ) {
				painter->setPen(_desc->minPen);
				drawLine(painter, widget, *_desc->minValue, false, _desc->minBrush);
			}

			if ( _desc->maxValue ) {
				painter->setPen(_desc->maxPen);
				drawLine(painter, widget, *_desc->maxValue, true, _desc->maxBrush);
			}

			int yofs = 0;
			if ( !_desc->description.isEmpty() ) {
				painter->setFont(SCScheme.fonts.highlight);
				QRect r = painter->boundingRect(painter->window(), Qt::AlignLeft | Qt::AlignTop, _desc->description);
				r.adjust(-4,-4,4,4);
				r.moveTo(0,0);
				painter->fillRect(r, widget->palette().color(QPalette::Text));
				painter->setPen(widget->palette().color(QPalette::Window));
				painter->drawText(r, Qt::AlignCenter, _desc->description);
				yofs += r.height();
			}

			QPair<float,float> amplRange = widget->amplitudeDataRange(0);
			QString amps = QString("min: %1").arg(amplRange.first, 0, 'f', 1);
			if ( !_desc->unit.isEmpty() ) amps += _desc->unit;
			amps += "\n";
			amps += QString("max: %1").arg(amplRange.second, 0, 'f', 1);
			if ( !_desc->unit.isEmpty() ) amps += _desc->unit;
			painter->setFont(SCScheme.fonts.base);
			painter->setPen(widget->palette().color(QPalette::Text));
			QRect r = painter->boundingRect(painter->window(), Qt::AlignLeft | Qt::AlignTop, amps);
			r.adjust(-4,-4,4,4);
			r.moveTo(0,yofs);
			painter->fillRect(r, widget->palette().color(QPalette::Base));
			r.translate(4,4);
			painter->drawText(r, Qt::AlignLeft | Qt::AlignTop, amps);
		}

	private:
		const MainWindow::DecorationDesc *_desc;
};



TraceViewTabBar::TraceViewTabBar(QWidget *parent) : QTabBar(parent) {}


int TraceViewTabBar::findIndex(const QPoint& p) {
	for ( int i = 0; i < count(); ++i )
		if ( tabRect(i).contains(p) )
			return i;

	return -1;
}

void TraceViewTabBar::mousePressEvent(QMouseEvent *e) {
	QTabBar::mousePressEvent(e);
	return;

	// Disable renaming of tabs for now
	/*
	if ( e->button() != Qt::LeftButton ) {
		QTabBar::mousePressEvent(e);
		return;
	}

	int pressedTab = findIndex(e->pos());
	if ( pressedTab != currentIndex() ) {
		QTabBar::mousePressEvent(e);
		return;
	}

	TabEditWidget *edit = new TabEditWidget(currentIndex(), this);

	connect(edit, SIGNAL(returnPressed()),
	        this, SLOT(textChanged()));

	connect(this, SIGNAL(currentChanged(int)),
	        edit, SLOT(close()));

	edit->setGeometry(tabRect(currentIndex()));
	edit->setText(tabText(currentIndex()));
	edit->selectAll();
	edit->setFocus();
	edit->show();
	*/
}

void TraceViewTabBar::textChanged() {
	TabEditWidget *editor = (TabEditWidget*)sender();
	if ( !editor ) return;

	QTabWidget* tw = dynamic_cast<QTabWidget*>(parent());
	if ( tw )
		tw->setTabText(editor->index(), editor->text());
	editor->close();
}


TraceView::TraceView(const Seiscomp::Core::TimeSpan& span,
                     QWidget *parent, Qt::WFlags f)
: Seiscomp::Gui::RecordView(span, parent, f) {
	_timeSpan = (double)span;
}

TraceView::~TraceView() {}


TraceTabWidget::TraceTabWidget(QWidget* parent)
	: QTabWidget(parent) {

	_nonInteractive = true;

	_tabBar = new TraceViewTabBar;
	setTabBar(_tabBar);

	setAcceptDrops(true);
	_tabBar->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(_tabBar, SIGNAL(customContextMenuRequested(const QPoint&)),
	        this, SLOT(showContextMenu(const QPoint&)));

	QAction *closeTab = new QAction("Close tab", this);
	closeTab->setShortcut(Qt::Key_F4 | Qt::CTRL);

	addAction(closeTab);
	_tabActions.append(closeTab);

	connect(closeTab, SIGNAL(triggered()),
	        this, SLOT(closeTab()));

	_tabToClose = -1;
}


void TraceTabWidget::closeTab() {
	if ( _nonInteractive ) return;

	int tab = _tabToClose;
	if ( tab == -1 )
		tab = currentIndex();

	emit tabRemovalRequested(tab);
}


void TraceTabWidget::checkActiveTab(const QPoint& p) {
	int tab = _tabBar->findIndex(_tabBar->mapFromParent(p));
	if ( tab != -1 )
		setCurrentIndex(tab);
}

bool TraceTabWidget::checkDraging(QDropEvent *event) {
	checkActiveTab(event->pos());

	Seiscomp::Gui::RecordView* view = dynamic_cast<Seiscomp::Gui::RecordView*>(event->source());
	if ( view == widget(currentIndex()) )
		return false;

	return true;
}

void TraceTabWidget::dragEnterEvent(QDragEnterEvent *event) {
	checkActiveTab(event->pos());
	event->acceptProposedAction();
}

void TraceTabWidget::showContextMenu(const QPoint& p) {
	if ( _nonInteractive ) return;
	_tabToClose = _tabBar->findIndex(p);
	_tabToClose = -1;
}

void TraceTabWidget::dragMoveEvent(QDragMoveEvent *event) {
	if ( !checkDraging(event) ) return;
	event->acceptProposedAction();
}

void TraceTabWidget::dropEvent(QDropEvent *event) {
	if ( !checkDraging(event) ) return;

	if (event->possibleActions() & Qt::MoveAction) {
		Seiscomp::Gui::RecordView* source = dynamic_cast<Seiscomp::Gui::RecordView*>(event->source());
		Seiscomp::Gui::RecordView* target = static_cast<Seiscomp::Gui::RecordView*>(widget(currentIndex()));
		if ( source && target ) {
			//source->moveSelectionTo(target);
			emit moveSelectionRequested(target, source);
			event->acceptProposedAction();
		}
	}
}

void TraceTabWidget::showEvent(QShowEvent *) {
	for ( int i = 0; i < count(); ++i )
		widget(i)->setVisible(widget(i) == currentWidget());
}


#define TRACEVIEWS(method)\
	foreach ( TraceView* view, _traceViews ) view->method

#define CURRENT_TRACEVIEW(method)\
	if ( _tabWidget ) \
		static_cast<TraceView*>(_tabWidget->currentWidget())->method;\
	else\
		_traceViews.front()->method

MainWindow::MainWindow() : _questionApplyChanges(this) {
	_ui.setupUi(this);

	_questionApplyChanges.setText("You are about to enable/disable one or more streams.\n"
	                              "As a result all streams of the station(s) the stream(s)\n"
	                              "belong(s) to will be changed as well.\n"
	                              "This information will be sent as a message\n"
	                              "to tell all clients adding/removing this station(s).\n"
	                              "Do you want to continue changing the state?");

	_bufferSize = Core::TimeSpan(1800,0);
	_recordStreamThread = NULL;
	_tabWidget = NULL;
	_currentFilterIdx = -1;
	_autoApplyFilter = false;
	_allowTimeWindowExtraction = true;

	_statusBarFile   = new QLabel;
	_statusBarFilter = new QLabel(" Filter OFF ");
	_statusBarSearch = new QLineEdit;
	_statusBarProg   = new Seiscomp::Gui::ProgressBar;

	statusBar()->addPermanentWidget(_statusBarFilter, 1);
	statusBar()->addPermanentWidget(_statusBarSearch, 1);
	statusBar()->addPermanentWidget(_statusBarFile,   5);
	statusBar()->addPermanentWidget(_statusBarProg,   1);

	_statusBarSearch->setVisible(false);

	_searchBase = _statusBarSearch->palette().color(QPalette::Base);
	_searchError = Gui::blend(Qt::red, _searchBase, 50);

	_showPicks = true;

	connect(_statusBarSearch, SIGNAL(textChanged(const QString&)),
	        this, SLOT(search(const QString&)));

	connect(_statusBarSearch, SIGNAL(returnPressed()),
	        this, SLOT(nextSearch()));

	//setCentralWidget(createTraceView());

	connect(_ui.actionSelectStreams, SIGNAL(triggered()), this, SLOT(selectStreams()));
	connect(_ui.actionListHiddenStreams, SIGNAL(triggered()), this, SLOT(listHiddenStreams()));
	//connect(_ui.actionAddTabulator, SIGNAL(triggered()), this, SLOT(addTabulator()));

	connect(_ui.actionOpen, SIGNAL(triggered()), this, SLOT(openFile()));
	connect(_ui.actionOpenSeedLink, SIGNAL(triggered()), this, SLOT(openAcquisition()));
	connect(_ui.actionOpenXMLFile, SIGNAL(triggered()), this, SLOT(openXML()));
	connect(_ui.actionQuit, SIGNAL(triggered()), this, SLOT(close()));

	connect(_ui.actionCycleFilters, SIGNAL(triggered(bool)), this, SLOT(cycleFilters(bool)));
	connect(_ui.actionCycleFiltersReverse, SIGNAL(triggered(bool)), this, SLOT(cycleFiltersReverse(bool)));
	connect(_ui.actionApplyGain, SIGNAL(toggled(bool)), this, SLOT(showScaledValues(bool)));
	connect(_ui.actionRestoreConfigOrder, SIGNAL(triggered()), this, SLOT(sortByConfig()));
	connect(_ui.actionSortDistance, SIGNAL(triggered()), this, SLOT(sortByDistance()));
	connect(_ui.actionSortStaCode, SIGNAL(triggered()), this, SLOT(sortByStationCode()));
	connect(_ui.actionSortNetStaCode, SIGNAL(triggered()), this, SLOT(sortByNetworkStationCode()));

	connect(_ui.actionAlignLeft, SIGNAL(triggered()), this, SLOT(alignLeft()));
	connect(_ui.actionAlignRight, SIGNAL(triggered()), this, SLOT(alignRight()));
	connect(_ui.actionJumpToLastRecord, SIGNAL(triggered()), this, SLOT(jumpToLastRecord()));
	connect(_ui.actionClearPickMarkers, SIGNAL(triggered()), this, SLOT(clearPickMarkers()));

	connect(_ui.actionAlignOriginTime, SIGNAL(triggered()), this, SLOT(alignOriginTime()));

	connect(_ui.actionLineUp, SIGNAL(triggered()), this, SLOT(scrollLineUp()));
	connect(_ui.actionLineDown, SIGNAL(triggered()), this, SLOT(scrollLineDown()));
	connect(_ui.actionPageUp, SIGNAL(triggered()), this, SLOT(scrollPageUp()));
	connect(_ui.actionPageDown, SIGNAL(triggered()), this, SLOT(scrollPageDown()));
	connect(_ui.actionToTop, SIGNAL(triggered()), this, SLOT(scrollToTop()));
	connect(_ui.actionToBottom, SIGNAL(triggered()), this, SLOT(scrollToBottom()));
	connect(_ui.actionScrollLeft, SIGNAL(triggered()), this, SLOT(scrollLeft()));
	connect(_ui.actionScrollRight, SIGNAL(triggered()), this, SLOT(scrollRight()));

	connect(_ui.actionSearch, SIGNAL(triggered()), this, SLOT(enableSearch()));
	connect(_ui.actionAbortSearch, SIGNAL(triggered()), this, SLOT(abortSearch()));
	
	connect(SCApp, SIGNAL(messageAvailable(Seiscomp::Core::Message*, Seiscomp::Communication::NetworkMessage*)),
	        this, SLOT(messageArrived(Seiscomp::Core::Message*, Seiscomp::Communication::NetworkMessage*)));

	connect(SCApp, SIGNAL(addObject(const QString&, Seiscomp::DataModel::Object*)),
	        this, SLOT(objectAdded(const QString&, Seiscomp::DataModel::Object*)));

	connect(SCApp, SIGNAL(updateObject(const QString&, Seiscomp::DataModel::Object*)),
	        this, SLOT(objectUpdated(const QString&, Seiscomp::DataModel::Object*)));

	_timer = new QTimer(this);
	_timer->setInterval(1000);

	_automaticSortEnabled = true;
	_inventoryEnabled = true;

	_switchBack = new QTimer(this);
	_switchBack->setSingleShot(true);

	connect(_timer, SIGNAL(timeout()), this, SLOT(step()));
	connect(_switchBack, SIGNAL(timeout()), this, SLOT(sortByConfig()));

	addAction(_ui.actionAddTabulator);
	addAction(_ui.actionSearch);
	addAction(_ui.actionAbortSearch);
	addActions(_ui.menu_Interaction->actions());

	_rowSpacing = 0;
	_withFrames = false;
	_frameMargin = 0;
	_rowHeight = -1;
	_numberOfRows = -1;

	try { _rowSpacing = SCApp->configGetInt("streams.rowSpacing"); }
	catch ( ... ) {}
	try { _withFrames = SCApp->configGetBool("streams.withFrames"); }
	catch ( ... ) {}
	try { _frameMargin = SCApp->configGetInt("streams.frameMargin"); }
	catch ( ... ) {}

	try { _rowHeight = SCApp->configGetInt("streams.height"); }
	catch ( ... ) {}

	try { _numberOfRows = SCApp->configGetInt("streams.rows"); }
	catch ( ... ) {}

	QPen defaultMinPen, defaultMaxPen;
	QBrush defaultMinBrush, defaultMaxBrush;
	OPT(double) defaultMinMaxMargin;

	try {
		_autoApplyFilter = SCApp->configGetBool("autoApplyFilter");
	}
	catch ( ... ) {}

	try {
		defaultMinMaxMargin = SCApp->configGetDouble("streams.defaults.minMaxMargin");
	}
	catch ( ... ) {}

	try {
		defaultMinPen = SCApp->configGetPen("streams.defaults.minimum.pen", defaultMinPen);
	}
	catch ( ... ) {}
	try {
		defaultMaxPen = SCApp->configGetPen("streams.defaults.maximum.pen", defaultMaxPen);
	}
	catch ( ... ) {}

	try {
		defaultMinBrush = SCApp->configGetBrush("streams.defaults.minimum.brush", defaultMinBrush);
	}
	catch ( ... ) {}
	try {
		defaultMaxBrush = SCApp->configGetBrush("streams.defaults.maximum.brush", defaultMaxBrush);
	}
	catch ( ... ) {}

	try {
		vector<string> profiles = SCApp->configGetStrings("streams.profiles");
		for ( size_t i = 0; i < profiles.size(); ++i ) {
			string match;
			string prefix = "streams.profile." + profiles[i] + ".";

			try {
				match = SCApp->configGetString(prefix + "match");
			}
			catch ( ... ) {
				continue;
			}

			_decorationDescs.resize(_decorationDescs.size()+1);
			DecorationDesc &desc = _decorationDescs.back();
			desc.matchID = match;
			desc.minMaxMargin = defaultMinMaxMargin;
			try { desc.description = SCApp->configGetString(prefix + "description").c_str(); }
			catch ( ... ) {}
			try { desc.minMaxMargin = SCApp->configGetDouble(prefix + "minMaxMargin"); }
			catch ( ... ) {}
			try { desc.unit = SCApp->configGetString(prefix + "unit").c_str(); }
			catch ( ... ) {}
			try { desc.gain = SCApp->configGetDouble(prefix + "gain"); }
			catch ( ... ) {}
			try { desc.minValue = SCApp->configGetDouble(prefix + "minimum.value"); }
			catch ( ... ) {}
			try { desc.fixedScale = SCApp->configGetBool(prefix + "fixedScale"); }
			catch ( ... ) { desc.fixedScale = false; }
			try { desc.minPen = SCApp->configGetPen(prefix + "minimum.pen", defaultMinPen); }
			catch ( ... ) {}
			try { desc.minBrush = SCApp->configGetBrush(prefix + "minimum.brush", defaultMinBrush); }
			catch ( ... ) {}
			try { desc.maxValue = SCApp->configGetDouble(prefix + "maximum.value"); }
			catch ( ... ) {}
			try { desc.maxPen = SCApp->configGetPen(prefix + "maximum.pen", defaultMaxPen); }
			catch ( ... ) {}
			try { desc.maxBrush = SCApp->configGetBrush(prefix + "maximum.brush", defaultMaxBrush); }
			catch ( ... ) {}
		}
	}
	catch ( ... ) {}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MainWindow::~MainWindow() {
	if ( _recordStreamThread ) {
		_recordStreamThread->stop(true);
		delete _recordStreamThread;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::start() {
	std::vector<std::string> files = SCApp->commandline().unrecognizedOptions();
	std::vector<std::string>::iterator it;
	for ( it = files.begin(); it != files.end(); ) {
		if ( it->size() > 1 && (*it)[0] == '-' )
			it = files.erase(it);
		else
			++it;
	}

	if ( SCApp->commandline().hasOption("record-file") || !files.empty() ) {
		try {
			files.push_back(SCApp->commandline().option<std::string>("record-file"));
		}
		catch ( ... ) {}

		openFile(files);
	}
	else
		openAcquisition();

	try {
		double lat = Gui::Application::Instance()->configGetDouble("streams.sort.latitude");
		double lon = Gui::Application::Instance()->configGetDouble("streams.sort.longitude");

		sortByOrigin(lat, lon);
	}
	catch ( ... ) {}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::setStartTime(const Seiscomp::Core::Time &t) {
	_startTime = t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::setEndTime(const Seiscomp::Core::Time &t) {
	_endTime = t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::setAllowTimeWindowExtraction(bool f) {
	_allowTimeWindowExtraction = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::setMaximumDelay(int d) {
	_maxDelay = d;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::setShowPicks(bool e) {
	_showPicks = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::setAutomaticSortEnabled(bool e) {
	_automaticSortEnabled = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::setInventoryEnabled(bool e) {
	_inventoryEnabled = e;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::setBufferSize(Core::TimeSpan bs) {
	_bufferSize = bs;

	foreach ( TraceView* view, _traceViews ) {
		view->setTimeSpan(_bufferSize);
		view->setTimeRange(-_bufferSize, 0);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::setFiltersByName(const std::vector<std::string> &filters) {
	_filters = filters;

	if ( _autoApplyFilter )
		cycleFilters(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::ConfigStation* MainWindow::configStation(const string& networkCode,
                                                    const string& stationCode) const {
	if ( !SCApp->configModule() )
		return NULL;

	for ( size_t i = 0; i < SCApp->configModule()->configStationCount(); ++i ) {
		ConfigStation* cs = SCApp->configModule()->configStation(i);
		if ( cs->networkCode() == networkCode &&
		     cs->stationCode() == stationCode )
			return cs;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MainWindow::isStationEnabled(const string& networkCode,
                                  const string& stationCode) const {
	ConfigStation* cs = configStation(networkCode, stationCode);
	return cs?cs->enabled():true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::setStationEnabled(const string& networkCode,
                                   const string& stationCode,
                                   bool enable) {
	updateTraces(networkCode, stationCode, enable);

	ConfigStation* cs = configStation(networkCode, stationCode);
	if ( !cs ) {
		ConfigModule *module = SCApp->configModule();
		if ( !module ) {
			_statusBarFile->setText("Settings the station state is disabled, no configmodule available");
			return;
		}

		ConfigStationPtr newCs = ConfigStation::Create("Config/" + module->name() + "/" + networkCode + "/" + stationCode);
		newCs->setNetworkCode(networkCode);
		newCs->setStationCode(stationCode);
		newCs->setEnabled(enable);

		CreationInfo ci;
		ci.setAuthor(SCApp->author());
		ci.setAgencyID(SCApp->agencyID());
		ci.setCreationTime(Core::Time::GMT());

		newCs->setCreationInfo(ci);

		Notifier::Enable();
		module->add(newCs.get());
		Notifier::Disable();

		cs = newCs.get();
	}

	if ( cs->enabled() != enable ) {
		cs->setEnabled(enable);
		SEISCOMP_INFO("Set station %s.%s state to: %d",
		              cs->networkCode().c_str(),
		              cs->stationCode().c_str(),
		              enable);

		CreationInfo *ci;
		try {
			ci = &cs->creationInfo();
			ci->setModificationTime(Core::Time::GMT());
		}
		catch ( ... ) {
			cs->setCreationInfo(CreationInfo());
			ci = &cs->creationInfo();
			ci->setCreationTime(Core::Time::GMT());
		}

		ci->setAuthor(SCApp->author());
		ci->setAgencyID(SCApp->agencyID());

		Notifier::Enable();
		cs->update();
		Notifier::Disable();
	}

	Core::MessagePtr msg = Notifier::GetMessage(true);
	
	if ( msg )
		SCApp->sendMessage("CONFIG", msg.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::updateTraces(const std::string& networkCode,
                              const std::string& stationCode,
                              bool enable) {
	QList<Seiscomp::Gui::RecordViewItem*> items;

	foreach ( TraceView* view, _traceViews )
		items += view->stationStreams(networkCode, stationCode);

	if ( items.empty() ) return;

	_statusBarFile->setText(QString("Station %1.%2 has been %3 at %4 (localtime)")
	                        .arg(networkCode.c_str())
	                        .arg(stationCode.c_str())
	                        .arg(enable?"enabled":"disabled")
	                        .arg(Seiscomp::Core::Time::LocalTime().toString("%F %T").c_str()));

	if ( SCApp->nonInteractive() ) {
		foreach ( Seiscomp::Gui::RecordViewItem* item, items )
			if ( item->isInvisibilityForced() == enable )
				item->forceInvisibilty(!enable);
	}
	else {
		Seiscomp::Gui::RecordView* targetView = _traceViews[enable?0:1];

		foreach ( Seiscomp::Gui::RecordViewItem* item, items ) {
			if ( item->recordView() != targetView ) {
				if ( item->recordView()->takeItem(item) )
					targetView->addItem(item);
				else
					throw Core::GeneralException("Unable to take item");
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::moveSelection(Seiscomp::Gui::RecordView* target,
                               Seiscomp::Gui::RecordView* source) {
	if ( target == source ) return;

	bool newState;
	if ( target == _traceViews[0] )
		newState = true;
	else if ( target == _traceViews[1] )
		newState = false;
	else
		throw Core::GeneralException("Unknown view");

	set< pair<string,string> > stations;
	QList<Seiscomp::Gui::RecordViewItem*> items = source->selectedItems();
	foreach(Seiscomp::Gui::RecordViewItem* item, items)
		stations.insert(pair<string,string>(item->streamID().networkCode(),
		                                    item->streamID().stationCode()));

	QString stationList("Stations to be changed:\n");
	for ( set< pair<string,string> >::iterator it = stations.begin();
	      it != stations.end(); ++it )
		stationList += QString("- %1.%2\n").arg(it->first.c_str()).arg(it->second.c_str());

	_questionApplyChanges.setInfo(stationList);

	if ( _questionApplyChanges.exec() == QDialog::Rejected )
		return;

	for ( set< pair<string,string> >::iterator it = stations.begin();
	      it != stations.end(); ++it )
		setStationEnabled(it->first, it->second, newState);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
TraceView* MainWindow::createTraceView() {
	TraceView* traceView = new TraceView(Seiscomp::Core::TimeSpan(_bufferSize));
	traceView->setRowSpacing(_rowSpacing);
	traceView->setFramesEnabled(_withFrames);
	traceView->setFrameMargin(_frameMargin);
	if ( _rowHeight > 0 ) {
		traceView->setMinimumRowHeight(_rowHeight);
		traceView->setMaximumRowHeight(_rowHeight);
	}
	traceView->setRelativeRowHeight(_numberOfRows);
	traceView->setAlternatingRowColors(true);
	traceView->setAutoScale(true);
	traceView->setDefaultDisplay();
	traceView->setSelectionEnabled(false);
	traceView->setSelectionMode(Seiscomp::Gui::RecordView::ExtendedSelection);
	traceView->setDefaultItemColumns(4);
	traceView->layout()->setMargin(6);
	traceView->showScrollBar(!SCApp->nonInteractive());

	// determine the required label width
	int labelWidth=0;
	QFont f(traceView->font());
	QFontMetrics fm(f);
	labelWidth += fm.boundingRect("WW WW WWW").width();
	f.setBold(true);
	fm = QFontMetrics(f);
	labelWidth += fm.boundingRect("WWWW ").width();

	traceView->setLabelWidth(labelWidth);

	connect(traceView, SIGNAL(progressStarted()), _statusBarProg, SLOT(reset()));
	connect(traceView, SIGNAL(progressChanged(int)), _statusBarProg, SLOT(setValue(int)));
	connect(traceView, SIGNAL(progressFinished()), _statusBarProg, SLOT(reset()));

	connect(traceView->timeWidget(), SIGNAL(dragged(double)),
	        traceView, SLOT(move(double)));

	connect(traceView, SIGNAL(filterChanged(const QString&)),
	        this, SLOT(filterChanged(const QString&)));

	connect(_ui.actionToggleAllRecords, SIGNAL(toggled(bool)), traceView, SLOT(showAllRecords(bool)));

	connect(_ui.actionHorZoomIn, SIGNAL(triggered()), traceView, SLOT(horizontalZoomIn()));
	connect(_ui.actionHorZoomOut, SIGNAL(triggered()), traceView, SLOT(horizontalZoomOut()));
	connect(_ui.actionVerZoomIn, SIGNAL(triggered()), traceView, SLOT(verticalZoomIn()));
	connect(_ui.actionVerZoomOut, SIGNAL(triggered()), traceView, SLOT(verticalZoomOut()));
	connect(_ui.actionToggleZoom, SIGNAL(triggered(bool)), traceView, SLOT(setZoomEnabled(bool)));
	
	//connect(_ui.actionAlignPickTime, SIGNAL(triggered()), _traceView, SLOT(setAlignPickTime()));
	connect(_ui.actionDefaultDisplay, SIGNAL(triggered()), traceView, SLOT(setDefaultDisplay()));
	//connect(_ui.actionFilter, SIGNAL(triggered()), _traceView, SLOT(openFilterDialog()));
	//connect(_ui.actionDisplay, SIGNAL(triggered()), _traceView, SLOT(openDisplayDialog()));

	connect(_ui.actionNormalizeVisibleAmplitudes, SIGNAL(toggled(bool)), this, SLOT(scaleVisibleAmplitudes(bool)));

	connect(traceView, SIGNAL(selectedTime(Seiscomp::Gui::RecordWidget*, Seiscomp::Core::Time)),
	        this, SLOT(selectedTime(Seiscomp::Gui::RecordWidget*, Seiscomp::Core::Time)));

	connect(traceView, SIGNAL(addedItem(const Seiscomp::Record*, Seiscomp::Gui::RecordViewItem*)),
	        this, SLOT(setupItem(const Seiscomp::Record*, Seiscomp::Gui::RecordViewItem*)));

	if ( !_traceViews.empty() )
		traceView->copyState(_traceViews.front());

	_traceViews.append(traceView);

	return traceView;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::scaleVisibleAmplitudes(bool enable) {
	TRACEVIEWS(setAutoMaxScale(enable));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::listHiddenStreams() {
	Seiscomp::Gui::InfoText info(this);
	info.setWindowModality(Qt::WindowModal);
	info.setWindowTitle("Hidden streams");

	QString data;

	data = "<table>";
	data += "<tr><td><strong>ID</strong></td><td><strong>Delay</strong></td></tr>";

	int numberOfLines = 0;

	Core::Time now = Core::Time::GMT();
	foreach ( TraceView* view, _traceViews ) {
		for ( int i = 0; i < view->rowCount(); ++i ) {
			Seiscomp::Gui::RecordViewItem* item = view->itemAt(i);

			if ( item->isVisible() ) continue;

			Seiscomp::Gui::RecordWidget *w = item->widget();
			Seiscomp::Core::Time lastSample;
			if ( w->records() != NULL ) lastSample = w->records()->timeWindow().endTime();

			QString state;

			if ( item->isInvisibilityForced() )
				state = "disabled";

			if ( !lastSample.valid() )
				data += QString("<tr><td>%1</td><td></td><td>%2</td></tr>")
				                .arg(waveformIDToString(item->streamID())).arg(state);
			else
				data += QString("<tr><td>%1</td><td align=right>%2</td><td>%3</td></tr>")
				                .arg(waveformIDToString(item->streamID()))
				                .arg(prettyPrint((now - lastSample).seconds()))
				                .arg(state);

			++numberOfLines;
		}
	}

	data += "</table>";

	if ( !numberOfLines ) return;

	info.setText(data);
	info.exec();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::removeTab(int index) {
	TraceView* traceView = static_cast<TraceView*>(_tabWidget->widget(index));
	_tabWidget->removeTab(index);

	if ( _tabWidget->count() == 0 )
		return;

	TraceView* mainView = static_cast<TraceView*>(_tabWidget->widget(0));

	_traceViews.remove(_traceViews.indexOf(traceView));
	traceView->moveItemsTo(mainView);
	delete traceView;

	if ( _tabWidget->count() == 1 ) {
		mainView->setParent(NULL);

		delete _tabWidget;
		_tabWidget = NULL;

		//setCentralWidget(mainView);
		centralWidget()->layout()->addWidget(mainView);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::setupItem(const Record*, Gui::RecordViewItem* item) {
	item->label()->setInteractive(false);
	item->label()->setOrientation(Qt::Horizontal);
	QFont f(item->label()->font(0));
	f.setBold(true);
	item->label()->setFont(f, 0);

	const WaveformStreamID &streamID = item->streamID();
	string streamIDStr = waveformIDToStdString(streamID);

	for ( size_t i = 0; i < _decorationDescs.size(); ++i ) {
		DecorationDesc &desc = _decorationDescs[i];
		if ( !Core::wildcmp(desc.matchID, streamIDStr) ) continue;

		TraceDecorator *deco = new TraceDecorator(item->widget(), &desc);
		item->widget()->setDecorator(deco);
		item->widget()->setDrawOffset(false);
		if ( desc.gain && *desc.gain != 0.0 )
			item->widget()->setRecordScale(0, 1.0 / *desc.gain);

		if ( desc.minValue && desc.maxValue && desc.minMaxMargin ) {
			double center = (*desc.minValue + *desc.maxValue) * 0.5;
			double vmin = (*desc.minValue-center)*(1+*desc.minMaxMargin)+center;
			double vmax = (*desc.maxValue-center)*(1+*desc.minMaxMargin)+center;
			if ( desc.fixedScale )
				item->widget()->setAmplRange(vmin / *item->widget()->recordScale(0),
				                             vmax / *item->widget()->recordScale(0));
			else
				item->widget()->setMinimumAmplRange(vmin / *item->widget()->recordScale(0),
				                                    vmax / *item->widget()->recordScale(0));
		}

		break;
	}

	item->widget()->showScaledValues(_ui.actionApplyGain->isChecked());

	if ( !_scaleMap.contains(streamID) ) {
		try {
			double scale = 1.0 / Client::Inventory::Instance()->getGain(
					streamID.networkCode(), streamID.stationCode(),
					streamID.locationCode(), streamID.channelCode(),
					_endTime.valid()?_endTime:Core::Time::GMT()
				);

			_scaleMap[streamID] = scale;
			item->widget()->setRecordScale(0, scale);
		}
		catch ( ... ) {}
	}

	item->label()->setText(streamID.stationCode().c_str(), 0);
	item->label()->setText(streamID.networkCode().c_str(), 1);
	item->label()->setText(streamID.locationCode().empty()?
	                       "  ":streamID.locationCode().c_str(), 2);
	item->label()->setText(streamID.channelCode().c_str(), 3);

	QFontMetrics fm(item->label()->font(0));
	item->label()->setWidth(fm.boundingRect("WWWW ").width(), 0);

	fm = QFontMetrics(item->label()->font(1));
	item->label()->setWidth(fm.boundingRect("WW ").width(), 1);

	fm = QFontMetrics(item->label()->font(1));
	item->label()->setWidth(fm.boundingRect("WW ").width(), 2);

	fm = QFontMetrics(item->label()->font(1));
	item->label()->setWidth(fm.boundingRect("WWW").width(), 3);

	item->setContextMenuPolicy(Qt::CustomContextMenu);

	if ( SCApp->nonInteractive() ) return;

	item->setDraggingEnabled(true);
	connect(item, SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(itemCustomContextMenuRequested(const QPoint &)));
	connect(item, SIGNAL(clickedOnTime(Seiscomp::Gui::RecordViewItem*, Seiscomp::Core::Time)),
	        this, SLOT(createOrigin(Seiscomp::Gui::RecordViewItem*, Seiscomp::Core::Time)));
	connect(item->label(), SIGNAL(doubleClicked()), this, SLOT(changeTraceState()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::itemCustomContextMenuRequested(const QPoint &p) {
	Gui::RecordViewItem *item = static_cast<Gui::RecordViewItem*>(sender());
	QPoint tracePos = item->widget()->mapFromGlobal(item->mapToGlobal(p));

	QMenu menu(this);

	QAction *artificialOrigin = NULL;

	if ( tracePos.x() >= 0 && tracePos.x() < item->widget()->width() )
		artificialOrigin = menu.addAction("Create artificial origin");

	if ( menu.isEmpty() ) return;

	QAction *action = menu.exec(item->mapToGlobal(p));

	if ( artificialOrigin && action == artificialOrigin ) {
		QPoint tracePos = item->widget()->mapFromGlobal(item->mapToGlobal(p));
		createOrigin(item, item->widget()->unmapTime(tracePos.x()));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::changeTraceState() {
	Seiscomp::Gui::RecordLabel* label = (Seiscomp::Gui::RecordLabel*)sender();
	Seiscomp::Gui::RecordViewItem* item = label->recordViewItem();
	if ( item == NULL ) return;

	Seiscomp::Gui::RecordView* view = item->recordView();
	if ( view == NULL ) return;

	bool newState;
	if ( view == _traceViews[0] )
		newState = false;
	else if ( view == _traceViews[1] )
		newState = true;
	else
		throw Core::GeneralException("Unknown view");

	_questionApplyChanges.setInfo("Stations to be changed:\n" +
	                              QString(" - %1.%2")
	                               .arg(item->streamID().networkCode().c_str())
	                               .arg(item->streamID().stationCode().c_str()));

	if ( _questionApplyChanges.exec() == QDialog::Rejected )
		return;

	setStationEnabled(item->streamID().networkCode(),
	                  item->streamID().stationCode(),
	                  newState);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::createOrigin(Gui::RecordViewItem* item, Core::Time time) {
	if ( item->data().type() == QVariant::Invalid ) {
		if ( !_statusBarFile ) return;

		const double *v = item->widget()->value(time);
		if ( v == NULL ) {
			_statusBarFile->setText("");
		}
		else {
			_statusBarFile->setText(QString("value = %1").arg(*v, 0, 'f', 2));
		}

		return;
	}

	Client::StationLocation loc = item->data().value<TraceState>().location;

	Gui::OriginDialog dlg(loc.longitude, loc.latitude, this);
	dlg.setTime(time);
	if ( dlg.exec() != QDialog::Accepted ) return;

	DataModel::Origin* origin = DataModel::Origin::Create();
	DataModel::CreationInfo ci;
	ci.setAgencyID(SCApp->agencyID());
	ci.setAuthor(SCApp->author());
	ci.setCreationTime(Core::Time::GMT());
	origin->setCreationInfo(ci);
	origin->setLongitude(dlg.longitude());
	origin->setLatitude(dlg.latitude());
	origin->setDepth(DataModel::RealQuantity(dlg.depth()));
	origin->setTime(Core::Time(dlg.getTime_t()));

	//Seiscomp::DataModel::ArtificialOriginMessage message(origin);
	//SCApp->sendMessage("GUI", &message);
	SCApp->sendCommand(Gui::CM_OBSERVE_LOCATION, "", origin);

	if ( _automaticSortEnabled )
		sortByOrigin(loc.latitude, loc.longitude);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::openFile() {
	QString filename = QFileDialog::getOpenFileName(this, "Choose a file", "");
	if ( filename.isEmpty() )
		return;

	std::vector<std::string> files;
	files.push_back(filename.toStdString());
	openFile(files);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::openFile(const std::vector<std::string> &files) {
	RecordStream::File stream;

	//setCentralWidget(createTraceView());
	centralWidget()->layout()->addWidget(createTraceView());

	while ( _tabWidget != NULL )
		removeTab(0);

	TRACEVIEWS(setBufferSize(Core::TimeSpan(0,0)));

	_traceViews.front()->setAutoInsertItem(true);
	_traceViews.front()->clear();

	_traceViews.front()->setUpdatesEnabled(false);

	for ( size_t i = 0; i < files.size(); ++i ) {
		//TODO: Resolve wildcards using boost filesystem
		stream.setRecordType("mseed");

		if ( !stream.setSource(files[i]) ) {
			cerr << "could not open file '" << files[i] << "'" << endl;
			continue;
		}

		try {
			std::string type = SCApp->commandline().option<std::string>("record-type");
			if ( !stream.setRecordType(type.c_str()) ) {
				cerr << "unable to set recordtype '" << type << "'" << endl;
				continue;
			}
		}
		catch ( ... ) {}

		IO::RecordInput input(&stream, Array::FLOAT, Record::DATA_ONLY);

		cout << "loading " << files[i] << "..." << flush;
		Util::StopWatch t;
	
		for ( RecordIterator it = input.begin(); it != input.end(); ++it )
			_traceViews.front()->feed(*it);
	
		cout << "(" << t.elapsed() << " sec)" << endl;
	}

	Core::TimeWindow tw = _traceViews.front()->coveredTimeRange();
	_originTime = tw.endTime();
	_traceViews.front()->setAlignment(_originTime);

	setBufferSize(tw.length());
	alignRight();

	_traceViews.front()->setUpdatesEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::openAcquisition() {
	_recordStreamThread = new Seiscomp::Gui::RecordStreamThread(SCApp->recordStreamURL());
	if ( !_endTime )
		_originTime = Core::Time::GMT();
	else
		_originTime = _endTime;

	TRACEVIEWS(setBufferSize(Core::TimeSpan(_bufferSize)));

	if ( !_recordStreamThread->connect() ) {
		delete _recordStreamThread;
		_recordStreamThread = NULL;
		QMessageBox::information(this, "Error", QString("Could not connect to stream '%1'").arg(SCApp->recordStreamURL().c_str()));
		return;
	}

	centralWidget()->layout()->addWidget(createTraceView());

	while ( _tabWidget != NULL )
		removeTab(0);

	if ( !SCApp->nonInteractive() ) {
		// Create the Enabled/Disabled tabs
		addTabulator();

		_tabWidget->setTabText(_tabWidget->indexOf(_traceViews[0]), "Enabled");
		_tabWidget->setTabIcon(_tabWidget->indexOf(_traceViews[0]), QIcon(":icons/icons/enabled.png"));
		_tabWidget->setTabText(_tabWidget->indexOf(_traceViews[1]), "Disabled");
		_tabWidget->setTabIcon(_tabWidget->indexOf(_traceViews[1]), QIcon(":icons/icons/disabled.png"));

		_tabWidget->setCurrentIndex(0);
	}

	if ( _allowTimeWindowExtraction )
		_recordStreamThread->setTimeWindow(Core::TimeWindow(_originTime - Core::TimeSpan(_bufferSize), _endTime));

	if ( _inventoryEnabled ) {
		typedef QPair<QString,int> ChannelEntry;
		
		QMap<QString, QMap<QString, QMultiMap<QString, ChannelEntry> > > streamMap;
		QList<WaveformStreamID> requestMap;

		bool usePreconfigured = false;

		try {
			std::vector<std::string> vstreams = SCApp->configGetStrings("streams.codes");
			if ( vstreams.empty() ) usePreconfigured = true;
			else if ( vstreams[0] == "default" ) usePreconfigured = true;

			QStringList streams;
			for ( size_t i = 0; i < vstreams.size(); ++i ) {
				if ( vstreams[i] == "default" ) continue;
				streams << vstreams[i].c_str();
			}

			int index = 0;
			foreach ( const QString& stream, streams ) {
				QStringList tokens = stream.split(".");

				if ( tokens.count() >= 1 ) {
					if ( tokens.count() > 4 ) {
						cerr << "error in entry '" << stream.toStdString() << "': too many tokens, missing ',' ? -> ignoring" << endl;
						continue;
					}
					else
						requestMap.append(WaveformStreamID(tokens[0].toStdString(),
						                                   tokens.count()>1?tokens[1].toStdString():"*",
						                                   tokens.count()>2?tokens[2].toStdString():"*",
						                                   tokens.count()>3?tokens[3].toStdString():"*",""));

					QMap<QString, QMultiMap<QString, ChannelEntry> > & stationMap = streamMap[requestMap.last().networkCode().c_str()];
					stationMap[requestMap.last().stationCode().c_str()].insert(requestMap.last().locationCode().c_str(), ChannelEntry(requestMap.last().channelCode().c_str(),index));
				}

				++index;
			}
		}
		catch ( ... ) {
			usePreconfigured = true;
		}

		if ( usePreconfigured ) {
			cout << "using configured streams of config module" << endl;
			//streamMap["*"]["*"].insert("*", ChannelEntry("*",0));

			DataModel::ConfigModule* module = SCApp->configModule();

			if ( module ) {
				int index = 0;
		
				for ( size_t j = 0; j < module->configStationCount(); ++j ) {
					DataModel::ConfigStation *station = module->configStation(j);
					DataModel::Setup *setup = DataModel::findSetup(station, SCApp->name());
					if ( setup ) {
						DataModel::ParameterSet* ps = DataModel::ParameterSet::Find(setup->parameterSetID());
						if ( !ps ) {
							SEISCOMP_ERROR("Cannot find parameter set %s", setup->parameterSetID().c_str());
							continue;
						}
		
						Util::KeyValues params;
						params.init(ps);

						std::string net, sta, loc, cha;
						net = station->networkCode();
						sta = station->stationCode();

						params.getString(loc, "detecLocid");
						params.getString(cha, "detecStream");

						if ( !cha.empty() ) {
							if ( cha.size() < 3 ) {
								char compCode = 'Z';

								//cerr << " * " << net << "." << sta << "." << loc << "." << cha << 'Z' << endl;
								SensorLocation *sloc = Client::Inventory::Instance()->getSensorLocation(net, sta, loc, _originTime);
								if ( sloc ) {
									Stream *stream = getVerticalComponent(sloc, cha.c_str(), _originTime);
									if ( stream && !stream->code().empty() )
										cha = stream->code();
									else
										cha += compCode;
								}
								else
									cha += compCode;
							}

							requestMap.append(WaveformStreamID(net, sta, loc, cha, ""));
							QMap<QString, QMultiMap<QString, ChannelEntry> > & stationMap = streamMap[requestMap.last().networkCode().c_str()];
							stationMap[requestMap.last().stationCode().c_str()].insert(requestMap.last().locationCode().c_str(), ChannelEntry(requestMap.last().channelCode().c_str(),index));
							++index;
						}
					}
				}
			}
		}

		QRectF regionRect;

		try {
			double lonMin = Gui::Application::Instance()->configGetDouble("streams.region.lonmin");
			double lonMax = Gui::Application::Instance()->configGetDouble("streams.region.lonmax");
			double latMin = Gui::Application::Instance()->configGetDouble("streams.region.latmin");
			double latMax = Gui::Application::Instance()->configGetDouble("streams.region.latmax");

			regionRect.setRect(lonMin, latMin, lonMax-lonMin, latMax-latMin);
		}
		catch ( ... ) {
		}

		TRACEVIEWS(clear());
		TRACEVIEWS(setAutoInsertItem(false));

		if ( _recordStreamThread )
			_recordStreamThread->stop(true);

		Inventory* inv = Seiscomp::Client::Inventory::Instance()->inventory();
		if ( inv == NULL ) {
			QMessageBox::information(this, "Error", "Could not read inventory (NULL)");
			return;
		}

		for ( size_t i = 0; i < inv->networkCount(); ++i ) {
			Network* net = inv->network(i);
			try {
				if ( net->end() < _originTime ) continue;
			}
			catch ( ... ) {}

			foreach ( const WaveformStreamID& wfsi, requestMap ) {
				if ( wfsi.networkCode() == "*" ) {
					WaveformStreamID nwfsi(net->code(), wfsi.stationCode(),
					                       wfsi.locationCode(), wfsi.channelCode(), "");
					if ( requestMap.contains(nwfsi) ) continue;
					requestMap.append(nwfsi);
				}
			}

			QMap<QString, QMultiMap<QString, ChannelEntry> >& staCodes = streamMap[net->code().c_str()];
			if ( staCodes.isEmpty() ) staCodes = streamMap["*"];

			if ( staCodes.isEmpty() ) continue;

			for ( size_t j = 0; j < net->stationCount(); ++j ) {
				Station* sta = net->station(j);
				try {
					if ( sta->end() < _originTime ) continue;
				}
				catch ( ... ) {}

				if ( !regionRect.isEmpty() ) {
					try {
						if ( !regionRect.contains(sta->longitude(), sta->latitude()) )
							continue;
					}
					catch ( ... ) {
						continue;
					}
				}

				foreach ( const WaveformStreamID& wfsi, requestMap ) {
					if ( wfsi.stationCode() == "*" && wfsi.networkCode() == net->code() ) {
						WaveformStreamID nwfsi(wfsi.networkCode(), sta->code(),
						                       wfsi.locationCode(), wfsi.channelCode(), "");
						if ( requestMap.contains(nwfsi) ) continue;
						requestMap.append(nwfsi);
					}
				}

				QMultiMap<QString, ChannelEntry>& locCodes = staCodes[sta->code().c_str()];
				if ( locCodes.isEmpty() ) locCodes = staCodes["*"];

				if ( locCodes.isEmpty() ) continue;

				for ( size_t l = 0; l < sta->sensorLocationCount(); ++l ) {
					SensorLocation *loc = sta->sensorLocation(l);

					try {
						if ( loc->end() < _originTime ) continue;
					}
					catch ( ... ) {}

					QList<ChannelEntry> chaCodes = locCodes.values(loc->code().c_str());
					if ( chaCodes.isEmpty() ) chaCodes = locCodes.values("*");
					if ( chaCodes.isEmpty() ) continue;

					for ( size_t s = 0; s < loc->streamCount(); ++s ) {
						Stream* stream = loc->stream(s);

						try {
							if ( stream->end() < _originTime ) continue;
						}
						catch ( ... ) {}

						QString compCode;

						bool foundChaCode = false;
						int index = 0;

						foreach ( const ChannelEntry& chaCode, chaCodes ) {
							if ( chaCode.first == "*" || Core::wildcmp(chaCode.first.toStdString(), stream->code()) ) {
								foundChaCode = true;
								index = chaCode.second;
							}
						}

						if ( foundChaCode ) {
							double scale = 1.0;
							try { scale = 1.0 / stream->gain(); }
							catch ( ... ) {}

							_waveformStreams.insert(WaveformStreamEntry(
							                          WaveformStreamID(net->code(), sta->code(),
							                                           loc->code(), stream->code(),
							                                           ""), index, scale));
							cerr << " + " << net->code() << "." << sta->code()
							     << "." << loc->code() << "." << stream->code() << endl;
						}
					}
				}
			}
		}

		TRACEVIEWS(hide());

		//QProgressDialog progress(this);
		//progress.setWindowTitle(tr("Please wait..."));
		//progress.setRange(0, _waveformStreams.size());

		SCApp->showMessage(QString("Added 0/%1 streams")
		                   .arg(_waveformStreams.size()).toAscii());
		cout << "Adding " << _waveformStreams.size() << " streams" << endl;

		//ofstream of("streams");

		int count = 0;
		for ( WaveformStreamSet::iterator it = _waveformStreams.begin();
		      it != _waveformStreams.end(); ++it, ++count ) {
			SEISCOMP_DEBUG("Adding row: %s.%s.%s.%s", it->streamID.networkCode().c_str(),
			               it->streamID.stationCode().c_str(), it->streamID.locationCode().c_str(),
			               it->streamID.channelCode().c_str());

			bool stationEnabled = isStationEnabled(it->streamID.networkCode(), it->streamID.stationCode());

			// Later on the items should be sorted into the right tabs
			TraceView *view = _traceViews[SCApp->nonInteractive()?0:(stationEnabled?0:1)];

			Seiscomp::Gui::RecordViewItem* item = view->addItem(it->streamID, waveformIDToString(it->streamID));

			if ( item == NULL ) continue;

			if ( SCApp->nonInteractive() )
				item->forceInvisibilty(!stationEnabled);

			_scaleMap[item->streamID()] = it->scale;
			item->widget()->setRecordScale(0, it->scale);

			QPalette pal = item->widget()->palette();
			pal.setColor(QPalette::WindowText, QColor(128,128,128));
			pal.setColor(QPalette::HighlightedText, QColor(128,128,128));
			item->widget()->setPalette(pal);

			setupItem(NULL, item);

			item->setValue(0, (*it).index);
			item->setValue(1, 0.0);

			//of << waveformIDToString((*it).first).toStdString() << endl;

			/*
			_recordStreamThread->addStream((*it).first.networkCode(), (*it).first.stationCode(),
			                               (*it).first.locationCode(), (*it).first.channelCode());
			*/

			try {
				TraceState state;
				state.location = Seiscomp::Client::Inventory::Instance()->stationLocation(it->streamID.networkCode(),
				                                                         it->streamID.stationCode(),
				                                                         _originTime);
				QVariant d;
				d.setValue(state);

				item->setData(d);
			}
			catch ( ... ) {}

			SCApp->showMessage(QString("Added %1/%2 streams")
			                   .arg(count+1)
			                   .arg(_waveformStreams.size()).toAscii());
			//progress.setValue(progress.value()+1);
			//progress.update();
		}

		foreach ( const WaveformStreamID& wfsi, requestMap ) {
			if ( wfsi.networkCode() == "*" || wfsi.stationCode() == "*" ) continue;
			_recordStreamThread->addStream(wfsi.networkCode(), wfsi.stationCode(),
			                               wfsi.locationCode(), wfsi.channelCode());
		}

		sortByConfig();
		TRACEVIEWS(show());
	}
	else {
		while ( _tabWidget != NULL )
			removeTab(0);

		if ( _recordStreamThread )
			_recordStreamThread->stop(true);

		try {
			std::vector<std::string> vstreams = SCApp->configGetStrings("streams.codes");

			QStringList streams;
			for ( size_t i = 0; i < vstreams.size(); ++i )
				streams << vstreams[i].c_str();

			foreach ( const QString& stream, streams ) {
				QStringList tokens = stream.split(".");

				if ( tokens.count() >= 1 ) {
					if ( tokens.count() > 4 )
						cout << "error in entry '" << stream.toStdString() << "': too many tokens, missing ',' ? -> ignoring" << endl;
					else
						_recordStreamThread->addStream(tokens[0].toStdString(),
						                               tokens.count()>1?tokens[1].toStdString():"*",
						                               tokens.count()>2?tokens[2].toStdString():"*",
						                               tokens.count()>3?tokens[3].toStdString():"*");
				}
			}
		}
		catch ( ... ) {}

		TRACEVIEWS(clear());
		TRACEVIEWS(setAutoInsertItem(true));
		TRACEVIEWS(show());
	}

	connect(_recordStreamThread, SIGNAL(receivedRecord(Seiscomp::Record*)),
	        this, SLOT(receivedRecord(Seiscomp::Record*)));
	_recordStreamThread->start();

	TRACEVIEWS(setJustification(1.0));
	TRACEVIEWS(horizontalZoom(1.0));
	TRACEVIEWS(setAlignment(_originTime));

	_timer->start();

	if ( _showPicks && SCApp->query() ) {
		SCApp->showMessage("Loading picks");
		DatabaseIterator it = SCApp->query()->getPicks(
			_originTime - Core::TimeSpan(_bufferSize), _originTime
		);

		for ( ; *it; ++it ) {
			PickPtr pick = Pick::Cast(*it);
			if ( pick )
				addPick(pick.get());
		}

		SCApp->showMessage(QString("Loaded %1 picks").arg(it.count()).toAscii());
		cout << "Added " << it.count() << " picks from database" << endl;
	}

	alignRight();
	checkTraceDelay();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::openXML() {
	QString filename = QFileDialog::getOpenFileName(this,
		tr("Open XML file"), "", tr("XML files (*.xml);;All (*.*)"));

	if ( filename.isEmpty() ) return;

	qApp->setOverrideCursor(Qt::WaitCursor);

	IO::XMLArchive ar;
	ar.open(filename.toStdString().c_str());

	bool regWasEnabled = PublicObject::IsRegistrationEnabled();
	PublicObject::SetRegistrationEnabled(false);

	DataModel::EventParametersPtr ep;
	ar >> ep;

	PublicObject::SetRegistrationEnabled(regWasEnabled);

	if ( !ep ) {
		qApp->restoreOverrideCursor();
		QMessageBox::information(this, tr("Error"), tr("No event parameters found in XML file."));
		return;
	}

	cerr << "Loaded " << ep->pickCount() << " picks" << endl;

	int accepted = 0, rejected = 0;
	for ( size_t i = 0; i < ep->pickCount(); ++i ) {
		if ( addPick(ep->pick(i)) )
			++accepted;
		else
			++rejected;
	}

	QMessageBox::information(this, "Load XML", tr("Added %1/%2 picks")
	                         .arg(accepted).arg(accepted+rejected));

	qApp->restoreOverrideCursor();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::selectStreams() {
	Gui::InventoryListView* list = new Gui::InventoryListView(this, Qt::Window);
	list->setAttribute(Qt::WA_DeleteOnClose);

	for ( WaveformStreamSet::iterator it = _waveformStreams.begin();
	      it != _waveformStreams.end(); ++it ) {
		list->selectStream(waveformIDToString(it->streamID), true);
	}

	list->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::addTabulator() {
	if ( !_tabWidget ) {
		_tabWidget = new TraceTabWidget(this);

		SCScheme.applyTabPosition(_tabWidget);

		if ( !_traceViews.isEmpty() )
			_traceViews.front()->setParent(NULL);
		else
			createTraceView();

		_tabWidget->addTab(_traceViews.front(), "Default");

		connect(_tabWidget, SIGNAL(tabRemovalRequested(int)),
		        this, SLOT(removeTab(int)));

		connect(_tabWidget, SIGNAL(moveSelectionRequested(Seiscomp::Gui::RecordView*,
		                                                  Seiscomp::Gui::RecordView*)),
		        this, SLOT(moveSelection(Seiscomp::Gui::RecordView*,
		                                 Seiscomp::Gui::RecordView*)));

		//setCentralWidget(_tabWidget);
		centralWidget()->layout()->addWidget(_tabWidget);
	}

	Seiscomp::Gui::RecordView* source = static_cast<Seiscomp::Gui::RecordView*>(_tabWidget->currentWidget());
	Seiscomp::Gui::RecordView* target = createTraceView();
	source->moveSelectionTo(target);
	_tabWidget->addTab(target, "New Tab");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::filterChanged(const QString &s) {
	_filters.clear();
	_filters.push_back(s.toStdString());
	_currentFilterIdx = 0;
	_statusBarFilter->setText(QString(" Filter ON : %1").arg(_filters[_currentFilterIdx].c_str()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::enableSearch() {
	_statusBarSearch->selectAll();
	_statusBarSearch->setVisible(true);
	//_statusBarSearch->grabKeyboard();
	_statusBarSearch->setFocus();

	foreach ( TraceView* view, _traceViews )
		view->setFocusProxy(_statusBarSearch);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::searchByText(const QString &text) {
	if ( text.isEmpty() ) return;

	QRegExp rx(text + "*");
	rx.setPatternSyntax(QRegExp::Wildcard);
	rx.setCaseSensitivity(Qt::CaseInsensitive);

	TraceView *current = NULL, *found = NULL;

	if ( _tabWidget )
		current = _traceViews[_tabWidget->currentIndex()];
	else
		current = _traceViews[0];

	foreach ( TraceView *view, _traceViews ) {
		view->deselectAllItems();
		int row = view->findByText(0, rx, view == current?_lastFoundRow+1:0);
		if ( row != -1 ) {
			view->setItemSelected(view->itemAt(row), true);
			if ( found == NULL || current == view ) {
				found = view;
				_lastFoundRow = row;
			}
		}
	}

	if ( !found ) {
		QPalette pal = _statusBarSearch->palette();
		pal.setColor(QPalette::Base, _searchError);
		_statusBarSearch->setPalette(pal);
		_lastFoundRow = -1;
	}
	else {
		if ( found != current && _tabWidget )
			_tabWidget->setCurrentIndex(_tabWidget->indexOf(found));

		QPalette pal = _statusBarSearch->palette();
		pal.setColor(QPalette::Base, _searchBase);
		_statusBarSearch->setPalette(pal);

		found->ensureVisible(_lastFoundRow);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::search(const QString &text) {
	_lastFoundRow = -1;

	searchByText(text);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::nextSearch() {
	searchByText(_statusBarSearch->text());
	if ( _lastFoundRow == -1 )
		searchByText(_statusBarSearch->text());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::abortSearch() {
	if ( _statusBarSearch->isVisible() ) {
		_statusBarSearch->setVisible(false);
		_statusBarSearch->releaseKeyboard();

		foreach ( TraceView* view, _traceViews )
			view->setFocusProxy(NULL);
	}
	else {
		foreach ( TraceView* view, _traceViews )
			view->deselectAllItems();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::checkTraceDelay() {
	if ( _maxDelay == 0 ) return;

	Core::Time now = Core::Time::GMT();
	Core::TimeSpan maxDelay = Core::TimeSpan(_maxDelay);
	foreach ( TraceView* view, _traceViews ) {
		for ( int i = 0; i < view->rowCount(); ++i ) {
			Seiscomp::Gui::RecordViewItem* item = view->itemAt(i);
			Seiscomp::Gui::RecordWidget *w = item->widget();
			Seiscomp::Core::Time lastSample;

			if ( w->records() != NULL ) lastSample = w->records()->timeWindow().endTime();
			item->setVisible(now - (!lastSample.valid()?_startTime:lastSample) <= maxDelay);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::cycleFilters(bool) {
	if ( _currentFilterIdx < 0 ) {
		if ( !_filters.empty() )
			_currentFilterIdx = 0;
	}
	else {
		++_currentFilterIdx;
		if ( _currentFilterIdx >= (int)_filters.size() )
			_currentFilterIdx = -1;
	}

	applyFilter();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::cycleFiltersReverse(bool) {
	if ( _currentFilterIdx < 0 ) {
		if ( !_filters.empty() )
			_currentFilterIdx = _filters.size()-1;
	}
	else {
		--_currentFilterIdx;
		if ( _currentFilterIdx < 0 )
			_currentFilterIdx = -1;
	}

	applyFilter();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::applyFilter() {
	if ( _currentFilterIdx >= 0 ) {
		TRACEVIEWS(setFilterByName(_filters[_currentFilterIdx].c_str()));
		TRACEVIEWS(enableFilter(true));
		_statusBarFilter->setText(QString(" Filter ON : %1").arg(_filters[_currentFilterIdx].c_str()));
	}
	else {
		TRACEVIEWS(setFilter(NULL));
		TRACEVIEWS(enableFilter(false));
		_statusBarFilter->setText(" Filter OFF ");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::showScaledValues(bool enable) {
	foreach ( TraceView* view, _traceViews ) {
		for ( int i = 0; i < view->rowCount(); ++i ) {
			Seiscomp::Gui::RecordViewItem* item = view->itemAt(i);
			item->widget()->showScaledValues(enable);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::receivedRecord(Seiscomp::Record* r) {
	RecordPtr rec = r;
	/*
	std::cout << rec->streamID() << ": " << rec->startTime().iso() << " ~ "
	          << rec->endTime().iso() << ", " << (1.0 / rec->samplingFrequency()) << std::endl;
	*/

	foreach( TraceView* view, _traceViews )
		if ( view->feed(rec) ) {
			if ( _lastRecordTime < rec->endTime() )
				_lastRecordTime = rec->endTime();
		}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::selectedTime(Seiscomp::Gui::RecordWidget* widget,
                              Seiscomp::Core::Time time) {
	//widget->setAlignment(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::scrollLineUp() {
	CURRENT_TRACEVIEW(selectPreviousRow());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::scrollLineDown() {
	CURRENT_TRACEVIEW(selectNextRow());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::scrollPageUp() {
	CURRENT_TRACEVIEW(scrollPageUp());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::scrollPageDown() {
	CURRENT_TRACEVIEW(scrollPageDown());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::scrollToTop() {
	CURRENT_TRACEVIEW(selectFirstRow());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::scrollToBottom() {
	CURRENT_TRACEVIEW(selectLastRow());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::scrollLeft() {
	CURRENT_TRACEVIEW(scrollLeft());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::scrollRight() {
	CURRENT_TRACEVIEW(scrollRight());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::alignOriginTime() {
	SEISCOMP_DEBUG("OriginTime = %s", _originTime.iso().c_str());
	TRACEVIEWS(setAlignment(_originTime));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::sortByStationCode() {
	_switchBack->stop();
	TRACEVIEWS(sortByText(0));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::sortByNetworkStationCode() {
	_switchBack->stop();
	TRACEVIEWS(sortByText(1, 0));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::sortByConfig() {
	_switchBack->stop();
	TRACEVIEWS(sortByValue(0));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::sortByDistance() {
	_switchBack->stop();
	TRACEVIEWS(sortByValue(1));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::alignLeft() {
	TRACEVIEWS(setJustification(0));
	TRACEVIEWS(align());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::alignRight() {
	TRACEVIEWS(setJustification(1));
	TRACEVIEWS(align());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::jumpToLastRecord() {
	if ( _lastRecordTime ) {
		SEISCOMP_DEBUG("LastRecordTime = %s", _lastRecordTime.iso().c_str());
		TRACEVIEWS(align());
		TRACEVIEWS(move(_lastRecordTime - _traceViews.front()->alignment()));
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::clearPickMarkers() {
	foreach ( TraceView* view, _traceViews ) {
		for ( int i = 0; i < view->rowCount(); ++i ) {
			Seiscomp::Gui::RecordViewItem *item = view->itemAt(i);
			item->widget()->clearMarker();
			item->widget()->update();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::step() {
	static int stepper = 0;

	if ( _ui.actionToggleAutoMove->isChecked() && !_endTime )
		TRACEVIEWS(setAlignment(Core::Time::GMT()));

	// Check every 10 seconds the traces delay
	if ( stepper % 10 == 0 )
		checkTraceDelay();

	++stepper;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::switchToNormalState() {
	_statusBarFile->setText("Switching back to normal state");
	sortByConfig();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::sortByOrigin(double lat, double lon) {
	foreach (TraceView* view, _traceViews) {
		for ( int i = 0; i < view->rowCount(); ++i ) {
			Seiscomp::Gui::RecordViewItem* item = view->itemAt(i);
			Seiscomp::Client::StationLocation loc = item->data().value<TraceState>().location;
			
			double delta = 0, az1, az2;
	
			Math::Geo::delazi(lat, lon, loc.latitude, loc.longitude,
			                  &delta, &az1, &az2);
	
			item->setValue(1, delta);
		}
	}

	sortByDistance();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::toggledFullScreen(bool fs) {
	if ( menuBar() )
		menuBar()->setVisible(!fs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::messageArrived(Seiscomp::Core::Message* msg, Seiscomp::Communication::NetworkMessage*) {
	Origin* origin = NULL;

	Seiscomp::DataModel::ArtificialOriginMessage* ao = Seiscomp::DataModel::ArtificialOriginMessage::Cast(msg);
	if ( ao && ao->origin() ) {
		_statusBarFile->setText(QString("An artificial origin arrived at %1 (localtime)")
		                         .arg(Seiscomp::Core::Time::LocalTime().toString("%F %T").c_str()));
		origin = ao->origin();
	}

	if ( !origin ) {
		Seiscomp::Core::DataMessage* dm = Seiscomp::Core::DataMessage::Cast(msg);
		if ( dm ) {
			for ( Seiscomp::Core::DataMessage::iterator it = dm->begin(); it != dm->end(); ++it ) {
				origin = Seiscomp::DataModel::Origin::Cast(it->get());
				if ( origin ) {
					_statusBarFile->setText(QString("An preliminary origin has arrived at %1 (localtime)")
					                        .arg(Seiscomp::Core::Time::LocalTime().toString("%F %T").c_str()));
					break;
				}
			}
		}
	}

	if ( !origin ) {
		Gui::CommandMessage *cmd = Gui::CommandMessage::Cast(msg);
		if ( cmd ) {
			if ( cmd->command() == Gui::CM_OBSERVE_LOCATION )
				origin = Seiscomp::DataModel::Origin::Cast(cmd->object());
		}

		if ( origin ) {
			_statusBarFile->setText(QString("An preliminary origin has arrived at %1 (localtime)")
			                        .arg(Seiscomp::Core::Time::LocalTime().toString("%F %T").c_str()));
		}
	}

	if ( origin && _automaticSortEnabled ) {
		sortByOrigin(origin->latitude(), origin->longitude());
	
		try {
			_switchBack->setInterval(Gui::Application::Instance()->configGetInt("autoResetDelay")*1000);
		}
		catch ( ... ) {
			_switchBack->setInterval(1000*60*15);
		}
		_switchBack->start();

		return;
	}

	
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::objectAdded(const QString &parentID,
                             Seiscomp::DataModel::Object* object) {
	if ( _showPicks ) {
		Pick* pick = Pick::Cast(object);
		if ( pick ) {
			SEISCOMP_INFO("about to add a pick to stream %s",
			              waveformIDToString(pick->waveformID()).toStdString().c_str());
			addPick(pick);
			return;
		}
	}

	Origin* origin = Origin::Cast(object);
	if ( origin ) {
		_statusBarFile->setText(QString("An origin arrived at %1 (localtime)")
		                         .arg(Seiscomp::Core::Time::LocalTime().toString("%F %T").c_str()));

		if ( _automaticSortEnabled ) {
			sortByOrigin(origin->latitude(), origin->longitude());

			try {
				_switchBack->setInterval(Gui::Application::Instance()->configGetInt("autoResetDelay")*1000);
			}
			catch ( ... ) {
				_switchBack->setInterval(1000*60*15);
			}

			_switchBack->start();
		}

		return;
	}

	ConfigStation *cs = ConfigStation::Cast(object);
	if ( cs ) {
		if ( SCApp->configModule() && parentID == SCApp->configModule()->publicID().c_str() )
			updateTraces(cs->networkCode(), cs->stationCode(), cs->enabled());
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MainWindow::objectUpdated(const QString &parentID,
                               Seiscomp::DataModel::Object* object) {
	Event *event = Event::Cast(object);
	if ( event ) {
		return;
	}

	ConfigStation *cs = ConfigStation::Cast(object);
	if ( cs ) {
		if ( SCApp->configModule() && parentID == SCApp->configModule()->publicID().c_str() )
			updateTraces(cs->networkCode(), cs->stationCode(), cs->enabled());
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MainWindow::addPick(Pick* pick) {
	// HACK: Z is appended because sent Picks does not have the component code
	// set correctly
	Seiscomp::Gui::RecordViewItem* item = NULL;

	foreach ( TraceView* view, _traceViews ) {
		WaveformStreamID streamID(pick->waveformID());

		item = view->item(streamID);
		if ( !item ) {
			streamID.setChannelCode(streamID.channelCode() + "Z");
			item = view->item(streamID);
		}

		if ( item ) break;
	}

	// No trace found
	if ( item == NULL ) return false;

	// Remove old markers
	for ( int i = 0; i < item->widget()->markerCount(); ++i ) {
		Seiscomp::Gui::RecordMarker* marker = item->widget()->marker(i);
		if ( (double)(marker->time() - _traceViews.front()->alignment()) < -(double)_bufferSize )
			delete marker;
	}

	double age = (double)(pick->time().value() - _traceViews.front()->alignment());
	if ( age <= -(double)_bufferSize ) {
		cout << "pick '"
		     << pick->publicID()
		     << "' is too old ("
		     << pick->time().value().toString("%F %T") << "), "
		     << -age << " sec > "
		     << (double)(_bufferSize)
		     << " sec"
		     << endl;
		return false;
	}

	QString phaseCode;

	try {
		phaseCode = pick->phaseHint().code().c_str();
	}
	catch ( ... ) {}

	Seiscomp::Gui::RecordMarker* marker =
		new Seiscomp::Gui::RecordMarker(item->widget(),
		                                pick->time(),
		                                phaseCode);
	marker->setData(QString(pick->publicID().c_str()));
	marker->setMovable(false);

	try {
		switch ( pick->evaluationMode() ) {
			case AUTOMATIC:
				marker->setColor(SCScheme.colors.picks.automatic);
				break;
			case MANUAL:
				marker->setColor(SCScheme.colors.picks.manual);
				break;
			default:
				marker->setColor(SCScheme.colors.picks.undefined);
				break;
		}
	}
	catch ( ... ) {
		marker->setColor(SCScheme.colors.picks.undefined);
	}

	item->widget()->update();

	return true;
}


}
}
}
