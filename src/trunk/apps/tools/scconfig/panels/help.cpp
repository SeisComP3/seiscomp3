/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#include "help.h"
#include <seiscomp3/system/environment.h>

#include <iostream>


using namespace std;


#define TYPEROLE Qt::UserRole+1
#define PATHROLE Qt::UserRole+2


namespace {


/*
class IconItemDelegate : public QStyledItemDelegate {
	public:
		IconItemDelegate(QObject *parent = 0)
		: QStyledItemDelegate(parent) {}

	virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const {
		return QSize(96,96);
	}

	virtual void paint(QPainter *painter,
	                   const QStyleOptionViewItem &option,
	                   const QModelIndex &index) const {
		painter->fillRect(option.rect, Qt::blue);
	}
};
*/


}



HelpPanel::HelpPanel(QWidget *parent)
: ConfiguratorPanel(false, parent) {
	QAction *a;

	_name = "Docs";
	_icon = QIcon(":/res/icons/help.png");
	setHeadline("Docs & Changelogs");
	setDescription("Shows available application changelogs and documentations.");

	QToolBar *tools = new QToolBar;
	tools->setIconSize(QSize(24,24));
	tools->setAutoFillBackground(true);
	tools->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

	a = tools->addAction("Refresh");
	a->setShortcut(QKeySequence(Qt::Key_F5));
#if QT_VERSION >= 0x040400
	a->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
#endif
	connect(a, SIGNAL(triggered(bool)), this, SLOT(refresh()));

	QVBoxLayout *l = new QVBoxLayout;
	l->setMargin(0);
	l->setSpacing(0);
	setLayout(l);

	l->addWidget(tools);

	_folderView = new QListView;
	_folderView->setFrameShape(QFrame::NoFrame);
	_folderView->setResizeMode(QListView::Adjust);
	_folderView->setSelectionMode(QAbstractItemView::SingleSelection);

	_folderView->setViewMode(QListView::IconMode);
	_folderView->setMovement(QListView::Static);
	_folderView->setSpacing(6);
	/*
	_folderView->setIconSize(QSize(48,48));
	_folderView->setGridSize(QSize(96,96));
	*/

	/*
	IconItemDelegate *delegate = new IconItemDelegate(this);
	_folderView->setItemDelegate(delegate);
	*/

	connect(_folderView, SIGNAL(activated(QModelIndex)),
	        this, SLOT(openIndex(QModelIndex)));

	_model = new QStandardItemModel;
	_folderView->setModel(_model);

	l->addWidget(_folderView);

	refresh();
}


void HelpPanel::refresh() {
	_model->clear();
	_model->setHeaderData(0, Qt::Horizontal, tr("Name"));

	QIcon iconDoc = QIcon(":/res/icons/help-doc.png");
	QIcon iconChangelog = QIcon(":/res/icons/help-changelog.png");

	Seiscomp::Environment *env = Seiscomp::Environment::Instance();
	QDir docDir((env->shareDir() + "/doc").c_str());
	QFileInfoList entries = docDir.entryInfoList();

	foreach ( QFileInfo fileInfo, entries ) {
		if ( !fileInfo.isDir() ) continue;
		QString name = fileInfo.baseName();
		if ( name == "." || name == ".." ) continue;
		if ( name.isEmpty() ) continue;

		if ( QFile::exists(fileInfo.absoluteFilePath() + "/html/index.html") ) {
			QStandardItem *item = new QStandardItem;
			item->setText(name);
			item->setIcon(iconDoc);
			item->setEditable(false);
			// Type HTML
			item->setData(1, TYPEROLE);
			item->setData(fileInfo.absoluteFilePath() + "/html/index.html", PATHROLE);
			_model->appendRow(item);
		}
		else if ( QFile::exists(fileInfo.absoluteFilePath() + "/index.html") ) {
			QStandardItem *item = new QStandardItem;
			item->setText(name);
			item->setIcon(iconDoc);
			item->setEditable(false);
			// Type HTML
			item->setData(1, TYPEROLE);
			item->setData(fileInfo.absoluteFilePath() + "/index.html", PATHROLE);
			_model->appendRow(item);
		}

		if ( QFile::exists(fileInfo.absoluteFilePath() + "/CHANGELOG") ) {
			QStandardItem *item = new QStandardItem;
			item->setText(name);
			item->setIcon(iconChangelog);
			item->setEditable(false);
			// Type changelog
			item->setData(2, TYPEROLE);
			item->setData(fileInfo.absoluteFilePath() + "/CHANGELOG", PATHROLE);
			_model->appendRow(item);
		}
	}
}


void HelpPanel::openIndex(const QModelIndex &index) {
	int type = index.data(TYPEROLE).toInt();
	QString path = index.data(PATHROLE).toString();

	if ( type == 1 ) {
		QDesktopServices::openUrl(path);
	}
	else if ( type == 2 ) {
		QFile file(path);
		file.open(QFile::ReadOnly);
		QByteArray data = file.readAll();
		file.close();

		QDialog dlg;
		dlg.setWindowTitle(index.data().toString() + " - changelog");
		dlg.resize(QSize(dlg.fontMetrics().height()*60, dlg.fontMetrics().height()*25));

		QVBoxLayout *vl = new QVBoxLayout;

		QTextEdit *edit = new QTextEdit();
		edit->setWordWrapMode(QTextOption::NoWrap);
		edit->setReadOnly(true);
		edit->setText(data);

		vl->addWidget(edit);
		dlg.setLayout(vl);

		dlg.exec();
	}
}
