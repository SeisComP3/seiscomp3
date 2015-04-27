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


#include "inventory.h"
#include "inspector.h"

#include <seiscomp3/system/environment.h>
#include <seiscomp3/io/archive/xmlarchive.h>

#include <QtGui>


/** The code to parse VT100 escape sequences is taken from KSysGuard,
    KTextEditVT.cpp.

    KSysGuard, the KDE System Guard

    Copyright (C) 2007 Trent Waddington <trent.waddington@gmail.com>
    Copyright (c) 2008 John Tapsell <tapsell@kde.org>
*/
class LogDialog : public QTextEdit {
	public:
		LogDialog(QWidget * parent = 0) : QTextEdit(parent) {
			escape_sequence = false;
			escape_CSI = false;
			escape_OSC = false;
			escape_number1 = -1;
			escape_number_seperator = false;
			escape_number2 = -1;
			escape_code = 0;
		}

		LogDialog(const QString &text, QWidget * parent = 0)
		: QTextEdit(text, parent) {
			escape_sequence = false;
			escape_CSI = false;
			escape_OSC = false;
			escape_number1 = -1;
			escape_number_seperator = false;
			escape_number2 = -1;
			escape_code = 0;
		}


	public:
		void insertVTText(const QByteArray &string) {
			int size = string.size();
			for ( int i = 0; i < size; ++i )
				insertVTChar(QChar(string.at(i)));
		}

		void insertVTText(const QString &string) {
			int size = string.size();
			for ( int i =0; i < size; ++i )
				insertVTChar(string.at(i));
		}


	private:
		void insertVTChar(const QChar & c) {
			if ( escape_sequence ) {
				if ( escape_CSI || escape_OSC ) {
					if ( c.isDigit() ) {
						if ( !escape_number_seperator ) {
							if ( escape_number1 == -1 )
								escape_number1 = c.digitValue();
							else
								escape_number1 = escape_number1*10 + c.digitValue();
						}
						else {
							if ( escape_number2 == -1 )
								escape_number2 = c.digitValue();
							else
								escape_number2 = escape_number2*10 + c.digitValue();

						}
					}
					else if ( c == ';' ) {
						escape_number_seperator = true;
					}
					else if ( escape_OSC && c==7 ) {
						//Throw away any letters that are not OSC
						escape_code = c;
					}
					else if ( escape_CSI )
						escape_code = c;
				}
				else if ( c=='[' )
					escape_CSI = true;
				else if ( c==']' )
					escape_OSC = true;
				else if ( c=='(' || c==')' ) {}
				else
					escape_code = c;

				if ( !escape_code.isNull() ) {
					//We've read in the whole escape sequence.  Now parse it
					if ( escape_code == 'm' ) { // change color
						switch ( escape_number2 ) {
							case 0: //all off
								setFontWeight(QFont::Normal);
								setTextColor(Qt::black);
								break;
							case 1: //bold
								setFontWeight(QFont::Bold);
								break;
							case 31: //red
								setTextColor(Qt::red);
								break;
							case 32: //green
								setTextColor(Qt::green);
								break;
							case 33: //yellow
								setTextColor(Qt::yellow);
								break;
							case 34: //blue
								setTextColor(Qt::blue);
								break;
							case 35: //magenta
								setTextColor(Qt::magenta);
								break;
							case 36: //cyan
								setTextColor(Qt::cyan);
								break;
							case -1:
							case 30: //black
							case 39: //reset
							case 37: //white
								setTextColor(Qt::black);
								break;
						}
					}

					escape_code = 0;
					escape_number1 = -1;
					escape_number2 = -1;
					escape_CSI = false;
					escape_OSC = false;
					escape_sequence = false;
					escape_number_seperator = false;
				}
			}
			else if ( c == 0x0d )
				insertPlainText(QChar('\n'));
			else if ( c.isPrint() || c == '\n' )
				insertPlainText(QChar(c));
			else if ( true /* parse ansi */ ) {
				if ( c == 127 || c == 8 )
					// delete or backspace, respectively
					textCursor().deletePreviousChar();
				else if ( c == 27 )
					// escape key
					escape_sequence = true;
				else if ( c == 0x9b ) {
					// CSI - equivalent to esc [
					escape_sequence = true;
					escape_CSI = true;
				}
				else if ( c == 0x9d ) {
					// OSC - equivalent to esc ]
					escape_sequence = true;
					escape_OSC = true;
				}

			}
			else if ( !c.isNull() ) {
				insertPlainText("[");
				QByteArray num;
				num.setNum(c.toAscii());
				insertPlainText(num);
				insertPlainText("]");
			}
		}


	private:
		bool  escape_sequence;
		bool  escape_CSI;
		bool  escape_OSC;
		int   escape_number1;
		int   escape_number2;
		bool  escape_number_seperator;
		QChar escape_code;
};


class StatusPanel : public QWidget {
	public:
		StatusPanel(QWidget *parent = NULL) : QWidget(parent) {
			setAutoFillBackground(true);

			QPalette pal = palette();
			pal.setColor(QPalette::WindowText, Qt::black);
			setPalette(pal);

			_label = new QLabel;

			QVBoxLayout *l = new QVBoxLayout;
			setLayout(l);

			l->addWidget(_label);
		}

	public:
		void setError(const QString &msg) {
			QPalette pal = palette();
			pal.setColor(QPalette::Window, QColor(255,192,192));
			_label->setText(msg);
			setPalette(pal);
		}


		void setStatus(int exitCode, QProcess::ExitStatus exitStatus) {
			QPalette pal = palette();

			switch ( exitStatus ) {
				default:
				case QProcess::NormalExit:
					if ( exitCode == 0 ) {
						pal.setColor(QPalette::Window, QColor(192,255,192));
						_label->setText(tr("Program exited normally"));
					}
					else {
						pal.setColor(QPalette::Window, QColor(255,255,192));
						_label->setText(
							QString(tr("Program exited normally with code %1"))
							.arg(exitCode)
						);
					}
					break;

				case QProcess::CrashExit:
					pal.setColor(QPalette::Window, QColor(255,192,192));
					_label->setText(tr("Program crashed"));
					break;
			}

			setPalette(pal);
		}

	private:
		QLabel *_label;
};


ProcessWidget::ProcessWidget(QWidget *parent) : QDialog(parent) {
	// Create widgets
	_btnOK = new QPushButton;
	_btnOK->setText(tr("OK"));
	connect(_btnOK, SIGNAL(clicked()), this, SLOT(accept()));

	_btnStop = new QPushButton;
	_btnStop->setText(tr("Stop"));
	_btnStop->setEnabled(false);

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->addStretch();
	buttonLayout->addWidget(_btnOK);
	buttonLayout->addWidget(_btnStop);

	_logWindow = new LogDialog;
	_logWindow->setAutoFillBackground(true);
	_logWindow->setReadOnly(true);
	_logWindow->setWordWrapMode(QTextOption::NoWrap);
	//_logWindow->setFrameShape(QFrame::NoFrame);

	_status = new StatusPanel;
	_status->hide();

	QVBoxLayout *layout = new QVBoxLayout;
	layout->addWidget(_logWindow);
	layout->addWidget(_status);
	layout->addLayout(buttonLayout);

	setLayout(layout);

	resize(500, 600);

	_process = NULL;
}


ProcessWidget::~ProcessWidget() {
	if ( _process ) {
		delete _process;
		_process = NULL;
	}
}


int ProcessWidget::start(const QString &cmd, const QStringList &params) {
	_exitCode = -1;

	if ( _process != NULL ) return _exitCode;

	_status->hide();

	_process = new QProcess(this);

	connect(_process, SIGNAL(started()), this, SLOT(started()));
	connect(_process, SIGNAL(error(QProcess::ProcessError)),
	        this, SLOT(error(QProcess::ProcessError)));

	connect(_process, SIGNAL(readyReadStandardError()),
	        this, SLOT(readStderr()));
	connect(_process, SIGNAL(readyReadStandardOutput()),
	        this, SLOT(readStdout()));
	connect(_process, SIGNAL(finished(int, QProcess::ExitStatus)),
	        this, SLOT(processFinished(int,QProcess::ExitStatus)));

	connect(_btnStop, SIGNAL(clicked()), _process, SLOT(terminate()));

	_btnOK->setEnabled(false);
	_btnStop->setEnabled(true);

	_process->start(cmd, params, QProcess::Unbuffered | QProcess::ReadWrite);

	setWindowTitle(cmd);
	exec();

	return _exitCode;
}


void ProcessWidget::done(int r) {
	if ( _process ) {
		if ( _process->state() != QProcess::NotRunning ) {
			_process->terminate();
			if ( !_process->waitForFinished() ) {
				_process->kill();
				_process->waitForFinished();
			}
		}
		delete _process;
		_process = NULL;
	}

	QDialog::done(r);
}


void ProcessWidget::started() {
	//
}


void ProcessWidget::error(QProcess::ProcessError) {
	_btnOK->setEnabled(true);
	_btnStop->setEnabled(false);

	_status->setError(tr("Program failed to start"));
	_status->show();
}


void ProcessWidget::readStderr() {
	_logWindow->setTextColor(Qt::darkGray);
	_logWindow->insertPlainText(_process->readAllStandardError());
}


void ProcessWidget::readStdout() {
	_logWindow->setTextColor(Qt::black);
	_logWindow->insertPlainText(_process->readAllStandardOutput());
}


void ProcessWidget::processFinished(int exitCode, QProcess::ExitStatus exitStatus) {
	if ( exitStatus == QProcess::NormalExit )
		_exitCode = exitCode;

	_btnOK->setEnabled(true);
	_btnStop->setEnabled(false);

	_status->setStatus(exitCode, exitStatus);
	_status->show();
}


ImportDialog::ImportDialog(const QStringList &formats, QWidget *parent) : QDialog(parent) {
	setWindowTitle("Import");

	QVBoxLayout *layout = new QVBoxLayout;
	setLayout(layout);

	setWhatsThis(tr(
		"Import inventory data involves converting the imported format "
		"to the native format SeisComP3 can handle (SC3 inventory XML). "
		"Furthermore synchronization is required for populating the "
		"database with the imported inventory data. The last step is to "
		"create binding for new stations."
	));

	QGridLayout *grid = new QGridLayout;

	QLabel *formatLabel = new QLabel(tr("Format:"));
	_formats = new QComboBox;
	_formats->addItems(formats);
	_formats->setWhatsThis(tr("Select among available formats the format for "
	                          "the data you are going to import."));
	grid->addWidget(formatLabel, 0, 0);
	grid->addWidget(_formats, 0, 1, 1, 2);

	QLabel *sourceLabel = new QLabel(tr("Source:"));
	_source = new QLineEdit(this);
	_source->setWhatsThis(tr("The data source used to import inventory data. This "
	                         "can be either file or a directory or a URL depending "
	                         "on the selected format (and thus the converter being "
	                         "used)."));

	QToolButton *openFD = new QToolButton;
	openFD->setText(tr("..."));

	grid->addWidget(sourceLabel, 1, 0);
	grid->addWidget(_source, 1, 1);
	grid->addWidget(openFD, 1, 2);

	connect(openFD, SIGNAL(clicked()), this, SLOT(openFileDialog()));

	layout->addLayout(grid);

	layout->addStretch();

	QHBoxLayout *hl = new QHBoxLayout;

	QPushButton *ok = new QPushButton;
	ok->setText(tr("OK"));
	connect(ok, SIGNAL(clicked()), this, SLOT(accept()));

	QPushButton *cancel = new QPushButton;
	cancel->setText(tr("Cancel"));
	connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));

	hl->addStretch();
	hl->addWidget(ok);
	hl->addWidget(cancel);

	layout->addLayout(hl);
}


QString ImportDialog::format() const {
	return _formats->currentText();
}


QString ImportDialog::source() const {
	return _source->text();
}


void ImportDialog::openFileDialog() {
	QFileDialog dialog(this);
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setAcceptMode(QFileDialog::AcceptOpen);

	if ( dialog.exec() ) {
		QStringList fileNames = dialog.selectedFiles();
		if ( fileNames.empty() ) return;
		_source->setText(fileNames.front());
	}
}


InventoryPanel::InventoryPanel(QWidget *parent)
: ConfiguratorPanel(false, parent) {
	_name = "Inventory";
	_icon = QIcon(":/res/icons/inventory.png");
	setHeadline("Inventory");
	setDescription("Shows available inventory files and provides options to "
	               "import inventory data also from other formats via import_inv.");

	Seiscomp::Environment *env = Seiscomp::Environment::Instance();
	QString inventoryDir = QDir::toNativeSeparators((env->installDir() + "/etc/inventory").c_str());
	QDir invDir(inventoryDir);
	if ( !invDir.exists() ) {
		if ( !invDir.mkpath(".") ) {
			QMessageBox::warning(NULL, "Path missing",
			                     "Inventory folder does not exists and\n"
			                     "creation failed.");
			return;
		}
	}

	QVBoxLayout *l = new QVBoxLayout;
	l->setMargin(0);
	l->setSpacing(0);
	setLayout(l);

	QToolBar *folderViewTools = new QToolBar;
	folderViewTools->setIconSize(QSize(24,24));
	folderViewTools->setAutoFillBackground(true);
	folderViewTools->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

	QAction *deleteFileAction = new QAction(tr("Delete"), this);
	connect(deleteFileAction, SIGNAL(triggered()), this, SLOT(deleteFiles()));

	QAction *inspectFileAction = new QAction(tr("Show content"), this);
	connect(inspectFileAction, SIGNAL(triggered()), this, SLOT(inspectFile()));

	_folderView = new QListView;
	_folderView->setFrameShape(QFrame::NoFrame);
	_folderView->setResizeMode(QListView::Adjust);
	_folderView->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_folderView->setContextMenuPolicy(Qt::ActionsContextMenu);
	_folderView->addAction(inspectFileAction);
	_folderView->addAction(deleteFileAction);

	_folderTree = new QTreeView;
	_folderTree->setAutoFillBackground(true);
	_folderTree->setFrameShape(QFrame::NoFrame);
	_folderTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_folderTree->setSortingEnabled(true);
	_folderTree->setContextMenuPolicy(Qt::ActionsContextMenu);
	_folderTree->addAction(inspectFileAction);
	_folderTree->addAction(deleteFileAction);

	_folderModel = new QDirModel(this);
	_folderModel->setReadOnly(false);
	_folderModel->setFilter(QDir::Files);

	connect(_folderTree->header(), SIGNAL(sectionClicked(int)),
	        this, SLOT(headerSectionClicked(int)));

	QAction *a = folderViewTools->addAction("Icons");
	a->setIcon(style()->standardIcon(QStyle::SP_FileDialogContentsView));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(switchToIconView()));

	a = folderViewTools->addAction("List");
	a->setIcon(style()->standardIcon(QStyle::SP_FileDialogListView));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(switchToListView()));

	a = folderViewTools->addAction("Details");
	a->setIcon(style()->standardIcon(QStyle::SP_FileDialogDetailedView));
	connect(a, SIGNAL(triggered(bool)), this, SLOT(switchToDetailedView()));

	a = folderViewTools->addAction("Refresh");
#if QT_VERSION >= 0x040400
	a->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
#endif
	connect(a, SIGNAL(triggered(bool)), this, SLOT(refresh()));

	QFileSystemWatcher *monitor = new QFileSystemWatcher(this);
	monitor->addPath(inventoryDir);
	connect(monitor, SIGNAL(directoryChanged(QString)), this, SLOT(refresh()));
	connect(monitor, SIGNAL(fileChanged(QString)), this, SLOT(refresh()));

	folderViewTools->addSeparator();

	a = folderViewTools->addAction("Import");
	connect(a, SIGNAL(triggered(bool)), this, SLOT(import()));

	folderViewTools->addSeparator();

	a = folderViewTools->addAction("Test sync");
	connect(a, SIGNAL(triggered(bool)), this, SLOT(testSync()));
	a = folderViewTools->addAction("Sync");
	connect(a, SIGNAL(triggered(bool)), this, SLOT(sync()));
	a = folderViewTools->addAction("Sync keys");
	connect(a, SIGNAL(triggered(bool)), this, SLOT(syncKeys()));

	l->addWidget(folderViewTools);
	l->addWidget(_folderView);
	l->addWidget(_folderTree);

	_folderView->setModel(_folderModel);
	_folderTree->setModel(_folderModel);

	_selectionModel = new QItemSelectionModel(_folderModel, this);

	_folderTree->setSelectionModel(_selectionModel);
	_folderView->setSelectionModel(_selectionModel);

	QModelIndex root = _folderModel->index(inventoryDir);

	_folderView->setRootIndex(root);
	_folderTree->setRootIndex(root);

	switchToDetailedView();
}


int InventoryPanel::runProc(const QString &cmd, const QStringList &params) {
	ProcessWidget proc;
	return proc.start(cmd, params);
}


int InventoryPanel::runSCProc(const QString &cmd, const QStringList &params) {
	QStringList sc_params;
	sc_params << "exec" << cmd << params;
	return runSC(sc_params);
}


int InventoryPanel::runSC(const QStringList &params) {
	Seiscomp::Environment *env = Seiscomp::Environment::Instance();
	QString cmd = QString("%1%2")
	              .arg(env->installDir().c_str()).arg("/bin/seiscomp");
	return runProc(cmd, params);
}


void InventoryPanel::headerSectionClicked(int logicalIndex) {
	Seiscomp::Environment *env = Seiscomp::Environment::Instance();
	QString inventoryDir = QDir::toNativeSeparators((env->installDir() + "/etc/inventory").c_str());

	QModelIndex root = _folderModel->index(inventoryDir);

	_folderView->setRootIndex(root);
	_folderTree->setRootIndex(root);
}


void InventoryPanel::switchToIconView() {
	_folderTree->hide();
	_folderView->show();
	_folderView->setViewMode(QListView::IconMode);
	_folderView->setGridSize(QSize(64,64));
}


void InventoryPanel::switchToListView() {
	_folderTree->hide();
	_folderView->show();
	_folderView->setViewMode(QListView::ListMode);
	_folderView->setGridSize(QSize());
	_folderView->setSpacing(0);
}


void InventoryPanel::switchToDetailedView() {
	_folderView->hide();
	_folderTree->show();
}


void InventoryPanel::refresh() {
	Seiscomp::Environment *env = Seiscomp::Environment::Instance();
	QString rootDir = QDir::toNativeSeparators((env->installDir() + "/etc/inventory").c_str());
	QModelIndex root = _folderModel->index(rootDir, 0);
	_folderModel->refresh(root);
}


void InventoryPanel::deleteFiles() {
	QModelIndexList indexes;
	indexes = _selectionModel->selectedIndexes();

	if ( indexes.isEmpty() ) return;

	if ( QMessageBox::question(NULL, tr("Delete"),
	           tr("Do you really want to delete all selected files?"),
	           QMessageBox::Yes | QMessageBox::No
	     ) != QMessageBox::Yes )
		return;

	QList<QPersistentModelIndex> toBeDeleted;
	foreach ( const QModelIndex &i, indexes )
		toBeDeleted.append(/*_sortModel->mapToSource(i)*/i);

	foreach ( const QPersistentModelIndex &i, toBeDeleted )
		if ( i.isValid() ) _folderModel->remove(i);
}


void InventoryPanel::inspectFile() {
	QModelIndexList indexes;
	indexes = _selectionModel->selectedRows();

	if ( indexes.isEmpty() ) return;

	if ( indexes.count() > 1 ) {
		QMessageBox::critical(NULL, tr("Show content"),
		                      tr("More than one file selected"));
		return;
	}

	Seiscomp::Environment *env = Seiscomp::Environment::Instance();
	QString target = (env->installDir() + "/etc/inventory").c_str();
	target = QDir::toNativeSeparators(target + "/" + indexes[0].data().toString());

	Seiscomp::IO::XMLArchive ar;
	if ( !ar.open(target.toAscii()) ) {
		QMessageBox::critical(NULL, tr("Show content"),
		                      tr("Could not open file"));
		return;
	}

	Seiscomp::Core::BaseObjectPtr obj;
	ar >> obj;
	ar.close();

	if ( obj == NULL ) {
		QMessageBox::critical(NULL, tr("Show content"),
		                      tr("Empty file"));
		return;
	}

	Inspector dlg;
	dlg.setObject(obj.get());
	dlg.exec();
}


void InventoryPanel::import() {
	QProcess proc;
	proc.start("import_inv", QStringList() << "help" << "formats", QProcess::Unbuffered | QProcess::ReadWrite);

	if ( !proc.waitForFinished(10000) ) {
		proc.terminate();
		if ( !proc.waitForFinished(10000) )
			proc.kill();
	}

	QStringList formats;

	if ( proc.exitStatus() == QProcess::NormalExit &&
	     proc.exitCode() == 0 ) {
		QString stdout = proc.readAllStandardOutput().trimmed().constData();
		formats = stdout.split('\n');
	}
	else {
		QMessageBox::critical(NULL, "Query formats",
		                      "Could not query for supported formats.\n"
		                      "Is import_inv installed and working?\n"
		                      "Hint: try to run 'import_inv help formats' manually.");
		return;
	}

	if ( formats.isEmpty() ) {
		QMessageBox::information(NULL, "Import formats",
		                         "No importers available. Nothing to do.");
		return;
	}

	ImportDialog dlg(formats);
	if ( dlg.exec() != QDialog::Accepted ) return;

	QString format = dlg.format();
	QString source = dlg.source();

	if ( format.isEmpty() ) {
		QMessageBox::critical(NULL, "Import inventory",
		                      "No format specified. Nothing to import.");
		return;
	}

	if ( source.isEmpty() ) {
		QMessageBox::critical(NULL, "Import inventory",
		                      "Source location is not set. Nothing to import.");
		return;
	}

	runSCProc("import_inv", QStringList() << format << source);
	refresh();
}


void InventoryPanel::testSync() {
	runSCProc("scinv", QStringList() << "sync" << "--test");
}


void InventoryPanel::sync() {
	if ( QMessageBox::question(NULL, "Sync",
	           "Synchronization will modify the database.\n"
	           "Unless an error is raised the current unsaved bindings "
	           "configuration will be lost. Make sure that you have saved "
	           "all changes.\n"
	           "Do you want to continue?",
	           QMessageBox::Yes | QMessageBox::No
	     ) != QMessageBox::Yes )
		return;

	if ( runSCProc("scinv", QStringList() << "sync") == 0 )
		emit reloadRequested();
}


void InventoryPanel::syncKeys() {
	if ( QMessageBox::question(NULL, "Sync keys",
	           "Unless an error is raised the current unsaved bindings "
	           "configuration will be lost. Make sure that you have saved "
	           "all changes.\n"
	           "Do you want to continue?",
	           QMessageBox::Yes | QMessageBox::No
	     ) != QMessageBox::Yes )
		return;

	if ( runSCProc("scinv", QStringList() << "keys") == 0 )
		emit reloadRequested();
}
