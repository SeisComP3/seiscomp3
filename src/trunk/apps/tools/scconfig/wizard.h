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


#ifndef __SEISCOMP_CONFIGURATION_GUI_WIZARD_H__
#define __SEISCOMP_CONFIGURATION_GUI_WIZARD_H__


#ifndef Q_MOC_RUN
#include <seiscomp3/system/schema.h>
#endif

#include <QtGui>
#include <string>
#include <list>
#include <map>


typedef std::list<Seiscomp::System::SchemaSetupGroup*> SetupGroups;
typedef std::map<std::string, SetupGroups> WizardModel;


class WizardPage : public QWidget {
	public:
		WizardPage(QWidget *parent = 0);

		void setTitle(const QString &title);
		void setSubTitle(const QString &subtitle);

		const QString &title() const;
		const QString &subtitle() const;


	private:
		QString _title;
		QString _subtitle;
};



class WizardWidget : public QDialog {
	Q_OBJECT

	public:
		WizardWidget(WizardModel *model, QWidget *parent = NULL);
		~WizardWidget();

		void reject();

		//! Returns whether a process has started or not
		//! to trigger reload of configuration files downstream.
		bool ranSetup() const;


	private slots:
		void back();
		void next();
		void finish();

		void textChanged(const QString &text);
		void checkStateChanged(bool);
		void radioStateChanged(bool);

		void readProcStdOut();
		void readProcStdErr();
		void finishedProc(int, QProcess::ExitStatus);

	private:
		void setPage(WizardPage *p);

		WizardPage *createIntroPage();
		WizardPage *createExtroPage();
		WizardPage *createOutputPage();
		WizardPage *createCurrentPage();

	private:
		typedef Seiscomp::System::SchemaSetupInput Input;
		typedef Seiscomp::System::SchemaSetupGroup Group;

		typedef WizardModel::iterator ModelIterator;
		typedef std::vector<Seiscomp::System::SchemaSetupInputPtr> Inputs;

		struct Node {
			Node(Node *p, Node *left, Node *right, Input *i);

			~Node();

			// Parent node
			Node         *parent;
			// Left sibling
			Node         *prev;
			// Right sibling
			Node         *next;
			// First child node
			Node         *child;

			// Selected child if it is a choice
			Node         *activeChild;

			QString       modname;
			Group        *group;
			Input        *input;
			QString       value;
			QString       path;
			bool          lastInGroup;
		};

		void addGroups(Node *, const QString &modname, const SetupGroups &);
		void addInputs(Node *, bool isOption, const QString &modname, Group *g,
		               const Inputs &, const QString &path);

		WizardModel   *_model;
		Node          *_modelTree;
		QStack<Node*>  _path;

		QLabel        *_titleLabel;
		QLabel        *_subtitleLabel;
		QFrame        *_headerBreak;
		QWidget       *_header;
		QWidget       *_content;
		QLayout       *_contentLayout;
		QPushButton   *_buttonBack;
		QPushButton   *_buttonNext;
		QPushButton   *_buttonCancel;

		Node          *_currentNode;
		WizardPage    *_currentPage;
		QWidget       *_currentInput;

		QProcess      *_procSeisComP;
		QTextEdit     *_logPanel;
		QLabel        *_procStatus;

		bool           _ranSetup;

	friend QByteArray &operator<<(QByteArray&, Node&);
	friend QByteArray &operator<<(QByteArray&, Node*);
};


#endif
