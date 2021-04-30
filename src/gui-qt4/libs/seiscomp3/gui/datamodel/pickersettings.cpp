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



#include <seiscomp3/gui/datamodel/pickersettings.h>
#include <QHeaderView>


namespace Seiscomp {
namespace Gui {


namespace {


class FilterModel : public QAbstractListModel {
	public:
		typedef QPair<QString, QString> FilterEntry;
		typedef QVector<FilterEntry> FilterList;

		FilterModel(FilterList &data, QObject *parent = 0)
		: QAbstractListModel(parent), _data(data) {}

		int columnCount(const QModelIndex &parent = QModelIndex()) const {
			return 2;
		}

		int rowCount(const QModelIndex &parent = QModelIndex()) const {
			return _data.count();
		}

		QVariant headerData(int section, Qt::Orientation orientation,
		                    int role = Qt::DisplayRole) const {
			if ( role != Qt::DisplayRole )
				return QVariant();
			
			if ( orientation == Qt::Horizontal ) {
				switch ( section ) {
					case 0:
						return "Name";
					case 1:
						return "Filter";
					default:
						return QVariant();
				}
			}
			else
				return section;
		}

		Qt::ItemFlags flags(const QModelIndex &index) const {
			if ( !index.isValid() )
				return Qt::ItemIsEnabled;

			return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
		}

		QVariant data(const QModelIndex &index, int role) const {
			if ( !index.isValid() )
				return QVariant();

			if ( index.row() >= _data.size() )
				return QVariant();

			if ( role == Qt::DisplayRole || role == Qt::EditRole ) {
				switch ( index.column() ) {
					case 0:
						return _data[index.row()].first;
					case 1:
						return _data[index.row()].second;
				}
			}
			
			return QVariant();
		}

		bool setData(const QModelIndex &index, const QVariant &value,
		             int role = Qt::EditRole) {
			if ( index.isValid() && role == Qt::EditRole ) {
				FilterList::value_type v = _data[index.row()];
				switch ( index.column() ) {
					case 0:
						_data[index.row()].first = value.toString();
						break;
					case 1:
						_data[index.row()].second = value.toString();
						break;
					default:
						return false;
				}

				emit dataChanged(index, index);
				return true;
			}
			return false;
		}

		bool insertRows(int position, int rows, const QModelIndex &index = QModelIndex()) {
			beginInsertRows(QModelIndex(), position, position+rows-1);

			for ( int row = 0; row < rows; ++row )
				_data.insert(position, FilterList::value_type());
			
			endInsertRows();
			return true;
		}

		bool removeRows(int position, int rows, const QModelIndex &index = QModelIndex()) {
			beginRemoveRows(QModelIndex(), position, position+rows-1);

			for ( int row = 0; row < rows; ++row )
				_data.remove(position);

			endRemoveRows();
			return true;
		}

	private:
		FilterList &_data;
};

}


PickerSettings::PickerSettings(const OriginLocatorView::Config &c1,
                               const PickerView::Config &c2,
                               const AmplitudeView::Config &c3,
                               QWidget *parent, Qt::WindowFlags f)
: QDialog(parent, f) {
	_ui.setupUi(this);

	_locatorConfig = c1;
	_pickerConfig = c2;
	_amplitudeConfig = c3;

	_pickerFilterModel = new FilterModel(_pickerConfig.filters, this);
	_ui.tableFilter->setModel(_pickerFilterModel);
	_ui.tableFilter->horizontalHeader()->setStretchLastSection(true);

	_amplitudeFilterModel = new FilterModel(_amplitudeConfig.filters, this);
	_ui.tableAFilter->setModel(_amplitudeFilterModel);
	_ui.tableAFilter->horizontalHeader()->setStretchLastSection(true);

	connect(_ui.slPreOffset, SIGNAL(sliderMoved(int)),
	        this, SLOT(adjustPreTime(int)));

	connect(_ui.preTimeEdit, SIGNAL(timeChanged(const QTime&)),
	        this, SLOT(adjustPreSlider(const QTime&)));

	connect(_ui.slPostOffset, SIGNAL(sliderMoved(int)),
	        this, SLOT(adjustPostTime(int)));

	connect(_ui.postTimeEdit, SIGNAL(timeChanged(const QTime&)),
	        this, SLOT(adjustPostSlider(const QTime&)));

	connect(_ui.slMinimumLength, SIGNAL(sliderMoved(int)),
	        this, SLOT(adjustLength(int)));

	connect(_ui.minimumLengthTimeEdit, SIGNAL(timeChanged(const QTime&)),
	        this, SLOT(adjustLengthSlider(const QTime&)));

	connect(_ui.slWaveformAlignment, SIGNAL(sliderMoved(int)),
	        _ui.waveformAlignmentEdit, SLOT(setValue(int)));
	connect(_ui.waveformAlignmentEdit, SIGNAL(valueChanged(int)),
	        _ui.slWaveformAlignment, SLOT(setValue(int)));

	connect(_ui.minimumLengthTimeEdit, SIGNAL(timeChanged(const QTime&)),
	        this, SLOT(adjustLengthSlider(const QTime&)));


	connect(_ui.slAmplitudePreOffset, SIGNAL(sliderMoved(int)),
	        this, SLOT(adjustAmplitudePreTime(int)));

	connect(_ui.preAmplitudeTimeEdit, SIGNAL(timeChanged(const QTime&)),
	        this, SLOT(adjustAmplitudePreSlider(const QTime&)));

	connect(_ui.slAmplitudePostOffset, SIGNAL(sliderMoved(int)),
	        this, SLOT(adjustAmplitudePostTime(int)));

	connect(_ui.postAmplitudeTimeEdit, SIGNAL(timeChanged(const QTime&)),
	        this, SLOT(adjustAmplitudePostSlider(const QTime&)));

	connect(_ui.cbRepickerStart, SIGNAL(toggled(bool)),
	        _ui.editRepickerStart, SLOT(setEnabled(bool)));

	connect(_ui.cbRepickerEnd, SIGNAL(toggled(bool)),
	        _ui.editRepickerEnd, SLOT(setEnabled(bool)));

	connect(_ui.btnAddPickFilter, SIGNAL(clicked()),
	        this, SLOT(addPickFilter()));

	// Connect pick filter signals/slots
	connect(_ui.btnAddPickFilter, SIGNAL(clicked()),
	        this, SLOT(addPickFilter()));

	connect(_ui.btnRemovePickFilter, SIGNAL(clicked()),
	        this, SLOT(removePickFilter()));

	connect(_ui.btnMovePickFilterUp, SIGNAL(clicked()),
	        this, SLOT(movePickFilterUp()));

	connect(_ui.btnMovePickFilterDown, SIGNAL(clicked()),
	        this, SLOT(movePickFilterDown()));

	// Connect amplitude filter signals/slots
	connect(_ui.btnAddAmplitudeFilter, SIGNAL(clicked()),
	        this, SLOT(addAmplitudeFilter()));

	connect(_ui.btnRemoveAmplitudeFilter, SIGNAL(clicked()),
	        this, SLOT(removeAmplitudeFilter()));

	connect(_ui.btnMoveAmplitudeFilterUp, SIGNAL(clicked()),
	        this, SLOT(moveAmplitudeFilterUp()));

	connect(_ui.btnMoveAmplitudeFilterDown, SIGNAL(clicked()),
	        this, SLOT(moveAmplitudeFilterDown()));

	connect(_ui.saveButton, SIGNAL(clicked()),
	        this, SLOT(save()));

	_ui.spinPVel->setValue(_locatorConfig.reductionVelocityP);
	_ui.cbMaplines->setChecked(_locatorConfig.drawMapLines);
	_ui.cbPlotGridlines->setChecked(_locatorConfig.drawGridLines);
	_ui.cbComputeMissingTakeOffAngles->setChecked(_locatorConfig.computeMissingTakeOffAngles);
	_ui.spinAddStationsDistance->setValue(_pickerConfig.defaultAddStationsDistance);
	_ui.cbHideStationsWithoutData->setChecked(_pickerConfig.hideStationsWithoutData);
	_ui.cbHideDisabledStations->setChecked(_pickerConfig.hideDisabledStations);
	_ui.cbIgnoreDisabledStations->setChecked(_pickerConfig.ignoreDisabledStations);

	_ui.cbShowCrossHair->setChecked(_pickerConfig.showCrossHair);
	_ui.cbIgnoreUnconfiguredStations->setChecked(_pickerConfig.ignoreUnconfiguredStations);
	_ui.cbAllComponents->setChecked(_pickerConfig.loadAllComponents);
	_ui.cbLoadAllPicks->setChecked(_pickerConfig.loadAllPicks);
	_ui.cbStrongMotion->setChecked(_pickerConfig.loadStrongMotionData);
	_ui.cbLimitStationCount->setChecked(_pickerConfig.limitStations);
	_ui.spinLimitStationCount->setValue(_pickerConfig.limitStationCount);
	_ui.spinLimitStationCount->setEnabled(_ui.cbLimitStationCount->isChecked());
	_ui.cbShowAllComponents->setChecked(_pickerConfig.showAllComponents);
	_ui.maximumDistanceEdit->setValue(_pickerConfig.allComponentsMaximumStationDistance);
	_ui.frameMaximumDistance->setEnabled(_ui.cbShowAllComponents->isChecked());
	_ui.cbRemoveAllAutomaticStationPicks->setChecked(_pickerConfig.removeAutomaticStationPicks);
	_ui.cbRemoveAllAutomaticPicks->setChecked(_pickerConfig.removeAutomaticPicks);

	_ui.cbUsePerStreamTimeWindow->setChecked(_pickerConfig.usePerStreamTimeWindows);
	_ui.preTimeEdit->setTime(QTime(0, 0, 0, 0).addSecs(_pickerConfig.preOffset.seconds()));
	_ui.postTimeEdit->setTime(QTime(0, 0, 0, 0).addSecs(_pickerConfig.postOffset.seconds()));
	_ui.minimumLengthTimeEdit->setTime(QTime(0, 0, 0, 0).addSecs(_pickerConfig.minimumTimeWindow.seconds()));

	_ui.slWaveformAlignment->setValue(_pickerConfig.alignmentPosition*100);
	_ui.waveformAlignmentEdit->setValue(_pickerConfig.alignmentPosition*100);

	_ui.editIntegrationPreFilter->setText(_pickerConfig.integrationFilter);
	_ui.checkIntegrationPreFilterOnce->setChecked(_pickerConfig.onlyApplyIntegrationFilterOnce);

	_ui.editRecordSource->setText(_pickerConfig.recordURL);

	_ui.preAmplitudeTimeEdit->setTime(QTime(0, 0, 0, 0).addSecs(_amplitudeConfig.preOffset.seconds()));
	_ui.postAmplitudeTimeEdit->setTime(QTime(0, 0, 0, 0).addSecs(_amplitudeConfig.postOffset.seconds()));

	QFont font;

	font = _ui.labelRemoveStationPicksInfo->font();
	font.setPointSize(font.pointSize()-1);
	_ui.labelRemoveStationPicksInfo->setFont(font);

	font = _ui.labelRemoveAllPicksInfo->font();
	font.setPointSize(font.pointSize()-1);
	_ui.labelRemoveAllPicksInfo->setFont(font);

	/*
	// Deactivate this feature for now
	_ui.cbLimitStationCount->setVisible(false);
	_ui.spinLimitStationCount->setVisible(false);
	_ui.labelLimitStations->setVisible(false);
	*/

	PickerView::Config::UncertaintyProfiles::iterator it;
	for ( it = _pickerConfig.uncertaintyProfiles.begin();
	      it != _pickerConfig.uncertaintyProfiles.end(); ++it )
		_ui.listPickUncertainties->addItem(it.key());
	_ui.listPickUncertainties->setCurrentIndex(0);

	_ui.listPickUncertainties->setEnabled(_ui.listPickUncertainties->count() > 0);
	_ui.labelPickUncertainties->setEnabled(_ui.listPickUncertainties->count() > 0);

	if ( _pickerConfig.repickerSignalStart ) {
		_ui.cbRepickerStart->setChecked(true);
		_ui.editRepickerStart->setValue(*_pickerConfig.repickerSignalStart);
	}

	if ( _pickerConfig.repickerSignalEnd ) {
		_ui.cbRepickerEnd->setChecked(true);
		_ui.editRepickerEnd->setValue(*_pickerConfig.repickerSignalEnd);
	}

	_ui.saveButton->setVisible(false);
}


PickerSettings::~PickerSettings() {}


void PickerSettings::setSaveEnabled(bool e) {
	_ui.saveButton->setVisible(e);
}


void PickerSettings::save() {
	_saveSettings = true;
	accept();
}


int PickerSettings::exec() {
	_saveSettings = false;
	return QDialog::exec();
}


bool PickerSettings::saveSettings() const {
	return _saveSettings;
}


void PickerSettings::adjustPreTime(int value) {
	_ui.preTimeEdit->setTime(QTime(0, 0, 0, 0).addSecs(_ui.slPreOffset->value()*60));
}


void PickerSettings::adjustPostTime(int value) {
	_ui.postTimeEdit->setTime(QTime(0, 0, 0, 0).addSecs(_ui.slPostOffset->value()*60));
}


void PickerSettings::adjustLength(int value) {
	_ui.minimumLengthTimeEdit->setTime(QTime(0, 0, 0, 0).addSecs(_ui.slMinimumLength->value()*60));
}


void PickerSettings::adjustPreSlider(const QTime &t) {
	int value = QTime().secsTo(t) / 60;
	_ui.slPreOffset->setValue(value);
}


void PickerSettings::adjustPostSlider(const QTime &t) {
	int value = QTime().secsTo(t) / 60;
	_ui.slPostOffset->setValue(value);
}


void PickerSettings::adjustLengthSlider(const QTime &t) {
	int value = QTime().secsTo(t) / 60;
	_ui.slMinimumLength->setValue(value);
}


void PickerSettings::adjustAmplitudePreTime(int value) {
	_ui.preAmplitudeTimeEdit->setTime(QTime(0, 0, 0, 0).addSecs(_ui.slAmplitudePreOffset->value()*60));
}


void PickerSettings::adjustAmplitudePostTime(int value) {
	_ui.postAmplitudeTimeEdit->setTime(QTime(0, 0, 0, 0).addSecs(_ui.slAmplitudePostOffset->value()*60));
}


void PickerSettings::adjustAmplitudePreSlider(const QTime &t) {
	int value = QTime().secsTo(t) / 60;
	_ui.slAmplitudePreOffset->setValue(value);
}


void PickerSettings::adjustAmplitudePostSlider(const QTime &t) {
	int value = QTime().secsTo(t) / 60;
	_ui.slAmplitudePostOffset->setValue(value);
}


void PickerSettings::addPickFilter() {
	_pickerFilterModel->insertRows(_pickerFilterModel->rowCount(), 1);
	_ui.tableFilter->setCurrentIndex(_pickerFilterModel->index(_pickerFilterModel->rowCount()-1, 0));
}


void PickerSettings::removePickFilter() {
	QModelIndex index = _ui.tableFilter->currentIndex();
	if ( !index.isValid() ) return;
	_pickerFilterModel->removeRows(index.row(), 1);
	if ( index.row() >= _pickerFilterModel->rowCount() )
		_ui.tableFilter->setCurrentIndex(_pickerFilterModel->index(_pickerFilterModel->rowCount()-1, index.column()));
	else
		_ui.tableFilter->setCurrentIndex(index);
}


void PickerSettings::movePickFilterUp() {
	QModelIndex index = _ui.tableFilter->currentIndex();
	int row = index.row();
	if ( row > 0 ) {
		PickerView::Config::FilterList::value_type prev = _pickerConfig.filters[row-1];
		PickerView::Config::FilterList::value_type curr = _pickerConfig.filters[row];
		
		_pickerFilterModel->setData(_pickerFilterModel->index(row-1, 0), curr.first, Qt::EditRole);
		_pickerFilterModel->setData(_pickerFilterModel->index(row-1, 1), curr.second, Qt::EditRole);

		_pickerFilterModel->setData(_pickerFilterModel->index(row, 0), prev.first, Qt::EditRole);
		_pickerFilterModel->setData(_pickerFilterModel->index(row, 1), prev.second, Qt::EditRole);

		_ui.tableFilter->setCurrentIndex(_pickerFilterModel->index(row-1, index.column()));
	}
}


void PickerSettings::movePickFilterDown() {
	QModelIndex index = _ui.tableFilter->currentIndex();
	int row = _ui.tableFilter->currentIndex().row();
	if ( row < _pickerFilterModel->rowCount()-1 ) {
		PickerView::Config::FilterList::value_type next = _pickerConfig.filters[row+1];
		PickerView::Config::FilterList::value_type curr = _pickerConfig.filters[row];
		
		_pickerFilterModel->setData(_pickerFilterModel->index(row+1, 0), curr.first, Qt::EditRole);
		_pickerFilterModel->setData(_pickerFilterModel->index(row+1, 1), curr.second, Qt::EditRole);

		_pickerFilterModel->setData(_pickerFilterModel->index(row, 0), next.first, Qt::EditRole);
		_pickerFilterModel->setData(_pickerFilterModel->index(row, 1), next.second, Qt::EditRole);

		_ui.tableFilter->setCurrentIndex(_pickerFilterModel->index(row+1, index.column()));
	}
}


void PickerSettings::addAmplitudeFilter() {
	_amplitudeFilterModel->insertRows(_amplitudeFilterModel->rowCount(), 1);
	_ui.tableAFilter->setCurrentIndex(_amplitudeFilterModel->index(_amplitudeFilterModel->rowCount()-1, 0));
}


void PickerSettings::removeAmplitudeFilter() {
	QModelIndex index = _ui.tableAFilter->currentIndex();
	if ( !index.isValid() ) return;
	_amplitudeFilterModel->removeRows(index.row(), 1);
	if ( index.row() >= _amplitudeFilterModel->rowCount() )
		_ui.tableAFilter->setCurrentIndex(_amplitudeFilterModel->index(_amplitudeFilterModel->rowCount()-1, index.column()));
	else
		_ui.tableAFilter->setCurrentIndex(index);
}


void PickerSettings::moveAmplitudeFilterUp() {
	QModelIndex index = _ui.tableAFilter->currentIndex();
	int row = index.row();
	if ( row > 0 ) {
		AmplitudeView::Config::FilterList::value_type prev = _amplitudeConfig.filters[row-1];
		AmplitudeView::Config::FilterList::value_type curr = _amplitudeConfig.filters[row];
		
		_amplitudeFilterModel->setData(_amplitudeFilterModel->index(row-1, 0), curr.first, Qt::EditRole);
		_amplitudeFilterModel->setData(_amplitudeFilterModel->index(row-1, 1), curr.second, Qt::EditRole);

		_amplitudeFilterModel->setData(_amplitudeFilterModel->index(row, 0), prev.first, Qt::EditRole);
		_amplitudeFilterModel->setData(_amplitudeFilterModel->index(row, 1), prev.second, Qt::EditRole);

		_ui.tableAFilter->setCurrentIndex(_amplitudeFilterModel->index(row-1, index.column()));
	}
}


void PickerSettings::moveAmplitudeFilterDown() {
	QModelIndex index = _ui.tableAFilter->currentIndex();
	int row = _ui.tableAFilter->currentIndex().row();
	if ( row < _amplitudeFilterModel->rowCount()-1 ) {
		AmplitudeView::Config::FilterList::value_type next = _amplitudeConfig.filters[row+1];
		AmplitudeView::Config::FilterList::value_type curr = _amplitudeConfig.filters[row];
		
		_amplitudeFilterModel->setData(_amplitudeFilterModel->index(row+1, 0), curr.first, Qt::EditRole);
		_amplitudeFilterModel->setData(_amplitudeFilterModel->index(row+1, 1), curr.second, Qt::EditRole);

		_amplitudeFilterModel->setData(_amplitudeFilterModel->index(row, 0), next.first, Qt::EditRole);
		_amplitudeFilterModel->setData(_amplitudeFilterModel->index(row, 1), next.second, Qt::EditRole);

		_ui.tableAFilter->setCurrentIndex(_amplitudeFilterModel->index(row+1, index.column()));
	}
}


PickerView::Config PickerSettings::pickerConfig() const {
	_pickerConfig.recordURL = _ui.editRecordSource->text();

	_pickerConfig.showCrossHair = _ui.cbShowCrossHair->isChecked();
	_pickerConfig.ignoreUnconfiguredStations = _ui.cbIgnoreUnconfiguredStations->isChecked();
	_pickerConfig.loadAllComponents = _ui.cbAllComponents->isChecked();
	_pickerConfig.loadAllPicks = _ui.cbLoadAllPicks->isChecked();
	_pickerConfig.loadStrongMotionData = _ui.cbStrongMotion->isChecked();
	_pickerConfig.limitStations = _ui.cbLimitStationCount->isChecked();
	_pickerConfig.limitStationCount = _ui.spinLimitStationCount->value();
	_pickerConfig.showAllComponents = _ui.cbShowAllComponents->isChecked();
	_pickerConfig.allComponentsMaximumStationDistance = _ui.maximumDistanceEdit->value();
	_pickerConfig.removeAutomaticStationPicks = _ui.cbRemoveAllAutomaticStationPicks->isChecked();
	_pickerConfig.removeAutomaticPicks = _ui.cbRemoveAllAutomaticPicks->isChecked();

	_pickerConfig.usePerStreamTimeWindows = _ui.cbUsePerStreamTimeWindow->isChecked();
	_pickerConfig.preOffset = Core::TimeSpan(QTime().secsTo(_ui.preTimeEdit->time()));
	_pickerConfig.postOffset = Core::TimeSpan(QTime().secsTo(_ui.postTimeEdit->time()));
	_pickerConfig.minimumTimeWindow = Core::TimeSpan(QTime().secsTo(_ui.minimumLengthTimeEdit->time()));

	_pickerConfig.alignmentPosition = _ui.slWaveformAlignment->value()*0.01;
	if ( _pickerConfig.alignmentPosition < 0 )
		_pickerConfig.alignmentPosition = 0;
	else if ( _pickerConfig.alignmentPosition > 1 )
		_pickerConfig.alignmentPosition = 1;

	_pickerConfig.defaultAddStationsDistance = _ui.spinAddStationsDistance->value();
	_pickerConfig.hideStationsWithoutData = _ui.cbHideStationsWithoutData->isChecked();
	_pickerConfig.hideDisabledStations = _ui.cbHideDisabledStations->isChecked();
	_pickerConfig.ignoreDisabledStations =
	_amplitudeConfig.ignoreDisabledStations = _ui.cbIgnoreDisabledStations->isChecked();

	_pickerConfig.uncertaintyProfile = _ui.listPickUncertainties->currentText();

	if ( _ui.cbRepickerStart->isChecked() )
		_pickerConfig.repickerSignalStart = _ui.editRepickerStart->value();
	else
		_pickerConfig.repickerSignalStart = Core::None;

	if ( _ui.cbRepickerEnd->isChecked() )
		_pickerConfig.repickerSignalEnd = _ui.editRepickerEnd->value();
	else
		_pickerConfig.repickerSignalEnd = Core::None;

	_pickerConfig.integrationFilter = _ui.editIntegrationPreFilter->text();
	_pickerConfig.onlyApplyIntegrationFilterOnce = _ui.checkIntegrationPreFilterOnce->isChecked();

	return _pickerConfig;
}


AmplitudeView::Config PickerSettings::amplitudeConfig() const {
	_amplitudeConfig.recordURL = _ui.editRecordSource->text();
	_amplitudeConfig.preOffset = Core::TimeSpan(QTime().secsTo(_ui.preAmplitudeTimeEdit->time()));
	_amplitudeConfig.postOffset = Core::TimeSpan(QTime().secsTo(_ui.postAmplitudeTimeEdit->time()));
	_amplitudeConfig.defaultAddStationsDistance = _ui.spinAddStationsDistance->value();
	_amplitudeConfig.hideStationsWithoutData = _ui.cbHideStationsWithoutData->isChecked();
	_amplitudeConfig.loadStrongMotionData = _ui.cbStrongMotion->isChecked();

	return _amplitudeConfig;
}


OriginLocatorView::Config PickerSettings::locatorConfig() const {
	_locatorConfig.reductionVelocityP = _ui.spinPVel->value();
	_locatorConfig.drawMapLines = _ui.cbMaplines->isChecked();
	_locatorConfig.drawGridLines = _ui.cbPlotGridlines->isChecked();
	_locatorConfig.computeMissingTakeOffAngles = _ui.cbComputeMissingTakeOffAngles->isChecked();
	return _locatorConfig;
}


}

}
