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


#ifndef __SEISCOMP_SELECTSTATION_H__
#define __SEISCOMP_SELECTSTATION_H__

#include <QDialog>
#include <QAbstractTableModel>

#ifndef Q_MOC_RUN
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/station.h>
#endif
#include <seiscomp3/gui/qt4.h>

#include <seiscomp3/gui/datamodel/ui_selectstation.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API SelectStation : public QDialog {
	Q_OBJECT

	// ------------------------------------------------------------------
	// X'struction
	// ------------------------------------------------------------------
	public:
		explicit SelectStation(Core::Time time, bool ignoreDisabledStations,
		                       QWidget* parent = 0, Qt::WindowFlags f = 0);
		explicit SelectStation(Core::Time time, bool ignoreDisabledStations,
		                       const QSet<QString> &blackList,
		                       QWidget* parent = 0, Qt::WindowFlags f = 0);
		~SelectStation();

		QList<DataModel::Station*> selectedStations() const;

		void setReferenceLocation(double lat, double lon);


	// ------------------------------------------------------------------
	// Private Interface
	// ------------------------------------------------------------------
	private slots:
		void listMatchingStations(const QString& substr);


	private:
		void init(Core::Time, bool ignoreDisabledStations,
		          const QSet<QString> *blackList);


	// ------------------------------------------------------------------
	// Private data members
	// ------------------------------------------------------------------
	private:
		Ui::SelectStation _ui;
};

} // namespace Gui
} // namespace Seiscomp

#endif
