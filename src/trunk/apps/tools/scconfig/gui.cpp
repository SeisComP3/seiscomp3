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


#define SEISCOMP_COMPONENT Configurator

#include <QtGui>

#include <iostream>
#include <list>
#include <map>

#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/system/environment.h>

#include "gui.h"
#include "fancyview.h"
#include "wizard.h"
#include "editor.h"

#include "panels/bindings.h"
#include "panels/information.h"
#include "panels/inventory.h"
#include "panels/modules.h"
#include "panels/system.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::System;


static QColor darkBg(64,64,64);
static QColor midBg(92,92,92);
static QColor midLightBg(108,108,108);
static QColor lightBg(128,128,128);

static QColor borderBg(192,192,192);
static QColor borderLightBg(240,240,240);

static QColor hoverBg(160,160,160);

static QColor normalText(255,255,255);
static QColor selectText(0,0,0);
static QColor selectBg(192,192,192);
static QColor selectLightBg(240,240,240);


enum Columns {
	NAME,
	TYPE,
	VALUE,
	LOCKED
};


class ModuleListDelegate : public QItemDelegate {
	public:
		ModuleListDelegate(QObject *parent) : QItemDelegate(parent) {}

	public:
		void paint(QPainter *painter,
		           const QStyleOptionViewItem &option,
		           const QModelIndex &index) const;

		QSize sizeHint(const QStyleOptionViewItem & option,
		               const QModelIndex & index) const;
};



class ModuleListPainter : public QObject {
	public:
		ModuleListPainter(QObject *parent = NULL) : QObject(parent) {}

	protected:
		bool eventFilter(QObject *obj, QEvent *event);
};



class ModuleNamePainter : public QObject {
	public:
		ModuleNamePainter(QObject *parent = NULL) : QObject(parent) {}

	protected:
		bool eventFilter(QObject *obj, QEvent *event) {
			if ( event->type() == QEvent::Paint ) {
				QLabel *l = static_cast<QLabel*>(obj);
				QPainter painter(l);

				int sep = 20;//l->height()*40/100;
				QLinearGradient grad(0,0,0,sep);
				grad.setColorAt(0,midBg);
				grad.setColorAt(0.25,midLightBg);
				grad.setColorAt(1,midLightBg);
				painter.fillRect(0,0,l->width(),sep,grad);

				grad = QLinearGradient(0,sep,0,40);//l->height());
				grad.setColorAt(0,midLightBg);
				grad.setColorAt(0.25,midBg);
				grad.setColorAt(1,midBg);
				painter.fillRect(0,sep,l->width(),l->height()-sep,grad);

				painter.setPen(borderBg);
				painter.drawLine(0,0,l->width(),0);
				/*
				painter.setPen(Qt::black);
				painter.drawLine(0,0,0,l->height());
				painter.drawLine(0,0,l->width(),0);
				*/

				if ( !l->text().isEmpty() ) {
					grad = QLinearGradient(0,0,l->width(),0);
					grad.setColorAt(0, QColor(255,255,255,0));
					grad.setColorAt(0.25, QColor(255,255,255,128));
					grad.setColorAt(0.5, QColor(255,255,255,192));
					grad.setColorAt(0.75, QColor(255,255,255,128));
					grad.setColorAt(1, QColor(255,255,255,0));
					painter.fillRect(0,l->height()-1,l->width(),1,grad);
				}

				const QPixmap *pm = l->pixmap();
				if ( pm != NULL ) {
					int xofs = (l->width() - pm->width()) / 2;
					int yofs = (l->height() - pm->height()) / 2;
					painter.drawPixmap(xofs,yofs, *pm);
				}

				painter.setPen(Qt::white);
				painter.drawText(l->rect().adjusted(20,0,0,0), Qt::AlignLeft | Qt::AlignVCenter, l->text());

				return true;
			}

			return QObject::eventFilter(obj, event);
		}
};


class ModuleInfoPainter : public QObject {
	public:
		ModuleInfoPainter(QObject *parent = NULL) : QObject(parent) {}

	protected:
		bool eventFilter(QObject *obj, QEvent *event) {
			if ( event->type() == QEvent::Paint ) {
				QLabel *l = static_cast<QLabel*>(obj);
				QPainter painter(l);

				painter.fillRect(l->rect(), midBg);

				/*
				painter.setPen(Qt::black);
				painter.drawLine(0,0,0,l->height());
				painter.drawLine(0,l->height()-1,l->width(),l->height()-1);
				*/
				painter.setPen(normalText);

				int tw = l->fontMetrics().width(l->text());
				QRect tr = l->rect().adjusted(20,0,-20,0);

				if ( tw > tr.width() ) {
					QPixmap pixmap(tr.size());
					pixmap.fill(Qt::transparent);

					QPainter p(&pixmap);
					p.setPen(painter.pen());
					p.setFont(painter.font());

					p.drawText(pixmap.rect(), l->alignment() | Qt::TextSingleLine, l->text());

					QLinearGradient alphaGradient(QPoint(0,0), QPoint(tr.width(),0));
					alphaGradient.setColorAt(0.8, QColor(0,0,0,255));
					alphaGradient.setColorAt(1.0, QColor(0,0,0,0));
					p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
					p.fillRect(pixmap.rect(), alphaGradient);

					painter.drawPixmap(tr.topLeft(), pixmap);
				}
				else
					painter.drawText(tr, Qt::AlignLeft | Qt::AlignVCenter,
					                 l->text());

				return true;
			}

			return QObject::eventFilter(obj, event);
		}
};


class StageSelectionDialog : public QDialog {
	public:
		enum Mode {
			User,
			System
		};

		StageSelectionDialog(QWidget *parent) : QDialog(parent) {
			Environment *env = Environment::Instance();

			QVBoxLayout *layout = new QVBoxLayout;
			QHBoxLayout *hlayout;
			QLabel *label;

			label = new QLabel;
			QFont f = label->font();
			f.setBold(true);
			f.setPointSize(f.pointSize()*150/100);
			label->setFont(f);
			label->setText(tr("Select configuration mode"));
			label->setAlignment(Qt::AlignCenter);
			layout->addWidget(label);

			layout->addSpacing(fontMetrics().ascent());

			// Create dialog here
			_systemMode = new QPushButton;
			_systemMode->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
			_systemMode->setIcon(QIcon(":/res/icons/system-settings.png"));
			_systemMode->setIconSize(QSize(72,72));

			label = new QLabel;
			label->setWordWrap(true);
			label->setAlignment(Qt::AlignCenter);
			label->setText(QString(tr("Manage system configuration in <i>%1</i>.")).arg(env->appConfigDir().c_str()));

			hlayout = new QHBoxLayout;
			hlayout->addStretch();
			hlayout->addWidget(_systemMode);
			hlayout->addStretch();

			layout->addLayout(hlayout);
			layout->addWidget(label);

			QFrame *frame = new QFrame;
			frame->setFrameShape(QFrame::HLine);

			layout->addWidget(frame);

			_userMode = new QPushButton;
			_userMode->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum));
			_userMode->setIcon(QIcon(":/res/icons/user-settings.png"));
			_userMode->setIconSize(QSize(72,72));

			label = new QLabel;
			label->setWordWrap(true);
			label->setAlignment(Qt::AlignCenter);
			label->setText(QString(tr("Manage user configuration in <i>%1</i>.")).arg(env->configDir().c_str()));
			hlayout = new QHBoxLayout;
			hlayout->addStretch();
			hlayout->addWidget(_userMode);
			hlayout->addStretch();

			layout->addLayout(hlayout);
			layout->addWidget(label);

			layout->addStretch();

			setLayout(layout);

			connect(_userMode, SIGNAL(clicked()), this, SLOT(accept()));
			connect(_systemMode, SIGNAL(clicked()), this, SLOT(accept()));
		}

		void accept() {
			QObject *w = sender();

			if ( w == _userMode )
				_mode = User;
			else if ( w == _systemMode )
				_mode = System;

			QDialog::accept();
		}

		Mode mode() { return _mode; }


	private:
		Mode         _mode;
		QPushButton *_userMode;
		QPushButton *_systemMode;
};


class MouseTrackLabel : public QLabel {
	public:
		MouseTrackLabel(QWidget *parent = NULL, Qt::WindowFlags f = 0)
		: QLabel(parent, f), _hovered(false) { setMouseTracking(true); }

		void setIcon(const QIcon &icon, const QSize &size) {
			_icon = icon;
			_iconSize = size;
			updatePixmap();
		}

	protected:
		void enterEvent(QEvent *) {
			_hovered = true;
			updatePixmap();
			setCursor(Qt::PointingHandCursor);
		}

		void leaveEvent(QEvent *) {
			_hovered = false;
			updatePixmap();
			setCursor(Qt::ArrowCursor);
		}

	private:
		void updatePixmap() {
			if ( _hovered )
				setPixmap(_icon.pixmap(_iconSize));
			else
				setPixmap(_icon.pixmap(_iconSize, QIcon::Disabled));
		}

	private:
		bool  _hovered;
		QIcon _icon;
		QSize _iconSize;
};


typedef QList<QStandardItem*> QStandardItemList;



namespace {


QString paramValue(const Parameter *param, Environment::ConfigStage targetStage) {
	QString value;

	// If the current stage is defined, display its value
	if ( param->symbols[targetStage] &&
	     param->symbols[targetStage]->symbol.stage != Environment::CS_UNDEFINED ) {
		value = param->symbols[targetStage]->symbol.content.c_str();
	}
	else {
		// Otherwise use the resolved value
		value = param->symbol.content.c_str();
	}

	/*
	QString value;
	for ( size_t i = 0; i < param->symbol.values.size(); ++i ) {
		if ( i > 0 ) value += ", ";
		value += param->symbol.values[i].c_str();
	}
	*/

	return value;
}


void addParameter(QStandardItem *item, Parameter *param, int level,
                  Environment::ConfigStage targetStage) {
	QStandardItem *name = new QStandardItem(param->definition->name.c_str());
	QStandardItem *type = new QStandardItem(param->definition->type.c_str());

	QString valueText = paramValue(param, targetStage);

	QStandardItem *value = new QStandardItem(valueText);
	QStandardItem *locked = new QStandardItem();

	SymbolMapItem *symbol = param->symbols[targetStage].get();
	bool paramLocked = symbol?symbol->symbol.stage == Environment::CS_UNDEFINED:true;

	name->setData(level, ConfigurationTreeItemModel::Level);
	name->setData(qVariantFromValue((void*)param), ConfigurationTreeItemModel::Link);
	name->setData(ConfigurationTreeItemModel::TypeParameter, ConfigurationTreeItemModel::Type);
	locked->setData(paramLocked, Qt::DisplayRole);

	name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	type->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	value->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable);
	locked->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);

	name->setToolTip(param->definition->description.c_str());
	type->setToolTip(name->toolTip());
	value->setToolTip(name->toolTip());
	locked->setToolTip(param->symbol.uri.c_str());

	item->appendRow(QStandardItemList() << name << type << value << locked);

	//for ( size_t i = 0; i < param->childs.size(); ++i )
	//	addParameters(name, param->childs[i].get(), level+1);
}


void addGroup(QStandardItem *item, Group *group, int level,
              Environment::ConfigStage targetStage);
void addStructure(QStandardItem *item, const Structure *struc, int level,
                  Environment::ConfigStage targetStage);

void loadStructure(QStandardItem *item, const Structure *struc, int level,
                   Environment::ConfigStage targetStage) {
	for ( size_t i = 0; i < struc->groups.size(); ++i )
		addGroup(item, struc->groups[i].get(), level+1, targetStage);

	for ( size_t i = 0; i < struc->parameters.size(); ++i )
		addParameter(item, struc->parameters[i].get(), level+1, targetStage);

	for ( size_t i = 0; i < struc->structures.size(); ++i )
		addStructure(item, struc->structures[i].get(), level+1, targetStage);

	for ( size_t i = 0; i < struc->structureTypes.size(); ++i )
		addStructure(item, struc->structureTypes[i].get(), level+1, targetStage);
}


void addStructure(QStandardItem *item, const Structure *struc, int level,
                  Environment::ConfigStage targetStage) {
	QStandardItem *child = new QStandardItem(struc->definition->type.c_str());
	child->setData(level, ConfigurationTreeItemModel::Level);
	child->setData(qVariantFromValue((void*)struc), ConfigurationTreeItemModel::Link);
	child->setData(ConfigurationTreeItemModel::TypeStruct, ConfigurationTreeItemModel::Type);
	child->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item->appendRow(QStandardItemList() << child);
	loadStructure(child, struc, level, targetStage);
}


void addGroup(QStandardItem *item, Group *group, int level,
              Environment::ConfigStage targetStage) {
	QStandardItem *name = new QStandardItem(group->definition->name.c_str());

	name->setData(level, ConfigurationTreeItemModel::Level);
	name->setData(qVariantFromValue((void*)group), ConfigurationTreeItemModel::Link);
	name->setData(ConfigurationTreeItemModel::TypeGroup, ConfigurationTreeItemModel::Type);

	name->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

	name->setToolTip(group->definition->description.c_str());

	item->appendRow(QStandardItemList() << name);

	for ( size_t i = 0; i < group->groups.size(); ++i )
		addGroup(name, group->groups[i].get(), level+1, targetStage);

	for ( size_t i = 0; i < group->parameters.size(); ++i )
		addParameter(name, group->parameters[i].get(), level+1, targetStage);

	for ( size_t i = 0; i < group->structures.size(); ++i )
		addStructure(name, group->structures[i].get(), level+1, targetStage);

	for ( size_t i = 0; i < group->structureTypes.size(); ++i )
		addStructure(name, group->structureTypes[i].get(), level+1, targetStage);
}


QString firstLine(const QString &input) {
	return input.mid(0, input.indexOf('\n'));
}


QString multiline(const QString &input, int lineWidth) {
	if ( input.length() <= lineWidth ) return input;

	QString txt = input;
	int s = 0;
	int to = s + lineWidth;

	while ( to < input.length() ) {
		int p = input.indexOf('\n', s);
		if ( p >= 0 && p <= to ) {
			s = p+1;
			to = s + lineWidth;
			continue;
		}

		p = input.lastIndexOf(' ', to);
		if ( p <= s ) {
			txt.insert(lineWidth, '\n');
			s = lineWidth+1;
		}
		else {
			txt[p] = '\n';
			s = p+1;
		}

		to = s + lineWidth;
	}

	return txt;
}


class StaticDialog : public QDialog {
	public:
		StaticDialog() {}

		void accept() {
			setResult(QDialog::Accepted);
			emit finished(result());
		}

		void reject() {
			setResult(QDialog::Rejected);
			emit finished(result());
		}
};


struct QtConfigDelegate : System::ConfigDelegate {
	QtConfigDelegate(QSettings *sets) : dialog(NULL), settings(sets), hasErrors(false) {}
	~QtConfigDelegate() {
		if ( dialog != NULL ) {
			settings->beginGroup("CodeView");
			settings->setValue("geometry", dialog->saveGeometry());
			settings->endGroup();
			delete dialog;
		}
	}

	void log(Config::LogLevel level, const char *filename, int line, const char *msg) {
		if ( level != Config::ERROR ) return;
		errors.append(QPair<int,QString>(line, QString(msg).trimmed()));
	}

	void finishedReading(const char *filename) {
		if ( dialog == NULL ) return;

		if ( lastErrorFile == filename ) {
			handleReadError(filename);
			lastErrorFile = QString();
		}
	}

	bool handleReadError(const char *filename) {
		lastErrorFile = filename;

		if ( dialog == NULL ) {
			dialog = new StaticDialog;
			dialog->setWindowModality(Qt::ApplicationModal);

			QVBoxLayout *l = new QVBoxLayout;
			l->setSpacing(0);
			l->setMargin(0);

			headerLabel = new StatusLabel;
			headerLabel->setWordWrap(true);
			l->addWidget(headerLabel);

			fileWidget = new ConfigFileWidget;
			l->addWidget(fileWidget);

			dialog->resize(800,600);
			settings->beginGroup("CodeView");
			dialog->restoreGeometry(settings->value("geometry").toByteArray());
			settings->endGroup();

			QHBoxLayout *buttonLayout = new QHBoxLayout;
			buttonLayout->addStretch();
			buttonLayout->setMargin(4);

			okButton = new QPushButton;
			cancelButton = new QPushButton;

			QObject::connect(okButton, SIGNAL(clicked()), dialog, SLOT(accept()));
			QObject::connect(cancelButton, SIGNAL(clicked()), dialog, SLOT(reject()));

			buttonLayout->addWidget(okButton);
			buttonLayout->addWidget(cancelButton);

			l->addLayout(buttonLayout);

			dialog->setLayout(l);
		}

		dialog->setWindowTitle(QString("CodeView %1").arg(filename));

		if ( errors.empty() ) {
			okButton->setText(QObject::tr("OK"));
			okButton->show();
			cancelButton->hide();
			headerLabel->setSuccessText(QString("%1 parsed successfully").arg(filename));
		}
		else {
			okButton->setText(QObject::tr("Reload"));
			okButton->show();
			cancelButton->setText(QObject::tr("Ignore"));
			cancelButton->show();
			headerLabel->setErrorText(QString("%1 failed to parse.\n"
			                                  "Please correct the errors "
			                                  "below and press 'Reload' to continue with a "
			                                  "correct configuration file.\n"
			                                  "'Ignore' will ignore the errors which "
			                                  "can lead to undesired behaviour.")
			                          .arg(filename));
		}

		if ( !fileWidget->loadFile(filename) )
			return false;

		fileWidget->setErrors(errors, true);

		errors.clear();

		QEventLoop eventLoop;

		if ( !dialog->isVisible() )
			dialog->show();

		QObject::connect(dialog, SIGNAL(finished(int)), &eventLoop, SLOT(quit()));
		eventLoop.exec();

		int res = dialog->result();

		if ( res == QDialog::Rejected ) {
			hasErrors = true;
			return false;
		}

		fileWidget->saveFile(filename);
		return true;
	}

	void caseSensitivityConflict(const CSConflict &csc) {
		SEISCOMP_WARNING("%s: possible cs conflict: %s should be %s",
		                 csc.symbol->uri.c_str(), csc.symbol->name.c_str(),
		                 csc.parameter->variableName.c_str());
		conflicts.append(csc);
	}

	void showConflicts() {
		// No conflicts?
		if ( conflicts.empty() ) return;

		QDialog dlg;
		QVBoxLayout *l = new QVBoxLayout;
		l->setSpacing(0);
		l->setMargin(0);
		dlg.setLayout(l);

		StatusLabel *headerLabel = new StatusLabel;
		headerLabel->setWordWrap(true);
		l->addWidget(headerLabel);

		headerLabel->setWarningText(headerLabel->tr("Found possible conflicts due to "
		                            "case sensitivity of parameter names.\n"
		                            "Files which are disabled below are not under control "
		                            "of scconfig and cannot be changed. This needs to be fixed by "
		                            "either recreating aliases or updating the default configuration files."));

		ConfigConflictWidget *w = new ConfigConflictWidget;
		w->setConflicts(conflicts);
		l->addWidget(w);

		QHBoxLayout *buttonLayout = new QHBoxLayout;
		buttonLayout->setMargin(4);

		QPushButton *fix = new QPushButton;
		buttonLayout->addWidget(fix);
		fix->setText(fix->tr("Fix parameter(s)"));
		fix->setToolTip(fix->tr("Fixes all selected conflicts by correcting the parameter names."));

		buttonLayout->addStretch();

		QPushButton *close = new QPushButton;
		buttonLayout->addWidget(close);
		close->setText(close->tr("Close"));

		QObject::connect(fix, SIGNAL(clicked()), w, SLOT(fixConflicts()));
		QObject::connect(close, SIGNAL(clicked()), &dlg, SLOT(accept()));

		l->addLayout(buttonLayout);

		dlg.resize(800,600);
		settings->beginGroup("Conflicts");
		dlg.restoreGeometry(settings->value("geometry").toByteArray());
		settings->endGroup();

		dlg.exec();

		settings->beginGroup("Conflicts");
		settings->setValue("geometry", dlg.saveGeometry());
		settings->endGroup();
	}

	QDialog *dialog;
	QSettings *settings;
	StatusLabel *headerLabel;
	ConfigFileWidget *fileWidget;
	QPushButton *okButton;
	QPushButton *cancelButton;
	QString lastErrorFile;
	QList<ConfigFileWidget::Error> errors;
	bool hasErrors;
	QList<CSConflict> conflicts;
};


}


StatusLabel::StatusLabel(QWidget *parent) : QLabel(parent) {
	setMargin(4);
	setInfoText("");
	setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
}


void StatusLabel::setInfoText(const QString &t) {
	_icon = QIcon(":/res/icons/info_small.png").pixmap(16,16);
	setMinimumHeight(_icon.height()+margin()*2);

	QPalette pal = palette();
	pal.setColor(QPalette::WindowText, Qt::darkGray);
	setPalette(pal);

	QLabel::setText(t);
}


void StatusLabel::setSuccessText(const QString &t) {
	_icon = QIcon(":/res/icons/success_small.png").pixmap(16,16);
	setMinimumHeight(_icon.height()+margin()*2);

	QPalette pal = palette();
	pal.setColor(QPalette::WindowText, Qt::darkGray);
	setPalette(pal);

	QLabel::setText(t);
}

void StatusLabel::setWarningText(const QString &t) {
	_icon = QIcon(":/res/icons/warning_small.png").pixmap(16,16);
	setMinimumHeight(_icon.height()+margin()*2);

	QPalette pal = palette();
	pal.setColor(QPalette::WindowText, Qt::darkRed);
	setPalette(pal);

	QLabel::setText(t);
}


void StatusLabel::setErrorText(const QString &t) {
	_icon = QIcon(":/res/icons/error_small.png").pixmap(16,16);
	setMinimumHeight(_icon.height()+margin()*2);

	QPalette pal = palette();
	pal.setColor(QPalette::WindowText, Qt::red);
	setPalette(pal);

	QLabel::setText(t);
}


void StatusLabel::paintEvent(QPaintEvent *) {
	QPainter p(this);
	p.fillRect(rect(), QColor(255,255,204));

	p.drawPixmap(margin(), (height()-_icon.height())/2, _icon);

	if ( wordWrap() )
		p.drawText(rect().adjusted(margin()+_icon.width()+4, margin(), -margin(), -margin()), alignment() | Qt::TextWordWrap, text());
	else
		p.drawText(rect().adjusted(margin()+_icon.width()+4, margin(), -margin(), -margin()), alignment(), text());

	p.setPen(Qt::gray);
	p.drawLine(0,0,width(),0);
	p.drawLine(0,height()-1,width(),height()-1);
}


ClickFilter::ClickFilter(QObject *parent) : QObject(parent) {}


bool ClickFilter::eventFilter(QObject *obj, QEvent *event) {
	if ( event->type() == QEvent::MouseButtonRelease ) {
		QMouseEvent *e = static_cast<QMouseEvent*>(event);
		if ( e->button() == Qt::LeftButton ) {
			emit clicked(obj);
			return true;
		}
	}

	return QObject::eventFilter(obj, event);
}



ConfigurationTreeItemModel::ConfigurationTreeItemModel(
	QObject *parent,
	System::Model *tree,
	Environment::ConfigStage stage
) : QStandardItemModel(parent), _modified(false)
{
	if ( tree ) setModel(tree, stage);
	_configStage = Environment::CS_CONFIG_APP;
}


void ConfigurationTreeItemModel::setModel(System::Model *tree,
                                          Environment::ConfigStage stage) {
	_model = tree;
	_configStage = stage;

	clear();
	setColumnCount(4);
	setHorizontalHeaderLabels(QStringList() << "Name" << "Type" << "Value" << "Locked");

	QStandardItem *root = invisibleRootItem();

	for ( size_t i = 0; i < tree->modules.size(); ++i ) {
		Module *mod = tree->modules[i].get();
		QStandardItem *modItem = new QStandardItem(mod->definition->name.c_str());
		modItem->setData(0, Level);
		modItem->setData(qVariantFromValue((void*)mod), Link);
		modItem->setData(TypeModule, Type);
		modItem->setColumnCount(columnCount());
		modItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

		root->appendRow(modItem);

		for ( size_t j = 0; j < mod->sections.size(); ++j ) {
			Section *sec = mod->sections[j].get();
			QStandardItem *secItem = new QStandardItem(sec->name.c_str());
			secItem->setData(1, Level);
			secItem->setData(qVariantFromValue((void*)sec), Link);
			secItem->setData(TypeSection, Type);
			secItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			modItem->appendRow(secItem);

			for ( size_t g = 0; g < sec->groups.size(); ++g )
				addGroup(secItem, sec->groups[g].get(), 2, _configStage);

			for ( size_t p = 0; p < sec->parameters.size(); ++p )
				addParameter(secItem, sec->parameters[p].get(), 2, _configStage);

			for ( size_t s = 0; s < sec->structureTypes.size(); ++s )
				addStructure(secItem, sec->structureTypes[s].get(), 2, _configStage);
		}
	}
}


void ConfigurationTreeItemModel::setModel(System::ModuleBinding *b) {
	_model = NULL;
	// The stage for the binding is always config app
	_configStage = Environment::CS_CONFIG_APP;

	clear();
	setColumnCount(4);
	setHorizontalHeaderLabels(QStringList() << "Name" << "Type" << "Value" << "Locked");

	QStandardItem *root = invisibleRootItem();

	Module *mod = static_cast<Module*>(b->parent);

	QStandardItem *bindItem = new QStandardItem(mod->definition->name.c_str());
	bindItem->setData(0, Level);
	bindItem->setData(qVariantFromValue((void*)b), Link);
	bindItem->setData(TypeBinding, Type);
	bindItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	root->appendRow(bindItem);

	QStandardItem *secItem;

	for ( size_t s = 0; s < b->sections.size(); ++s ) {
		Section *sec = b->sections[s].get();

		secItem = new QStandardItem(sec->name.c_str());
		secItem->setData(1, Level);
		secItem->setData(qVariantFromValue((void*)sec), Link);
		secItem->setData(TypeSection, Type);
		secItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		bindItem->appendRow(secItem);

		for ( size_t g = 0; g < sec->groups.size(); ++g )
			addGroup(secItem, sec->groups[g].get(), 2, _configStage);

		for ( size_t i = 0; i < sec->structures.size(); ++i )
			addStructure(secItem, sec->structures[i].get(), 2, _configStage);

		for ( size_t i = 0; i < sec->structureTypes.size(); ++i )
			addStructure(secItem, sec->structureTypes[i].get(), 2, _configStage);

		for ( size_t p = 0; p < sec->parameters.size(); ++p )
			addParameter(secItem, sec->parameters[p].get(), 2, _configStage);
	}

	for ( size_t c = 0; c < b->categories.size(); ++c ) {
		BindingCategory *cat = b->categories[c].get();
		QStandardItem *catItem = new QStandardItem(cat->name.c_str());
		catItem->setData(1, Level);
		catItem->setData(qVariantFromValue((void*)cat), Link);
		catItem->setData(TypeCategory, Type);
		catItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		catItem->setColumnCount(columnCount());
		bindItem->appendRow(catItem);

		for ( size_t s = 0; s < cat->bindings.size(); ++s ) {
			Binding *catBinding = cat->bindings[s].binding.get();

			secItem = new QStandardItem(cat->bindings[s].alias.c_str());
			secItem->setData(2, Level);
			secItem->setData(qVariantFromValue((void*)catBinding), Link);
			secItem->setData(TypeCategoryBinding, Type);
			secItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
			catItem->appendRow(secItem);

			for ( size_t i = 0; i < catBinding->sections.size(); ++i ) {
				Section *sec = catBinding->sections[i].get();

				for ( size_t g = 0; g < sec->groups.size(); ++g )
					addGroup(secItem, sec->groups[g].get(), 3, _configStage);

				for ( size_t p = 0; p < sec->parameters.size(); ++p )
					addParameter(secItem, sec->parameters[p].get(), 3, _configStage);

				for ( size_t i = 0; i < sec->structures.size(); ++i )
					addStructure(secItem, sec->structures[i].get(), 3, _configStage);

				for ( size_t i = 0; i < sec->structureTypes.size(); ++i )
					addStructure(secItem, sec->structureTypes[i].get(), 3, _configStage);
			}
		}
	}
}


void ConfigurationTreeItemModel::setModified(bool m) {
	bool changed = _modified != m;
	_modified = m;
	if ( changed ) emit modificationChanged(_modified);
}


Environment::ConfigStage ConfigurationTreeItemModel::configStage() const {
	return _configStage;
}


bool ConfigurationTreeItemModel::setData(const QModelIndex &idx, const QVariant &v, int role) {
	int type = idx.sibling(idx.row(),NAME).data(ConfigurationTreeItemModel::Type).toInt();

	// Only parameters or none types are allowed to be changed for now
	if ( type != TypeParameter && type != TypeNone ) return false;

	if ( type == TypeParameter ) {
		Parameter *param = reinterpret_cast<Parameter*>(
			idx.sibling(idx.row(),NAME).data(ConfigurationTreeItemModel::Link).value<void*>()
		);

		switch ( idx.column() ) {
			case VALUE:
				param->symbols[_configStage]->symbol.content = v.toString().toStdString();
				// Reset URI to show that this value has not yet been written
				// to the configuration file
				param->symbols[_configStage]->symbol.uri = "";
				param->updateFinalValue();
				updateDerivedParameters(QModelIndex(), param, param->symbols[_configStage].get());
				setModified();
				return QStandardItemModel::setData(idx, paramValue(param, _configStage), role);
			case LOCKED:
				if ( v.toBool() )
					param->symbols[_configStage]->symbol.stage = Environment::CS_UNDEFINED;
				else {
					param->symbols[_configStage]->symbol.content = param->symbol.content;
					param->symbols[_configStage]->symbol.stage = _configStage;
				}

				param->updateFinalValue();
				updateDerivedParameters(QModelIndex(), param, param->symbols[_configStage].get());
				QStandardItemModel::setData(idx.sibling(idx.row(),VALUE), paramValue(param, _configStage));
				break;
			default:
				return false;
		}
	}
	else if ( type == TypeNone ) {
		if ( role == Type && v.toInt() == TypeStruct ) {
			// Load the struct tree here
			QStandardItem *structItem = itemFromIndex(idx);
			Structure *struc = reinterpret_cast<Structure*>(idx.data(Link).value<void*>());

			QStandardItem *modItem = structItem->parent();
			while ( modItem &&
			        modItem->data(ConfigurationTreeItemModel::Type).toInt() != ConfigurationTreeItemModel::TypeModule )
				modItem = modItem->parent();

			if ( modItem ) {
				Module *mod = (Module*)modItem->data(ConfigurationTreeItemModel::Link).value<void*>();
				_model->update(mod, struc);
			}

			loadStructure(structItem, struc, idx.data(Level).toInt(), _configStage);
		}
		else if ( role == Type && v.toInt() == TypeCategoryBinding ) {
			// Load the binding tree here
			QStandardItem *bindingItem = itemFromIndex(idx);
			Binding *binding = reinterpret_cast<Binding*>(idx.data(Link).value<void*>());
			BindingCategory *cat = reinterpret_cast<BindingCategory*>(binding->parent);
			Module *mod = (Module*)cat->parent->parent;
			mod->model->updateBinding((ModuleBinding*)cat->parent, binding);

			for ( size_t s = 0; s < binding->sections.size(); ++ s ) {
				Section *sec = binding->sections[s].get();

				for ( size_t g = 0; g < sec->groups.size(); ++g )
					addGroup(bindingItem, sec->groups[g].get(), idx.data(Level).toInt(), _configStage);

				for ( size_t p = 0; p < sec->parameters.size(); ++p )
					addParameter(bindingItem, sec->parameters[p].get(), idx.data(Level).toInt(), _configStage);

				for ( size_t i = 0; i < sec->structures.size(); ++i )
					addStructure(bindingItem, sec->structures[i].get(), idx.data(Level).toInt(), _configStage);

				for ( size_t i = 0; i < sec->structureTypes.size(); ++i )
					addStructure(bindingItem, sec->structureTypes[i].get(), idx.data(Level).toInt(), _configStage);
			}
		}
	}

	setModified();
	return QStandardItemModel::setData(idx, v, role);
}


Qt::ItemFlags ConfigurationTreeItemModel::flags(const QModelIndex &idx) const {
	int type = idx.sibling(idx.row(),NAME).data(ConfigurationTreeItemModel::Type).toInt();
	if ( type != ConfigurationTreeItemModel::TypeParameter)
		return Qt::ItemIsSelectable | Qt::ItemIsEnabled;

	Qt::ItemFlags flags = Qt::ItemIsSelectable;

	switch ( idx.column() ) {
		case NAME:
		case TYPE:
			flags |= Qt::ItemIsEnabled;
			break;
		case VALUE:
			flags |= Qt::ItemIsEditable;
			if ( !idx.sibling(idx.row(),LOCKED).data().toBool() ) flags |= Qt::ItemIsEnabled;
			break;
		case LOCKED:
			flags |= Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsUserCheckable;
			break;
		default:
			break;
	}

	return flags;
}


void ConfigurationTreeItemModel::updateDerivedParameters(
	const QModelIndex &idx,
	System::Parameter *super,
	System::SymbolMapItem *item
)
{
	if ( idx.isValid() ) {
		int type = idx.sibling(idx.row(),NAME).data(ConfigurationTreeItemModel::Type).toInt();

		if ( type == ConfigurationTreeItemModel::TypeParameter ) {
			Parameter *param = reinterpret_cast<Parameter*>(
				idx.sibling(idx.row(),NAME).data(ConfigurationTreeItemModel::Link).value<void*>()
			);

			if ( param != super ) {
				for ( int i = 0; i < Environment::CS_QUANTITY; ++i ) {
					if ( param->symbols[i] == item) {
						if ( i == _configStage ) {
							bool locked = item->symbol.stage == Environment::CS_UNDEFINED;
							if ( idx.sibling(idx.row(),LOCKED).data().toBool() != locked )
								QStandardItemModel::setData(idx.sibling(idx.row(),LOCKED), locked);
						}

						param->updateFinalValue();
						QStandardItemModel::setData(idx.sibling(idx.row(),VALUE), paramValue(param, _configStage));

						break;
					}
				}
			}

			/*
			if ( param->inherts(super) ) {
				param->updateFinalValue();
				QStandardItemModel::setData(idx.sibling(idx.row(),VALUE), paramValue(param));
			}
			*/
		}
	}

	int rows = rowCount(idx);
	for ( int i = 0; i < rows; ++i )
		updateDerivedParameters(index(i,NAME,idx), super, item);
}


void ModuleListDelegate::paint(QPainter *painter,
                               const QStyleOptionViewItem &option,
                               const QModelIndex &index) const {
	//static QIcon ico("app.png");

	int left = option.rect.left();
	int right = option.rect.right();
	int top = option.rect.top();
	int bottom = option.rect.bottom();

	if ( option.state & QStyle::State_Selected ) {
		if ( index.row() == 0 ) top -= 2;

		QLinearGradient grad(left,0,right,0);
		grad.setColorAt(0, selectBg);
		grad.setColorAt(1, selectLightBg);
		painter->fillRect(left,top,right-left,bottom-top, grad);

		painter->setPen(Qt::black);
		painter->drawLine(left, top+1, right, top+1);
		painter->drawLine(left, bottom-1, right, bottom-1);

		painter->setPen(borderBg);
		if ( index.row() > 0 )
			painter->drawLine(left, top, right, top);
		painter->drawLine(left, bottom, right, bottom);

		painter->setPen(borderLightBg);
		painter->drawLine(left, top+2, right, top+2);
		painter->setPen(Qt::white);
		painter->drawLine(right, top+2, right, bottom-2);

		painter->setPen(selectText);
	}
	else if ( option.state & QStyle::State_MouseOver ) {
		QColor c = hoverBg;
		QLinearGradient grad(left,0,right,0);
		c.setAlpha(0); grad.setColorAt(0, c);
		c.setAlpha(128); grad.setColorAt(0.5, c);
		c.setAlpha(0); grad.setColorAt(1, c);
		painter->fillRect(left,top,right-left,bottom-top, grad);

		c = borderBg;
		c.setAlpha(0); grad.setColorAt(0, c);
		c.setAlpha(192); grad.setColorAt(0.5, c);
		c.setAlpha(0); grad.setColorAt(1, c);

		painter->fillRect(left,top,right-left,1, grad);
		painter->fillRect(left,bottom,right-left,1, grad);

		painter->setPen(normalText);
	}
	else
		painter->setPen(normalText);

	//painter->drawPixmap((left+right)/2-16,top+6,ico.pixmap(32,32));
	QIcon ico = index.data(Qt::DecorationRole).value<QIcon>();
	if ( !ico.isNull() ) {
		QPixmap pm = ico.pixmap(QSize(32,32));
		painter->drawPixmap(option.rect.left()+(option.rect.width()-pm.width()-2)/2,option.rect.top()+4, pm);
	}

	QFont f = painter->font();
	f.setBold(true);
	painter->setFont(f);
	painter->drawText(option.rect.adjusted(0,0,0,-6),
	                  Qt::AlignHCenter | Qt::AlignBottom,
	                  index.data(Qt::DisplayRole).toString());
}


QSize ModuleListDelegate::sizeHint(const QStyleOptionViewItem & option,
                                   const QModelIndex & index) const {
	return QSize(64, 64);
}


bool ModuleListPainter::eventFilter(QObject *obj, QEvent *event) {
	if ( event->type() == QEvent::Paint ) {
		QWidget *w = static_cast<QWidget*>(obj);
		QPainter painter(w);

		QLinearGradient grad(0,0,w->width(),0);
		grad.setColorAt(0,darkBg);
		grad.setColorAt(1,lightBg);
		painter.fillRect(w->rect(),grad);
		painter.setPen(borderBg);
		painter.drawLine(w->rect().right(),0,w->rect().right(),w->rect().height());
		//painter.setPen(Qt::black);
		//painter.drawRect(w->rect().adjusted(0,0,-1,-1));

		return false;
	}

	return QObject::eventFilter(obj, event);
}


void ConfiguratorPanel::setHeadline(QString head) {
	_headline = head;
	emit headlineChanged(_headline);
}


void ConfiguratorPanel::setModel(ConfigurationTreeItemModel *model) {
	_model = model;
}


void ConfiguratorPanel::setDescription(QString desc) {
	_description = desc;
	emit descriptionChanged(_description);
}


Configurator::Configurator(Environment::ConfigStage stage, QWidget *parent)
: QMainWindow(parent), _settings("gempa", "Configurator") {
	setObjectName("Configurator");

	_configurationStage = stage;

	_proxy = NULL;

	menuBar()->setAutoFillBackground(true);
	QMenu *fileMenu = menuBar()->addMenu("&File");
	QMenu *editMenu = menuBar()->addMenu("&Edit");
	QMenu *editModeMenu = editMenu->addMenu("&Mode");

	QAction *systemMode = editModeMenu->addAction("&System");
	connect(systemMode, SIGNAL(triggered()), this, SLOT(switchToSystemMode()));

	QAction *userMode = editModeMenu->addAction("&User");
	connect(userMode, SIGNAL(triggered()), this, SLOT(switchToUserMode()));

	QAction *setupAction = fileMenu->addAction("&Wizard");
	setupAction->setShortcut(QKeySequence("Ctrl+N"));
	connect(setupAction, SIGNAL(triggered()), this, SLOT(wizard()));

	QAction *reloadAction = fileMenu->addAction("&Reload");
	reloadAction->setShortcut(QKeySequence("Ctrl+R"));
	connect(reloadAction, SIGNAL(triggered()), this, SLOT(reload()));

	QAction *saveAction = fileMenu->addAction("&Save");
	saveAction->setShortcut(QKeySequence("Ctrl+S"));
	connect(saveAction, SIGNAL(triggered()), this, SLOT(save()));

	QAction *quitAction = fileMenu->addAction("&Quit");
	quitAction->setShortcut(QKeySequence("Ctrl+Q"));
	connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

	QObject *modNamePainter = new ModuleNamePainter(this);
	QObject *modInfoPainter = new ModuleInfoPainter(this);

	QWidget *centralWidget = new QWidget(this);
	QGridLayout *centralLayout = new QGridLayout(centralWidget);

	centralLayout->setSpacing(1);
	centralLayout->setMargin(1);
	centralWidget->setLayout(centralLayout);
	setCentralWidget(centralWidget);

	_model = new ConfigurationTreeItemModel(centralWidget);

	QFont f = font();
	f.setPointSize(f.pointSize()*2);
	f.setBold(true);

	ClickFilter *clickFilter = new ClickFilter(this);
	connect(clickFilter, SIGNAL(clicked(QObject*)), this, SLOT(clicked(QObject*)));

	_modeLabel = new MouseTrackLabel;
	_modeLabel->setToolTip(tr("Click to change the configuration mode (system->user or user->system"));
	_modeLabel->installEventFilter(modNamePainter);
	_modeLabel->installEventFilter(clickFilter);
	centralLayout->addWidget(_modeLabel,0,0);

	QWidget *infoWidget = new QWidget;
	_headline = new QLabel;

	_headline->setFont(f);
	_headline->installEventFilter(modNamePainter);
	_headline->setMargin(fontMetrics().ascent());

	_description = new QLabel;
	_description->installEventFilter(modInfoPainter);
	_description->setMargin(fontMetrics().ascent());
	_description->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred));
	//_description->setWordWrap(true);

	QVBoxLayout *vl = new QVBoxLayout;
	vl->addWidget(_headline);
	vl->addWidget(_description);
	vl->setMargin(0);
	vl->setSpacing(0);
	infoWidget->setLayout(vl);
	infoWidget->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));
	centralLayout->addWidget(infoWidget,0,1);

	_listWidget = new QListWidget;

	_listWidget->setFixedWidth(100);
	_listWidget->setFrameShape(QFrame::NoFrame);
	_listWidget->viewport()->installEventFilter(new ModuleListPainter(this));

	_listWidget->setItemDelegate(new ModuleListDelegate(this));
	_listWidget->setSizePolicy(QSizePolicy(QSizePolicy::Maximum, QSizePolicy::Preferred));

	centralLayout->addWidget(_listWidget,1,0);

	QList<ConfiguratorPanel*> panels;
	panels.append(new InformationPanel);
	panels.append(new SystemPanel);
	panels.append(new InventoryPanel);
	panels.append(new ModulesPanel);
	panels.append(new BindingsPanel);

	_statusLabel = new StatusLabel;
	_statusLabel->hide();
	QVBoxLayout *vlayout = new QVBoxLayout;
	vlayout->setSpacing(0);
	vlayout->addWidget(_statusLabel);

	foreach ( ConfiguratorPanel *panel, panels) {
		Panel p;
		p.second = panel;
		p.first = new QListWidgetItem(panel->title(), _listWidget);
		vlayout->addWidget(panel);
		panel->hide();
		p.first->setData(Qt::DecorationRole, panel->icon());
		p.first->setData(Qt::UserRole, qVariantFromValue((void*)panel));
		_panels.append(p);

		connect(panel, SIGNAL(reloadRequested()), this, SLOT(reload()));
	}

	centralLayout->addLayout(vlayout,1,1);

	connect(_listWidget, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
	        this, SLOT(sectionChanged(QListWidgetItem*, QListWidgetItem*)));

	_listWidget->setCurrentRow(0);

	QAction *help = QWhatsThis::createAction(this);
	help->setShortcut(QKeySequence("F1"));
	addAction(help);

	_firstShow = true;

	connect(&_statusTimer, SIGNAL(timeout()), this, SLOT(statusTimer()));
}


Configurator::~Configurator() {
	_settings.beginGroup(objectName());
	_settings.setValue("geometry", saveGeometry());
	_settings.setValue("state", saveState());
	_settings.endGroup();
}


void Configurator::updateModeLabel() {
	switch ( _configurationStage ) {
		case Environment::CS_USER_APP:
			setWindowTitle(tr("SeisComP3 - user configuration @ %1").arg(Core::getHostname().c_str()));
			static_cast<MouseTrackLabel*>(_modeLabel)->setIcon(QIcon(":/res/icons/user-settings.png"), QSize(72,72));
			break;
		case Environment::CS_CONFIG_APP:
			setWindowTitle(tr("SeisComP3 - system configuration @ %1").arg(Core::getHostname().c_str()));
			static_cast<MouseTrackLabel*>(_modeLabel)->setIcon(QIcon(":/res/icons/system-settings.png"), QSize(72,72));
			break;
		default:
			break;
	}
}


bool Configurator::setModel(System::Model *model) {
	QtConfigDelegate cd(&_settings);
	model->readConfig(Environment::CS_USER_APP, &cd);

	cd.showConflicts();

	if ( _configurationStage == Environment::CS_UNDEFINED ) {
		StageSelectionDialog dlg(this);
		if ( dlg.exec() == QDialog::Accepted ) {
			switch ( dlg.mode() ) {
				case StageSelectionDialog::User:
					_configurationStage = Environment::CS_USER_APP;
					break;
				case StageSelectionDialog::System:
					_configurationStage = Environment::CS_CONFIG_APP;
					break;
			}
		}
		else
			return false;
	}

	if ( cd.hasErrors )
		showWarningMessage("Configuration loaded with errors");

	updateModeLabel();

	_model->setModel(model, _configurationStage);

	foreach ( Panel p, _panels )
		p.second->setModel(_model);

	//_treeView->hideColumn(1);
	return true;
}


void Configurator::showEvent(QShowEvent *event) {
	if ( _firstShow ) {
		_settings.beginGroup(objectName());
		restoreGeometry(_settings.value("geometry").toByteArray());
		restoreState(_settings.value("state").toByteArray());
		_settings.endGroup();

		_firstShow = false;

		// Check lock file
		Environment *env = Environment::Instance();

		QString statDir = QDir::toNativeSeparators(
			(env->installDir() + "/var/run").c_str());

		QString statFile = QDir::toNativeSeparators(
			statDir + "/seiscomp.init");

		if ( QFile::exists(statFile) ) return;

		// Create state file
		QDir dir(statDir);
		dir.mkpath(".");
		QFile f(statFile);
		f.open(QIODevice::WriteOnly | QIODevice::Truncate);
		f.close();

		if ( QMessageBox::question(this, tr("First start"),
		                           tr("This seems to be the first start of the SC3 configurator.\n"
		                              "Do you want to run the initial setup?\n"
		                              "Hint: You can say no here and start the wizard at any "
		                              "time with Ctrl+N."),
		                           QMessageBox::Yes | QMessageBox::No) == QMessageBox::No )
			return;

		wizard();
	}
}


void Configurator::closeEvent(QCloseEvent *event) {
	if ( _model->isModified() ) {
		if ( QMessageBox::question(
		       this, "Configuration changed",
		       "The configuration is modified. Do you want to save it?",
		       QMessageBox::Yes, QMessageBox::No
		     ) == QMessageBox::Yes )
			save();
	}

	QMainWindow::closeEvent(event);
}


void Configurator::paintEvent(QPaintEvent *event) {
	QPainter p(this);
	p.fillRect(rect(), Qt::black);
}


void Configurator::panelHeadlineChanged(const QString &text) {
	_headline->setText(text);
}


void Configurator::panelDescriptionChanged(const QString &text) {
	_description->setText(firstLine(text));
	_description->setToolTip(multiline(text, 80));
}


void Configurator::switchToSystemMode() {
	if ( _configurationStage == Environment::CS_CONFIG_APP ) return;
	_configurationStage = Environment::CS_CONFIG_APP;
	_model->setModel(_model->model(), _configurationStage);
	foreach ( Panel p, _panels )
		p.second->setModel(_model);
	updateModeLabel();
}


void Configurator::switchToUserMode() {
	if ( _configurationStage == Environment::CS_USER_APP ) return;
	_configurationStage = Environment::CS_USER_APP;
	_model->setModel(_model->model(), _configurationStage);
	foreach ( Panel p, _panels )
		p.second->setModel(_model);
	updateModeLabel();
}


void Configurator::showStatusMessage(const QString &msg) {
	_statusTimer.stop();
	_statusLabel->setSuccessText(msg);
	_statusLabel->show();
	_statusTimer.setSingleShot(true);
	_statusTimer.start(3000);
}


void Configurator::showWarningMessage(const QString &msg) {
	_statusTimer.stop();
	_statusLabel->setWarningText(msg);
	_statusLabel->show();
	_statusTimer.setSingleShot(true);
	_statusTimer.start(3000);
}


void Configurator::clicked(QObject *o) {
	if ( _modeLabel == o ) {
		if ( _configurationStage == Environment::CS_CONFIG_APP )
			switchToUserMode();
		else
			switchToSystemMode();
	}
}


void Configurator::statusTimer() {
	if ( _statusTimer.isSingleShot() ) {
		_statusLabel->setInfoText("");
		_statusLabel->hide();
	}
}


void Configurator::wizard() {
	WizardModel wizard;

	SchemaDefinitions *schema = _model->model()->schema;
	for ( size_t i = 0; i < schema->moduleCount(); ++i ) {
		SchemaModule *mod = schema->module(i);
		SchemaSetup *setup = mod->setup.get();
		if ( setup != NULL ) {
			for ( size_t g = 0; g < setup->groups.size(); ++g )
				wizard[mod->name].push_back(setup->groups[g].get());
		}

		SchemaDefinitions::PluginList plugins;
		plugins = schema->pluginsForModule(mod->name);

		for ( size_t p = 0; p < plugins.size(); ++p ) {
			SchemaPlugin *plugin = plugins[p];
			setup = plugin->setup.get();
			if ( setup != NULL ) {
				for ( size_t g = 0; g < setup->groups.size(); ++g )
					wizard[mod->name].push_back(setup->groups[g].get());
			}
		}
	}

	if ( wizard.empty() ) {
		return;
	}

	WizardWidget w(&wizard);
	w.exec();

	if ( w.ranSetup() ) {
		QtConfigDelegate cd(&_settings);
		_model->model()->readConfig(Environment::CS_USER_APP, &cd);
		_model->setModel(_model->model(), _configurationStage);
	}
}


void Configurator::reload() {
	bool errors = false;

	if ( _model->model() ) {
		QtConfigDelegate cd(&_settings);
		_model->model()->readConfig(Environment::CS_USER_APP, &cd);
		_model->setModel(_model->model(), _configurationStage);
		_model->setModified(false);
		errors = cd.hasErrors;
		cd.showConflicts();
	}

	foreach ( Panel p, _panels )
		p.second->setModel(_model);

	if ( errors )
		showWarningMessage("Configuration reloaded with errors");
	else
		showStatusMessage("Configuration reloaded");
}


void Configurator::save() {
	_model->model()->writeConfig(_configurationStage);
	_model->setModified(false);
	showStatusMessage("Configuration saved");
}


void Configurator::sectionChanged(QListWidgetItem *curr, QListWidgetItem *prev) {
	ConfiguratorPanel *pw = prev?(ConfiguratorPanel*)prev->data(Qt::UserRole).value<void*>():NULL;
	ConfiguratorPanel *cw = curr?(ConfiguratorPanel*)curr->data(Qt::UserRole).value<void*>():NULL;

	// Do we switch to a panel that depends on the configuration on disk?
	// We should then ask the user to flush internal modifications.
	if ( cw && cw->isExternalConfigurationUsed() && _model->isModified() ) {
		if ( QMessageBox::question(
		       this, "Configuration changed",
		       "The configuration has changed and the current panel depends "
		       "on it. You must save the configuration to ensure the correct "
		       "behaviour.\nSave?",
		       QMessageBox::Yes, QMessageBox::No
		     ) == QMessageBox::Yes )
			save();
	}

	if ( pw ) {
		pw->hide();
		pw->disconnect(this);
	}

	if ( cw ) {
		cw->show();
		connect(cw, SIGNAL(headlineChanged(QString)),
		        this, SLOT(panelHeadlineChanged(const QString &)));
		connect(cw, SIGNAL(descriptionChanged(QString)),
		        this, SLOT(panelDescriptionChanged(const QString &)));
		connect(cw, SIGNAL(reloadRequested()), this, SLOT(reload()));
		cw->activated();
	}

	if ( curr ) {
		panelHeadlineChanged(cw->headline());
		panelDescriptionChanged(cw->description());
	}
	else {
		_headline->setText("");
		_description->setText("");
		_description->setToolTip("");
	}
}
