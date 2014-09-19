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




#include <QMainWindow>
#include <QStatusBar>
#include <QLabel>
#include <QString>


#include "progressbar.h"
#include "mainwindow.h"

/*
void
PickerMainWindow::setupStatusBar()
{
	statusBar();
	statusBar()->setSizeGripEnabled(false);

	statusBarFile   = new QLabel;
	statusBarFilter = new QLabel(" Filter OFF ");
	statusBarProg   = new MyProgressBar;

	statusBar()->addPermanentWidget(statusBarFilter, 1);
	statusBar()->addPermanentWidget(statusBarFile,   5);
	statusBar()->addPermanentWidget(statusBarProg,   1);
}

void
PickerMainWindow::statusBarUpdate()
{
	if (statusBarFilter)
		statusBarFilter->setText(filtering ?
					 preferences.filter.info() :
					 QString(" Filter OFF "));
	if (statusBarFile) {
		QString str;
		str.sprintf(" %d traces ", data.size());
		statusBarFile->setText(str);
	}
}

void
PickerMainWindow::statusBarShowProgress(float f)
{
	//  cerr<<i<<endl;
	//  statusBarProg->setProgress(i/100.);
	statusBarProg->setValue(f/100.);
	//  statusBarProg->update();
}
*/
