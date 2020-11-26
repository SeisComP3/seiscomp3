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


#ifndef __SEISCOMP_CONFIGURATION_FANCYVIEW_H__
#define __SEISCOMP_CONFIGURATION_FANCYVIEW_H__


#ifndef Q_MOC_RUN
#include <seiscomp3/system/model.h>
#endif

#include <QAbstractItemView>
#include <QScrollArea>


class QLabel;
class QBoxLayout;



struct FancyViewItemEdit {
	virtual ~FancyViewItemEdit() {}
	virtual void setValue(const QString &value) = 0;
	virtual QString value() const = 0;
	virtual QWidget *widget() = 0;
};


struct FancyViewItem {
	FancyViewItem(const QModelIndex &idx = QModelIndex(),
	              QWidget *c = NULL);

	bool isValid() const { return index.isValid(); }

	QPersistentModelIndex  index;
	QWidget               *container;
	QWidget               *label;
	FancyViewItemEdit     *input;
	QWidget               *description;
};


class FancyView : public QAbstractItemView {
	Q_OBJECT


	// ------------------------------------------------------------------
	//  X'truction
	// ------------------------------------------------------------------
	public:
		FancyView(QWidget *parent = 0);


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		QRect visualRect(const QModelIndex &index) const;
		void scrollTo(const QModelIndex &index, ScrollHint hint = EnsureVisible);
		QModelIndex indexAt(const QPoint &point) const;

		void setModel(QAbstractItemModel * model);
		void setRootIndex(const QModelIndex &index);
		void setConfigStage(Seiscomp::Environment::ConfigStage);


	// ------------------------------------------------------------------
	//  Protected slots
	// ------------------------------------------------------------------
	protected slots:
#if QT_VERSION < 0x050000
		void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
#else
		void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight,
		                 const QVector<int> &roles = QVector<int>());
#endif
		void rowsInserted(const QModelIndex &parent, int start, int end);
		void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		bool eventFilter(QObject *o, QEvent *e);
		bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event);
		QModelIndex moveCursor(QAbstractItemView::CursorAction cursorAction,
		                       Qt::KeyboardModifiers modifiers);

		int horizontalOffset() const;
		int verticalOffset() const;

		bool isIndexHidden(const QModelIndex &index) const;

		void setSelection(const QRect&, QItemSelectionModel::SelectionFlags command);

		void mousePressEvent(QMouseEvent *event);

		void mouseMoveEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);

		void paintEvent(QPaintEvent *event);
		void resizeEvent(QResizeEvent *event);
		void scrollContentsBy(int dx, int dy);

		QRegion visualRegionForSelection(const QItemSelection &selection) const;

		void currentChanged(const QModelIndex &curr, const QModelIndex &prev);
		void keyboardSearch(const QString &search);


	// ------------------------------------------------------------------
	//  Private slots
	// ------------------------------------------------------------------
	private slots:
		void lockChanged(bool);
		void optionTextEdited();
		void optionTextChanged(const QString &);
		void optionToggled(bool);
		void bindingCategoryChanged(int);
		void addStruct();
		void removeStruct();
		void addCategoryBinding();
		void removeCategoryBinding();


	// ------------------------------------------------------------------
	//  Private interface
	// ------------------------------------------------------------------
	private:
		void updateToolTip(QWidget *w, Seiscomp::System::Parameter *param);
		void updateContentGeometry();
		QWidget *createWidgetFromIndex(const QModelIndex &idx, const QString &);

		bool add(QBoxLayout *&layout, FancyViewItem &item,
		         Seiscomp::System::BindingCategory *cat, bool collapsed);
		bool add(QBoxLayout *&layout, FancyViewItem &item,
		         Seiscomp::System::Binding *binding, bool collapsed);
		bool add(QBoxLayout *&layout, FancyViewItem &item,
		         Seiscomp::System::Section *sec, bool collapsed);
		bool add(QBoxLayout *&layout, FancyViewItem &item, Seiscomp::System::Group *group);
		bool add(QBoxLayout *&layout, FancyViewItem &item, Seiscomp::System::Structure *struc);
		FancyViewItem add(QLayout *layout, const QModelIndex &idx);


	private:
		typedef QHash<QPersistentModelIndex, FancyViewItem> ViewItems;
		Seiscomp::Environment::ConfigStage _configStage;
		QWidget *_rootWidget;
		ViewItems _viewItems;
		QIcon _lockIcon;
		QIcon _unlockIcon;
		QIcon _traceIcon;
		QWidget *_currentItem;
		QLabel *_optionEditHint;
		void *_blockPopulate;
};


#endif
