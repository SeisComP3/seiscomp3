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


#ifndef __ORIGINDIALOG_H__
#define __ORIGINDIALOG_H__

#include <ctime>

#include <QDialog>
#include <QDateTime>
#include <QString>

#include <seiscomp3/gui/datamodel/ui_origindialog.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API OriginDialog : public QDialog
{

	Q_OBJECT

public:
	static void SetDefaultDepth(double depth);
	static double DefaultDepth();

	OriginDialog(QWidget * parent = 0, Qt::WFlags f = 0);
	OriginDialog(double lon, double lat,
	             QWidget * parent = 0, Qt::WFlags f = 0);

	OriginDialog(double lon, double lat, double dep,
	             QWidget * parent = 0, Qt::WFlags f = 0);

	~OriginDialog();

	time_t getTime_t() const;
	void setTime(Core::Time t);

	double longitude() const;
	void setLongitude(double lon);

	double latitude() const;
	void setLatitude(double lat);

	double depth() const;
	void setDepth(double dep);

	// enable advanced group box
	void enableAdvancedOptions(bool enable = true, bool checkable = true);

	bool advanced() const;
	void setAdvanced(bool checked);

	int phaseCount() const;
	void setPhaseCount(int count);

	double magValue() const;
	void setMagValue(double mag);

	QString magType() const;
	void setMagType(const QString &type);
	void setMagTypes(const QStringList &types);

	void setSendButtonText(const QString &text);

	void loadSettings(const QString &groupName = "OriginDialog");
	void saveSettings(const QString &groupName = "OriginDialog");

private:
	void init(double lon, double lat, double dep);

private:
	static double _defaultDepth;

	Ui::OriginDialog _ui;
	QStringList      _magTypes;
};


} // namespace Gui
} // namespace Seiscomp

#endif
