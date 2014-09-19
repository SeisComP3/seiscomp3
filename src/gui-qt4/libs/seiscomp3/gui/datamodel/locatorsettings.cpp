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



#define SEISCOMP_COMPONENT Gui

#include <seiscomp3/gui/datamodel/locatorsettings.h>


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LocatorSettings::LocatorSettings(QWidget * parent, Qt::WindowFlags flags)
: QDialog(parent, flags) {
	_ui.setupUi(this);

	_ui.tableParameters->setColumnCount(2);
	_ui.tableParameters->setHorizontalHeaderLabels(
		QStringList() << "Parameter" << "Value"
	);
	
	_ui.tableParameters->horizontalHeader()->setStretchLastSection(true);
	_ui.tableParameters->verticalHeader()->hide();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LocatorSettings::addRow(const QString &name, const QString &value) {
	QTableWidgetItem *nameItem = new QTableWidgetItem;
	QTableWidgetItem *valueItem = new QTableWidgetItem;
	nameItem->setText(name);
	nameItem->setFlags(valueItem->flags() & ~Qt::ItemIsEditable);
	valueItem->setText(value);
	valueItem->setFlags(valueItem->flags() | Qt::ItemIsEditable);

	int row = _ui.tableParameters->rowCount();
	_ui.tableParameters->insertRow(row);
	_ui.tableParameters->setItem(row, 0, nameItem);
	_ui.tableParameters->setItem(row, 1, valueItem);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LocatorSettings::lastRowAdded() {
	_ui.tableParameters->resizeColumnToContents(0);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LocatorSettings::ContentList LocatorSettings::content() const {
	ContentList list;

	for ( int i = 0; i < _ui.tableParameters->rowCount(); ++i ) {
		list.append(
			QPair<QString,QString>(
				_ui.tableParameters->item(i,0)->text(),
				_ui.tableParameters->item(i,1)->text()
			)
		);
	}

	return list;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}

}
