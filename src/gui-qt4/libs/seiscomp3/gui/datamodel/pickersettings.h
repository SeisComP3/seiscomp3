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



#ifndef __SEISCOMP_GUI_WAVEFORMSETTINGS_H__
#define __SEISCOMP_GUI_WAVEFORMSETTINGS_H__

#include <seiscomp3/gui/datamodel/ui_pickersettings.h>
#include <seiscomp3/gui/datamodel/pickerview.h>
#include <seiscomp3/gui/datamodel/amplitudeview.h>
#include <seiscomp3/gui/datamodel/originlocatorview.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API PickerSettings : public QDialog {
	Q_OBJECT

	public:
		PickerSettings(const OriginLocatorView::Config &c1,
		               const PickerView::Config &c2,
		               const AmplitudeView::Config &c3,
		               QWidget *parent = 0, Qt::WindowFlags f = 0);
		~PickerSettings();


	public:
		const Ui::PickerSettings &ui() const { return _ui; }

		OriginLocatorView::Config locatorConfig() const;
		PickerView::Config pickerConfig() const;
		AmplitudeView::Config amplitudeConfig() const;

		void setSaveEnabled(bool);

		bool saveSettings() const;

		int exec();


	signals:
		void saveRequested();


	private slots:
		void save();

		void adjustPreTime(int);
		void adjustPostTime(int);
		void adjustLength(int);

		void adjustPreSlider(const QTime&);
		void adjustPostSlider(const QTime&);
		void adjustLengthSlider(const QTime&);

		void adjustAmplitudePreTime(int);
		void adjustAmplitudePostTime(int);

		void adjustAmplitudePreSlider(const QTime&);
		void adjustAmplitudePostSlider(const QTime&);

		void addPickFilter();
		void removePickFilter();

		void movePickFilterUp();
		void movePickFilterDown();

		void addAmplitudeFilter();
		void removeAmplitudeFilter();

		void moveAmplitudeFilterUp();
		void moveAmplitudeFilterDown();


	private:
		Ui::PickerSettings                _ui;
		bool                              _saveSettings;
		QAbstractListModel               *_pickerFilterModel;
		QAbstractListModel               *_amplitudeFilterModel;
		mutable OriginLocatorView::Config _locatorConfig;
		mutable PickerView::Config        _pickerConfig;
		mutable AmplitudeView::Config     _amplitudeConfig;
};


}
}


#endif
