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


#include "infowidget.h"

#include <QHeaderView>
#include <QBoxLayout>
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QGroupBox>

#define SEISCOMP_COMPONENT MapView
#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>

using namespace Seiscomp;


namespace {

const QColor WARNING_COLOR(255, 165, 0);
const QColor ERROR_COLOR(Qt::red);




void createConfigureAndAddTreeItem(QTreeWidgetItem* rootItem, const QString& title,
                                   const QString& text, const QColor& backgroundColor = Qt::white) {
	int column = 1;

	QTreeWidgetItem* item = new QTreeWidgetItem(rootItem, QStringList(title));
	item->setText(column, text);
	item->setBackgroundColor(column, backgroundColor);
}


} // namespace





bool __compareInfoWidgetIds(const InfoWidget* infoWidget, std::string id) {
	if ( infoWidget->id() == id )
		return true;
	return false;
}




template <typename T>
InfoWidgetRegistry<T>* InfoWidgetRegistry<T>::_Registry = NULL;




InfoWidget::InfoWidget(const std::string& id, QWidget* parent, Qt::WindowFlags f)
 : QWidget(parent, f),
   _id(id) {

	uiInit();
}




QTreeWidget* InfoWidget::treeWidget() {
	return _treeWidget;
}




QSplitter* InfoWidget::splitter() {
	return _splitter;
}




void InfoWidget::uiInit() {
	setWindowFlags(Qt::Tool);
	setAttribute(Qt::WA_DeleteOnClose);
	setGeometry(0, 0, 380, 480);

	_treeWidget = new QTreeWidget(this);
	_treeWidget->setColumnCount(2);
	_treeWidget->adjustSize ();
	_treeWidget->header()->hide();

	_splitter = new QSplitter(Qt::Vertical);
	_splitter->addWidget(_treeWidget);

	QVBoxLayout* boxLayout = new QVBoxLayout;
	boxLayout->addWidget(_splitter);
	setLayout(boxLayout);

	activateWindow();
}




const std::string& InfoWidget::id() const {
	return _id;
}




StationInfoWidget::StationInfoWidget(const std::string& id, QWidget* parent, Qt::WindowFlags f)
 : InfoWidget(id, parent, f),
   _stationItem(NULL),
   _amplitudeItem(NULL),
   _qcItem(NULL) {

	StationInfoWidgetRegistry::Instance()->add(this);

	uiInit();
	startWaveformAcquisition();
}




StationInfoWidget::~StationInfoWidget() {
	StationInfoWidgetRegistry::Instance()->remove(this);

	stopWaveformAcquisition();
}




void StationInfoWidget::init() {
	_stationItem = createAndAddTopLevelItem("Station");
	_qcItem = createAndAddTopLevelItem("Quality Control");
	_amplitudeItem = createAndAddTopLevelItem("Amplitude");


	updateContent();
	startWaveformAcquisition();
}




void StationInfoWidget::updateContent() {
	removeContent(_stationItem);
	removeContent(_qcItem);
	removeContent(_amplitudeItem);

	updateStationContent();

	if ( !_qcInfo.empty() )
		updateQCContent();

	if ( !_amplitudeInfo.empty() )
		updateAmplitudeContent();

	resizeColumnsToContent();
}




void StationInfoWidget::addRecordMarker(const Core::Time& time,
                                        const std::string& phaseHint) {
	Gui::RecordMarker* marker = createRecordMarkerFromTime(time, phaseHint);

	_recordMarkerCache.push_back(marker);
	_recordWidget->addMarker(marker);

	removeExpiredRecordMarker();
}




void StationInfoWidget::setLongitude(const QString& longitude)  {
	_stationInfo[STATION_LONGITUDE_TAG] = longitude;
}




void StationInfoWidget::setLatitude(const QString& latitude)  {
	_stationInfo[STATION_LATITUDE_TAG] = latitude;
}




void StationInfoWidget::setElevation(const QString& elevation)  {
	_stationInfo[STATION_ELEVATION_TAG] = elevation;
}




void StationInfoWidget::setDepth(const QString& depth)  {
	//_stationInfo[STATION_DEPTH_TAG] = depth;
}




void StationInfoWidget::setPlace(const QString& place)  {
	_stationInfo[STATION_PLACE_TAG] = place;
}




void StationInfoWidget::setCountry(const QString& country)  {
	_stationInfo[STATION_COUNTRY_TAG] = country;
}




void StationInfoWidget::setDescription(const QString& description)  {
	_stationInfo[STATION_DESCRIPTION_TAG] = description;
}




void StationInfoWidget::setAmplitudeTime(const QString& time) {
	_amplitudeInfo[AMPLITUDE_TIME_TAG] = time;
}




void StationInfoWidget::setAmplitudeMaxValue(const QString& value) {
	_amplitudeInfo[AMPLITUDE_MAX_VALUE_TAG] = value;
}




void StationInfoWidget::setAmplitudeMaxValueTime(const QString& time) {
	_amplitudeInfo[AMPLITUDE_MAX_VALUE_TIME_TAG] = time;
}




void StationInfoWidget::setQCContent(const QString& name, const QString& value,
		                             const QString& lowerUncertainty, const QString& upperUncertainty,
		                             const QColor& backgroundColor) {
	QCItemData data;
	data.name             = name;
	data.value            = value;
	data.lowerUncertainty = lowerUncertainty;
	data.upperUncertainty = upperUncertainty;
	data.backgroundColor  = backgroundColor;

	_qcInfo[name] = data;
}




void StationInfoWidget::setRecordFilterString(const std::string& filterStr) {
	_recordFilterStr = filterStr;
}




void StationInfoWidget::setRecordSequeceTimeSpan(const Core::TimeSpan& timeSpan) {
	_recordSequenceTimeSpan = timeSpan;
}




void StationInfoWidget::setRecordStreamUrl(const std::string& url) {
	_recordStreamUrl = url;
}




void StationInfoWidget::showEvent(QShowEvent*) {
	// Use the same code as resizeEvent which does not depend on the
	// parameter QResizeEvent*
	// Reason: When the widget is shown for the first time the resizeEvent
	// handler is called before all child widgets have been layouted and
	// resized properly. So the record scale will be calculated based on
	// a wrong _recordWidget width. The showEvent handler is called after
	// everything has been resized and layouted and we make sure that the
	// record scale has the value we want it to be.
	resizeEvent(NULL);
}




void StationInfoWidget::resizeEvent(QResizeEvent* evt) {
	adjustRecordWidgetSize();
}




void StationInfoWidget::uiInit() {
	QString title = QString("Station Info: %1").arg(id().c_str());
	setWindowTitle(title);

	//	QMenu* menu = new QMenu("Menu");
	//	menuBar()->addMenu(menu);
	//
	//	QAction* action = menu->addAction("Show Waveform Data");
	//	action->setCheckable(true);
	//	action->setChecked(true);

	//  connect(action, SIGNAL(triggered()), this, SLOT(showWaveformData()));

	_recordWidget = new Gui::RecordWidget;
	_recordWidget->setMinimumHeight(40);

	QPalette tmpPalette = palette();
	tmpPalette.setColor(_recordWidget->backgroundRole(), Qt::white);
	tmpPalette.setColor(_recordWidget->foregroundRole(), Qt::darkGray);
	_recordWidget->setPalette(tmpPalette);

	_recordWidgetTimeScale = new Gui::TimeScale;
	_recordWidgetTimeScale->setAbsoluteTimeEnabled(false);

	QGroupBox* groupBox = new QGroupBox(QString("Waveform Data %1").arg(id().c_str()));

	QVBoxLayout* boxLayout = new QVBoxLayout();
	boxLayout->setMargin(1);
	boxLayout->addWidget(_recordWidget);
	boxLayout->addWidget(_recordWidgetTimeScale);

	groupBox->setLayout(boxLayout);
	splitter()->addWidget(groupBox);

	connect(_recordWidgetTimeScale, SIGNAL(changedInterval(double, double, double)),
			_recordWidget, SLOT(setGridSpacing(double, double, double)));
}




void StationInfoWidget::startWaveformAcquisition() {
	Math::Filtering::InPlaceFilter<float> *filter =
		Math::Filtering::InPlaceFilter<float>::Create(_recordFilterStr);
	if ( !filter ) {
		SEISCOMP_ERROR("StationInfoWidget: Could not create filter for: %s", _recordFilterStr.c_str());
		return;
	}

	_recordWidget->setFilter(filter);
	delete filter;

	_recordStreamThread = RecordStreamThreadPtr(new Gui::RecordStreamThread(_recordStreamUrl));


	if ( !_recordStreamThread->connect() ) {
		SEISCOMP_ERROR("StationInfoWidget: Could not connect to url: %s", _recordStreamUrl.c_str());
		return;
	}

	std::vector<std::string> tokens;
	Core::split(tokens, id().c_str(), ".", false);
	_recordStreamThread->addStream(tokens[0], tokens[1], tokens[2], tokens[3]);

	Core::Time start = Core::Time::GMT() - _recordSequenceTimeSpan;
	Core::Time end = Core::Time::GMT();
	_recordStreamThread->setTimeWindow(Core::TimeWindow(start, end));

	_recordSequence = new RingBuffer(_recordSequenceTimeSpan);
	_recordWidget->setRecords(0, _recordSequence);

	connect(_recordStreamThread.get(), SIGNAL(receivedRecord(Seiscomp::Record*)),
			this, SLOT(updateRecordWidget(Seiscomp::Record*)));

	_recordStreamThread->start();

	adjustRecordWidgetSize();

	_recordWidget->setAlignment(Core::Time::GMT());

	_recordWidgetTimer.setInterval(1000);
	connect(&_recordWidgetTimer, SIGNAL(timeout()), this, SLOT(updateRecordWidgetAlignment()));

	_recordWidgetTimer.start();
}




void StationInfoWidget::stopWaveformAcquisition() {
	_recordWidgetTimer.stop();
	_recordStreamThread->stop(true);
	disconnect(_recordStreamThread.get(), 0, 0, 0);
}




void StationInfoWidget::updateStationContent() {
	createConfigureAndAddTreeItem(_stationItem, "ID", id().c_str());
	createConfigureAndAddTreeItem(_stationItem, "Longitude", _stationInfo[STATION_LONGITUDE_TAG]);
	createConfigureAndAddTreeItem(_stationItem, "Latitude", _stationInfo[STATION_LATITUDE_TAG]);
	createConfigureAndAddTreeItem(_stationItem, "Elevation", _stationInfo[STATION_ELEVATION_TAG]);
	// TODO: Find a replacement
	//createConfigureAndAddTreeItem(_stationItem, "Depth", _stationInfo[STATION_DEPTH_TAG]);
	createConfigureAndAddTreeItem(_stationItem, "Place", _stationInfo[STATION_PLACE_TAG]);
	createConfigureAndAddTreeItem(_stationItem, "Country", _stationInfo[STATION_COUNTRY_TAG]);
	createConfigureAndAddTreeItem(_stationItem, "Description", _stationInfo[STATION_DESCRIPTION_TAG]);
}




void StationInfoWidget::updateAmplitudeContent() {
	createConfigureAndAddTreeItem(_amplitudeItem, "Time", _amplitudeInfo[AMPLITUDE_TIME_TAG]);
	createConfigureAndAddTreeItem(_amplitudeItem, "STA/LTA max. Amp", _amplitudeInfo[AMPLITUDE_MAX_VALUE_TAG]);
	createConfigureAndAddTreeItem(_amplitudeItem, "Time of max. Amp.", _amplitudeInfo[AMPLITUDE_MAX_VALUE_TIME_TAG]);
}




void StationInfoWidget::updateQCContent() {
	QCInfo::const_iterator it = _qcInfo.begin();
	for ( ; it != _qcInfo.end(); it++ ) {
		QCItemData data = it->second;
        QString description;
        if ( !data.value.isEmpty() )
            description = QString("%1 [%2, %3]").arg(data.value)
		                                        .arg(data.lowerUncertainty)
		                                        .arg(data.upperUncertainty);

		createConfigureAndAddTreeItem(_qcItem, data.name, description, data.backgroundColor);
	}
}




void StationInfoWidget::adjustRecordWidgetSize() {
	double seconds = _recordSequenceTimeSpan.seconds();
	_recordWidget->setTimeScale( _recordWidget->width() / seconds);
	_recordWidget->setTimeRange(-seconds, 0);

	_recordWidgetTimeScale->setScale( _recordWidget->width() / seconds);
	_recordWidgetTimeScale->setTimeRange(-1 * seconds, 0);
}




Gui::RecordMarker* StationInfoWidget::createRecordMarkerFromTime(const Core::Time& time,
                                                                 const std::string& phaseHint) {
	Gui::RecordMarker* marker = new Gui::RecordMarker(NULL, time);
	marker->setText(QString("%1").arg(phaseHint.c_str()));
	marker->setColor(Qt::red);

	return marker;
}




void StationInfoWidget::removeExpiredRecordMarker() {
	RecordMarkerCollection::iterator it = _recordMarkerCache.begin();
	for ( ; it != _recordMarkerCache.end(); it ++ ) {
		Core::Time referenceTime = Core::Time::GMT() - _recordSequenceTimeSpan;
		if ( (*it)->time() < referenceTime ) {
			_recordWidget->removeMarker(*it);
			_recordMarkerCache.erase(it);
		}
	}
}




QTreeWidgetItem* StationInfoWidget::createAndAddTopLevelItem(const QString& text) {
	QTreeWidgetItem* item = new QTreeWidgetItem(treeWidget(), QStringList(text));
	treeWidget()->expandItem(item);

	return item;
}




void StationInfoWidget::removeContent(QTreeWidgetItem* item) {
	QList<QTreeWidgetItem*> children = item->takeChildren();
	for (QList<QTreeWidgetItem*>::iterator it = children.begin(); it != children.end(); ++it)
		delete *it;
}




void StationInfoWidget::resizeColumnsToContent() {
	bool isStationItemExpanded = _stationItem->isExpanded();
	bool isAmplitudeItemExpanded = _amplitudeItem->isExpanded();
	bool isQCItemExpanded = _qcItem->isExpanded();

	treeWidget()->expandItem(_stationItem);
	treeWidget()->expandItem(_amplitudeItem);
	treeWidget()->expandItem(_qcItem);

	treeWidget()->resizeColumnToContents(0);

	treeWidget()->setItemExpanded(_stationItem, isStationItemExpanded);
	treeWidget()->setItemExpanded(_amplitudeItem, isAmplitudeItemExpanded);
	treeWidget()->setItemExpanded(_qcItem, isQCItemExpanded);
}




//void StationInfoWidget::showWaveformData() {
//}




void StationInfoWidget::updateRecordWidget(Seiscomp::Record* record) {
	std::string streamCode = record->networkCode() + "." +
	                         record->stationCode() + "." +
	                         record->locationCode() + "." +
	                         record->channelCode();

	if ( streamCode != id() ) {
		SEISCOMP_DEBUG("StationInfoWidget: received record does not match stations streamcode %s != %s",
		               streamCode.c_str(), id().c_str());
		return;
	}

	if ( _recordSequence->feed(record) )
		_recordWidget->fed(0, record);

	_recordWidget->update();
}





void StationInfoWidget::updateRecordWidgetAlignment() {
	_recordWidget->setAlignment(Core::Time::GMT());
}




OriginInfoWidget::OriginInfoWidget(const std::string& id, QWidget* parent, Qt::WindowFlags f)
 : InfoWidget(id, parent, f) {
	EventInfoWidgetRegistry::Instance()->add(this);

	uiInit();
}




OriginInfoWidget::~OriginInfoWidget() {
	EventInfoWidgetRegistry::Instance()->remove(this);
}




void OriginInfoWidget::updateContent() {
	treeWidget()->clear();

	QTreeWidgetItem* rootItem = new QTreeWidgetItem(treeWidget(),
	                                                QStringList(QString("Preferred Origin for %1").arg(id().c_str())));
	createConfigureAndAddTreeItem(rootItem, "ID", _originInfo[ORIGIN_ID_TAG]);
	createConfigureAndAddTreeItem(rootItem, "Time", _originInfo[ORIGIN_TIME_TAG]);
	createConfigureAndAddTreeItem(rootItem, "Latitude", _originInfo[ORIGIN_LATITUDE_TAG]);
	createConfigureAndAddTreeItem(rootItem, "Longitude", _originInfo[ORIGIN_LONGITUDE_TAG]);
	createConfigureAndAddTreeItem(rootItem, "Depth", _originInfo[ORIGIN_DEPTH_TAG]);
	createConfigureAndAddTreeItem(rootItem, "Magnitude", _originInfo[ORIGIN_MAGNITUDE_TAG]);
	createConfigureAndAddTreeItem(rootItem, "Agency", _originInfo[ORIGIN_AGENCY_TAG]);
	createConfigureAndAddTreeItem(rootItem, "Mode", _originInfo[ORIGIN_MODE_TAG]);
	createConfigureAndAddTreeItem(rootItem, "Status", _originInfo[ORIGIN_STATUS_TAG]);
	createConfigureAndAddTreeItem(rootItem, "Station Count", _originInfo[ORIGIN_STATION_COUNT_TAG]);
	createConfigureAndAddTreeItem(rootItem, "Azimuth Gap", _originInfo[ORIGIN_AZIMUTH_GAP_TAG]);
	createConfigureAndAddTreeItem(rootItem, "Minimum Distance", _originInfo[ORIGIN_MINIMUM_DISTANCE_TAG]);
	createConfigureAndAddTreeItem(rootItem, "Maximum Distance", _originInfo[ORIGIN_MAXIMUM_DISTANCE_TAG]);

	treeWidget()->expandItem(rootItem);
	treeWidget()->resizeColumnToContents(1);
}




void OriginInfoWidget::setPreferredOriginId(const QString& id) {
	_originInfo[ORIGIN_ID_TAG] = id;
}




void OriginInfoWidget::setTime(const QString& time) {
	_originInfo[ORIGIN_TIME_TAG] = time;
}




void OriginInfoWidget::setLongitude(const QString& longitude) {
	_originInfo[ORIGIN_LONGITUDE_TAG] = longitude;
}




void OriginInfoWidget::setLatitude(const QString& latitude) {
	_originInfo[ORIGIN_LATITUDE_TAG] = latitude;
}




void OriginInfoWidget::setDepth(const QString& depth) {
	_originInfo[ORIGIN_DEPTH_TAG] = depth;
}




void OriginInfoWidget::setMagnitude(const QString& magnitude) {
	_originInfo[ORIGIN_MAGNITUDE_TAG] = magnitude;
}




void OriginInfoWidget::setAgency(const QString& agency) {
	_originInfo[ORIGIN_AGENCY_TAG] = agency;
}




void OriginInfoWidget::setMode(const QString& mode) {
	_originInfo[ORIGIN_MODE_TAG] = mode;
}




void OriginInfoWidget::setStatus(const QString& status) {
	_originInfo[ORIGIN_STATUS_TAG] = status;
}




void OriginInfoWidget::setStationCount(const QString& stationCount) {
	_originInfo[ORIGIN_STATION_COUNT_TAG] = stationCount;
}




void OriginInfoWidget::setAzimuthGap(const QString& azimuthGap) {
	_originInfo[ORIGIN_AZIMUTH_GAP_TAG] = azimuthGap;
}




void OriginInfoWidget::setMinimumDistance(const QString& minimumDistance) {
	_originInfo[ORIGIN_MINIMUM_DISTANCE_TAG] = minimumDistance;
}




void OriginInfoWidget::setMaximumDistance(const QString& maximumDistance) {
	_originInfo[ORIGIN_MAXIMUM_DISTANCE_TAG] = maximumDistance;
}




void OriginInfoWidget::uiInit() {
	QString title = QString("Event Info: %1").arg(id().c_str());
	setWindowTitle(title);
	activateWindow();
}
