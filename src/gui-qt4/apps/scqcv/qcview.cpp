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




#define SEISCOMP_COMPONENT Gui::QcView

#include "qcview.h"

#include <seiscomp3/logging/log.h>

#include <seiscomp3/gui/core/streamwidget.h>
#include <seiscomp3/datamodel/waveformstreamid.h>

#include <QCheckBox>
#include <QHeaderView>
#include <QLineEdit>
#include <QMessageBox>
#include <QTableView>

using namespace Seiscomp::Gui;

namespace Seiscomp {
namespace Applications {
namespace Qc {

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class QcSortFilterProxyModel : public QSortFilterProxyModel {
	public:
		QcSortFilterProxyModel(QObject* obj=0) 
		:	QSortFilterProxyModel(obj) {}

		bool lessThan(const QModelIndex &left, const QModelIndex &right) const {
			if ( sourceModel()->data(left).type() == QVariant::Invalid )
				return true;
			if ( sourceModel()->data(right).type() == QVariant::Invalid )
				return false;

			return QSortFilterProxyModel::lessThan(left, right);
		}
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QcView::QcView(QcModel* qcModel, QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f) {
	_qcModel = qcModel;
	_qcProxyModel = new QcSortFilterProxyModel(this);
	_qcProxyModel->setSourceModel(_qcModel);
	_qcProxyModel->setDynamicSortFilter(true); //! >= Qt-4.2
	_qcProxyModel->setSortRole(Qt::UserRole);
	_qcProxyModel->setFilterKeyColumn(0);

	_layout = new QVBoxLayout(this);
	_layout->setMargin(2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
QcView::~QcView(){}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcView::init() {
	_filterWidget = new QWidget(this);
	_layout->addWidget(_filterWidget);

	_layout2 = new QHBoxLayout;

	_leFilter = new QLineEdit;

	_lbLeFilter = new QLabel("StreamID Filter: ");
	_lbLeFilter->setToolTip("enter a regular expression");

	_lbVSecCount = new QLabel("");

	_layout2->addWidget(_lbLeFilter);
	_layout2->addWidget(_leFilter);
	_layout2->addWidget(_lbVSecCount);
	_layout2->addStretch();

	_filterWidget->setLayout(_layout2);

	_filterWidget->show();

	connect(_leFilter, SIGNAL(textChanged(const QString&)), this, SLOT(filterRegExpChanged(const QString&)));
	connect(_qcProxyModel, SIGNAL(layoutChanged()), this, SLOT(updateStreamCount()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcView::setRecordStreamURL(const std::string& recordStreamURL) {
	_recordStreamURL = recordStreamURL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& QcView::recordStreamURL() const {
	return _recordStreamURL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcView::setDatabaseQueryInterface(const DataModel::DatabaseQuery* dbQuery) {
	_dbQuery = dbQuery;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const DataModel::DatabaseQuery* QcView::databaseQueryInterface() const {
	return _dbQuery;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcView::setExpireTime(double time) {
	_qcModel->setCleanUpTime(time);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcView::hideFilterWidget(bool hide){
	hide?_filterWidget->hide():_filterWidget->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcView::setFilterRegExp(const QString& expr){
	_leFilter->setText(expr);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcView::filterRegExpChanged(const QString& filter) {
	QRegExp regExp(filter, Qt::CaseInsensitive, QRegExp::RegExp);
	_qcProxyModel->setFilterRegExp(regExp);

	updateStreamCount();
 }
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcView::updateStreamCount() {
	_lbVSecCount->setText(QString(" %1 / %2 streams listed")
	             .arg(_qcProxyModel->rowCount())
	             .arg(_qcModel->rowCount()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QcTableView::QcTableView(QcModel* qcModel, QWidget* parent, Qt::WindowFlags f)
: QcView(qcModel, parent, f) {
	_cornerButton = NULL;
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
QcTableView::~QcTableView(){}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcTableView::init() {
	_qcTable = new QTableView();
	_qcTable->verticalHeader()->setDefaultSectionSize(fontMetrics().height()+6);
	_qcTable->verticalHeader()->setDefaultAlignment(Qt::AlignLeft);
	_qcTable->setModel(_qcProxyModel);
	_qcTable->hideColumn(0);
	_qcTable->sortByColumn(0, Qt::AscendingOrder); 
	_qcTable->setSortingEnabled(true);
#if QT_VERSION >= 0x050000
	_qcTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	_qcTable->horizontalHeader()->setSectionsMovable(true);
#else
	_qcTable->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
	_qcTable->horizontalHeader()->setMovable(true);
#endif
	_qcTable->horizontalHeader()->setTextElideMode(Qt::ElideRight);
	_qcTable->horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);

	_qcTable->setSelectionMode(QAbstractItemView::NoSelection);

	QcItemDelegate* itemDelegate =  new QcItemDelegate(0);
	itemDelegate->setModel(_qcProxyModel);
	_qcTable->setItemDelegate(itemDelegate);


	//! set the Corner Widget
	_cornerButton = _qcTable->findChild<QAbstractButton*>();
	if (_cornerButton) {
		_cornerButton = new QcTableCornerButton(_qcTable);
		_cornerButton->setText("streamID");
		_cornerButton->hide();
		_cornerButton->setEnabled(true);
		_cornerButton->installEventFilter(this);
		connect(_cornerButton, SIGNAL(pressed()), this, SLOT(resetTableSorting()));
	}

	_layout->addWidget(_qcTable);

	QcView::init();

	connect(_qcModel, SIGNAL(modelReset()), this, SLOT(alterCornerButton()));
	connect(_leFilter, SIGNAL(textChanged(const QString&)), this, SLOT(alterCornerButton()));

	connect(_qcTable->verticalHeader(), SIGNAL(sectionPressed(int)), this, SLOT(showStream(int)));

	_streamWidgetLength = _qcModel->config()->streamWidgetLength();

	connect(_qcTable, SIGNAL(pressed(const QModelIndex&)), this, SLOT(showDialog(const QModelIndex&)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QTableView* QcTableView::qTableView() {
	return _qcTable;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcTableView::showStream(int sec) {
	//! translate: proxyModelIndex --> ModelIndex
	QModelIndex index = _qcProxyModel->index(sec, 0);
	QModelIndex rawindex(_qcProxyModel->mapToSource(index));

	std::string streamID = _qcModel->getKey(rawindex).toLatin1().data();

	Gui::StreamWidget* streamView = new Gui::StreamWidget(
                       _recordStreamURL,
                       streamID,
                       _streamWidgetLength,
                       this);

	streamView->show();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcTableView::setStreamWidgetLength(double length){
	_streamWidgetLength = length;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double QcTableView::streamWidgetLength() const {
	return _streamWidgetLength;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcTableView::alterCornerButton() {
	if (_qcProxyModel->rowCount() == 0)
		_cornerButton->hide();
	else {
		_cornerButton->show();
		_qcTable->horizontalHeader()->hideSection(0);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//! install eventFilter for the Table Corner Widget
bool QcTableView::eventFilter(QObject* o, QEvent* e) {
	if (e->type() == QEvent::Paint){
		QcTableCornerButton* btn = qobject_cast<QcTableCornerButton*>(o);
			
		if (btn) {
			btn->paintEvent(static_cast<QPaintEvent*>(e));
			return true;
		}
	}
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

void QcTableView::showDialog(const QModelIndex& index) {
	QModelIndex inx(_qcProxyModel->mapToSource(index));

	if (inx.column() != 1) return;

	// show the enable/disable message box
	QMessageBox msgBox;

	msgBox.setWindowTitle(QString("%1").arg(_qcModel->getKey(inx)));

	bool state = _qcModel->streamEnabled(inx);
	QString what = "Enable";
	if (state)
		what = "Do you really want to disable";
	msgBox.setText(QString("%1 stream?").arg(what));
		
	msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	switch (msgBox.exec()) {
	case QMessageBox::Yes:
		_qcModel->setStreamEnabled(inx, !state);
		break;
	case QMessageBox::No:
		// do nothing
		break;
	default:
		// should never be reached
		break;
	}
}



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcTableView::resetTableSorting() {
	_qcTable->sortByColumn(0, Qt::AscendingOrder);
 }
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
QcOverView::QcOverView(QcModel* qcModel, QWidget* parent, Qt::WindowFlags f)
: QcView(qcModel, parent, f) {
	init();
}
QcOverView::~QcOverView(){}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcOverView::init() {
	_overView = new QcItemView(this);
	_overView->setModel(_qcProxyModel);

	QcItemDelegate* itemDelegate =  new QcItemDelegate(1);
	itemDelegate->setModel(_qcProxyModel);
	_overView->setItemDelegate(itemDelegate);


	_layout->addWidget(_overView);

	QCheckBox* compactView = new QCheckBox("compact view");
	compactView->setToolTip("switch between compact view of streams and network separated view");
	compactView->setCheckState(Qt::Checked);
	connect(compactView, SIGNAL(stateChanged(int)), _overView, SLOT(setCompactView(int)));

	QcView::init();

	_layout2->addWidget(compactView);

	connect(_leFilter, SIGNAL(textChanged(const QString&)), _overView, SLOT(reset()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}

