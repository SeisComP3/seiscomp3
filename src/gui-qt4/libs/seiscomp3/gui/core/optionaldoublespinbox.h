/***************************************************************************
 *   Copyright (C) gempa GmbH                                              *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 *                                                                         *
 *   Author: Jan Becker                                                    *
 *   Email: jabe@gempa.de                                                  *
 ***************************************************************************/


#ifndef SEISCOMP_GUI_OPTIONALDOUBLESPINBOX_H__
#define SEISCOMP_GUI_OPTIONALDOUBLESPINBOX_H__


#include <seiscomp3/gui/qt4.h>
#include <QDoubleSpinBox>


class QToolButton;


namespace Seiscomp {
namespace Gui {


class SC_GUI_API OptionalDoubleSpinBox : public QDoubleSpinBox {
	Q_OBJECT

	public:
		OptionalDoubleSpinBox(QWidget *parent = 0);

	public:
		/**
		 * @brief Returns whether the value is valid or not. A valid value
		 *        is larger than the defined minimum of the value range.
		 * @return Flag indicating validity
		 */
		bool isValid() const;

	private slots:
		void resetContent();
		void changedContent();


	private:
		QToolButton *_resetButton;
		int          _spacing;
};


}
}


#endif
