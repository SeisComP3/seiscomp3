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





#ifndef __QCVIEW_H__
#define __QCVIEW_H__


#include <QtGui>
#include <string>

#ifndef Q_MOC_RUN
#include <seiscomp3/datamodel/databasequery.h>
#endif

#include "qcmodel.h"
#include "qcviewconfig.h"
#include "qcitemview.h"

namespace Seiscomp {
namespace Applications {
namespace Qc {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class QcView : public QWidget {
	Q_OBJECT

	public:
		QcView(QcModel* qcModel, QWidget* parent=0, Qt::WFlags f=0);

		~QcView();

		virtual void init();
		void setExpireTime(double time);

		void setRecordStreamURL(const std::string& recordStreamURL);
		const std::string& recordStreamURL() const ;

		void setDatabaseQueryInterface(const DataModel::DatabaseQuery* query);
		const DataModel::DatabaseQuery* databaseQueryInterface() const ;

	public slots:
		void hideFilterWidget(bool hide);
		void setFilterRegExp(const QString& expr);
		void filterRegExpChanged(const QString&);
		void updateStreamCount();

	protected:
		QcModel                        *_qcModel;
		QSortFilterProxyModel          *_qcProxyModel;
		QVBoxLayout                    *_layout;
		QHBoxLayout                    *_layout2;
		QWidget                        *_filterWidget;
		QLabel                         *_lbVSecCount;
		QLineEdit                      *_leFilter;
		QLabel                         *_lbLeFilter;
		QWidget                        *_parent;

		std::string                     _recordStreamURL;
		const DataModel::DatabaseQuery *_dbQuery;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class QcTableView : public QcView {
	Q_OBJECT

	public:
		QcTableView(QcModel* qcModel, QWidget* parent=0, Qt::WFlags f=0);

		~QcTableView();
		void init();
		QTableView* qTableView();

	protected:
		bool eventFilter(QObject* o, QEvent* e);

	public slots:
		void resetTableSorting();
		void alterCornerButton();
		void showStream(int);
		double streamWidgetLength() const;
		void setStreamWidgetLength(double length);

		void showDialog(const QModelIndex& inx);


	private:
		QTableView* _qcTable;
		QAbstractButton* _cornerButton;
		double _streamWidgetLength;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class QcOverView : public QcView {
	Q_OBJECT

	public:
		QcOverView(QcModel* qcModel, QWidget* parent=0, Qt::WFlags f=0);
		~QcOverView();
		void init();

	private:
		QcItemView *_overView;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class QcTableCornerButton : public QPushButton {
	Q_OBJECT

	public:
		QcTableCornerButton(QWidget* parent) : QPushButton(parent) {}

		void paintEvent(QPaintEvent*) {
			QStyleOptionButton opt;
			opt.init(this);
			opt.state = QStyle::State_None;
			opt.features = QStyleOptionButton::Flat;
			opt.rect = rect();
			opt.text = text();
			QPainter painter(this);
			painter.setBackgroundMode(Qt::TransparentMode);
			style()->drawControl(QStyle::CE_PushButton, &opt, &painter, this);
		}

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}
}
}

#endif
