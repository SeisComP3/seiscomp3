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


#define SEISCOMP_COMPONENT DataModel
#include <seiscomp3/datamodel/momenttensor.h>
#include <seiscomp3/datamodel/focalmechanism.h>
#include <seiscomp3/datamodel/dataused.h>
#include <seiscomp3/datamodel/momenttensorstationcontribution.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(MomentTensor, PublicObject, "MomentTensor");


namespace {
static Seiscomp::Core::MetaEnumImpl<MomentTensorMethod> metaMomentTensorMethod;
static Seiscomp::Core::MetaEnumImpl<MomentTensorStatus> metaMomentTensorStatus;
}


MomentTensor::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("derivedOriginID", "string", false, false, false, true, false, false, NULL, &MomentTensor::setDerivedOriginID, &MomentTensor::derivedOriginID));
	addProperty(Core::simpleProperty("momentMagnitudeID", "string", false, false, false, true, false, false, NULL, &MomentTensor::setMomentMagnitudeID, &MomentTensor::momentMagnitudeID));
	addProperty(objectProperty<RealQuantity>("scalarMoment", "RealQuantity", false, false, true, &MomentTensor::setScalarMoment, &MomentTensor::scalarMoment));
	addProperty(objectProperty<Tensor>("tensor", "Tensor", false, false, true, &MomentTensor::setTensor, &MomentTensor::tensor));
	addProperty(Core::simpleProperty("variance", "float", false, false, false, false, true, false, NULL, &MomentTensor::setVariance, &MomentTensor::variance));
	addProperty(Core::simpleProperty("varianceReduction", "float", false, false, false, false, true, false, NULL, &MomentTensor::setVarianceReduction, &MomentTensor::varianceReduction));
	addProperty(Core::simpleProperty("doubleCouple", "float", false, false, false, false, true, false, NULL, &MomentTensor::setDoubleCouple, &MomentTensor::doubleCouple));
	addProperty(Core::simpleProperty("clvd", "float", false, false, false, false, true, false, NULL, &MomentTensor::setClvd, &MomentTensor::clvd));
	addProperty(Core::simpleProperty("iso", "float", false, false, false, false, true, false, NULL, &MomentTensor::setIso, &MomentTensor::iso));
	addProperty(Core::simpleProperty("greensFunctionID", "string", false, false, false, false, false, false, NULL, &MomentTensor::setGreensFunctionID, &MomentTensor::greensFunctionID));
	addProperty(Core::simpleProperty("filterID", "string", false, false, false, false, false, false, NULL, &MomentTensor::setFilterID, &MomentTensor::filterID));
	addProperty(objectProperty<SourceTimeFunction>("sourceTimeFunction", "SourceTimeFunction", false, false, true, &MomentTensor::setSourceTimeFunction, &MomentTensor::sourceTimeFunction));
	addProperty(Core::simpleProperty("methodID", "string", false, false, false, false, false, false, NULL, &MomentTensor::setMethodID, &MomentTensor::methodID));
	addProperty(enumProperty("method", "MomentTensorMethod", false, true, &metaMomentTensorMethod, &MomentTensor::setMethod, &MomentTensor::method));
	addProperty(enumProperty("status", "MomentTensorStatus", false, true, &metaMomentTensorStatus, &MomentTensor::setStatus, &MomentTensor::status));
	addProperty(Core::simpleProperty("cmtName", "string", false, false, false, false, false, false, NULL, &MomentTensor::setCmtName, &MomentTensor::cmtName));
	addProperty(Core::simpleProperty("cmtVersion", "string", false, false, false, false, false, false, NULL, &MomentTensor::setCmtVersion, &MomentTensor::cmtVersion));
	addProperty(objectProperty<CreationInfo>("creationInfo", "CreationInfo", false, false, true, &MomentTensor::setCreationInfo, &MomentTensor::creationInfo));
	addProperty(arrayClassProperty<Comment>("comment", "Comment", &MomentTensor::commentCount, &MomentTensor::comment, static_cast<bool (MomentTensor::*)(Comment*)>(&MomentTensor::add), &MomentTensor::removeComment, static_cast<bool (MomentTensor::*)(Comment*)>(&MomentTensor::remove)));
	addProperty(arrayClassProperty<DataUsed>("dataUsed", "DataUsed", &MomentTensor::dataUsedCount, &MomentTensor::dataUsed, static_cast<bool (MomentTensor::*)(DataUsed*)>(&MomentTensor::add), &MomentTensor::removeDataUsed, static_cast<bool (MomentTensor::*)(DataUsed*)>(&MomentTensor::remove)));
	addProperty(arrayClassProperty<MomentTensorPhaseSetting>("phaseSetting", "MomentTensorPhaseSetting", &MomentTensor::momentTensorPhaseSettingCount, &MomentTensor::momentTensorPhaseSetting, static_cast<bool (MomentTensor::*)(MomentTensorPhaseSetting*)>(&MomentTensor::add), &MomentTensor::removeMomentTensorPhaseSetting, static_cast<bool (MomentTensor::*)(MomentTensorPhaseSetting*)>(&MomentTensor::remove)));
	addProperty(arrayObjectProperty("stationMomentTensorContribution", "MomentTensorStationContribution", &MomentTensor::momentTensorStationContributionCount, &MomentTensor::momentTensorStationContribution, static_cast<bool (MomentTensor::*)(MomentTensorStationContribution*)>(&MomentTensor::add), &MomentTensor::removeMomentTensorStationContribution, static_cast<bool (MomentTensor::*)(MomentTensorStationContribution*)>(&MomentTensor::remove)));
}


IMPLEMENT_METAOBJECT(MomentTensor)


MomentTensor::MomentTensor() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensor::MomentTensor(const MomentTensor& other)
: PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensor::MomentTensor(const std::string& publicID)
: PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensor::~MomentTensor() {
	std::for_each(_comments.begin(), _comments.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Comment::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&CommentPtr::get)));
	std::for_each(_dataUseds.begin(), _dataUseds.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&DataUsed::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&DataUsedPtr::get)));
	std::for_each(_momentTensorPhaseSettings.begin(), _momentTensorPhaseSettings.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&MomentTensorPhaseSetting::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&MomentTensorPhaseSettingPtr::get)));
	std::for_each(_momentTensorStationContributions.begin(), _momentTensorStationContributions.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&MomentTensorStationContribution::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&MomentTensorStationContributionPtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensor* MomentTensor::Create() {
	MomentTensor* object = new MomentTensor();
	return static_cast<MomentTensor*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensor* MomentTensor::Create(const std::string& publicID) {
	if ( PublicObject::IsRegistrationEnabled() && Find(publicID) != NULL ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return NULL;
	}

	return new MomentTensor(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensor* MomentTensor::Find(const std::string& publicID) {
	return MomentTensor::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::operator==(const MomentTensor& rhs) const {
	if ( _derivedOriginID != rhs._derivedOriginID ) return false;
	if ( _momentMagnitudeID != rhs._momentMagnitudeID ) return false;
	if ( _scalarMoment != rhs._scalarMoment ) return false;
	if ( _tensor != rhs._tensor ) return false;
	if ( _variance != rhs._variance ) return false;
	if ( _varianceReduction != rhs._varianceReduction ) return false;
	if ( _doubleCouple != rhs._doubleCouple ) return false;
	if ( _clvd != rhs._clvd ) return false;
	if ( _iso != rhs._iso ) return false;
	if ( _greensFunctionID != rhs._greensFunctionID ) return false;
	if ( _filterID != rhs._filterID ) return false;
	if ( _sourceTimeFunction != rhs._sourceTimeFunction ) return false;
	if ( _methodID != rhs._methodID ) return false;
	if ( _method != rhs._method ) return false;
	if ( _status != rhs._status ) return false;
	if ( _cmtName != rhs._cmtName ) return false;
	if ( _cmtVersion != rhs._cmtVersion ) return false;
	if ( _creationInfo != rhs._creationInfo ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::operator!=(const MomentTensor& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::equal(const MomentTensor& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setDerivedOriginID(const std::string& derivedOriginID) {
	_derivedOriginID = derivedOriginID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& MomentTensor::derivedOriginID() const {
	return _derivedOriginID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setMomentMagnitudeID(const std::string& momentMagnitudeID) {
	_momentMagnitudeID = momentMagnitudeID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& MomentTensor::momentMagnitudeID() const {
	return _momentMagnitudeID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setScalarMoment(const OPT(RealQuantity)& scalarMoment) {
	_scalarMoment = scalarMoment;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& MomentTensor::scalarMoment() {
	if ( _scalarMoment )
		return *_scalarMoment;
	throw Seiscomp::Core::ValueException("MomentTensor.scalarMoment is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& MomentTensor::scalarMoment() const {
	if ( _scalarMoment )
		return *_scalarMoment;
	throw Seiscomp::Core::ValueException("MomentTensor.scalarMoment is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setTensor(const OPT(Tensor)& tensor) {
	_tensor = tensor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Tensor& MomentTensor::tensor() {
	if ( _tensor )
		return *_tensor;
	throw Seiscomp::Core::ValueException("MomentTensor.tensor is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Tensor& MomentTensor::tensor() const {
	if ( _tensor )
		return *_tensor;
	throw Seiscomp::Core::ValueException("MomentTensor.tensor is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setVariance(const OPT(double)& variance) {
	_variance = variance;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MomentTensor::variance() const {
	if ( _variance )
		return *_variance;
	throw Seiscomp::Core::ValueException("MomentTensor.variance is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setVarianceReduction(const OPT(double)& varianceReduction) {
	_varianceReduction = varianceReduction;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MomentTensor::varianceReduction() const {
	if ( _varianceReduction )
		return *_varianceReduction;
	throw Seiscomp::Core::ValueException("MomentTensor.varianceReduction is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setDoubleCouple(const OPT(double)& doubleCouple) {
	_doubleCouple = doubleCouple;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MomentTensor::doubleCouple() const {
	if ( _doubleCouple )
		return *_doubleCouple;
	throw Seiscomp::Core::ValueException("MomentTensor.doubleCouple is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setClvd(const OPT(double)& clvd) {
	_clvd = clvd;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MomentTensor::clvd() const {
	if ( _clvd )
		return *_clvd;
	throw Seiscomp::Core::ValueException("MomentTensor.clvd is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setIso(const OPT(double)& iso) {
	_iso = iso;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double MomentTensor::iso() const {
	if ( _iso )
		return *_iso;
	throw Seiscomp::Core::ValueException("MomentTensor.iso is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setGreensFunctionID(const std::string& greensFunctionID) {
	_greensFunctionID = greensFunctionID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& MomentTensor::greensFunctionID() const {
	return _greensFunctionID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setFilterID(const std::string& filterID) {
	_filterID = filterID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& MomentTensor::filterID() const {
	return _filterID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setSourceTimeFunction(const OPT(SourceTimeFunction)& sourceTimeFunction) {
	_sourceTimeFunction = sourceTimeFunction;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SourceTimeFunction& MomentTensor::sourceTimeFunction() {
	if ( _sourceTimeFunction )
		return *_sourceTimeFunction;
	throw Seiscomp::Core::ValueException("MomentTensor.sourceTimeFunction is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const SourceTimeFunction& MomentTensor::sourceTimeFunction() const {
	if ( _sourceTimeFunction )
		return *_sourceTimeFunction;
	throw Seiscomp::Core::ValueException("MomentTensor.sourceTimeFunction is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setMethodID(const std::string& methodID) {
	_methodID = methodID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& MomentTensor::methodID() const {
	return _methodID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setMethod(const OPT(MomentTensorMethod)& method) {
	_method = method;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorMethod MomentTensor::method() const {
	if ( _method )
		return *_method;
	throw Seiscomp::Core::ValueException("MomentTensor.method is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setStatus(const OPT(MomentTensorStatus)& status) {
	_status = status;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorStatus MomentTensor::status() const {
	if ( _status )
		return *_status;
	throw Seiscomp::Core::ValueException("MomentTensor.status is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setCmtName(const std::string& cmtName) {
	_cmtName = cmtName;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& MomentTensor::cmtName() const {
	return _cmtName;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setCmtVersion(const std::string& cmtVersion) {
	_cmtVersion = cmtVersion;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& MomentTensor::cmtVersion() const {
	return _cmtVersion;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::setCreationInfo(const OPT(CreationInfo)& creationInfo) {
	_creationInfo = creationInfo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CreationInfo& MomentTensor::creationInfo() {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("MomentTensor.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CreationInfo& MomentTensor::creationInfo() const {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("MomentTensor.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FocalMechanism* MomentTensor::focalMechanism() const {
	return static_cast<FocalMechanism*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensor& MomentTensor::operator=(const MomentTensor& other) {
	PublicObject::operator=(other);
	_derivedOriginID = other._derivedOriginID;
	_momentMagnitudeID = other._momentMagnitudeID;
	_scalarMoment = other._scalarMoment;
	_tensor = other._tensor;
	_variance = other._variance;
	_varianceReduction = other._varianceReduction;
	_doubleCouple = other._doubleCouple;
	_clvd = other._clvd;
	_iso = other._iso;
	_greensFunctionID = other._greensFunctionID;
	_filterID = other._filterID;
	_sourceTimeFunction = other._sourceTimeFunction;
	_methodID = other._methodID;
	_method = other._method;
	_status = other._status;
	_cmtName = other._cmtName;
	_cmtVersion = other._cmtVersion;
	_creationInfo = other._creationInfo;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::assign(Object* other) {
	MomentTensor* otherMomentTensor = MomentTensor::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherMomentTensor;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	FocalMechanism* focalMechanism = FocalMechanism::Cast(parent);
	if ( focalMechanism != NULL )
		return focalMechanism->add(this);

	SEISCOMP_ERROR("MomentTensor::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	FocalMechanism* focalMechanism = FocalMechanism::Cast(object);
	if ( focalMechanism != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return focalMechanism->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			MomentTensor* child = focalMechanism->findMomentTensor(publicID());
			if ( child != NULL )
				return focalMechanism->remove(child);
			else {
				SEISCOMP_DEBUG("MomentTensor::detachFrom(FocalMechanism): momentTensor has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("MomentTensor::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* MomentTensor::clone() const {
	MomentTensor* clonee = new MomentTensor();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::updateChild(Object* child) {
	Comment* commentChild = Comment::Cast(child);
	if ( commentChild != NULL ) {
		Comment* commentElement = comment(commentChild->index());
		if ( commentElement != NULL ) {
			*commentElement = *commentChild;
			return true;
		}
		return false;
	}

	// Do not know how to fetch child of type DataUsed without an index

	MomentTensorPhaseSetting* momentTensorPhaseSettingChild = MomentTensorPhaseSetting::Cast(child);
	if ( momentTensorPhaseSettingChild != NULL ) {
		MomentTensorPhaseSetting* momentTensorPhaseSettingElement = momentTensorPhaseSetting(momentTensorPhaseSettingChild->index());
		if ( momentTensorPhaseSettingElement != NULL ) {
			*momentTensorPhaseSettingElement = *momentTensorPhaseSettingChild;
			return true;
		}
		return false;
	}

	MomentTensorStationContribution* momentTensorStationContributionChild = MomentTensorStationContribution::Cast(child);
	if ( momentTensorStationContributionChild != NULL ) {
		MomentTensorStationContribution* momentTensorStationContributionElement
			= MomentTensorStationContribution::Cast(PublicObject::Find(momentTensorStationContributionChild->publicID()));
		if ( momentTensorStationContributionElement && momentTensorStationContributionElement->parent() == this ) {
			*momentTensorStationContributionElement = *momentTensorStationContributionChild;
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::accept(Visitor* visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;

	for ( std::vector<CommentPtr>::iterator it = _comments.begin(); it != _comments.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<DataUsedPtr>::iterator it = _dataUseds.begin(); it != _dataUseds.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<MomentTensorPhaseSettingPtr>::iterator it = _momentTensorPhaseSettings.begin(); it != _momentTensorPhaseSettings.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<MomentTensorStationContributionPtr>::iterator it = _momentTensorStationContributions.begin(); it != _momentTensorStationContributions.end(); ++it )
		(*it)->accept(visitor);

	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t MomentTensor::commentCount() const {
	return _comments.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment* MomentTensor::comment(size_t i) const {
	return _comments[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment* MomentTensor::comment(const CommentIndex& i) const {
	for ( std::vector<CommentPtr>::const_iterator it = _comments.begin(); it != _comments.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::add(Comment* comment) {
	if ( comment == NULL )
		return false;

	// Element has already a parent
	if ( comment->parent() != NULL ) {
		SEISCOMP_ERROR("MomentTensor::add(Comment*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<CommentPtr>::iterator it = _comments.begin(); it != _comments.end(); ++it ) {
		if ( (*it)->index() == comment->index() ) {
			SEISCOMP_ERROR("MomentTensor::add(Comment*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_comments.push_back(comment);
	comment->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		comment->accept(&nc);
	}

	// Notify registered observers
	childAdded(comment);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::remove(Comment* comment) {
	if ( comment == NULL )
		return false;

	if ( comment->parent() != this ) {
		SEISCOMP_ERROR("MomentTensor::remove(Comment*) -> element has another parent");
		return false;
	}

	std::vector<CommentPtr>::iterator it;
	it = std::find(_comments.begin(), _comments.end(), comment);
	// Element has not been found
	if ( it == _comments.end() ) {
		SEISCOMP_ERROR("MomentTensor::remove(Comment*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_comments.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::removeComment(size_t i) {
	// index out of bounds
	if ( i >= _comments.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_comments[i]->accept(&nc);
	}

	_comments[i]->setParent(NULL);
	childRemoved(_comments[i].get());
	
	_comments.erase(_comments.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::removeComment(const CommentIndex& i) {
	Comment* object = comment(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t MomentTensor::dataUsedCount() const {
	return _dataUseds.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataUsed* MomentTensor::dataUsed(size_t i) const {
	return _dataUseds[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataUsed* MomentTensor::findDataUsed(DataUsed* dataUsed) const {
	std::vector<DataUsedPtr>::const_iterator it;
	for ( it = _dataUseds.begin(); it != _dataUseds.end(); ++it ) {
		if ( *dataUsed == **it )
			return (*it).get();
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::add(DataUsed* dataUsed) {
	if ( dataUsed == NULL )
		return false;

	// Element has already a parent
	if ( dataUsed->parent() != NULL ) {
		SEISCOMP_ERROR("MomentTensor::add(DataUsed*) -> element has already a parent");
		return false;
	}

	// Add the element
	_dataUseds.push_back(dataUsed);
	dataUsed->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		dataUsed->accept(&nc);
	}

	// Notify registered observers
	childAdded(dataUsed);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::remove(DataUsed* dataUsed) {
	if ( dataUsed == NULL )
		return false;

	if ( dataUsed->parent() != this ) {
		SEISCOMP_ERROR("MomentTensor::remove(DataUsed*) -> element has another parent");
		return false;
	}

	std::vector<DataUsedPtr>::iterator it;
	it = std::find(_dataUseds.begin(), _dataUseds.end(), dataUsed);
	// Element has not been found
	if ( it == _dataUseds.end() ) {
		SEISCOMP_ERROR("MomentTensor::remove(DataUsed*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_dataUseds.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::removeDataUsed(size_t i) {
	// index out of bounds
	if ( i >= _dataUseds.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_dataUseds[i]->accept(&nc);
	}

	_dataUseds[i]->setParent(NULL);
	childRemoved(_dataUseds[i].get());
	
	_dataUseds.erase(_dataUseds.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t MomentTensor::momentTensorPhaseSettingCount() const {
	return _momentTensorPhaseSettings.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorPhaseSetting* MomentTensor::momentTensorPhaseSetting(size_t i) const {
	return _momentTensorPhaseSettings[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorPhaseSetting* MomentTensor::momentTensorPhaseSetting(const MomentTensorPhaseSettingIndex& i) const {
	for ( std::vector<MomentTensorPhaseSettingPtr>::const_iterator it = _momentTensorPhaseSettings.begin(); it != _momentTensorPhaseSettings.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::add(MomentTensorPhaseSetting* momentTensorPhaseSetting) {
	if ( momentTensorPhaseSetting == NULL )
		return false;

	// Element has already a parent
	if ( momentTensorPhaseSetting->parent() != NULL ) {
		SEISCOMP_ERROR("MomentTensor::add(MomentTensorPhaseSetting*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<MomentTensorPhaseSettingPtr>::iterator it = _momentTensorPhaseSettings.begin(); it != _momentTensorPhaseSettings.end(); ++it ) {
		if ( (*it)->index() == momentTensorPhaseSetting->index() ) {
			SEISCOMP_ERROR("MomentTensor::add(MomentTensorPhaseSetting*) -> an element with the same index has been added already");
			return false;
		}
	}

	// Add the element
	_momentTensorPhaseSettings.push_back(momentTensorPhaseSetting);
	momentTensorPhaseSetting->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		momentTensorPhaseSetting->accept(&nc);
	}

	// Notify registered observers
	childAdded(momentTensorPhaseSetting);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::remove(MomentTensorPhaseSetting* momentTensorPhaseSetting) {
	if ( momentTensorPhaseSetting == NULL )
		return false;

	if ( momentTensorPhaseSetting->parent() != this ) {
		SEISCOMP_ERROR("MomentTensor::remove(MomentTensorPhaseSetting*) -> element has another parent");
		return false;
	}

	std::vector<MomentTensorPhaseSettingPtr>::iterator it;
	it = std::find(_momentTensorPhaseSettings.begin(), _momentTensorPhaseSettings.end(), momentTensorPhaseSetting);
	// Element has not been found
	if ( it == _momentTensorPhaseSettings.end() ) {
		SEISCOMP_ERROR("MomentTensor::remove(MomentTensorPhaseSetting*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_momentTensorPhaseSettings.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::removeMomentTensorPhaseSetting(size_t i) {
	// index out of bounds
	if ( i >= _momentTensorPhaseSettings.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_momentTensorPhaseSettings[i]->accept(&nc);
	}

	_momentTensorPhaseSettings[i]->setParent(NULL);
	childRemoved(_momentTensorPhaseSettings[i].get());
	
	_momentTensorPhaseSettings.erase(_momentTensorPhaseSettings.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::removeMomentTensorPhaseSetting(const MomentTensorPhaseSettingIndex& i) {
	MomentTensorPhaseSetting* object = momentTensorPhaseSetting(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t MomentTensor::momentTensorStationContributionCount() const {
	return _momentTensorStationContributions.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorStationContribution* MomentTensor::momentTensorStationContribution(size_t i) const {
	return _momentTensorStationContributions[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensorStationContribution* MomentTensor::findMomentTensorStationContribution(const std::string& publicID) const {
	for ( std::vector<MomentTensorStationContributionPtr>::const_iterator it = _momentTensorStationContributions.begin(); it != _momentTensorStationContributions.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::add(MomentTensorStationContribution* momentTensorStationContribution) {
	if ( momentTensorStationContribution == NULL )
		return false;

	// Element has already a parent
	if ( momentTensorStationContribution->parent() != NULL ) {
		SEISCOMP_ERROR("MomentTensor::add(MomentTensorStationContribution*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		MomentTensorStationContribution* momentTensorStationContributionCached = MomentTensorStationContribution::Find(momentTensorStationContribution->publicID());
		if ( momentTensorStationContributionCached ) {
			if ( momentTensorStationContributionCached->parent() ) {
				if ( momentTensorStationContributionCached->parent() == this )
					SEISCOMP_ERROR("MomentTensor::add(MomentTensorStationContribution*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("MomentTensor::add(MomentTensorStationContribution*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				momentTensorStationContribution = momentTensorStationContributionCached;
		}
	}

	// Add the element
	_momentTensorStationContributions.push_back(momentTensorStationContribution);
	momentTensorStationContribution->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		momentTensorStationContribution->accept(&nc);
	}

	// Notify registered observers
	childAdded(momentTensorStationContribution);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::remove(MomentTensorStationContribution* momentTensorStationContribution) {
	if ( momentTensorStationContribution == NULL )
		return false;

	if ( momentTensorStationContribution->parent() != this ) {
		SEISCOMP_ERROR("MomentTensor::remove(MomentTensorStationContribution*) -> element has another parent");
		return false;
	}

	std::vector<MomentTensorStationContributionPtr>::iterator it;
	it = std::find(_momentTensorStationContributions.begin(), _momentTensorStationContributions.end(), momentTensorStationContribution);
	// Element has not been found
	if ( it == _momentTensorStationContributions.end() ) {
		SEISCOMP_ERROR("MomentTensor::remove(MomentTensorStationContribution*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_momentTensorStationContributions.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool MomentTensor::removeMomentTensorStationContribution(size_t i) {
	// index out of bounds
	if ( i >= _momentTensorStationContributions.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_momentTensorStationContributions[i]->accept(&nc);
	}

	_momentTensorStationContributions[i]->setParent(NULL);
	childRemoved(_momentTensorStationContributions[i].get());
	
	_momentTensorStationContributions.erase(_momentTensorStationContributions.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void MomentTensor::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,11>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: MomentTensor skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("derivedOriginID", _derivedOriginID, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("momentMagnitudeID", _momentMagnitudeID, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("scalarMoment", _scalarMoment, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("tensor", _tensor, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("variance", _variance, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("varianceReduction", _varianceReduction, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("doubleCouple", _doubleCouple, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("clvd", _clvd, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("iso", _iso, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("greensFunctionID", _greensFunctionID, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("filterID", _filterID, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("sourceTimeFunction", _sourceTimeFunction, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("methodID", _methodID, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("method", _method, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("status", _status, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("cmtName", _cmtName, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("cmtVersion", _cmtVersion, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("creationInfo", _creationInfo, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("comment",
	                       Seiscomp::Core::Generic::containerMember(_comments,
	                       Seiscomp::Core::Generic::bindMemberFunction<Comment>(static_cast<bool (MomentTensor::*)(Comment*)>(&MomentTensor::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("dataUsed",
	                       Seiscomp::Core::Generic::containerMember(_dataUseds,
	                       Seiscomp::Core::Generic::bindMemberFunction<DataUsed>(static_cast<bool (MomentTensor::*)(DataUsed*)>(&MomentTensor::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("phaseSetting",
	                       Seiscomp::Core::Generic::containerMember(_momentTensorPhaseSettings,
	                       Seiscomp::Core::Generic::bindMemberFunction<MomentTensorPhaseSetting>(static_cast<bool (MomentTensor::*)(MomentTensorPhaseSetting*)>(&MomentTensor::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("stationMomentTensorContribution",
	                       Seiscomp::Core::Generic::containerMember(_momentTensorStationContributions,
	                       Seiscomp::Core::Generic::bindMemberFunction<MomentTensorStationContribution>(static_cast<bool (MomentTensor::*)(MomentTensorStationContribution*)>(&MomentTensor::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
