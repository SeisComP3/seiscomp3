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


#include "system.h"
#include <seiscomp3/system/environment.h>

#include <iostream>


using namespace std;


SystemPanel::SystemPanel(QWidget *parent)
: ConfiguratorPanel(false, parent) {
	QPalette pal;

	_process = NULL;

	_name = "System";
	_icon = QIcon(":/res/icons/system.png");
	setHeadline("System");
	setDescription("The current status of the system");

	QVBoxLayout *l = new QVBoxLayout;
	l->setMargin(0);
	l->setSpacing(1);
	setLayout(l);

	_status = new StatusLabel;
	_status->hide();
	l->addWidget(_status);

	_cmdToolBar = new QToolBar;
	_cmdToolBar->setAutoFillBackground(true);
	_cmdToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
	l->addWidget(_cmdToolBar);

	_helpLabel = new QLabel;
	pal = _helpLabel->palette();
	pal.setColor(QPalette::Window, QColor(255,255,204));
	_helpLabel->setPalette(pal);
	_helpLabel->setWordWrap(true);
	_helpLabel->setAutoFillBackground(true);
	_helpLabel->setMargin(4);
	//_helpLabel->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
	_helpLabel->setText(
	            tr("All commands (such as 'start', 'stop') will affect all "
	               "modules which rows are currently selected. If no row is "
	               "selected, all modules are affected. You can clear the row "
	               "selection with ESC.")
	);

	l->addWidget(_helpLabel);

	QAction *a = _cmdToolBar->addAction("Update");
#if QT_VERSION >= 0x040400
	a->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
#endif
	connect(a, SIGNAL(triggered(bool)), this, SLOT(updateModuleState()));

	_cmdToolBar->addSeparator();
	//_cmdToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

	_start = _cmdToolBar->addAction("Start");
#if QT_VERSION >= 0x040400
	_start->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
#endif
	connect(_start, SIGNAL(triggered(bool)), this, SLOT(start()));

	_stop = _cmdToolBar->addAction("Stop");
#if QT_VERSION >= 0x040400
	_stop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
#endif
	connect(_stop, SIGNAL(triggered(bool)), this, SLOT(stop()));

	_restart = _cmdToolBar->addAction("Restart");
#if QT_VERSION >= 0x040400
	_restart->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
#endif
	connect(_restart, SIGNAL(triggered(bool)), this, SLOT(restart()));

	_check = _cmdToolBar->addAction("Check");
#if QT_VERSION >= 0x040400
	_check->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
#endif
	connect(_check, SIGNAL(triggered(bool)), this, SLOT(check()));

	_cmdToolBar->addSeparator();

	_enable = _cmdToolBar->addAction("Enable module(s)");
	_enable->setIcon(QIcon(":/res/icons/autostart_add.png"));
	connect(_enable, SIGNAL(triggered(bool)), this, SLOT(enable()));

	_disable = _cmdToolBar->addAction("Disable module(s)");
	_disable->setIcon(QIcon(":/res/icons/autostart_remove.png"));
	connect(_disable, SIGNAL(triggered(bool)), this, SLOT(disable()));

	_cmdToolBar->addSeparator();

	_updateConfig = _cmdToolBar->addAction("Update configuration");
	_updateConfig->setIcon(QIcon(":/res/icons/update-config.png"));
	connect(_updateConfig, SIGNAL(triggered(bool)), this, SLOT(updateConfig()));

	QSplitter *splitter = new QSplitter(Qt::Horizontal);
	splitter->setHandleWidth(1);

	_procTable = new QTableWidget;
	_procTable->setAutoFillBackground(true);
	_procTable->setFrameShape(QFrame::NoFrame);
	_procTable->setColumnCount(3);
	_procTable->verticalHeader()->setVisible(false);
	_procTable->horizontalHeader()->setStretchLastSection(true);
	_procTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
	_procTable->setHorizontalHeaderLabels(QStringList() << "Auto" << "Module" << "Status");
	_procTable->setAlternatingRowColors(true);
	_procTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	_procTable->setSortingEnabled(true);

	QSizePolicy sp = _procTable->sizePolicy();
	sp.setVerticalStretch(1);
	_procTable->setSizePolicy(sp);

	_logWindow = new QTextEdit;
	_logWindow->setAutoFillBackground(true);
	_logWindow->setReadOnly(true);
	_logWindow->setFrameShape(QFrame::NoFrame);
	/*
	QPalette pal = _logWindow->palette();
	pal.setColor(QPalette::Base, QColor(64,64,64));
	pal.setColor(QPalette::Text, Qt::white);
	_logWindow->setPalette(pal);
	*/

	QWidget *container = new QWidget;
	QVBoxLayout *cl = new QVBoxLayout;
	cl->setMargin(0);
	cl->setSpacing(0);
	container->setLayout(cl);
	_procLabel = new QLabel;
	_procLabel->setAutoFillBackground(true);
	_procLabel->setMargin(4);
	_procLabel->setText("Idle");
	cl->addWidget(_procLabel);
	cl->addWidget(_logWindow);

	splitter->addWidget(_procTable);
	splitter->addWidget(container);

	sp = splitter->sizePolicy();
	sp.setVerticalStretch(2);
	splitter->setSizePolicy(sp);
	l->addWidget(splitter);

	QAction *clearSelection = new QAction(this);
	clearSelection->setShortcut(QKeySequence("Escape"));
	connect(clearSelection, SIGNAL(triggered()), _procTable->selectionModel(), SLOT(clearSelection()));
	addAction(clearSelection);
}


void SystemPanel::setModel(ConfigurationTreeItemModel *model) {
	if ( _model ) _model->disconnect(this);

	ConfiguratorPanel::setModel(model);

	connect(_model, SIGNAL(modificationChanged(bool)),
	        this, SLOT(modificationChanged(bool)));
}


void SystemPanel::activated() {
	modificationChanged(_model->isModified());
	updateModuleState();
}


void SystemPanel::modificationChanged(bool changed) {
	if ( changed ) {
		_status->setWarningText(
			tr("The configuration is changed but has not been saved yet. It must "
		       "be saved to run all configuration commands as expected.")
		);

		_status->show();
	}
	else
		_status->hide();
}


void SystemPanel::updateModuleState(bool logOutput) {
	setCursor(Qt::WaitCursor);

	Seiscomp::Environment *env = Seiscomp::Environment::Instance();
	QProcess seiscomp;
	seiscomp.start(QString("%1%2")
	               .arg(env->installDir().c_str()).arg("/bin/seiscomp"),
	               QStringList() << "--csv" << "status");

	if ( !seiscomp.waitForStarted() ) {
		unsetCursor();
		return;
	}

	_procLabel->setText("Running");

	if ( !seiscomp.waitForFinished() ) {
		unsetCursor();
		return;
	}

	_procLabel->setText("Idle");

	for ( int i = 0; i < _procTable->rowCount(); ++i ) {
		QTableWidgetItem *item = _procTable->item(i, 1);
		item->setData(Qt::UserRole, 0);
	}

	_procTable->setSortingEnabled(false);

	QByteArray stdout = seiscomp.readAllStandardOutput();
	QList<QByteArray> output = stdout.split('\n');
	QList<QByteArray>::iterator it;
	for ( it = output.begin(); it != output.end(); ++it ) {
		if ( it->isEmpty() ) continue;
		QStringList toks = QString(it->constData()).split(';');
		if ( toks.count() < 4 ) continue;

		int row = -1;
		for ( int i = 0; i < _procTable->rowCount(); ++i ) {
			QTableWidgetItem *item = _procTable->item(i, 1);
			if ( item->text() == toks[0] ) {
				row = i;
				break;
			}
		}

		QTableWidgetItem *active;
		QTableWidgetItem *state;

		if ( row == -1 ) {
			QTableWidgetItem *name = new QTableWidgetItem(toks[0]);
			name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			QFont f = name->font();
			f.setBold(true);
			name->setFont(f);

			state = new QTableWidgetItem;
			state->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			active = new QTableWidgetItem;
			active ->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			active->setFont(f);

			row = _procTable->rowCount();
			_procTable->insertRow(row);
			_procTable->setItem(row, 0, active);
			_procTable->setItem(row, 1, name);
			_procTable->setItem(row, 2, state);
		}
		else {
			active = _procTable->item(row, 0);
			state = _procTable->item(row, 2);
		}

		bool ok;
		int s = toks[1].toInt(&ok);
		if ( !ok ) s = -1;
		int f = toks[2].toInt(&ok);
		if ( !ok ) f = -1;
		int a = toks[3].toInt(&ok);
		if ( !ok ) a = -1;

		switch ( s ) {
			case -1:
				state->setBackground(Qt::gray);
				state->setText("undefined");
				break;
			case 0:
				state->setText("not running");
				if ( f == 1 )
					state->setBackground(QColor(255,192,192));
				else
					state->setBackground(Qt::NoBrush);
				break;
			default:
				state->setBackground(QColor(192,255,192));
				state->setText("running");
				break;
		}

		switch ( a ) {
			case 0:
				active->setText("Off");
				active->setForeground(Qt::darkRed);
				break;
			case 1:
				active->setText("On");
				active->setForeground(Qt::darkGreen);
				break;
			default:
				active->setText("-");
				active->setForeground(Qt::NoBrush);
				break;
		}

		// Set updated flag
		state->setData(Qt::UserRole, 1);
	}

	for ( int i = 0; i < _procTable->rowCount(); ) {
		QTableWidgetItem *item = _procTable->item(i, 2);
		if ( item->data(Qt::UserRole).toInt() != 1 )
			_procTable->removeRow(i);
		else
			++i;
	}

	if ( logOutput ) {
		_logWindow->clear();
		logStdOut(stdout);
		logStdErr(seiscomp.readAllStandardError());
	}

	_procTable->setSortingEnabled(true);
	_procTable->sortByColumn(_procTable->horizontalHeader()->sortIndicatorSection(),
	                         _procTable->horizontalHeader()->sortIndicatorOrder());

	unsetCursor();
}


void SystemPanel::logStdOut(const QByteArray &data) {
	_logWindow->setTextColor(palette().color(QPalette::Text));
	_logWindow->insertPlainText(data.constData());
}


void SystemPanel::logStdErr(const QByteArray &data) {
	_logWindow->setTextColor(QColor(128,92,0));
	_logWindow->insertPlainText(data.constData());
}


void SystemPanel::start() {
	runSeiscomp(QStringList() << "start");
}


void SystemPanel::stop() {
	runSeiscomp(QStringList() << "stop");
}


void SystemPanel::restart() {
	runSeiscomp(QStringList() << "restart");
}


void SystemPanel::check() {
	runSeiscomp(QStringList() << "check");
}


void SystemPanel::enable() {
	runSeiscomp(QStringList() << "enable");
}


void SystemPanel::disable() {
	runSeiscomp(QStringList() << "disable");
}


void SystemPanel::updateConfig() {
	runSeiscomp(QStringList() << "update-config");
}


void SystemPanel::runSeiscomp(const QStringList &params_) {
	if ( _process ) return;

	QStringList params = params_;
	QList<QTableWidgetItem*> items = _procTable->selectedItems();
	foreach ( QTableWidgetItem *item, items ) {
		if ( item->column() != 1 ) continue;
		params.append(item->text());
	}

	_logWindow->clear();
	_logWindow->setTextColor(palette().color(QPalette::Text));

	Seiscomp::Environment *env = Seiscomp::Environment::Instance();
	_process = new QProcess;
	connect(_process, SIGNAL(readyReadStandardError()),
	        this, SLOT(readStderr()));
	connect(_process, SIGNAL(readyReadStandardOutput()),
	        this, SLOT(readStdout()));
	connect(_process, SIGNAL(finished(int, QProcess::ExitStatus)),
	        this, SLOT(processFinished(int,QProcess::ExitStatus)));

	QString cmd = QString("%1%2")
	              .arg(env->installDir().c_str()).arg("/bin/seiscomp");
	_process->start(cmd, params, QIODevice::ReadWrite | QIODevice::Unbuffered);

	_logWindow->insertPlainText(QString("$ seiscomp %1\n").arg(params.join(" ")));

	if ( !_process->waitForStarted() )
		return;

	_procLabel->setText("Running");
	_cmdToolBar->setEnabled(false);
}


void SystemPanel::readStdout() {
	logStdOut(_process->readAllStandardOutput());
}


void SystemPanel::readStderr() {
	logStdErr(_process->readAllStandardError());
}


void SystemPanel::processFinished(int code, QProcess::ExitStatus status) {
	delete _process;
	_process = NULL;
	_cmdToolBar->setEnabled(true);
	_procLabel->setText("Idle");

	updateModuleState(false);
}
