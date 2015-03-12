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


#include "information.h"

#include <seiscomp3/system/environment.h>
#include <QtGui>


InformationPanel::InformationPanel(QWidget *parent)
: ConfiguratorPanel(false, parent) {
	_name = "Information";
	_icon = QIcon(":/res/icons/info.png");
	setHeadline("Information");
	setDescription("System paths and variables");

	QVBoxLayout *l = new QVBoxLayout;
	l->setMargin(0);
	setLayout(l);

	QTableWidget *table = new QTableWidget;
	table->setFrameShape(QFrame::NoFrame);
	table->setColumnCount(2);
	table->verticalHeader()->setVisible(false);
	table->horizontalHeader()->setStretchLastSection(true);
	table->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
	table->setHorizontalHeaderLabels(QStringList() << "Name" << "Value");
	table->setAlternatingRowColors(true);
	table->setSelectionMode(QAbstractItemView::NoSelection);

	l->addWidget(table);

	Seiscomp::Environment *env = Seiscomp::Environment::Instance();
	addRow(table, "PATH", getenv("PATH"));
	addRow(table, "ROOTDIR", env->installDir().c_str());
	addRow(table, "DEFAULTCONFIGDIR", env->globalConfigDir().c_str());
	addRow(table, "SYSTEMCONFIGDIR", env->appConfigDir().c_str());
	addRow(table, "CONFIGDIR", env->configDir().c_str());
	addRow(table, "LOGDIR", env->logDir().c_str());
	addRow(table, "DATADIR", env->shareDir().c_str());

	table->resizeColumnsToContents();
}


void InformationPanel::addRow(QTableWidget *t, const QString &name, const QString &value) {
	int row = t->rowCount();
	t->insertRow(row);
	QTableWidgetItem *nameItem = new QTableWidgetItem(name);
	QTableWidgetItem *valueItem = new QTableWidgetItem(value);
	nameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	valueItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	t->setItem(row, 0, nameItem);
	t->setItem(row, 1, valueItem);
}
