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




#ifndef __MAINFRAME_H__
#define __MAINFRAME_H__

#define SEISCOMP_COMPONENT MessageMonitor

#include <seiscomp3/gui/core/mainwindow.h>
#include <seiscomp3/logging/output.h>
#include <QtGui>

#include "ui_mainframe.h"


namespace Seiscomp {
namespace Gui {

class XMLView;
class TraceWidget;

namespace MessageMonitor {

class ClientItem;
class Plugin;

class Logger : public QObject {
	Q_OBJECT
	
	signals:
		void logReceived(const QString &channelName,
		                 int level,
		                 const QString &msg,
		                 int time);

	friend class LogDispatcher;
};


class LogDispatcher : public Seiscomp::Logging::Output {
	public:
		LogDispatcher() : _logger(new Logger) {}
		~LogDispatcher() { delete _logger; }

		operator QObject*() { return _logger; }
	
	protected:
		void log(const char* channelName,
		         Seiscomp::Logging::LogLevel level,
		         const char* msg,
		         time_t time) {
		_logger->logReceived(channelName, (int)level, msg, (int)time);
	}

	private:
		Logger *_logger;
};



class MainFrame : public Seiscomp::Gui::MainWindow {
	Q_OBJECT
	public:
		MainFrame();
		~MainFrame();

	signals:
		void messageReceived(Seiscomp::Core::MessagePtr);

	private slots:
		void onConnectionChanged();

		void onMessageSkipped(Seiscomp::Communication::NetworkMessage*);
		void onMessageReceived(Seiscomp::Core::Message*, Seiscomp::Communication::NetworkMessage *);
		void onConsoleLogEnabled(bool);
		void onAutoOpenMessage(bool);
		void clearMessages();

		void onPluginOpen(bool);
		void onPluginClosed(QObject*);
		
		void onTimer();

		void onLog(const QString &channelName,
		           int level,
		           const QString &msg,
		           int time);

		void updateUISettings();

		void onItemSelected(QTreeWidgetItem *item, int column);

	private:
		void loadPlugins();

		void updateView(Seiscomp::Core::Message*, Seiscomp::Communication::NetworkMessage*);
		void addMessage(Seiscomp::Core::Message*, Seiscomp::Communication::NetworkMessage*);
		void updateClient(Seiscomp::Core::Message*);
		void updateClient(const QString &name, const QString &upTime);

		int  findRow(QTreeWidget*, int column, const QString &text);

		void clearStatistics();
		void updateStats(const QString &client, const QString &group, size_t msgSize);

		void closeEvent(QCloseEvent*);

	private:
		struct StatEntry {
			StatEntry() : messageCount(0), bytes(0), item(NULL) {}

			int messageCount;
			double bytes;
			QTreeWidgetItem* item;
		};
	
		Ui::MainWindow _ui;

		TraceWidget* _traceWidget;
		XMLView* _messageView;

		LogDispatcher _logger;
		QTimer _timer;

		int _msgInInterval;

		int _maxRows;
		int _pollingInterval;

		typedef QMap<QString, StatEntry> StatMap;

		StatMap _clientStats;
		StatMap _groupStats;

		typedef QMap<QWidget*, Plugin*> PluginWidgetMap;
		typedef QMap<Plugin*, QAction*> PluginActionMap;

		PluginWidgetMap _pluginWidgets;
		PluginActionMap _pluginActions;

		int _msgReceived;
		double _bytesReceived;

		QString _lastSender;
		QString _lastDestination;
		QString _lastType;

		QMap<QString, ClientItem*> _clientMap;
};


}
}
}

#endif
