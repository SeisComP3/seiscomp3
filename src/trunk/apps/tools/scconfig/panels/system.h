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


#ifndef __SEISCOMP_CONFIGURATION_GUI_PANEL_SYSTEM_H__
#define __SEISCOMP_CONFIGURATION_GUI_PANEL_SYSTEM_H__

#include "../gui.h"
#include <QtGui>


class SystemPanel : public ConfiguratorPanel {
	Q_OBJECT

	public:
		SystemPanel(QWidget *parent = 0);

	public:
		void setModel(ConfigurationTreeItemModel *model);
		void activated();

	private slots:
		void onContextMenuRequested(const QPoint&);
		void modificationChanged(bool changed);
		void updateModuleState(bool logOutput = true);
		void start();
		void stop();
		void restart();
		void reload();
		void check();
		void enable();
		void disable();
		void updateConfig();
		void readStdout();
		void readStderr();

		void processFinished(int, QProcess::ExitStatus);

	private:
		void runSeiscomp(const QStringList &params);
		void showStartLog(const QString &text);
		void logStdOut(const QByteArray &data);
		void logStdErr(const QByteArray &data);

	private:
		QTableWidget *_procTable;
		QLabel       *_procLabel;
		QTextEdit    *_logWindow;
		QProcess     *_process;
		QToolBar     *_cmdToolBar;
		QAction      *_enable;
		QAction      *_disable;
		QAction      *_updateConfig;
		StatusLabel  *_status;
		QLabel       *_helpLabel;
};


#endif
