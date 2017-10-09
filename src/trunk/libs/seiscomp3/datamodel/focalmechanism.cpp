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
#include <seiscomp3/datamodel/focalmechanism.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/momenttensor.h>
#include <algorithm>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {


IMPLEMENT_SC_CLASS_DERIVED(FocalMechanism, PublicObject, "FocalMechanism");


namespace {
static Seiscomp::Core::MetaEnumImpl<EvaluationMode> metaEvaluationMode;
static Seiscomp::Core::MetaEnumImpl<EvaluationStatus> metaEvaluationStatus;
}


FocalMechanism::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("triggeringOriginID", "string", false, false, false, true, false, false, NULL, &FocalMechanism::setTriggeringOriginID, &FocalMechanism::triggeringOriginID));
	addProperty(objectProperty<NodalPlanes>("nodalPlanes", "NodalPlanes", false, false, true, &FocalMechanism::setNodalPlanes, &FocalMechanism::nodalPlanes));
	addProperty(objectProperty<PrincipalAxes>("principalAxes", "PrincipalAxes", false, false, true, &FocalMechanism::setPrincipalAxes, &FocalMechanism::principalAxes));
	addProperty(Core::simpleProperty("azimuthalGap", "float", false, false, false, false, true, false, NULL, &FocalMechanism::setAzimuthalGap, &FocalMechanism::azimuthalGap));
	addProperty(Core::simpleProperty("stationPolarityCount", "int", false, false, false, false, true, false, NULL, &FocalMechanism::setStationPolarityCount, &FocalMechanism::stationPolarityCount));
	addProperty(Core::simpleProperty("misfit", "float", false, false, false, false, true, false, NULL, &FocalMechanism::setMisfit, &FocalMechanism::misfit));
	addProperty(Core::simpleProperty("stationDistributionRatio", "float", false, false, false, false, true, false, NULL, &FocalMechanism::setStationDistributionRatio, &FocalMechanism::stationDistributionRatio));
	addProperty(Core::simpleProperty("methodID", "string", false, false, false, false, false, false, NULL, &FocalMechanism::setMethodID, &FocalMechanism::methodID));
	addProperty(enumProperty("evaluationMode", "EvaluationMode", false, true, &metaEvaluationMode, &FocalMechanism::setEvaluationMode, &FocalMechanism::evaluationMode));
	addProperty(enumProperty("evaluationStatus", "EvaluationStatus", false, true, &metaEvaluationStatus, &FocalMechanism::setEvaluationStatus, &FocalMechanism::evaluationStatus));
	addProperty(objectProperty<CreationInfo>("creationInfo", "CreationInfo", false, false, true, &FocalMechanism::setCreationInfo, &FocalMechanism::creationInfo));
	addProperty(arrayClassProperty<Comment>("comment", "Comment", &FocalMechanism::commentCount, &FocalMechanism::comment, static_cast<bool (FocalMechanism::*)(Comment*)>(&FocalMechanism::add), &FocalMechanism::removeComment, static_cast<bool (FocalMechanism::*)(Comment*)>(&FocalMechanism::remove)));
	addProperty(arrayObjectProperty("momentTensor", "MomentTensor", &FocalMechanism::momentTensorCount, &FocalMechanism::momentTensor, static_cast<bool (FocalMechanism::*)(MomentTensor*)>(&FocalMechanism::add), &FocalMechanism::removeMomentTensor, static_cast<bool (FocalMechanism::*)(MomentTensor*)>(&FocalMechanism::remove)));
}


IMPLEMENT_METAOBJECT(FocalMechanism)


FocalMechanism::FocalMechanism() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FocalMechanism::FocalMechanism(const FocalMechanism& other)
 : PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FocalMechanism::FocalMechanism(const std::string& publicID)
 : PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FocalMechanism::~FocalMechanism() {
	std::for_each(_comments.begin(), _comments.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&Comment::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&CommentPtr::get)));
	std::for_each(_momentTensors.begin(), _momentTensors.end(),
	              std::compose1(std::bind2nd(std::mem_fun(&MomentTensor::setParent),
	                                         (PublicObject*)NULL),
	                            std::mem_fun_ref(&MomentTensorPtr::get)));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FocalMechanism* FocalMechanism::Create() {
	FocalMechanism* object = new FocalMechanism();
	return static_cast<FocalMechanism*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FocalMechanism* FocalMechanism::Create(const std::string& publicID) {
	if ( PublicObject::IsRegistrationEnabled() && Find(publicID) != NULL ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return NULL;
	}

	return new FocalMechanism(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FocalMechanism* FocalMechanism::Find(const std::string& publicID) {
	return FocalMechanism::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FocalMechanism::operator==(const FocalMechanism& rhs) const {
	if ( _triggeringOriginID != rhs._triggeringOriginID ) return false;
	if ( _nodalPlanes != rhs._nodalPlanes ) return false;
	if ( _principalAxes != rhs._principalAxes ) return false;
	if ( _azimuthalGap != rhs._azimuthalGap ) return false;
	if ( _stationPolarityCount != rhs._stationPolarityCount ) return false;
	if ( _misfit != rhs._misfit ) return false;
	if ( _stationDistributionRatio != rhs._stationDistributionRatio ) return false;
	if ( _methodID != rhs._methodID ) return false;
	if ( _evaluationMode != rhs._evaluationMode ) return false;
	if ( _evaluationStatus != rhs._evaluationStatus ) return false;
	if ( _creationInfo != rhs._creationInfo ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FocalMechanism::operator!=(const FocalMechanism& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FocalMechanism::equal(const FocalMechanism& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FocalMechanism::setTriggeringOriginID(const std::string& triggeringOriginID) {
	_triggeringOriginID = triggeringOriginID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& FocalMechanism::triggeringOriginID() const {
	return _triggeringOriginID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FocalMechanism::setNodalPlanes(const OPT(NodalPlanes)& nodalPlanes) {
	_nodalPlanes = nodalPlanes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
NodalPlanes& FocalMechanism::nodalPlanes() {
	if ( _nodalPlanes )
		return *_nodalPlanes;
	throw Seiscomp::Core::ValueException("FocalMechanism.nodalPlanes is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const NodalPlanes& FocalMechanism::nodalPlanes() const {
	if ( _nodalPlanes )
		return *_nodalPlanes;
	throw Seiscomp::Core::ValueException("FocalMechanism.nodalPlanes is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FocalMechanism::setPrincipalAxes(const OPT(PrincipalAxes)& principalAxes) {
	_principalAxes = principalAxes;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
PrincipalAxes& FocalMechanism::principalAxes() {
	if ( _principalAxes )
		return *_principalAxes;
	throw Seiscomp::Core::ValueException("FocalMechanism.principalAxes is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const PrincipalAxes& FocalMechanism::principalAxes() const {
	if ( _principalAxes )
		return *_principalAxes;
	throw Seiscomp::Core::ValueException("FocalMechanism.principalAxes is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FocalMechanism::setAzimuthalGap(const OPT(double)& azimuthalGap) {
	_azimuthalGap = azimuthalGap;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double FocalMechanism::azimuthalGap() const {
	if ( _azimuthalGap )
		return *_azimuthalGap;
	throw Seiscomp::Core::ValueException("FocalMechanism.azimuthalGap is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FocalMechanism::setStationPolarityCount(const OPT(int)& stationPolarityCount) {
	_stationPolarityCount = stationPolarityCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int FocalMechanism::stationPolarityCount() const {
	if ( _stationPolarityCount )
		return *_stationPolarityCount;
	throw Seiscomp::Core::ValueException("FocalMechanism.stationPolarityCount is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FocalMechanism::setMisfit(const OPT(double)& misfit) {
	_misfit = misfit;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double FocalMechanism::misfit() const {
	if ( _misfit )
		return *_misfit;
	throw Seiscomp::Core::ValueException("FocalMechanism.misfit is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FocalMechanism::setStationDistributionRatio(const OPT(double)& stationDistributionRatio) {
	_stationDistributionRatio = stationDistributionRatio;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
double FocalMechanism::stationDistributionRatio() const {
	if ( _stationDistributionRatio )
		return *_stationDistributionRatio;
	throw Seiscomp::Core::ValueException("FocalMechanism.stationDistributionRatio is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FocalMechanism::setMethodID(const std::string& methodID) {
	_methodID = methodID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& FocalMechanism::methodID() const {
	return _methodID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FocalMechanism::setEvaluationMode(const OPT(EvaluationMode)& evaluationMode) {
	_evaluationMode = evaluationMode;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EvaluationMode FocalMechanism::evaluationMode() const {
	if ( _evaluationMode )
		return *_evaluationMode;
	throw Seiscomp::Core::ValueException("FocalMechanism.evaluationMode is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FocalMechanism::setEvaluationStatus(const OPT(EvaluationStatus)& evaluationStatus) {
	_evaluationStatus = evaluationStatus;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EvaluationStatus FocalMechanism::evaluationStatus() const {
	if ( _evaluationStatus )
		return *_evaluationStatus;
	throw Seiscomp::Core::ValueException("FocalMechanism.evaluationStatus is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FocalMechanism::setCreationInfo(const OPT(CreationInfo)& creationInfo) {
	_creationInfo = creationInfo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
CreationInfo& FocalMechanism::creationInfo() {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("FocalMechanism.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const CreationInfo& FocalMechanism::creationInfo() const {
	if ( _creationInfo )
		return *_creationInfo;
	throw Seiscomp::Core::ValueException("FocalMechanism.creationInfo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
EventParameters* FocalMechanism::eventParameters() const {
	return static_cast<EventParameters*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FocalMechanism& FocalMechanism::operator=(const FocalMechanism& other) {
	PublicObject::operator=(other);
	_triggeringOriginID = other._triggeringOriginID;
	_nodalPlanes = other._nodalPlanes;
	_principalAxes = other._principalAxes;
	_azimuthalGap = other._azimuthalGap;
	_stationPolarityCount = other._stationPolarityCount;
	_misfit = other._misfit;
	_stationDistributionRatio = other._stationDistributionRatio;
	_methodID = other._methodID;
	_evaluationMode = other._evaluationMode;
	_evaluationStatus = other._evaluationStatus;
	_creationInfo = other._creationInfo;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FocalMechanism::assign(Object* other) {
	FocalMechanism* otherFocalMechanism = FocalMechanism::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherFocalMechanism;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FocalMechanism::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	EventParameters* eventParameters = EventParameters::Cast(parent);
	if ( eventParameters != NULL )
		return eventParameters->add(this);

	SEISCOMP_ERROR("FocalMechanism::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FocalMechanism::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	EventParameters* eventParameters = EventParameters::Cast(object);
	if ( eventParameters != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return eventParameters->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			FocalMechanism* child = eventParameters->findFocalMechanism(publicID());
			if ( child != NULL )
				return eventParameters->remove(child);
			else {
				SEISCOMP_DEBUG("FocalMechanism::detachFrom(EventParameters): focalMechanism has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("FocalMechanism::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FocalMechanism::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* FocalMechanism::clone() const {
	FocalMechanism* clonee = new FocalMechanism();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FocalMechanism::updateChild(Object* child) {
	Comment* commentChild = Comment::Cast(child);
	if ( commentChild != NULL ) {
		Comment* commentElement = comment(commentChild->index());
		if ( commentElement != NULL ) {
			*commentElement = *commentChild;
			return true;
		}
		return false;
	}

	MomentTensor* momentTensorChild = MomentTensor::Cast(child);
	if ( momentTensorChild != NULL ) {
		MomentTensor* momentTensorElement
			= MomentTensor::Cast(PublicObject::Find(momentTensorChild->publicID()));
		if ( momentTensorElement && momentTensorElement->parent() == this ) {
			*momentTensorElement = *momentTensorChild;
			return true;
		}
		return false;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FocalMechanism::accept(Visitor* visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;

	for ( std::vector<CommentPtr>::iterator it = _comments.begin(); it != _comments.end(); ++it )
		(*it)->accept(visitor);
	for ( std::vector<MomentTensorPtr>::iterator it = _momentTensors.begin(); it != _momentTensors.end(); ++it )
		(*it)->accept(visitor);

	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t FocalMechanism::commentCount() const {
	return _comments.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment* FocalMechanism::comment(size_t i) const {
	return _comments[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Comment* FocalMechanism::comment(const CommentIndex& i) const {
	for ( std::vector<CommentPtr>::const_iterator it = _comments.begin(); it != _comments.end(); ++it )
		if ( i == (*it)->index() )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FocalMechanism::add(Comment* comment) {
	if ( comment == NULL )
		return false;

	// Element has already a parent
	if ( comment->parent() != NULL ) {
		SEISCOMP_ERROR("FocalMechanism::add(Comment*) -> element has already a parent");
		return false;
	}

	// Duplicate index check
	for ( std::vector<CommentPtr>::iterator it = _comments.begin(); it != _comments.end(); ++it ) {
		if ( (*it)->index() == comment->index() ) {
			SEISCOMP_ERROR("FocalMechanism::add(Comment*) -> an element with the same index has been added already");
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
bool FocalMechanism::remove(Comment* comment) {
	if ( comment == NULL )
		return false;

	if ( comment->parent() != this ) {
		SEISCOMP_ERROR("FocalMechanism::remove(Comment*) -> element has another parent");
		return false;
	}

	std::vector<CommentPtr>::iterator it;
	it = std::find(_comments.begin(), _comments.end(), comment);
	// Element has not been found
	if ( it == _comments.end() ) {
		SEISCOMP_ERROR("FocalMechanism::remove(Comment*) -> child object has not been found although the parent pointer matches???");
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
bool FocalMechanism::removeComment(size_t i) {
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
bool FocalMechanism::removeComment(const CommentIndex& i) {
	Comment* object = comment(i);
	if ( object == NULL ) return false;
	return remove(object);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
size_t FocalMechanism::momentTensorCount() const {
	return _momentTensors.size();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensor* FocalMechanism::momentTensor(size_t i) const {
	return _momentTensors[i].get();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
MomentTensor* FocalMechanism::findMomentTensor(const std::string& publicID) const {
	for ( std::vector<MomentTensorPtr>::const_iterator it = _momentTensors.begin(); it != _momentTensors.end(); ++it )
		if ( (*it)->publicID() == publicID )
			return (*it).get();

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FocalMechanism::add(MomentTensor* momentTensor) {
	if ( momentTensor == NULL )
		return false;

	// Element has already a parent
	if ( momentTensor->parent() != NULL ) {
		SEISCOMP_ERROR("FocalMechanism::add(MomentTensor*) -> element has already a parent");
		return false;
	}

	if ( PublicObject::IsRegistrationEnabled() ) {
		MomentTensor* momentTensorCached = MomentTensor::Find(momentTensor->publicID());
		if ( momentTensorCached ) {
			if ( momentTensorCached->parent() ) {
				if ( momentTensorCached->parent() == this )
					SEISCOMP_ERROR("FocalMechanism::add(MomentTensor*) -> element with same publicID has been added already");
				else
					SEISCOMP_ERROR("FocalMechanism::add(MomentTensor*) -> element with same publicID has been added already to another object");
				return false;
			}
			else
				momentTensor = momentTensorCached;
		}
	}

	// Add the element
	_momentTensors.push_back(momentTensor);
	momentTensor->setParent(this);

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_ADD);
		momentTensor->accept(&nc);
	}

	// Notify registered observers
	childAdded(momentTensor);
	
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FocalMechanism::remove(MomentTensor* momentTensor) {
	if ( momentTensor == NULL )
		return false;

	if ( momentTensor->parent() != this ) {
		SEISCOMP_ERROR("FocalMechanism::remove(MomentTensor*) -> element has another parent");
		return false;
	}

	std::vector<MomentTensorPtr>::iterator it;
	it = std::find(_momentTensors.begin(), _momentTensors.end(), momentTensor);
	// Element has not been found
	if ( it == _momentTensors.end() ) {
		SEISCOMP_ERROR("FocalMechanism::remove(MomentTensor*) -> child object has not been found although the parent pointer matches???");
		return false;
	}

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		(*it)->accept(&nc);
	}

	(*it)->setParent(NULL);
	childRemoved((*it).get());
	
	_momentTensors.erase(it);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool FocalMechanism::removeMomentTensor(size_t i) {
	// index out of bounds
	if ( i >= _momentTensors.size() )
		return false;

	// Create the notifiers
	if ( Notifier::IsEnabled() ) {
		NotifierCreator nc(OP_REMOVE);
		_momentTensors[i]->accept(&nc);
	}

	_momentTensors[i]->setParent(NULL);
	childRemoved(_momentTensors[i].get());
	
	_momentTensors.erase(_momentTensors.begin() + i);

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void FocalMechanism::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,10>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: FocalMechanism skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("triggeringOriginID", _triggeringOriginID, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("nodalPlanes", _nodalPlanes, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("principalAxes", _principalAxes, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("azimuthalGap", _azimuthalGap, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("stationPolarityCount", _stationPolarityCount, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("misfit", _misfit, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("stationDistributionRatio", _stationDistributionRatio, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("methodID", _methodID, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("evaluationMode", _evaluationMode, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("evaluationStatus", _evaluationStatus, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("creationInfo", _creationInfo, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	if ( ar.hint() & Archive::IGNORE_CHILDS ) return;
	ar & NAMED_OBJECT_HINT("comment",
	                       Seiscomp::Core::Generic::containerMember(_comments,
	                       Seiscomp::Core::Generic::bindMemberFunction<Comment>(static_cast<bool (FocalMechanism::*)(Comment*)>(&FocalMechanism::add), this)),
	                       Archive::STATIC_TYPE);
	ar & NAMED_OBJECT_HINT("momentTensor",
	                       Seiscomp::Core::Generic::containerMember(_momentTensors,
	                       Seiscomp::Core::Generic::bindMemberFunction<MomentTensor>(static_cast<bool (FocalMechanism::*)(MomentTensor*)>(&FocalMechanism::add), this)),
	                       Archive::STATIC_TYPE);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
