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



#include <seiscomp3/client/inventory.h>
#include <seiscomp3/datamodel/network.h>
#include <seiscomp3/math/geo.h>

#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/datamodel/selectstation.h>

#include <string>

#include <QMessageBox>
#include <QSizePolicy>
#include <QHeaderView>
#include <QKeyEvent>
#include <QRegExp>


namespace Seiscomp {
namespace Gui {

namespace {


class StationsModel : public QAbstractTableModel {
	public:
		struct Entry {
			DataModel::Station *station;
			QString code;
			double distance;
			double azimuth;
		};


	public:
		StationsModel(Core::Time time,
		              const QSet<QString> *blackList,
		              bool ignoreDisabledStations,
		              QObject *parent = 0) : QAbstractTableModel(parent) {
			DataModel::Inventory* inv = Client::Inventory::Instance()->inventory();
			if ( inv != NULL ) {
				for ( size_t i = 0; i < inv->networkCount(); ++i ) {
					DataModel::Network* n = inv->network(i);
		
					try {
						if ( n->end() <= time )
							continue;
					}
					catch ( Core::ValueException& ) {}
		
					for ( size_t j = 0; j < n->stationCount(); ++j ) {
						DataModel::Station* s = n->station(j);
		
						try {
							if ( s->end() <= time )
								continue;
						}
						catch ( Core::ValueException& ) {}
		
						if ( ignoreDisabledStations
						  && !SCApp->isStationEnabled(n->code(), s->code()) )
							continue;
		
						QString code = (n->code() + "." + s->code()).c_str();
		
						if ( blackList && blackList->contains(code) ) continue;
		
						Entry entry;
						entry.station = s;
						entry.code = code;
						entry.distance = 0;
						entry.azimuth = 0;

						_data.push_back(entry);
					}
				}
			}
		}

		void setReferenceLocation(double lat, double lon) {
			for ( int row = 0; row < _data.size(); ++row ) {
				DataModel::Station *s = _data[row].station;
		
				double azi2;
				Math::Geo::delazi(lat, lon, s->latitude(), s->longitude(), &_data[row].distance, &_data[row].azimuth, &azi2);
			}
		}

		int rowCount(const QModelIndex &) const {
			return _data.size();
		}

		int columnCount(const QModelIndex &) const {
			return 3;
		}

		DataModel::Station *station(int row) const {
			return _data[row].station;
		}

		QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const {
			if ( orientation == Qt::Horizontal && role == Qt::DisplayRole ) {
				switch ( section ) {
					case 0:
						return "Name";
					case 1:
						return "Distance";
					case 2:
						return "Azimuth";
					default:
						break;
				}
			}
			return QVariant();
		}

		QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const {
			switch ( role ) {
				case Qt::DisplayRole:
					switch ( index.column() ) {
						case 0:
							return _data[index.row()].code;
						case 1:
							return QString("%1").arg(_data[index.row()].distance, 0, 'f', 1);
						case 2:
							return QString("%1").arg(_data[index.row()].azimuth, 0, 'f', 1);
						default:
							break;
					}
					break;
				case Qt::UserRole:
					switch ( index.column() ) {
						case 1:
							return _data[index.row()].distance;
						case 2:
							return _data[index.row()].azimuth;
						default:
							break;
					}
					break;
				default:
					break;
			}

			return QVariant();
		}

	private:
		QVector<Entry> _data;
};


class StationsSortFilterProxyModel : public QSortFilterProxyModel {
	public:
		StationsSortFilterProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent) {
			setFilterCaseSensitivity(Qt::CaseInsensitive);
		}

	protected:
		bool lessThan(const QModelIndex &left, const QModelIndex &right) const {
			if ( (left.column() == 1 && right.column() == 1)
			  || (left.column() == 2 && right.column() == 2) )
				return sourceModel()->data(left, Qt::UserRole).toDouble() < sourceModel()->data(right, Qt::UserRole).toDouble();
			else
				return QSortFilterProxyModel::lessThan(left, right);
		}
};


}



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SelectStation::SelectStation(Core::Time time, bool ignoreDisabledStations,
                             QWidget* parent, Qt::WFlags f)
 : QDialog(parent, f) {
	init(time, ignoreDisabledStations, NULL);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SelectStation::SelectStation(Core::Time time, bool ignoreDisabledStations,
                             const QSet<QString> &blackList,
                             QWidget* parent, Qt::WFlags f)
 : QDialog(parent, f) {
	init(time, ignoreDisabledStations, &blackList);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SelectStation::~SelectStation()
{
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SelectStation::init(Core::Time time, bool ignoreDisabledStations,
                         const QSet<QString> *blackList) {
	_ui.setupUi(this);

	_ui.stationLineEdit->setFocus(Qt::TabFocusReason);

	StationsModel *model = new StationsModel(time, blackList,
	                                         ignoreDisabledStations, this);
	QSortFilterProxyModel *filterModel = new StationsSortFilterProxyModel(this);
	filterModel->setSourceModel(model);
	_ui.table->setModel(filterModel);

	// Configure widget
	connect(_ui.table->horizontalHeader(), SIGNAL(sectionClicked(int)),
	        _ui.table, SLOT(sortByColumn(int)));
	_ui.table->horizontalHeader()->setSortIndicatorShown(true);
	_ui.table->horizontalHeader()->setStretchLastSection(true);
	_ui.table->verticalHeader()->hide();

	_ui.table->hideColumn(1);
	_ui.table->hideColumn(2);

	_ui.table->horizontalHeader()->setSortIndicator(0, Qt::AscendingOrder);
	_ui.table->resizeColumnsToContents();
	//_ui.table->horizontalHeader()->hide();

	//_stationNames;
	
	connect(_ui.stationLineEdit, SIGNAL(textChanged(const QString&)),
	        this, SLOT(listMatchingStations(const QString&)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SelectStation::listMatchingStations(const QString& substr) {
	QString trimmedStr = substr.trimmed();

	static_cast<QSortFilterProxyModel*>(_ui.table->model())->setFilterWildcard(trimmedStr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QList<DataModel::Station*> SelectStation::selectedStations() const {
	QModelIndexList list = _ui.table->selectionModel()->selectedIndexes();
	QList<DataModel::Station *> result;

	QSortFilterProxyModel *proxy = static_cast<QSortFilterProxyModel*>(_ui.table->model());
	StationsModel* model = static_cast<StationsModel*>(proxy->sourceModel());

	foreach ( const QModelIndex &index, list )
		result.push_back(model->station(proxy->mapToSource(index).row()));

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SelectStation::setReferenceLocation(double lat, double lon) {
	static_cast<StationsModel*>(static_cast<QSortFilterProxyModel*>(_ui.table->model())->sourceModel())->setReferenceLocation(lat, lon);

	_ui.table->showColumn(1);
	_ui.table->showColumn(2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


} // namespace Gui
} // namespace Seiscomp
