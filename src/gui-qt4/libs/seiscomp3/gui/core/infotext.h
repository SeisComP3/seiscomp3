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



#ifndef __SEISCOMP_GUI_INFOTEXT_H__
#define __SEISCOMP_GUI_INFOTEXT_H__

#include <seiscomp3/gui/core/ui_infotext.h>
#include <seiscomp3/gui/qt4.h>

#include <QDialog>

namespace Seiscomp {
namespace Gui {


class SC_GUI_API InfoText : public QDialog {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		InfoText(QWidget* parent = 0, Qt::WindowFlags f = 0);
		~InfoText();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		void setText(const QString& text);
		void appendText(const QString& text);

		void setReadOnly(bool readOnly);
		bool isReadOnly() const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	private:
		Ui::InfoText _ui;
};


}
}


#endif
