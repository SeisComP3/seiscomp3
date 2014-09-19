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



#ifndef __QCITEMVIEW_H__
#define __QCITEMVIEW_H__

// #define FANCY_GRAPHICS

#include <QtGui>

namespace Seiscomp {
namespace Applications {
namespace Qc {

class QcTableView;
class QcModel;

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class QcItemDelegate : public QItemDelegate {

	Q_OBJECT

	public:
		QcItemDelegate(int mode=0, QObject *parent=0);
		~QcItemDelegate();

		void setModel(QAbstractItemModel* model);

		void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		void paint0(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		void paint1(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
		QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

	private:
		QColor getSumColor(const QModelIndex& index) const;
		int _mode;
		const QAbstractProxyModel* _model;
		QcModel* _qcModel;

};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class QcItemView : public QAbstractItemView {
	Q_OBJECT

	public:
		QcItemView(QWidget *parent = 0);

		QRect visualRect(const QModelIndex &index) const;
		void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);
		QModelIndex indexAt(const QPoint &point) const;

	public slots:
		void edit() {};
		void reset();
		void setCompactView(int);
		void showInfo(const QModelIndex& index);

	protected slots:
		void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
		void rowsInserted(const QModelIndex &parent, int start, int end);
		void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);
	

	protected:
	// 	bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event);
		QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction,
								Qt::KeyboardModifiers modifiers);

		int horizontalOffset() const;
		int verticalOffset() const;

		bool isIndexHidden(const QModelIndex &index) const;

		void setSelection(const QRect&, QItemSelectionModel::SelectionFlags command);

		void mousePressEvent(QMouseEvent *event);
	//
		void mouseMoveEvent(QMouseEvent *event);
	// 	void mouseReleaseEvent(QMouseEvent *event);
		bool viewportEvent(QEvent *event);

		void paintEvent(QPaintEvent *event);
		void resizeEvent(QResizeEvent *event);
		void scrollContentsBy(int dx, int dy);

		QRegion visualRegionForSelection(const QItemSelection &selection) const;

	private:
		QWidget* _parent;
		QRect itemRect(const QModelIndex &item) const;
	// 	QRegion itemRegion(const QModelIndex &index) const;
		int rows(const QModelIndex &index = QModelIndex()) const;
		void updateGeometries();

		bool _compactView;
		int margin;
		int _totalSize;
		int pieSize;
		int validItems;
		double totalValue;
		QPoint origin;
		typedef QMap<QModelIndex, QRect> ItemPositions;
		ItemPositions _itemPositions;
		QcTableView* _infoTable; // FIXME
		QWidget* _infoWidget;
		QStatusBar* _statusBar;
		QCheckBox* _hideInfoWidget;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
}
#endif
