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


#define SEISCOMP_COMPONENT EventSummaryView

#include "eventsummaryview.h"

#include <seiscomp3/logging/log.h>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/originquality.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/datamodel/momenttensor.h>
#include <seiscomp3/datamodel/journalentry.h>
#include <seiscomp3/datamodel/databasequery.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/config/config.h>
#include <seiscomp3/system/environment.h>
#include <seiscomp3/math/conversions.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/seismology/regions.h>
#include <seiscomp3/utils/replace.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/seismology/ttt.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/datamodel/tensorsymbol.h>
#include <seiscomp3/gui/datamodel/utils.h>

#include <QMessageBox>
#include <QStringList>

#ifdef WIN32
#undef min
#undef max
#endif

using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Core;
using namespace Seiscomp::DataModel;


namespace {


#define SET_COLOR(w,c) {\
	QPalette p = w->palette();\
	p.setColor(QPalette::WindowText, c);\
	w->setPalette(p);\
}\


struct PoiResolver : public Seiscomp::Util::VariableResolver {
	PoiResolver(const double& dist,
	            const std::string& dir,
	            const std::string& name,
	            double lat, double lon)
	 : _lat(lat), _lon(lon), _dist(dist), _dir(dir), _name(name) {}

	bool resolve(std::string& variable) const {
		if ( VariableResolver::resolve(variable) )
			return true;

		if ( variable == "dist" )
			variable = toString(_dist);
		else if ( variable == "dir" )
			variable = _dir;
		else if ( variable == "poi" )
			variable = _name;
		else if ( variable == "region" )
			variable = Seiscomp::Regions().getRegionName(_lat, _lon);
		else
			return false;

		return true;
	}

	double _lat, _lon;
	const double& _dist;
	const std::string& _dir;
	const std::string& _name;
};


}



namespace Seiscomp {
namespace Gui {


// <---------------------------------------------------------------------------------------------->


MagList::MagList(QWidget* parent)
 : QWidget(parent), _space(true)
{
	_referenceMagsVisible = false;
// 	setGeometry(0, 0, 10, 10);

//     QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
//     sizePolicy.setHorizontalStretch(0);
//     sizePolicy.setVerticalStretch(0);
// //     sizePolicy.setHeightForWidth(_widget->sizePolicy().hasHeightForWidth());
//     setSizePolicy(sizePolicy);
//
// //     setMinimumSize(QSize(100, 100));
//      setMaximumSize(QSize(500, 400));
//
//
// 	_widget = new QWidget(this);

	_mainLayout = new QGridLayout(this);
	_mainLayout->setSpacing(0);
	_mainLayout->setMargin(0);
// 	setLayout(_mainLayout);

// 	setWidget(_widget);
// 	setFrameStyle(QFrame::NoFrame);
// 	setWidgetResizable(true);
// 	setAttribute( Qt::WA_StaticContents );

// 	updateGeometry();


	// set Header Line
	_header = new MagRow(NULL, false, this);
	_header->setVisible(false);
	_mainLayout->addWidget(_header->_type, 0, 0);
	_mainLayout->addWidget(_header->_magnitude, 0, 1, 1, 2);
	_mainLayout->addWidget(_header->_stdev, 0, 3, 1, 2);
	_mainLayout->addWidget(_header->_quality, 0, 5, 1, 2);
}

// QSize MagList::sizeHint() const{
//
// return _widget->size();
// // return QSize(300,10);
// }


MagList::~MagList(){
	clear();
}


void MagList::clear(){
	// close and delete magnitude rows
	for (int i = 0; i < _magRows.size(); i++) {
		delete _magRows.at(i);
	}
	_magRows.clear();
}


void MagList::reset() {
	foreach(MagRow* row, _magRows) {
		row->setMagnitude(NULL);
		row->setBold(false);
	}
}


void MagList::addMag(DataModel::Magnitude* netMag, bool bold, bool visible){
	// create new magnitude display row
	MagRow *magRow = NULL;

	if ( netMag )
		magRow = row(netMag->type());

	if ( !magRow ) {
		magRow = new MagRow(netMag, bold, this);
		magRow->setReferenceMagnitudeVisible(_referenceMagsVisible);
		magRow->setReferenceMagnitudeColor(_referenceColor);
		_magRows.push_back(magRow);
		//_mainLayout->addWidget(_magRows.back());
		int row = _mainLayout->rowCount();
		_mainLayout->addWidget(_magRows.back()->_type, row, 0);
		_mainLayout->addWidget(_magRows.back()->_magnitude, row, 1);
		_mainLayout->addWidget(_magRows.back()->_magnitudeReference, row, 2);
		_mainLayout->addWidget(_magRows.back()->_stdev, row, 3);
		_mainLayout->addWidget(_magRows.back()->_stdevReference, row, 4);
		_mainLayout->addWidget(_magRows.back()->_quality, row, 5);
		_mainLayout->addWidget(_magRows.back()->_qualityReference, row, 6);

		if ( visible )
			_header->setVisible(true);
	}
	else {
		magRow->setMagnitude(netMag);
		magRow->setBold(bold);
	}

	magRow->setVisible(visible);
}


void MagList::addMag(const std::string& type, bool bold, bool visible) {
	MagRow *magRow = row(type);
	if ( !magRow ) {
		magRow = new MagRow(type, bold, this);
		magRow->setReferenceMagnitudeVisible(_referenceMagsVisible);
		magRow->setReferenceMagnitudeColor(_referenceColor);
		_magRows.push_back(magRow);
		//_mainLayout->addWidget(_magRows.back());
		int row = _mainLayout->rowCount();
		_mainLayout->addWidget(_magRows.back()->_type, row, 0);
		_mainLayout->addWidget(_magRows.back()->_magnitude, row, 1);
		_mainLayout->addWidget(_magRows.back()->_magnitudeReference, row, 2);
		_mainLayout->addWidget(_magRows.back()->_stdev, row, 3);
		_mainLayout->addWidget(_magRows.back()->_stdevReference, row, 4);
		_mainLayout->addWidget(_magRows.back()->_quality, row, 5);
		_mainLayout->addWidget(_magRows.back()->_qualityReference, row, 6);

		if ( visible )
			_header->setVisible(true);
	}
	else
		magRow->setBold(bold);
	
	magRow->setVisible(visible);
}


void MagList::updateMag(DataModel::Magnitude* netMag) {
	foreach(MagRow* row, _magRows) {
		if ( row->magnitude() && (row->magnitude()->publicID() == netMag->publicID()) ) {
			row->updateContent();
			return;
		}
	}
}


void MagList::updateReferenceMag(DataModel::Magnitude* netMag) {
	foreach(MagRow* row, _magRows) {
		if ( row->referenceMagnitude() && (row->referenceMagnitude()->publicID() == netMag->publicID()) ) {
			row->updateContent();
			return;
		}
	}
}


void MagList::selectMagnitude(const char *id) {
	foreach(MagRow* row, _magRows) {
		if ( row->magnitude() &&
		     row->magnitude()->publicID() == id )
			row->setBold(true);
		else
			row->setBold(false);
	}
}


MagRow* MagList::row(const std::string& type) const {
	foreach(MagRow* row, _magRows) {
		if ( row->_type->text() == type.c_str() )
			return row;
	}

	return NULL;
}


void MagList::showAll() {
	_header->setVisible(true);
	foreach(MagRow* row, _magRows)
		row->setVisible(true);
}


void MagList::hideTypes(const std::set<std::string>& types) {
	bool allHidden = true;
	foreach(MagRow* row, _magRows) {
		row->setVisible(types.find(row->_type->text().toStdString()) != types.end());
		if ( row->isVisible() )
			allHidden = false;
	}

	_header->setVisible(!allHidden);
}


void MagList::setReferenceMagnitudesVisible(bool v) {
	_referenceMagsVisible = v;
	foreach(MagRow* row, _magRows) {
		row->setReferenceMagnitudeVisible(v);
	}
}


void MagList::setReferenceMagnitudesColor(QColor c) {
	_referenceColor = c;

	foreach(MagRow* row, _magRows) {
		row->setReferenceMagnitudeColor(c);
	}
}


// <---------------------------------------------------------------------------------------------->



MagRow::MagRow(DataModel::Magnitude* netMag, bool bold, QWidget* parent)
 : QWidget(parent), _netMag(netMag), _netMagReference(NULL)
{
	_header = _netMag == NULL;
	_referenceMagVisible = false;
	init();
	setBold(bold);
}


MagRow::MagRow(const std::string& type, bool bold, QWidget *parent)
 : QWidget(parent), _netMag(NULL), _netMagReference(NULL)
{
	_header = false;
	_referenceMagVisible = false;
	init();
	_type->setText(type.c_str());
	setBold(bold);
}


MagRow::~MagRow() {
}


void MagRow::init() {
	_type      = new QLabel();
	_magnitude = new QLabel();
	_stdev     = new QLabel();
	_quality   = new QLabel();

	/*
	_type->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	_magnitude->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	_stdev->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	_quality->setFrameStyle(QFrame::StyledPanel | QFrame::Sunken);
	*/

	if ( !_header ) {
		_magnitudeReference = new QLabel();
		_stdevReference     = new QLabel();
		_qualityReference   = new QLabel();
	}
	else {
		_magnitudeReference = NULL;
		_stdevReference     = NULL;
		_qualityReference   = NULL;
	}

	_type->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
	_magnitude->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	_stdev->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	_quality->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

	_magnitude->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	_stdev->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	_quality->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

	if ( !_header ) {
		_magnitudeReference->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		_stdevReference->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
		_qualityReference->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

		_magnitudeReference->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		_stdevReference->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		_qualityReference->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	}

	updateContent();

	_rowsLayout = new QHBoxLayout(this);
	_rowsLayout->setSpacing(0);
	_rowsLayout->setMargin(0);
	_rowsLayout->addWidget(_type);

	if ( !_header ) {
		QHBoxLayout *l = new QHBoxLayout;
		l->setMargin(0);
		l->addWidget(_magnitude);
		//l->addWidget(_magnitudeReference);
		_rowsLayout->addLayout(l);
	}
	else {
		_rowsLayout->addWidget(_magnitude);
	}

	if ( !_header ) {
		QHBoxLayout *l = new QHBoxLayout;
		l->setMargin(0);
		l->addWidget(_stdev);
		//l->addWidget(_stdevReference);
		_rowsLayout->addLayout(l);
	}
	else {
		_rowsLayout->addWidget(_stdev);
	}

	if ( !_header ) {
		QHBoxLayout *l = new QHBoxLayout;
		l->setMargin(0);
		l->addWidget(_quality);
		//l->addWidget(_qualityReference);
		_rowsLayout->addLayout(l);
	}
	else {
		_rowsLayout->addWidget(_quality);
	}

	setReferenceMagnitudeVisible(_referenceMagVisible);
}


void MagRow::setMagnitude(DataModel::Magnitude* nm) {
	_netMag = nm;
	updateContent();
}


void MagRow::setReferenceMagnitude(DataModel::Magnitude* nm) {
	_netMagReference = nm;
	updateContent();
}


void MagRow::setReferenceMagnitudeVisible(bool v) {
	_referenceMagVisible = v;

	if ( _magnitudeReference ) _magnitudeReference->setVisible(v && isVisible());
	if ( _qualityReference ) _qualityReference->setVisible(v && isVisible());
	if ( _stdevReference ) _stdevReference->setVisible(v && isVisible());

	if ( v ) updateContent();
}


void MagRow::setReferenceMagnitudeColor(QColor c) {
	if ( _magnitudeReference )
		SET_COLOR(_magnitudeReference, c);

	if ( _qualityReference )
		SET_COLOR(_qualityReference, c);

	if ( _stdevReference )
		SET_COLOR(_stdevReference, c);
}


void MagRow::setBold(bool bold) {
	QFont font(_netMag || !_header ? SCScheme.fonts.large : SCScheme.fonts.normal);
	if ( bold ) font.setBold(true);

	_magnitude->setFont(font);
	_type->setFont(font);
	_quality->setFont(font);
	_stdev->setFont(font);

	font.setBold(false);
	if ( _magnitudeReference ) _magnitudeReference->setFont(font);
	if ( _qualityReference ) _qualityReference->setFont(font);
	if ( _stdevReference ) _stdevReference->setFont(font);
}


void MagRow::setVisible(bool v) {
	QWidget::setVisible(v);

	_magnitude->setVisible(v);
	_type->setVisible(v);
	_quality->setVisible(v);
	_stdev->setVisible(v);

	if ( _magnitudeReference ) _magnitudeReference->setVisible(_referenceMagVisible && v);
	if ( _qualityReference ) _qualityReference->setVisible(_referenceMagVisible && v);
	if ( _stdevReference ) _stdevReference->setVisible(_referenceMagVisible && v);
}


void MagRow::updateContent() {
	if (_netMag) { // set Magnitude Row
		char buf[10] = "-";
		double netmagval = _netMag->magnitude().value();
		if (netmagval<12)
			sprintf(buf, "%.1f", netmagval);
		_magnitude->setText(buf);
		_type->setText(QString("%1").arg(_netMag->type().c_str()));
		try {
			int count = _netMag->stationCount();
			_quality->setText(QString("%1").arg(count));
		}
		catch ( ... ) {
			_quality->setText("-");
		}

		double stdev = 100;
		try {
			stdev = _netMag->magnitude().uncertainty();
		}
		catch ( ... ) {
			try {
				stdev = 0.5*(_netMag->magnitude().lowerUncertainty() + _netMag->magnitude().upperUncertainty());
			}
			catch ( ... ) {}
		}

		if ( stdev < 10 ) {
			sprintf(buf, "%.2f", stdev);
			_stdev->setText(buf);
		}
		else
			_stdev->setText("-");
	}
	else if ( _header ){ // set Header Row
		_magnitude->setText(QString("Value"));
		_type->setText(QString("Type"));
		_quality->setText(QString("Count"));
		_stdev->setText(QString("+/-"));
	}
	else {
		_magnitude->setText(QString("-"));
		_quality->setText(QString("-"));
		_stdev->setText(QString("-"));
	}

	if ( _referenceMagVisible ) {
		if ( _netMagReference ) {
			char buf[10] = "-";
			double netmagval = _netMagReference->magnitude().value();
			if (netmagval<12)
				sprintf(buf, "%.1f", netmagval);
			_magnitudeReference->setText(buf);
			try {
				int count = _netMagReference->stationCount();
				_qualityReference->setText(QString("%1").arg(count));
			}
			catch ( ... ) {
				_qualityReference->setText("-");
			}

			double stdev = 100;
			try {
				stdev = _netMagReference->magnitude().uncertainty();
			}
			catch ( ... ) {
				try {
					stdev = 0.5*(_netMagReference->magnitude().lowerUncertainty() + _netMagReference->magnitude().upperUncertainty());
				}
				catch ( ... ) {}
			}

			if ( stdev < 10 ) {
				sprintf(buf, "%.2f", stdev);
				_stdevReference->setText(buf);
			}
			else
				_stdevReference->setText(QString("-"));
		}
		else {
			_magnitudeReference->setText(QString("-"));
			_qualityReference->setText(QString("-"));
			_stdevReference->setText(QString("-"));
		}
	}

	update();
}

// <---------------------------------------------------------------------------------------------->


EventSummaryView::EventSummaryView(const MapsDesc &maps,
                                   DataModel::DatabaseQuery* reader,
                                   QWidget *parent)
: QWidget(parent), _reader(reader)
{
	_maptree = new Gui::Map::ImageTree(maps);
	init();
}


EventSummaryView::EventSummaryView(Map::ImageTree* mapTree,
                                   DataModel::DatabaseQuery* reader,
                                   QWidget *parent)
: QWidget(parent), _reader(reader)
{
	_maptree = mapTree;
	init();
}


EventSummaryView::~EventSummaryView(){
}


#define SET_FONT(labelName, f1, f2) \
	ui.label##labelName->setFont(SCScheme.fonts.f1); \
	ui.label##labelName##Value->setFont(SCScheme.fonts.f2)


#define SET_VALUE(labelName, txt) \
	ui.label##labelName##Value->setText(txt)


#define DISABLE_FRAME(f) \
	f->setFrameShape(QFrame::NoFrame); \
	if ( f->layout() ) \
		f->layout()->setMargin(0)


#define ENABLE_FRAME(f) \
	f->setFrameStyle(QFrame::StyledPanel | QFrame::Raised); \
	if ( f->layout() ) f->layout()->setMargin(4); \
	if ( f->layout() ) f->layout()->setSpacing(4)


class ScrollArea : public QScrollArea {
	public:
		ScrollArea(QWidget *parent)
		: QScrollArea(parent) {
			viewport()->setAutoFillBackground(false);
		}

		QSize sizeHint() const {
			if ( widget() == NULL )
				return QScrollArea::sizeHint();

			return QSize(widget()->minimumWidth(), 0);
		}
};


class ScrollPanelWidget : public QWidget {
	public:
		ScrollPanelWidget(QScrollArea *forward) : _widget(forward) {}

	protected:
		void resizeEvent(QResizeEvent *e) {
			QWidget::resizeEvent(e);
			//_widget->setFixedWidth(e->size().width());
			//_widget->setMinimumWidth(e->size().width());
			_widget->setMinimumWidth(layout()->minimumSize().width());
		}


	private:
		QScrollArea *_widget;
};


void EventSummaryView::init() {
	ui.setupUi(this);

	_enableFullTensor = false;
	_displayFocMechs = true;

	ScrollArea *area = new ScrollArea(ui.frameEpicenterInformation);
	area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	QWidget *infoPanel = new ScrollPanelWidget(area);
	QSizePolicy sp = infoPanel->sizePolicy();
	//sp.setHorizontalPolicy(QSizePolicy::Maximum);
	sp.setVerticalPolicy(QSizePolicy::Maximum);
	infoPanel->setSizePolicy(sp);

	QVBoxLayout *l = new QVBoxLayout(infoPanel);
	l->setMargin(0);

	QWidget *hypoCenterInfo = new QWidget(infoPanel);
	uiHypocenter.setupUi(hypoCenterInfo);
	l->addWidget(hypoCenterInfo);

	/*
	QWidget *focalMechanismInfo = new QWidget(infoPanel);
	uiHypocenter.setupUi(focalMechanismInfo);
	l->addWidget(focalMechanismInfo);
	//focalMechanismInfo->setVisible(false);
	*/

	setFMParametersVisible(false);

	area->setWidget(infoPanel);
	// Set by setWidget, reset it again
	infoPanel->setAutoFillBackground(false);
	area->setWidgetResizable(true);
	//area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	area->setFrameShape(QFrame::NoFrame);
	if ( area->layout() )
		area->layout()->setMargin(0);

	l = new QVBoxLayout(ui.frameEpicenterInformation);
	l->setMargin(0);
	l->addWidget(area);

	_automaticOriginEnabledColor = Qt::darkRed;
	_automaticOriginDisabledColor = palette().color(QPalette::Disabled, QPalette::WindowText);
	_automaticOriginColor = palette().color(QPalette::WindowText);
	_automaticFMColor = palette().color(QPalette::WindowText);

	QObject *drawFilter = new ElideFadeDrawer(this);

	// Set the font sizes
	//ui.frameInformation->setVisible(false);
	bool withBorders = false;
	try { withBorders = SCApp->configGetBool("summary.borders"); } catch ( ... ) {}

	try { _ignoreOtherEvents = SCApp->configGetBool("ignoreOtherEvents"); }
	catch ( ... ) { _ignoreOtherEvents = true; }

	double lonmin = -180;
	double lonmax = 180;
	double latmin = -90;
	double latmax = 90;

	try { lonmin = SCApp->configGetDouble("display.lonmin"); } catch (Config::Exception) {}
	try { lonmax = SCApp->configGetDouble("display.lonmax"); } catch (Config::Exception) {}
	try { latmin = SCApp->configGetDouble("display.latmin"); } catch (Config::Exception) {}
	try { latmax = SCApp->configGetDouble("display.latmax"); } catch (Config::Exception) {}

	QRectF displayRect;
	displayRect.setRect(lonmin, latmin, lonmax-lonmin, latmax-latmin);

	uiHypocenter.labelVDistance->setText("");
	uiHypocenter.labelVDistanceAutomatic->setText("");
	uiHypocenter.labelFrameInfoSpacer->setText("");
	uiHypocenter.labelFrameInfoSpacer->setFixedWidth(16);
	uiHypocenter.fmLabelFrameInfoSpacer->setText("");
	uiHypocenter.fmLabelFrameInfoSpacer->setFixedWidth(16);
	uiHypocenter.labelFMSeparator->setText("\n\nFocalMechanism\n");

	uiHypocenter.fmLabelVDistance->setText("");
	uiHypocenter.fmLabelVDistanceAutomatic->setText("");

	if ( !withBorders ) {
		QFontMetrics fm(SCScheme.fonts.normal);
		QRect rect = fm.boundingRect('M');

		ui.frameVDistance->setFixedHeight(rect.height()*2);
		ui.frameHDistance->setFixedWidth(rect.width());
		ui.frameMagnitudeDistance->setFixedHeight(rect.height());

		DISABLE_FRAME(ui.frameRegion);
		DISABLE_FRAME(ui.frameMagnitudes);
		DISABLE_FRAME(uiHypocenter.frameInformation);
		DISABLE_FRAME(uiHypocenter.frameInformationAutomatic);
		DISABLE_FRAME(uiHypocenter.fmFrameInformation);
		DISABLE_FRAME(uiHypocenter.fmFrameInformationAutomatic);
		DISABLE_FRAME(ui.frameProcessing);
		DISABLE_FRAME(ui.framePlugable);
	}
	else {
		ui.frameVDistance->setVisible(false);
		ui.frameHDistance->setVisible(false);
		ui.frameMagnitudeDistance->setVisible(false);

		ENABLE_FRAME(ui.frameRegion);
		ENABLE_FRAME(ui.frameMagnitudes);
		ENABLE_FRAME(uiHypocenter.frameInformation);
		ENABLE_FRAME(uiHypocenter.frameInformationAutomatic);
		ENABLE_FRAME(uiHypocenter.fmFrameInformation);
		ENABLE_FRAME(uiHypocenter.fmFrameInformationAutomatic);
		ENABLE_FRAME(ui.frameProcessing);
	}

	ui._lbOriginTime->setFont(SCScheme.fonts.heading1);
	ui._lbOriginTimeAutomatic->setFont(SCScheme.fonts.heading1);
	ui._lbTimeAgo->setFont(SCScheme.fonts.heading2);
	ui._lbRegion->setFont(SCScheme.fonts.highlight);
	ui._lbRegion->installEventFilter(drawFilter);
	ui._lbRegionExtra->setFont(SCScheme.fonts.normal);
	ui._lbRegionExtra->installEventFilter(drawFilter);

	ui._lbPreMagType->setFont(SCScheme.fonts.heading1);
	ui._lbPreMagVal->setFont(SCScheme.fonts.heading1);
	ui.labelDepth->setFont(SCScheme.fonts.heading1);
	ui.labelCustomName->setFont(SCScheme.fonts.heading1);
	ui.labelCustomValue->setFont(SCScheme.fonts.heading1);
	ui.labelCustomValue->installEventFilter(drawFilter);

	uiHypocenter._lbLatitudeTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbLatitude->setFont(SCScheme.fonts.highlight);
	uiHypocenter._lbLatitudeUnit->setFont(SCScheme.fonts.highlight);
	uiHypocenter._lbLatError->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbLongitudeTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbLongitude->setFont(SCScheme.fonts.highlight);
	uiHypocenter._lbLongitudeUnit->setFont(SCScheme.fonts.highlight);
	uiHypocenter._lbLongError->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbDepthTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbDepth->setFont(SCScheme.fonts.highlight);
	uiHypocenter._lbDepthUnit->setFont(SCScheme.fonts.highlight);
	uiHypocenter._lbDepthError->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbNoPhasesTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbNoPhases->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbRMSTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbComment->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbCommentTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbRMS->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbAgencyTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbAgency->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbOriginStatusTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbOriginStatus->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbFirstLocTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbFirstLocation->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbThisLocTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbThisLocation->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbEventIDTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbEventID->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbEventID->setSizePolicy(QSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred));
	uiHypocenter._lbEventID->installEventFilter(drawFilter);

	uiHypocenter._lbLatitudeAutomatic->setFont(SCScheme.fonts.highlight);
	uiHypocenter._lbLatitudeUnitAutomatic->setFont(SCScheme.fonts.highlight);
	uiHypocenter._lbLatErrorAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbLongitudeAutomatic->setFont(SCScheme.fonts.highlight);
	uiHypocenter._lbLongitudeUnitAutomatic->setFont(SCScheme.fonts.highlight);
	uiHypocenter._lbLongErrorAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbDepthAutomatic->setFont(SCScheme.fonts.highlight);
	uiHypocenter._lbDepthUnitAutomatic->setFont(SCScheme.fonts.highlight);
	uiHypocenter._lbDepthErrorAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbNoPhasesAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbRMSAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbCommentAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbAgencyAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter._lbOriginStatusAutomatic->setFont(SCScheme.fonts.normal);

	uiHypocenter.labelFMSeparator->setFont(SCScheme.fonts.highlight);

	uiHypocenter.labelLatitudeTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelLatitude->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelLatitudeUnit->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelLatitudeError->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelLongitudeTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelLongitude->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelLongitudeUnit->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelLongitudeError->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelDepthTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelDepth->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelDepthUnit->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelDepthError->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMwTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMw->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelMomentTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMoment->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelMomentUnit->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelPhasesTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelPhases->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMisfitTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMisfit->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelCLVDTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelCLVD->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMinDistTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMinDist->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMinDistUnit->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMaxDistTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMaxDist->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMaxDistUnit->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelNPTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelNP0->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelNP1->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelTypeTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelType->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelAgencyTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelAgency->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelStatusTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelStatus->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelThisSolutionTxt->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelThisSolution->setFont(SCScheme.fonts.normal);

	uiHypocenter.labelLatitudeAutomatic->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelLatitudeUnitAutomatic->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelLatitudeErrorAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelLongitudeAutomatic->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelLongitudeUnitAutomatic->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelLongitudeErrorAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelDepthAutomatic->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelDepthUnitAutomatic->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelDepthErrorAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMwAutomatic->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelMomentAutomatic->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelMomentUnitAutomatic->setFont(SCScheme.fonts.highlight);
	uiHypocenter.labelPhasesAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMisfitAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelCLVDAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMinDistAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMinDistUnitAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMaxDistAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelMaxDistUnitAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelNP0Automatic->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelNP1Automatic->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelTypeAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelAgencyAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelStatusAutomatic->setFont(SCScheme.fonts.normal);
	uiHypocenter.labelThisSolutionAutomatic->setFont(SCScheme.fonts.normal);

	_displayEventComment = false;
	try {
		_displayEventCommentID = SCApp->configGetString("display.event.comment.id");
		_displayEventComment = true;
	}
	catch ( ... ) {}

	try {
		_displayEventCommentDefault = SCApp->configGetString("display.event.comment.default");
	}
	catch ( ... ) {
		_displayEventCommentDefault = ui.labelCustomValue->text().toStdString();
	}

	try {
		ui.labelCustomName->setText(QString("%1").arg(SCApp->configGetString("display.event.comment.label").c_str()));
	}
	catch ( ... ) {
		ui.labelCustomName->setText(_displayEventCommentID.c_str());
	}

	if ( !_displayEventComment ) {
		ui.labelCustomName->setVisible(false);
		ui.labelCustomValue->setVisible(false);
	}

	_displayComment = false;
	try {
		_displayCommentID = SCApp->configGetString("display.origin.comment.id");
		_displayComment = true;
	}
	catch ( ... ) {}

	uiHypocenter._lbCommentTxt->setVisible(_displayComment);
	uiHypocenter._lbComment->setVisible(_displayComment);
	uiHypocenter._lbCommentAutomatic->setVisible(_displayComment);

	try {
		_displayCommentDefault = SCApp->configGetString("display.origin.comment.default");
	}
	catch ( ... ) {
		_displayCommentDefault = uiHypocenter._lbComment->text().toStdString();
	}

	try {
		uiHypocenter._lbCommentTxt->setText(QString("%1:").arg(SCApp->configGetString("display.origin.comment.label").c_str()));
	}
	catch ( ... ) {
		uiHypocenter._lbCommentTxt->setText(_displayCommentID.c_str());
	}

	_maxHotspotDist = 20.0;
	_minHotspotPopulation = 50000;
	_recenterMap = true;

	_showLastAutomaticSolution = false;
	_showOnlyMostRecentEvent = true;
	ui.btnSwitchToAutomatic->setVisible(false);

	try { _maxHotspotDist = SCApp->configGetDouble("poi.maxDist"); } catch ( ... ) {}
	try { _hotSpotDescriptionPattern = SCApp->configGetString("poi.message"); } catch ( ... ) {}
	try { _minHotspotPopulation = SCApp->configGetDouble("poi.minPopulation"); } catch ( ... ) {}
	try { _showLastAutomaticSolution = SCApp->configGetBool("showLastAutomaticSolution"); } catch ( ... ) {}
	try { _showOnlyMostRecentEvent = SCApp->configGetBool("showOnlyMostRecentEvent"); } catch ( ... ) {}
	try {
		if ( SCApp->configGetBool("enableFixAutomaticSolutions") )
			ui.btnSwitchToAutomatic->setVisible(true);
	}
	catch ( ... ) {}

	_maxMinutesSecondDisplay = -1;
	try { _maxMinutesSecondDisplay = SCApp->configGetInt("displayAgoSecondsUpToMaximumMinutes"); }
	catch ( ... ) {}

	try {
		std::vector<std::string> mags = SCApp->configGetStrings("visibleMagnitudes");
		std::copy(mags.begin(), mags.end(), std::inserter(_visibleMagnitudes, _visibleMagnitudes.end()));
	} catch ( ... ) {
		_visibleMagnitudes.insert("M");
		_visibleMagnitudes.insert("MLv");
		_visibleMagnitudes.insert("mb");
		_visibleMagnitudes.insert("mB");
		_visibleMagnitudes.insert("Mw(mB)");
	}

	try {
		ui.btnPlugable0->setText(SCApp->configGetString("button0").c_str());
	} catch ( ... ) {}

	try {
		ui.btnPlugable1->setText(SCApp->configGetString("button1").c_str());
	} catch ( ... ) {}

	ui.btnPlugable0->setVisible(false);
	ui.btnPlugable1->setVisible(false);

	_map = new OriginLocatorMap(_maptree.get(), ui.frameMap);
	_map->setStationsInteractive(false);
	_map->setMouseTracking(true);
	_map->canvas().displayRect(displayRect);

	QHBoxLayout* hboxLayout = new QHBoxLayout(ui.frameMap);
	hboxLayout->setObjectName("hboxLayoutMap");
	hboxLayout->setSpacing(6);
	hboxLayout->setMargin(0);
	hboxLayout->addWidget(_map);

	QAction* refreshAction = new QAction(this);
	refreshAction->setObjectName(QString::fromUtf8("refreshAction"));
	refreshAction->setShortcut(QApplication::translate("EventSummaryView", "F5", 0, QApplication::UnicodeUTF8));
	addAction(refreshAction);

	_magList = new MagList();
	ui.frameMagnitudes->layout()->addWidget(_magList);

	_currentEvent = DataModel::Event::Create("NULL");

//	QFont f(uiHypocenter._lbSystem->font());
//	f.setBold(true);
//	uiHypocenter._lbSystem->setFont(f);

	_autoSelect = TRUE;

	addAction(ui.actionShowInvisibleMagnitudes);

	connect(ui.actionShowInvisibleMagnitudes, SIGNAL(triggered(bool)),
	        this, SLOT(showVisibleMagnitudes(bool)));

	connect(ui.btnShowDetails, SIGNAL(clicked()), this, SIGNAL(toolButtonPressed()));
	connect(ui.btnSwitchToAutomatic, SIGNAL(clicked()), this, SLOT(switchToAutomaticPressed()));
	connect(ui.btnPlugable0, SIGNAL(clicked()), this, SLOT(runScript0()));
	connect(ui.btnPlugable1, SIGNAL(clicked()), this, SLOT(runScript1()));

	connect(refreshAction, SIGNAL(triggered(bool)), this,  SLOT(updateEvent()));

	QTimer* TimeAgoTimer = new QTimer();
	connect(TimeAgoTimer, SIGNAL(timeout()), this, SLOT(updateTimeAgoLabel()));
	TimeAgoTimer->start(1000);

	ui._lbPreMagType->setText(" --");

	ui.btnShowDetails->setEnabled(false);
	ui.btnPlugable0->setEnabled(false);
	ui.btnPlugable1->setEnabled(false);

	_mapTimer = new QTimer(this);
	_mapTimer->setSingleShot(true);

	setInteractiveEnabled(!SCApp->nonInteractive());

	setLastAutomaticOriginColor(_automaticOriginDisabledColor);
	setLastAutomaticFMColor(_automaticOriginDisabledColor);
	setLastAutomaticOriginVisible(_showLastAutomaticSolution);
	clearOriginParameter();
}


void EventSummaryView::setToolButtonText(const QString& text) {
	ui.btnShowDetails->setText(text);
}


void EventSummaryView::setScript0(const std::string& script, bool oldStyle) {
	_script0 = script;
	_scriptStyle0 = oldStyle;
	bool visible0 = !_script0.empty() && _interactive;
	bool visible1 = !_script1.empty() && _interactive;
	ui.btnPlugable0->setVisible(visible0);
	ui.framePlugable->setVisible(visible0 || visible1);
}


void EventSummaryView::setScript1(const std::string& script, bool oldStyle) {
	_script1 = script;
	_scriptStyle1 = oldStyle;
	bool visible0 = !_script0.empty() && _interactive;
	bool visible1 = !_script1.empty() && _interactive;
	ui.btnPlugable1->setVisible(visible1);
	ui.framePlugable->setVisible(visible0 || visible1);
}


OriginLocatorMap* EventSummaryView::map() const {
	return _map;
}


bool EventSummaryView::ignoreOtherEvents() const {
	return _ignoreOtherEvents;
}


Seiscomp::DataModel::Event* EventSummaryView::currentEvent() const {
	return _currentEvent.get();
}

Seiscomp::DataModel::Origin* EventSummaryView::currentOrigin() const {

	if (_currentOrigin)
		return _currentOrigin.get();
	else
		return NULL;
}

Seiscomp::DataModel::Magnitude* EventSummaryView::currentMagnitude() const {

	if (_currentNetMag)
		return _currentNetMag.get();
	else
		return NULL;
}


void EventSummaryView::updateEvent(){

	if (!_reader) return;

	if (_currentEvent){
		emit showInStatusbar(QString("update event received: %1").arg(_currentEvent->publicID().c_str()), 1000);

		setOriginParameter(_currentEvent->preferredOriginID());
		setPrefMagnitudeParameter(_currentEvent->preferredMagnitudeID());
	}
}


void EventSummaryView::addObject(const QString& parentID, Seiscomp::DataModel::Object* obj){
	DataModel::EventPtr event = DataModel::Event::Cast(obj);
	if ( event ) {
		if ( _ignoreOtherEvents ) {
			try {
				if ( event->type() == DataModel::OTHER_EVENT ||
					event->type() == DataModel::NOT_EXISTING ) {
					emit showInStatusbar(QString("filtered new event (type: '%1'): %2")
					                     .arg(event->type().toString())
					                     .arg(event->publicID().c_str()), 10000);
					return;
				}
			}
			catch ( ... ) {}
		}

		emit showInStatusbar(QString("new event received: %1").arg(event->publicID().c_str()), 10000);

		if ( _autoSelect ) {
			if ( checkAndDisplay(event.get()) ) {
	// 			clearLastMagnitudes();
				_mapTimer->stop();
			}
		}
		else {
			emit showInStatusbar(QString("a new event has arrived: %1 [event displayed is %2]")
                   .arg(event->publicID().c_str()).arg(_currentEvent->publicID().c_str()), 10000);
		}
		return;
	}

	DataModel::MagnitudePtr netMag = DataModel::Magnitude::Cast(obj);
	if ( netMag ) {
		if ( !_currentOrigin ) return;

		if ( parentID.toStdString() == _currentOrigin->publicID() ) {
			Magnitude* mag = Magnitude::Find(netMag->publicID());
			if ( mag ) {
				bool visibleType = (_visibleMagnitudes.find(mag->type()) != _visibleMagnitudes.end()) || ui.actionShowInvisibleMagnitudes->isChecked();
				_magList->addMag(mag, false, visibleType);
				if ( visibleType ) {
					QString display;
					try {
						display =
							QString("%1 %2 (%3)")
								.arg(mag->type().c_str())
								.arg(mag->magnitude().value(), 0, 'f', 1)
								.arg(mag->stationCount());
					}
					catch ( ... ) {
						display =
							QString("%1 %2 (-)")
								.arg(mag->type().c_str())
								.arg(mag->magnitude().value(), 0, 'f', 1);
					}
				}
			}
			else
				SEISCOMP_DEBUG("Could not find NetMag %s in Origin %s",
				               netMag->publicID().c_str(), _currentOrigin->publicID().c_str());
		}
		else if ( _showLastAutomaticSolution && _lastAutomaticOrigin && parentID.toStdString() == _lastAutomaticOrigin->publicID() ) {
			Magnitude* mag = Magnitude::Find(netMag->publicID());
			if ( mag ) {
				MagRow *row = _magList->row(mag->type());
				if ( row ) row->setReferenceMagnitude(mag);
			}
			else
				SEISCOMP_DEBUG("Could not find NetMag %s in Origin %s",
				               netMag->publicID().c_str(), _currentOrigin->publicID().c_str());
		}

		return;
	}

	DataModel::OriginReferencePtr oref = DataModel::OriginReference::Cast(obj);
	if ( oref ) {
		if ( !_showLastAutomaticSolution ) return;
		if ( _currentEvent && parentID.toStdString() == _currentEvent->publicID() ) {
			OriginPtr o = Origin::Find(oref->originID());
			if (!o && _reader)
				o = Origin::Cast(_reader->getObject(Origin::TypeInfo(), oref->originID()));

			if ( o ) {
				if ( updateLastAutomaticOrigin(o.get()) )
					setAutomaticOrigin(_lastAutomaticOrigin.get());
			}
		}
		return;
	}

	DataModel::FocalMechanismReferencePtr fmref = DataModel::FocalMechanismReference::Cast(obj);
	if ( fmref ) {
		if ( !_showLastAutomaticSolution ) return;
		if ( _currentEvent && parentID.toStdString() == _currentEvent->publicID() ) {
			FocalMechanismPtr fm = FocalMechanism::Find(fmref->focalMechanismID());
			if (!fm && _reader)
				fm = FocalMechanism::Cast(_reader->getObject(FocalMechanism::TypeInfo(), fmref->focalMechanismID()));

			if ( fm ) {
				if ( updateLastAutomaticFM(fm.get()) ) {
					setAutomaticFM(_lastAutomaticFocalMechanism.get());
					updateMap(false);
				}
			}
		}
		return;
	}

	DataModel::CommentPtr comment = DataModel::Comment::Cast(obj);
	if ( comment ) {
		if ( !_currentEvent ) return;
		if ( parentID.toStdString() == _currentEvent->publicID() )
			updateEventComment();
		return;
	}

	DataModel::EventDescriptionPtr ed = DataModel::EventDescription::Cast(obj);
	if ( ed ) {
		if ( !_currentEvent ) return;
		if ( parentID.toStdString() == _currentEvent->publicID() && ed->type() == EARTHQUAKE_NAME )
			updateEventName();
		return;
	}
}


void EventSummaryView::updateObject(const QString& parentID, Seiscomp::DataModel::Object* obj) {
	DataModel::EventPtr event = DataModel::Event::Cast(obj);
	if ( event ) {
		if ( _ignoreOtherEvents ) {
			try {
				if ( event->type() == DataModel::OTHER_EVENT ||
					event->type() == DataModel::NOT_EXISTING ) {

					// Remove current event and set the last one
					if ( _currentEvent && event->publicID() == _currentEvent->publicID() ) {
						_currentEvent = NULL;
						emit requestNonFakeEvent();
						if ( _currentEvent == NULL )
							showEvent(NULL, NULL);
					}

					emit showInStatusbar(QString("filtered new event (type: '%1'): %2")
					                     .arg(event->type().toString())
					                     .arg(event->publicID().c_str()), 10000);
					return;
				}
			}
			catch ( ... ) {}
		}

		emit showInStatusbar(QString("event update received: %1").arg(event->publicID().c_str()), 10000);

		if ( _autoSelect ) {
			EventPtr registeredEvent = Event::Find(event->publicID());
			if ( registeredEvent )
				event = registeredEvent;

			checkAndDisplay(event.get());
		}
		else if ( _currentEvent ) {
			if ( event->publicID() == _currentEvent->publicID() ) {
				//setPrefMagnitudeParameter(_currentEvent->preferredMagnitudeID());
				processEventMsg(event.get());
			}
			else
				emit showInStatusbar(QString("an event update has arrived: %1 [event displayed is %2]")
				                     .arg(event->publicID().c_str()).arg(_currentEvent->publicID().c_str()), 60000);
		}
		return;
	}

	DataModel::MagnitudePtr netMag = DataModel::Magnitude::Cast(obj);
	if ( netMag ) {
		if ( _currentOrigin && parentID == _currentOrigin->publicID().c_str() ) {
			updateMagnitude(netMag.get());
			if ( _currentEvent && netMag->publicID() == _currentEvent->preferredMagnitudeID() )
				setPrefMagnitudeParameter(_currentEvent->preferredMagnitudeID());
		}
		else if ( _showLastAutomaticSolution && _lastAutomaticOrigin && parentID.toStdString() == _lastAutomaticOrigin->publicID() ) {
			_magList->updateReferenceMag(netMag.get());
		}
		return;
	}

	DataModel::OriginPtr origin = DataModel::Origin::Cast(obj);
	if ( origin ) {
		if ( !_showLastAutomaticSolution ) return;
		if ( _lastAutomaticOrigin && origin->publicID() == _lastAutomaticOrigin->publicID() ) {
			// Origin status changed -> lookup a new last automatic origin
			try {
				if ( origin->evaluationMode() != AUTOMATIC ) {
					_lastAutomaticOrigin = NULL;
					for ( size_t i = 0; i < _currentEvent->originReferenceCount(); ++i ) {
						OriginReference *ref = _currentEvent->originReference(i);
						OriginPtr o = Origin::Find(ref->originID());
						if ( !o && _reader )
							o = Origin::Cast(_reader->getObject(Origin::TypeInfo(), ref->originID()));

						if ( !o ) continue;

						updateLastAutomaticOrigin(o.get());
					}

					setAutomaticOrigin(_lastAutomaticOrigin.get());
				}
			}
			catch ( ... ) {}
		}
		return;
	}

	DataModel::CommentPtr comment = DataModel::Comment::Cast(obj);
	if ( comment ) {
		if ( !_currentEvent ) return;
		if ( parentID.toStdString() == _currentEvent->publicID() )
			updateEventComment();
	}

	DataModel::EventDescriptionPtr ed = DataModel::EventDescription::Cast(obj);
	if ( ed ) {
		if ( !_currentEvent ) return;
		if ( parentID.toStdString() == _currentEvent->publicID() && ed->type() == EARTHQUAKE_NAME )
			updateEventName();
		return;
	}
}


void EventSummaryView::removeObject(const QString &parentID, Seiscomp::DataModel::Object *obj) {
	DataModel::EventPtr event = DataModel::Event::Cast(obj);
	if ( event ) {
		// Remove current event and set the last one
		if ( _currentEvent && event->publicID() == _currentEvent->publicID() ) {
			_currentEvent = NULL;
			emit requestNonFakeEvent();
			if ( _currentEvent == NULL )
				showEvent(NULL, NULL);
		}

		emit showInStatusbar(QString("event %1 removed")
		                     .arg(event->publicID().c_str()), 10000);
		return;
	}
}


void EventSummaryView::showEvent(Seiscomp::DataModel::Event* event, Seiscomp::DataModel::Origin* org){
	if ( event )
		emit showInStatusbar(QString("selected event: %1").arg(event->publicID().c_str()), 1000);
	else
		_currentEvent = DataModel::Event::Create("NULL");

// 	clearLastMagnitudes();
	_mapTimer->stop();
	processEventMsg(event, org);
}


void EventSummaryView::showOrigin(Seiscomp::DataModel::Origin* origin){

	emit showInStatusbar(QString("selected origin: %1").arg(origin->publicID().c_str()), 1000);

// 	clearLastMagnitudes();
	_mapTimer->stop();

	processEventMsg(_currentEvent.get(), origin);
}


void EventSummaryView::processEventMsg(DataModel::Event* event, Seiscomp::DataModel::Origin* org){
	if ( event == NULL ) {
		_currentOrigin = NULL;
		_currentFocalMechanism = NULL;
		_lastAutomaticOrigin = NULL;
		_lastAutomaticFocalMechanism = NULL;
		clearOriginParameter();
		clearMagnitudeParameter();
		clearMap();
		updateTimeAgoLabel();
		return;
	}

	if ( event->preferredOriginID().empty() ) return;

	bool changedEvent = true;
	if ( event && _currentEvent )
		changedEvent = _currentEvent->publicID() != event->publicID();
	else
		changedEvent = true;

	_recenterMap = changedEvent;

	_currentEvent = event;

	if ( _currentEvent ) {
		SEISCOMP_DEBUG("pe  publicID: %s ", _currentEvent->publicID().c_str());
	}

	_currentFocalMechanism = FocalMechanism::Find(_currentEvent->preferredFocalMechanismID());
	if ( !_currentFocalMechanism && _reader )
		_currentFocalMechanism = FocalMechanism::Cast(_reader->getObject(FocalMechanism::TypeInfo(), _currentEvent->preferredFocalMechanismID()));
	if ( _currentFocalMechanism && _reader )
		_reader->loadMomentTensors(_currentFocalMechanism.get());


	if ( org )
		setOrigin(org);
	else
		setOriginParameter(_currentEvent->preferredOriginID());

	if ( _showLastAutomaticSolution ) {
		if ( changedEvent ) {
			_lastAutomaticOrigin = NULL;
			_lastAutomaticFocalMechanism = NULL;
			if ( _showLastAutomaticSolution && _reader ) {
				DatabaseIterator it = _reader->getOriginsDescending(_currentEvent->publicID());
				for ( ; *it; ++it ) {
					OriginPtr o = Origin::Cast(*it);
					if ( updateLastAutomaticOrigin(o.get()) )
						break;
				}
				it.close();

				it = _reader->getFocalMechanismsDescending(_currentEvent->publicID());
				for ( ; *it; ++it ) {
					FocalMechanismPtr fm = FocalMechanism::Cast(*it);
					if ( updateLastAutomaticFM(fm.get()) )
						break;
				}
				it.close();
			}
		}

		setAutomaticOrigin(_lastAutomaticOrigin.get());
		setAutomaticFM(_lastAutomaticFocalMechanism.get());
		updateMap(false);
	}

	if ( _currentEvent )
		setPrefMagnitudeParameter(_currentEvent->preferredMagnitudeID());
	else
		setPrefMagnitudeParameter("");

	updateEventComment();
	updateEventName();
}


void EventSummaryView::updateEventComment() {
	ui.labelCustomValue->setText(_displayEventCommentDefault.c_str());
	ui.labelCustomValue->setToolTip("- no information available -");

	if ( !_currentEvent ) return;
	if ( !_displayEventComment ) return;

	if ( _reader && _currentEvent->commentCount() == 0 )
		_reader->loadComments(_currentEvent.get());

	for ( size_t i = 0; i < _currentEvent->commentCount(); ++i ) {
		if ( _currentEvent->comment(i)->id() == _displayEventCommentID ) {
			if ( !_currentEvent->comment(i)->text().empty() ) {
				ui.labelCustomValue->setText(_currentEvent->comment(i)->text().c_str());
				ui.labelCustomValue->setToolTip(_currentEvent->comment(i)->text().c_str());
			}
			break;
		}
	}
}


void EventSummaryView::updateEventName() {
	if ( !_currentEvent ) return;

	return;

	if ( _reader && _currentEvent->eventDescriptionCount() == 0 )
		_reader->loadEventDescriptions(_currentEvent.get());

	for ( size_t i = 0; i < _currentEvent->eventDescriptionCount(); ++i ) {
		if ( _currentEvent->eventDescription(i)->type() == EARTHQUAKE_NAME ) {
			if ( !_currentEvent->eventDescription(i)->text().empty() ) {
				//ui._lbName->setText(_currentEvent->eventDescription(i)->text().c_str());
				return;
			}
			break;
		}
	}

	//ui._lbName->setText("-");
}


static void elapsedTimeString(const Core::TimeSpan &dt, QString &str)
{
	int d=0, h=0, m=0, s=0;
	QLatin1Char fill('0');
	dt.elapsedTime(&d, &h, &m, &s);
	if (d)
		str = QString("O.T. +%1d %2h").arg(d,2).arg(h, 2, 10, fill);
	else if (h)
		str = QString("O.T. +%1h %2m").arg(h,2).arg(m, 2, 10, fill);
	else
		str = QString("O.T. +%1m %2s").arg(m,2).arg(s, 2, 10, fill);
}


bool EventSummaryView::setOriginParameter(std::string OriginID){
	OriginPtr origin = Origin::Find(OriginID);
	if (!origin && _reader)
		origin = Origin::Cast(_reader->getObject(Origin::TypeInfo(), OriginID));

	if ( origin == NULL ){
		_currentOrigin = NULL;
		_currentFocalMechanism = NULL;
		SEISCOMP_DEBUG("scesv: setOriginParameter:  origin not found %s ", OriginID.c_str());
		clearOriginParameter();
		clearMap();
	 	clearMagnitudeParameter();
		return false;
	}

	if ( !origin->arrivalCount() && _reader )
		_reader->loadArrivals(origin.get());

	if ( !origin->magnitudeCount() && _reader ) {
		// We do not need the station magnitude references
		//_reader->loadMagnitudes(_currentOrigin.get());
		DatabaseIterator it = _reader->getObjects(origin.get(), Magnitude::TypeInfo());
		while ( *it ) {
			origin->add(Magnitude::Cast(*it));
			++it;
		}
	}

	setOrigin(origin.get());
	return true;
}


void EventSummaryView::setOrigin(Seiscomp::DataModel::Origin* origin) {
	_currentOrigin = origin;

	calcOriginDistances();

	std::string desc = description(_currentOrigin.get());
	if ( desc.empty() )
		//desc = _currentEvent->description();
		ui._lbRegionExtra->setVisible(false);
	else {
		ui._lbRegionExtra->setVisible(true);
		ui._lbRegionExtra->setText(desc.c_str());
	}

	ui._lbRegion->setVisible(true);
	std::string region = _currentEvent?eventRegion(_currentEvent.get()):"";
	if ( _currentEvent && !region.empty() )
		ui._lbRegion->setText(region.c_str());
	else {
		Regions regions;
		ui._lbRegion->setText(regions.getRegionName(_currentOrigin->latitude(), _currentOrigin->longitude()).c_str());
	}

	updateTimeAgoLabel();

	// set "not preferred" magnitudes
	setMagnitudeParameter(_currentOrigin.get());

	// set origin parameters
	uiHypocenter._lbLatitude->setText(latitudeToString(_currentOrigin->latitude(), true, false, SCScheme.precision.location));
	uiHypocenter._lbLatitudeUnit->setText(latitudeToString(_currentOrigin->latitude(), false, true));

	uiHypocenter._lbLongitude->setText(longitudeToString(_currentOrigin->longitude(), true, false, SCScheme.precision.location));
	uiHypocenter._lbLongitudeUnit->setText(longitudeToString(_currentOrigin->longitude(), false, true));

	try { // depth error
		double err_z = quantityUncertainty(_currentOrigin->depth());
		if (err_z == 0.0)
			uiHypocenter._lbDepthError->setText(QString("  fixed"));
		else
			uiHypocenter._lbDepthError->setText(QString("+/-%1 km").arg(err_z, 4, 'f', SCScheme.precision.uncertainties));
	}
	catch(...) {
		uiHypocenter._lbDepthError->setText(QString(""));
	}
	try { // depth
		uiHypocenter._lbDepth->setText(depthToString(_currentOrigin->depth(), SCScheme.precision.depth));
		ui.labelDepth->setText(depthToString(_currentOrigin->depth(), SCScheme.precision.depth) + " km");
		uiHypocenter._lbDepthUnit->setText("km");

	} catch(...) {
		uiHypocenter._lbDepth->setText(QString("---"));
		uiHypocenter._lbDepthUnit->setText("");
		uiHypocenter._lbDepthError->setText(QString(""));
		ui.labelDepth->setText("");
	}


	timeToLabel(ui._lbOriginTime, _currentOrigin->time().value(), "%F %T", true);

	try {
		uiHypocenter._lbOriginStatus->setText(_currentOrigin->evaluationMode().toString());
	} catch(...){
		uiHypocenter._lbOriginStatus->setText("---");
	}

	try {
		uiHypocenter._lbLatError->setText(QString("+/-%1 km").arg(quantityUncertainty(_currentOrigin->latitude()), 4, 'f', SCScheme.precision.uncertainties));
	}
	catch ( ... ) {
		uiHypocenter._lbLatError->setText("");
	}

	try {
		uiHypocenter._lbLongError->setText(QString("+/-%1 km").arg(quantityUncertainty(_currentOrigin->longitude()), 4, 'f', SCScheme.precision.uncertainties));
	}
	catch ( ... ) {
		uiHypocenter._lbLongError->setText("");
	}

	try {
		DataModel::OriginQuality quality = _currentOrigin->quality();
		try{
			uiHypocenter._lbNoPhases->setText(QString("%1").arg(quality.usedPhaseCount(), 0, 'd', 0, ' '));
		}
		catch(Core::ValueException&) {
			uiHypocenter._lbNoPhases->setText("--");
		}

		try {
			uiHypocenter._lbRMS->setText(QString("%1").arg(quality.standardError(), 0, 'f', 1));
		}
		catch(Core::ValueException&) {
			uiHypocenter._lbRMS->setText("--");
		}
	}
	catch ( ... ) {
		uiHypocenter._lbNoPhases->setText("--");
		uiHypocenter._lbRMS->setText("--");
	}

	if ( _displayComment ) {
		if ( _reader && _currentOrigin->commentCount() == 0 )
			_reader->loadComments(_currentOrigin.get());

		uiHypocenter._lbComment->setText(_displayCommentDefault.c_str());
		for ( size_t i = 0; i < _currentOrigin->commentCount(); ++i ) {
			if ( _currentOrigin->comment(i)->id() == _displayCommentID ) {
				uiHypocenter._lbComment->setText(_currentOrigin->comment(i)->text().c_str());
				break;
			}
		}
	}

	try {
		uiHypocenter._lbAgency->setText(_currentOrigin->creationInfo().agencyID().c_str());
	}
	catch ( ... ) {
		uiHypocenter._lbAgency->setText("");
	}

	// get the time of first location of an origin belonging to this Event
	QString str("-");
	try {
		if ( _currentEvent ) {
			Core::TimeSpan dt = _currentEvent->creationInfo().creationTime() - _currentOrigin->time().value();
			elapsedTimeString(dt, str);
		}
	}
	catch (...) {}
	uiHypocenter._lbFirstLocation->setText(str);

	str = "-";
	// get the time of the current location
	try {
		Core::TimeSpan dt = _currentOrigin->creationInfo().creationTime() - _currentOrigin->time().value();
		elapsedTimeString(dt, str);
	}
	catch (...) {}
	uiHypocenter._lbThisLocation->setText(str);

	if ( _currentEvent )
		uiHypocenter._lbEventID->setText(_currentEvent->publicID().c_str());
	else
		uiHypocenter._lbEventID->setText("");

	// set map
	if ( !_mapTimer->isActive() ){
		SEISCOMP_DEBUG("updating map ...");
		updateMap(true);
		_mapTimer->start(2000); //! minimum time between two map updates
		disconnect (_mapTimer, 0, this, 0);
	}
	else{
		SEISCOMP_WARNING("updating map deferred!");
		connect (_mapTimer, SIGNAL(timeout()), this, SLOT(deferredMapUpdate()));
	}


	if ( _currentFocalMechanism ) {
		setFMParametersVisible(true);
		setFM(_currentFocalMechanism.get());
		setAutomaticFM(_currentFocalMechanism.get());
	}
	else
		setFMParametersVisible(false);


	ui.btnShowDetails->setEnabled(true);
	ui.btnPlugable0->setEnabled(true);
	ui.btnPlugable1->setEnabled(true);

	SEISCOMP_DEBUG("***** Setting origin %s", _currentOrigin->publicID().c_str());
}


void EventSummaryView::setAutomaticOrigin(DataModel::Origin* origin) {
	if ( origin == NULL ) {
		clearAutomaticOriginParameter();
		return;
	}

	if ( origin->magnitudeCount() == 0 && _reader )
		_reader->loadMagnitudes(origin);

	if ( _currentOrigin && _currentOrigin->publicID() == origin->publicID() ) {
		ui.btnSwitchToAutomatic->setEnabled(false);
		setLastAutomaticOriginColor(_automaticOriginDisabledColor);
	}
	else {
		ui.btnSwitchToAutomatic->setEnabled(true);
		setLastAutomaticOriginColor(_automaticOriginEnabledColor);
	}

	setAutomaticMagnitudeParameter(origin);

	// set origin parameters
	uiHypocenter._lbLatitudeAutomatic->setText(latitudeToString(origin->latitude(), true, false, SCScheme.precision.location));
	uiHypocenter._lbLatitudeUnitAutomatic->setText(latitudeToString(origin->latitude(), false, true));

	uiHypocenter._lbLongitudeAutomatic->setText(longitudeToString(origin->longitude(), true, false, SCScheme.precision.location));
	uiHypocenter._lbLongitudeUnitAutomatic->setText(longitudeToString(origin->longitude(), false, true));

	try { // depth error
		double err_z = quantityUncertainty(origin->depth());
		if (err_z == 0.0)
			uiHypocenter._lbDepthErrorAutomatic->setText(QString("  fixed"));
		else
			uiHypocenter._lbDepthErrorAutomatic->setText(QString("+/-%1 km").arg(err_z, 4, 'f', SCScheme.precision.uncertainties));
	}
	catch(...) {
		uiHypocenter._lbDepthErrorAutomatic->setText(QString(""));
	}
	try { // depth
		uiHypocenter._lbDepthAutomatic->setText(depthToString(origin->depth(), SCScheme.precision.depth));
		uiHypocenter._lbDepthUnitAutomatic->setText("km");

	} catch(...) {
		uiHypocenter._lbDepthAutomatic->setText(QString("---"));
		uiHypocenter._lbDepthUnitAutomatic->setText("");
		uiHypocenter._lbDepthErrorAutomatic->setText(QString(""));
	}


	timeToLabel(ui._lbOriginTimeAutomatic, origin->time().value(), "%F %T", true);

	try {
		uiHypocenter._lbOriginStatusAutomatic->setText(origin->evaluationMode().toString());
	} catch(...){
		uiHypocenter._lbOriginStatusAutomatic->setText("---");
	}

	try {
		uiHypocenter._lbLatErrorAutomatic->setText(QString("+/-%1 km").arg(quantityUncertainty(origin->latitude()), 4, 'f', SCScheme.precision.uncertainties));
	}
	catch ( ... ) {
		uiHypocenter._lbLatErrorAutomatic->setText("");
	}

	try {
		uiHypocenter._lbLongErrorAutomatic->setText(QString("+/-%1 km").arg(quantityUncertainty(origin->longitude()), 4, 'f', SCScheme.precision.uncertainties));
	}
	catch ( ... ) {
		uiHypocenter._lbLongErrorAutomatic->setText("");
	}

	try {
		DataModel::OriginQuality quality = origin->quality();
		try{
			uiHypocenter._lbNoPhasesAutomatic->setText(QString("%1").arg(quality.usedPhaseCount(), 0, 'd', 0, ' '));
		}
		catch(Core::ValueException&) {
			uiHypocenter._lbNoPhasesAutomatic->setText("--");
		}

		try {
			uiHypocenter._lbRMSAutomatic->setText(QString("%1").arg(quality.standardError(), 0, 'f', 1));
		}
		catch(Core::ValueException&) {
			uiHypocenter._lbRMSAutomatic->setText("--");
		}
	}
	catch ( ... ) {
		uiHypocenter._lbNoPhasesAutomatic->setText("--");
		uiHypocenter._lbRMSAutomatic->setText("--");
	}


	if ( _displayComment ) {
		if ( _reader && origin->commentCount() == 0 )
			_reader->loadComments(origin);

		uiHypocenter._lbCommentAutomatic->setText(_displayCommentDefault.c_str());
		for ( size_t i = 0; i < origin->commentCount(); ++i ) {
			if ( origin->comment(i)->id() == _displayCommentID ) {
				uiHypocenter._lbCommentAutomatic->setText(origin->comment(i)->text().c_str());
				break;
			}
		}
	}


	try {
		uiHypocenter._lbAgencyAutomatic->setText(origin->creationInfo().agencyID().c_str());
	}
	catch ( ... ) {
		uiHypocenter._lbAgencyAutomatic->setText("");
	}
}


void EventSummaryView::setFM(DataModel::FocalMechanism *fm) {
	OriginPtr derivedOrigin = NULL;

	try {
		uiHypocenter.labelMisfit->setText(QString("%1").arg(fm->misfit(), 0, 'f', 2));
	}
	catch ( ... ) {
		uiHypocenter.labelMisfit->setText("-");
	}

	try {
		uiHypocenter.labelNP0->setText(
			QString("S: %1, D: %2, R: %3")
			.arg((int)fm->nodalPlanes().nodalPlane1().strike())
			.arg((int)fm->nodalPlanes().nodalPlane1().dip())
			.arg((int)fm->nodalPlanes().nodalPlane1().rake()));
	}
	catch ( ... ) {
		uiHypocenter.labelNP0->setText("S: -, D: -, R: -");
	}

	try {
		uiHypocenter.labelNP1->setText(
			QString("S: %1, D: %2, R: %3")
			.arg((int)fm->nodalPlanes().nodalPlane2().strike())
			.arg((int)fm->nodalPlanes().nodalPlane2().dip())
			.arg((int)fm->nodalPlanes().nodalPlane2().rake()));
	}
	catch ( ... ) {
		uiHypocenter.labelNP1->setText("S: -, D: -, R: -");
	}

	try {
		uiHypocenter.labelAgency->setText(fm->creationInfo().agencyID().c_str());
	}
	catch ( ... ) {
		uiHypocenter.labelAgency->setText("-");
	}

	try {
		uiHypocenter.labelStatus->setText(fm->evaluationMode().toString());
	}
	catch ( ... ) {
		uiHypocenter.labelStatus->setText("-");
	}

	QString str = "-";
	try {
		Core::TimeSpan dt = fm->creationInfo().creationTime() - _currentOrigin->time().value();
		elapsedTimeString(dt, str);
	}
	catch ( ... ) {}
	uiHypocenter.labelThisSolution->setText(str);

	if ( fm->momentTensorCount() > 0 ) {
		MomentTensor *mt = fm->momentTensor(0);
		derivedOrigin = Origin::Find(mt->derivedOriginID());
		if (!derivedOrigin && _reader)
			derivedOrigin = Origin::Cast(_reader->getObject(Origin::TypeInfo(), mt->derivedOriginID()));

		try {
			uiHypocenter.labelCLVD->setText(QString("%1").arg(mt->clvd(), 0, 'f', 2));
		}
		catch ( ... ) {
			uiHypocenter.labelCLVD->setText("-");
		}

		MagnitudePtr mag = Magnitude::Find(mt->momentMagnitudeID());
		if ( !mag && _reader)
			mag = Magnitude::Cast(_reader->getObject(Magnitude::TypeInfo(), mt->momentMagnitudeID()));

		if ( mag )
			uiHypocenter.labelMw->setText(QString("%1").arg(mag->magnitude().value(), 0, 'f', 1));
		else
			uiHypocenter.labelMw->setText("-");

		try {
			uiHypocenter.labelMoment->setText(QString("%1").arg(mt->scalarMoment(), 0, 'E', 2));
		}
		catch ( ... ) {
			uiHypocenter.labelMoment->setText("-");
		}
	}
	else {
		uiHypocenter.labelCLVD->setText("-");
		uiHypocenter.labelMw->setText("-");
		uiHypocenter.labelMoment->setText("-");

		derivedOrigin = Origin::Find(fm->triggeringOriginID());
	}

	if ( derivedOrigin ) {
		uiHypocenter.labelLatitude->setText(latitudeToString(derivedOrigin->latitude(), true, false, SCScheme.precision.location));
		uiHypocenter.labelLatitudeUnit->setText(latitudeToString(derivedOrigin->latitude(), false, true));

		uiHypocenter.labelLongitude->setText(longitudeToString(derivedOrigin->longitude(), true, false, SCScheme.precision.location));
		uiHypocenter.labelLongitudeUnit->setText(longitudeToString(derivedOrigin->longitude(), false, true));

		try {
			uiHypocenter.labelLatitudeError->setText(QString("+/-%1 km").arg(quantityUncertainty(derivedOrigin->latitude()), 4, 'f', SCScheme.precision.depth));
		}
		catch ( ... ) {
			uiHypocenter.labelLatitudeError->setText("");
		}

		try {
			uiHypocenter.labelLongitudeError->setText(QString("+/-%1 km").arg(quantityUncertainty(derivedOrigin->longitude()), 4, 'f', SCScheme.precision.depth));
		}
		catch ( ... ) {
			uiHypocenter.labelLongitudeError->setText("");
		}
		
		try { // depth error
			double err_z = quantityUncertainty(derivedOrigin->depth());
			if (err_z == 0.0)
				uiHypocenter.labelDepthError->setText(QString("  fixed"));
			else
				uiHypocenter.labelDepthError->setText(QString("+/-%1 km").arg(err_z, 4, 'f', SCScheme.precision.depth));
		}
		catch ( ... ) {
			uiHypocenter.labelDepthError->setText(QString(""));
		}

		try { // depth
			uiHypocenter.labelDepth->setText(depthToString(derivedOrigin->depth(), SCScheme.precision.depth));
			uiHypocenter.labelDepthUnit->setText("km");

		}
		catch ( ... ) {
			uiHypocenter.labelDepth->setText(QString("---"));
			uiHypocenter.labelDepthUnit->setText("");
			uiHypocenter.labelDepthError->setText(QString(""));
		}

		try {
			uiHypocenter.labelPhases->setText(
				QString("%1").arg(derivedOrigin->quality().usedPhaseCount()));
		}
		catch ( ... ) {
			uiHypocenter.labelPhases->setText("-");
		}

		try {
			uiHypocenter.labelType->setText(derivedOrigin->type().toString());
		}
		catch ( ... ) {
			uiHypocenter.labelType->setText("-");
		}

		try {
			uiHypocenter.labelMinDist->setText(
				QString("%1").arg(derivedOrigin->quality().minimumDistance(), 0, 'f', 1));
		}
		catch ( ... ) {
			uiHypocenter.labelMinDist->setText("-");
		}

		try {
			uiHypocenter.labelMaxDist->setText(
				QString("%1").arg(derivedOrigin->quality().maximumDistance(), 0, 'f', 1));
		}
		catch ( ... ) {
			uiHypocenter.labelMaxDist->setText("-");
		}
	}
	else {
		uiHypocenter.labelLatitude->setText("---.--");
		uiHypocenter.labelLatitudeUnit->setText("");
		uiHypocenter.labelLongitude->setText("---.--");
		uiHypocenter.labelLongitudeUnit->setText("");
		uiHypocenter.labelDepth->setText("---");
		uiHypocenter.labelDepthUnit->setText("");
		uiHypocenter.labelPhases->setText("-");
		uiHypocenter.labelType->setText("-");
		uiHypocenter.labelMinDist->setText("-");
		uiHypocenter.labelMaxDist->setText("-");
	}
}


void EventSummaryView::clearAutomaticFMParameter() {
	setLastAutomaticFMColor(_automaticOriginDisabledColor);

	uiHypocenter.labelMisfitAutomatic->setText("-");
	uiHypocenter.labelNP0Automatic->setText("S: -, D: -, R: -");
	uiHypocenter.labelNP1Automatic->setText("S: -, D: -, R: -");
	uiHypocenter.labelAgencyAutomatic->setText("-");
	uiHypocenter.labelStatusAutomatic->setText("-");
	uiHypocenter.labelThisSolutionAutomatic->setText("-");
	uiHypocenter.labelCLVDAutomatic->setText("-");
	uiHypocenter.labelMwAutomatic->setText("-");
	uiHypocenter.labelMomentAutomatic->setText("-");
	uiHypocenter.labelLatitudeAutomatic->setText("---.--");
	uiHypocenter.labelLatitudeUnitAutomatic->setText("");
	uiHypocenter.labelLongitudeAutomatic->setText("---.--");
	uiHypocenter.labelLongitudeUnitAutomatic->setText("");
	uiHypocenter.labelDepthAutomatic->setText("---");
	uiHypocenter.labelDepthUnitAutomatic->setText("");
	uiHypocenter.labelPhasesAutomatic->setText("-");
	uiHypocenter.labelTypeAutomatic->setText("-");
}


void EventSummaryView::setAutomaticFM(DataModel::FocalMechanism *fm) {
	if ( fm == NULL ) {
		clearAutomaticFMParameter();
		return;
	}

	if ( _currentFocalMechanism && _currentFocalMechanism->publicID() == fm->publicID() )
		setLastAutomaticFMColor(_automaticOriginDisabledColor);
	else
		setLastAutomaticFMColor(_automaticOriginEnabledColor);

	if ( _reader && fm->momentTensorCount() == 0 )
		_reader->loadMomentTensors(fm);

	OriginPtr derivedOrigin = NULL;

	try {
		uiHypocenter.labelMisfitAutomatic->setText(QString("%1").arg(fm->misfit(), 0, 'f', 2));
	}
	catch ( ... ) {
		uiHypocenter.labelMisfitAutomatic->setText("-");
	}

	try {
		uiHypocenter.labelNP0Automatic->setText(
			QString("S: %1, D: %2, R: %3")
			.arg((int)fm->nodalPlanes().nodalPlane1().strike())
			.arg((int)fm->nodalPlanes().nodalPlane1().dip())
			.arg((int)fm->nodalPlanes().nodalPlane1().rake()));
	}
	catch ( ... ) {
		uiHypocenter.labelNP0Automatic->setText("S: -, D: -, R: -");
	}

	try {
		uiHypocenter.labelNP1Automatic->setText(
			QString("S: %1, D: %2, R: %3")
			.arg((int)fm->nodalPlanes().nodalPlane2().strike())
			.arg((int)fm->nodalPlanes().nodalPlane2().dip())
			.arg((int)fm->nodalPlanes().nodalPlane2().rake()));
	}
	catch ( ... ) {
		uiHypocenter.labelNP1Automatic->setText("S: -, D: -, R: -");
	}

	try {
		uiHypocenter.labelAgencyAutomatic->setText(fm->creationInfo().agencyID().c_str());
	}
	catch ( ... ) {
		uiHypocenter.labelAgencyAutomatic->setText("-");
	}

	try {
		uiHypocenter.labelStatusAutomatic->setText(fm->evaluationMode().toString());
	}
	catch ( ... ) {
		uiHypocenter.labelStatusAutomatic->setText("-");
	}

	QString str = "-";
	try {
		Core::TimeSpan dt = fm->creationInfo().creationTime() - _currentOrigin->time().value();
		elapsedTimeString(dt, str);
	}
	catch ( ... ) {}
	uiHypocenter.labelThisSolutionAutomatic->setText(str);

	if ( fm->momentTensorCount() > 0 ) {
		MomentTensor *mt = fm->momentTensor(0);
		derivedOrigin = Origin::Find(mt->derivedOriginID());
		if (!derivedOrigin && _reader)
			derivedOrigin = Origin::Cast(_reader->getObject(Origin::TypeInfo(), mt->derivedOriginID()));

		try {
			uiHypocenter.labelCLVDAutomatic->setText(QString("%1").arg(mt->clvd(), 0, 'f', 2));
		}
		catch ( ... ) {
			uiHypocenter.labelCLVDAutomatic->setText("-");
		}

		MagnitudePtr mag = Magnitude::Find(mt->momentMagnitudeID());
		if ( !mag && _reader)
			mag = Magnitude::Cast(_reader->getObject(Magnitude::TypeInfo(), mt->momentMagnitudeID()));

		if ( mag )
			uiHypocenter.labelMwAutomatic->setText(QString("%1").arg(mag->magnitude().value(), 0, 'f', 1));
		else
			uiHypocenter.labelMwAutomatic->setText("-");

		try {
			uiHypocenter.labelMomentAutomatic->setText(QString("%1").arg(mt->scalarMoment(), 0, 'E', 2));
		}
		catch ( ... ) {
			uiHypocenter.labelMomentAutomatic->setText("-");
		}
	}
	else {
		uiHypocenter.labelCLVDAutomatic->setText("-");
		uiHypocenter.labelMwAutomatic->setText("-");
		uiHypocenter.labelMomentAutomatic->setText("-");
	}

	if ( derivedOrigin ) {
		uiHypocenter.labelLatitudeAutomatic->setText(latitudeToString(derivedOrigin->latitude(), true, false, SCScheme.precision.location));
		uiHypocenter.labelLatitudeUnitAutomatic->setText(latitudeToString(derivedOrigin->latitude(), false, true));

		uiHypocenter.labelLongitudeAutomatic->setText(longitudeToString(derivedOrigin->longitude(), true, false, SCScheme.precision.location));
		uiHypocenter.labelLongitudeUnitAutomatic->setText(longitudeToString(derivedOrigin->longitude(), false, true));

		try {
			uiHypocenter.labelLatitudeErrorAutomatic->setText(QString("+/-%1 km").arg(quantityUncertainty(derivedOrigin->latitude()), 4, 'f', SCScheme.precision.uncertainties));
		}
		catch ( ... ) {
			uiHypocenter.labelLatitudeErrorAutomatic->setText("");
		}

		try {
			uiHypocenter.labelLongitudeErrorAutomatic->setText(QString("+/-%1 km").arg(quantityUncertainty(derivedOrigin->longitude()), 4, 'f', SCScheme.precision.uncertainties));
		}
		catch ( ... ) {
			uiHypocenter.labelLongitudeErrorAutomatic->setText("");
		}
		
		try { // depth error
			double err_z = quantityUncertainty(derivedOrigin->depth());
			if (err_z == 0.0)
				uiHypocenter.labelDepthErrorAutomatic->setText(QString("  fixed"));
			else
				uiHypocenter.labelDepthErrorAutomatic->setText(QString("+/-%1 km").arg(err_z, 4, 'f', SCScheme.precision.uncertainties));
		}
		catch ( ... ) {
			uiHypocenter.labelDepthErrorAutomatic->setText(QString(""));
		}

		try { // depth
			uiHypocenter.labelDepthAutomatic->setText(depthToString(derivedOrigin->depth(), SCScheme.precision.depth));
			uiHypocenter.labelDepthUnitAutomatic->setText("km");

		}
		catch ( ... ) {
			uiHypocenter.labelDepthAutomatic->setText(QString("---"));
			uiHypocenter.labelDepthUnitAutomatic->setText("");
			uiHypocenter.labelDepthErrorAutomatic->setText(QString(""));
		}

		try {
			uiHypocenter.labelPhasesAutomatic->setText(
				QString("%1").arg(derivedOrigin->quality().usedPhaseCount()));
		}
		catch ( ... ) {
			uiHypocenter.labelPhasesAutomatic->setText("-");
		}

		try {
			uiHypocenter.labelTypeAutomatic->setText(derivedOrigin->type().toString());
		}
		catch ( ... ) {
			uiHypocenter.labelTypeAutomatic->setText("-");
		}

		try {
			uiHypocenter.labelMinDistAutomatic->setText(
				QString("%1").arg(derivedOrigin->quality().minimumDistance(), 0, 'f', 1));
		}
		catch ( ... ) {
			uiHypocenter.labelMinDistAutomatic->setText("-");
		}

		try {
			uiHypocenter.labelMaxDistAutomatic->setText(
				QString("%1").arg(derivedOrigin->quality().maximumDistance(), 0, 'f', 1));
		}
		catch ( ... ) {
			uiHypocenter.labelMaxDist->setText("-");
		}

	}
	else {
		uiHypocenter.labelLatitudeAutomatic->setText("---.--");
		uiHypocenter.labelLatitudeUnitAutomatic->setText("");
		uiHypocenter.labelLongitudeAutomatic->setText("---.--");
		uiHypocenter.labelLongitudeUnitAutomatic->setText("");
		uiHypocenter.labelDepthAutomatic->setText("---");
		uiHypocenter.labelDepthUnitAutomatic->setText("");
		uiHypocenter.labelPhasesAutomatic->setText("-");
		uiHypocenter.labelTypeAutomatic->setText("-");
		uiHypocenter.labelMinDistAutomatic->setText("-");
		uiHypocenter.labelMaxDistAutomatic->setText("-");
	}
}


void EventSummaryView::deferredMapUpdate(){

	disconnect (_mapTimer, 0, this, 0);
	SEISCOMP_DEBUG("processing deferred map update...");
	updateMap(true);
	_mapTimer->start(2000); //! minimum time between two map updates

}


void EventSummaryView::updateMap(bool realignView){
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	if ( _currentOrigin && !_currentOrigin->arrivalCount() && _reader )
		_reader->loadArrivals(_currentOrigin.get());

	_map->setOrigin(_currentOrigin.get());

	if ( _currentOrigin && realignView ) {
		if ( _recenterMap ) {
			double radius = 30;
			try { radius = std::min(radius, _currentOrigin->quality().maximumDistance()+0.1); }
			catch ( ... ) {}
			_map->canvas().displayRect(QRectF(_currentOrigin->longitude()-radius, _currentOrigin->latitude()-radius, radius*2, radius*2));
		}
		else {
			if ( !_map->canvas().isVisible(_currentOrigin->longitude(), _currentOrigin->latitude()) ) {
				_map->canvas().setView(QPointF(_currentOrigin->longitude(), _currentOrigin->latitude()), _map->canvas().zoomLevel());
			}
		}
	}

	if ( _displayFocMechs ) {
		if ( _currentFocalMechanism )
			showFocalMechanism(_currentFocalMechanism.get(), -80, -80, palette().color(QPalette::WindowText));

		// Only show the last automatic solution if there is a preferred
		// solution and if both are different
		if ( _lastAutomaticFocalMechanism && _currentFocalMechanism &&
			 _lastAutomaticFocalMechanism != _currentFocalMechanism )
			showFocalMechanism(_lastAutomaticFocalMechanism.get(), 80, -80, _automaticOriginEnabledColor);
	}

	_map->update();

	QApplication::restoreOverrideCursor();
}


void EventSummaryView::showFocalMechanism(DataModel::FocalMechanism *fm,
                                          int ofsX, int ofsY, QColor borderColor) {
	Math::NODAL_PLANE np;
	Math::Tensor2Sd tensor;
	bool hasTensor = false;
	DataModel::OriginPtr derivedOrigin;

	QColor c = Qt::black;
	if ( fm->momentTensorCount() > 0 ) {
		DataModel::MomentTensor *mt = fm->momentTensor(0);

		if ( _enableFullTensor ) {
			try {
				tensor._33 = +mt->tensor().Mrr();
				tensor._11 = +mt->tensor().Mtt();
				tensor._22 = +mt->tensor().Mpp();
				tensor._13 = +mt->tensor().Mrt();
				tensor._23 = -mt->tensor().Mrp();
				tensor._12 = -mt->tensor().Mtp();

				hasTensor = true;
			}
			catch ( ... ) {}
		}

		derivedOrigin = DataModel::Origin::Find(mt->derivedOriginID());
		if ( !derivedOrigin && _reader )
			derivedOrigin = Origin::Cast(_reader->getObject(Origin::TypeInfo(), mt->derivedOriginID()));
	}
	else {
		derivedOrigin = DataModel::Origin::Find(fm->triggeringOriginID());
		if ( !derivedOrigin && _reader )
			derivedOrigin = Origin::Cast(_reader->getObject(Origin::TypeInfo(), fm->triggeringOriginID()));
	}

	if ( derivedOrigin ) {
		try {
			float depth = derivedOrigin->depth().value();

			if ( depth < 50 )
				c = Qt::red;
			else if ( depth < 100 )
				c = QColor(255, 165, 0);
			else if ( depth < 250 )
				c = Qt::yellow;
			else if ( depth < 600 )
				c = Qt::green;
			else
				c = Qt::blue;
		}
		catch ( ... ) {}
	}

	try {
		if ( !hasTensor ) {
			np.str = fm->nodalPlanes().nodalPlane1().strike();
			np.dip = fm->nodalPlanes().nodalPlane1().dip();
			np.rake = fm->nodalPlanes().nodalPlane1().rake();

			Math::np2tensor(np, tensor);
		}
	}
	catch ( ... ) {
		return;
	}

	try {
		TensorSymbol *symbol = new TensorSymbol(tensor);
		symbol->setSize(QSize(64, 64));
		if ( derivedOrigin )
			symbol->setPosition(QPointF(derivedOrigin->longitude(), derivedOrigin->latitude()));
		else
			symbol->setPosition(QPointF(_currentOrigin->longitude(), _currentOrigin->latitude()));
		symbol->setOffset(QPoint(ofsX, ofsY));
		symbol->setPriority(Map::Symbol::HIGH);
		symbol->setShadingEnabled(true);
		symbol->setTColor(c);
		symbol->setBorderColor(borderColor);
		_map->canvas().symbolCollection()->add(symbol);
	}
	catch ( ... ) {}
}


void EventSummaryView::updateMagnitude(DataModel::Magnitude *mag) {
	_magList->updateMag(mag);
}


void EventSummaryView::setPrefMagnitudeParameter(std::string MagnitudeID){
	DataModel::MagnitudePtr Magnitude = Magnitude::Find(MagnitudeID);
	if (!Magnitude && _reader)
		Magnitude = DataModel::Magnitude::Cast(_reader->getObject(DataModel::Magnitude::TypeInfo(), MagnitudeID));

	if (Magnitude == NULL){
		clearPrefMagnitudeParameter();
		emit showInStatusbar(QString("no magnitude %1").arg(MagnitudeID.c_str()), 1000);
		return;
	}

	ui._lbPreMagType->setText((Magnitude->type()).c_str());
	double premagval = Magnitude->magnitude().value();
	char buf[10] = "-";
	if (premagval<12)
		sprintf(buf, "%.1f", premagval);
	ui._lbPreMagVal->setText(buf);

	_magList->selectMagnitude(MagnitudeID.c_str());

	return;
}


void EventSummaryView::setMagnitudeParameter(DataModel::Origin* origin){

	// clear all mag parameters incl. buttons
	clearMagnitudeParameter();

	for ( size_t i = 0; i < origin->magnitudeCount(); ++i ) {

		// if preferred Magnitude --> text style BOLD
		bool pref = false;
		if (origin->magnitude(i)->publicID() == _currentEvent->preferredMagnitudeID() )
			pref = true;

		bool typeEnabled =
			(_visibleMagnitudes.find(origin->magnitude(i)->type()) != _visibleMagnitudes.end())
			|| ui.actionShowInvisibleMagnitudes->isChecked();

		// create new magnitudeList display row
		_magList->addMag(origin->magnitude(i), pref, typeEnabled);
	}
}


void EventSummaryView::setAutomaticMagnitudeParameter(DataModel::Origin* origin) {
	for ( int i = 0; i < _magList->rowCount(); ++i )
		_magList->rowAt(i)->setReferenceMagnitude(NULL);

	if ( !origin ) return;

	for ( size_t i = 0; i < origin->magnitudeCount(); ++i ) {
		Magnitude *mag = origin->magnitude(i);

		MagRow *row = _magList->row(mag->type());
		if ( row )
			row->setReferenceMagnitude(mag);
	}
}


void EventSummaryView::clearPrefMagnitudeParameter(){

	// clear preferred magnitude parameter
	ui._lbPreMagType->setText("--");
	ui._lbPreMagVal->setText("-.-");

}


void EventSummaryView::clearMagnitudeParameter(){
	// clear magnitudeList display rows
	_magList->reset();

	for ( std::set<std::string>::iterator it = _visibleMagnitudes.begin();
	      it != _visibleMagnitudes.end(); ++it ) {
		_magList->addMag(*it, false, true);
	}

	ui._lbPreMagType->setText("");
	ui._lbPreMagVal->setText("");
}


void EventSummaryView::clearOriginParameter(){
	ui.labelDepth->setText("");
	uiHypocenter._lbAgency->setText("");
	uiHypocenter._lbFirstLocation->setText("");
	uiHypocenter._lbThisLocation->setText("");
	uiHypocenter._lbEventID->setText("");

	uiHypocenter._lbLatitude->setText("---.--");
	uiHypocenter._lbLatitudeUnit->setText("");
	uiHypocenter._lbLongitude->setText("---.--");
	uiHypocenter._lbLongitudeUnit->setText("");
	uiHypocenter._lbDepth->setText("---");
	uiHypocenter._lbDepthUnit->setText("---");

	uiHypocenter._lbNoPhases->setText("--");
	uiHypocenter._lbRMS->setText("--");
//	uiHypocenter._lbMinDist->setText("--");

	uiHypocenter._lbLatError->setText(QString("+/-%1 km").arg(0.0, 6, 'f', 0));
	uiHypocenter._lbLongError->setText(QString("+/-%1 km").arg(0.0, 6, 'f', 0));
	uiHypocenter._lbDepthError->setText(QString("+/-%1 km").arg(0.0, 6, 'f', 0));

	ui._lbOriginTime->setText("0000/00/00  00:00:00");
	ui._lbTimeAgo->setVisible(false);

	ui._lbRegion->setText(""); ui._lbRegion->setVisible(false);
	ui._lbRegionExtra->setText(""); ui._lbRegionExtra->setVisible(false);
	uiHypocenter._lbOriginStatus->setText("");

	clearAutomaticOriginParameter();

	ui.btnShowDetails->setEnabled(false);
	ui.btnPlugable0->setEnabled(false);
	ui.btnPlugable1->setEnabled(false);

	ui.btnSwitchToAutomatic->setEnabled(false);

	setFMParametersVisible(false);

	if ( _map )
		_map->canvas().setSelectedCity(NULL);
}


void EventSummaryView::clearAutomaticOriginParameter() {
	setLastAutomaticOriginColor(_automaticOriginDisabledColor);

	ui._lbOriginTimeAutomatic->setText("0000/00/00  00:00:00");
	uiHypocenter._lbLatitudeAutomatic->setText("---.--");
	uiHypocenter._lbLatitudeUnitAutomatic->setText("");
	uiHypocenter._lbLongitudeAutomatic->setText("---.--");
	uiHypocenter._lbLongitudeUnitAutomatic->setText("");
	uiHypocenter._lbDepthAutomatic->setText("---");
	uiHypocenter._lbDepthUnitAutomatic->setText("");
	uiHypocenter._lbNoPhasesAutomatic->setText("--");
	uiHypocenter._lbRMSAutomatic->setText("--");
	uiHypocenter._lbCommentAutomatic->setText(_displayCommentDefault.c_str());
	uiHypocenter._lbLatErrorAutomatic->setText(QString("+/-%1 km").arg(0.0, 6, 'f', 0));
	uiHypocenter._lbLongErrorAutomatic->setText(QString("+/-%1 km").arg(0.0, 6, 'f', 0));
	uiHypocenter._lbDepthErrorAutomatic->setText(QString("+/-%1 km").arg(0.0, 6, 'f', 0));
	uiHypocenter._lbOriginStatusAutomatic->setText("");
	uiHypocenter._lbAgencyAutomatic->setText("");

	setAutomaticMagnitudeParameter(NULL);
}


void EventSummaryView::clearMap(){

	if (_map){
		_map->canvas().symbolCollection()->clear();
		_map->canvas().displayRect(QRectF(-180.0, -90.0, 360.0, 180.0));
		_map->update();
	}

}


bool EventSummaryView::updateLastAutomaticOrigin(DataModel::Origin *origin) {
	try {
		if ( origin->evaluationMode() != AUTOMATIC ) return false;
	}
	catch ( ... ) {}

	if ( !_lastAutomaticOrigin ) {
		_lastAutomaticOrigin = origin;
		return true;
	}

	Core::Time created;
	try {
		created = origin->creationInfo().creationTime();
	}
	catch ( ... ) {
		return false;
	}

	try {
		if ( created > _lastAutomaticOrigin->creationInfo().creationTime() ) {
			_lastAutomaticOrigin = origin;
			return true;
		}
	}
	catch ( ... ) {}

	return false;
}


bool EventSummaryView::updateLastAutomaticFM(DataModel::FocalMechanism *fm) {
	try {
		if ( fm->evaluationMode() != AUTOMATIC ) return false;
	}
	catch ( ... ) {}

	if ( !_lastAutomaticFocalMechanism ) {
		_lastAutomaticFocalMechanism = fm;
		return true;
	}

	Core::Time created;
	try {
		created = fm->creationInfo().creationTime();
	}
	catch ( ... ) {
		return false;
	}

	try {
		if ( created > _lastAutomaticFocalMechanism->creationInfo().creationTime() ) {
			_lastAutomaticFocalMechanism = fm;
			return true;
		}
	}
	catch ( ... ) {}

	return false;
}


void EventSummaryView::updateTimeAgoLabel(){

// 	emit showInStatusbar(QString("%1").arg(Core::BaseObject::ObjectCount(), 0, 'd', 0, ' '), 0);

	if (!_currentOrigin){
		ui._lbTimeAgo->setVisible(false);
		return;
	}

	if ( _map && _map->waveformPropagation() )
		_map->update();

	Core::TimeSpan ts;
	Core::Time ct;

	ct.gmt();
	ts = ct - _currentOrigin->time();

	if ( !ui._lbTimeAgo->isVisible() )
		ui._lbTimeAgo->setVisible(true);

	int sec = ts.seconds();
	int days = sec / 86400;
	int hours = (sec - days*86400) / 3600;
	int minutes = (sec - days*86400 - hours*3600) / 60;
	int seconds = sec - days*86400 - hours*3600 - 60*minutes;

	QString text;

	if (days>0)
		text = QString("%1 days and %2 hours ago").arg(days, 0, 'd', 0, ' ').arg(hours, 0, 'd', 0, ' ');
	else if ((days==0)&&(hours>0))
		text = QString("%1 hours and %2 minutes ago").arg(hours, 0, 'd', 0, ' ').arg(minutes, 0, 'd', 0, ' ');
	else if ((days==0)&&(hours==0)&&(minutes>0)) {
		if ( _maxMinutesSecondDisplay >= 0 && minutes > _maxMinutesSecondDisplay )
			text = QString("%1 minutes").arg(minutes, 0, 'd', 0, ' ');
		else
			text = QString("%1 minutes and %2 seconds ago").arg(minutes, 0, 'd', 0, ' ').arg(seconds, 0, 'd', 0, ' ');
	}
	else if ((days==0)&&(hours==0)&&(minutes==0)&&(seconds>0))
		text = QString("%1 seconds ago").arg(seconds, 0, 'd', 0, ' ');

	if ( text != ui._lbTimeAgo->text() )
		ui._lbTimeAgo->setText(text);

	/*
	double tsd = ts;

	while ( _originStationsIndex < _originStations.size() ) {
		if ( _originStations[_originStationsIndex].second > tsd )
			break;

		++_originStationsIndex;
	}

	if ( _originStationsIndex >= _originStations.size() )
		return;

	std::cout << "--------" << std::endl;
	std::cout << "Arrivals: " << _currentOrigin->arrivalCount() << std::endl;
	std::cout << "Passed picks: " << _originStationsIndex << std::endl;
	std::cout << "Awaited picks: " << _originStations.size() - _originStationsIndex << std::endl;
	*/
}


void EventSummaryView::drawStations(bool enable) {
	_map->setDrawStations(enable);
	_map->update();
}


void EventSummaryView::drawBeachballs(bool enable) {
	if ( _displayFocMechs == enable ) return;
	_displayFocMechs = enable;
	updateMap(false);
}


void EventSummaryView::drawFullTensor(bool enable) {
	if ( _enableFullTensor == enable ) return;
	_enableFullTensor = enable;
	updateMap(false);
}


void EventSummaryView::setWaveformPropagation(bool enable) {
	_map->setWaveformPropagation(enable);
}


void EventSummaryView::setAutoSelect(bool s) {
	_autoSelect = s;
}


void EventSummaryView::setInteractiveEnabled(bool e) {
	_interactive = e;

	ui.frameProcessing->setVisible(_interactive);
	//ui.btnShowDetails->setVisible(_interactive);

	setScript0(_script0, _scriptStyle0);
	setScript1(_script1, _scriptStyle1);
}


void EventSummaryView::runScript(const QString& script, const QString& name, bool oldStyle) {
	if ( QMessageBox::question(this, "Run action",
	                           tr("Do you really want to continue (%1)?").arg(name),
	                           QMessageBox::Yes, QMessageBox::No) == QMessageBox::No )
		return;

	QString cmd;

	if ( oldStyle ) {
		Magnitude *nm = Magnitude::Find(_currentEvent->preferredMagnitudeID());

		std::string extraDescription = _currentOrigin?description(_currentOrigin.get()):"";

		cmd = QString("%1 %2 %3 %4 \"%5\"")
		      .arg(script)
		      .arg(_currentEvent->publicID().c_str())
		      .arg(_currentOrigin->arrivalCount())
		      .arg(nm?QString("%1").arg(nm->magnitude().value(), 0, 'f', 1):"")
		      .arg(extraDescription.c_str());
	}
	else {
		cmd = QString("%1 %2 \"%3\" \"%4\" \"%5\"")
		      .arg(script)
		      .arg(_currentEvent->publicID().c_str())
		      .arg(_currentEvent->preferredOriginID().c_str())
		      .arg(_currentEvent->preferredMagnitudeID().c_str())
		      .arg(_currentEvent->preferredFocalMechanismID().c_str());
	}

	QString command = QString(cmd);
	SEISCOMP_DEBUG("%s", qPrintable(cmd));
	// start as background process w/o any communication channel
	if( !QProcess::startDetached(command) ) {
		QMessageBox::warning(this, "Export event", tr("Can't execute script"));
	}
}


void EventSummaryView::switchToAutomaticPressed() {
	if ( _currentEvent == NULL ) return;

	JournalEntryPtr entry = new JournalEntry;
	entry->setObjectID(_currentEvent->publicID());
	entry->setAction("EvPrefOrgEvalMode");
	entry->setParameters("automatic");
	entry->setSender(SCApp->name() + "@" + Core::getHostname());
	entry->setCreated(Core::Time::GMT());

	NotifierPtr n = new Notifier("Journaling", OP_ADD, entry.get());
	NotifierMessagePtr nm = new NotifierMessage;
	nm->attach(n.get());
	SCApp->sendMessage(SCApp->messageGroups().event.c_str(), nm.get());
}


void EventSummaryView::runScript0() {
	runScript(_script0.c_str(), ui.btnPlugable0->text(), _scriptStyle0);
}

void EventSummaryView::runScript1() {
	runScript(_script1.c_str(), ui.btnPlugable1->text(), _scriptStyle1);
}

std::string EventSummaryView::description(Origin* origin) const {
	double dist, azi;
	const Math::Geo::CityD* coord =
		Math::Geo::nearestCity(origin->latitude(), origin->longitude(), _maxHotspotDist, _minHotspotPopulation,
		                       SCApp->cities(), &dist, &azi);

	if ( _map ) _map->canvas().setSelectedCity(coord);

	if ( !coord )
		return "";

	dist = (int)Math::Geo::deg2km(dist);
	std::string dir;

	if ( azi < 22.5 || azi > 360.0-22.5 )
		dir = "N";
	else if ( azi >= 22.5 && azi <= 90.0-22.5 )
		dir = "NE";
	else if ( azi > 90.0-22.5 && azi < 90.0+22.5 )
		dir = "E";
	else if ( azi >= 90.0+22.5 && azi <= 180.0-22.5 )
		dir = "SE";
	else if ( azi > 180.0-22.5 && azi < 180.0+22.5 )
		dir = "S";
	else if ( azi >= 180.0+22.5 && azi <= 270.0-22.5 )
		dir = "SW";
	else if ( azi > 270.0-22.5 && azi < 270.0+22.5 )
		dir = "W";
	else if ( azi >= 270.0+22.5 && azi <= 360.0-22.5 )
		dir = "NW";
	else
		dir = "?";

	return Util::replace(_hotSpotDescriptionPattern,
	                      PoiResolver(dist, dir, coord->name(), origin->latitude(), origin->longitude()));
}


bool EventSummaryView::checkAndDisplay(DataModel::Event *e) {
	// If no current event set display the requested events
	if ( !_currentEvent ) {
		processEventMsg(e);
		return true;
	}

	// If no constraints in terms of origin time exists,
	// display the requested event
	if ( !_showOnlyMostRecentEvent ) {
		processEventMsg(e);
		return true;
	}

	// If current origin is empty, display the event
	if ( !_currentOrigin ) {
		processEventMsg(e);
		return true;
	}

	// Otherwise check the origin time
	if ( _currentEvent->publicID() != e->publicID() ) {
		OriginPtr o = Origin::Find(e->preferredOriginID());
		if ( !o && _reader )
			o = Origin::Cast(_reader->getObject(Origin::TypeInfo(), e->preferredOriginID()));

		if ( !o ) return false;

		try {
			if ( o->time().value() < _currentOrigin->time().value() )
				return false;
		}
		catch ( ... ) {
			return false;
		}
	}

	processEventMsg(e);
	return true;
}


void EventSummaryView::calcOriginDistances() {
	/*
	_originStations.clear();
	_originStationsIndex = 0;

	if ( !_currentOrigin ) return;

	double depth = 0;
	try { depth = _currentOrigin->depth(); } catch (...) {}

	Client::Inventory *inv = Client::Inventory::Instance();
	if ( inv == NULL ) return;

	Inventory *inventory = inv->inventory();
	if ( inventory == NULL ) return;

	TravelTimeTable ttt;

	for ( size_t ni = 0; ni < inventory->networkCount(); ++ni ) {
		Network *n = inventory->network(ni);
		try { if ( n->end() ) continue; } catch (...) {}

		for ( size_t si = 0; si < n->stationCount(); ++si ) {
			Station *s = n->station(si);
			try { if ( s->end() ) continue; } catch (...) {}

			try {
				double lat = s->latitude();
				double lon = s->longitude();

				TravelTime tt =
				ttt.computeFirst(_currentOrigin->latitude(), _currentOrigin->longitude(), depth,
				                 lat, lon);

				_originStations.push_back(StationDistances::value_type(s, tt.time));
			}
			catch (... ) {}
		}
	}

	std::sort(_originStations.begin(), _originStations.end(), lessThan);
	*/
}


void EventSummaryView::setLastAutomaticOriginColor(QColor c) {
	if ( _automaticOriginColor == c ) return;

	SET_COLOR(ui._lbOriginTimeAutomatic, c);
	SET_COLOR(uiHypocenter.frameInformationAutomatic, c);

	_magList->setReferenceMagnitudesColor(c);

	_automaticOriginColor = c;
}


void EventSummaryView::setLastAutomaticFMColor(QColor c) {
	if ( _automaticFMColor == c ) return;

	SET_COLOR(uiHypocenter.fmFrameInformationAutomatic, c);

	_automaticFMColor = c;
}


void EventSummaryView::setLastAutomaticOriginVisible(bool v) {
	uiHypocenter.frameInformationAutomatic->setVisible(v);
	uiHypocenter.labelFrameInfoSpacer->setVisible(v);
	ui._lbOriginTimeAutomatic->setVisible(v);
	_magList->setReferenceMagnitudesVisible(v);
}


void EventSummaryView::setFMParametersVisible(bool v) {
	uiHypocenter.labelFMSeparator->setVisible(v);
	uiHypocenter.fmFrameInformation->setVisible(v);
	uiHypocenter.fmFrameInformationAutomatic->setVisible(v && _showLastAutomaticSolution);
	uiHypocenter.fmLabelFrameInfoSpacer->setVisible(v && _showLastAutomaticSolution);
}


void EventSummaryView::showVisibleMagnitudes(bool e) {
	if ( e )
		_magList->showAll();
	else
		_magList->hideTypes(_visibleMagnitudes);
}


void EventSummaryView::showOnlyMostRecentEvent(bool e) {
	_showOnlyMostRecentEvent = e;
}


void EventSummaryView::ignoreOtherEvents(bool e) {
	_ignoreOtherEvents = e;
}


}
}
