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


#ifndef __SEISCOMP_CONFIGURATION_GUI_PANEL_INVENTORY_H__
#define __SEISCOMP_CONFIGURATION_GUI_PANEL_INVENTORY_H__

#include "../gui.h"

#include <QDialog>
#include <QProcess>


class QListView;
class QTreeView;
class QDirModel;
class QSortFilterProxyModel;
class QTextEdit;
class QLineEdit;
class QPushButton;
class QComboBox;

class LogDialog;
class StatusPanel;


class ProcessWidget : public QDialog {
	Q_OBJECT

	public:
		ProcessWidget(QWidget *parent = 0);
		~ProcessWidget();

		int start(const QString &cmd, const QStringList &params);
		void done(int r);


	private slots:
		void started();
		void error(QProcess::ProcessError error);

		void readStderr();
		void readStdout();
		void processFinished(int, QProcess::ExitStatus);


	private:
		QProcess    *_process;
		LogDialog   *_logWindow;
		QPushButton *_btnOK;
		QPushButton *_btnStop;
		StatusPanel *_status;
		int          _exitCode;
};


class ImportDialog : public QDialog {
	Q_OBJECT

	public:
		ImportDialog(const QStringList &formats, QWidget *parent = 0);

		QString format() const;
		QString source() const;

	private slots:
		void openFileDialog();

	private:
		QComboBox *_formats;
		QLineEdit *_source;
};


class InventoryPanel : public ConfiguratorPanel {
	Q_OBJECT

	public:
		InventoryPanel(QWidget *parent = 0);


	private:
		int runProc(const QString &cmd, const QStringList &params);
		int runSCProc(const QString &cmd, const QStringList &params);
		int runSC(const QStringList &params);


	private slots:
		void headerSectionClicked(int);

		void switchToIconView();
		void switchToListView();
		void switchToDetailedView();
		void refresh();
		void deleteFiles();
		void inspectFile();

		void import();
		void testSync();
		void sync();
		void syncKeys();


	private:
		QListView             *_folderView;
		QTreeView             *_folderTree;
		QDirModel             *_folderModel;
		QItemSelectionModel   *_selectionModel;
};


#endif
