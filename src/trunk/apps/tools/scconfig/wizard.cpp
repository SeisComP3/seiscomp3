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

#include "wizard.h"
#include <seiscomp3/config/config.h>
#include <seiscomp3/system/environment.h>
#include <iostream>
#include <sstream>


using namespace std;


WizardWidget::Node::Node(Node *p, Node *left, Node *right, Input *i)
: parent(p), prev(left), next(right), child(NULL),
  activeChild(NULL), input(i), lastInGroup(false) {}


WizardWidget::Node::~Node() {
	while ( child ) {
		Node *n = child;
		child = child->next;
		delete n;
	}
}


WizardPage::WizardPage(QWidget *parent) : QWidget(parent) {}


void WizardPage::setTitle(const QString &title) {
	_title = title;
}


void WizardPage::setSubTitle(const QString &subtitle) {
	_subtitle = subtitle;
}


const QString &WizardPage::title() const {
	return _title;
}


const QString &WizardPage::subtitle() const {
	return _subtitle;
}


WizardWidget::WizardWidget(WizardModel *model, QWidget *parent)
: QDialog(parent), _model(model) {
	resize(500,400);
	setWindowTitle(tr("SeisComP3: Setup wizard"));

	_currentInput = NULL;

	_ranSetup = false;
	_procSeisComP = NULL;
	_currentPage = NULL;

	_buttonBack = new QPushButton(tr("Back"));
	_buttonNext = new QPushButton(tr("Next"));
	_buttonCancel = new QPushButton(tr("Cancel"));

	QHBoxLayout *buttonLayout = new QHBoxLayout;
	buttonLayout->setMargin(6);
	buttonLayout->addStretch();
	buttonLayout->addWidget(_buttonBack);
	buttonLayout->addWidget(_buttonNext);
	buttonLayout->addWidget(_buttonCancel);

	_header = new QWidget;
	_header->setBackgroundRole(QPalette::Base);
	_header->setAutoFillBackground(true);

	_titleLabel = new QLabel;
	_subtitleLabel = new QLabel;

	QVBoxLayout *headerLayout = new QVBoxLayout;
	headerLayout->addWidget(_titleLabel);
	headerLayout->addWidget(_subtitleLabel);
	headerLayout->setMargin(fontMetrics().ascent());
	_header->setLayout(headerLayout);
	_header->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum));

	_content = new QWidget;
	_contentLayout = new QVBoxLayout;
	_contentLayout->setMargin(fontMetrics().ascent());
	_content->setLayout(_contentLayout);

	_headerBreak = new QFrame;
	_headerBreak->setFrameShape(QFrame::HLine);

	QFrame *hrule = new QFrame;
	hrule->setFrameShape(QFrame::HLine);

	QVBoxLayout *pageLayout = new QVBoxLayout;
	pageLayout->setSpacing(0);
	pageLayout->setMargin(0);
	pageLayout->addWidget(_header);
	pageLayout->addWidget(_headerBreak);
	pageLayout->addWidget(_content);
	pageLayout->addWidget(hrule);
	pageLayout->addLayout(buttonLayout);

	setLayout(pageLayout);

	setTabOrder(_buttonNext, _buttonCancel);
	setTabOrder(_buttonCancel, _buttonBack);

	connect(_buttonBack, SIGNAL(clicked()), this, SLOT(back()));
	connect(_buttonNext, SIGNAL(clicked()), this, SLOT(next()));
	connect(_buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));

	// Build model tree
	_modelTree = new Node(NULL, NULL, NULL, NULL);

	for ( ModelIterator it = _model->begin(); it != _model->end(); ++it ) {
		QString modname = it->first.c_str();
		addGroups(_modelTree, modname, it->second);
	}

	_modelTree->activeChild = _modelTree->child;

	_currentNode = _modelTree;
	setPage(createIntroPage());
}


WizardWidget::~WizardWidget() {
	if ( _modelTree ) delete _modelTree;
}


bool WizardWidget::ranSetup() const {
	return _ranSetup;
}


void WizardWidget::reject() {
	if ( _procSeisComP != NULL ) {
		if ( QMessageBox::question(this, tr("Abort setup"),
		                           tr("Setup is still running. Do you really\n"
		                              "want to abort the process? This is not\n"
		                              "recommended and can lead to undefined results."),
		                           QMessageBox::Yes | QMessageBox::No) == QMessageBox::No )
			return;
	}

	QDialog::reject();
}


void WizardWidget::addGroups(Node *parent, const QString &modname,
                             const SetupGroups &groups) {
	SetupGroups::const_iterator it;

	for ( it = groups.begin(); it != groups.end(); ++it )
		addInputs(parent, false, modname, *it, (*it)->inputs, ((*it)->name + ".").c_str());
}


void WizardWidget::addInputs(Node *parent, bool isOption, const QString &modname,
                             Group *g, const Inputs &inputs,
                             const QString &path) {
	Inputs::const_iterator it;
	Node *last = parent->child;

	// find the last child and add the current list to it
	while ( last ) {
		if ( last->next == NULL ) break;
		last = last->next;
	}

	for ( it = inputs.begin(); it != inputs.end(); ++it ) {
		Node *n = new Node(parent, last, NULL, it->get());

		n->path = path + (*it)->name.c_str();
		n->value = (*it)->defaultValue.c_str();
		n->modname = modname;
		n->group = g;

		if ( last != NULL ) last->next = n;
		last = n;

		if ( parent->child == NULL ) parent->child = last;

		std::vector<Seiscomp::System::SchemaSetupInputOptionPtr>::iterator oit;
		for ( oit = (*it)->options.begin(); oit != (*it)->options.end(); ++oit )
			addInputs(n, true, modname, g, (*oit)->inputs, n->path + ".");
	}

	if ( isOption && last != NULL ) last->lastInGroup = true;
}


void WizardWidget::back() {
	_buttonCancel->setText(tr("Cancel"));
	_buttonNext->setText(tr("Next"));
	_buttonNext->setEnabled(true);

	_currentNode = _path.pop();
	if ( _currentNode == _modelTree )
		setPage(createIntroPage());
	else
		setPage(createCurrentPage());
}


void WizardWidget::next() {
	if ( _buttonNext->text() == tr("Finish") ) {
		finish();
		return;
	}

	if ( _currentNode == _modelTree ) {
		_path.push(_currentNode);
		_currentNode = _currentNode->child;
		setPage(createCurrentPage());
		return;
	}

	Input *input = _currentNode->input;
	bool step = true;

	_currentNode->activeChild = NULL;

	// Choice input?
	if ( !input->options.empty() ) {
		for ( size_t i = 0; i < input->options.size(); ++i ) {
			if ( input->options[i]->value == _currentNode->value.toStdString() ) {
				if ( !input->options[i]->inputs.empty() ) {
					// Found new path to descend
					Node *child = _currentNode->child;

					// Search corresponding child in the three
					while ( child ) {
						if ( child->input == input->options[i]->inputs[0].get() ) {
							_path.push(_currentNode);
							_currentNode->activeChild = child;
							_currentNode = _currentNode->activeChild;
							step = false;
							break;
						}
						child = child->next;
					}

				}

				break;
			}
		}
	}

	if ( step ) {
		bool pushed = false;
		if ( _currentNode->next == NULL ) {
			Node *parent = _currentNode->parent;

			while ( parent ) {
				if ( parent->next ) {
					_path.push(_currentNode);
					pushed = true;
					_currentNode = parent;
					break;
				}

				parent = parent->parent;
			}

			if ( parent == NULL ) {
				_path.push(_currentNode);
				setPage(createExtroPage());
				_buttonNext->setText(tr("Finish"));
				return;
			}
		}

		if ( !pushed ) _path.push(_currentNode);
		_currentNode = _currentNode->next;
	}

	setPage(createCurrentPage());
}


QByteArray &operator<<(QByteArray &ar, WizardWidget::Node *n) {
	if ( n->input ) {
		ar.append(n->modname);
		ar.append(".");
		ar.append(n->path);
		ar.append(" = ");

		stringstream ss;
		Seiscomp::Config::Symbol sym;
		sym.values.push_back(n->value.toStdString());
		Seiscomp::Config::Config::writeValues(ss, &sym);

		ar.append(ss.str().c_str());
		ar.append("\n");
	}

	if ( n->activeChild != NULL )
		ar << n->activeChild;

	if ( !n->lastInGroup && n->next != NULL ) ar << n->next;
	return ar;
}


QByteArray &operator<<(QByteArray &ar, WizardWidget::Node &n) {
	return ar << &n;
}


void WizardWidget::finish() {
	setPage(createOutputPage());

	Seiscomp::Environment *env = Seiscomp::Environment::Instance();
	_procSeisComP = new QProcess(this);
	_procSeisComP->start(QString("%1/bin/seiscomp --stdin setup")
	                     .arg(env->installDir().c_str()),
	                     QProcess::Unbuffered | QProcess::ReadWrite);
	if ( !_procSeisComP->waitForStarted() ) {
		cerr << "Failed to start 'seiscomp'" << endl;
		delete _procSeisComP;
		_procSeisComP = NULL;
		return;
	}

	_ranSetup = true;

	_buttonBack->setEnabled(false);
	_buttonNext->setEnabled(false);

	QByteArray data;
	data << _modelTree;

	_procSeisComP->write(data);
	_procSeisComP->closeWriteChannel();
	_procSeisComP->setReadChannel(QProcess::StandardOutput);

	connect(_procSeisComP, SIGNAL(readyReadStandardOutput()),
	        this, SLOT(readProcStdOut()));
	connect(_procSeisComP, SIGNAL(readyReadStandardError()),
	        this, SLOT(readProcStdErr()));
	connect(_procSeisComP, SIGNAL(finished(int, QProcess::ExitStatus)),
	        this, SLOT(finishedProc(int, QProcess::ExitStatus)));
}


void WizardWidget::setPage(WizardPage *p) {
	if ( _currentPage ) delete _currentPage;

	_currentPage = p;

	_buttonBack->setEnabled(!_path.isEmpty());

	_titleLabel->setText(_currentPage->title());
	_subtitleLabel->setText(_currentPage->subtitle());

	QFont f = _titleLabel->font();
	f.setBold(true);

	if ( _currentPage->subtitle().isEmpty() ) {
		f.setPointSize(font().pointSize()*150/100);
		_headerBreak->setVisible(false);
		_subtitleLabel->setVisible(false);
		_content->setBackgroundRole(QPalette::Base);
		_content->setAutoFillBackground(true);
	}
	else {
		_headerBreak->setVisible(true);
		_subtitleLabel->setVisible(true);
		_content->setBackgroundRole(QPalette::NoRole);
		_content->setAutoFillBackground(false);
	}

	_titleLabel->setFont(f);

	_contentLayout->addWidget(_currentPage);

	if ( _currentInput ) _currentInput->setFocus();
	_currentInput = NULL;
}


WizardPage *WizardWidget::createIntroPage() {
	WizardPage *w = new WizardPage;
	w->setTitle(tr("Introduction"));

	QVBoxLayout *l = new QVBoxLayout;
	l->setMargin(0);
	w->setLayout(l);

	QLabel *text = new QLabel;
	text->setWordWrap(true);
	text->setText(tr("This wizard will guide you through the steps "
	                 "of the initial setup. Use the back and next "
	                 "buttons to navigate through the pages and "
	                 "press cancel to close this wizard without "
	                 "implications to your configuration."));

	l->addWidget(text);
	l->addStretch();

	return w;
}


WizardPage *WizardWidget::createExtroPage() {
	WizardPage *w = new WizardPage;
	w->setTitle(tr("Finished"));

	QVBoxLayout *l = new QVBoxLayout;
	l->setMargin(0);
	w->setLayout(l);

	QLabel *text = new QLabel;
	text->setWordWrap(true);
	text->setText(tr("All setup questions have been answered. "
	                 "You can now go back again to correct settings "
	                 "or press 'Finish' to create the configuration."));

	l->addWidget(text);
	l->addStretch();

	return w;
}


WizardPage *WizardWidget::createOutputPage() {
	WizardPage *w = new WizardPage;
	w->setTitle(tr("Setup system"));
	w->setSubTitle(tr("Running seiscomp setup"));

	QVBoxLayout *l = new QVBoxLayout;
	l->setMargin(0);
	w->setLayout(l);

	_procStatus = new QLabel;
	l->addWidget(_procStatus);
	_procStatus->setText(tr("Waiting ..."));

	_logPanel = new QTextEdit;
	_logPanel->setReadOnly(true);
	_logPanel->setLineWrapMode(QTextEdit::NoWrap);
	l->addWidget(_logPanel);

	return w;
}


WizardPage *WizardWidget::createCurrentPage() {
	WizardPage *w = new WizardPage;
	w->setTitle(_currentNode->modname);
	w->setSubTitle(_currentNode->group->name.c_str());

	QVBoxLayout *l = new QVBoxLayout;
	l->setMargin(0);
	w->setLayout(l);

	_currentInput = NULL;

	if ( _currentNode->input->type == "boolean" ) {
		QCheckBox *checkBox = new QCheckBox();
		checkBox->setText(_currentNode->input->text.c_str());
		checkBox->setChecked(_currentNode->value == "true");
		l->addWidget(checkBox);
		_currentInput = checkBox;

		connect(checkBox, SIGNAL(toggled(bool)), this, SLOT(checkStateChanged(bool)));
	}
	else if ( _currentNode->input->options.empty() ) {
		QLabel *text = new QLabel;
		text->setText(_currentNode->input->text.c_str());
		l->addWidget(text);
		QLineEdit *edit = new QLineEdit;
		edit->setText(_currentNode->value);
		if ( _currentNode->input->echo == "password" )
			edit->setEchoMode(QLineEdit::Password);
		else if ( _currentNode->input->echo == "none" )
			edit->setEchoMode(QLineEdit::NoEcho);
		l->addWidget(edit);
		_currentInput = edit;

		connect(edit, SIGNAL(textChanged(const QString &)),
		        this, SLOT(textChanged(const QString &)));
	}
	else {
		QLabel *text = new QLabel;
		text->setText(_currentNode->input->text.c_str());
		l->addWidget(text);

		for ( size_t i = 0; i < _currentNode->input->options.size(); ++i ) {
			QRadioButton *radio = new QRadioButton;
			radio->setText(_currentNode->input->options[i]->value.c_str());
			radio->setToolTip(_currentNode->input->options[i]->description.c_str());
			if ( _currentNode->value == radio->text() )
				radio->setChecked(true);
			l->addWidget(radio);

			if ( i == 0 ) _currentInput = radio;

			connect(radio, SIGNAL(toggled(bool)), this, SLOT(radioStateChanged(bool)));
		}
	}

	l->addStretch();

	if ( !_currentNode->input->description.empty() ) {
		l->addSpacing(12);
		QLabel *desc = new QLabel;
		desc->setWordWrap(true);
		desc->setText(_currentNode->input->description.c_str());
		l->addWidget(desc);
	}

	/*
	QLabel *path = new QLabel;
	path->setWordWrap(true);
	path->setText(_currentNode->path);
	l->addWidget(path);
	QFont f = path->font();
	f.setItalic(true);
	path->setFont(f);
	*/

	return w;
}


void WizardWidget::textChanged(const QString &text) {
	_currentNode->value = text;
}


void WizardWidget::checkStateChanged(bool e) {
	_currentNode->value = e?"true":"false";
}


void WizardWidget::radioStateChanged(bool e) {
	if ( e )
		_currentNode->value = static_cast<QRadioButton*>(sender())->text();
}


void WizardWidget::readProcStdOut() {
	QString text = _procSeisComP->readAllStandardOutput();
	_logPanel->setTextColor(_logPanel->palette().color(QPalette::Text));
	_logPanel->insertPlainText(text);
}


void WizardWidget::readProcStdErr() {
	QString text = _procSeisComP->readAllStandardError();
	_logPanel->setTextColor(_logPanel->palette().color(QPalette::Disabled, QPalette::Text));
	_logPanel->insertPlainText(text);
}


void WizardWidget::finishedProc(int res, QProcess::ExitStatus stat) {
	delete _procSeisComP;
	_procSeisComP = NULL;

	if ( res != 0 ) {
		QPalette pal = _procStatus->palette();
		pal.setColor(QPalette::WindowText, Qt::red);
		_procStatus->setPalette(pal);
		QFont f = _procStatus->font();
		f.setBold(true);
		_procStatus->setFont(f);
		_procStatus->setText(tr("Setup finished with exit code %1.").arg(res));
	}
	else {
		QPalette pal = _procStatus->palette();
		pal.setColor(QPalette::WindowText, Qt::darkGreen);
		_procStatus->setPalette(pal);
		QFont f = _procStatus->font();
		f.setBold(true);
		_procStatus->setFont(f);
		_procStatus->setText(tr("Setup ran successfully."));
	}

	_buttonBack->setEnabled(true);
	_buttonNext->setEnabled(false);
	_buttonCancel->setText(tr("Close"));
}
