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


#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>
#include <QToolButton>
#include <QPushButton>
#include <QComboBox>
#include <QDialog>
#include <QMessageBox>

#include <QScrollBar>
#include <QResizeEvent>

#include <iostream>
#include <seiscomp3/system/environment.h>
#include <seiscomp3/config/config.h>

#include "fancyview.h"
#include "flowlayout.h"
#include "gui.h"


using namespace std;

using namespace Seiscomp;
using namespace Seiscomp::System;


Q_DECLARE_METATYPE(FancyViewItem)


namespace {

//static QColor TextColor(100,144,47);
static QColor CategoryTextColor(255,255,255);
static QColor CategoryBgColor(38,80,128);

static QColor TextColor(38,80,128);
static QColor DescColor(TextColor.red()*3/4+64,
                        TextColor.green()*3/4+64,
                        TextColor.blue()*3/4+64);
static QColor DescBackColor(255,255,204);
static QColor AlertColor(128*3/4+64,64,64);


QColor blend(const QColor& c1, const QColor& c2, int percentOfC1) {
	return QColor((c1.red()*percentOfC1 + c2.red()*(100-percentOfC1)) / 100,
	              (c1.green()*percentOfC1 + c2.green()*(100-percentOfC1)) / 100,
	              (c1.blue()*percentOfC1 + c2.blue()*(100-percentOfC1)) / 100);
}




QSize qSmartMinSize(const QSize &sizeHint, const QSize &minSizeHint,
                    const QSize &minSize, const QSize &maxSize,
                    const QSizePolicy &sizePolicy) {
	QSize s(0, 0);

	if (sizePolicy.horizontalPolicy() != QSizePolicy::Ignored) {
		if (sizePolicy.horizontalPolicy() & QSizePolicy::ShrinkFlag)
			s.setWidth(minSizeHint.width());
		else
			s.setWidth(qMax(sizeHint.width(), minSizeHint.width()));
	}

	if (sizePolicy.verticalPolicy() != QSizePolicy::Ignored) {
		if (sizePolicy.verticalPolicy() & QSizePolicy::ShrinkFlag) {
			s.setHeight(minSizeHint.height());
		} else {
			s.setHeight(qMax(sizeHint.height(), minSizeHint.height()));
		}
	}

	s = s.boundedTo(maxSize);
	if (minSize.width() > 0)
		s.setWidth(minSize.width());
	if (minSize.height() > 0)
		s.setHeight(minSize.height());

	return s.expandedTo(QSize(0,0));
}


QSize qSmartMinSize(const QWidget *w) {
	return qSmartMinSize(w->sizeHint(), w->minimumSizeHint(),
	                     w->minimumSize(), w->maximumSize(),
	                     w->sizePolicy());
}


string maxSize(const string &text, size_t maxWidth) {
	if ( text.size() <= maxWidth ) return text;
	size_t pos = text.find_last_of(' ', maxWidth-4);

	if ( pos != string::npos )
		return text.substr(0, pos) + " ...";

	return text.substr(0, maxWidth-4) + " ...";
}


string string2Block(const string &input, size_t lineWidth) {
	string txt = input;
	size_t s = 0;
	size_t to = s + lineWidth;

	while ( to < txt.length() ) {
		// find linebreaks and comment each new line
		size_t p = txt.find_first_of('\n', s);
		if ( p != string::npos && (p - s) < lineWidth) {
			s = p + 1;
		}
		else {
			// insert line break if possible at last space else inside word
			// without hyphenation
			p = txt.find_last_of(' ', to-1);
			if ( p == string::npos || p < s || (p -s) > lineWidth) {
				txt.insert(to, "\n");
				s = to + 1;
			}
			else {
				txt[p] = '\n';
				s = p+1;
			}
		}

		to = s + lineWidth;
	}

	// comment line breaks in last line
	while ( s < txt.length() ) {
		size_t p = txt.find_first_of('\n', s);
		if ( p == string::npos ) break;
		s = p+1;
	}

	return txt;
}



class NewStructDialog : public QDialog {
	public:
		NewStructDialog(const Container *c, QWidget *parent = 0)
			: QDialog(parent), _container(c) {
			QVBoxLayout *layout = new QVBoxLayout;
			setLayout(layout);

			QHBoxLayout *hlayout = new QHBoxLayout;
			QLabel *label = new QLabel("Name:");
			hlayout->addWidget(label);
			_name = new QLineEdit;
			QRegExp rx("[A-Za-z0-9_\\-]+");
			_name->setValidator(new QRegExpValidator(rx,0));
			hlayout->addWidget(_name);
			layout->addLayout(hlayout);

			hlayout = new QHBoxLayout;
			hlayout->addStretch();
			QPushButton *ok = new QPushButton("Ok");
			hlayout->addWidget(ok);
			QPushButton *cancel = new QPushButton("Cancel");
			hlayout->addWidget(cancel);

			layout->addLayout(hlayout);

			connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
			connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		}

		QString name() const {
			return _name->text();
		}

		void accept() {
			if ( _name->text().isEmpty() ) {
				QMessageBox::critical(NULL, "Empty name",
				                      "Empty names are not allowed. ");
				return;
			}

			if ( _container->hasStructure(qPrintable(_name->text())) ) {
				QMessageBox::critical(NULL, "Duplicate name",
				                      "The name exists already and duplicate "
				                      "names are not allowed.");
				return;
			}

			QDialog::accept();
		}

	private:
		const Container *_container;
		QLineEdit       *_name;
};



class NewCatBindingDialog : public QDialog {
	public:
		NewCatBindingDialog(const BindingCategory *c,
		                    const std::string &type,
		                    QWidget *parent = 0)
		    : QDialog(parent), _cat(c), _type(type) {
			QVBoxLayout *layout = new QVBoxLayout;
			setLayout(layout);

			QHBoxLayout *hlayout = new QHBoxLayout;
			QLabel *label = new QLabel("Name:");
			hlayout->addWidget(label);
			_name = new QLineEdit;
			QRegExp rx("[A-Za-z0-9_\\-]+");
			_name->setValidator(new QRegExpValidator(rx,0));
			hlayout->addWidget(_name);
			layout->addLayout(hlayout);

			hlayout = new QHBoxLayout;
			hlayout->addStretch();
			QPushButton *ok = new QPushButton("Ok");
			hlayout->addWidget(ok);
			QPushButton *cancel = new QPushButton("Cancel");
			hlayout->addWidget(cancel);

			layout->addLayout(hlayout);

			connect(ok, SIGNAL(clicked()), this, SLOT(accept()));
			connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
		}

		std::string name() const {
			return _name->text().toStdString();
		}

		void accept() {
			std::string alias;
			if ( _name->text().isEmpty() )
				alias = _type;
			else
				alias = _name->text().toStdString();

			if ( _cat->hasBinding(alias.c_str()) ) {
				QMessageBox::critical(NULL, "Duplicate alias",
				                      "The alias exists already and duplicate "
				                      "aliases are not allowed.");
				return;
			}

			QDialog::accept();
		}

	private:
		const BindingCategory *_cat;
		std::string            _type;
		QLineEdit             *_name;
};



class BlockWidget : public QWidget {
	public:
		BlockWidget(QWidget *parent = 0) : QWidget(parent) {
			_hasCustomBackground = false;
		}

		void setBackgroundColor(QColor bg) {
			_hasCustomBackground = true;
			_bg = bg;
		}

	protected:
		void paintEvent(QPaintEvent *) {
			QPainter p(this);

			if ( _hasCustomBackground )
				p.fillRect(rect(), _bg);

			int left, top, right, bottom;
			getContentsMargins(&left, &top, &right, &bottom);
			bottom = rect().bottom()-bottom;
			//p.drawLine(0,top,left,top);
			//p.drawLine(0,top,0,bottom);
			//p.drawLine(0,bottom,left,bottom);
			QLinearGradient grad(0,top,0,bottom);
			QColor fg = Qt::gray;
			grad.setColorAt(0.5,fg);
			fg.setAlpha(0);
			grad.setColorAt(0,fg);
			grad.setColorAt(1,fg);
			p.setPen(QPen(grad,1));
			p.drawLine(left-1,top,left-1,bottom);
		}

	private:
		bool   _hasCustomBackground;
		QColor _bg;
};


class ViewItemWidget : public QWidget {
	public:
		ViewItemWidget(QWidget *parent = 0) : QWidget(parent) {
			_isSelected = false;
		}

		void setSelected(bool s) {
			_isSelected = s;
			update();
		}

		bool isSelected() const { return _isSelected; }

	protected:
		void paintEvent(QPaintEvent *e) {
			QPainter p(this);
			if ( _isSelected ) {
				QLinearGradient grad(0,0,0,height());
				QColor highlight = palette().color(QPalette::Highlight);
				QColor base = palette().color(QPalette::Base);

				highlight.setAlpha(64);

				grad.setColorAt(0,base);
				grad.setColorAt(0.5,highlight);
				grad.setColorAt(1,base);
				p.setPen(highlight);
				p.setBrush(grad);
				p.drawRect(0,0,width()-1,height()-1);
			}
		}

	private:
		bool _isSelected;
};


class HRuler : public QWidget {
	public:
		HRuler(QWidget *parent = 0) : QWidget(parent) {
			setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
		}

		QSize sizeHint() const { return QSize(-1,1); }

	protected:
		void paintEvent(QPaintEvent *) {
			QPainter p(this);
			int cy = rect().center().y();
			QLinearGradient grad(0,0,rect().width(),0);
			QColor fg = Qt::gray;
			grad.setColorAt(0.5,fg);
			fg.setAlpha(0);
			grad.setColorAt(0,fg);
			grad.setColorAt(1,fg);
			p.setPen(QPen(grad,1));
			p.drawLine(rect().left(),cy,rect().right(),cy);
		}
};


class BlockHandle : public QToolButton {
	public:
		BlockHandle(QWidget *parent = 0) : QToolButton(parent) {
			setFixedWidth(16);
			setFixedHeight(16);
		}

	protected:
		void paintEvent(QPaintEvent *) {
			QPainter p(this);

			p.setPen(Qt::gray);
			p.setBrush(Qt::NoBrush);
			p.setRenderHint(QPainter::Antialiasing, true);
			QPolygon poly;
			if ( isChecked() ) {
				poly.append(QPoint(4,4));
				poly.append(QPoint(12,4));
				poly.append(QPoint(8,12));
				/*
				p.drawLine(4,4,12,4);
				p.drawLine(4,4,8,12);
				p.drawLine(8,12,12,4);
				*/
			}
			else {
				poly.append(QPoint(4,4));
				poly.append(QPoint(12,8));
				poly.append(QPoint(4,12));
			}
			p.drawPolygon(poly);
		}
};


class Header : public QWidget {
	public:
		Header(QColor bg, QWidget *parent = 0) : QWidget(parent), _bg(bg) {}

	protected:
		void paintEvent(QPaintEvent *) {
			QPainter p(this);
			p.fillRect(rect(), _bg);
		}

	private:
		QColor _bg;
};


class IconButton : public QAbstractButton {
	public:
		IconButton(const QIcon &normalIcon, const QIcon &checkedIcon)
		: QAbstractButton() {
			_normalIcon = normalIcon;
			_checkedIcon = checkedIcon;
		}


	protected:
		void paintEvent(QPaintEvent *) {
			QPixmap pixmap;
			if ( isChecked() )
				pixmap = _checkedIcon.pixmap(
				           size(),
				           isEnabled()?QIcon::Normal:QIcon::Disabled
				         );
			else
				pixmap = _normalIcon.pixmap(
				           size(),
				           isEnabled()?QIcon::Normal:QIcon::Disabled
				         );

			QPainter p(this);
			p.drawPixmap(0,0,pixmap);
		}


	private:
		QIcon _normalIcon;
		QIcon _checkedIcon;
};


class DescLabel : public QWidget {
	public:
		DescLabel(QWidget *parent = 0) : QWidget(parent) {
			QSizePolicy sp(QSizePolicy::Preferred, QSizePolicy::Preferred);
			sp.setHeightForWidth(true);
			setSizePolicy(sp);
		}

		void setText(const QString &text) {
			_text = text;
		}

		const QString &text() const {
			return _text;
		}

		int heightForWidth(int w) const {
			int l,t,r,b;
			getContentsMargins(&l,&t,&r,&b);
			int prefHeight =
				fontMetrics().boundingRect(
					0,0,w-l-r,QWIDGETSIZE_MAX,
					Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, _text
				).height()+t+b;
			return prefHeight;
		}

		void paintEvent(QPaintEvent *e) {
			QPainter p(this);

			if ( autoFillBackground() )
				p.fillRect(e->rect(), palette().color(QPalette::Window));

			p.drawText(contentsRect(), Qt::AlignLeft|Qt::AlignTop|Qt::TextWordWrap, _text);
		}


	private:
		QString _text;
};


class StringEdit : public QLineEdit, public FancyViewItemEdit {
	public:
		StringEdit(QWidget *parent = 0) : QLineEdit(parent) {}

		QWidget *widget() { return this; }

		void setValue(const QString &value) {
			setText(value);
		}

		QString value() const {
			return text();
		}

	/*
	protected:
		void focusInEvent(QFocusEvent *e) {
			QPalette pal = palette();
			pal.setColor(QPalette::Base, QColor(255,255,224));
			setPalette(pal);
			QLineEdit::focusInEvent(e);
		}

		void focusOutEvent(QFocusEvent *e) {
			QPalette pal;
			setPalette(pal);
			QLineEdit::focusOutEvent(e);
		}
	*/
};


class BoolEdit : public QCheckBox, public FancyViewItemEdit {
	public:
		BoolEdit(QWidget *parent = 0) : QCheckBox(parent) {}

		QWidget *widget() { return this; }

		void setValue(const QString &value) {
			setChecked(value.compare("true", Qt::CaseInsensitive) == 0);
		}

		QString value() const {
			return isChecked()?"true":"false";
		}
};


class EvalHintWidget : public QLabel {
	public:
		EvalHintWidget(QWidget *w) : QLabel(w) {
			setFrameStyle(QFrame::NoFrame);
		}

	protected:
		void paintEvent(QPaintEvent *e) {
			QPainter p(this);
			p.setPen(QPen(palette().color(QPalette::WindowText), 3, Qt::DotLine));
			p.setBrush(palette().color(QPalette::Window));
			p.drawRect(rect().adjusted(0,0,-1,-1));
			QLabel::paintEvent(e);
		}
};


}


FancyViewItem::FancyViewItem(const QModelIndex &idx, QWidget *c)
: index(idx), container(c), label(NULL), input(NULL), description(NULL) {
	if ( container )
		// Link the container widget with its FancyViewItem
		container->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(*this));
}


FancyView::FancyView(QWidget *parent) : QAbstractItemView(parent) {
	_rootWidget = NULL;
	_currentItem = NULL;
	_optionEditHint = NULL;
	_lockIcon = QIcon(":/res/icons/lock.png");
	_unlockIcon = QIcon(":/res/icons/unlock.png");
	_traceIcon = QIcon(":/res/icons/trace.png");
	_configStage = Environment::CS_CONFIG_APP;
	_blockPopulate = NULL;
	setFrameShape(QFrame::NoFrame);
}


QRect FancyView::visualRect(const QModelIndex &index) const {
	return QRect();
}


void FancyView::scrollTo(const QModelIndex &index, ScrollHint hint) {
	ViewItems::iterator it = _viewItems.find(index);
	if ( it == _viewItems.end() ) return;

	QWidget *w = it.value().container;
	QPoint p = _rootWidget->mapFromGlobal(w->mapToGlobal(QPoint(0,0)));
	horizontalScrollBar()->setValue(p.x());
	verticalScrollBar()->setValue(p.y());
}


QModelIndex FancyView::indexAt(const QPoint &point) const {
	return QModelIndex();
}


void FancyView::setModel(QAbstractItemModel *model) {
	QAbstractItemView::setModel(model);

	if ( _rootWidget != NULL ) {
		delete _rootWidget;
		_rootWidget = NULL;
	}

	horizontalScrollBar()->setRange(0,0);
	verticalScrollBar()->setRange(0,0);
}



void FancyView::setRootIndex(const QModelIndex &index) {
	QAbstractItemView::setRootIndex(index);
	if ( _rootWidget ) {
		delete _rootWidget;
		_rootWidget = NULL;
	}

	_currentItem = NULL;
	_viewItems = ViewItems();

	if ( !index.isValid() ) return;

	_rootWidget = new QWidget(viewport());
	//_rootWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
	QVBoxLayout *l = new QVBoxLayout;
	_rootWidget->setLayout(l);

	int rows = model()->rowCount(index);

	/*
	if ( index.data(ConfigurationTreeItemModel::Type).toInt() == ConfigurationTreeItemModel::TypeModule ) {
		Module *mod = reinterpret_cast<Module*>(index.data(ConfigurationTreeItemModel::Link).value<void*>());
		QLabel *about = new QLabel(_rootWidget);
		about->setWordWrap(true);
		about->setText(mod->definition->description.c_str());
		about->setMargin(8);
		l->addWidget(about);
	}
	*/

	if ( index.data(ConfigurationTreeItemModel::Type).toInt() == ConfigurationTreeItemModel::TypeModule ) {
		Module *mod = reinterpret_cast<Module*>(index.data(ConfigurationTreeItemModel::Link).value<void*>());
		QLabel *info = new QLabel(_rootWidget);
		info->setWordWrap(true);
		QPalette pal = info->palette();
		pal.setColor(QPalette::Text, DescColor);
		info->setPalette(pal);
		QFont f = info->font();
		f.setBold(true);
		info->setFont(f);
		if ( mod->supportsBindings() )
			info->setText(tr(
				"This module considers module configuration parameters. "
				"It also requires bindings which may overwrite module "
				"configuration parameters."
			));
		else
			info->setText(tr(
				"This module only considers module configuration parameters. "
				"It does not provide a bindings configuration."
			));
		info->setMargin(8);
		l->addWidget(info);
	}

	QString secName;
	int type = index.data(ConfigurationTreeItemModel::Type).toInt();
	if ( type == ConfigurationTreeItemModel::TypeModule ||
	     type == ConfigurationTreeItemModel::TypeBinding )
		secName = index.data().toString();

	for ( int i = 0; i < rows; ++i ) {
		QModelIndex idx = index.child(i,0);
		QWidget *w = createWidgetFromIndex(idx, secName);
		if ( w )
			l->addWidget(w);
	}

	l->addStretch();

	//_rootWidget->setBackgroundRole(QPalette::ToolTipBase);
	//_rootWidget->setAutoFillBackground(true);

	_rootWidget->installEventFilter(this);
	_rootWidget->show();
	//updateContentGeometry();
}


void FancyView::setConfigStage(Seiscomp::Environment::ConfigStage cs) {
	_configStage = cs;
}


QWidget *FancyView::createWidgetFromIndex(const QModelIndex &idx,
                                          const QString &rootSecName) {
	ViewItemWidget *w = new ViewItemWidget;
	QBoxLayout *l = new QVBoxLayout;
	w->setLayout(l);

	w->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

	int rows = model()->rowCount(idx);
	int type = idx.data(ConfigurationTreeItemModel::Type).toInt();

	switch ( type ) {
		case ConfigurationTreeItemModel::TypeCategoryBinding:
		{
			Binding *binding = reinterpret_cast<Binding*>(idx.data(ConfigurationTreeItemModel::Link).value<void*>());
			if ( binding ) {
				FancyViewItem item(idx, w);
				add(l, item, binding, true);

				w->setProperty("viewBinding", qVariantFromValue((void*)binding));

				bool firstParameter = true;
				QLayout *paramLayout = NULL;

				for ( int i = 0; i < rows; ++i ) {
					QModelIndex child = idx.child(i,0);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() != ConfigurationTreeItemModel::TypeParameter )
						continue;

					if ( firstParameter ) {
						QFrame *paramWidget = new QFrame;
						paramLayout = new FlowLayout(-1, -1, fontMetrics().ascent());
						paramWidget->setLayout(paramLayout);
						l->addWidget(paramWidget);
						firstParameter = false;
					}

					FancyViewItem item = add(paramLayout, child);

					if ( item.isValid() )
						_viewItems[child] = item;
				}

				for ( int i = 0; i < rows; ++i ) {
					QModelIndex child = idx.child(i,0);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() == ConfigurationTreeItemModel::TypeParameter )
						continue;
					QWidget *cw = createWidgetFromIndex(child, rootSecName);
					if ( cw )
						l->addWidget(cw);
				}

				if ( rows == 0 ) {
					DescLabel *desc = new DescLabel;
					desc->setContentsMargins(8,0,0,0);
					desc->setText("This section does not contain a parameter to configure...");
					//desc->setWordWrap(true);
					//desc->setMinimumWidth(100);
					QPalette pal = desc->palette();
					pal.setColor(QPalette::Text, AlertColor);
					desc->setPalette(pal);
					l->addWidget(desc);
				}

				_viewItems[idx] = item;
			}
			break;
		}
		case ConfigurationTreeItemModel::TypeCategory:
		{
			BindingCategory *cat = reinterpret_cast<BindingCategory*>(idx.data(ConfigurationTreeItemModel::Link).value<void*>());
			if (cat ) {
				FancyViewItem item(idx, w);
				add(l, item, cat, false);

				DescLabel *desc = new DescLabel;
				desc->setContentsMargins(8,0,0,0);
				desc->setText(QString("No binding for \"%1\" selected.").arg(cat->name.c_str()));
				QPalette pal = desc->palette();
				pal.setColor(QPalette::Text, AlertColor);
				desc->setPalette(pal);
				l->addWidget(desc);
				desc->setVisible(cat->bindings.empty());

				w->setProperty("statusLabel", qVariantFromValue((void*)desc));

				for ( int r = 0; r < rows; ++r ) {
					QModelIndex secIdx = idx.child(r,0);
					int type = secIdx.data(ConfigurationTreeItemModel::Type).toInt();
					if ( type != ConfigurationTreeItemModel::TypeCategoryBinding )
						continue;

					QWidget *bw = createWidgetFromIndex(secIdx, rootSecName);
					bw->setProperty("statusLabel", qVariantFromValue((void*)desc));
					l->addWidget(bw);
				}

				size_t catBindingCount = cat->bindingTypes.size();
				QComboBox *comboBox = NULL;

				comboBox = new QComboBox;
				comboBox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
				//comboBox->addItem("- None -");

				for ( size_t i = 0; i < catBindingCount; ++i ) {
					Binding *b = cat->bindingTypes[i].get();
					if ( b->definition->description.empty() )
						comboBox->addItem(b->definition->name.c_str(), item.index.child(i,0).data());
					else {
						comboBox->addItem(
							QString("%1 - %2")
							.arg(b->definition->name.c_str())
							.arg(maxSize(b->definition->description, 40).c_str()),
							item.index.child(i,0).data()
						);
						comboBox->setItemData(comboBox->count()-1, string2Block(b->definition->description, 100).c_str(), Qt::ToolTipRole);
					}
					comboBox->setItemData(comboBox->count()-1, b->definition->name.c_str());
				}

				comboBox->model()->sort(0);
				comboBox->setCurrentIndex(0);

				QToolButton *addButton = new QToolButton;
				addButton->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
				addButton->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));
				addButton->setProperty("comboBox", qVariantFromValue((void*)comboBox));
				addButton->setIcon(QIcon(":/res/icons/add.png"));
				addButton->setToolTip(QString("Add a new '%1' instance").arg(cat->name.c_str()));
				addButton->setEnabled(catBindingCount > 0);

				QHBoxLayout *hlayout = new QHBoxLayout;
				hlayout->addWidget(addButton);
				hlayout->addWidget(comboBox);
				hlayout->addStretch();
				connect(comboBox, SIGNAL(currentIndexChanged(int)),
				        this, SLOT(bindingCategoryChanged(int)));

				connect(addButton, SIGNAL(clicked()), this, SLOT(addCategoryBinding()));

				if ( comboBox ) {
					comboBox->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));
					comboBox->setProperty("cat.layout", qVariantFromValue((void*)l));
				}

				l->addLayout(hlayout);

				_viewItems[idx] = item;
			}
			break;
		}
		case ConfigurationTreeItemModel::TypeSection:
		{
			Section *sec = reinterpret_cast<Section*>(idx.data(ConfigurationTreeItemModel::Link).value<void*>());
			if ( sec ) {
				FancyViewItem item(idx, w);
				add(l, item, sec, idx.data().toString() != rootSecName);

				bool firstParameter = true;
				QLayout *paramLayout = NULL;

				if ( !sec->description.empty() ) {
					StatusLabel *desc = new StatusLabel;
					desc->setWordWrap(true);
					desc->setContentsMargins(8,0,0,0);
					desc->setInfoText(sec->description.c_str());
					l->addWidget(desc);
				}

				for ( int i = 0; i < rows; ++i ) {
					QModelIndex child = idx.child(i,0);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() != ConfigurationTreeItemModel::TypeParameter )
						continue;

					if ( firstParameter ) {
						QFrame *paramWidget = new QFrame;
						paramLayout = new FlowLayout(-1, -1, fontMetrics().ascent());
						paramWidget->setLayout(paramLayout);
						l->addWidget(paramWidget);
						firstParameter = false;
					}

					FancyViewItem item = add(paramLayout, child);

					if ( item.isValid() )
						_viewItems[child] = item;
				}

				for ( int i = 0; i < rows; ++i ) {
					QModelIndex child = idx.child(i,0);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() == ConfigurationTreeItemModel::TypeParameter )
						continue;
					QWidget *cw = createWidgetFromIndex(child, rootSecName);
					if ( cw )
						l->addWidget(cw);
				}

				if ( rows == 0 ) {
					DescLabel *desc = new DescLabel;
					desc->setContentsMargins(8,0,0,0);
					desc->setText("This section does not contain a parameter to configure...");
					//desc->setWordWrap(true);
					//desc->setMinimumWidth(100);
					QPalette pal = desc->palette();
					pal.setColor(QPalette::Text, AlertColor);
					desc->setPalette(pal);
					l->addWidget(desc);
				}

				_viewItems[idx] = item;
			}
			break;
		}
		case ConfigurationTreeItemModel::TypeGroup:
		{
			Group *group = reinterpret_cast<Group*>(idx.data(ConfigurationTreeItemModel::Link).value<void*>());
			if ( group ) {
				FancyViewItem item(idx, w);
				add(l, item, group);

				bool firstParameter = true;
				QLayout *paramLayout = NULL;

				for ( int i = 0; i < rows; ++i ) {
					QModelIndex child = idx.child(i,0);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() != ConfigurationTreeItemModel::TypeParameter )
						continue;

					if ( firstParameter ) {
						QFrame *paramWidget = new QFrame;
						paramLayout = new FlowLayout(-1, -1, fontMetrics().ascent());
						paramWidget->setLayout(paramLayout);
						l->addWidget(paramWidget);
						firstParameter = false;
					}

					FancyViewItem item = add(paramLayout, child);
					if ( item.isValid() )
						_viewItems[child] = item;
				}

				for ( int i = 0; i < rows; ++i ) {
					QModelIndex child = idx.child(i,0);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() == ConfigurationTreeItemModel::TypeParameter )
						continue;
					QWidget *cw = createWidgetFromIndex(child, rootSecName);
					if ( cw )
						l->addWidget(cw);
				}

				if ( rows == 0 ) {
					DescLabel *desc = new DescLabel;
					desc->setContentsMargins(8,0,0,0);
					desc->setText("This group does not contain a parameter to configure...");
					//desc->setWordWrap(true);
					//desc->setMinimumWidth(100);
					QPalette pal = desc->palette();
					pal.setColor(QPalette::Text, AlertColor);
					desc->setPalette(pal);
					l->addWidget(desc);
				}

				_viewItems[idx] = item;
			}
			break;
		}
		case ConfigurationTreeItemModel::TypeParameter:
		{
			FancyViewItem item = add(l, idx);
			if ( item.isValid() )
				_viewItems[idx] = item;

			break;
		}
		case ConfigurationTreeItemModel::TypeStruct:
		{
			Structure *struc = reinterpret_cast<Structure*>(idx.data(ConfigurationTreeItemModel::Link).value<void*>());
			if ( struc ) {
				FancyViewItem item(idx, w);
				add(l, item, struc);

				if ( struc->name.empty() ) break;

				bool firstParameter = true;
				QLayout *paramLayout = NULL;

				for ( int i = 0; i < rows; ++i ) {
					QModelIndex child = idx.child(i,0);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() != ConfigurationTreeItemModel::TypeParameter )
						continue;

					if ( firstParameter ) {
						QFrame *paramWidget = new QFrame;
						paramLayout = new FlowLayout(-1, -1, fontMetrics().ascent());
						paramWidget->setLayout(paramLayout);
						l->addWidget(paramWidget);
						firstParameter = false;
					}

					FancyViewItem item = add(paramLayout, child);
					if ( item.isValid() )
						_viewItems[child] = item;
				}

				for ( int i = 0; i < rows; ++i ) {
					QModelIndex child = idx.child(i,0);
					if ( child.data(ConfigurationTreeItemModel::Type).toInt() == ConfigurationTreeItemModel::TypeParameter )
						continue;
					QWidget *cw = createWidgetFromIndex(child, rootSecName);
					if ( cw )
						l->addWidget(cw);
				}

				if ( rows == 0 ) {
					DescLabel *desc = new DescLabel;
					desc->setContentsMargins(8,0,0,0);
					desc->setText("This group does not contain a parameter to configure...");
					//desc->setWordWrap(true);
					//desc->setMinimumWidth(100);
					QPalette pal = desc->palette();
					pal.setColor(QPalette::Text, AlertColor);
					desc->setPalette(pal);
					l->addWidget(desc);
				}
			}
			break;
		}
		default:
			break;
	}

	return w;
}


bool FancyView::add(QBoxLayout *&layout, FancyViewItem &item,
                    Seiscomp::System::BindingCategory *cat, bool collapsed) {
	QHBoxLayout *hlayout = new QHBoxLayout;
	hlayout->setMargin(0);

	BlockHandle *catHandle = new BlockHandle;
	catHandle->setCheckable(true);
	catHandle->setChecked(true);

	Header *header = new Header(CategoryBgColor);
	header->setLayout(hlayout);

	hlayout->addWidget(catHandle);

	QLabel *catName = new QLabel;
	catName->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	QFont f = catName->font();
	f.setPointSize(f.pointSize()*150/100);
	f.setBold(true);
	//f.setItalic(true);
	catName->setFont(f);
	catName->setText(item.index.data().toString());
	QPalette pal = catName->palette();
	pal.setColor(QPalette::Text, CategoryTextColor);
	catName->setPalette(pal);

	item.label = catName;

	hlayout->addWidget(catName);

	HRuler *hline = new HRuler;
	hlayout->addWidget(hline);

	//layout->addLayout(hlayout);
	layout->addWidget(header);

	BlockWidget *catWidget = new BlockWidget;
	catWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
	catWidget->setContentsMargins(8,0,0,0);

	layout->addWidget(catWidget);

	layout = new QVBoxLayout;
	catWidget->setLayout(layout);

	if ( collapsed ) {
		catHandle->setChecked(false);
		catWidget->setVisible(false);
	}

	connect(catHandle, SIGNAL(toggled(bool)),
	        catWidget, SLOT(setVisible(bool)));

	/*
	size_t catBindingCount = cat->bindingTypes.size();
	QComboBox *comboBox = NULL;

	comboBox = new QComboBox;
	comboBox->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));
	comboBox->addItem("- None -");

	for ( size_t i = 0; i < catBindingCount; ++i ) {
		Binding *b = cat->bindingTypes[i].get();
		//comboBox->addItem(b->definition->description.c_str(), item.index.child(i,0).data());
		comboBox->addItem(b->definition->name.c_str(), item.index.child(i,0).data());
	}

	catWidget->setBackgroundColor(blend(pal.color(QPalette::Base), CategoryBgColor, 90));
	comboBox->setCurrentIndex(0);
	hlayout->addWidget(comboBox);
	connect(comboBox, SIGNAL(currentIndexChanged(int)),
	        this, SLOT(bindingCategoryChanged(int)));

	QToolButton *addButton = new QToolButton;
	addButton->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	addButton->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));
	addButton->setIcon(QIcon(":/res/icons/add.png"));
	addButton->setToolTip(QString("Add a new '%1' instance").arg(cat->name.c_str()));

	hlayout->addWidget(addButton);

	if ( comboBox ) {
		comboBox->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));
		comboBox->setProperty("cat.layout", qVariantFromValue((void*)layout));
	}
	*/

	return true;
}


bool FancyView::add(QBoxLayout *&layout, FancyViewItem &item,
                    Seiscomp::System::Binding *binding, bool collapsed) {
	QHBoxLayout *hlayout = new QHBoxLayout;

	BlockHandle *secHandle = new BlockHandle;
	secHandle->setCheckable(true);
	secHandle->setChecked(true);

	hlayout->addWidget(secHandle);

	QToolButton *removeButton = new QToolButton;
	removeButton->setIcon(QIcon(":/res/icons/remove.png"));
	removeButton->setToolTip(QString("Remove binding '%1'").arg(binding->name.c_str()));
	removeButton->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));

	connect(removeButton, SIGNAL(clicked()), this, SLOT(removeCategoryBinding()));

	QLabel *sectionName = new QLabel;
	sectionName->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	QFont f = sectionName->font();
	//f.setPointSize(f.pointSize()*150/100);
	f.setBold(true);
	//f.setItalic(true);
	sectionName->setFont(f);

	QString label = item.index.data().toString();
	if ( label != binding->name.c_str() )
		sectionName->setText(item.index.data().toString() + " : " + binding->name.c_str());
	else
		sectionName->setText(item.index.data().toString());
	QPalette pal = sectionName->palette();
	pal.setColor(QPalette::Text, TextColor);
	sectionName->setPalette(pal);

	item.label = sectionName;

	hlayout->addWidget(sectionName);

	HRuler *hline = new HRuler;
	hlayout->addWidget(hline);

	hlayout->addWidget(removeButton);

	layout->addLayout(hlayout);

	BlockWidget *sectionWidget = new BlockWidget;
	sectionWidget->setBackgroundColor(blend(pal.color(QPalette::Base), CategoryBgColor, 90));
	sectionWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
	sectionWidget->setContentsMargins(8,0,0,0);
	layout->addWidget(sectionWidget);

	layout = new QVBoxLayout;
	sectionWidget->setLayout(layout);

	if ( collapsed ) {
		secHandle->setChecked(false);
		sectionWidget->setVisible(false);
	}

	connect(secHandle, SIGNAL(toggled(bool)),
	        sectionWidget, SLOT(setVisible(bool)));

	return true;
}


bool FancyView::add(QBoxLayout *&layout, FancyViewItem &item,
                    Seiscomp::System::Section *sec, bool collapsed) {
	QHBoxLayout *hlayout = new QHBoxLayout;

	BlockHandle *secHandle = new BlockHandle;
	secHandle->setCheckable(true);
	secHandle->setChecked(true);

	hlayout->addWidget(secHandle);

	QLabel *sectionName = new QLabel;
	sectionName->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	QFont f = sectionName->font();
	f.setPointSize(f.pointSize()*150/100);
	f.setBold(true);
	//f.setItalic(true);
	sectionName->setFont(f);
	sectionName->setText(item.index.data().toString());
	QPalette pal = sectionName->palette();
	pal.setColor(QPalette::Text, TextColor);
	sectionName->setPalette(pal);

	item.label = sectionName;

	hlayout->addWidget(sectionName);

	HRuler *hline = new HRuler;
	hlayout->addWidget(hline);

	layout->addLayout(hlayout);

	QWidget *sectionWidget = new BlockWidget;
	sectionWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
	sectionWidget->setContentsMargins(8,0,0,0);
	layout->addWidget(sectionWidget);

	layout = new QVBoxLayout;
	sectionWidget->setLayout(layout);

	if ( collapsed ) {
		secHandle->setChecked(false);
		sectionWidget->setVisible(false);
	}

	connect(secHandle, SIGNAL(toggled(bool)),
	        sectionWidget, SLOT(setVisible(bool)));

	return true;
}


bool FancyView::add(QBoxLayout *&layout, FancyViewItem &item, Group *group) {
	layout->addSpacing(16);

	// Build header
	QHBoxLayout *hlayout = new QHBoxLayout;
	BlockHandle *groupHandle= new BlockHandle;
	groupHandle->setCheckable(true);
	groupHandle->setChecked(true);
	hlayout->addWidget(groupHandle);

	QLabel *name = new QLabel;
	name->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	QFont f = name->font();
	//f.setPointSize(f.pointSize()*125/100);
	f.setBold(true);
	name->setFont(f);
	QPalette pal = name->palette();
	pal.setColor(QPalette::Text, TextColor);
	name->setPalette(pal);
	name->setText(item.index.data().toString());//  group->definition->name.c_str());
	hlayout->addWidget(name);

	item.label = name;

	HRuler *hline = new HRuler;
	hlayout->addWidget(hline);

	layout->addLayout(hlayout);

	// Build group widget
	QWidget *groupWidget = new BlockWidget;
	groupWidget->setContentsMargins(8,0,0,0);
	//groupWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
	//groupWidget->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);

	QVBoxLayout *groupLayout = new QVBoxLayout;
	groupWidget->setLayout(groupLayout);

	if ( !group->definition->description.empty() ) {
		DescLabel *desc = new DescLabel;
		desc->setContentsMargins(8,0,0,0);
		desc->setText(group->definition->description.c_str());
		//desc->setWordWrap(true);
		QPalette pal = desc->palette();
		pal.setColor(QPalette::Text, DescColor);
		desc->setPalette(pal);
		layout->addWidget(desc);
		item.description = desc;
	}

	layout->addWidget(groupWidget);
	layout = groupLayout;

	connect(groupHandle, SIGNAL(toggled(bool)),
	        groupWidget, SLOT(setVisible(bool)));

	return true;
}


bool FancyView::add(QBoxLayout *&layout, FancyViewItem &item, Structure *struc) {
	QHBoxLayout *hlayout = new QHBoxLayout;

	QLabel *type = new QLabel;
	type->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	BlockHandle *groupHandle = NULL;

	QToolButton *modify = new QToolButton;
	modify->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
	modify->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));

	// Just the definition?
	if ( struc->name.empty() ) {
		type->setText(struc->definition->type.c_str());
		modify->setIcon(QIcon(":/res/icons/add.png"));
		modify->setToolTip(QString("Create a new '%1' instance").arg(type->text()));
		//add->setFlat(true);
		//add->setIconSize(QSize(20,20));
		//add->setFixedSize(QSize(22,22));

		connect(modify, SIGNAL(clicked()), this, SLOT(addStruct()));

		hlayout->addWidget(modify);
		hlayout->addWidget(type);
		hlayout->addStretch();
	}
	else {
		// Build header
		groupHandle = new BlockHandle;
		groupHandle->setCheckable(true);
		hlayout->addWidget(groupHandle);

		type->setText(struc->name.c_str());
		modify->setIcon(QIcon(":/res/icons/remove.png"));
		modify->setToolTip(QString("Delete structure '%1'").arg(type->text()));

		connect(modify, SIGNAL(clicked()), this, SLOT(removeStruct()));

		QFont f = type->font();
		f.setBold(true);
		f.setItalic(true);
		type->setFont(f);
		QPalette pal = type->palette();
		pal.setColor(QPalette::Text, TextColor);
		type->setPalette(pal);

		groupHandle->setChecked(true);

		hlayout->addWidget(type);
		hlayout->addWidget(new HRuler);
		hlayout->addWidget(modify);
	}

	layout->addLayout(hlayout);

	item.label = type;

	if ( !struc->name.empty() ) {
		// Build group widget
		QWidget *groupWidget = new BlockWidget;
		groupWidget->setContentsMargins(8,0,0,0);

		QVBoxLayout *groupLayout = new QVBoxLayout;
		groupWidget->setLayout(groupLayout);

		if ( !struc->definition->description.empty() ) {
			DescLabel *desc = new DescLabel;
			desc->setContentsMargins(8,0,0,0);
			desc->setText(struc->definition->description.c_str());
			//desc->setWordWrap(true);
			QPalette pal = desc->palette();
			pal.setColor(QPalette::Text, DescColor);
			desc->setPalette(pal);
			layout->addWidget(desc);
			item.description = desc;
		}

		layout->addWidget(groupWidget);
		layout = groupLayout;

		groupWidget->setVisible(groupHandle->isChecked());

		connect(groupHandle, SIGNAL(toggled(bool)),
		        groupWidget, SLOT(setVisible(bool)));
	}

	return true;
}


FancyViewItem FancyView::add(QLayout *layout, const QModelIndex &idx) {
	Parameter *param = reinterpret_cast<Parameter*>(idx.data(ConfigurationTreeItemModel::Link).value<void*>());
	if ( param == NULL ) return FancyViewItem();

	ViewItemWidget *paramWidget = new ViewItemWidget;
	QVBoxLayout *paramLayout = new QVBoxLayout;
	paramLayout->setSpacing(0);
	paramWidget->setLayout(paramLayout);

	paramWidget->setObjectName(param->definition->name.c_str());

	// Value not set but imported?
	bool isOverridden = param->symbol.stage > _configStage;
	bool isDefined = param->symbol.stage > Environment::CS_UNDEFINED;

	FancyViewItemEdit *inputWidget;
	QWidget *textWidget;
	QHBoxLayout *nameLayout = new QHBoxLayout;

	QString paramLabel = idx.data().toString();
	if ( !param->definition->unit.empty() )
		paramLabel += QString(" [%1]").arg(param->definition->unit.c_str());

	if ( param->definition->type == "boolean" ) {
		paramLayout->addStretch();

		BoolEdit *checkBox = new BoolEdit;
		QFont f = checkBox->font();
		f.setBold(true);
		checkBox->setFont(f);
		checkBox->setText(paramLabel);
		checkBox->setValue(idx.sibling(idx.row(),2).data().toString());
		inputWidget = checkBox;
		textWidget = checkBox;

		connect(checkBox, SIGNAL(toggled(bool)),
		        this, SLOT(optionToggled(bool)));

		nameLayout->addWidget(checkBox);
		paramLayout->addLayout(nameLayout);

		if ( isOverridden ) {
			QPalette pal = checkBox->palette();
			QColor oldButton = pal.color(QPalette::Disabled, QPalette::Button);
			pal.setColor(QPalette::Button, AlertColor);
			pal.setColor(QPalette::ButtonText, Qt::white);
			pal.setColor(QPalette::Disabled, QPalette::Button, blend(AlertColor, oldButton, 50));
			checkBox->setPalette(pal);
		}
	}
	else {
		QLabel *name = new QLabel;
		QFont f = name->font();
		f.setBold(true);
		name->setFont(f);
		name->setText(paramLabel);

		StringEdit *edit = new StringEdit;
		edit->setValue(idx.sibling(idx.row(),2).data().toString());

		textWidget = name;
		inputWidget = edit;

		connect(edit, SIGNAL(editingFinished()),
		        this, SLOT(optionTextEdited()));
		connect(edit, SIGNAL(textEdited(const QString&)),
		        this, SLOT(optionTextChanged(const QString&)));

		nameLayout->addWidget(name);
		paramLayout->addLayout(nameLayout);

		if ( isOverridden ) {
			QPalette pal = edit->palette();

			QColor oldBase = pal.color(QPalette::Disabled, QPalette::Base);
			QColor oldWindow = pal.color(QPalette::Disabled, QPalette::Window);
			QColor oldText = pal.color(QPalette::Disabled, QPalette::Text);

			pal.setColor(QPalette::Base, AlertColor);
			pal.setColor(QPalette::Text, Qt::white);
			pal.setColor(QPalette::Disabled, QPalette::Base, blend(AlertColor, oldBase, 50));
			pal.setColor(QPalette::Disabled, QPalette::Window, blend(AlertColor, oldWindow, 50));
			pal.setColor(QPalette::Disabled, QPalette::Text, blend(Qt::white, oldText, 50));
			edit->setPalette(pal);
		}

		paramLayout->addWidget(edit);
	}

	nameLayout->addStretch();
	QAbstractButton *locker = new IconButton(_unlockIcon, _lockIcon);
	nameLayout->addWidget(locker);

	vector<string> values;
	QString eval;
	string errmsg;
	if ( Config::Config::Eval(param->symbol.content, values, true, NULL, &errmsg) ) {
		for ( size_t i = 0; i < values.size(); ++i ) {
			if ( i > 0 ) eval += "<br/>";
			eval += QString(values[i].c_str()).replace(' ', "&nbsp;");
		}
	}
	else
		eval = QString("<i>%1</i>").arg(errmsg.c_str()).replace('\n', "<br/>");

	QString toolTip = QString("<b>Location</b><br/>%1<br/><br/>"
	                          "<b>Evaluated</b><br/>%2")
	                  .arg(param->symbol.uri.c_str())
	                  .arg(eval);

	if ( isOverridden ) {
		toolTip += QString("<br/><br/><b>WARNING</b><br/><i>This value is overridden in a "
		                   "later stage which supersedes the current stage. "
		                   "Whatever is entered here will not be active in the "
		                   "final configuration. The superseded value is used instead.</i>");
	}

	inputWidget->widget()->setToolTip(toolTip);

	FancyViewItem item(idx, paramWidget);
	item.label = textWidget;
	item.input = inputWidget;

	if ( !param->definition->description.empty() ) {
		DescLabel *desc = new DescLabel;
		desc->setText(maxSize(param->definition->description, 60).c_str());
		QString content(param->definition->description.c_str());
		content
		.replace("<", "&lt;")
		.replace(">", "&gt;")
		.replace('\n', "<br/>");

		QString toolTip = QString("<p>%1</p>").arg(content);
		desc->setToolTip(toolTip);

		//desc->setWordWrap(true);
		QPalette pal = desc->palette();
		pal.setColor(QPalette::Text, Qt::gray);
		desc->setPalette(pal);
		/*
		f = desc->font();
		f.setBold(true);
		name->setFont(f);
		*/
		paramLayout->addWidget(desc);
		item.description = desc;

		if ( item.input->widget() )
			item.input->widget()->setWhatsThis(param->definition->description.c_str());
	}
	else
		item.description = NULL;

	if ( (idx.sibling(idx.row(),2).flags() & Qt::ItemIsEnabled) == 0 ) {
		textWidget->setEnabled(false);
		inputWidget->widget()->setEnabled(false);
	}

	if ( locker ) {
		locker->setCheckable(true);
		locker->setFixedSize(16,16);
		locker->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));

		if ( idx.sibling(idx.row(),3).data().toBool() ) {
			locker->setChecked(true);
			locker->setToolTip(isDefined?
			                   "This parameter is locked. Its is already "
			                   "defined in an earlier or later configuration stage.\n"
			                   "If you want to redefine it you can press "
			                   "the button to unlock it."
			                   :
			                   "This parameter is currently not set and the "
			                   "default value is displayed.\n"
			                   "To redefine the parameter, press the button "
			                   "to unlock it.");
		}
		else {
			locker->setChecked(false);
			locker->setToolTip("This parameter is present in the application "
			                   "configuration. To remove the parameter\nin "
			                   "order to use the applications default, press "
			                   "the button and lock it.");
		}

		connect(locker, SIGNAL(toggled(bool)), this, SLOT(lockChanged(bool)));
	}

	paramLayout->addStretch();

	layout->addWidget(paramWidget);

	// Link the view item with the input widget
	inputWidget->widget()->setProperty("viewItem", QVariant::fromValue<FancyViewItem>(item));

	return item;
}


void FancyView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight) {
	ViewItems::iterator it = _viewItems.find(topLeft.sibling(topLeft.row(),0));
	if ( it == _viewItems.end() ) return;

	if ( _blockPopulate != it.value().input->widget() ) {
		it.value().label->setEnabled(model()->flags(topLeft.sibling(topLeft.row(),2)) & Qt::ItemIsEnabled);
		it.value().input->widget()->setEnabled(model()->flags(topLeft.sibling(topLeft.row(),2)) & Qt::ItemIsEnabled);

		// Change values
		if ( topLeft.column() == 2 )
			it.value().input->setValue(topLeft.data().toString());
	}

	QAbstractItemView::dataChanged(topLeft, bottomRight);
}


void FancyView::rowsInserted(const QModelIndex &parent, int start, int end) {
	QAbstractItemView::rowsInserted(parent, start, end);
}


void FancyView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end) {
	QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
}


bool FancyView::eventFilter(QObject *o, QEvent *e) {
	if ( o == _rootWidget && e->type() == QEvent::LayoutRequest )
		updateContentGeometry();

	return false;
}


bool FancyView::edit(const QModelIndex &index, EditTrigger trigger, QEvent *event) {
	return false;
}


QModelIndex FancyView::moveCursor(QAbstractItemView::CursorAction cursorAction,
                                  Qt::KeyboardModifiers modifiers) {
	return QModelIndex();
}


int FancyView::horizontalOffset() const {
	return horizontalScrollBar()->value();
}


int FancyView::verticalOffset() const {
	return verticalScrollBar()->value();
}


bool FancyView::isIndexHidden(const QModelIndex &index) const {
	return false;
}


void FancyView::setSelection(const QRect&, QItemSelectionModel::SelectionFlags command) {

}


void FancyView::mousePressEvent(QMouseEvent *event) {

}


void FancyView::mouseMoveEvent(QMouseEvent *event) {

}


void FancyView::mouseReleaseEvent(QMouseEvent *event) {

}


void FancyView::paintEvent(QPaintEvent *event) {
	QAbstractItemView::paintEvent(event);
}


void FancyView::lockChanged(bool state) {
	QWidget *w = static_cast<QWidget*>(sender());

	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();
	if ( !item.isValid() ) return;

	Parameter *param = reinterpret_cast<Parameter*>(
		item.index.sibling(item.index.row(),0).data(ConfigurationTreeItemModel::Link).value<void*>()
	);

	//if ( item.input ) item.input->setDisabled(state);
	//if ( item.label ) item.label->setDisabled(state);

	model()->setData(item.index.sibling(item.index.row(), 3), state);

	updateToolTip(item.input->widget(), param);
}


void FancyView::updateToolTip(QWidget *w, Seiscomp::System::Parameter *param) {
	bool isOverridden = param->symbol.stage > _configStage;

	vector<string> values;
	QString eval;
	string errmsg;
	if ( Config::Config::Eval(param->symbol.content, values, true, NULL, &errmsg) ) {
		for ( size_t i = 0; i < values.size(); ++i ) {
			if ( i > 0 ) eval += "<br/>";
			eval += QString(values[i].c_str()).replace(' ', "&nbsp;");
		}
	}
	else
		eval = QString("<i>%1</i>").arg(errmsg.c_str()).replace('\n', "<br/>");

	QString toolTip = QString("<b>Location</b><br/>%1<br/><br/>"
	                          "<b>Evaluated</b><br/>%2")
	                  .arg(param->symbol.uri.c_str())
	                  .arg(eval);

	if ( isOverridden ) {
		toolTip += QString("<br/><br/><b>WARNING</b><br/><i>This value is overridden in a "
		                   "later stage which supersedes the current stage. "
		                   "Whatever is entered here will not be active in the "
		                   "final configuration. The superseded value is used instead.</i>");
	}

	w->setToolTip(toolTip);
}


void FancyView::optionTextEdited() {
	if ( _optionEditHint != NULL ) _optionEditHint->hide();

	QWidget *w = static_cast<QWidget*>(sender());

	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();
	if ( !item.isValid() ) return;

	Parameter *param = reinterpret_cast<Parameter*>(
		item.index.sibling(item.index.row(),0).data(ConfigurationTreeItemModel::Link).value<void*>()
	);

	model()->setData(item.index.sibling(item.index.row(), 2), item.input->value());
	/*
	if ( model()->data(item.index.sibling(item.index.row(), 2)).toString() != text )
		item.input->setValue(model()->data(item.index.sibling(item.index.row(), 2)).toString());
	*/

	//setFocus(Qt::ActiveWindowFocusReason);

	updateToolTip(item.input->widget(), param);
}


void FancyView::optionTextChanged(const QString &txt) {
	QWidget *w = static_cast<QWidget*>(sender());

	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();
	if ( !item.isValid() ) return;

	if ( _optionEditHint == NULL ) {
		_optionEditHint = new EvalHintWidget(this);
		_optionEditHint->setMargin(6);
	}

	/*
	Parameter *param = reinterpret_cast<Parameter*>(
		item.index.sibling(item.index.row(),0).data(ConfigurationTreeItemModel::Link).value<void*>()
	);
	*/

	vector<string> values;
	QString eval;
	string errmsg;
	QPalette pal = _optionEditHint->palette();
	pal.setColor(QPalette::Window, QColor(255,255,255,192));

	if ( Config::Config::Eval(item.input->value().toStdString(), values, true, NULL, &errmsg) ) {
		for ( size_t i = 0; i < values.size(); ++i ) {
			if ( i > 0 ) eval += "<hr/>";
			eval += QString(values[i].c_str()).replace(' ', "&nbsp;");
		}
		pal.setColor(QPalette::WindowText, QColor(32,128,32));

		_optionEditHint->setText(QString("<b>Evaluation</b> (%1 item%2)<br/><br/>%3")
		                         .arg(values.size()).arg(values.size() == 1?"":"s")
		                         .arg(eval));
	}
	else {
		pal.setColor(QPalette::WindowText, QColor(128,32,32));
		eval = QString("<i>%1</i>").arg(errmsg.c_str()).replace('\n', "<br/>");

		_optionEditHint->setText(QString("<b>Error</b><br/><br/>%1")
		                         .arg(eval));
	}

	_optionEditHint->setPalette(pal);
	QSize size = _optionEditHint->sizeHint();
	_optionEditHint->resize(size);
	// Find best position

	QPoint tl = item.input->widget()->mapToGlobal(QPoint(0,0));
	QPoint br = item.input->widget()->mapToGlobal(QPoint(item.input->widget()->width(),
	                                                     item.input->widget()->height()));

	tl -= QPoint(-6,-6);
	br += QPoint(+6,+6);

	tl = mapFromGlobal(tl);
	br = mapFromGlobal(br);

	int x,y;

	if ( tl.x() + size.width() <= width() )
		x = tl.x();
	else {
		x = width()-size.width();
		if ( x < 0 ) x = 0;
	}

	if ( br.y() + size.height() <= height() || (height()-br.y()) >= tl.y() )
		y = br.y();
	else
		y = tl.y() - size.height();

	_optionEditHint->move(x,y);
	_optionEditHint->show();

	_blockPopulate = w;
	model()->setData(item.index.sibling(item.index.row(), 2), item.input->value());
	_blockPopulate = NULL;
}


void FancyView::optionToggled(bool opt) {
	QWidget *w = static_cast<QWidget*>(sender());

	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();
	if ( !item.isValid() ) return;

	model()->setData(item.index.sibling(item.index.row(), 2), item.input->value());
}


void FancyView::bindingCategoryChanged(int idx) {
	/*
	QComboBox *comboBox = static_cast<QComboBox*>(sender());

	FancyViewItem item = comboBox->property("viewItem").value<FancyViewItem>();
	BindingCategory *cat = reinterpret_cast<BindingCategory*>(item.index.data(ConfigurationTreeItemModel::Link).value<void*>());

	QLayout *l = reinterpret_cast<QLayout*>(comboBox->property("cat.layout").value<void*>());
	QString name;

	if ( idx > 0 )
		name = comboBox->itemData(idx).toString();

	cat->activeBinding = NULL;

	// Update visibility state
	for ( int i = 0; i < l->count(); ++i ) {
		QWidget *child = l->itemAt(i)->widget();
		Binding *b = reinterpret_cast<Binding*>(child->property("viewBinding").value<void*>());
		if ( (b && name == b->definition->name.c_str()) || (!b && idx == 0) ) {
			child->setVisible(true);
			cat->activeBinding = b;
		}
		else
			child->setVisible(false);
	}

	// Update link to trigger model.dataChanged signal
	model()->setData(item.index, qVariantFromValue((void*)cat), ConfigurationTreeItemModel::Link);
	*/
}


void FancyView::addCategoryBinding() {
	QWidget *w = (QWidget*)sender();
	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();
	QComboBox *cb = (QComboBox*)w->property("comboBox").value<void*>();
	QString type = cb->itemData(cb->currentIndex()).toString();
	if ( type.isEmpty() ) {
		QMessageBox::critical(NULL, "Internal error",
		                      "The type must not be empty.");
		return;
	}

	BindingCategory *cat = reinterpret_cast<BindingCategory*>(item.index.data(ConfigurationTreeItemModel::Link).value<void*>());
	Binding *typeBinding = cat->binding(type.toStdString());
	if ( typeBinding == NULL ) {
		QMessageBox::critical(NULL, "Internal error",
		                      "The selected type is not available.");
		return;
	}

	NewCatBindingDialog dlg(cat, type.toStdString());
	if ( dlg.exec() != QDialog::Accepted ) return;

	Binding *nb = cat->instantiate(typeBinding, dlg.name().c_str());
	if ( nb == NULL ) {
		QMessageBox::critical(NULL, "Internal error",
		                      "Adding binding failed.");
		return;
	}

	const char *alias = cat->alias(nb);

	// Propagate the new entry to the model and create new widgets
	int row = item.index.model()->rowCount(item.index);
	model()->insertRow(row, item.index);
	QModelIndex ni = item.index.child(row, 0);
	model()->setData(ni, alias);
	model()->setData(ni, item.index.data(ConfigurationTreeItemModel::Level), ConfigurationTreeItemModel::Level);
	model()->setData(ni, qVariantFromValue((void*)nb), ConfigurationTreeItemModel::Link);
	model()->setData(ni, ConfigurationTreeItemModel::TypeCategoryBinding, ConfigurationTreeItemModel::Type);

	QWidget *status = (QWidget*)item.container->property("statusLabel").value<void*>();
	if ( status ) status->setVisible(cat->bindings.empty());

	QBoxLayout *l = (QBoxLayout*)w->parentWidget()->layout();
	l->insertWidget(row+1, createWidgetFromIndex(ni, ""));
}


void FancyView::removeCategoryBinding() {
	QWidget *w = (QWidget*)sender();
	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();

	Binding *b = reinterpret_cast<Binding*>(item.index.data(ConfigurationTreeItemModel::Link).value<void*>());
	BindingCategory *c = reinterpret_cast<BindingCategory*>(item.index.parent().data(ConfigurationTreeItemModel::Link).value<void*>());

	if ( !c->removeInstance(b) ) {
		cerr << "ERROR: failed to remove binding from category, registered "
		        "bindings: " << c->bindings.size() << endl;
		return;
	}

	QWidget *status = (QWidget*)item.container->property("statusLabel").value<void*>();
	if ( status ) status->setVisible(c->bindings.empty());

	ViewItems::iterator it = _viewItems.find(item.index);
	if ( it != _viewItems.end() )
		_viewItems.erase(it);
	else
		std::cerr << "ERROR: view item does not exist for index" << std::endl;

	model()->removeRow(item.index.row(), item.index.parent());

	if ( item.container ) delete item.container;
}


void FancyView::addStruct() {
	QWidget *w = (QWidget*)sender();
	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();

	Structure *s = reinterpret_cast<Structure*>(item.index.data(ConfigurationTreeItemModel::Link).value<void*>());
	Container *c = reinterpret_cast<Container*>(item.index.parent().data(ConfigurationTreeItemModel::Link).value<void*>());

	NewStructDialog dlg(c);
	if ( dlg.exec() != QDialog::Accepted ) return;

	Structure *ns = c->instantiate(s, qPrintable(dlg.name()));
	if ( ns ) {
		// Propagate the new entry to the model and create new widgets
		int row = item.index.row();//model()->rowCount(item.index.parent());
		model()->insertRow(row, item.index.parent());
		QModelIndex ni = item.index.parent().child(row, 0);
		model()->setData(ni, ns->name.c_str());
		model()->setData(ni, item.index.data(ConfigurationTreeItemModel::Level), ConfigurationTreeItemModel::Level);
		model()->setData(ni, qVariantFromValue((void*)ns), ConfigurationTreeItemModel::Link);
		model()->setData(ni, ConfigurationTreeItemModel::TypeStruct, ConfigurationTreeItemModel::Type);

		QBoxLayout *l = (QBoxLayout*)item.container->parentWidget()->layout();
		l->insertWidget(row, createWidgetFromIndex(ni, ""));
	}
}


void FancyView::removeStruct() {
	QWidget *w = (QWidget*)sender();
	FancyViewItem item = w->property("viewItem").value<FancyViewItem>();

	Structure *s = reinterpret_cast<Structure*>(item.index.data(ConfigurationTreeItemModel::Link).value<void*>());
	Container *c = reinterpret_cast<Container*>(item.index.parent().data(ConfigurationTreeItemModel::Link).value<void*>());

	if ( !c->remove(s) ) {
		cerr << "ERROR: failed to remove structure from container, registered "
		        "structures: " << c->structures.size() << endl;
		return;
	}

	ViewItems::iterator it = _viewItems.find(item.index);
	if ( it != _viewItems.end() )
		_viewItems.erase(it);
	else
		cerr << "ERROR: view item does not exist for index" << endl;

	model()->removeRow(item.index.row(), item.index.parent());

	if ( item.container ) delete item.container;
}


void FancyView::updateContentGeometry() {
	if ( _rootWidget == NULL ) return;

	QSize p = viewport()->size();
	QSize min = qSmartMinSize(_rootWidget);

	if ((_rootWidget->layout() ? _rootWidget->layout()->hasHeightForWidth() : _rootWidget->sizePolicy().hasHeightForWidth())) {
		QSize p_hfw = p.expandedTo(min);
		int h = _rootWidget->heightForWidth(p_hfw.width());
		min = QSize(p_hfw.width(), qMax(p_hfw.height(), h));
	}

	min = p.expandedTo(min);

	_rootWidget->setGeometry(-horizontalScrollBar()->value(),
	                         -verticalScrollBar()->value(),
	                         min.width(), min.height());

	horizontalScrollBar()->setPageStep(viewport()->width());
	horizontalScrollBar()->setRange(0, qMax(0, _rootWidget->width() - viewport()->width()));
	verticalScrollBar()->setSingleStep(20);
	verticalScrollBar()->setPageStep(viewport()->height());
	verticalScrollBar()->setRange(0, qMax(0, _rootWidget->height() - viewport()->height()));
}


void FancyView::resizeEvent(QResizeEvent *event) {
	QAbstractItemView::resizeEvent(event);
	updateContentGeometry();
}


void FancyView::scrollContentsBy(int dx, int dy) {
	viewport()->scroll(dx, dy);
}


QRegion FancyView::visualRegionForSelection(const QItemSelection &selection) const {
	return QRegion();
}


void FancyView::currentChanged(const QModelIndex &curr, const QModelIndex &) {
	if ( _currentItem ) {
		static_cast<ViewItemWidget*>(_currentItem)->setSelected(false);
		_currentItem = NULL;
	}

	ViewItems::iterator it = _viewItems.find(curr);
	if ( it == _viewItems.end() ) return;

	_currentItem = it.value().container;
	static_cast<ViewItemWidget*>(_currentItem)->setSelected(true);
}


void FancyView::keyboardSearch(const QString &search) {
	if ( _currentItem ) {
		static_cast<ViewItemWidget*>(_currentItem)->setSelected(false);
		_currentItem = NULL;
	}

	// TODO: implement keyboard search
}
