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


#ifndef __SEISCOMP_CONFIGURATION_GUI_H__
#define __SEISCOMP_CONFIGURATION_GUI_H__


#ifndef Q_MOC_RUN
#include <seiscomp3/system/model.h>
#endif

#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QItemDelegate>
#include <QMainWindow>
#include <QListWidget>
#include <QTreeView>
#include <QLabel>
#include <QTimer>
#include <QSettings>


class QTreeWidget;
class QTreeWidgetItem;


class ConfigurationTreeItemModel : public QStandardItemModel {
	Q_OBJECT

	public:
		enum ItemType {
			TypeNone = 0,
			TypeBindings,
			TypeModule,
			TypeNetwork,
			TypeStation,
			TypeBinding,
			TypeCategory,
			TypeCategoryBinding,
			TypeSection,
			TypeGroup,
			TypeStruct,
			TypeParameter
		};

		enum UserData {
			Level = Qt::UserRole + 1,
			Link  = Qt::UserRole + 2,
			Type  = Qt::UserRole + 3
		};


	public:
		ConfigurationTreeItemModel(QObject *parent,
		                           Seiscomp::System::Model * = NULL,
		                           Seiscomp::Environment::ConfigStage = Seiscomp::Environment::CS_CONFIG_APP);

	public:
		void setModel(Seiscomp::System::Model *, Seiscomp::Environment::ConfigStage);
		Seiscomp::System::Model *model() const { return _model; }

		void setModel(Seiscomp::System::ModuleBinding *);

		//! Returns whether the model is modified or not
		bool isModified() const { return _modified; }


	public:
		Seiscomp::Environment::ConfigStage configStage() const;

		bool setData(const QModelIndex &, const QVariant &, int role = Qt::EditRole);
		Qt::ItemFlags flags(const QModelIndex &index) const;


	public slots:
		//! Sets the modified state
		void setModified(bool m = true);


	signals:
		void modificationChanged(bool changed);

	private:
		void updateDerivedParameters(
			const QModelIndex &, Seiscomp::System::Parameter*,
			Seiscomp::System::SymbolMapItem *);

	private:
		Seiscomp::Environment::ConfigStage _configStage;
		Seiscomp::System::Model           *_model;
		bool                               _modified;
};


class ConfiguratorPanel : public QWidget {
	Q_OBJECT

	public:
		ConfiguratorPanel(bool usesExternalConfiguration, QWidget *parent)
		: QWidget(parent), _model(NULL),
		  _usesExternalConfiguration(usesExternalConfiguration) {}

	public:
		virtual void setModel(ConfigurationTreeItemModel *model);

		//! Event handler that can be reimplemented in derived classes. It
		//! is called whenever a panel is activated and shown.
		virtual void activated() {}

		bool isExternalConfigurationUsed() const { return _usesExternalConfiguration; }

		QString title() const { return _name; }

		void setHeadline(QString head);
		QString headline() const { return _headline; }

		void setDescription(QString desc);
		QString description() const { return _description; }

		QIcon icon() const { return _icon; }


	signals:
		void reloadRequested();
		void headlineChanged(QString head);
		void descriptionChanged(QString desc);


	protected:
		ConfigurationTreeItemModel *_model;
		QString                     _name;
		QString                     _headline;
		QString                     _description;
		QIcon                       _icon;
		bool                        _usesExternalConfiguration;
};


class StatusLabel : public QLabel {
	public:
		StatusLabel(QWidget *parent = NULL);

		void setInfoText(const QString &);
		void setSuccessText(const QString &);
		void setWarningText(const QString &);
		void setErrorText(const QString &);

	protected:
		void paintEvent(QPaintEvent *);

	private:
		QPixmap _icon;
};


class ClickFilter : public QObject {
	Q_OBJECT

	public:
		ClickFilter(QObject *parent = NULL);

	signals:
		void clicked(QObject *);

	protected:
		bool eventFilter(QObject *obj, QEvent *event);
};


class Configurator : public QMainWindow {
	Q_OBJECT

	public:
		Configurator(Seiscomp::Environment::ConfigStage stage,
		             QWidget *parent = NULL);
		~Configurator();

		bool setModel(Seiscomp::System::Model *model);


	protected:
		void showEvent(QShowEvent *event);
		void closeEvent(QCloseEvent *event);
		void paintEvent(QPaintEvent *event);

	private:
		void updateModeLabel();

	private slots:
		void wizard();
		void reload();
		void save();
		void sectionChanged(QListWidgetItem*, QListWidgetItem*);
		void panelHeadlineChanged(const QString &);
		void panelDescriptionChanged(const QString &);

		void switchToSystemMode();
		void switchToUserMode();

		void clicked(QObject *);
		void showStatusMessage(const QString &msg);
		void showWarningMessage(const QString &msg);

		void statusTimer();


	private:
		QSettings                   _settings;

		typedef QPair<QListWidgetItem*,ConfiguratorPanel*> Panel;
		ConfigurationTreeItemModel *_model;
		QSortFilterProxyModel      *_proxy;

		QList<Panel>                _panels;

		QLabel                     *_modeLabel;
		QLabel                     *_headline;
		QLabel                     *_description;
		QListWidget                *_listWidget;
		StatusLabel                *_statusLabel;

		QListWidgetItem            *_infoItem;
		QListWidgetItem            *_configItem;
		QListWidgetItem            *_inventoryItem;
		QListWidgetItem            *_bindingsItem;

		QWidget                    *_issueLog;

		bool                        _firstShow;

		QTimer                      _statusTimer;

		Seiscomp::Environment::ConfigStage _configurationStage;
};



#endif
