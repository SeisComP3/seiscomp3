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



#ifndef __SEISCOMP_GUI_QUESTIONBOX_H__
#define __SEISCOMP_GUI_QUESTIONBOX_H__


#include <seiscomp3/gui/core/ui_questionbox.h>
#include <seiscomp3/gui/qt4.h>

namespace Seiscomp {
namespace Gui {


class SC_GUI_API QuestionBox {
	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		QuestionBox(QWidget* parent = 0, Qt::WindowFlags = 0);

		~QuestionBox();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		void setText(const QString& text);
		QString text() const;

		void setInfo(const QString& text);
		QString info() const;


	// ------------------------------------------------------------------
	//  Reimplemented derived interface
	// ------------------------------------------------------------------
	public:
		int exec();


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	private:
		QWidget* _parent;
		Qt::WindowFlags _flags;
		QString _text;
		QString _info;

		bool _show;
		int _lastResult;
};


}
}


#endif
