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




#define SEISCOMP_COMPONENT Gui::QcView
#include <seiscomp3/logging/log.h>

#include <seiscomp3/core/exceptions.h>

#include "qcview.h"
#include "qcmodel.h"
#include "qcitemview.h"
#include "qcviewconfig.h"

namespace Seiscomp {
namespace Applications {
namespace Qc {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QcItemDelegate::QcItemDelegate(int mode, QObject *parent)
	: QItemDelegate(parent), _mode(mode) {

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QcItemDelegate::~QcItemDelegate() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemDelegate::setModel(QAbstractItemModel* model) {
	if ( !model )
		throw Core::GeneralException();

	try {
		_model = static_cast<const QAbstractProxyModel*>(model);
		_qcModel = static_cast<QcModel*>(_model->sourceModel());
	}
	catch ( std::exception &e ){
		SEISCOMP_ERROR("%s", e.what());
		exit(1);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const {

	if (_mode == 0)
		paint0(painter, opt, index);
	else
		paint1(painter, opt, index);

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! (normal) table view
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemDelegate::paint0(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const {
 	painter->save();

	//! translate: proxyModelIndex --> ModelIndex
	QModelIndex rawindex(_model->mapToSource(index));

#ifdef FANCY_GRAPHICS
		QStyleOptionViewItem option = opt;
		painter->setClipRect(opt.rect);
		QColor color;
		QVariant var = _model->data(index, Qt::BackgroundColorRole);
		if (var.isValid()) color = var.value<QColor>();
		else QColor();

		QPoint start = opt.rect.bottomLeft();
		start.rx() -= 20;
		QLinearGradient grad(start, opt.rect.bottomRight());
		grad.setColorAt(1, color);
		grad.setColorAt(0, Qt::white);

// 		painter->setPen(Qt::NoPen);
// 		painter->setBrush(grad);
// 		painter->drawRect(opt.rect);

// 	painter->setPen(Qt::black);
//  	QString content = opt.fontMetrics.elidedText(_model->data(index, Qt::DisplayRole).toString(), Qt::ElideRight, opt.rect.width());
 	QString content = _model->data(index, Qt::DisplayRole).toString();
// 	painter->drawText(opt.rect, Qt::AlignHCenter|Qt::AlignRight, content);

	option.palette.setBrush(QPalette::Background, grad);
	QItemDelegate::drawBackground(painter, option, index);
	QItemDelegate::drawDisplay(painter, option, option.rect, content);
#else
	//! paint standard base cell
	QItemDelegate::paint(painter, opt, index);
#endif	


	/*
	if (_qcModel->getAlertData(rawindex) != NULL) {
			QPen p;
			p.setStyle(Qt::SolidLine);
			p.setWidth(1);
			p.setColor(Qt::black);
			painter->setPen(p);
			painter->setBrush(Qt::NoBrush);
			QRect r(opt.rect);
			//r.setX(opt.rect.x()+0);
			//r.setY(opt.rect.y()+0);
			r.setWidth(opt.rect.width()-1);
			r.setHeight(opt.rect.height()-1);
			painter->drawRect(r);
	}
	*/

	painter->restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



//! summary view
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemDelegate::paint1(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const {
	painter->save();

		painter->setClipRect(opt.rect);

		//! translate: proxyModelIndex --> ModelIndex
		QModelIndex rawindex(_model->mapToSource(index));

		QStyleOptionViewItem option = opt;
// 		option.font = QFont("arial", 8);

		QString streamID = option.fontMetrics.elidedText(_qcModel->getKey(rawindex), Qt::ElideRight, option.rect.width());

		painter->setPen(Qt::NoPen);
#ifdef FANCY_GRAPHICS
		QLinearGradient grad(QPointF(0, 7), QPointF(200, 0));
		grad.setColorAt(0, getSumColor(rawindex));
		grad.setColorAt(1, Qt::white);
		painter->setBrush(grad);
#else
		painter->setBrush(getSumColor(rawindex));
#endif
		painter->drawRect(option.rect);

		painter->setPen(Qt::SolidLine);
		painter->setFont(option.font);
		painter->drawText(3, option.rect.height()-2, streamID);

		if (_qcModel->hasAlerts(streamID)) {
				painter->setBrush(Qt::NoBrush);
				painter->setPen(Qt::SolidLine);
				painter->setPen(Qt::black);
				painter->drawRect(option.rect);
		}

		if (!_qcModel->streamEnabled(rawindex)) {
				painter->setBrush(Qt::NoBrush);
				painter->setPen(Qt::SolidLine);
				painter->setPen(Qt::black);
				painter->drawLine(option.rect.topLeft(), option.rect.bottomRight());
				painter->drawLine(option.rect.topRight(), option.rect.bottomLeft());
		}
	

		
	painter->restore();

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QSize QcItemDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const {

	return QSize(opt.rect.width(), opt.rect.height());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline QColor QcItemDelegate::getSumColor(const QModelIndex& index) const {
	QModelIndex inx;
	int unset = 0;
	int score = 0;
	int count = _qcModel->config()->parameter().size();

	for (int i = 0; i < count; i++) {
		inx = index.child(index.row(), i+2); // offset for: streamID, enabled

		QVariant v = _qcModel->data(inx, Qt::UserRole);
		if (!v.isValid()) continue;
		unset++;
		score += _qcModel->config()->count(_qcModel->config()->parameterName(i), v.toDouble());
	}

	if (unset == 0) score = 1000;

	return _qcModel->config()->sumColor("default", score);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QcItemView::QcItemView(QWidget *parent)
	: QAbstractItemView(parent), _parent(parent)
{
	setSelectionMode(QAbstractItemView::NoSelection);
	setEditTriggers(QAbstractItemView::NoEditTriggers);

	horizontalScrollBar()->setRange(0, 0);
	verticalScrollBar()->setRange(0, 0);
	verticalScrollBar()->setSingleStep(20);

	_totalSize = 0;
	_compactView = true;

	QGroupBox* infoWidget = new QGroupBox(this);
	infoWidget->setTitle("Detailed Info");
	infoWidget->setToolTip("Displays detailed QC Parameter Info for the selected Stream");
	infoWidget->setFlat(false);
	_infoWidget = infoWidget;
	_infoWidget->hide();
	_infoWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	QLayout* mainLayout = new QHBoxLayout();
	_infoWidget->setLayout(mainLayout);

	_infoTable = NULL;

	_hideInfoWidget = new QCheckBox();
	_hideInfoWidget->setCheckState(Qt::Checked);
	_hideInfoWidget->setToolTip("hide this Info Display");
	mainLayout->addWidget(_hideInfoWidget);
	
	parent->layout()->addWidget(_infoWidget);

	connect(this, SIGNAL(pressed(const QModelIndex&)), this, SLOT(showInfo(const QModelIndex&)));
	connect(_hideInfoWidget, SIGNAL(clicked()), _infoWidget, SLOT(hide()));

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemView::showInfo(const QModelIndex& index) {

 	const QAbstractProxyModel* mod = static_cast<const QAbstractProxyModel*>(index.model());
 	if (!mod) return;
 	QcModel* qcModel = static_cast<QcModel*>(mod->sourceModel());

	QString streamID = mod->data(index, Qt::UserRole).toString();

	if (! _infoTable) {
		_infoTable = new QcTableView(qcModel);
		_infoTable->setRecordStreamURL(static_cast<QcView*>(_parent)->recordStreamURL());
		_infoTable->setDatabaseQueryInterface(static_cast<QcView*>(_parent)->databaseQueryInterface());

		_infoTable->qTableView()->verticalHeader()->setStretchLastSection(true);
		_infoTable->setMaximumHeight(2*(fontMetrics().height()+10));
		_infoTable->hideFilterWidget(true);
		_infoWidget->layout()->addWidget(_infoTable);
	}
	

	_infoTable->setFilterRegExp(streamID);
	_hideInfoWidget->setCheckState(Qt::Checked);
	_infoWidget->show();

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemView::dataChanged(const QModelIndex &topLeft,
						const QModelIndex &bottomRight)
{
	QAbstractItemView::dataChanged(topLeft, bottomRight);

	viewport()->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemView::reset() {

	QAbstractItemView::reset();

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QModelIndex QcItemView::indexAt(const QPoint &point) const
{
	QPoint p = point;
	p.setY(point.y() + verticalOffset());

	for (ItemPositions::const_iterator it = _itemPositions.begin(); it !=_itemPositions.end(); ++it) {
		
		if (it.value().contains(p))
			return it.key();
		}
	
	return QModelIndex();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QcItemView::isIndexHidden(const QModelIndex & /*index*/) const
{
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QRect QcItemView::itemRect(const QModelIndex &index) const
{
	if (!index.isValid())
		return QRect();

	// ...

	return QRect();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int QcItemView::horizontalOffset() const
{
	return horizontalScrollBar()->value();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemView::paintEvent(QPaintEvent *event)
{
// 	QItemSelectionModel *selections = selectionModel();
	QStyleOptionViewItem option = viewOptions();

	QBrush background = option.palette.base();
	QPen foreground(option.palette.color(QPalette::WindowText));

 	QPainter painter(viewport());
  	painter.setRenderHint(QPainter::Antialiasing);

	painter.fillRect(event->rect(), background);
	painter.setPen(foreground);

	int xmargin = 5;

	// Viewport rectangles
	QRect vp = painter.viewport();
	int vx = vp.width();
// 	int vy = vp.height();
	int cdx = 0;
	int cdy = 0;
	int border = 1;

	// min item size x/y
	QFontMetrics qfm(option.font);
	int wx = qfm.width("WW.WWWW.00.BHZ");
	int wy = qfm.height();

	// column/row count
	int nx = vx/(wx+border);
// 	int ny = vy/(wy+border);

	// remaining space px
	int rx = vx % (wx+border);

	// divide rem. space
	wx += (rx/nx -1);	

	ItemPositions itemPos;
// 	_itemPositions.clear();

	painter.translate(xmargin, -verticalOffset());

	
// 	option.font = QFont("arial", 8);
	option.rect = QRect(0, 0, wx, wy);

	QModelIndex ij;
	QString streamID;
	QString netCode;

	int total = model()->rowCount(rootIndex());
	int start = 0;
	int count = total;

	painter.setPen(Qt::black);

	for (int row = start; row < count; ++row) {
		ij = model()->index(row, 0, rootIndex());

		if (!_compactView) {
			streamID = model()->data(ij, Qt::UserRole).toString();
			QString net = streamID.left(2).replace(QChar('.'), QString(""), Qt::CaseSensitive);
			if (net != netCode) {
				netCode = net;
				int lf = cdx==0?0:1*(wy+border);
				painter.translate(-cdx, lf+5);
				cdx = 0; cdy += lf+5;
#ifdef FANCY_GRAPHICS
				painter.save();
					painter.setPen(Qt::NoPen);
					QLinearGradient grad(QPointF(0, 7), QPointF(vx/2, 0));
					grad.setColorAt(0, QColor(0,0,255,100));
					grad.setColorAt(1, Qt::white);
					painter.setBrush(grad);
					painter.drawRect(0, wy-1, vx, -wy);
				painter.restore();
				painter.drawText(0, wy-5, "Network "+net);
#else
				painter.drawText(0, wy-1, "Network "+net);
#endif
				painter.translate(0, wy+border);
				cdy += wy+border;
			}
		}

 		itemDelegate()->paint(&painter, option, ij);
		itemPos.insert(ij, QRect(cdx+xmargin, cdy, wx, wy));

		painter.translate( wx+border, 0); cdx += wx+border;
		if ( cdx >= vx-(wx+border) ) {
			painter.translate(-cdx, wy+border);
			cdx  = 0; cdy += wy+border;
		}
	}

	_itemPositions = itemPos;
	
	_totalSize = cdy + wy;
	updateGeometries();

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemView::resizeEvent(QResizeEvent * /* event */)
{
	updateGeometries();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int QcItemView::rows(const QModelIndex &index) const
{
	return model()->rowCount(model()->parent(index));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemView::rowsInserted(const QModelIndex &parent, int start, int end)
{
	for (int row = start; row <= end; ++row) {

		QModelIndex index = model()->index(row, 1, rootIndex());
		double value = model()->data(index, Qt::UserRole).toDouble();

		if (value > 0.0) {
			_totalValue += value;
			_validItems++;
		}
	}

	QAbstractItemView::rowsInserted(parent, start, end);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
	for (int row = start; row <= end; ++row) {

		QModelIndex index = model()->index(row, 1, rootIndex());
		double value = model()->data(index, Qt::UserRole).toDouble();
		if (value > 0.0) {
			_totalValue -= value;
			_validItems--;
		}
	}

	QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemView::scrollContentsBy(int dx, int dy)
{
	viewport()->scroll(dx, dy);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemView::scrollTo(const QModelIndex &index, ScrollHint)
{
	QRect area = viewport()->rect();
	QRect rect = visualRect(index);

	if (rect.left() < area.left())
		horizontalScrollBar()->setValue(
			horizontalScrollBar()->value() + rect.left() - area.left());
	else if (rect.right() > area.right())
		horizontalScrollBar()->setValue(
			horizontalScrollBar()->value() + qMin(
				rect.right() - area.right(), rect.left() - area.left()));

	if (rect.top() < area.top())
		verticalScrollBar()->setValue(
			verticalScrollBar()->value() + rect.top() - area.top());
	else if (rect.bottom() > area.bottom())
		verticalScrollBar()->setValue(
			verticalScrollBar()->value() + qMin(
				rect.bottom() - area.bottom(), rect.top() - area.top()));

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QModelIndex QcItemView::moveCursor(QAbstractItemView::CursorAction cursorAction,
 							Qt::KeyboardModifiers modifiers) {

	return QModelIndex();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
	// Use content widget coordinates because we will use the itemRegion()
	// function to check for intersections.


	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemView::updateGeometries()
{
/*	horizontalScrollBar()->setPageStep(viewport()->width());
	horizontalScrollBar()->setRange(0, qMax(0, _totalSize - viewport()->width()));*/
	verticalScrollBar()->setPageStep(viewport()->height());
	verticalScrollBar()->setRange(0, qMax(0, _totalSize - viewport()->height()));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int QcItemView::verticalOffset() const
{
	return verticalScrollBar()->value();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QRect QcItemView::visualRect(const QModelIndex &index) const
{
	QRect rect = itemRect(index);
	if (rect.isValid())
		return QRect(rect.left() - horizontalScrollBar()->value(),
					rect.top() - verticalScrollBar()->value(),
					rect.width(), rect.height());
	else
		return rect;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QRegion QcItemView::visualRegionForSelection(const QItemSelection &selection) const
{
	int ranges = selection.count();

	if (ranges == 0)
		return QRect();

	QRegion region;
	for (int i = 0; i < ranges; ++i) {
		QItemSelectionRange range = selection.at(i);
		for (int row = range.top(); row <= range.bottom(); ++row) {
			for (int col = range.left(); col <= range.right(); ++col) {
				QModelIndex index = model()->index(row, col, rootIndex());
				region += visualRect(index);
			}
		}
	}
	return region;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemView::setCompactView(int state) {

	_compactView = state==Qt::Checked?true:false;

	viewport()->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemView::mousePressEvent(QMouseEvent *event)
{
	QAbstractItemView::mousePressEvent(event);

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QcItemView::mouseMoveEvent(QMouseEvent *event)
{

	return;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QcItemView::viewportEvent(QEvent* event)
{
	if (event->type() == QEvent::ToolTip || event->type() == QEvent::WhatsThis)
		return false;
		
	return QAbstractItemView::viewportEvent(event);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




}
}
}
