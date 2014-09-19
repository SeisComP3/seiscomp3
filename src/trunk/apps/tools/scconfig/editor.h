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


#ifndef __SEISCOMP_CONFIGURATION_EDITOR_H__
#define __SEISCOMP_CONFIGURATION_EDITOR_H__


#include <QtGui>
#include <seiscomp3/system/model.h>


class ConfigHighlighter : public QSyntaxHighlighter {
	public:
		ConfigHighlighter(QTextDocument *parent = NULL);

		virtual void highlightBlock(const QString & text);

	private:
		struct Rule {
			QRegExp pattern;
			QTextCharFormat format;
		};

		QVector<Rule> _rules;

		QTextCharFormat _keywordFormat;
		QTextCharFormat _commentFormat;
		QTextCharFormat _quotationFormat;
		QTextCharFormat _variableFormat;
		QTextCharFormat _placeHolderFormat;
};


class ConfigEditor : public QTextEdit {
	Q_OBJECT

	public:
		ConfigEditor(QWidget *parent = NULL);
		ConfigEditor(const QString &text, QWidget *parent = NULL);

	public:
		int lineNumberAreaWidth();

		void setErrorLine(int line);
		void gotoLine(int line);

	protected:
		void resizeEvent(QResizeEvent *event);

	private slots:
		void updateExtraSelections();

		void updateLineNumberAreaWidth(int newBlockCount);
		void updateLineNumberArea(const QRect &, int);

		void layoutChanged(const QRectF &);
		void scrollBarValueChanged(int);

	private:
		void init();

	private:
		QWidget *_lineNumbers;
		int _errorLine;
};


class ConfigFileWidget : public QWidget {
	Q_OBJECT

	public:
		typedef QPair<int,QString> Error;

		ConfigFileWidget(QWidget *parent = NULL);

	public:
		ConfigEditor *editor() const;

		bool loadFile(const QString &filename);
		bool saveFile(const QString &filename);
		void setErrors(const QList<Error> &, bool selectFirst = false);

	private slots:
		void setError(int);

	private:
		ConfigEditor *_editor;
		QListWidget  *_errorlist;
		QSplitter    *_splitter;
};


class ConfigConflictWidget : public QWidget {
	Q_OBJECT

	public:
		ConfigConflictWidget(QWidget *parent = NULL);

	public:
		void setConflicts(const QList<Seiscomp::System::ConfigDelegate::CSConflict> &);

	public slots:
		void fixConflicts();

	private:
		QList<Seiscomp::System::ConfigDelegate::CSConflict> _conflicts;
		QListWidget *_list;
};



#endif
