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


#ifndef __SEISCOMP_CONFIGURATION_INSPECTOR_H__
#define __SEISCOMP_CONFIGURATION_INSPECTOR_H__


#include <QTreeWidget>
#include <QTableWidget>
#include <QWidget>

#include "ui_inspector.h"
#ifndef Q_MOC_RUN
#include <seiscomp3/core/baseobject.h>
#endif


class Inspector : public QDialog {
	Q_OBJECT

	public:
		Inspector(QWidget *parent = 0, Qt::WindowFlags f = 0);
		~Inspector();


	public:
		void setObject(Seiscomp::Core::BaseObject *obj);


	private:
		void addObject(QTreeWidgetItem *parent);
		void addProperty(const std::string &name, const std::string &type,
		                 const std::string &value, bool isIndex = false,
		                 bool isOptional = false, bool isReference = false);


	private slots:
		void selectionChanged();
		void linkClicked(QString);


	private:
		Seiscomp::Core::BaseObjectPtr _object;
		Ui::Inspector _ui;
};


#endif
