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



#define SEISCOMP_COMPONENT Gui::MagnitudeView

#include "magnitudeview.h"
#include <seiscomp3/gui/core/diagramwidget.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/math/mean.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/processing/amplitudeprocessor.h>
#include <seiscomp3/processing/magnitudeprocessor.h>
#include <seiscomp3/seismology/regions.h>
#include <seiscomp3/utils/misc.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/core/utils.h>
#include <seiscomp3/gui/datamodel/amplitudeview.h>
#include <seiscomp3/gui/datamodel/utils.h>

#include <functional>

#ifdef WIN32
#define snprintf _snprintf
#endif

using namespace std;
using namespace Seiscomp::Core;
using namespace Seiscomp::Client;
using namespace Seiscomp::IO;
using namespace Seiscomp::DataModel;


#define INVALID_MAG 0.0


namespace {


struct TabData {
	TabData()
	: valid(true), selected(false) {}

	TabData(const string &pid)
	: publicID(pid), valid(true), selected(false) {}

	string publicID;
	bool   valid;
	bool   selected;
};


}


namespace Seiscomp {
namespace Gui {


namespace {

MAKEENUM(
	StaMagsListColumns,
	EVALUES(
		USED,
		NETWORK,
		STATION,
		CHANNEL,
		MAGNITUDE,
		RESIDUAL,
		DISTANCE,
		AMP,
		SNR,
		PERIOD,
		CREATED,
		UPDATED
	),
	ENAMES(
		"Sel",
		"Net",
		"Sta",
		"Loc/Cha",
		"Mag",
		"Res",
		"Dist",
		"Amp",
		"SNR",
		"Per (s)",
		"Created",
		"Updated"
	)
);


bool colVisibility[StaMagsListColumns::Quantity] = {
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	true,
	true,
	false,
	false
};


QVariant colAligns[StaMagsListColumns::Quantity] = {
	QVariant(),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignHCenter | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight | Qt::AlignVCenter),
	int(Qt::AlignRight| Qt::AlignVCenter),
	int(Qt::AlignLeft | Qt::AlignVCenter),
	int(Qt::AlignLeft | Qt::AlignVCenter)
};


class StaMagsSortFilterProxyModel : public QSortFilterProxyModel {
	public:
		StaMagsSortFilterProxyModel(QObject *parent = 0) : QSortFilterProxyModel(parent) {}

	protected:
		bool lessThan(const QModelIndex &left, const QModelIndex &right) const {
			if ( (left.column() == MAGNITUDE && right.column() == MAGNITUDE) ||
			     (left.column() == RESIDUAL && right.column() == RESIDUAL) ||
			     (left.column() == DISTANCE && right.column() == DISTANCE) ||
			     (left.column() == SNR && right.column() == SNR) ||
			     (left.column() == PERIOD && right.column() == PERIOD) )
				return sourceModel()->data(left, Qt::UserRole).toDouble() < sourceModel()->data(right, Qt::UserRole).toDouble();
			else
				return QSortFilterProxyModel::lessThan(left, right);
		}
};


Util::KeyValuesPtr getParams(const string &net, const string &sta) {
	ConfigModule *module = SCApp->configModule();
	if ( module == NULL ) return NULL;

	for ( size_t ci = 0; ci < module->configStationCount(); ++ci ) {
		ConfigStation* cs = module->configStation(ci);
		if ( cs->networkCode() != net || cs->stationCode() != sta ) continue;
		Setup *setup = findSetup(cs, SCApp->name());
		if ( setup == NULL ) continue;
		if ( !setup->enabled() ) continue;

		DataModel::ParameterSet *ps = DataModel::ParameterSet::Find(setup->parameterSetID());
		if ( ps == NULL ) {
			SEISCOMP_WARNING("Cannot find parameter set %s for station %s.%s",
			                 setup->parameterSetID().data(),
			                 net.data(), sta.data());
			continue;
		}

		Util::KeyValuesPtr keys = new Util::KeyValues;
		keys->init(ps);
		return keys;
	}

	return NULL;
}


int findType(QTabBar *tab, const char *text) {
	for ( int i = 0; i < tab->count(); ++i ) {
		Magnitude *mag = Magnitude::Find(tab->tabData(i).value<TabData>().publicID);
		if ( mag && mag->type() == text )
			return i;
	}

	return -1;
}


int findData(QTabBar *tab, const string &publicID) {
	for ( int i = 0; i < tab->count(); ++i ) {
		if ( tab->tabData(i).value<TabData>().publicID == publicID )
			return i;
	}

	return -1;
}


void setBold(QWidget *w, bool b) {
	QFont f = w->font();
	f.setBold(b);
	w->setFont(f);
}


string waveformIDToStdString(const WaveformStreamID& id) {
	return (id.networkCode() + "." + id.stationCode() + "." +
	        id.locationCode() + "." + id.channelCode());
}


template <typename T>
struct like {
	bool operator()(const T &lhs, const T &rhs) const {
		return false;
	}
};


template <>
struct like<QString> {
	bool operator()(const QString &lhs, const QString &rhs) const {
		return Core::wildcmp(rhs.toAscii(), lhs.toAscii());
	}
};


template <typename T>
class ModelFieldValueFilter : public ModelAbstractRowFilter {
	public:
		explicit ModelFieldValueFilter(int column, CompareOperation op, T value) : _column(column), _op(op), _value(value) {}

		virtual int column() const { return _column; }
		virtual CompareOperation operation() const { return _op; }
		virtual QString value() const { return QVariant(_value).toString(); }

		virtual QString toString() {
			StaMagsListColumns c = (EStaMagsListColumns)_column;
			QString str = QString("%1,%2,%3").arg(c.toString()).arg(_op.toString()).arg(_value);
			return str;
		}

		virtual bool fromString(const QString &) { return false; }

		virtual bool passes(QAbstractItemModel *model, int row) {
			return check(model, model->index(row, _column), _value);
		}

	protected:
		bool check(QAbstractItemModel *model, const QModelIndex &idx, const T &v) {
			switch ( _op ) {
				case Less:
					return std::less<T>()(model->data(idx, Qt::UserRole).value<T>(), v);
				case LessEqual:
					return std::less_equal<T>()(model->data(idx, Qt::UserRole).value<T>(), v);
				case Equal:
					return std::equal_to<T>()(model->data(idx, Qt::UserRole).value<T>(), v);
				case NotEqual:
					return std::not_equal_to<T>()(model->data(idx, Qt::UserRole).value<T>(), v);
				case Greater:
					return std::greater<T>()(model->data(idx, Qt::UserRole).value<T>(), v);
				case GreaterEqual:
					return std::greater_equal<T>()(model->data(idx, Qt::UserRole).value<T>(), v);
				case Like:
					return like<T>()(model->data(idx, Qt::UserRole).value<T>(), v);
				default:
					break;
			}

			return false;
		}

	protected:
		int              _column;
		CompareOperation _op;
		T                _value;
};


class ModelDistanceFilter : public ModelFieldValueFilter<double> {
	public:
		explicit ModelDistanceFilter(int column, CompareOperation op, double value)
		: ModelFieldValueFilter<double>(column, op, value) {
			if ( SCScheme.unit.distanceInKM )
				_baseValue = Math::Geo::km2deg(value);
			else
				_baseValue = _value;
		}

		virtual bool passes(QAbstractItemModel *model, int row) {
			return check(model, model->index(row, _column), _baseValue);
		}

	private:
		double _baseValue;
};


template <class FUNC>
class ModelRowFilterMultiOperation : public ModelAbstractRowFilter {
	public:
		// Takes ownership
		explicit ModelRowFilterMultiOperation() {}
		~ModelRowFilterMultiOperation() {
			foreach ( ModelAbstractRowFilter *f, _filters ) delete f;
		}

	public:
		int column() const { return -1; }
		virtual CompareOperation operation() const { return Undefined; }
		virtual QString value() const { return QString(); }

		bool passes(QAbstractItemModel *model, int row) {
			if ( _filters.empty() ) return true;

			bool ret = _filters[0]->passes(model,row);

			for ( int i = 1; i < _filters.size(); ++i )
				ret = _func(ret, _filters[i]->passes(model,row));

			return ret;
		}

		virtual QString toString() {
			QString str;

			for ( int i = 0; i < _filters.size(); ++i ) {
				if ( i ) str += QString(" %1 ").arg(FUNC::str());
				str += _filters[i]->toString();
			}

			return str;
		}

		virtual bool fromString(const QString &s) {
			QStringList items = s.split(FUNC::str());
			foreach ( const QString &item, items ) {
				QStringList toks = item.trimmed().split(",");
				if ( toks.size() != 3 ) return false;
				StaMagsListColumns c;
				CompareOperation op;
				QString value;
				if ( !c.fromString(toks[0].trimmed().toStdString()) )
					return false;
				if ( !op.fromString(toks[1].trimmed().toStdString()) )
					return false;
				value = toks[2].trimmed();

				ModelAbstractRowFilter *stage = NULL;

				switch ( c ) {
					case CHANNEL:
					{
						stage = new ModelFieldValueFilter<QString>(c, op, value);
						break;
					}
					case MAGNITUDE:
					case RESIDUAL:
					case DISTANCE:
					{
						bool ok;
						double v = value.toDouble(&ok);
						if ( ok ) {
							if ( c == DISTANCE )
								stage = new ModelDistanceFilter(c, op, v);
							else
								stage = new ModelFieldValueFilter<double>(c, op, v);
						}
						break;
					}
					default:
						break;
				}

				if ( stage != NULL )
					add(stage);
				else
					return false;
			}

			return true;
		}

		void add(ModelAbstractRowFilter *f) { _filters.append(f); }
		int count() const { return _filters.count(); }
		ModelAbstractRowFilter *filter(int i) const { return _filters[i]; }

	private:
		FUNC                              _func;
		QVector<ModelAbstractRowFilter *> _filters;
};


struct opAnd {
	static const char *str() { static const char *s = "&&"; return s; }
	bool operator()(bool lhs, bool rhs) const {
		return lhs && rhs;
	}
};

struct opOr {
	static const char *str() { static const char *s = "||"; return s; }
	bool operator()(bool lhs, bool rhs) const {
		return lhs || rhs;
	}
};

struct opXor {
	static const char *str() { static const char *s = "^^"; return s; }
	bool operator()(bool lhs, bool rhs) const {
		return lhs != rhs;
	}
};


typedef ModelRowFilterMultiOperation<opOr> ModelRowFilter;

QColor failedQCColor(255,160,0,64);

}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


// Implementation of StationMagnitudeModel

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StationMagnitudeModel::StationMagnitudeModel(DataModel::Origin* origin,
                                             DataModel::Magnitude* netMag,
                                             PublicObjectCache *cache,
                                             QObject *parent)
: QAbstractTableModel(parent), _cache(cache) {
	for ( int i = 0; i < StaMagsListColumns::Quantity; ++i )
		if ( i == DISTANCE ) {
			if ( SCScheme.unit.distanceInKM )
				_header << QString("%1 (km)").arg(EStaMagsListColumnsNames::name(i));
			else
				_header << QString("%1 (°)").arg(EStaMagsListColumnsNames::name(i));
		}
		else
			_header << EStaMagsListColumnsNames::name(i);

	setOrigin(origin, netMag);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void StationMagnitudeModel::setOrigin(DataModel::Origin* origin,
                                      DataModel::Magnitude* magnitude) {
	_origin = origin;
	_magnitude = magnitude;
	_rowCount = 0;
	if ( _origin ) {
		if ( _magnitude ) {
			_rowCount = _magnitude->stationMagnitudeContributionCount();
			_distance.fill(-1.0, _rowCount);
			_used.fill(Qt::Checked, _rowCount);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StationMagnitudeModel::rowCount(const QModelIndex &) const {
	/*
	if ( _magnitude ) {
		cout << "------ REQUESTED ROWCOUNT: " << _magnitude->stationMagnitudeContributionCount() << endl;
		return _magnitude->stationMagnitudeContributionCount();
	}
	return 0;
	*/
	return _rowCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int StationMagnitudeModel::columnCount(const QModelIndex &) const {
	return _header.count();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationMagnitudeModel::insertRow(int row, const QModelIndex & parent) {
	beginInsertRows(QModelIndex(), row, row);
	++_rowCount;
	endInsertRows();
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant StationMagnitudeModel::data(const QModelIndex &index, int role) const {
	if ( !index.isValid() )
		return QVariant();

	if ( !_magnitude )
		return QVariant();

	if ( index.row() >= (int)_magnitude->stationMagnitudeContributionCount() )
		return QVariant();


	if ( role == Qt::CheckStateRole && index.column() == 0 ) {
		if ( index.row() < _used.size() )
			return _used[index.row()];
		else
			return QVariant();
	}

	StationMagnitude* sm = StationMagnitude::Find(_magnitude->stationMagnitudeContribution(index.row())->stationMagnitudeID());

	if ( role == Qt::DisplayRole ) {
		if ( sm == NULL )
			return QVariant();

		char buf[10];
		double smval = sm->magnitude().value();

		switch ( index.column() ) {
			case USED:
				try {
					snprintf(buf, 10, "   %.3f", _magnitude->stationMagnitudeContribution(index.row())->weight());
					return buf;
				}
				catch ( Core::ValueException& ) {}
				break;

			case NETWORK:
				try{return sm->waveformID().networkCode().c_str();}catch(...){return QVariant();}

			case STATION:
				try{return sm->waveformID().stationCode().c_str();}catch(...){return QVariant();}

			case CHANNEL:
				try {
					if ( sm->waveformID().locationCode().empty() )
						return sm->waveformID().channelCode().c_str();
					else
						return (sm->waveformID().locationCode() + '.' + sm->waveformID().channelCode()).c_str();
				}
				catch ( ValueException& ) {}
				break;

			case MAGNITUDE:
				snprintf(buf, 10, "%.2f", smval);
				return buf;

			case RESIDUAL:
				if ( _magnitude ) {
					try {
						snprintf(buf, 10, "%.2f", _magnitude->stationMagnitudeContribution(index.row())->residual());
						return buf;
					}
					catch ( ... ) {}
				}
				break;

			case DISTANCE: // dist
				if ( index.row() < _distance.size() ) {
					double distance = _distance[index.row()];
					if ( distance >= 0 ) {
						if ( SCScheme.unit.distanceInKM )
							snprintf(buf, 10, "%.*f", SCScheme.precision.distance, Math::Geo::deg2km(distance));
						else
							snprintf(buf, 10, distance<10 ? "%.2f" : "%.1f", distance);
						return buf;
					}
					// return _distance[index.row()] < 0?QVariant():_distance[index.row()];
				}
				break;

			case AMP:
			{
				AmplitudePtr amp = _cache->get<Amplitude>(sm->amplitudeID());
				if ( amp ) {
					try {
						return QString("%1").arg(amp->amplitude().value());
					}
					catch ( ... ) {}
				}
				break;
			}

			case SNR:
			{
				AmplitudePtr amp = _cache->get<Amplitude>(sm->amplitudeID());
				if ( amp ) {
					try {
						return QString("%1").arg(amp->snr(), 0, 'f', 1);
					}
					catch ( ... ) {}
				}
				break;
			}

			case PERIOD:
			{
				AmplitudePtr amp = _cache->get<Amplitude>(sm->amplitudeID());
				if ( amp ) {
					try {
						return QString("%1").arg(amp->period().value(), 0, 'f', 2);
					}
					catch ( ... ) {}
				}
				break;
			}

			case CREATED:
				try {
					return timeToString(sm->creationInfo().creationTime(), "%T.%1f");
				}
				catch ( ValueException& ) {}
				break;

			case UPDATED:
				try {
					return timeToString(sm->creationInfo().modificationTime(), "%T.%1f");
				}
				catch ( ValueException& ) {}
				break;

			default:
				break;
		}
	}
	else if ( role == Qt::BackgroundRole ) {
		if ( sm != NULL ) {
			try {
				if ( !sm->passedQC() )
					return failedQCColor;
			}
			catch ( ... ) {}
		}
	}
	else if ( role == Qt::UserRole ) {
		switch ( index.column() ) {
			case NETWORK:
				try {
					return sm->waveformID().networkCode().c_str();
				}
				catch ( ... ) {}
				break;

			case STATION:
				try {
					return sm->waveformID().stationCode().c_str();
				}
				catch ( ... ) {}
				break;

			case CHANNEL:
				try {
					return sm->waveformID().channelCode().c_str();
				}
				catch ( ValueException& ) {}
				break;

			// Magnitude
			case MAGNITUDE:
				try { return sm->magnitude().value(); }
				catch ( ValueException& ) {}
				break;
			// Residual
			case RESIDUAL:
				try {
					return fabs(_magnitude->stationMagnitudeContribution(index.row())->residual());
				}
				catch ( ValueException& ) {}
				break;
			// Distance
			case DISTANCE:
				if ( index.row() < _distance.size() ) {
					double distance = _distance[index.row()];
					if ( distance >= 0 )
						return distance;
				}
				break;
			case SNR:
			{
				AmplitudePtr amp = _cache->get<Amplitude>(sm->amplitudeID());
				if ( amp ) {
					try {
						return amp->snr();
					}
					catch ( ... ) {}
				}
				return -1;
			}
			case PERIOD:
			{
				AmplitudePtr amp = _cache->get<Amplitude>(sm->amplitudeID());
				if ( amp ) {
					try {
						return amp->period().value();
					}
					catch ( ... ) {}
				}
				return -1;
			}
			default:
				break;
		}
	}
	/*
	else if ( role == Qt::TextColorRole ) {
		switch ( index.column() ) {
			// Residual
			case RESIDUAL:
				try {
					double smval = sm->magnitude().value(),
					nmval = _magnitude ? _magnitude->magnitude().value() : 0;
					double res = smval = nmval;
					return res < 0?Qt::darkRed:Qt::darkGreen;
				}
				catch ( ValueException& ) {}
				break;
			default:
				break;
		}
	}
	*/
	else if ( role == Qt::TextAlignmentRole ) {
		return colAligns[index.column()];
	}
	else if ( role == Qt::ToolTipRole ) {
		switch ( index.column() ) {
			case CREATED:
				try {
					return sm->creationInfo().creationTime().toString("%F %T.%f").c_str();
				}
				catch ( ValueException& ) {}
				break;

			case UPDATED:
				try {
					return sm->creationInfo().modificationTime().toString("%F %T.%f").c_str();
				}
				catch ( ValueException& ) {}
				break;
		}
	}

	return QVariant();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QVariant StationMagnitudeModel::headerData(int section, Qt::Orientation orientation,
                                  int role) const {
	if ( section < 0 ) return QVariant();

	if ( orientation == Qt::Horizontal ) {
		switch ( role ) {
			case Qt::DisplayRole:
				if ( section >= _header.count() )
					return QString("%1").arg(section);
				else
					return _header[section];
				break;
			case Qt::TextAlignmentRole:
				return colAligns[section];
		}
	}
	else
		return section;

	return QVariant();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Qt::ItemFlags StationMagnitudeModel::flags(const QModelIndex &index) const {
	if ( !index.isValid() )
		return Qt::ItemIsEnabled;

	if ( index.column() == 0 )
		return QAbstractTableModel::flags(index) | Qt::ItemIsUserCheckable;


	return QAbstractTableModel::flags(index);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationMagnitudeModel::setData(const QModelIndex &index, const QVariant &value,
                                    int role) {
	// set checkBox
	if ( index.isValid() && role == Qt::CheckStateRole ) {
		if ( _used.count() <= index.row() )
			_used.resize(index.row()+1);

		_used[index.row()] = (Qt::CheckState)value.toInt();
		emit dataChanged(index, index);
		return true;
	}

	// set data fields
	if ( index.isValid() && role == Qt::DisplayRole ) {
		// distance
		if (index.column() == DISTANCE){
			if ( _distance.size() <= index.row() )
				_distance.resize(index.row()+1);

			//_distance[index.row()] = QString("%1").arg(value.toDouble(), 0, 'f', 1).toDouble();
			_distance[index.row()] = value.toDouble();
			emit dataChanged(index, index);
			return true;
		}
	}

	return QAbstractTableModel::setData(index, value, role);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool StationMagnitudeModel::useMagnitude(int row) const {
	return _used[row] == Qt::Checked;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeRowFilter::MagnitudeRowFilter(ModelAbstractRowFilter **filter_ptr, QWidget * parent, Qt::WFlags f)
: QDialog(parent, f) {
	_ui.setupUi(this);

	_filter = filter_ptr;

	if ( SCScheme.unit.distanceInKM )
		_ui.labelInfo->setText(tr("NOTE: Distance is specified in km."));
	else
		_ui.labelInfo->setText(tr("NOTE: Distance is specified in degree."));

	ModelRowFilter *filter = NULL;

	if ( _filter != NULL )
		filter = reinterpret_cast<ModelRowFilter*>(*_filter);

	if ( filter != NULL ) {
		for ( int i = 0; i < filter->count(); ++i ) {
			ModelAbstractRowFilter *stage = filter->filter(i);
			Row &row = addRow();

			switch ( stage->column() ) {
				case CHANNEL:
					row.column->setCurrentIndex(0);
					break;
				case MAGNITUDE:
					row.column->setCurrentIndex(1);
					break;
				case RESIDUAL:
					row.column->setCurrentIndex(2);
					break;
				case DISTANCE:
					row.column->setCurrentIndex(3);
					break;
				default:
					break;
			}

			if ( stage->operation() != ModelAbstractRowFilter::Undefined )
				row.operation->setCurrentIndex((int)stage->operation()-1);
			row.value->setText(stage->value());
		}
	}
	else {
		// At least one row is mandatory
		addRow();
	}

	QVBoxLayout *layout = new QVBoxLayout;
	layout->setMargin(0);
	_ui.frameFilters->setLayout(layout);

	for ( int i = 0; i < _rows.count(); ++i ) {
		layout->addLayout(_rows[i].layout);
	}

	connect(_ui.btnAdd, SIGNAL(clicked()), this, SLOT(addFilter()));
	connect(_ui.btnRemove, SIGNAL(clicked()), this, SLOT(removeFilter()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeRowFilter::Row &MagnitudeRowFilter::addRow() {
	Row row;
	row.layout = new QHBoxLayout;
	row.column = new QComboBox(_ui.frameFilters);
	row.operation = new QComboBox(_ui.frameFilters);
	row.value = new QLineEdit(_ui.frameFilters);

	row.layout->addWidget(row.column);
	row.layout->addWidget(row.operation);
	row.layout->addWidget(row.value);

	row.column->addItem(tr("Channel"));
	row.column->addItem(tr("Magnitude"));
	row.column->addItem(tr("Residual"));
	row.column->addItem(tr("Distance"));

	for ( int i = 1; i < ModelAbstractRowFilter::CompareOperation::Quantity; ++i )
		row.operation->addItem(tr(ModelAbstractRowFilter::ECompareOperationNames::name(i)));

	_rows.append(row);
	return _rows.back();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeRowFilter::accept() {
	if ( _filter != NULL ) {
		ModelRowFilter *filter = new ModelRowFilter();

		foreach ( Row r, _rows ) {
			ModelAbstractRowFilter::CompareOperation op;
			int column;

			op.fromInt(r.operation->currentIndex()+1);

			switch ( r.column->currentIndex() ) {
				case 0:
					column = CHANNEL;
					break;
				case 1:
					column = MAGNITUDE;
					break;
				case 2:
					column = RESIDUAL;
					break;
				case 3:
					column = DISTANCE;
					break;
				default:
					QMessageBox::critical(this, tr("Error"), tr("Internal error: invalid column %1").arg(r.column->currentIndex()));
					delete filter;
					return;
			}

			ModelAbstractRowFilter *stage = NULL;

			switch ( column ) {
				case CHANNEL:
				{
					QString value = r.value->text();
					stage = new ModelFieldValueFilter<QString>(column, op, value);
					break;
				}
				case MAGNITUDE:
				case RESIDUAL:
				case DISTANCE:
				{
					bool ok;
					double value = r.value->text().toDouble(&ok);
					if ( ok ) {
						if ( column == DISTANCE )
							stage = new ModelDistanceFilter(column, op, value);
						else
							stage = new ModelFieldValueFilter<double>(column, op, value);
					}
					else {
						QMessageBox::critical(this, tr("Error"), tr("Expected double value"));
						delete filter;
						return;
					}
					break;
				}
				default:
					QMessageBox::critical(this, tr("Error"), tr("Internal error: invalid target column %1").arg(column));
					delete filter;
					return;
			}

			filter->add(stage);
		}

		if ( *_filter != NULL )
			delete *_filter;

		*_filter = filter;

		QSettings &s = SCApp->settings();
		s.beginGroup("MagnitudeView");
		s.setValue("selectionFilter", filter->toString());
		s.endGroup();
	}

	QDialog::accept();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeRowFilter::addFilter() {
	static_cast<QVBoxLayout*>(_ui.frameFilters->layout())->addLayout(addRow().layout);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeRowFilter::removeFilter() {
	if ( _rows.size() <= 1 ) return;

	delete _rows.back().layout;
	delete _rows.back().column;
	delete _rows.back().operation;
	delete _rows.back().value;
	_rows.pop_back();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


ModelAbstractRowFilter *&selectionFilter() {
	static ModelAbstractRowFilter *selectionFilter = NULL;
	static bool filterReadFromSettings = false;

	if ( !filterReadFromSettings ) {
		QSettings &s = SCApp->settings();
		s.beginGroup("MagnitudeView");
		QString f_str = s.value("selectionFilter").toString();
		if ( !f_str.isEmpty() ) {
			ModelRowFilter *filter = new ModelRowFilter;
			if ( !filter->fromString(f_str) ) {
				delete filter;
				QMessageBox::warning(NULL, SCApp->tr("Settings"), SCApp->tr("Could not restore magnitude selection filter"), QMessageBox::Ok);
			}
			else {
				if ( selectionFilter != NULL )
					delete selectionFilter;
				selectionFilter = filter;
			}
		}
		s.endGroup();
		filterReadFromSettings = true;
	}

	return selectionFilter;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! Implementation of MagnitudeView
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeView::MagnitudeView(const MapsDesc &maps,
                             Seiscomp::DataModel::DatabaseQuery *reader,
                             QWidget *parent, Qt::WFlags f)
: QWidget(parent, f)
, _reader(reader)
, _modelStationMagnitudes(NULL, NULL, &_objCache)
, _origin(NULL)
, _objCache(_reader, 500) {
	_maptree = new Map::ImageTree(maps);
	_modelStationMagnitudesProxy = NULL;
	init(reader);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeView::MagnitudeView(Map::ImageTree* mapTree,
                             Seiscomp::DataModel::DatabaseQuery* reader,
                             QWidget * parent, Qt::WFlags f)
: QWidget(parent, f)
, _reader(reader)
, _modelStationMagnitudes(NULL, NULL, &_objCache)
, _origin(NULL)
, _objCache(_reader, 500) {
	_maptree = mapTree;
	_modelStationMagnitudesProxy = NULL;
	init(reader);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::closeEvent(QCloseEvent *e) {
	if ( _amplitudeView ) _amplitudeView->close();
	QWidget::closeEvent(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::closeTab(int idx) {
	std::string magID = _tabMagnitudes->tabData(idx).value<TabData>().publicID;
	MagnitudePtr mag = Magnitude::Find(magID);

	if ( mag != NULL ) {
		if ( mag->detach() ) {
			emit magnitudeRemoved(_origin->publicID().c_str(), mag.get());
			_tabMagnitudes->removeTab(idx);
		}
		else
			QMessageBox::critical(this, "Error", tr("An error occured while removing magnitude %1").arg(magID.c_str()));
	}
	else
		QMessageBox::critical(this, "Error", tr("Did not find magnitude %1").arg(magID.c_str()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::debugCreateMagRef() {
	StationMagnitudePtr staMag = StationMagnitude::Create();
	staMag->setMagnitude(10.0);
	staMag->setType(_netMag->type());
	staMag->setWaveformID(WaveformStreamID("II", "KAPI", "00", "BH", ""));

	StationMagnitudeContributionPtr magRef = new StationMagnitudeContribution(staMag->publicID());
	_netMag->add(magRef.get());
	addObject(_netMag->publicID().c_str(), magRef.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::init(Seiscomp::DataModel::DatabaseQuery* reader) {
	_ui.setupUi(this);

	_amplitudeView = NULL;
	_computeMagnitudesSilently = false;
	_enableMagnitudeTypeSelection = true;

	QObject *drawFilter = new ElideFadeDrawer(this);

	_ui.labelRegion->setFont(SCScheme.fonts.heading3);
	_ui.labelRegion->installEventFilter(drawFilter);
	//_ui.comboMagType->setFont(SCScheme.fonts.highlight);

	_ui.label_7->setFont(SCScheme.fonts.normal);
	_ui.label->setFont(SCScheme.fonts.normal);
	_ui.label_8->setFont(SCScheme.fonts.normal);
	_ui.label_3->setFont(SCScheme.fonts.normal);
	_ui.label_2->setFont(SCScheme.fonts.normal);

	_ui.labelMagnitude->setFont(SCScheme.fonts.highlight);
	_ui.labelRMS->setFont(SCScheme.fonts.normal);
	_ui.labelNumStaMags->setFont(SCScheme.fonts.highlight);
	_ui.labelMinMag->setFont(SCScheme.fonts.normal);
	_ui.labelMaxMag->setFont(SCScheme.fonts.normal);

	/*
	_ui.lbAgencyID->setFont(SCScheme.fonts.normal);
	_ui.lbAuthor->setFont(SCScheme.fonts.normal);
	_ui.labelAgencyID->setFont(SCScheme.fonts.highlight);
	_ui.labelAuthor->setFont(SCScheme.fonts.highlight);
	_ui.lbPublicID->setFont(SCScheme.fonts.normal);
	_ui.labelMethod->setFont(SCScheme.fonts.highlight);
	*/

	setBold(_ui.labelAgencyID, true);
	setBold(_ui.labelAuthor, true);
	setBold(_ui.labelMethod, true);

	ElideFadeDrawer *elider = new ElideFadeDrawer(this);

	fixWidth(_ui.labelMethod, 8);
	_ui.labelMethod->installEventFilter(elider);
	//fixWidth(_ui.labelAgencyID, 8);
	_ui.labelAgencyID->installEventFilter(elider);
	_ui.labelAuthor->installEventFilter(elider);

	_ui.btnCommit->setFont(SCScheme.fonts.highlight);

	/*
	QAction* debugAction = new QAction(this);
	debugAction->setShortcut(Qt::Key_C);
	addAction(debugAction);
	connect(debugAction, SIGNAL(triggered()),
	        this, SLOT(debugCreateMagRef()));
	*/

	_stamagnitudes = new DiagramWidget(_ui.groupMagnitudes);
	if ( SCScheme.unit.distanceInKM )
		_stamagnitudes->setAbscissaName("Distance (km)");
	else
		_stamagnitudes->setAbscissaName("Distance (°)");
	_stamagnitudes->setOrdinateName("Residual");
	_stamagnitudes->setMarkerDistance(10, 0.1);
	_stamagnitudes->setDisplayRect(QRectF(0,-2,180,4));
	_stamagnitudes->setValueDisabledColor(SCScheme.colors.magnitudes.disabled);
	_ui.groupMagnitudes->layout()->addWidget(_stamagnitudes);

	_map = new MagnitudeMap(_maptree.get(), _ui.frameMap);

	if ( _map ) {
		connect(_map, SIGNAL(magnitudeChanged(int, bool)),
		        this, SLOT(changeStationState(int, bool)));
		connect(_map, SIGNAL(clickedMagnitude(int)), this, SLOT(selectMagnitude(int)));
		connect(_map, SIGNAL(clickedStation(const std::string &, const std::string &)), this, SLOT(selectStation(const std::string &, const std::string &)));
	}

	QHBoxLayout* hboxLayout = new QHBoxLayout(_ui.frameMap);
	hboxLayout->setMargin(0);
	if ( _map ) {
		_map->setMouseTracking(true);
		connect(_map, SIGNAL(hoverMagnitude(int)), this, SLOT(hoverMagnitude(int)));
		hboxLayout->addWidget(_map);

		try {
			_map->setStationsMaxDist(SCApp->configGetDouble("olv.map.stations.unassociatedMaxDist"));
		}
		catch ( ... ) {}
	}

	hboxLayout = new QHBoxLayout(_ui.frameMagnitudeTypes);
	hboxLayout->setMargin(0);

	_tabMagnitudes = new QTabBar(_ui.frameMagnitudeTypes);
	_tabMagnitudes->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred));
	_tabMagnitudes->setShape(QTabBar::RoundedNorth);
	_tabMagnitudes->setUsesScrollButtons(true);

#if QT_VERSION >= 0x040500
	connect(_tabMagnitudes, SIGNAL(tabCloseRequested(int)), this, SLOT(closeTab(int)));
#endif

	hboxLayout->addWidget(_tabMagnitudes);

	// reset GUI to default
	resetContent();

	_ui.tableStationMagnitudes->horizontalHeader()->setSortIndicatorShown(true);
	_ui.tableStationMagnitudes->horizontalHeader()->setSortIndicator(DISTANCE, Qt::AscendingOrder);
	//_ui.tableStationMagnitudes->horizontalHeader()->setStretchLastSection(true);
	_ui.tableStationMagnitudes->setContextMenuPolicy(Qt::CustomContextMenu);
	_ui.tableStationMagnitudes->setSelectionMode(QAbstractItemView::ExtendedSelection);
	_ui.tableStationMagnitudes->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(_ui.tableStationMagnitudes->horizontalHeader(), SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(tableStationMagnitudesHeaderContextMenuRequested(const QPoint &)));

	connect(_ui.tableStationMagnitudes->horizontalHeader(), SIGNAL(sectionClicked(int)),
	        _ui.tableStationMagnitudes, SLOT(sortByColumn(int)));
	connect(_ui.tableStationMagnitudes, SIGNAL(customContextMenuRequested(const QPoint &)),
	        this, SLOT(tableStationMagnitudesContextMenuRequested(const QPoint &)));

	// on selection in  diagram
	connect(_stamagnitudes, SIGNAL(valueActiveStateChanged(int, bool)), this, SLOT(changeMagnitudeState(int, bool)));
	connect(_stamagnitudes, SIGNAL(endSelection()), this, SLOT(magnitudesSelected()));
	connect(_stamagnitudes, SIGNAL(adjustZoomRect(QRectF&)), this, SLOT(adjustMagnitudeRect(QRectF&)));
	connect(_stamagnitudes, SIGNAL(hover(int)), this, SLOT(hoverMagnitude(int)));
	connect(_stamagnitudes, SIGNAL(clicked(int)), this, SLOT(selectMagnitude(int)));

	// on selection in table
	connect(&_modelStationMagnitudes, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
	        this, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));

	connect(_ui.btnRecalculate, SIGNAL(clicked()), this, SLOT(recalculateMagnitude()));
	connect(_ui.btnSelect, SIGNAL(clicked()), this, SLOT(selectChannels()));
	connect(_ui.btnActivate, SIGNAL(clicked()), this, SLOT(activateChannels()));
	connect(_ui.btnDeactivate, SIGNAL(clicked()), this, SLOT(deactivateChannels()));
	connect(_ui.btnWaveforms, SIGNAL(clicked()), this, SLOT(openWaveforms()));

	QMenu *selectMenu = new QMenu;
	QAction *editSelectionFilter = new QAction(tr("Edit"), this);
	editSelectionFilter->setShortcut(QKeySequence("shift+s"));
	selectMenu->addAction(editSelectionFilter);

	_ui.btnSelect->setMenu(selectMenu);
	connect(editSelectionFilter, SIGNAL(triggered()), this, SLOT(selectChannelsWithEdit()));

	//NOTE: Set to visible if you want to activate magnitude recalculation and
	//      comitting: Very alpha and comitting does still not work
	_ui.btnCommit->setVisible(false);
	//_ui.btnWaveforms->setVisible(false);

	_ui.groupReview->setEnabled(false);

	try {
		_magnitudeTypes = SCApp->configGetStrings("magnitudes");
	}
	catch ( ... ) {
		_magnitudeTypes.push_back("MLv");
		_magnitudeTypes.push_back("mb");
		_magnitudeTypes.push_back("mB");
		_magnitudeTypes.push_back("Mwp");
	}

	// Gather available magnitudes types
	_availableMagTypes = Processing::MagnitudeProcessorFactory::Services();
	if ( _availableMagTypes != NULL ) {
		for ( size_t i = 0; i < _magnitudeTypes.size(); ) {
			if ( std::find(_availableMagTypes->begin(), _availableMagTypes->end(), _magnitudeTypes[i])
			     == _availableMagTypes->end() ) {
				SEISCOMP_WARNING("Removing unavailable magnitude: %s", _magnitudeTypes[i].c_str());
				_magnitudeTypes.erase(_magnitudeTypes.begin()+i);
			}
			else
				++i;
		}
	}

	_currentMagnitudeTypes = _magnitudeTypes;

	setReadOnly(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MagnitudeView::~MagnitudeView() {
	if ( _availableMagTypes != NULL )
		delete _availableMagTypes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MapWidget* MagnitudeView::map() const {
	return _map;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setPreferredMagnitudeID(const string &id) {
	_preferredMagnitudeID = id;

	//QColor col = palette().color(QPalette::WindowText);
	for ( int i = 0; i < _tabMagnitudes->count(); ++i ) {
		TabData d = _tabMagnitudes->tabData(i).value<TabData>();
		if ( d.publicID == _preferredMagnitudeID ) {
			//_tabMagnitudes->setTabTextColor(i, Qt::green);
			_tabMagnitudes->setTabIcon(i, QIcon(":icons/icons/ok.png"));
			resetPreferredMagnitudeSelection();
		}
		else {
			//_tabMagnitudes->setTabTextColor(i, col);
			_tabMagnitudes->setTabIcon(i, QIcon());
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeView::setDefaultAggregationType(const std::string &type) {
	if ( type == "mean" || type == "trimmed mean" || type == "median" ) {
		_defaultMagnitudeAggregation = type;
		return true;
	}
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::recalculateMagnitude() {
	// Recalculate the magnitudes according the defined settings
	if ( !_netMag ) return;

	vector<double> mags;
	vector<double> weights;

	for ( int i = 0; i < _modelStationMagnitudes.rowCount(); ++i ) {
		if ( _modelStationMagnitudes.useMagnitude(i) ) {
			StationMagnitude* sm = StationMagnitude::Find(_netMag->stationMagnitudeContribution(i)->stationMagnitudeID());
			if ( !sm ) {
				QMessageBox::critical(this, "Error", QString("StationMagnitude %1 not found").arg(_netMag->stationMagnitudeContribution(i)->stationMagnitudeID().c_str()));
				return;
			}

			mags.push_back(sm->magnitude().value());
		}
	}

	if ( mags.empty() ) {
		QMessageBox::critical(this, "Error", "At least one station magnitude must be selected");
		return;
	}

	double netmag, stdev;

	if ( _ui.btnDefault->isChecked() ) {
		if ( mags.size() < 4 ) {
			Math::Statistics::computeMean(mags, netmag, stdev);
			weights.resize(mags.size(), 1.);
			_netMag->setMethodID("mean");
		}
		else {
			Math::Statistics::computeTrimmedMean(mags, 25.0, netmag, stdev, &weights);
			_netMag->setMethodID("trimmed mean");
		}
	}
	else if ( _ui.btnMean->isChecked() ) {
		if ( !Math::Statistics::computeMean(mags, netmag, stdev) ) {
			QMessageBox::critical(this, "Error", "Recalculating the magnitude using the trimmed mean failed for unknown reason");
			return;
		}

		weights.resize(mags.size(), 1.);

		_netMag->setMethodID("mean");
	}
	else if ( _ui.btnTrimmedMean->isChecked() ) {
		if ( !Math::Statistics::computeTrimmedMean(mags, _ui.spinBox->value(), netmag, stdev, &weights) ) {
			QMessageBox::critical(this, "Error", "Recalculating the magnitude using the trimmed mean failed for unknown reason");
			return;
		}

		_netMag->setMethodID("trimmed mean");
	}
	else if ( _ui.btnMedian->isChecked() ) {
		netmag = Math::Statistics::median(mags);
		if ( mags.size() > 1 ) {
			stdev = 0;
			for ( size_t i = 0; i < mags.size(); ++i )
				stdev += (mags[i] - netmag) * (mags[i] - netmag);
			stdev /= mags.size()-1;
			stdev = sqrt(stdev);
		}

		weights.resize(mags.size(), 1.);

		_netMag->setMethodID("median");
	}
	else {
		QMessageBox::critical(this, "Error", "Please select a method to recalculate the magnitude.");
		return;
	}

	_netMag->setMagnitude(DataModel::RealQuantity(netmag, stdev, Core::None, Core::None, Core::None));
	_netMag->setEvaluationStatus(EvaluationStatus(CONFIRMED));

	int idx = findType(_tabMagnitudes, _netMag->type().c_str());
	_tabMagnitudes->setTabTextColor(idx, QColor());
	_tabMagnitudes->setTabIcon(idx, QIcon());

	// Update corresponding Mw estimation
	Processing::MagnitudeProcessorPtr proc = Processing::MagnitudeProcessorFactory::Create(_netMag->type().c_str());
	if ( proc ) {
		double Mw, MwError;
		string type = proc->typeMw();

		if ( proc->estimateMw(netmag, Mw, MwError) == Processing::MagnitudeProcessor::OK ) {
			idx = findType(_tabMagnitudes, type.c_str());
			//int idx = _ui.comboMagType->findText(type.c_str());
			if ( idx != -1 ) {
				MagnitudePtr magMw =
					//Magnitude::Find(_ui.comboMagType->itemData(idx).value<QString>().toStdString());
					Magnitude::Find(_tabMagnitudes->tabData(idx).value<TabData>().publicID);

				if ( magMw && magMw != _netMag ) {
					stdev = stdev > MwError?stdev:MwError;
					magMw->setMagnitude(RealQuantity(Mw, stdev, Core::None, Core::None, Core::None));
					try {
						magMw->setStationCount(_netMag->stationCount());
					}
					catch ( ... ) {
						magMw->setStationCount(Core::None);
					}
					emit magnitudeUpdated(_origin->publicID().c_str(), magMw.get());
				}

				_tabMagnitudes->setTabText(idx, QString("%1 %2").arg(magMw->type().c_str()).arg(magMw->magnitude().value(), 0, 'f', 2));
			}
			else {
				MagnitudePtr magMw;

				CreationInfo ci;
				ci.setAgencyID(SCApp->agencyID());
				ci.setAuthor(SCApp->author());
				ci.setCreationTime(Core::Time::GMT());
				stdev = stdev > MwError ? stdev : MwError;

				for ( size_t m = 0; m < _origin->magnitudeCount(); ++m ) {
					if ( _origin->magnitude(m)->type() == proc->typeMw() ) {
						magMw = _origin->magnitude(m);
						break;
					}
				}

				if ( !magMw ) {
					magMw = Magnitude::Create();
					_origin->add(magMw.get());
				}

				magMw->setCreationInfo(ci);
				magMw->setType(proc->typeMw());
				magMw->setMagnitude(RealQuantity(Mw, stdev, Core::None, Core::None, Core::None));

				try {
					magMw->setStationCount(_netMag->stationCount());
				}
				catch ( ... ) {
					magMw->setStationCount(Core::None);
				}

				addMagnitude(magMw.get());
			}
		}
		else {
			idx = findType(_tabMagnitudes, type.c_str());
			if ( idx != -1 )
				_tabMagnitudes->removeTab(idx);
		}
	}

	idx = findData(_tabMagnitudes, _netMag->publicID());
	if ( idx != -1 )
		_tabMagnitudes->setTabText(idx, QString("%1 %2").arg(_netMag->type().c_str()).arg(_netMag->magnitude().value(), 0, 'f', 2));

	idx = 0;
	int staCount = 0;
	for ( int i = 0; i < _modelStationMagnitudes.rowCount(); ++i ) {
		if ( _modelStationMagnitudes.useMagnitude(i) ) {
			if ( weights[idx] > 0.0 )
				++staCount;

			_netMag->stationMagnitudeContribution(i)->setWeight(weights[idx]);
			++idx;
		}
		else
			_netMag->stationMagnitudeContribution(i)->setWeight(0.);
	}

	for ( int i = 0; i < _modelStationMagnitudes.rowCount(); ++i ) {
		if ( staCount ) {
			StationMagnitude* sm = StationMagnitude::Find(_netMag->stationMagnitudeContribution(i)->stationMagnitudeID());
			_netMag->stationMagnitudeContribution(i)->setResidual(sm->magnitude().value() - netmag);
		}
		else
			_netMag->stationMagnitudeContribution(i)->setResidual(Core::None);
	}

	_netMag->setStationCount(staCount);

	updateMagnitudeLabels();
	_ui.tableStationMagnitudes->reset();

	emit magnitudeUpdated(_origin->publicID().c_str(), _netMag.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::selectChannels() {
	if ( selectionFilter() == NULL ) {
		if ( !editSelectionFilter() )
			return;
	}

	ModelAbstractRowFilter *filter = selectionFilter();

	if ( filter == NULL )
		return;

	_ui.tableStationMagnitudes->selectionModel()->clear();

	int rowCount = _modelStationMagnitudesProxy->rowCount();
	for ( int i = 0; i < rowCount; ++i ) {
		if ( reinterpret_cast<ModelAbstractRowFilter*>(filter)->passes(_modelStationMagnitudesProxy, i) )
			_ui.tableStationMagnitudes->selectionModel()->select(_modelStationMagnitudesProxy->index(i, 0), QItemSelectionModel::Rows | QItemSelectionModel::Select);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::selectChannelsWithEdit() {
	if ( !editSelectionFilter() )
		return;

	selectChannels();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::activateChannels() {
	QModelIndexList rows = _ui.tableStationMagnitudes->selectionModel()->selectedRows();
	foreach ( const QModelIndex &idx, rows )
		changeStationState(_modelStationMagnitudesProxy->mapToSource(idx).row(), true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::deactivateChannels() {
	QModelIndexList rows = _ui.tableStationMagnitudes->selectionModel()->selectedRows();
	foreach ( const QModelIndex &idx, rows )
		changeStationState(_modelStationMagnitudesProxy->mapToSource(idx).row(), false);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::openWaveforms() {
	if ( !_netMag ) return;

	if ( _amplitudeView ) {
		if ( _amplitudeView->currentMagnitudeType() == _netMag->type() ) {
			_amplitudeView->activateWindow();
			_amplitudeView->raise();
			return;
		}
		else {
			if ( QMessageBox::question(this, "Waveform review",
			                           QString("A waveform review window for type %1 is still active.\n"
			                                   "Do you want to replace it with current type %2?")
			                           .arg(_amplitudeView->currentMagnitudeType().c_str())
			                           .arg(_netMag->type().c_str()),
			                           QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes)
			     == QMessageBox::No )
				return;
		}
	}
	else {
		_amplitudeView = new AmplitudeView(NULL, Qt::Window);
		_amplitudeView->setAttribute(Qt::WA_DeleteOnClose);
		_amplitudeView->setDatabase(_reader);

		try {
			_amplitudeView->setStrongMotionCodes(SCApp->configGetStrings("picker.accelerationChannelCodes"));
		}
		catch ( ... ) {}

		connect(_amplitudeView, SIGNAL(magnitudeCreated(Seiscomp::DataModel::Magnitude*)),
		        this, SLOT(magnitudeCreated(Seiscomp::DataModel::Magnitude*)));
		connect(_amplitudeView, SIGNAL(amplitudesConfirmed(Seiscomp::DataModel::Origin*, QList<Seiscomp::DataModel::AmplitudePtr>)),
		        this, SLOT(amplitudesConfirmed(Seiscomp::DataModel::Origin*, QList<Seiscomp::DataModel::AmplitudePtr>)));
		connect(_amplitudeView, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
	}

	_amplitudeView->setConfig(_amplitudeConfig);

	if ( !_amplitudeView->setOrigin(_origin.get(), _netMag->type()) ) {
		delete _amplitudeView;
		_amplitudeView = NULL;
		return;
	}

	_amplitudeView->show();
	_amplitudeView->raise();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::objectDestroyed(QObject *o) {
	if ( o == _amplitudeView )
		_amplitudeView = NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::computeMagnitudes() {
	try {
		_origin->depth();
	}
	catch ( ... ) {
		SEISCOMP_ERROR("Origin.depth not set: recompute magnitudes failed");
		if ( !_computeMagnitudesSilently ) {
			QMessageBox::critical(this, tr("Compute magnitudes"),
			                      tr("Origin.depth is not set, cannot compute "
			                      "magnitudes."));
		}
		return;
	}

	if ( _availableMagTypes == NULL ) {
		QMessageBox::critical(this, tr("Compute magnitudes"),
		                      tr("No magnitude processors available."));
		return;
	}

	std::vector<std::string> magnitudeTypes;

	if ( _enableMagnitudeTypeSelection && !_computeMagnitudesSilently ) {
		QDialog setupTypesDlg;
		setupTypesDlg.setWindowTitle(tr("Select magnitude types"));

		QVBoxLayout *ml = new QVBoxLayout;
		QHBoxLayout *hl;
		QVBoxLayout *vl;

		// Center layout
		hl = new QHBoxLayout;

		// Right tool buttons
		vl = new QVBoxLayout;
		QPushButton *selectAll = new QPushButton(tr("Select all"));
		vl->addWidget(selectAll);
		QPushButton *deselectAll = new QPushButton(tr("Deselect all"));
		vl->addWidget(deselectAll);
		vl->addStretch();

		hl->addLayout(vl);

		// Magnitude type grid
		vl = new QVBoxLayout;

		QGridLayout *grid = new QGridLayout;
		QVector<QCheckBox*> typeChecks;

		for ( size_t i = 0; i < _availableMagTypes->size(); ++i ) {
			CheckBox *check = new CheckBox;
			check->setChecked(std::find(_currentMagnitudeTypes.begin(), _currentMagnitudeTypes.end(), _availableMagTypes->at(i)) != _currentMagnitudeTypes.end());
			connect(selectAll, SIGNAL(clicked()), check, SLOT(check()));
			connect(deselectAll, SIGNAL(clicked()), check, SLOT(unCheck()));

			QLabel *label = new QLabel;
			label->setText(_availableMagTypes->at(i).c_str());

			grid->addWidget(check, i, 0);
			grid->addWidget(label, i, 1);

			typeChecks.append(check);
		}

		QSpacerItem *spacer = new QSpacerItem(0, 0, QSizePolicy::Maximum, QSizePolicy::MinimumExpanding);
		grid->addItem(spacer, _availableMagTypes->size(), 0);
		grid->setColumnStretch(0, 0);
		grid->setColumnStretch(1, 1);
		grid->setMargin(0);

		QWidget *typeSelections = new QWidget;
		typeSelections->setLayout(grid);

		QScrollArea *scrollArea = new QScrollArea;
		scrollArea->setFrameShape(QFrame::NoFrame);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollArea->setWidget(typeSelections);
		scrollArea->setWidgetResizable(true);

		vl->addWidget(scrollArea);

		hl->insertLayout(0, vl);

		QLabel *info = new QLabel;
		info->setText(tr("Select the magnitudes that should be computed."));
		ml->addWidget(info);

		QFrame *hline = new QFrame;
		hline->setFrameShape(QFrame::HLine);
		ml->addWidget(hline);

		ml->addLayout(hl);

		// Main buttons
		hl = new QHBoxLayout;
		QPushButton *ok = new QPushButton(tr("OK"));
		connect(ok, SIGNAL(clicked()), &setupTypesDlg, SLOT(accept()));
		QPushButton *cancel = new QPushButton(tr("Cancel"));
		connect(cancel, SIGNAL(clicked()), &setupTypesDlg, SLOT(reject()));
		hl->addStretch();
		hl->addWidget(ok);
		hl->addWidget(cancel);

		ml->addLayout(hl);

		setupTypesDlg.setLayout(ml);

		if ( setupTypesDlg.exec() != QDialog::Accepted )
			return;

		for ( int i = 0; i < typeChecks.size(); ++i ) {
			QCheckBox *check = typeChecks[i];
			if ( check->isChecked() )
				magnitudeTypes.push_back(_availableMagTypes->at(i));
		}
	}
	else
		magnitudeTypes = _magnitudeTypes;

	// Save the last choice
	_currentMagnitudeTypes = magnitudeTypes;

	// Gather all amplitude types required to compute the magnitudes
	CalculateAmplitudes::TypeSet ampTypes;
	for ( size_t i = 0; i < magnitudeTypes.size(); ++i ) {
		Processing::MagnitudeProcessorPtr proc;
		proc = Processing::MagnitudeProcessorFactory::Create(magnitudeTypes[i].c_str());
		if ( proc ) {
			ampTypes.insert(proc->amplitudeType());
			SEISCOMP_DEBUG("+ %s : %s", magnitudeTypes[i].c_str(), proc->amplitudeType().c_str());
		}
	}

	CalculateAmplitudes dlg(_origin.get(), this);
	static QByteArray geomCalculateAmplitudes;

	dlg.restoreGeometry(geomCalculateAmplitudes);
	// TODO: This should go into the settings panel
	dlg.setRecomputeAmplitudes(false);
	dlg.setAmplitudeTypes(ampTypes);
	dlg.setDatabase(_reader);
	dlg.setStreamURL(_amplitudeConfig.recordURL.toStdString());
	dlg.setAmplitudeCache(&_amplitudes);
	dlg.setSilentMode(_computeMagnitudesSilently);
	if ( dlg.exec() != QDialog::Accepted ) return;

	geomCalculateAmplitudes = dlg.saveGeometry();

	QList<Seiscomp::DataModel::AmplitudePtr> sessionAmplitudes;

	CalculateAmplitudes::iterator it;
	for ( it = dlg.begin(); it != dlg.end(); ++it ) {
		sessionAmplitudes.append(it->second.first);

		// Find all cached amplitudes for the current pick
		pair<PickAmplitudeMap::iterator, PickAmplitudeMap::iterator> itp;
		itp = _amplitudes.equal_range(it->first);

		// Check if this pick-amplitude association has been registered already
		bool found = false;
		for ( PickAmplitudeMap::iterator ita = itp.first; ita != itp.second; ++ita ) {
			if ( ita->second.first == it->second.first ) {
				found = true;
				break;
			}
		}

		if ( !found )
			_amplitudes.insert(PickAmplitudeMap::value_type(it->first, it->second));
	}

	SEISCOMP_DEBUG("Amplitude cache size: %d", (int)_amplitudes.size());

	MagnitudeStats magErrors;

	for ( size_t i = 0; i < magnitudeTypes.size(); ++i ) {
		Magnitude *mag = computeStationMagnitudes(magnitudeTypes[i], &sessionAmplitudes, &magErrors);
		if ( mag ) {
			computeMagnitude(mag, _defaultMagnitudeAggregation?*_defaultMagnitudeAggregation:"");

			Processing::MagnitudeProcessorPtr proc = Processing::MagnitudeProcessorFactory::Create(mag->type().c_str());
			if ( proc ) {
				double stddev = 0;
				double Mw, MwError;
				if ( proc->estimateMw(mag->magnitude(), Mw, MwError) == Processing::MagnitudeProcessor::OK ) {
					try {
						stddev = mag->magnitude().uncertainty();
					}
					catch ( ... ) {}

					CreationInfo ci;
					ci.setAgencyID(SCApp->agencyID());
					ci.setAuthor(SCApp->author());
					ci.setCreationTime(Core::Time::GMT());
					stddev = stddev > MwError ? stddev : MwError;
					MagnitudePtr MagMw;

					for ( size_t m = 0; m < _origin->magnitudeCount(); ++m ) {
						if ( _origin->magnitude(m)->type() == proc->typeMw() ) {
							MagMw = _origin->magnitude(m);
							break;
						}
					}

					if ( !MagMw ) MagMw = Magnitude::Create();

					MagMw->setCreationInfo(ci);
					MagMw->setType(proc->typeMw());
					MagMw->setMagnitude(RealQuantity(Mw, stddev, Core::None, Core::None, Core::None));
					try {
						MagMw->setStationCount(mag->stationCount());
					}
					catch ( ... ) {
						MagMw->setStationCount(Core::None);

					}
					_origin->add(MagMw.get());
				}
			}
		}
		else {
			MagnitudePtr mag = Magnitude::Create();
			mag->setType(magnitudeTypes[i]);
			mag->setEvaluationStatus(EvaluationStatus(REJECTED));
			mag->setMagnitude(RealQuantity(INVALID_MAG));

			CreationInfo ci;
			ci.setAgencyID(SCApp->agencyID());
			ci.setAuthor(SCApp->author());
			ci.setCreationTime(Core::Time::GMT());

			mag->setCreationInfo(ci);

			_origin->add(mag.get());
		}
	}

	if ( !magErrors.isEmpty() && !_computeMagnitudesSilently ) {
		static QByteArray geomCalculateMagnitudes;

		QDialog dlg;
		dlg.setWindowTitle(tr("Compute magnitudes"));
		dlg.restoreGeometry(geomCalculateMagnitudes);

		QTableWidget *table = new QTableWidget;
		QVBoxLayout *vlayout = new QVBoxLayout;
		dlg.setLayout(vlayout);
		vlayout->addWidget(table);

		QHBoxLayout *hlayout = new QHBoxLayout;
		hlayout->addStretch();
		QPushButton *btnOK = new QPushButton(tr("OK"));
		hlayout->addWidget(btnOK);
		connect(btnOK, SIGNAL(pressed()), &dlg, SLOT(accept()));
		vlayout->addLayout(hlayout);

		table->setColumnCount(3);

		table->horizontalHeader()->setStretchLastSection(true);
		table->horizontalHeader()->setVisible(true);
		table->setHorizontalHeaderLabels(QStringList() << tr("Channel") << tr("Type") << tr("Error"));

		QColor orange(255,128,0);

		// Populate errors
		foreach ( const MagnitudeStatus &s, magErrors ) {
			QTableWidgetItem *itemStream = new QTableWidgetItem(waveformIDToStdString(s.amplitude->waveformID()).c_str());
			itemStream->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			QTableWidgetItem *itemType = new QTableWidgetItem(s.type.c_str());
			itemType->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			QTableWidgetItem *itemState = new QTableWidgetItem(s.status.toString());
			itemState->setData(Qt::TextColorRole, s.warning ? orange : Qt::red);
			itemState->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

			int row = table->rowCount();
			table->insertRow(row);

			table->setItem(row, 0, itemStream);
			table->setItem(row, 1, itemType);
			table->setItem(row, 2, itemState);
		}

		table->resizeColumnsToContents();
		table->setSortingEnabled(true);

		dlg.exec();
		geomCalculateMagnitudes = dlg.saveGeometry();
	}

	// Synchronize local amplitudes for commit
	set<string> ampIDs;

	for ( size_t i = 0; i < _origin->stationMagnitudeCount(); ++i ) {
		if ( _origin->stationMagnitude(i)->amplitudeID().empty() ) continue;
		ampIDs.insert(_origin->stationMagnitude(i)->amplitudeID());
	}

	AmplitudeSet ampSet;

	for ( PickAmplitudeMap::iterator it = _amplitudes.begin();
	      it != _amplitudes.end(); ++it ) {

		// Skip external amplitudes
		if ( it->second.second == false ) continue;

		// Skip unrequested amplitudes
		if ( ampIDs.find(it->second.first->publicID()) == ampIDs.end() )
			continue;

		ampSet.insert(AmplitudeSet::value_type(it->second.first, true));
	}

	emit localAmplitudesAvailable(_origin.get(), &ampSet, &ampIDs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::magnitudeCreated(Seiscomp::DataModel::Magnitude *netMag) {
	AmplitudeView *view = (AmplitudeView*)sender();
	ObjectChangeList<DataModel::Amplitude> changedAmps;
	view->getChangedAmplitudes(changedAmps);

	if ( netMag->origin() != _origin ) return;

	if ( _ui.btnMean->isChecked() )
		computeMagnitude(netMag, "mean");
	else if ( _ui.btnTrimmedMean->isChecked() )
		computeMagnitude(netMag, "trimmed mean");
	else if ( _ui.btnMedian->isChecked() )
		computeMagnitude(netMag, "median");
	else
		computeMagnitude(netMag, "");

	ObjectChangeList<DataModel::Amplitude>::iterator it;
	for ( it = changedAmps.begin(); it != changedAmps.end(); ++it )
		_amplitudes.insert(PickAmplitudeMap::value_type(it->first->pickID(), AmplitudeEntry(it->first, true)));

	SEISCOMP_DEBUG("Amplitude cache size: %d", (int)_amplitudes.size());

	// Estimate Mw
	Processing::MagnitudeProcessorPtr proc = Processing::MagnitudeProcessorFactory::Create(netMag->type().c_str());
	if ( proc ) {
		double Mw, MwError;
		string type = proc->typeMw();
		int idx = findType(_tabMagnitudes, type.c_str());
		if ( proc->estimateMw(netMag->magnitude().value(), Mw, MwError) == Processing::MagnitudeProcessor::OK ) {
			//int idx = _ui.comboMagType->findText(type.c_str());
			if ( idx != -1 ) {
				MagnitudePtr magMw =
					Magnitude::Find(_tabMagnitudes->tabData(idx).value<TabData>().publicID);

				if ( magMw ) {
					if ( MwError < netMag->magnitude().uncertainty() )
						MwError = netMag->magnitude().uncertainty();
					magMw->setMagnitude(RealQuantity(Mw, MwError, Core::None, Core::None, Core::None));
					magMw->setStationCount(netMag->stationCount());
					emit magnitudeUpdated(_origin->publicID().c_str(), magMw.get());
				}

				_tabMagnitudes->setTabText(idx, QString("%1 %2").arg(magMw->type().c_str()).arg(magMw->magnitude().value(), 0, 'f', 2));
			}
		}
		else if ( idx != -1 )
			_tabMagnitudes->removeTab(idx);
	}

	int typeIdx = findType(_tabMagnitudes, netMag->type().c_str());
	if ( typeIdx == -1 ) {
		typeIdx = addMagnitude(netMag);
		_tabMagnitudes->setCurrentIndex(typeIdx);
		emit magnitudeUpdated(_origin->publicID().c_str(), netMag);
		return;
	}

	/*
	data.setValue(QString(netMag->publicID().c_str()));
	int idx = findData(_tabMagnitudes, data);
	if ( idx != -1 ) {
		updateObject(_origin->publicID().c_str(), netMag);
		_tabMagnitudes->setCurrentIndex(idx);
		emit magnitudeUpdated(_origin->publicID().c_str(), netMag);
		return;
	}
	*/

	// Replace magnitude
	_tabMagnitudes->setTabText(typeIdx, QString("%1 %2").arg(netMag->type().c_str()).arg(netMag->magnitude().value(), 0, 'f', 2));
	_tabMagnitudes->setTabData(typeIdx, QVariant::fromValue<TabData>(netMag->publicID()));
	if ( _tabMagnitudes->currentIndex() != typeIdx )
		_tabMagnitudes->setCurrentIndex(typeIdx);
	else
		updateContent();

	emit magnitudeUpdated(_origin->publicID().c_str(), netMag);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::amplitudesConfirmed(Origin *origin,
                                        QList<Seiscomp::DataModel::AmplitudePtr> amps) {
	AmplitudeView *view = (AmplitudeView*)sender();
	ObjectChangeList<DataModel::Amplitude> changedAmps;
	view->getChangedAmplitudes(changedAmps);

	if ( origin != _origin ) return;
	try {
		_origin->depth();
	}
	catch ( ... ) {
		QMessageBox::critical(this, "Compute magnitudes",
		                      "Origin.depth is not set, cannot compute "
		                      "magnitudes.");
		return;
	}

	ObjectChangeList<DataModel::Amplitude>::iterator it;
	for ( it = changedAmps.begin(); it != changedAmps.end(); ++it )
		_amplitudes.insert(PickAmplitudeMap::value_type(it->first->pickID(), AmplitudeEntry(it->first, true)));

	SEISCOMP_DEBUG("Amplitude cache size: %d", (int)_amplitudes.size());

	MagnitudePtr mag = computeStationMagnitudes(_amplitudeView->currentMagnitudeType(), &amps);
	if ( !mag ) return;

	int typeIdx = findType(_tabMagnitudes, mag->type().c_str());

	// Type not yet available, add it
	if ( typeIdx == -1 )
		typeIdx = addMagnitude(mag.get());
	// Update magnitude for type
	else
		_tabMagnitudes->setTabData(typeIdx, QVariant::fromValue<TabData>(mag->publicID()));

	if ( _tabMagnitudes->currentIndex() != typeIdx )
		showMagnitude(mag->publicID());
	else
		updateContent();

	recalculateMagnitude();

	set<string> ampIDs;
	for ( size_t i = 0; i < _origin->stationMagnitudeCount(); ++i ) {
		if ( _origin->stationMagnitude(i)->amplitudeID().empty() ) continue;
		ampIDs.insert(_origin->stationMagnitude(i)->amplitudeID());
	}

	AmplitudeSet ampSet;

	for ( PickAmplitudeMap::iterator it = _amplitudes.begin();
	      it != _amplitudes.end(); ++it ) {

		// Skip external amplitudes
		if ( it->second.second == false ) continue;

		// Skip unrequested amplitudes
		if ( ampIDs.find(it->second.first->publicID()) == ampIDs.end() )
			continue;

		ampSet.insert(AmplitudeSet::value_type(it->second.first, true));
	}

	emit localAmplitudesAvailable(_origin.get(), &ampSet, &ampIDs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Magnitude *
MagnitudeView::computeStationMagnitudes(const string &magType,
                                        QList<Seiscomp::DataModel::AmplitudePtr> *amps,
                                        MagnitudeStats *errors) {
	Processing::MagnitudeProcessorPtr magProc = Processing::MagnitudeProcessorFactory::Create(magType.c_str());
	if ( !magProc ) return NULL;

	string ampType = magProc->amplitudeType();
	bool addMag = false;

	MagnitudePtr mag;
	QList<Seiscomp::DataModel::AmplitudePtr> tmpAmps;

	SEISCOMP_DEBUG("Origin.stationMags before: %d", (int)_origin->stationMagnitudeCount());

	// Remove all station magnitudes of origin with requested type
	for ( size_t i = 0; i < _origin->stationMagnitudeCount(); ) {
		if ( _origin->stationMagnitude(i)->type() == magType )
			_origin->removeStationMagnitude(i);
		else
			++i;
	}

	SEISCOMP_DEBUG("Origin.stationMags after: %d", (int)_origin->stationMagnitudeCount());

	for ( size_t i = 0; i < _origin->magnitudeCount(); ++i ) {
		if ( _origin->magnitude(i)->type() == magType ) {
			mag = _origin->magnitude(i);
			break;
		}
	}

	if ( !mag ) {
		mag = Magnitude::Create();
		mag->setType(magType);
		addMag = true;
	}
	else {
		// Remove all stationmagnitude references from magnitude
		while ( mag->stationMagnitudeContributionCount() > 0 )
			mag->removeStationMagnitudeContribution(0);

		SEISCOMP_DEBUG("Mag.stationMagRefs after: %d",
		               (int)mag->stationMagnitudeContributionCount());
	}

	CreationInfo ci;
	ci.setAgencyID(SCApp->agencyID());
	ci.setAuthor(SCApp->author());
	ci.setCreationTime(Core::Time::GMT());

	mag->setCreationInfo(ci);
	mag->setEvaluationStatus(Core::None);
	mag->setOriginID("");

	if ( amps == NULL ) {
		// Typedef a pickmap that maps a streamcode to a pick
		typedef QMap<string, PickCPtr> PickStreamMap;

		// This map is needed to find the earliest P pick of
		// a certain stream
		PickStreamMap pickStreamMap;

		// Fetch all picks to get amplitudes for
		for ( size_t i = 0; i < _origin->arrivalCount(); ++i ) {
			Arrival *ar = _origin->arrival(i);

			double weight = 1.;
			try { weight = ar->weight(); } catch (Seiscomp::Core::ValueException &) {}

			if ( Util::getShortPhaseName(ar->phase().code()) != 'P' || weight < 0.5 )
				continue;

			Pick *pick = Pick::Find(ar->pickID());
			if ( !pick ) continue;

			DataModel::WaveformStreamID wfid = pick->waveformID();
			// Strip the component code because every AmplitudeProcessor
			// will use its own component to pick the amplitude on
			wfid.setChannelCode(wfid.channelCode().substr(0,2));

			string streamID = waveformIDToStdString(wfid);
			PickCPtr p = pickStreamMap[streamID];

			// When there is already a pick registered for this stream which has
			// been picked earlier, ignore the current pick
			if ( p && p->time().value() < pick->time().value() )
				continue;

			pickStreamMap[streamID] = pick;
		}

		// Fetch all amplitudes for all picks
		for ( PickStreamMap::iterator it = pickStreamMap.begin(); it != pickStreamMap.end(); ++it ) {
			string streamID = it.key();
			PickCPtr pick = it.value();

			// The amplitude map contains always the newest manual amplitudes
			pair<PickAmplitudeMap::iterator, PickAmplitudeMap::iterator> itp;
			itp = _amplitudes.equal_range(pick->publicID());

			bool gotAmplitude = false;

			for ( PickAmplitudeMap::iterator it = itp.first; it != itp.second; ++it ) {
				AmplitudePtr amp = it->second.first;

				// Skip unrequestes amplitude types
				if ( amp->type() != ampType ) continue;

				// Use this amplitude
				tmpAmps.append(amp);
				gotAmplitude = true;

				break;
			}

			if ( gotAmplitude ) continue;

			if ( _reader ) {
				DatabaseIterator it = _reader->getAmplitudesForPick(pick->publicID());
				for ( ; *it; ++it ) {
					AmplitudePtr amp = Amplitude::Cast(*it);
					if ( !amp ) continue;

					// Save to cache
					_amplitudes.insert(PickAmplitudeMap::value_type(amp->pickID(), AmplitudeEntry(amp, false)));

					// Skip unrequestes amplitude types
					if ( amp->type() != ampType ) continue;

					// Use this amplitude
					tmpAmps.append(amp);
					gotAmplitude = true;
				}
			}

			if ( gotAmplitude ) continue;

			EventParameters *ep = EventParameters::Cast(PublicObject::Find("EventParameters"));
			if ( ep ) {
				for ( size_t i = 0; i < ep->amplitudeCount(); ++i ) {
					Amplitude *amp = ep->amplitude(i);
					if ( amp->pickID() != pick->publicID() ) continue;
					if ( amp->type() != ampType ) continue;

					// Use this amplitude
					tmpAmps.append(amp);
					gotAmplitude = true;
				}
			}
		}

		amps = &tmpAmps;
	}

	if ( amps ) {
		QList<Seiscomp::DataModel::AmplitudePtr>::iterator it;
		for ( it = amps->begin(); it != amps->end(); ++it ) {
			AmplitudePtr amp = *it;
			if ( amp->type() != ampType ) continue;

			// Create a magnitude processor for each amplitude. Basically this is
			// done for safety reasons since each station might have another configuration
			// set after setup is called.
			magProc = Processing::MagnitudeProcessorFactory::Create(magType.c_str());
			if ( !magProc ) {
				cerr << amp->waveformID().networkCode() << "."
				     << amp->waveformID().stationCode() << ": unable to create magnitude processor "
				     << magType << ": ignoring station" << endl;
				continue;
			}

			Util::KeyValuesPtr keys = getParams(amp->waveformID().networkCode(),
			                                    amp->waveformID().stationCode());
			if ( !magProc->setup(
				Processing::Settings(
					SCApp->configModuleName(),
					amp->waveformID().networkCode(), amp->waveformID().stationCode(),
					amp->waveformID().locationCode(), amp->waveformID().channelCode().substr(0,2),
					&SCCoreApp->configuration(), keys.get())) ) {
				cerr << amp->waveformID().networkCode() << "."
				     << amp->waveformID().stationCode() << ": setup magnitude processor failed"
				     << ": ignoring station" << endl;
				continue;
			}

			double dist, az;

			// Compute station distance
			SensorLocation *loc =  Client::Inventory::Instance()->getSensorLocation(
				amp->waveformID().networkCode(), amp->waveformID().stationCode(),
				amp->waveformID().locationCode(), amp->timeWindow().reference());

			if ( loc == NULL ) {
				SEISCOMP_ERROR("Failed to get meta data for %s.%s.%s",
				               amp->waveformID().networkCode().c_str(),
				               amp->waveformID().stationCode().c_str(),
				               amp->waveformID().locationCode().empty() ? "--" : amp->waveformID().locationCode().c_str());
				continue;
			}

			try {
				double baz;
				Math::Geo::delazi(loc->latitude(), loc->longitude(),
				                  _origin->latitude(), _origin->longitude(),
				                  &dist, &az, &baz);
			}
			catch ( Core::GeneralException &e ) {
				SEISCOMP_ERROR("Magnitude distance computation: %s", e.what());
				continue;
			}

			double magValue;
			double period = 0, snr = 0;
			bool passedQC = true;

			try { period = amp->period().value(); } catch ( ... ) {}
			try { snr = amp->snr(); } catch ( ... ) {}

			Processing::MagnitudeProcessor::Status stat =
				magProc->computeMagnitude(
					amp->amplitude().value(), amp->unit(), period, snr,
					dist, _origin->depth(), _origin.get(), loc, amp.get(), magValue
				);

			if ( stat != Processing::MagnitudeProcessor::OK ) {
				SEISCOMP_ERROR("Failed to compute magnitude for station %s: %d (%s)",
				               amp->waveformID().stationCode().c_str(),
				               (int)stat, stat.toString());

				if ( !magProc->treatAsValidMagnitude() ) {
					if ( errors != NULL )
						errors->append(MagnitudeStatus(magProc->type(), amp.get(), stat));
					continue;
				}
				else {
					passedQC = false;
					if ( errors != NULL )
						errors->append(MagnitudeStatus(magProc->type(), amp.get(), stat, true));
				}
			}

			StationMagnitudePtr staMag = StationMagnitude::Create();
			CreationInfo ci;
			ci.setAgencyID(SCApp->agencyID());
			ci.setAuthor(SCApp->author());
			ci.setCreationTime(Core::Time::GMT());
			staMag->setPassedQC(passedQC);
			staMag->setType(mag->type());
			staMag->setCreationInfo(ci);
			staMag->setWaveformID(amp->waveformID());
			staMag->setMagnitude(magValue);
			staMag->setAmplitudeID(amp->publicID());

			magProc->finalizeMagnitude(staMag.get());

			_origin->add(staMag.get());

			StationMagnitudeContributionPtr ref = new StationMagnitudeContribution;
			ref->setStationMagnitudeID(staMag->publicID());
			ref->setWeight(passedQC ? 1.0 : 0.0);

			mag->add(ref.get());
		}
	}

	if ( addMag ) {
		if ( mag->stationMagnitudeContributionCount() > 0 )
			_origin->add(mag.get());
		else
			return NULL;
	}

	return mag.get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::computeMagnitude(DataModel::Magnitude *magnitude,
                                     const std::string &aggType) {
	vector<double> mags;
	vector<double> weights;
	vector<StationMagnitudeContribution*> stamags, stamagsZeroWeight;

	for ( size_t i = 0; i < magnitude->stationMagnitudeContributionCount(); ++i ) {
		StationMagnitude *sm = StationMagnitude::Find(magnitude->stationMagnitudeContribution(i)->stationMagnitudeID());

		try {
			if ( !sm->passedQC() ) {
				stamagsZeroWeight.push_back(magnitude->stationMagnitudeContribution(i));
				continue;
			}
		}
		catch ( ... ) {}

		stamags.push_back(magnitude->stationMagnitudeContribution(i));
		mags.push_back(sm->magnitude().value());
	}

	int staCount = 0;
	double netmag = INVALID_MAG;
	OPT(double) stdev = 0.0;

	if ( !stamags.empty() ) {
		bool fallback = true;

		if ( aggType == "mean" ) {
			Math::Statistics::computeMean(mags, netmag, *stdev);
			weights.resize(mags.size(), 1.);
			magnitude->setMethodID("mean");
			fallback = false;
		}
		else if ( aggType == "trimmed mean" ) {
			Math::Statistics::computeTrimmedMean(mags, 25.0, netmag, *stdev, &weights);
			magnitude->setMethodID("trimmed mean");
			fallback = false;
		}
		else if ( aggType == "median" ) {
			netmag = Math::Statistics::median(mags);
			if ( mags.size() > 1 ) {
				*stdev = 0;
				for ( size_t i = 0; i < mags.size(); ++i )
					*stdev += (mags[i] - netmag) * (mags[i] - netmag);
				*stdev /= mags.size()-1;
				*stdev = sqrt(*stdev);
			}

			weights.resize(mags.size(), 1.);
			magnitude->setMethodID("median");
			fallback = false;
		}

		if ( fallback ) {
			if ( mags.size() < 4 ) {
				Math::Statistics::computeMean(mags, netmag, *stdev);
				weights.resize(mags.size(), 1.);
				magnitude->setMethodID("mean");
			}
			else {
				Math::Statistics::computeTrimmedMean(mags, 25.0, netmag, *stdev, &weights);
				magnitude->setMethodID("trimmed mean");
			}
		}

		for ( size_t i = 0; i < stamags.size(); ++i ) {
			StationMagnitudeContribution *ref = stamags[i];
			ref->setWeight(weights[i]);
			ref->setResidual(mags[i]-netmag);

			if ( weights[i] > 0.0 )
				++staCount;
		}
	}
	else
		magnitude->setEvaluationStatus(EvaluationStatus(REJECTED));

	magnitude->setMagnitude(DataModel::RealQuantity(netmag, stdev, Core::None, Core::None, Core::None));
	magnitude->setStationCount(staCount);

	for ( size_t i = 0; i < stamagsZeroWeight.size(); ++i ) {
		StationMagnitudeContribution *ref = stamagsZeroWeight[i];
		StationMagnitude *sm = StationMagnitude::Find(ref->stationMagnitudeID());

		ref->setWeight(0.0);
		if ( staCount )
			ref->setResidual(sm->magnitude().value()-netmag);
		else
			ref->setResidual(Core::None);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeView::editSelectionFilter() {
	MagnitudeRowFilter dlg(&selectionFilter());
	return dlg.exec() == QDialog::Accepted;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! called, after selection made in diagram
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::magnitudesSelected() {
	// get selected Rect from diagram
	QRectF brect = _stamagnitudes->getSelectedValuesRect();

	// if nothing selected --> unCheck all everywhere
	if ( brect.isEmpty() && !brect.isNull() ) {
		for ( int i = 0; i < _modelStationMagnitudes.rowCount(); ++i )
			changeStationState(i, false);
		return;
	}

	// prevent loop, when changing table values [BEGIN]
	disconnect(&_modelStationMagnitudes, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
	        this, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));

	// set "used" checkBox in table and change color in map
	QVector<int> selectedIds = _stamagnitudes->getSelectedValues();
	int startIndex = 0;
	for ( int i = 0; i < selectedIds.count(); ++i ) {
		for ( int j = startIndex; j < selectedIds[i]; ++j ){
			_modelStationMagnitudes.setData(_modelStationMagnitudes.index(j, USED), Qt::Unchecked, Qt::CheckStateRole);
			if ( _map )
				_map->setMagnitudeState(j, false);
		}
		_modelStationMagnitudes.setData(_modelStationMagnitudes.index(selectedIds[i], USED), Qt::Checked, Qt::CheckStateRole);
		if ( _map )
			_map->setMagnitudeState(selectedIds[i], true);
		startIndex = selectedIds[i]+1;
	}

	for ( int j = startIndex; j < _modelStationMagnitudes.rowCount(); ++j ){
		_modelStationMagnitudes.setData(_modelStationMagnitudes.index(j, USED), Qt::Unchecked, Qt::CheckStateRole);
		if ( _map )
			_map->setMagnitudeState(j, false);
	}

	// prevent loop, when changing table values [END]
	connect(&_modelStationMagnitudes, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
	        this, SLOT(dataChanged(const QModelIndex&, const QModelIndex&)));

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::hoverMagnitude(int id) {
	QWidget *w = (QWidget*)sender();
	if ( id == -1 )
		w->setToolTip("");
	else
		w->setToolTip(
			_modelStationMagnitudes.data(_modelStationMagnitudes.index(id, NETWORK), Qt::DisplayRole).toString() + "." +
			_modelStationMagnitudes.data(_modelStationMagnitudes.index(id, STATION), Qt::DisplayRole).toString());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::selectMagnitude(int id) {
	if ( id == -1 ) return;

	QModelIndex idx = _modelStationMagnitudesProxy->mapFromSource(_modelStationMagnitudes.index(id, 0));
	_ui.tableStationMagnitudes->setCurrentIndex(idx);
	_ui.tableStationMagnitudes->scrollTo(idx);

	if ( _amplitudeView ) {
		_amplitudeView->setCurrentStation(
			_modelStationMagnitudes.data(_modelStationMagnitudes.index(id, NETWORK), Qt::DisplayRole).toString().toStdString(),
			_modelStationMagnitudes.data(_modelStationMagnitudes.index(id, STATION), Qt::DisplayRole).toString().toStdString()
		);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::selectStation(const std::string &net, const std::string &code) {
	if ( _amplitudeView )
		_amplitudeView->setCurrentStation(net, code);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::adjustMagnitudeRect(QRectF& rect) {
	//rect.setLeft(max(0.0, floor(rect.left()*0.1) * 10));
	//rect.setRight((int)(ceil(rect.right()*0.1)) * 10);
	rect.setLeft(max(0.0, double(floor(rect.left()))));
	rect.setRight((int)(ceil(rect.right())));
	rect.setBottom(max(double(max(ceil(abs(rect.bottom())), ceil(abs(rect.top())))), 1.0));
	rect.setTop(-rect.bottom());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::tableStationMagnitudesContextMenuRequested(const QPoint &pos) {
	if ( !_ui.tableStationMagnitudes->selectionModel() ) return;
	if ( !_ui.tableStationMagnitudes->selectionModel()->hasSelection() ) return;
	QMenu menu;
	QAction *actionCopy = menu.addAction("Copy selected rows to clipboard");
	QAction *result = menu.exec(_ui.tableStationMagnitudes->mapToGlobal(pos));
	if ( result == actionCopy )
		SCApp->copyToClipboard(_ui.tableStationMagnitudes);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::tableStationMagnitudesHeaderContextMenuRequested(const QPoint &pos) {
	int count = _ui.tableStationMagnitudes->horizontalHeader()->count();
	QAbstractItemModel *model = _ui.tableStationMagnitudes->horizontalHeader()->model();

	QMenu menu;

	QVector<QAction*> actions(count);

	for ( int i = 0; i < count; ++i ) {
		actions[i] = menu.addAction(model->headerData(i, Qt::Horizontal).toString());
		actions[i]->setCheckable(true);
		actions[i]->setChecked(!_ui.tableStationMagnitudes->horizontalHeader()->isSectionHidden(i));
	}

	QAction *result = menu.exec(_ui.tableStationMagnitudes->horizontalHeader()->mapToGlobal(pos));
	if ( result == NULL ) return;

	int section = actions.indexOf(result);
	if ( section == -1 ) return;

	for ( int i = 0; i < count; ++i )
		colVisibility[i] = actions[i]->isChecked();

	_ui.tableStationMagnitudes->horizontalHeader()->setSectionHidden(section, !colVisibility[section]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::changeMagnitudeState(int id, bool state) {
	_modelStationMagnitudes.setData(_modelStationMagnitudes.index(id, USED), state?Qt::Checked:Qt::Unchecked, Qt::CheckStateRole);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! DISABLED: called, after selection made in map
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::changeStationState(int id, bool state) {
	// set state (un/checked) in table
	_modelStationMagnitudes.setData(_modelStationMagnitudes.index(id, USED), state?Qt::Checked:Qt::Unchecked, Qt::CheckStateRole);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! called, after selection made in table
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::dataChanged(const QModelIndex& topLeft, const QModelIndex&){
	if ( topLeft.column() != 0 ) return;

	bool state = _modelStationMagnitudes.data(topLeft, Qt::CheckStateRole).toInt() == Qt::Checked;

	// set state (color) in diagram
	_stamagnitudes->setValueSelected(topLeft.row(), state);

	// set state (color) in map
	if ( _map )
		_map->setMagnitudeState(topLeft.row(), state);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::selectPreferredMagnitude(int idx) {
	Magnitude *mag = NULL;

	for ( int i = 0; i < _tabMagnitudes->count(); ++i ) {
		TabData d = _tabMagnitudes->tabData(i).value<TabData>();
		d.selected = d.valid && (i == idx);
		_tabMagnitudes->setTabData(i, QVariant::fromValue(d));

		if ( d.selected )
			mag = Magnitude::Find(d.publicID);
		#if QT_VERSION >= 0x040500

		QCheckBox *cb = static_cast<QCheckBox*>(_tabMagnitudes->tabButton(i, QTabBar::LeftSide));
		if ( cb ) {
			cb->blockSignals(true);
			cb->setCheckState(d.selected ? Qt::Checked : Qt::Unchecked);
			cb->blockSignals(false);
		}
		#endif
	}

	emit magnitudeSelected(_origin->publicID().c_str(), mag);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::tabStateChanged(int state) {
#if QT_VERSION >= 0x040500
	if ( state == Qt::Checked ) {
		for ( int i = 0; i < _tabMagnitudes->count(); ++i ) {
			QCheckBox *cb = static_cast<QCheckBox*>(_tabMagnitudes->tabButton(i, QTabBar::LeftSide));
			if ( cb == sender() ) {
				selectPreferredMagnitude(i);
				return;
			}
		}
	}

	selectPreferredMagnitude(-1);
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setDrawGridLines(bool f) {
	_stamagnitudes->setDrawGridLines(f);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setComputeMagnitudesSilently(bool f) {
	_computeMagnitudesSilently = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setMagnitudeTypeSelectionEnabled(bool f) {
	_enableMagnitudeTypeSelection = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::drawStations(bool enable) {
	_map->setDrawStations(enable);
	_map->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setAmplitudeConfig(const AmplitudeView::Config &config) {
	_amplitudeConfig = config;

	if ( _amplitudeView )
		_amplitudeView->setConfig(_amplitudeConfig);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const AmplitudeView::Config &MagnitudeView::amplitudeConfig() const {
	return _amplitudeConfig;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! set origin & event (and if applicable, the preferred magnitude)
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setOrigin(Origin* o, Event *e) {
	if ( _origin == o ) return;

	bool eventChanged = (_event != e) || ( e == NULL);

	_origin = o;
	_event = e;
	_netMag = NULL;
	_amplitudes.clear();

	if ( _amplitudeView )
		_amplitudeView->close();

	if ( _origin == NULL ) {
		_currentMagnitudeTypes = _magnitudeTypes;
		resetContent();
		return;
	}

	// If a new event is set reset the selected magnitudes to default
	if ( eventChanged )
		_currentMagnitudeTypes = _magnitudeTypes;

	reload();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MagnitudeView::showMagnitude(const string &id) {
	for ( int i = 0; i < _tabMagnitudes->count(); ++i ) {
		if ( _tabMagnitudes->tabData(i).value<TabData>().publicID == id ) {
			_tabMagnitudes->setCurrentIndex(i);

			return true;
		}
	}

	/*
	for ( int i = 0; i < _ui.comboMagType->count(); ++i ) {
		if ( _ui.comboMagType->itemData(i).toString() == id.c_str() ) {
			_ui.comboMagType->setCurrentIndex(i);
			return true;
		}
	}
	*/

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::reload() {
	// otherwise display the first magnitude
	_netMag = NULL;
	for ( size_t i = 0; i < _origin->magnitudeCount(); ++i ) {
		MagnitudePtr mag = _origin->magnitude(i);
		_netMag = mag;
		break;
	}

	setContent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::addObject(const QString& parentID, Seiscomp::DataModel::Object* o) {
	// Check whether a new stationmagnitude has been inserted into the current networkmagnitude
	StationMagnitudeContribution* magRef = StationMagnitudeContribution::Cast(o);
	if ( magRef ) {
		// It does not influence the currently displayed magnitude -> dont care
		if ( !_netMag || parentID.toStdString() != _netMag->publicID() )
			return;

		StationMagnitude* staMag = StationMagnitude::Find(magRef->stationMagnitudeID());
		if ( staMag == NULL ) {
			SEISCOMP_DEBUG("Received stationMagnitudeContribution for magnitude '%s' that has not been found",
			               magRef->stationMagnitudeID().c_str());
			return;
		}

		//cout << "Add StationStationMagnitudeContribution for Magnitude '" << _netMag->publicID() << "'" << endl;
		SEISCOMP_DEBUG("NetMag '%s' has %lu StaMags", _netMag->publicID().c_str(), (unsigned long)_netMag->stationMagnitudeContributionCount());
		addStationMagnitude(staMag, _netMag->stationMagnitudeContributionCount()-1);

		return;
	}

	// Check whether a new networkmagnitude has been inserted into the current origin
	Magnitude* netMag = Magnitude::Cast(o);
	if ( netMag ) {
		// Not for now
		if ( !_origin || _origin->publicID() != parentID.toStdString() )
			return;

		//cout << "Added Magnitude '" << netMag->publicID()
		//          << "' to Origin '" << _origin->publicID() << "'" << endl;

		//if ( _ui.comboMagType->count() == 0 )
		if ( _tabMagnitudes->count() == 0 )
			if ( _map ) _map->update();

		addMagnitude(netMag);
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::removeObject(const QString&, Seiscomp::DataModel::Object*) {
	// this shouldn't happen now
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::updateObject(const QString &parentID, Seiscomp::DataModel::Object* o) {
	// Check whether a networkmagnitude has been updated
	Magnitude* netMag = Magnitude::Cast(o);
	if ( netMag ) {
		if ( _origin && _origin->publicID() == parentID.toStdString() ) {
			int idx = findData(_tabMagnitudes, netMag->publicID());
			if ( idx != -1 )
				_tabMagnitudes->setTabText(idx, QString("%1 %2").arg(netMag->type().c_str()).arg(netMag->magnitude().value(), 0, 'f', 2));
		}

		// Not for now
		if ( !_netMag || _netMag->publicID() != netMag->publicID() )
			return;

		SEISCOMP_INFO("Updating networkmagnitude %s", netMag->publicID().c_str());
		updateMagnitudeLabels();
		return;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setReadOnly(bool e) {
	_ui.groupReview->setVisible(!e);

#if QT_VERSION >= 0x040500
	_tabMagnitudes->setTabsClosable(!e);
#endif

	if ( _amplitudeView && e )
		_amplitudeView->close();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::resetPreferredMagnitudeSelection() {
	selectPreferredMagnitude(-1);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::disableRework() {
	resetPreferredMagnitudeSelection();
	setReadOnly(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int MagnitudeView::addMagnitude(Seiscomp::DataModel::Magnitude* netMag) {
	//for ( int i = 0; i < _ui.comboMagType->count(); ++i ) {
	for ( int i = 0; i < _tabMagnitudes->count(); ++i ) {
		if ( _tabMagnitudes->tabData(i).value<TabData>().publicID == netMag->publicID() ) {
		//if ( _ui.comboMagType->itemData(i).toString() == netMag->publicID().c_str() ) {
			SEISCOMP_DEBUG("Magnitude '%s' has been added already", netMag->publicID().c_str());
			return i;
		}
	}

	//_ui.comboMagType->addItem(QString("%1").arg(netMag->type().c_str()), data);
	int tabIndex = _tabMagnitudes->addTab(QString("%1 %2").arg(netMag->type().c_str()).arg(netMag->magnitude().value(), 0, 'f', 2));
	TabData data(netMag->publicID());

	try {
		if ( netMag->evaluationStatus() == REJECTED ) {
			_tabMagnitudes->setTabText(tabIndex, QString("%1 -.--").arg(netMag->type().c_str()));
			_tabMagnitudes->setTabTextColor(tabIndex, palette().color(QPalette::Disabled, QPalette::WindowText));
			_tabMagnitudes->setTabIcon(tabIndex, QIcon(":icons/icons/disabled.png"));
			data.valid = false;
		}
	}
	catch ( ... ) {}

	_tabMagnitudes->setTabData(tabIndex, QVariant::fromValue<TabData>(data));

#if QT_VERSION >= 0x040500
	if ( data.valid ) {
		QCheckBox *btn = new QCheckBox;
		btn->setToolTip(tr("Select this magnitude type as preferred magnitude "
		                   "type when the event will be committed either "
		                   "with additional options or with custom commit profiles."));
		btn->setProperty("tabIndex", tabIndex);
		_tabMagnitudes->setTabButton(tabIndex, QTabBar::LeftSide, btn);
		connect(btn, SIGNAL(stateChanged(int)), this, SLOT(tabStateChanged(int)));
	}

#endif
	if ( tabIndex == _tabMagnitudes->currentIndex() )
		updateContent();

	if ( !_ui.frameMagnitudeTypes->isVisible() )
		_ui.frameMagnitudeTypes->setVisible(true);

	return tabIndex;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! set magnitude combo box and go on
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::setContent() {
	// fill MagComboBox with all magnitudes from origin
	//disconnect(_ui.comboMagType, SIGNAL(currentIndexChanged(int)), this, SLOT(updateContent()));
	disconnect(_tabMagnitudes, SIGNAL(currentChanged(int)), this, SLOT(updateContent()));
	//_ui.comboMagType->clear();
	while ( _tabMagnitudes->count() > 0 ) _tabMagnitudes->removeTab(0);
	emit magnitudeSelected(_origin ? QString(_origin->publicID().c_str()) : QString(), 0);

	_ui.frameMagnitudeTypes->setVisible(false);

	if ( _origin ) {
		for (size_t i = 0; i < _origin->magnitudeCount(); i++)
			addMagnitude(_origin->magnitude(i));

		if ( _netMag ) {
			// set combo box item according to desired netMag
			_tabMagnitudes->setCurrentIndex(findData(_tabMagnitudes, _netMag->publicID()));
		}
	}

	//connect(_ui.comboMagType,SIGNAL(currentIndexChanged(int)), this, SLOT(updateContent()));
	connect(_tabMagnitudes, SIGNAL(currentChanged(int)), this, SLOT(updateContent()));

	// setup map, show all stations in red; associated with netMag in green
	if ( _map ) {
		_map->setOrigin(_origin.get());

		int iLatitude = (int)_origin->latitude();
		int iLongitude = (int)_origin->longitude();

		if (_origin->arrivalCount() > 0){ // if true --> calculate map boundary so, that ALL stations are visible

			// min/max stations coordinates incl. Origin
			double latMin, latMax, lonMin, lonMax;
			calcMinMax(_origin.get(), latMin, latMax, lonMin, lonMax);

			// lower left corner of displaying rectangle, currentOrigin is inside
			double x = lonMin;
			double y = latMin;
			// width and height of displaying rectangle
			double dx = lonMax-x;
			double dy = latMax-y;

			if (dy == 0.0) dx = dy = 1.0;

			double displayRectAspectRatio;
			try{displayRectAspectRatio = dx/dy;}
			catch(...){displayRectAspectRatio = 1.0;}

			double frameMapAspectRatio;
			try {frameMapAspectRatio = (double)_ui.frameMap->width()/(double)_ui.frameMap->height();}
			catch(...){frameMapAspectRatio = 1.0;}

			double ndx, ndy;
			double frame = 0.05; // *100=% of rect width/height

			if ( (displayRectAspectRatio / frameMapAspectRatio) > 1.0)
				ndy = dx / frameMapAspectRatio;
			else
				ndy = dy;

			if ( ndy < 180.0 ){
				_map->canvas().displayRect(QRectF(x-(frame*dx), (y-((ndy-dy)/2))-(frame*ndy), dx+(2*frame*dx), ndy+(2*frame*ndy)));
			}
			else{
				ndx = 180.0 * frameMapAspectRatio;
				_map->canvas().displayRect(QRectF(iLongitude-ndx/2, -90.0, ndx, 180));
			}
		}
		else{ // if false --> center map around origin
			_map->canvas().displayRect(QRectF(iLongitude-30, iLatitude-30, 60, 60));
		}

		_map->update();
	}
	else
		SEISCOMP_ERROR("no Map");

	// Set default aggregation type
	if ( _defaultMagnitudeAggregation ) {
		if ( *_defaultMagnitudeAggregation == "median" )
			_ui.btnMedian->setChecked(true);
		else if ( *_defaultMagnitudeAggregation == "mean" )
			_ui.btnMean->setChecked(true);
		else if ( *_defaultMagnitudeAggregation == "trimmed mean" )
			_ui.btnTrimmedMean->setChecked(true);
	}
	else
		_ui.btnDefault->setChecked(true);

	updateContent();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! reset content to default, disconnect signals
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::resetContent() {
	_ui.frameMagnitudeTypes->setVisible(false);

	// remove all entries in comboBox
	//disconnect(_ui.comboMagType,SIGNAL(currentIndexChanged(int)), this, SLOT(updateContent()));
	disconnect(_tabMagnitudes, SIGNAL(currentChanged(int)), this, SLOT(updateContent()));
	//_ui.comboMagType->clear();
	while ( _tabMagnitudes->count() > 0 ) _tabMagnitudes->removeTab(0);
	emit magnitudeSelected(_origin ? QString(_origin->publicID().c_str()) : QString(), 0);

	_stamagnitudes->clear();
	_stamagnitudes->update();

 	_modelStationMagnitudes.setOrigin(NULL, NULL);

	QAbstractItemModel* m = _ui.tableStationMagnitudes->model();
	if ( m ) delete m;

	_modelStationMagnitudesProxy = new StaMagsSortFilterProxyModel(this);
	_modelStationMagnitudesProxy->setSourceModel(&_modelStationMagnitudes);
	_ui.tableStationMagnitudes->setModel(_modelStationMagnitudesProxy);

	if (_map){
		_map->setOrigin(NULL);
		_map->canvas().displayRect(QRectF(-180.0,-90.0, 360.0, 180.0));
	}

	_ui.labelRegion->setText("Region");

	updateMagnitudeLabels();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! update map, diagram, table and labels
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::updateContent() {
	Regions regions;

	_ui.labelRegion->setText("");

	if (!_origin) return;

	_ui.labelRegion->setText(regions.getRegionName(_origin->latitude(), _origin->longitude()).c_str());

	//  use selection from comboBox for netmagType
	//_netMag = _origin->findMagnitude((_ui.comboMagType->itemData(_ui.comboMagType->currentIndex()).value<QString>()).toAscii().data());
	_netMag = _origin->findMagnitude(_tabMagnitudes->tabData(_tabMagnitudes->currentIndex()).value<TabData>().publicID);
	if ( _map ) {
		_map->setMagnitude(_netMag.get());
		_map->update();
	}

	if ( _netMag ) {
		for ( size_t i = 0; i < _netMag->stationMagnitudeContributionCount(); ++i ) {
			if ( !StationMagnitude::Find(_netMag->stationMagnitudeContribution(i)->stationMagnitudeID()) ) {
				StationMagnitudePtr stamag = (StationMagnitude*)_reader->getObject(StationMagnitude::TypeInfo(), _netMag->stationMagnitudeContribution(i)->stationMagnitudeID());
				if ( stamag )
					_origin->add(stamag.get());
			}
		}
	}

	// clear diagram
	_stamagnitudes->clear();

	// setup model/view table
	_modelStationMagnitudes.setOrigin(_origin.get(), _netMag.get());

	QAbstractItemModel* m = _ui.tableStationMagnitudes->model();
	if ( m ) delete m;

	_modelStationMagnitudesProxy = new StaMagsSortFilterProxyModel(this);
	_modelStationMagnitudesProxy->setSourceModel(&_modelStationMagnitudes);
	_ui.tableStationMagnitudes->setModel(_modelStationMagnitudesProxy);

	_minStationMagnitude = 999.99;
	_maxStationMagnitude = -999.99;

	if ( _netMag ) {
		StationMagnitude* staMagnitude;
		for ( size_t i = 0; i < _netMag->stationMagnitudeContributionCount(); ++i ) {
			staMagnitude = StationMagnitude::Find(_netMag->stationMagnitudeContribution(i)->stationMagnitudeID());
			// set distance in distanceList
			if ( staMagnitude )
				addStationMagnitude(staMagnitude, i);
		}
	}

	// set labels ...
	updateMagnitudeLabels();

	if ( !_netMag ) {
		_ui.groupReview->setEnabled(false);

		// set dist column in table & add Net/Sta-Mag to diagram: dist = addStationMagnitude()
		updateMinMaxMagnitude();
		update();
		return;
	}

	SEISCOMP_DEBUG("selected magnitude: %s with %lu magRefs ", _netMag->publicID().c_str(), (unsigned long)_netMag->stationMagnitudeContributionCount() );
	SEISCOMP_DEBUG("selected Origin          : %s with %lu arrivals", _origin->publicID().c_str(), (unsigned long)_origin->arrivalCount() );

	for ( int i = 0; i < StaMagsListColumns::Quantity; ++i )
		_ui.tableStationMagnitudes->setColumnHidden(i, !colVisibility[i]);

	// update column width in table view
	_ui.tableStationMagnitudes->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	//_ui.tableStationMagnitudes->resizeColumnsToContents();
	_ui.tableStationMagnitudes->resizeRowsToContents();
	_ui.tableStationMagnitudes->sortByColumn(_ui.tableStationMagnitudes->horizontalHeader()->sortIndicatorSection());

	if ( _netMag->stationMagnitudeContributionCount() == 0 ) {
		_ui.groupReview->setEnabled(false);
		updateMinMaxMagnitude();

		try {
			if ( _netMag->evaluationStatus() == REJECTED )
				_ui.groupReview->setEnabled(true);
		}
		catch ( ... ) {}
	}
	else
		_ui.groupReview->setEnabled(true);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::updateMinMaxMagnitude() {
	_ui.labelMinMag->setText("-");
	_ui.labelMaxMag->setText("-");
	if (_minStationMagnitude <= _maxStationMagnitude && _minStationMagnitude > -10 && _maxStationMagnitude < 15) {
		char buf[10];
		snprintf(buf, 10, "%.2f", _minStationMagnitude);
		_ui.labelMinMag->setText(buf);
		snprintf(buf, 10, "%.2f", _maxStationMagnitude);
		_ui.labelMaxMag->setText(buf);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::updateMagnitudeLabels() {
	if ( _netMag ) {
		char buf[10] = "-";
		double netmagval = _netMag->magnitude().value();
		if ( netmagval < 12 )
			snprintf(buf, 10, "%.2f", netmagval);

		_ui.labelMethod->setText(_netMag->methodID().c_str());
		try {
			_ui.labelAgencyID->setText(_netMag->creationInfo().agencyID().c_str());
			_ui.labelAgencyID->setToolTip(_netMag->creationInfo().agencyID().c_str());
		}
		catch(Core::ValueException &) {
			_ui.labelAgencyID->setText("");
			_ui.labelAgencyID->setToolTip("");
		}

		try {
			_ui.labelAuthor->setText(_netMag->creationInfo().author().c_str());
			_ui.labelAuthor->setToolTip(_netMag->creationInfo().author().c_str());
		}
		catch(Core::ValueException &) {
			_ui.labelAuthor->setText("");
			_ui.labelAuthor->setToolTip("");
		}

		try {
			_ui.labelEvaluation->setText(_netMag->evaluationStatus().toString());
		}
		catch(Core::ValueException &) {
			_ui.labelEvaluation->setText("-");
		}

		_ui.labelMagnitude->setText(buf);
		try {
			_ui.labelNumStaMags->setText(QString("%1 (%2)").arg(_netMag->stationCount()).arg(_netMag->stationMagnitudeContributionCount()));
		}
		catch(Core::ValueException &) {
			_ui.labelNumStaMags->setText(QString("-"));
		}

		strcpy(buf, "-");
		try {
			double rms = quantityUncertainty(_netMag->magnitude());
			if (rms<10)
				snprintf(buf, 10, "%.2f", rms);
		}
		catch ( ... ) {}
		_ui.labelRMS->setText(buf);

		if ( (size_t)_stamagnitudes->count() == _netMag->stationMagnitudeContributionCount() ) {
			_stamagnitudes->setUpdatesEnabled(false);
			for ( size_t i = 0; i < _netMag->stationMagnitudeContributionCount(); ++i ) {
				StationMagnitudePtr staMagnitude = StationMagnitude::Find(_netMag->stationMagnitudeContribution(i)->stationMagnitudeID());
				// set distance in distanceList
				if ( staMagnitude ) {
					QPointF p = _stamagnitudes->value(i);
					double residual = staMagnitude->magnitude().value() - netmagval;
					p.setY(residual);
					_stamagnitudes->setValue(i, p);
					try {
						_stamagnitudes->setValueSelected(i, _netMag->stationMagnitudeContribution(i)->weight() > 0.0);
					}
					catch ( ... ) {
						_stamagnitudes->setValueSelected(i, true);
					}

					changeMagnitudeState(i, _stamagnitudes->isValueSelected(i));
				}
			}

			_stamagnitudes->updateBoundingRect();
			QRectF rect(_stamagnitudes->boundingRect());
			QRectF newRect(rect);
			adjustMagnitudeRect(newRect);
			newRect.setLeft(max(0.0, double(rect.left())));

			_stamagnitudes->setDisplayRect(newRect);
			_stamagnitudes->setUpdatesEnabled(true);

			_map->setMagnitude(_netMag.get());
			_map->update();
		}
	}
	else {
		/*
		if ( _ui.labelAgencyID->text().isEmpty() )
			_ui.labelAgencyID->setText("MagTool@GFZ");

		if ( _ui.labelMethod->text().isEmpty() )
			_ui.labelMethod->setText("trimmed mean");
		*/

		_ui.labelNumStaMags->setText("0");
		_ui.labelMagnitude->setText("-");
		_ui.labelMinMag->setText("-");
		_ui.labelMaxMag->setText("-");
		_ui.labelRMS->setText("-");

		_ui.labelAgencyID->setText("");
		_ui.labelAgencyID->setToolTip("");

		_ui.labelAuthor->setText("");
		_ui.labelAuthor->setToolTip("");

		_ui.labelMethod->setText("");
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::addStationMagnitude(Seiscomp::DataModel::StationMagnitude* stationMagnitude, int index) {
	//SEISCOMP_DEBUG("Adding stationMagnitude at index #%d", index);

	// StationMagnitude has been inserted already -> skip it
	if ( index < _stamagnitudes->count() ) {
		SEISCOMP_WARNING("Index out of bounds (%d >= %d), has been added already", index, _stamagnitudes->count());
		return;
	}

	//SEISCOMP_DEBUG("Current model rowCount = %d", _modelStationMagnitudes.rowCount());

	if ( _map )
		_map->addStationMagnitude(stationMagnitude, index);

	while ( _modelStationMagnitudes.rowCount() <= index ) {
		_modelStationMagnitudes.insertRow(_modelStationMagnitudes.rowCount());
		updateMagnitudeLabels();
	}

	double weight = 1.0;
	try {
		weight = _netMag->stationMagnitudeContribution(index)->weight();
	}
	catch ( ... ) {}

	_modelStationMagnitudes.setData(_modelStationMagnitudes.index(index, DISTANCE),
	                                addStationMagnitude(_netMag.get(), stationMagnitude, weight),
	                                Qt::DisplayRole);
	_modelStationMagnitudes.setData(_modelStationMagnitudes.index(index, USED),
	                                Qt::Checked, Qt::CheckStateRole);

	_ui.tableStationMagnitudes->resizeRowToContents(index);

	double value = stationMagnitude->magnitude().value();

	if ( value < _minStationMagnitude )
		_minStationMagnitude = value;

	if ( value > _maxStationMagnitude )
		_maxStationMagnitude = value;

	updateMinMaxMagnitude();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! add StationMag to distance-MagResidual diagram
//! returns the distance: origin-station
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MagnitudeView::addStationMagnitude(DataModel::Magnitude *magnitude,
                                          DataModel::StationMagnitude *stationMagnitude,
                                          double weight) {
	double distance = -999;

	try {
		const WaveformStreamID &wfsID = stationMagnitude->waveformID();

		try {
			StationLocation loc = Client::Inventory::Instance()->stationLocation(
				wfsID.networkCode(), wfsID.stationCode(), _origin->time().value());

			double azi1, azi2;
			Math::Geo::delazi(_origin->latitude(), _origin->longitude(),
			                  loc.latitude, loc.longitude,
			                  &distance, &azi1, &azi2);
		}
		catch ( ValueException& ) {
			SEISCOMP_ERROR("MagnitudeView::addStationMagnitude: Station %s.%s "
			               "not found. Not added.", wfsID.networkCode().c_str(),
			               wfsID.stationCode().c_str());
		}
	}
	catch ( ValueException& ) {
		SEISCOMP_ERROR("MagnitudeView::addStationMagnitude: WaveformID in "
		               "magnitude '%s' not set",
		               stationMagnitude->publicID().c_str());
	}

	double residual = stationMagnitude->magnitude().value() - magnitude->magnitude().value();

	QColor c = SCScheme.colors.arrivals.automatic;
	AmplitudePtr amp = _objCache.get<Amplitude>(stationMagnitude->amplitudeID());
	if ( amp ) {
		try {
			if ( amp->evaluationMode() == DataModel::MANUAL )
				c = SCScheme.colors.arrivals.manual;
		}
		catch ( ... ) {}
	}

	if ( SCScheme.unit.distanceInKM )
		_stamagnitudes->addValue(QPointF(Math::Geo::deg2km(distance), residual), c);
	else
		_stamagnitudes->addValue(QPointF(distance, residual), c);

	_stamagnitudes->setValueSelected(_stamagnitudes->count()-1, weight > 0.0);
	changeMagnitudeState(_stamagnitudes->count()-1, _stamagnitudes->isValueSelected(_stamagnitudes->count()-1));

	QRectF rect(_stamagnitudes->boundingRect());
	QRectF newRect(rect);
	adjustMagnitudeRect(newRect);
	newRect.setLeft(max(0.0, double(rect.left())));

	if ( newRect != _stamagnitudes->displayRect() ) {
		_stamagnitudes->setDisplayRect(newRect);
		_stamagnitudes->update();
	}

	return distance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


//! return pick for arrival
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::Pick* MagnitudeView::getPick(DataModel::Arrival* arrival){

	if (arrival){
		DataModel::Pick* pick = DataModel::Pick::Cast(DataModel::PublicObject::Find(arrival->pickID()));

		if (!pick && _reader)
			pick = DataModel::Pick::Cast(_reader->getObject(DataModel::Pick::TypeInfo(), arrival->pickID() ));

		return pick;
	}
	else
		return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




//! calculate the max/min coordinates for all stations incl. origin
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MagnitudeView::calcMinMax(Seiscomp::DataModel::Origin* o, double& latMin, double& latMax,
                               double& lonMin, double& lonMax ){

	latMin = o->latitude();
	latMax = o->latitude();
	lonMin = o->longitude();
	lonMax = o->longitude();

	double lat, lon;

	for (size_t i = 0; i < o->arrivalCount(); i++){

		DataModel::PickPtr p = getPick(o->arrival(i));

		if (p) {
			try {
				StationLocation loc = Client::Inventory::Instance()->stationLocation(
				p->waveformID().networkCode(),
				p->waveformID().stationCode(),
				p->time() );
				try{lat = loc.latitude;}catch(...){lat = -9999.9;}
				try{lon = loc.longitude;}catch(...){lon = -9999.9;}

				if (lat != -9999.9 && lon != -9999.9){
					latMax = lat>latMax?lat:latMax;
					latMin = lat<latMin?lat:latMin;
					lonMax = lon>lonMax?lon:lonMax;
					lonMin = lon<lonMin?lon:lonMin;
				}
			}
			catch(Core::ValueException& e){
				SEISCOMP_ERROR("While fetching the station location an error occured: %s", e.what());
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



}
}


Q_DECLARE_METATYPE(TabData)
