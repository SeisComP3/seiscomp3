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

#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/datamodel/origindialog.h>
#include <iostream>
#include <QTime>
#include <QDate>

#include <seiscomp3/core/datetime.h>


namespace Seiscomp {
namespace Gui {


double OriginDialog::_defaultDepth = 10;


void OriginDialog::SetDefaultDepth(double depth) {
	_defaultDepth = depth;
}


double OriginDialog::DefaultDepth() {
	return _defaultDepth;
}


OriginDialog::OriginDialog(QWidget * parent, Qt::WFlags f) :
 QDialog(parent, f) {
	init(0.0, 0.0, _defaultDepth);
}


OriginDialog::OriginDialog(double lon, double lat,
                           QWidget * parent, Qt::WFlags f) :
 QDialog(parent, f) {
	init(lon, lat, _defaultDepth);
}


OriginDialog::OriginDialog(double lon, double lat, double dep,
                           QWidget* parent, Qt::WFlags f) :
 QDialog(parent, f) {
	init(lon, lat, dep);
}


OriginDialog::~OriginDialog() {}


time_t OriginDialog::getTime_t() const {
	Seiscomp::Core::Time t;
	t.set(_ui.dateTimeEdit->dateTime().date().year(),
	      _ui.dateTimeEdit->dateTime().date().month(),
	      _ui.dateTimeEdit->dateTime().date().day(),
	      _ui.dateTimeEdit->dateTime().time().hour(),
	      _ui.dateTimeEdit->dateTime().time().minute(),
	      _ui.dateTimeEdit->dateTime().time().second(),
	      0);
	return t;
}


void OriginDialog::setTime(Core::Time t) {
	int y = 0, M = 0, d = 0, h = 0, m = 0, s = 0;
	t.get(&y, &M, &d, &h, &m, &s);
	_ui.dateTimeEdit->setTime(QTime(h, m, s));
	_ui.dateTimeEdit->setDate(QDate(y, M, d));
}


double OriginDialog::longitude() const {
	return _ui.lonDoubleSpinBox->value();
}


void OriginDialog::setLongitude(double lon) {
	_ui.lonDoubleSpinBox->setValue(lon);
}


double OriginDialog::latitude() const {
	return _ui.latDoubleSpinBox->value();
}


void OriginDialog::setLatitude(double lat) {
	_ui.latDoubleSpinBox->setValue(lat);
}


double OriginDialog::depth() const {
	return _ui.depthDoubleSpinBox->value();
}


void OriginDialog::setDepth(double dep) {
	_ui.depthDoubleSpinBox->setValue(dep);
}


void OriginDialog::enableAdvancedOptions(bool enable, bool checkable) {
	if ( enable ) {
		_ui.advancedGB->show();
		_ui.advancedGB->setCheckable(checkable);
	}
	else
		_ui.advancedGB->hide();
}


bool OriginDialog::advanced() const {
	return _ui.advancedGB->isChecked();
}


void OriginDialog::setAdvanced(bool checked) {
	_ui.advancedGB->setChecked(checked);
}


int OriginDialog::phaseCount() const {
	return (int) _ui.phaseCountSB->value();
}


void OriginDialog::setPhaseCount(int count) {
	_ui.phaseCountSB->setValue(count);
}


double OriginDialog::magValue() const {
	return _ui.magSB->value();
}


void OriginDialog::setMagValue(double mag) {
	_ui.magSB->setValue(mag);
}


QString OriginDialog::magType() const {
	return _ui.magTypeCB->currentText().trimmed();
}


void OriginDialog::setMagType(const QString &type) {
	int index = _magTypes.indexOf(type);
	if ( index < 0 ) {
		index = 0;
		_ui.magTypeCB->addItem(type);
	}
	_ui.magTypeCB->setCurrentIndex(index);
}


void OriginDialog::setMagTypes(const QStringList &types) {
	QString type = magType();
	_magTypes = types;

	_ui.magTypeCB->clear();
	_ui.magTypeCB->addItems(_magTypes);
	if ( !type.isEmpty() ) {
		setMagType(type);
	}
}


void OriginDialog::setSendButtonText(const QString &text) {
	_ui.sendButton->setText(text);
}


void OriginDialog::loadSettings(const QString &groupName) {
	QSettings &s = SCApp->settings();
	s.beginGroup(groupName);
	setLongitude(s.value("longitude", longitude()).toDouble());
	setLatitude(s.value("latitude", latitude()).toDouble());
	setDepth(s.value("depth", depth()).toDouble());
	setAdvanced(s.value("advanced", advanced()).toBool());
	setPhaseCount(s.value("phaseCount", phaseCount()).toInt());
	setMagValue(s.value("magValue", magValue()).toDouble());
	setMagType(s.value("magType", magType()).toString());
	s.endGroup();
}


void OriginDialog::saveSettings(const QString &groupName) {
	QSettings &s = SCApp->settings();
	s.beginGroup(groupName);
	s.setValue("longitude", longitude());
	s.setValue("latitude", latitude());
	s.setValue("depth", depth());
	s.setValue("advanced", advanced());
	if ( advanced() ) {
		s.setValue("phaseCount", phaseCount());
		s.setValue("magValue", magValue());
		s.setValue("magType", magType());
	}
	s.endGroup();
}


void OriginDialog::init(double lon, double lat, double dep) {
	_ui.setupUi(this);
	_ui.advancedGB->hide();

	setTime(Core::Time::GMT());
	setLongitude(lon);
	setLatitude(lat);
	setDepth(dep);

	setAdvanced(false);
	setPhaseCount(10);
	setMagValue(5.0);
}


} // namesapce Gui
} // namespace Seiscomp
