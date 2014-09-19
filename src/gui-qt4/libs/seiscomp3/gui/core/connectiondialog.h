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



#ifndef __SEISCOMP_GUI_CONNECTIONDIALOG_H__
#define __SEISCOMP_GUI_CONNECTIONDIALOG_H__

#include <QtGui>
#include <seiscomp3/gui/core/ui_connectiondialog.h>
#include <seiscomp3/gui/qt4.h>
#include <seiscomp3/io/database.h>
#include <seiscomp3/communication/connection.h>

namespace Seiscomp {
namespace Gui {


class SC_GUI_API ConnectionDialog : public QDialog {
	Q_OBJECT

	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		ConnectionDialog(Seiscomp::Communication::ConnectionPtr*,
		                 Seiscomp::IO::DatabaseInterfacePtr*,
		                 QWidget* parent = 0, Qt::WFlags f = 0);
		~ConnectionDialog();


	public:
		void setUsername(const QString& username);

		void setClientParameters(const QString& server,
		                         const QString& username,
		                         const QString& primaryGroup,
		                         const QStringList& groups,
		                         int timeout);

		bool setMessagingEnabled(bool);

		bool setDatabaseParameters(const QString& uri);
		bool setDatabaseParameters(const QString& type, const QString& connection);

		std::string databaseURI() const;

		bool connectToMessaging();
		bool connectToDatabase();
		bool fetchDatabase();

		bool hasConnectionChanged() const;
		bool hasDatabaseChanged() const;


	// ------------------------------------------------------------------
	//  Public signals
	// ------------------------------------------------------------------
	signals:
		void aboutToConnect(QString host, QString user, QString group, int timeout);
		void aboutToDisconnect();

		void databaseChanged();


	// ------------------------------------------------------------------
	//  Public slots
	// ------------------------------------------------------------------
	public slots:
		void onConnectionError(int code);
		int exec();


	// ------------------------------------------------------------------
	//  Protected slots
	// ------------------------------------------------------------------
	protected slots:
		void onConnect();
		void onFetch();
		void onDatabaseConnect();
		void onItemChanged(QListWidgetItem *item);
		void onSelectAll();
		void onDeselectAll();


	// ------------------------------------------------------------------
	//  Members
	// ------------------------------------------------------------------
	private:
		::Ui::ConnectionDialog _ui;
		Seiscomp::Communication::ConnectionPtr* _connection;
		Seiscomp::IO::DatabaseInterfacePtr* _db;
		QStringList _requestedGroups;
		bool _requestAllGroups;
		bool _messagingEnabled;

		bool _changedDatabase;
		bool _changedConnection;
};


}
}

#endif
