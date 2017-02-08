/***************************************************************************
 *   Copyright (C) by ASGSR
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

#ifndef __SEISCOMP_GUI_SENDEVENT_H__
#define __SEISCOMP_GUI_SENDEVENT_H__

#include <seiscomp3/gui/core/application.h>
#ifndef Q_MOC_RUN
#include <seiscomp3/datamodel/origin.h>
#endif
#include <string.h>


class SendEvent : public Seiscomp::Gui::Application {
	Q_OBJECT

	public:
		SendEvent(int& argc, char **argv, Seiscomp::Gui::Application::Type);

		void createCommandLineDescription();

		bool run();


//	private slots:


	private:

		std::string _eventID;

};

#endif
