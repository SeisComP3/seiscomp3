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
#define SEISCOMP_COMPONENT DataModelUtils

#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/core/metaobject.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/eventdescription.h>
#include <seiscomp3/datamodel/inventory.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/sensorlocation.h>
#include <seiscomp3/datamodel/stream.h>
#include <seiscomp3/datamodel/configstation.h>
#include <seiscomp3/datamodel/setup.h>
#include <seiscomp3/datamodel/utils.h>
#include <seiscomp3/math/vector3.h>
#include <seiscomp3/logging/log.h>
#include <cmath>


#include <cstring>
#include <deque>
#include <iostream>

namespace Seiscomp {
namespace DataModel {

namespace {

class CloneVisitor : public Visitor {
	public:
		CloneVisitor() : Visitor(TM_TOPDOWN) { reset(); }

		// Reset should be called before the object is used
		// a second time.
		void reset() {
			_clone = NULL;
			_parents.clear();
		}

		bool visit(PublicObject *po) {
			PublicObject *clone = static_cast<PublicObject*>(po->clone());
			if ( !commit(clone) ) return false;
			_parents.push_front(clone);
			return true;
		}

		void finished() {
			_parents.pop_front();
		}

		void visit(Object *o) {
			Object *clone = o->clone();
			commit(clone);
		}

		Object *result() const {
			return _clone;
		}

	private:
		bool commit(Object *o) {
			if ( !_parents.empty() ) {
				if ( !o->attachTo(_parents.front()) ) {
					delete o;
					o = NULL;
					return false;
				}
			}

			if ( _clone == NULL ) _clone = o;
			return true;
		}

	private:
		std::deque<PublicObject*> _parents;
		Object *_clone;
};

}

namespace {
	std::ostream &operator<<(std::ostream &os, const ComplexArray &ar) {
		os << "Complex Array of " << ar.content().size() << " elements";
		return os;
	}
	
	std::ostream &operator<<(std::ostream &os, const RealArray &ar) {
		os << "Real Array of " << ar.content().size() << " elements";
		return os;
	}
	
	std::ostream &operator<<(std::ostream &os, const Blob &ar) {
		os << "Blob size: " << ar.content().size();
		return os;
	}
}

ThreeComponents::ThreeComponents() {
	comps[0] = NULL;
	comps[1] = NULL;
	comps[2] = NULL;
}


std::string eventRegion(const Event *ev) {
	for ( size_t i = 0; i < ev->eventDescriptionCount(); ++i ) {
		EventDescription *ed = ev->eventDescription(i);
		try {
			if ( ed->type() == REGION_NAME ) {
				return ed->text();
			}
		}
		catch ( ... ) {}
	}
	return "";
}


Station* getStation(const Inventory *inventory,
                    const std::string &networkCode,
                    const std::string &stationCode,
                    const Core::Time &time) {
	if ( inventory == NULL )
		return NULL;

	for ( size_t i = 0; i < inventory->networkCount(); ++i ) {
		DataModel::Network* network = inventory->network(i);
		if ( network->code() != networkCode ) continue;

		try {
			if ( network->end() < time ) continue;
		}
		catch (...) {
		}

		if ( network->start() > time ) continue;

		for ( size_t j = 0; j < network->stationCount(); ++j ) {
			DataModel::Station* station = network->station(j);
			if ( station->code() != stationCode ) continue;

			try {
				if ( station->end() < time ) continue;
			}
			catch (...) {}

			if ( station->start() > time ) continue;

			return station;
		}
	}

	return NULL;
}


SensorLocation* getSensorLocation(const Inventory *inventory,
                                  const std::string &networkCode,
                                  const std::string &stationCode,
                                  const std::string &locationCode,
                                  const Core::Time &time) {
	/*
	DataModel::Station *sta = getStation(inventory, networkCode, stationCode, time);
	if ( sta == NULL )
		return NULL;

	for ( size_t i = 0; i < sta->sensorLocationCount(); ++i ) {
		DataModel::SensorLocation* loc = sta->sensorLocation(i);
		if ( loc->code() != locationCode ) continue;

		try {
			if ( loc->end() <= time ) continue;
		}
		catch (...) {}

		if ( loc->start() > time ) continue;

		return loc;
	}

	return NULL;
	*/

	if ( inventory == NULL )
		return NULL;

	for ( size_t i = 0; i < inventory->networkCount(); ++i ) {
		DataModel::Network* network = inventory->network(i);
		if ( network->code() != networkCode ) continue;

		for ( size_t j = 0; j < network->stationCount(); ++j ) {
			DataModel::Station* sta = network->station(j);
			if ( sta->code() != stationCode ) continue;

			for ( size_t i = 0; i < sta->sensorLocationCount(); ++i ) {
				DataModel::SensorLocation* loc = sta->sensorLocation(i);
				if ( loc->code() != locationCode ) continue;

				try {
					if ( loc->end() <= time ) continue;
				}
				catch (...) {}

				if ( loc->start() > time ) continue;

				return loc;
			}
		}
	}

	return NULL;
}


Stream* getStream(const Inventory *inventory,
                  const std::string &networkCode,
                  const std::string &stationCode,
                  const std::string &locationCode,
                  const std::string &channelCode,
                  const Core::Time &time) {
	DataModel::SensorLocation *loc = getSensorLocation(inventory, networkCode, stationCode, locationCode, time);
	if ( loc == NULL )
		return NULL;

	for ( size_t i = 0; i < loc->streamCount(); ++i ) {
		DataModel::Stream *stream = loc->stream(i);
		if ( stream->code() != channelCode ) continue;

		try {
			if ( stream->end() <= time ) continue;
		}
		catch (...) {}

		if ( stream->start() > time ) continue;

		return stream;
	}

	return NULL;
}


Station* getStation(const Inventory *inventory, const Pick *pick) {
	if ( pick == NULL ) return NULL;

	return getStation(inventory, pick->waveformID().networkCode(),
	                  pick->waveformID().stationCode(),
	                  pick->time().value());
}


SensorLocation* getSensorLocation(const Inventory *inventory,
                                  const Pick *pick) {
	if ( pick == NULL ) return NULL;

	return getSensorLocation(inventory, pick->waveformID().networkCode(),
	                         pick->waveformID().stationCode(), pick->waveformID().locationCode(),
	                         pick->time().value());
}


Stream* getStream(const Inventory *inventory,
                  const Pick *pick) {
	if ( pick == NULL ) return NULL;

	return getStream(inventory, pick->waveformID().networkCode(),
	                 pick->waveformID().stationCode(), pick->waveformID().locationCode(),
	                 pick->waveformID().channelCode(), pick->time().value());
}


Stream *getVerticalComponent(const SensorLocation *loc, const char *streamCode, const Core::Time &time) {
	int len = strlen(streamCode);

	Stream *best = NULL;
	float maxCorr = -100;

	for ( size_t i = 0; i < loc->streamCount(); ++i ) {
		Stream *stream = loc->stream(i);

		try {
			if ( stream->end() < time ) continue;
		}
		catch (...) {}

		if ( stream->start() > time ) continue;

		if ( stream->code().compare(0, len, streamCode) )
			continue;

		try {
			// Z is up vector. Since we do not care about the sign
			// the maximum absolute value will be selected.
			float z = (float)fabs(sin(-deg2rad(stream->dip())));

			if ( z > maxCorr ) {
				maxCorr = z;
				best = stream;
			}
		}
		catch ( ... ) {}
	}

	return best;
}


namespace {


struct ComponentAxis {
	ComponentAxis(Stream *m, const Math::Vector3f &ax) : model(m), axis(ax) {}

	Stream *model;
	Math::Vector3f axis;
};


bool by_Z_desc_and_code_asc(const ComponentAxis &axis1, const ComponentAxis &axis2) {
	if ( fabs(axis1.axis.z) > fabs(axis2.axis.z) ) return true;
	if ( fabs(axis1.axis.z) < fabs(axis2.axis.z) ) return false;
	if ( axis1.axis.z > axis2.axis.z ) return true;
	if ( axis1.axis.z < axis2.axis.z ) return false;
	return axis1.model->code() < axis2.model->code();
}


}


bool getThreeComponents(ThreeComponents &res, const SensorLocation *loc, const char *streamCode, const Core::Time &time) {
	int len = strlen(streamCode);

	res = ThreeComponents();

	std::vector<ComponentAxis> comps;

	for ( size_t i = 0; i < loc->streamCount(); ++i ) {
		Stream *stream = loc->stream(i);

		try {
			if ( stream->end() < time ) continue;
		}
		catch (...) {}

		if ( stream->start() > time ) continue;

		if ( stream->code().compare(0, len, streamCode) )
			continue;

		try {
			float azi = (float)deg2rad(stream->azimuth());
			float dip = (float)deg2rad(-stream->dip());

			Math::Vector3f axis;

			axis.fromAngles(azi, dip);

			// Check if there are already linearly dependent components
			bool newOrthogonal = true;

			for ( size_t a = 0; a < comps.size(); ++a ) {
				float alpha = comps[a].axis.dot(axis);

				// Allow one degree of uncertainty
				if ( alpha > 0.0174524064373 ) {
					newOrthogonal = false;
					break;
				}
			}

			if ( !newOrthogonal ) continue;

			comps.push_back(ComponentAxis(stream, axis));
		}
		catch ( ... ) {}
	}

	if ( comps.empty() ) return false;

	std::sort(comps.begin(), comps.end(), by_Z_desc_and_code_asc);
	res.comps[ThreeComponents::Vertical] = comps[0].model;

	if ( comps.size() < 3 ) return false;

	// Select the first horizontal left (math. positive) from the second
	Math::Vector3f cross;
	cross.cross(comps[1].axis, comps[2].axis);

	if ( cross.dot(comps[0].axis) > 0 )
		std::swap(comps[1], comps[2]);

	res.comps[ThreeComponents::FirstHorizontal] = comps[1].model;
	res.comps[ThreeComponents::SecondHorizontal] = comps[2].model;

	/*
	std::cout << res.comps[ThreeComponents::FirstHorizontal]->code() << ":   H1: " << comps[1].axis.x << ", " << comps[1].axis.y << ", " << comps[1].axis.z << std::endl;
	std::cout << res.comps[ThreeComponents::SecondHorizontal]->code() << ":   H2: " << comps[2].axis.x << ", " << comps[2].axis.y << ", " << comps[2].axis.z << std::endl;
	std::cout << res.comps[ThreeComponents::Vertical]->code() << ":    Z: " << comps[0].axis.x << ", " << comps[0].axis.y << ", " << comps[0].axis.z << std::endl;
	std::cout << "syn Z: " << cross.x << ", " << cross.y << ", " << cross.z << std::endl;
	*/

	return true;
}


Setup *findSetup(const ConfigStation *cs, const std::string &setupName,
                 bool allowGlobal) {
	DataModel::Setup *configSetup = NULL;

	// Find setup with setupName or "default" as fallback
	for ( size_t i = 0; i < cs->setupCount(); ++i ) {
		DataModel::Setup *setup = cs->setup(i);
		if ( !setup->enabled() ) continue;

		if ( allowGlobal && setup->name() == "default" ) {
			// Found "default", remember it and continue
			configSetup = setup;
			continue;
		}

		// Found the right one, break the loop
		if ( setup->name() == setupName )
			return setup;
	}

	return configSetup;
}



Object *copy(Object* obj) {
	CloneVisitor cv;
	obj->accept(&cv);
	return cv.result();
}


///////////////////////////////////////////////////////////////////////////////
// DataModel Diff / Merge
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// DiffMerge::LogNode
///////////////////////////////////////////////////////////////////////////////
DiffMerge::LogNode::LogNode(const Object* o1, int level = 0) {
	reset();
	_title = o2t(o1);
	_level = level;
}


DiffMerge::LogNode::LogNode(std::string title, int level = 0) {
	reset();
	_title = title;
	_level = level;
}


void DiffMerge::LogNode::setParent(LogNode *n) {
	_parent = n;
	_level = _parent->_level;
	
	return;
}


std::string DiffMerge::LogNode::o2t(const Object *o) {
	std::stringstream title;
	
	title << o->className() << " ";
	for ( size_t i = 0; i < o->meta()->propertyCount(); ++i ) {
		const Core::MetaProperty* prop = o->meta()->property(i);
		
		// we do only compare index properties
		if ( ! prop->isIndex() )
			continue;
		
		if ( prop->isClass() )
			throw Core::TypeException(
					"Violation of contract: property " +
					prop->name() +
					" is of class type and marked as index");
		
		Core::MetaValue value;
		bool isSet_o = true;
		try { value = prop->read(o); } catch ( ... ) { isSet_o = false; }
		if (!isSet_o) continue;
		
		if ( prop->isEnum() || prop->type() == "int")
			title << "[" << boost::any_cast<int>(value) << "]";
		
		if ( prop->type() == "float" )
			title << "[" << boost::any_cast<double>(value) << "]";
		
		if ( prop->type() == "string" )
			title << "[" << boost::any_cast<std::string>(value) << "]";
		
		if ( prop->type() == "datetime" )
			title << "[" << boost::any_cast<Core::Time>(value).iso() << "]";
		
		if ( prop->type() == "boolean" )
			title << "[" << boost::any_cast<bool>(value) << "]";
		
		if ( prop->type() == "ComplexArray") {
			Core::BaseObject* bo1 = boost::any_cast<Core::BaseObject*>(value);
			ComplexArray *ca = ComplexArray::Cast(bo1);
			title << "[ComplexArray of " << ca->content().size() << " elements]";
		}
		
		if ( prop->type() == "RealArray") {
			Core::BaseObject* bo1 = boost::any_cast<Core::BaseObject*>(value);
			RealArray *ra = RealArray::Cast(bo1);
			title << "[ComplexArray of " << ra->content().size() << " elements]";
		}
		
		if ( prop->type() == "Blob") {
			Core::BaseObject* bo1 = boost::any_cast<Core::BaseObject*>(value);
			Blob *ba = Blob::Cast(bo1);
			title << "[Blob: " << ba << "]";
		}
	}
	return title.str();
}


void DiffMerge::LogNode::add(std::string s1, std::string n){
	if (_level < 1) return;
	s1.append(": ");
	s1.append(n);
	_messages.push_back(s1);
	return;
}


template <class T>
std::string DiffMerge::LogNode::compare(T a, T b, bool quotes) {
	std::stringstream ss;
	if ( quotes )
		ss << "\"" << a << "\"" << ((a == b)?" == ":" != ")
		   << "\"" << b << "\"";
	else
		ss << a << ((a == b)?" == ":" != ") << b;
	return ss.str();
}


void DiffMerge::LogNode::reset () {
	_parent = NULL;
	_title.clear();
	_level = 0;
}


DiffMerge::LogNodePtr DiffMerge::LogNode::add(DiffMerge::LogNodePtr child){
	child->setParent(this);
	_childs.push_back(child);
	return child;
}


void DiffMerge::LogNode::add(std::string title, const Object *o1) {
	add(title, o2t(o1));
	return;
}


void DiffMerge::LogNode::add(std::string title, bool status, std::string comment){
	if (_level < 2 && status) return;
	
	comment.insert(0, ((status)?"== ":"!= "));
	add(title, comment);
	return;
}


template <class T>
void DiffMerge::LogNode::add(std::string title, T a, T b){
	if ( _level < 2 && a == b ) return;
	add(title, compare<T>(a,b));
	return;
}


template <>
void DiffMerge::LogNode::add(std::string title, std::string a, std::string b){
	if ( _level < 2 && a == b ) return;
	add(title, compare(a,b,true));
	return;
}


void DiffMerge::LogNode::show(std::ostream &os, int padding, int indent,
                              bool ignoreFirstPad) {
	if ( !ignoreFirstPad )
		for ( int p = 0; p < padding; ++p ) os << " ";

	os << _title << std::endl;

	padding += indent;

	for ( size_t i = 0; i < _messages.size(); ++i ) {
		for ( int p = 0; p < padding; ++p ) os << " ";
		os << _messages[i] << std::endl;
	}

	for ( size_t i = 0; i < _childs.size(); ++i )
		_childs[i]->show(os, padding, indent);

	return;
}


///////////////////////////////////////////////////////////////////////////////
// DiffMerge
///////////////////////////////////////////////////////////////////////////////
DiffMerge::DiffMerge() {
	_currentNode = NULL;
	_logLevel = -1;
}


void DiffMerge::setLoggingLevel(int level) {
	_logLevel = level;
}


void  DiffMerge::showLog(std::ostream &os, int padding, int indent,
                         bool ignoreFirstPad) {
	if ( _currentNode != NULL )
		_currentNode->show(os, padding, indent, ignoreFirstPad);
}


bool DiffMerge::compareNonArrayProperty(const Core::BaseObject *o1,
                                        const Core::BaseObject *o2) {
	if ( o1 == o2 ) return true;

	// Different types are not comparable
	if ( (o1->meta() == NULL) || (o1->meta() != o2->meta()) ) return false;

	for ( size_t i = 0; i < o1->meta()->propertyCount(); ++i ) {
		const Core::MetaProperty* prop = o1->meta()->property(i);
		if ( !compareNonArrayProperty(prop, o1, o2) )
			return false;
	}

	return true;
}


/**
 * Compares the value of a non array property.
 * @param prop property to compare
 * @param o1 first object to compare
 * @param o2 second object to compare
 * @return true if both values are equal or empty, else false
 * @throw TypeException if the property to compare is of class or array type
 */
bool DiffMerge::compareNonArrayProperty(const Core::MetaProperty *prop,
                                        const Core::BaseObject *o1,
                                        const Core::BaseObject *o2) {
	if ( prop->isArray() )
		throw Core::TypeException("Expected non array property");

	// Property values may be empty and must be casted to its native
	// type since MetaValue does not implement the comparison operator
	bool isSet_o1 = true;
	bool isSet_o2 = true;
	Core::MetaValue v_o1;
	Core::MetaValue v_o2;
	try { v_o1 = prop->read(o1); } catch ( ... ) { isSet_o1 = false; }
	try { v_o2 = prop->read(o2); } catch ( ... ) { isSet_o2 = false; }

	if ( !isSet_o1 && !isSet_o2 ) {
		if (_currentNode) _currentNode->add(prop->name(), true, "both unset");
		return true;
	}
	if ( v_o1.empty() && v_o2.empty() ) {
		if (_currentNode) _currentNode->add(prop->name(), true, "both empty");
		return true;
	}
	if ( !isSet_o1 || v_o1.empty() ) {
		if (_currentNode) _currentNode->add(prop->name(), false, "missing or empty on Object 1");
		return false;
	}
	if ( !isSet_o2 || v_o2.empty() ) {
		if (_currentNode) _currentNode->add(prop->name(), false, "missing or empty on Object 2");
		return false;
	}
	if ( v_o1.type() != v_o2.type() ) {
		if (_currentNode) _currentNode->add(prop->name(), false, "types differs");
		return false;
	}

	if ( prop->isEnum() || prop->type() == "int") {
		if (_currentNode) _currentNode->add<int>(prop->name(), boost::any_cast<int>(v_o1), boost::any_cast<int>(v_o2));
		return boost::any_cast<int>(v_o1) == boost::any_cast<int>(v_o2);
	}
	if ( prop->type() == "float" ) {
		if (_currentNode) _currentNode->add<double>(prop->name(), boost::any_cast<double>(v_o1), boost::any_cast<double>(v_o2));
		return boost::any_cast<double>(v_o1) == boost::any_cast<double>(v_o2);
	}
	if ( prop->type() == "string" ){
		if (_currentNode) _currentNode->add<std::string>(prop->name(), boost::any_cast<std::string>(v_o1), boost::any_cast<std::string>(v_o2));
		return boost::any_cast<std::string>(v_o1) == boost::any_cast<std::string>(v_o2);
	}
	if ( prop->type() == "datetime" ){
		if (_currentNode) _currentNode->add<std::string>(prop->name(), boost::any_cast<Core::Time>(v_o1).iso(), boost::any_cast<Core::Time>(v_o2).iso());
		return boost::any_cast<Core::Time>(v_o1) == boost::any_cast<Core::Time>(v_o2);
	}
	if ( prop->type() == "boolean" ) {
		if (_currentNode) _currentNode->add<bool>(prop->name(), boost::any_cast<bool>(v_o1), boost::any_cast<bool>(v_o2));
		return boost::any_cast<bool>(v_o1) == boost::any_cast<bool>(v_o2);
	}

	if ( !prop->isClass() )
		throw Core::TypeException("Unexpected type: " + prop->type());

	Core::BaseObject* bo1 = boost::any_cast<Core::BaseObject*>(v_o1);
	Core::BaseObject* bo2 = boost::any_cast<Core::BaseObject*>(v_o2);

	return compareNonArrayProperty(bo1, bo2);
}


/**
 * Compares the className and all index properties of two objects.
 * @param o1 first object to compare
 * @param o2 second object to compare
 * @return true if the className and all index properties of both objects
 * are equal, else false
 */
bool DiffMerge::equalsIndex(Object *o1, Object *o2) {
	LogNodePtr nodeCopy = _currentNode;
	_currentNode = NULL;
	
	// compare className
	if ( o1->className() != o2->className() ){
		_currentNode = nodeCopy;
		return false;
	}
	
	// iterate over all properties
	for ( size_t i = 0; i < o1->meta()->propertyCount(); ++i ) {
		const Core::MetaProperty* metaProp = o1->meta()->property(i);
		
		// we do only compare index properties
		if ( ! metaProp->isIndex() )
			continue;
		
		if ( metaProp->isClass() )
			throw Core::TypeException(
				"Violation of contract: property " +
				metaProp->name() +
				" is of class type and marked as index");
			
		if ( ! compareNonArrayProperty(metaProp, o1, o2) ){
			_currentNode = nodeCopy;
			return false;
		}
	}

	_currentNode = nodeCopy;
	return true;
}


/**
 * Scans a object tree for a particular node. Objects are compared on base of
 * their indexes, @see equalsIndex
 * @param tree object tree to scan
 * @param node item to search for
 * @return pointer to the item within the object tree or NULL if the node was
 * not found
 */
Object* DiffMerge::find(Object* tree, Object* node) {
	if ( equalsIndex(tree, node) ) return tree;
	
	Object* result = NULL;

	// recursive scan of all class properties
	for ( size_t i = 0; i < tree->meta()->propertyCount(); ++i ) {
		const Core::MetaProperty* prop = tree->meta()->property(i);
		
		if ( ! prop->isClass() )
			continue;

		if ( prop->isArray() ) {
			for ( size_t a = 0; a < prop->arrayElementCount(tree); ++a ) {
				Core::BaseObject* bo = prop->arrayObject(tree, a);
				if ( ! bo->typeInfo().isTypeOf(Object::TypeInfo()) )
					break;
				result = find(static_cast<Object*>(bo), node);
				if ( result != NULL ) return result;
			}
		}
		else {
			Core::MetaValue value = prop->read(tree);
			if ( value.empty() ) continue;
			Object* child = boost::any_cast<Object*>(value);
			result = find(child, node);
			if ( result != NULL ) return result;
		}
	}	
	return NULL;
}


/**
 * Returns the public id, if any, of an object. The object is casted to
 * PublicObject and a copy of the publicID is returned.
 * @param o the object to retrieve it's publicID from
 * @return PublicObject.publicID or "" if the Cast to PublicObject failed
 */
std::string DiffMerge::getPublicID(Object *o) {
	PublicObject* po = PublicObject::Cast(o);
	return po ? po->publicID() : "";
}


/**
 * Recursively compares two objects and collects all differences.
 * @param o1 first object to compare
 * @param o2 second object to compare
 * @param o1ParentID parent ID of the first object
 * @param diffList list to collect differences in
 * @throw TypeException if any type restriction is violated
 */
void DiffMerge::diffRecursive(Object* o1, Object* o2, const std::string& o1ParentID,
                   std::vector<NotifierPtr>& diffList) {
	// Both objects empty, nothing to compare here
	if ( !o1 && !o2 ) return;

	// No element on the left -> ADD
	if ( !o1 ) {
		if (_currentNode) _currentNode->add("Adding", o2);
		AppendNotifier(diffList, OP_ADD, o2, o1ParentID);
		return;
	}

	// No element on the right -> REMOVE
	if ( !o2 ) {
		if ( _currentNode != NULL ) _currentNode->add("Removing", o1);
		AppendNotifier(diffList, OP_REMOVE, o1, o1ParentID);
		return;
	}

	LogNodePtr l_currentNode = _currentNode;
	if ( _currentNode )
		_currentNode = _currentNode->add(new LogNode(o1));

	std::string id_o1 = getPublicID(o1);
	NotifierPtr n;

	bool updateAdded = false;

	// Iterate over all properties
	for ( size_t i = 0; i < o1->meta()->propertyCount(); ++i ) {
		const Core::MetaProperty* prop = o1->meta()->property(i);
		
		// Non array property: Compare simple value(s)
		if ( !prop->isArray() ) {
			bool status = compareNonArrayProperty(prop, o1, o2);
			if ( updateAdded ) continue;

			if ( !status ) {
				std::string id_o2 = getPublicID(o2);

				if ( id_o1 != id_o2 ) {
					if (_currentNode) _currentNode->add("Updating PID", o1);
					// PublicID's are different, clone o1 and assign
					// the values of o2 but not its publicID
					ObjectPtr tmp = o1->clone();
					tmp->assign(o2);
					n = new Notifier(o1ParentID, OP_UPDATE, tmp.get());
				}
				else {
					if (_currentNode) _currentNode->add("Updating", o1);
					n = new Notifier(o1ParentID, OP_UPDATE, o2);
				}

				diffList.push_back(n);
				updateAdded = true;
			}

			continue;
		}

		// Class and array property:
		// The order of elements of a class array is arbitrary, hence
		// each element of one array must be searched among all elements
		// of the other array. A temporary vector with elements of o2 is
		// used to mark previously found matchings.
		std::vector<Object*> v_o2;
		for ( size_t i_o2 = 0; i_o2 < prop->arrayElementCount(o2); ++i_o2 ) {
			const Core::BaseObject* bo = prop->arrayObject(o2, i_o2);
			v_o2.push_back(Object::Cast(const_cast<Core::BaseObject*>(bo)));
		}

		// For each element of o1 array search counterpart in o2
		for ( size_t i_o1 = 0; i_o1 < prop->arrayElementCount(o1); ++i_o1 ) {
			const Core::BaseObject* bo = prop->arrayObject(o1, i_o1);
			Object* o1Child = Object::Cast(const_cast<Core::BaseObject*>(bo));
			std::vector<Object*>::iterator it_o2 = v_o2.begin();
			bool found = false;
			for ( ; it_o2 != v_o2.end(); ++it_o2) {
				if ( equalsIndex(o1Child, *it_o2) ) {
					found = true;
					break;
				}
			}
			if ( found ) {
				diffRecursive(o1Child, *it_o2, id_o1, diffList);
				v_o2.erase(it_o2);
			}
			else {
				if (_currentNode) _currentNode->add("Removing", o1Child);
				AppendNotifier(diffList, OP_REMOVE, o1Child, id_o1);
			}
		}

		// Add all elements of o2 array which have no counterpart in o1
		std::vector<Object*>::iterator it_o2 = v_o2.begin();
		for ( ; it_o2 != v_o2.end(); ++it_o2 ){
			if (_currentNode) _currentNode->add("Adding", *it_o2);
			AppendNotifier(diffList, OP_ADD, *it_o2, id_o1);
		}

		v_o2.clear();
	}
	_currentNode = l_currentNode;
}


/**
 * Recursively compares two objects and collects all differences.
 * The root element of one of the objects must be included in the other object
 * tree, @see find(o1, o2) 
 * @param o1 first object to compare
 * @param o2 second object to compare
 * @param diffList list to collect differences in
 * @return true if the diff could be performed, false if one object was null or
 * no common child object could be found
 * @throw TypeException if any type restriction is violated
 */
bool DiffMerge::diff(Object* o1, Object* o2, std::vector<NotifierPtr>& diffList) {
	if ( !o1 || !o2 )
		return false;

	// Find a common node
	Object* fO1 = find(o1, o2);
	Object* fO2 = !fO1 ? find(o2, o1) : NULL;
	
	// No common node, bye
	if ( !fO1 && !fO2 ) return false;
	
	o1 = fO1 ? fO1 : o1;
	o2 = fO2 ? fO2 : o2;
	std::string parentID = o1->parent() ? getPublicID(o1->parent()) : "";
	
	LogNodePtr newNode = NULL;

	if ( _logLevel >= 0 )
		newNode = new LogNode(o1, _logLevel);
	
	// Set the logger
	_currentNode = newNode;

	// Recursively diff both objects
	diffRecursive(o1, o2, parentID, diffList);
	
	// Restore the logger
	_currentNode = newNode;
	
	return true;
}


NotifierMessage *DiffMerge::diff2Message(Object *o1, Object *o2) {
	std::vector<NotifierPtr> diffList;
	if ( !diff(o1, o2, diffList) )
		return NULL;

	NotifierMessage *msg = new NotifierMessage;
	std::vector<NotifierPtr>::iterator it;

	for ( it = diffList.begin(); it != diffList.end(); ++it )
		msg->attach(*it);

	return msg;
}


/**
 * Visitor which collects publicIDs.
 */
class SC_SYSTEM_CORE_API PublicIDCollector : protected Visitor {
	public:
		void collect(Object *o, std::vector<std::string> *ids) {
			if ( o == NULL || ids == NULL ) return;
			_publicIDs = ids;
			o->accept(this);
			_publicIDs = NULL;
		}

	private:
		bool visit(PublicObject* po) {
			_publicIDs->push_back(po->publicID());
			return true;
		}

		void visit(Object* o) {}

	private:
		std::vector<std::string> *_publicIDs;
};


/**
 * Visitor which validates/repairs all references
 * (MetaProperty::isReference() == true) to publicIDs.
 */
class SC_SYSTEM_CORE_API ReferenceValidator : public Visitor {
	public:
		/**
		 * Validates all references.
		 * @param o Object to traverse
		 * @param idVector Vector of publicIDs to compare the references
		 *        with
		 * @return true if for each reference a matching entry in the
		 *         idVector was found, else false
		 */
		bool validate(Object *o, const std::vector<std::string> *idVector) {
			if ( o == NULL || idVector == NULL ) return false;
			_validates = idVector;
			_mappings = NULL;
			_valid = true;
			o->accept(this);
			return _valid;
		}
		
		/**
		 * Repairs references.
		 * @param o Object to traverse
		 * @param idMap Mapping of deprecated to current publicIDs
		 */
		size_t repair(Object *o, const std::map<std::string, std::string> *idMap) {
			if ( o == NULL || idMap == NULL ) return 0;
			_validates = NULL;
			_mappings = idMap;
			_mapCount = 0;
			o->accept(this);
			return _mapCount;
		}

	private:
		bool visit(PublicObject* po) {
			processReferences(po);
			return true;
		}

		virtual void visit(Object* o) {
			processReferences(o);
		}

		void processReferences(Object* o) {
			// iterate over all properties
			for ( size_t i = 0; i < o->meta()->propertyCount(); ++i ) {
				const Core::MetaProperty* prop = o->meta()->property(i);
				if ( !prop->isReference() ) continue;
				
				if ( prop->type() != "string" && prop->type() != "Blob" ) {
					SEISCOMP_WARNING("Skipping invalid reference type '%s' in property '%s'",
					                 prop->type().c_str(), prop->name().c_str());
					continue;
				}

				Core::MetaValue value;
				try { value = prop->read(o); } catch ( ... ) {
					SEISCOMP_WARNING("Could not read value of property '%s'",
					                 prop->name().c_str());
					continue;
				}

				if ( _validates )
					validateReference(prop, value);
				else
					repairReference(o, prop, value);
			}
		}
		
		void validateReference(const Core::MetaProperty *prop, const Core::MetaValue &value) {
			std::string ref;
			bool found = true;

			if ( prop->type() == "string" ) {
				ref = boost::any_cast<std::string>(value);
				found = find(_validates->begin(), _validates->end(), ref) != _validates->end();
			}
			else {
				// Type == Blob -> iterate over string list
				Core::BaseObject* bo = boost::any_cast<Core::BaseObject*>(value);
				Blob* refList = Blob::Cast(bo);
				std::vector<std::string> v;
				Core::fromString<std::string>(v, refList->content());
				std::vector<std::string>::const_iterator it = v.begin();
				for ( ; it != v.end(); ++it ) {
					ref = *it;
					found = find(_validates->begin(), _validates->end(), ref) != _validates->end();
					if ( !found ) break;
				}
			}
			
			_valid = _valid && found;
			if (! found) {
				SEISCOMP_WARNING("Broken reference in property '%s': %s",
				                 prop->name().c_str(), ref.c_str());
			}
		}

		
		void repairReference(Object *o, const Core::MetaProperty *prop, const Core::MetaValue &value) {
			std::map<std::string, std::string>::const_iterator it_map;
			if ( prop->type() == "string" ) {
				it_map = _mappings->find(boost::any_cast<std::string>(value));
				if ( it_map != _mappings->end() ) {
					prop->writeString(o, it_map->second);
					SEISCOMP_DEBUG("Replaced reference in property '%s': %s --> %s",
					               prop->name().c_str(), it_map->first.c_str(),
					               it_map->second.c_str());
					++_mapCount;
				}
			}
			else {
				// Type == Blob -> iterate over string list and
				// lookup each token
				Core::BaseObject* bo = boost::any_cast<Core::BaseObject*>(value);
				Blob* refList = Blob::Cast(bo);
				bool modified = false;
				std::vector<std::string> v;
				Core::fromString<std::string>(v, refList->content());
				
				for ( size_t i = 0; i < v.size(); ++i ) {
					it_map = _mappings->find(v[i]);
					if ( it_map != _mappings->end() ) {
						v[i] = it_map->second;
						++_mapCount;
						modified = true;
						SEISCOMP_DEBUG("Replaced reference in blob property '%s[%ld]': %s --> %s",
						               prop->name().c_str(), (long unsigned int) i,
						               it_map->first.c_str(), it_map->second.c_str());
					}
				}

				// Replace Blob value if modified
				if ( modified )
					refList->setContent(Core::toString<std::string>(v));
			}
		}

	private:
		const std::vector<std::string> *_validates;
		const std::map<std::string, std::string> *_mappings;
		bool _valid;
		size_t _mapCount;
};


/**
 * Visitor which recursively clones an object.
 */
class DeepCloner : protected Visitor {
	public:
		//! Constructor, the TraversalMode of TM_TOPDOWN is fix
		DeepCloner() : Visitor(TM_TOPDOWN) {};

		~DeepCloner() { reset(); }
		
		ObjectPtr clone(Object *o) {
			o->accept(this);
			ObjectPtr retn = _clone;
			reset();
			return retn;
		}

	private:
		void reset() { _clone = NULL; _parents.clear(); }

		bool visit(PublicObject *po) {
			PublicObjectPtr clone = PublicObject::Cast(po->clone());

			commit(clone.get());
			_parents.push_front(clone);

			return true;
		}

		void finished() {
			_parents.pop_front();
		}

		void visit(Object *o) {
			ObjectPtr clone = o->clone();
			commit(clone.get());
		}

		void commit(Object *o) {
			if ( !_parents.empty() ) {
				bool wasEnabled = PublicObject::IsRegistrationEnabled();
				PublicObject::SetRegistrationEnabled(false);
				o->attachTo(_parents.front().get());
				PublicObject::SetRegistrationEnabled(wasEnabled);
			}

			if ( _clone == NULL ) _clone = o;
		}

	private:
		std::deque<PublicObjectPtr> _parents;
		ObjectPtr _clone;
};


/**
 * Recursively merges object o2 into object o1.
 * @param o1 object to merge o2 into
 * @param o2 object to merge into o1
 * @param idMap map that keeps track of any publicID attribute changes applied
 * during the merge
 * @throw ValueException if one of the object pointer is NULL
 * @throw TypeException if both objects are not of same type
 */
void DiffMerge::mergeRecursive(Object* o1, Object* o2, std::map<std::string, std::string> &idMap) {
	if ( !o1 || !o2 )
		throw Core::ValueException("Invalid object pointer (NULL)");
	if ( o1->typeInfo() != o2->typeInfo() )
		throw Core::TypeException("Type mismatch");

	// Compare the publicID of PublicObjects and create a map entry if they
	// are distinguished
	if ( o1->typeInfo().isTypeOf(PublicObject::TypeInfo()) ) {
		const std::string &o1PID = PublicObject::Cast(o1)->publicID();
		const std::string &o2PID = PublicObject::Cast(o2)->publicID();
		if ( o1PID != o2PID ) {
			std::map<std::string, std::string>::iterator it = idMap.find(o1PID);
			if ( it != idMap.end() && it->second != o2PID )
				throw Core::ValueException("can't map publicID '" +
					it->first + "' to '" + o2PID +
					"' because it is already mapped to '" + it->second + "'");
			else
				idMap[o1PID] = o2PID;
		}
		
		// Empty the publicID so that it can be overridden by the
		// following assign operation
		PublicObject::Cast(o1)->setPublicID("");
	}
	
	// Copy all non class properties
	o1->assign(o2);
	
	// Iterate over all properties and merge children
	for ( size_t i = 0; i < o1->meta()->propertyCount(); ++i ) {
		const Core::MetaProperty* prop = o1->meta()->property(i);
		
		// Skip non class array property
		if ( !prop->isArray() || !prop->isClass() ) continue;

		// Class array property:
		// The order of elements of a class array is arbitrary, hence
		// each element of one array must be searched among all elements
		// of the other array. A temporary vector with elements of the
		// o2 array is used to mark previously found matchings.
		std::vector<Object*> v_o2;
		for ( size_t i_o2 = 0; i_o2 < prop->arrayElementCount(o2); ++i_o2 ) {
			const Core::BaseObject* bo = prop->arrayObject(o2, i_o2);
			v_o2.push_back(Object::Cast(const_cast<Core::BaseObject*>(bo)));
		}
		// For each element of the o1 array search counterpart in the o2
		// and and merge it
		for ( size_t i_o1 = 0; i_o1 < prop->arrayElementCount(o1); ++i_o1 ) {
			const Core::BaseObject* bo = prop->arrayObject(o1, i_o1);
			Object* o1Child = Object::Cast(const_cast<Core::BaseObject*>(bo));
			std::vector<Object*>::iterator it_o2 = v_o2.begin();
			for ( ; it_o2 != v_o2.end(); ++it_o2) {
				if ( equalsIndex(o1Child, *it_o2) ) {
					mergeRecursive(o1Child, *it_o2, idMap);
					v_o2.erase(it_o2);
					break;
				}
			}
		}

		// Add all elements of o2 array which have no counterpart in the
		// o1 are added to the o1 array.
		DeepCloner dc;
		std::vector<Object*>::iterator it_o2 = v_o2.begin();
		for ( ; it_o2 != v_o2.end(); ++it_o2 ) {
			prop->arrayAddObject(o1, dc.clone(*it_o2).get());
		}
		v_o2.clear();
	}
}


/**
 * Recursively merges the node object (and its children) into the tree object.
 * The node must be part of the tree, @ref find(o1, o2). Properties of the node
 * object override properties of the tree.
 * @param tree main object to merge the node into
 * @param node object to be merged into the tree
 * @param idMap map that keeps track of any publicID attribute changes applied
 * during the merge
 * @return true if the merge could be performed, false if the node was not found
 * in the tree
 * @throw TypeException if any type restriction is violated
 */
bool DiffMerge::merge(Object* tree, Object* node, std::map<std::string, std::string>& idMap) {
	if ( !tree || !node ) {
		SEISCOMP_WARNING("Invalid merge objects (NULL)");
		return false;
	}

	// Find a common node
	tree = find(tree, node);
	if ( !tree ) {
		SEISCOMP_WARNING("Invalid merge objects (o2 not child of o1)");
		return false;
	}

	// Recursively merge both objects
	mergeRecursive(tree, node, idMap);
	return true;
}


/**
 * Merges all all objects in the vector in order of their appearance into the
 * mergeResult object, @ref merge(Object*, Object*). The mergeResult object must
 * be not null and must serve as a parent for the objects being merged. In a
 * subsequent processing step changes to publicIDs are applied to references,
 * @ref mapReferences.
 * @param mergeResult object to merge the vector into
 * @param objects vector of objects to merge
 * @return true if all objects could be merged successfully, else false.
 */
bool DiffMerge::merge(Object* mergeResult, const std::vector<Object*>& objects) {
	if ( mergeResult == NULL ) return false;

	// Track publicID changes
	std::map<std::string, std::string> idMap;

	// Merge all objects of the vector
	bool retn = true;
	std::vector<Object*>::const_iterator it = objects.begin();
	for ( ; it != objects.end(); ++it ) {
		retn = merge(mergeResult, *it, idMap) && retn;
	}

	// Repair references
	size_t mappingsPerformed = mapReferences(mergeResult, idMap);
	SEISCOMP_DEBUG("%ld mappings performed", (unsigned long) mappingsPerformed);

	return retn;
}


/**
 * Validates the internal publicID references of the specified object. In a
 * first step all publicIDs are collected, then the object is traversed top-down
 * and each reference's value is searched in the publicID set.
 * @param o object to validate
 * @return true if all references point to an existing publicID, else false
 */
bool DiffMerge::validateReferences(Object* o) {
	// Collect publicIDs
	std::vector<std::string> publicIDs;
	PublicIDCollector pc;
	pc.collect(o, &publicIDs);

	// Validate
	ReferenceValidator rv;
	return rv.validate(o, &publicIDs);
}


/**
 * Maps publicID references of the specified object. While the object is
 * traversed top-down, a lookup for each reference in the publicID map is
 * performed. If a matching entry is found the reference's value is updated.
 * @param o object which references should be mapped
 * @param map publicIDMap of deprecated to current publicIDs
 * @return number of mappings performed
 */
size_t DiffMerge::mapReferences(Object* o, const std::map<std::string, std::string> &publicIDMap) {
	ReferenceValidator rv;
	return rv.repair(o, &publicIDMap);
}


bool DiffMerge::compareObjects(const Object *o1, const Object *o2) {
	bool cmp = true;

	_currentNode = new LogNode(o1, _logLevel);

	for ( size_t i = 0; i < o1->meta()->propertyCount(); ++i ) {
		const Core::MetaProperty* prop = o1->meta()->property(i);

		// Non array property: Compare simple value(s)
		if ( prop->isArray() ) continue;
		if ( !compareNonArrayProperty(prop, o1, o2) ) cmp = false;
	}

	return cmp;
}


} // of ns DataModel
} // of ns Seiscomp
