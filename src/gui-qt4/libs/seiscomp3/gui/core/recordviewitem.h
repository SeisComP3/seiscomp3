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



#ifndef __SEISCOMP_GUI_RECORDVIEWITEM_H__
#define __SEISCOMP_GUI_RECORDVIEWITEM_H__

#ifndef Q_MOC_RUN
#include <seiscomp3/core/recordsequence.h>
#include <seiscomp3/math/filter.h>
#include <seiscomp3/gui/core/recordwidget.h>
#endif

#include <QFrame>

namespace Seiscomp {
namespace Gui {


class RecordView;
class RecordViewItem;


class SC_GUI_API RecordLabel : public QWidget {
	Q_OBJECT

	public:
		RecordLabel(QWidget *parent=0, const char *name=0);
		~RecordLabel();

	public:
		RecordViewItem* recordViewItem() const;

		void setBackgroundColor(const QColor&);

		virtual void setText(const QString &str, int item=0) = 0;
		virtual void setColor(QColor col, int item=0) = 0;
		virtual void setFont(const QFont& f, int item=0) = 0;
		virtual void setWidth(int width, int item=0) = 0;
		virtual void setAlignment(Qt::Alignment al, int item=0) = 0;
		virtual void setEditable(bool, int item=0) = 0;

		virtual QString text(int item) const = 0;
		virtual const QFont& font(int item) const = 0;

		virtual void setOrientation(Qt::Orientation) = 0;

		virtual int itemCount() const = 0;

		void setInteractive(bool interactive);
		bool isInteractive() const;

		bool isEnabled() const { return _enabled; }

	public slots:
		void setEnabled(bool enabled);

	signals:
		//! This signal is emitted when the label has been double-clicked
		//! and the label is not interactive
		void doubleClicked();

		//! This signal is emitted when the label has been double-clicked
		//! and the label is interactive
		void statusChanged(bool);

		//! This signal is emitted when a text entry has been double-clicked
		void editRequested(int item, QRect rect);

	protected:
		void mouseDoubleClickEvent(QMouseEvent *e);
		virtual void visibilityChanged(bool) {}

	private:
		RecordViewItem* _parent;
		bool _enabled;
		bool _interactive;

	friend class RecordViewItem;
};



class SC_GUI_API StandardRecordLabel : public RecordLabel {
	public:
		StandardRecordLabel(int items=3, QWidget *parent=0, const char* = 0);
		~StandardRecordLabel();

	public:
		void setText(const QString &str, int item=0);
		void setColor(QColor col, int item=0);
		void setFont(const QFont& f, int item=0);
		void setWidth(int width, int item=0);
		void setAlignment(Qt::Alignment al, int item=0);
		void setEditable(bool, int item=0);

		QString text(int item) const;
		const QFont& font(int item) const;

		void setOrientation(Qt::Orientation o);

		int itemCount() const;

	protected:
		void paintEvent(QPaintEvent *e);

	protected:
		struct Item {
			Item() : colorSet(false) {}
			QString text;
			QColor color;
			bool colorSet;
			QFont font;
			Qt::Alignment align;
			bool editable;
			int width;
		};

		bool _vertical;
		QVector<Item> _items;
};


class SC_GUI_API RecordViewItem : public QWidget {
	Q_OBJECT

	private:
		RecordViewItem(RecordView *parent=0, bool withFrame=false, int frameMargin=0,
		               int hSpacing=0);
		RecordViewItem(RecordView *parent, RecordWidget *widget,
		               RecordSequence* records,
		               bool withFrame=false, int frameMargin=0, int hSpacing=0);

	public:
		~RecordViewItem();

	public:
		RecordView* recordView() const;

		bool feed(const Seiscomp::Record *rec);

		void setValue(int column, double d);
		double value(int column) const;

		int columnCount() const;

		//! Returns the row inside the RecordView
		int row() const;

		//! Returns the streamID used for this item
		const DataModel::WaveformStreamID& streamID() const;

		void setRecords(RecordSequence*);
		RecordSequence* records(char componentCode = '?') const;
		RecordSequence* filteredRecords(char componentCode = '?') const;

		void setBuffer(RecordSequence*);

		float timingQuality(char componentCode) const;

		bool showSlot(int slot);
		bool showRecords(char componentCode);
		char currentComponent() const;

		void setLabel(RecordLabel* label);
		void setLabelText(const QString& text);

		void setRecordWidget(RecordWidget *widget);

		RecordWidget *widget() { return _widget; }
		const RecordWidget *widget() const { return _widget; }

		RecordLabel *label () { return _label;  }
		const RecordLabel *label () const { return _label;  }

		void enableFiltering(bool);
		bool isFilteringEnabled() const;

		void setBackgroundColor(const QColor&);
		QColor backgroundColor() const;

		bool isSelected() const;
		bool isHighlighted() const;

		void setData(const QVariant&);
		const QVariant& data() const;

		void setDraggingEnabled(bool);

		void setRowHeight(int h);
		void setVisible(bool visible);

		void forceInvisibilty(bool force);
		bool isInvisibilityForced() const;

		int mapComponentToSlot(char) const;
		char mapSlotToComponent(int) const;

		bool insertComponent(char, int);


	protected:
		void mousePressEvent(QMouseEvent *event);
		void mouseReleaseEvent(QMouseEvent *event);
		void mouseMoveEvent(QMouseEvent *event);


	private:
		void setupUi(bool withFrame, int frameMargin, int hSpacing);
		void setupConnection();
		void setColor();
		void clearRecords();

		int createComponentToSlotMapping(char);


	private slots:
		void setSelected(bool);


	private slots:
		void setHighlight(bool);
		void onClickedOnTime(Seiscomp::Core::Time);
		void handleLayoutRequest();


	signals:
		void firstRecordAdded(const Seiscomp::Record*);

		void clicked(RecordViewItem*, bool buttonDown, Qt::KeyboardModifiers);
		void clickedOnTime(Seiscomp::Gui::RecordViewItem*, Seiscomp::Core::Time);
		void componentChanged(RecordViewItem*, char);


	private:
		typedef QMap<char, int> SlotMap;

		RecordView*     _parent;

		SlotMap         _slotMapping;
		RecordSequence *_seqTemplate;
		RecordWidget   *_widget;
		RecordLabel    *_label;
		int             _row;
		bool            _filtering;
		bool            _selected;
		bool            _highlight;
		char            _requestedComponent;
		char            _currentComponent;
		QVector<double> _values;
		QLayout        *_labelLayout;
		QLayout        *_widgetLayout;
		QVariant        _data;
		QPoint          _dragStart;
		bool            _enableDragging;
		bool            _visible;
		bool            _forceInvisibility;

	friend class RecordView;
};


}
}

#endif
