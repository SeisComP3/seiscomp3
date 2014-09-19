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


#include <seiscomp3/gui/core/xmlview.h>
#include <QtXml>


namespace Seiscomp {
namespace Gui {


class XmlHandler : public QXmlDefaultHandler {
	public:
		XmlHandler(QTreeWidget* w)
		 : QXmlDefaultHandler(), _tree(w), _parent(NULL), _currentItem(NULL) {}

		bool startElement(const QString& /*namespaceURI*/,
		                  const QString& localName,
		                  const QString& /*qName*/,
		                  const QXmlAttributes& atts) {

			_currentItem = new QTreeWidgetItem();
			_currentItem->setText(0, localName);
			if ( _parent == NULL )
				_tree->addTopLevelItem(_currentItem);
			else
				_parent->addChild(_currentItem);

			QFont font(_currentItem->font(0));
			font.setBold(true);
			_currentItem->setFont(0, font);

			_parent = _currentItem;

			for ( int i = 0; i < atts.count(); ++i ) {
				QTreeWidgetItem* item = new QTreeWidgetItem(_parent);
				item->setText(0, atts.localName(i));
				item->setText(1, atts.value(i));
				QFont font(item->font(0));
				//font.setBold(true);
				font.setItalic(true);
				item->setFont(0, font);
			}

			_tree->expandItem(_parent);

			return true;
		}

		bool characters(const QString & ch) {
			if ( _currentItem ) {
				_currentItem->setText(1, ch);
			}

			return true;
		}

		bool endElement(const QString& /*namespaceURI*/,
		                const QString& /*localName*/,
		                const QString& /*qName*/) {
			if ( _parent != NULL )
				_parent = _parent->parent();

			_currentItem = NULL;

			return true;
		}

		bool endDocument() {
			//_tree->adjustSize();
			_tree->resizeColumnToContents(0);
			_tree->header()->resizeSection(0, _tree->header()->sectionSize(0) + 10);
			return true;
		}

	private:
		QTreeWidget* _tree;
		QTreeWidgetItem* _parent;
		QTreeWidgetItem* _currentItem;
};


XMLView::XMLView(QWidget * parent, Qt::WFlags f, bool deleteOnClose)
: QWidget(parent, f) {
	if ( deleteOnClose )
		setAttribute(Qt::WA_DeleteOnClose);

	_ui.setupUi(this);
	_ui.treeWidget->setColumnCount(2);
	QTreeWidgetItem* header = new QTreeWidgetItem();
	header->setText(0, tr("Name"));
	header->setText(1, tr("Value"));
	_ui.treeWidget->setHeaderItem(header);
}

XMLView::~XMLView() {
}

void XMLView::setContent(const QString& content) {
	_ui.treeWidget->clear();
	//_ui.treeWidget->adjustSize();

	QXmlInputSource source;
	source.setData(content);

	QXmlSimpleReader xmlReader;
	XmlHandler *handler = new XmlHandler(_ui.treeWidget);

	xmlReader.setContentHandler(handler);
	xmlReader.setErrorHandler(handler);
	xmlReader.parse(&source);

	delete handler;
}


}
}
