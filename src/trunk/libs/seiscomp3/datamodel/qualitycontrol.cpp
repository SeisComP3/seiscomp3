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

// This file was created by a source code generator.
// Do not modify the contents. Change the definition and run the generator
// again!


#define SEISCOMP_COMPONENT DataModel
#include <seiscomp3/datamodel/qualitycontrol.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(QualityControl, PublicObject, "QualityControl");


QualityControl::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(arrayObjectProperty("log", "QCLog", &QualityControl::qCLogCount, &QualityControl::qCLog, static_cast<bool (QualityControl::*)(QCLog*)>(&QualityControl::add), &QualityControl::removeQCLog, static_cast<bool (QualityControl::*)(QCLog*)>(&QualityControl::remove)));
	addProperty(arrayClassProperty<WaveformQuality>("waveformQuality", "WaveformQuality", &QualityControl::waveformQualityCount, &QualityControl::waveformQuality, static_cast<bool (QualityControl::*)(WaveformQuality*)>(&QualityControl::add), &QualityControl::removeWaveformQuality, static_cast<bool (QualityControl::*)(WaveformQuality*)>(&QualityControl::remove)));
	addProperty(arrayClassProperty<Outage>("outage", "Outage", &QualityControl::outageCount, &QualityControl::outage, static_cast<bool (QualityControl::*)(Outage*)>(&QualityControl::add), &QualityControl::removeOutage, static_cast<bool (QualityControl::*)(Outage*)>(&QualityControl::remove)));
}


IMPLEMENT_METAOBJECT(QualityControl)


QualityControl::QualityControl() : PublicObject("QualityControl") {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QualityControl::QualityControl(const QualityControl& other)
 : PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QualityControl::~QualityControl() {
	std::for_each(_qCLogs.begin(), _qCLogs.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&QCLog::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&QCLogPtr::get)));
	std::for_each(_waveformQualitys.begin(), _waveformQualitys.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&WaveformQuality::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&WaveformQualityPtr::get)));
	std::for_each(_outages.begin(), _outages.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Outage::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&OutagePtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::operator==(const QualityControl& rhs) const {
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::operator!=(const QualityControl& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::equal(const QualityControl& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QualityControl& QualityControl::operator=(const QualityControl& other) {
	PublicObject::operator=(other);
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::assign(Object* other) {
	QualityControl* otherQualityControl = QualityControl::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherQualityControl;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::attachTo(PublicObject* parent) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::detachFrom(PublicObject* object) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::detach() {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* QualityControl::clone() const {
	QualityControl* clonee = new QualityControl();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::updateChild(Object* child) {
	QCLog* qCLogChild = QCLog::Cast(child);
	if ( qCLogChild != NULL ) {
		QCLog* qCLogElement
			= QCLog::Cast(PublicObject::Find(qCLogChild->publicID()));
		if ( qCLogElement && qCLogElement->parent() == this ) {
			*qCLogElement = *qCLogChild;
			return true;
		}
		return false;
	}

	WaveformQuality* waveformQualityChild = WaveformQuality::Cast(child);
	if ( waveformQualityChild != NULL ) {
		WaveformQuality* waveformQualityElement = waveformQuality(waveformQualityChild->index());
		if ( waveformQualityElement != NULL ) {
			*waveformQualityElement = *waveformQualityChild;
			return true;
		}
		return false;
	}

	Outage* outageChild = Outage::Cast(child);
	if ( outageChild != NULL ) {
		Outage* outageElement = outage(outageChild->index());
		if ( outageElement != NULL ) {
			*outageElement = *outageChild;
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QualityControl::accept(Visitor* visitor) {
	for ( std::vector<QCLogPtr>::iterator it = _qCLogs.begin(); it != _qCLogs.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<WaveformQualityPtr>::iterator it = _waveformQualitys.begin(); it != _waveformQualitys.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<OutagePtr>::iterator it = _outages.begin(); it != _outages.end(); ++it )
		(*it)->accept(visitor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t QualityControl::qCLogCount() const {
	return _qCLogs.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QCLog* QualityControl::qCLog(size_t i) const {
	return _qCLogs[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QCLog* QualityControl::qCLog(const QCLogIndex& i) const {
	for ( std::vector<QCLogPtr>::const_iterator it = _qCLogs.begin(); it != _qCLogs.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QCLog* QualityControl::findQCLog(const std::string& publicID) const {
	for ( std::vector<QCLogPtr>::const_iterator it = _qCLogs.begin(); it != _qCLogs.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::add(QCLog* qCLog) {
	if ( qCLog == NULL )
		return false;

	// Element has already a parent
	if ( qCLog->parent() != NULL ) {
		SEISCOMP_ERROR("QualityControl::add(QCLog*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		QCLog* qCLogCached = QCLog::Find(qCLog->publicID());
		if ( qCLogCached ) {
			if ( qCLogCached->parent() ) {
				if ( qCLogCached->parent() == this )
					SEISCOMP_ERROR("QualityControl::add(QCLog*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("QualityControl::add(QCLog*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				qCLog = qCLogCached;
		}
	}

	// Add the element
	_qCLogs.push_back(qCLog);
	qCLog->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		qCLog->accept(&nc);
	}

	// Notify registered observers
	childAdded(qCLog);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::remove(QCLog* qCLog) {
	if ( qCLog == NULL )
		return false;

	if ( qCLog->parent() != this ) {
		SEISCOMP_ERROR("QualityControl::remove(QCLog*) -> element has another parent");
		return false;
	}

	std::vector<QCLogPtr>::iterator it;
	it = std::find(_qCLogs.begin(), _qCLogs.end(), qCLog);
	// Element has not been found
	if ( it == _qCLogs.end() ) {
		SEISCOMP_ERROR("QualityControl::remove(QCLog*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_qCLogs.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::removeQCLog(size_t i) {
	// index out of bounds
	if ( i >= _qCLogs.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_qCLogs[i]->accept(&nc);
	}

	_qCLogs[i]->setParent(NULL);
	childRemoved(_qCLogs[i].get());
	
	_qCLogs.erase(_qCLogs.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::removeQCLog(const QCLogIndex& i) {
	QCLog* object = qCLog(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t QualityControl::waveformQualityCount() const {
	return _waveformQualitys.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WaveformQuality* QualityControl::waveformQuality(size_t i) const {
	return _waveformQualitys[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
WaveformQuality* QualityControl::waveformQuality(const WaveformQualityIndex& i) const {
	for ( std::vector<WaveformQualityPtr>::const_iterator it = _waveformQualitys.begin(); it != _waveformQualitys.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::add(WaveformQuality* waveformQuality) {
	if ( waveformQuality == NULL )
		return false;

	// Element has already a parent
	if ( waveformQuality->parent() != NULL ) {
		SEISCOMP_ERROR("QualityControl::add(WaveformQuality*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<WaveformQualityPtr>::iterator it = _waveformQualitys.begin(); it != _waveformQualitys.end(); ++it ) {
		if ( (*it)->index() == waveformQuality->index() ) {
			SEISCOMP_ERROR("QualityControl::add(WaveformQuality*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_waveformQualitys.push_back(waveformQuality);
	waveformQuality->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		waveformQuality->accept(&nc);
	}

	// Notify registered observers
	childAdded(waveformQuality);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::remove(WaveformQuality* waveformQuality) {
	if ( waveformQuality == NULL )
		return false;

	if ( waveformQuality->parent() != this ) {
		SEISCOMP_ERROR("QualityControl::remove(WaveformQuality*) -> element has another parent");
		return false;
	}

	std::vector<WaveformQualityPtr>::iterator it;
	it = std::find(_waveformQualitys.begin(), _waveformQualitys.end(), waveformQuality);
	// Element has not been found
	if ( it == _waveformQualitys.end() ) {
		SEISCOMP_ERROR("QualityControl::remove(WaveformQuality*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_waveformQualitys.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::removeWaveformQuality(size_t i) {
	// index out of bounds
	if ( i >= _waveformQualitys.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_waveformQualitys[i]->accept(&nc);
	}

	_waveformQualitys[i]->setParent(NULL);
	childRemoved(_waveformQualitys[i].get());
	
	_waveformQualitys.erase(_waveformQualitys.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::removeWaveformQuality(const WaveformQualityIndex& i) {
	WaveformQuality* object = waveformQuality(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t QualityControl::outageCount() const {
	return _outages.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Outage* QualityControl::outage(size_t i) const {
	return _outages[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Outage* QualityControl::outage(const OutageIndex& i) const {
	for ( std::vector<OutagePtr>::const_iterator it = _outages.begin(); it != _outages.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::add(Outage* outage) {
	if ( outage == NULL )
		return false;

	// Element has already a parent
	if ( outage->parent() != NULL ) {
		SEISCOMP_ERROR("QualityControl::add(Outage*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<OutagePtr>::iterator it = _outages.begin(); it != _outages.end(); ++it ) {
		if ( (*it)->index() == outage->index() ) {
			SEISCOMP_ERROR("QualityControl::add(Outage*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_outages.push_back(outage);
	outage->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		outage->accept(&nc);
	}

	// Notify registered observers
	childAdded(outage);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::remove(Outage* outage) {
	if ( outage == NULL )
		return false;

	if ( outage->parent() != this ) {
		SEISCOMP_ERROR("QualityControl::remove(Outage*) -> element has another parent");
		return false;
	}

	std::vector<OutagePtr>::iterator it;
	it = std::find(_outages.begin(), _outages.end(), outage);
	// Element has not been found
	if ( it == _outages.end() ) {
		SEISCOMP_ERROR("QualityControl::remove(Outage*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_outages.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::removeOutage(size_t i) {
	// index out of bounds
	if ( i >= _outages.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_outages[i]->accept(&nc);
	}

	_outages[i]->setParent(NULL);
	childRemoved(_outages[i].get());
	
	_outages.erase(_outages.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool QualityControl::removeOutage(const OutageIndex& i) {
	Outage* object = outage(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QualityControl::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,9>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: QualityControl skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("log",
	                       Seiscomp::Core::Generic::containerMember(_qCLogs,
	                       Seiscomp::Core::Generic::bindMemberFunction<QCLog>(static_cast<bool (QualityControl::*)(QCLog*)>(&QualityControl::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("waveformQuality",
	                       Seiscomp::Core::Generic::containerMember(_waveformQualitys,
	                       Seiscomp::Core::Generic::bindMemberFunction<WaveformQuality>(static_cast<bool (QualityControl::*)(WaveformQuality*)>(&QualityControl::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("outage",
	                       Seiscomp::Core::Generic::containerMember(_outages,
	                       Seiscomp::Core::Generic::bindMemberFunction<Outage>(static_cast<bool (QualityControl::*)(Outage*)>(&QualityControl::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
