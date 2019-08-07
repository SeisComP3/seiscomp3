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
#include <seiscomp3/datamodel/strongmotion/rupture.h>
#include <seiscomp3/datamodel/strongmotion/strongorigindescription.h>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


IMPLEMENT_SC_CLASS_DERIVED(Rupture, PublicObject, "Rupture");


namespace {
static Seiscomp::Core::MetaEnumImpl<FwHwIndicator> metaFwHwIndicator;
}


Rupture::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(objectProperty<RealQuantity>("width", "RealQuantity", false, false, true, &Rupture::setWidth, &Rupture::width));
	addProperty(objectProperty<RealQuantity>("displacement", "RealQuantity", false, false, true, &Rupture::setDisplacement, &Rupture::displacement));
	addProperty(objectProperty<RealQuantity>("riseTime", "RealQuantity", false, false, true, &Rupture::setRiseTime, &Rupture::riseTime));
	addProperty(objectProperty<RealQuantity>("vt_to_vs", "RealQuantity", false, false, true, &Rupture::setVtToVs, &Rupture::vtToVs));
	addProperty(objectProperty<RealQuantity>("shallowAsperityDepth", "RealQuantity", false, false, true, &Rupture::setShallowAsperityDepth, &Rupture::shallowAsperityDepth));
	addProperty(Core::simpleProperty("shallowAsperity", "boolean", false, false, false, false, true, false, NULL, &Rupture::setShallowAsperity, &Rupture::shallowAsperity));
	addProperty(objectProperty<LiteratureSource>("literatureSource", "LiteratureSource", false, false, true, &Rupture::setLiteratureSource, &Rupture::literatureSource));
	addProperty(objectProperty<RealQuantity>("slipVelocity", "RealQuantity", false, false, true, &Rupture::setSlipVelocity, &Rupture::slipVelocity));
	addProperty(objectProperty<RealQuantity>("length", "RealQuantity", false, false, true, &Rupture::setLength, &Rupture::length));
	addProperty(objectProperty<RealQuantity>("area", "RealQuantity", false, false, true, &Rupture::setArea, &Rupture::area));
	addProperty(objectProperty<RealQuantity>("ruptureVelocity", "RealQuantity", false, false, true, &Rupture::setRuptureVelocity, &Rupture::ruptureVelocity));
	addProperty(objectProperty<RealQuantity>("stressdrop", "RealQuantity", false, false, true, &Rupture::setStressdrop, &Rupture::stressdrop));
	addProperty(objectProperty<RealQuantity>("momentReleaseTop5km", "RealQuantity", false, false, true, &Rupture::setMomentReleaseTop5km, &Rupture::momentReleaseTop5km));
	addProperty(enumProperty("fwHwIndicator", "FwHwIndicator", false, true, &metaFwHwIndicator, &Rupture::setFwHwIndicator, &Rupture::fwHwIndicator));
	addProperty(Core::simpleProperty("ruptureGeometryWKT", "string", false, false, false, true, false, false, NULL, &Rupture::setRuptureGeometryWKT, &Rupture::ruptureGeometryWKT));
	addProperty(Core::simpleProperty("faultID", "string", false, false, false, false, false, false, NULL, &Rupture::setFaultID, &Rupture::faultID));
	addProperty(objectProperty<SurfaceRupture>("surfaceRupture", "SurfaceRupture", false, false, true, &Rupture::setSurfaceRupture, &Rupture::surfaceRupture));
	addProperty(Core::simpleProperty("centroidReference", "string", false, false, false, false, false, false, NULL, &Rupture::setCentroidReference, &Rupture::centroidReference));
}


IMPLEMENT_METAOBJECT(Rupture)


Rupture::Rupture() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Rupture::Rupture(const Rupture& other)
 : PublicObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Rupture::Rupture(const std::string& publicID)
 : PublicObject(publicID) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Rupture::~Rupture() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Rupture* Rupture::Create() {
	Rupture* object = new Rupture();
	return static_cast<Rupture*>(GenerateId(object));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Rupture* Rupture::Create(const std::string& publicID) {
	if ( Find(publicID) != NULL ) {
		SEISCOMP_ERROR(
			"There exists already a PublicObject with Id '%s'",
			publicID.c_str()
		);
		return NULL;
	}

	return new Rupture(publicID);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Rupture* Rupture::Find(const std::string& publicID) {
	return Rupture::Cast(PublicObject::Find(publicID));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Rupture::operator==(const Rupture& rhs) const {
	if ( _width != rhs._width ) return false;
	if ( _displacement != rhs._displacement ) return false;
	if ( _riseTime != rhs._riseTime ) return false;
	if ( _vtToVs != rhs._vtToVs ) return false;
	if ( _shallowAsperityDepth != rhs._shallowAsperityDepth ) return false;
	if ( _shallowAsperity != rhs._shallowAsperity ) return false;
	if ( _literatureSource != rhs._literatureSource ) return false;
	if ( _slipVelocity != rhs._slipVelocity ) return false;
	if ( _length != rhs._length ) return false;
	if ( _area != rhs._area ) return false;
	if ( _ruptureVelocity != rhs._ruptureVelocity ) return false;
	if ( _stressdrop != rhs._stressdrop ) return false;
	if ( _momentReleaseTop5km != rhs._momentReleaseTop5km ) return false;
	if ( _fwHwIndicator != rhs._fwHwIndicator ) return false;
	if ( _ruptureGeometryWKT != rhs._ruptureGeometryWKT ) return false;
	if ( _faultID != rhs._faultID ) return false;
	if ( _surfaceRupture != rhs._surfaceRupture ) return false;
	if ( _centroidReference != rhs._centroidReference ) return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Rupture::operator!=(const Rupture& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Rupture::equal(const Rupture& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setWidth(const OPT(RealQuantity)& width) {
	_width = width;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& Rupture::width() {
	if ( _width )
		return *_width;
	throw Seiscomp::Core::ValueException("Rupture.width is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& Rupture::width() const {
	if ( _width )
		return *_width;
	throw Seiscomp::Core::ValueException("Rupture.width is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setDisplacement(const OPT(RealQuantity)& displacement) {
	_displacement = displacement;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& Rupture::displacement() {
	if ( _displacement )
		return *_displacement;
	throw Seiscomp::Core::ValueException("Rupture.displacement is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& Rupture::displacement() const {
	if ( _displacement )
		return *_displacement;
	throw Seiscomp::Core::ValueException("Rupture.displacement is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setRiseTime(const OPT(RealQuantity)& riseTime) {
	_riseTime = riseTime;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& Rupture::riseTime() {
	if ( _riseTime )
		return *_riseTime;
	throw Seiscomp::Core::ValueException("Rupture.riseTime is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& Rupture::riseTime() const {
	if ( _riseTime )
		return *_riseTime;
	throw Seiscomp::Core::ValueException("Rupture.riseTime is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setVtToVs(const OPT(RealQuantity)& vtToVs) {
	_vtToVs = vtToVs;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& Rupture::vtToVs() {
	if ( _vtToVs )
		return *_vtToVs;
	throw Seiscomp::Core::ValueException("Rupture.vtToVs is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& Rupture::vtToVs() const {
	if ( _vtToVs )
		return *_vtToVs;
	throw Seiscomp::Core::ValueException("Rupture.vtToVs is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setShallowAsperityDepth(const OPT(RealQuantity)& shallowAsperityDepth) {
	_shallowAsperityDepth = shallowAsperityDepth;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& Rupture::shallowAsperityDepth() {
	if ( _shallowAsperityDepth )
		return *_shallowAsperityDepth;
	throw Seiscomp::Core::ValueException("Rupture.shallowAsperityDepth is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& Rupture::shallowAsperityDepth() const {
	if ( _shallowAsperityDepth )
		return *_shallowAsperityDepth;
	throw Seiscomp::Core::ValueException("Rupture.shallowAsperityDepth is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setShallowAsperity(const OPT(bool)& shallowAsperity) {
	_shallowAsperity = shallowAsperity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Rupture::shallowAsperity() const {
	if ( _shallowAsperity )
		return *_shallowAsperity;
	throw Seiscomp::Core::ValueException("Rupture.shallowAsperity is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setLiteratureSource(const OPT(LiteratureSource)& literatureSource) {
	_literatureSource = literatureSource;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LiteratureSource& Rupture::literatureSource() {
	if ( _literatureSource )
		return *_literatureSource;
	throw Seiscomp::Core::ValueException("Rupture.literatureSource is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const LiteratureSource& Rupture::literatureSource() const {
	if ( _literatureSource )
		return *_literatureSource;
	throw Seiscomp::Core::ValueException("Rupture.literatureSource is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setSlipVelocity(const OPT(RealQuantity)& slipVelocity) {
	_slipVelocity = slipVelocity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& Rupture::slipVelocity() {
	if ( _slipVelocity )
		return *_slipVelocity;
	throw Seiscomp::Core::ValueException("Rupture.slipVelocity is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& Rupture::slipVelocity() const {
	if ( _slipVelocity )
		return *_slipVelocity;
	throw Seiscomp::Core::ValueException("Rupture.slipVelocity is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setLength(const OPT(RealQuantity)& length) {
	_length = length;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& Rupture::length() {
	if ( _length )
		return *_length;
	throw Seiscomp::Core::ValueException("Rupture.length is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& Rupture::length() const {
	if ( _length )
		return *_length;
	throw Seiscomp::Core::ValueException("Rupture.length is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setArea(const OPT(RealQuantity)& area) {
	_area = area;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& Rupture::area() {
	if ( _area )
		return *_area;
	throw Seiscomp::Core::ValueException("Rupture.area is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& Rupture::area() const {
	if ( _area )
		return *_area;
	throw Seiscomp::Core::ValueException("Rupture.area is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setRuptureVelocity(const OPT(RealQuantity)& ruptureVelocity) {
	_ruptureVelocity = ruptureVelocity;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& Rupture::ruptureVelocity() {
	if ( _ruptureVelocity )
		return *_ruptureVelocity;
	throw Seiscomp::Core::ValueException("Rupture.ruptureVelocity is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& Rupture::ruptureVelocity() const {
	if ( _ruptureVelocity )
		return *_ruptureVelocity;
	throw Seiscomp::Core::ValueException("Rupture.ruptureVelocity is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setStressdrop(const OPT(RealQuantity)& stressdrop) {
	_stressdrop = stressdrop;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& Rupture::stressdrop() {
	if ( _stressdrop )
		return *_stressdrop;
	throw Seiscomp::Core::ValueException("Rupture.stressdrop is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& Rupture::stressdrop() const {
	if ( _stressdrop )
		return *_stressdrop;
	throw Seiscomp::Core::ValueException("Rupture.stressdrop is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setMomentReleaseTop5km(const OPT(RealQuantity)& momentReleaseTop5km) {
	_momentReleaseTop5km = momentReleaseTop5km;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
RealQuantity& Rupture::momentReleaseTop5km() {
	if ( _momentReleaseTop5km )
		return *_momentReleaseTop5km;
	throw Seiscomp::Core::ValueException("Rupture.momentReleaseTop5km is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const RealQuantity& Rupture::momentReleaseTop5km() const {
	if ( _momentReleaseTop5km )
		return *_momentReleaseTop5km;
	throw Seiscomp::Core::ValueException("Rupture.momentReleaseTop5km is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setFwHwIndicator(const OPT(FwHwIndicator)& fwHwIndicator) {
	_fwHwIndicator = fwHwIndicator;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
FwHwIndicator Rupture::fwHwIndicator() const {
	if ( _fwHwIndicator )
		return *_fwHwIndicator;
	throw Seiscomp::Core::ValueException("Rupture.fwHwIndicator is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setRuptureGeometryWKT(const std::string& ruptureGeometryWKT) {
	_ruptureGeometryWKT = ruptureGeometryWKT;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Rupture::ruptureGeometryWKT() const {
	return _ruptureGeometryWKT;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setFaultID(const std::string& faultID) {
	_faultID = faultID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Rupture::faultID() const {
	return _faultID;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setSurfaceRupture(const OPT(SurfaceRupture)& surfaceRupture) {
	_surfaceRupture = surfaceRupture;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SurfaceRupture& Rupture::surfaceRupture() {
	if ( _surfaceRupture )
		return *_surfaceRupture;
	throw Seiscomp::Core::ValueException("Rupture.surfaceRupture is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const SurfaceRupture& Rupture::surfaceRupture() const {
	if ( _surfaceRupture )
		return *_surfaceRupture;
	throw Seiscomp::Core::ValueException("Rupture.surfaceRupture is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::setCentroidReference(const std::string& centroidReference) {
	_centroidReference = centroidReference;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& Rupture::centroidReference() const {
	return _centroidReference;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
StrongOriginDescription* Rupture::strongOriginDescription() const {
	return static_cast<StrongOriginDescription*>(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Rupture& Rupture::operator=(const Rupture& other) {
	PublicObject::operator=(other);
	_width = other._width;
	_displacement = other._displacement;
	_riseTime = other._riseTime;
	_vtToVs = other._vtToVs;
	_shallowAsperityDepth = other._shallowAsperityDepth;
	_shallowAsperity = other._shallowAsperity;
	_literatureSource = other._literatureSource;
	_slipVelocity = other._slipVelocity;
	_length = other._length;
	_area = other._area;
	_ruptureVelocity = other._ruptureVelocity;
	_stressdrop = other._stressdrop;
	_momentReleaseTop5km = other._momentReleaseTop5km;
	_fwHwIndicator = other._fwHwIndicator;
	_ruptureGeometryWKT = other._ruptureGeometryWKT;
	_faultID = other._faultID;
	_surfaceRupture = other._surfaceRupture;
	_centroidReference = other._centroidReference;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Rupture::assign(Object* other) {
	Rupture* otherRupture = Rupture::Cast(other);
	if ( other == NULL )
		return false;

	*this = *otherRupture;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Rupture::attachTo(PublicObject* parent) {
	if ( parent == NULL ) return false;

	// check all possible parents
	StrongOriginDescription* strongOriginDescription = StrongOriginDescription::Cast(parent);
	if ( strongOriginDescription != NULL )
		return strongOriginDescription->add(this);

	SEISCOMP_ERROR("Rupture::attachTo(%s) -> wrong class type", parent->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Rupture::detachFrom(PublicObject* object) {
	if ( object == NULL ) return false;

	// check all possible parents
	StrongOriginDescription* strongOriginDescription = StrongOriginDescription::Cast(object);
	if ( strongOriginDescription != NULL ) {
		// If the object has been added already to the parent locally
		// just remove it by pointer
		if ( object == parent() )
			return strongOriginDescription->remove(this);
		// The object has not been added locally so it must be looked up
		else {
			Rupture* child = strongOriginDescription->findRupture(publicID());
			if ( child != NULL )
				return strongOriginDescription->remove(child);
			else {
				SEISCOMP_DEBUG("Rupture::detachFrom(StrongOriginDescription): rupture has not been found");
				return false;
			}
		}
	}

	SEISCOMP_ERROR("Rupture::detachFrom(%s) -> wrong class type", object->className());
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Rupture::detach() {
	if ( parent() == NULL )
		return false;

	return detachFrom(parent());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Object* Rupture::clone() const {
	Rupture* clonee = new Rupture();
	*clonee = *this;
	return clonee;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Rupture::updateChild(Object* child) {
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::accept(Visitor* visitor) {
	if ( visitor->traversal() == Visitor::TM_TOPDOWN )
		if ( !visitor->visit(this) )
			return;


	if ( visitor->traversal() == Visitor::TM_BOTTOMUP )
		visitor->visit(this);
	else
		visitor->finished();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Rupture::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,11>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: Rupture skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	PublicObject::serialize(ar);
	if ( !ar.success() ) return;

	ar & NAMED_OBJECT_HINT("width", _width, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("displacement", _displacement, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("riseTime", _riseTime, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("vt_to_vs", _vtToVs, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("shallowAsperityDepth", _shallowAsperityDepth, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("shallowAsperity", _shallowAsperity, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("literatureSource", _literatureSource, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("slipVelocity", _slipVelocity, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("length", _length, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("area", _area, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("ruptureVelocity", _ruptureVelocity, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("stressdrop", _stressdrop, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("momentReleaseTop5km", _momentReleaseTop5km, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("fwHwIndicator", _fwHwIndicator, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("ruptureGeometryWKT", _ruptureGeometryWKT, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("faultID", _faultID, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("surfaceRupture", _surfaceRupture, Archive::STATIC_TYPE | Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("centroidReference", _centroidReference, Archive::XML_ELEMENT);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
