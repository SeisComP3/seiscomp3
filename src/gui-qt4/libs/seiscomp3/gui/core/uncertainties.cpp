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


#include <seiscomp3/gui/core/uncertainties.h>


namespace Seiscomp {
namespace Gui {


EditUncertainties::EditUncertainties(QWidget * parent, Qt::WFlags f)
: QDialog(parent, f) {
	_ui.setupUi(this);

	connect(_ui.cbAsymmetric, SIGNAL(toggled(bool)),
	        _ui.labelUpperUncertainty, SLOT(setEnabled(bool)));
	connect(_ui.cbAsymmetric, SIGNAL(toggled(bool)),
	        _ui.spinUpperUncertainty, SLOT(setEnabled(bool)));
	connect(_ui.cbAsymmetric, SIGNAL(toggled(bool)),
	        this, SLOT(symmetryChanged(bool)));

	connect(_ui.spinLowerUncertainty, SIGNAL(valueChanged(double)),
	        this, SLOT(lowerChanged(double)));
	connect(_ui.spinUpperUncertainty, SIGNAL(valueChanged(double)),
	        this, SLOT(upperChanged(double)));
}


EditUncertainties::~EditUncertainties() {
}


void EditUncertainties::lowerChanged(double d) {
	if ( !_ui.cbAsymmetric->isChecked() ) {
		_ui.spinUpperUncertainty->blockSignals(true);
		_ui.spinUpperUncertainty->setValue(d);
		_ui.spinUpperUncertainty->blockSignals(false);
	}

	emit uncertaintiesChanged(lowerUncertainty(), upperUncertainty());
}


void EditUncertainties::upperChanged(double d) {
	emit uncertaintiesChanged(lowerUncertainty(), upperUncertainty());
}

void EditUncertainties::symmetryChanged(bool) {
	emit uncertaintiesChanged(lowerUncertainty(), upperUncertainty());
}


void EditUncertainties::setUncertainties(double lower, double upper) {
	_ui.spinLowerUncertainty->setValue(lower);
	_ui.spinUpperUncertainty->setValue(upper);

	_ui.cbAsymmetric->setChecked(
		_ui.spinLowerUncertainty->value() != _ui.spinUpperUncertainty->value()
	);
}


double EditUncertainties::lowerUncertainty() const {
	return _ui.spinLowerUncertainty->value();
}


double EditUncertainties::upperUncertainty() const {
	if ( !_ui.cbAsymmetric->isChecked() )
		return lowerUncertainty();

	return _ui.spinUpperUncertainty->value();
}


}
}
