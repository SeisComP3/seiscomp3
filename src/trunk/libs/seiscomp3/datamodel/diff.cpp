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
 *                                                                         *
 *   Author: Stephan Herrnkind                                             *
 *   Email : herrnkind@gempa.de                                            *
 ***************************************************************************/


#define SEISCOMP_COMPONENT DataModelDiff


#include <seiscomp3/datamodel/blob.h>
#include <seiscomp3/datamodel/complexarray.h>
#include <seiscomp3/datamodel/creationinfo.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/realarray.h>
#include <seiscomp3/datamodel/diff.h>

#include <sstream>

using namespace std;


namespace Seiscomp {
namespace DataModel {


namespace {


typedef Diff2::LogNode LogNode;
typedef Diff2::LogNodePtr LogNodePtr;
typedef Diff2::Notifiers Notifiers;
typedef Diff2::PropertyIndex PropertyIndex;


// operation to string
string op2str(DataModel::Operation operation) {
	switch (operation) {
		case DataModel::OP_UPDATE: return "UPDATE";
		case DataModel::OP_ADD:    return "ADD";
		case DataModel::OP_REMOVE: return "REMOVE";
		default:                   return "UNDEFINED";
	}
}


ostream& operator<<(ostream& os, const Core::Time& t) {
	return os << t.iso();
}

template <class T>
bool diffProperty(const T &v1, const T &v2, LogNode *node, LogNode *child) {
	bool equals = v1 == v2;
	if ( node && node->level() >= LogNode::DIFFERENCES) {
		stringstream ss;
		if ( !equals ) {
			ss << "{ " << v1 << " != " << v2 << " }";
			node->addChild(child, ss.str());
		}
		else if (node->level() == LogNode::ALL) {
			ss << "{ " << v1 << " }";
			node->addChild(child, ss.str());
		}
	}
	return equals;
}

// forward declaration
bool compare(const Core::BaseObject *o1, const Core::BaseObject *o2,
             bool indexOnly = false, LogNode *logNode = NULL);

bool compareNonArrayProperty(const Core::MetaProperty* prop,
                             const Core::BaseObject *o1,
                             const Core::BaseObject *o2,
                             LogNode *logNode = NULL) {
	if ( prop->isArray() )
		throw Core::TypeException("expected non array property");

	// property values may be empty and must be casted to its native
	// type since MetaValue does not implement the comparison operator
	bool isSet_o1 = true;
	bool isSet_o2 = true;
	Core::MetaValue v_o1;
	Core::MetaValue v_o2;
	try { v_o1 = prop->read(o1); } catch ( ... ) { isSet_o1 = false; }
	try { v_o2 = prop->read(o2); } catch ( ... ) { isSet_o2 = false; }

	if ( !isSet_o1 && !isSet_o2 ) {
		if ( logNode && logNode->level() == LogNode::ALL )
			logNode->addChild(prop->name(), "unset");
		return true;
	}

	if ( !isSet_o1 ) {
		if ( logNode ) logNode->addChild(prop->name(), "missing locally");
		return false;
	}
	if ( !isSet_o2 ) {
		if ( logNode ) logNode->addChild(prop->name(), + "missing remotely");
		return false;
	}
	if ( v_o1.type() != v_o2.type() ) {
		if ( logNode ) logNode->addChild(prop->name(), + "type mismatch");
		return false;
	}

	LogNodePtr childLogNode;
	if ( logNode )
		childLogNode = new LogNode(prop->name(), logNode->level());
	bool equals = false;

	if ( prop->isClass() ) {
		o1 = boost::any_cast<Core::BaseObject*>(v_o1);
		o2 = boost::any_cast<Core::BaseObject*>(v_o2);
		equals = compare(o1, o2, false, childLogNode.get());
		if ( logNode && logNode->level() >= LogNode::DIFFERENCES) {
			if ( !equals )
				logNode->addChild(childLogNode.get(), "!=");
			else if (logNode->level() == LogNode::ALL)
				logNode->addChild(childLogNode.get());
		}
		return equals;
	}

	if ( prop->isEnum() || prop->type() == "int")
		return diffProperty(boost::any_cast<int>(v_o1),
		                    boost::any_cast<int>(v_o2),
		                    logNode, childLogNode.get());
	if ( prop->type() == "float" )
		return diffProperty(boost::any_cast<double>(v_o1),
		                    boost::any_cast<double>(v_o2),
		                    logNode, childLogNode.get());
	if ( prop->type() == "string" )
		return diffProperty(boost::any_cast<string>(v_o1),
		                    boost::any_cast<string>(v_o2),
		                    logNode, childLogNode.get());
	if ( prop->type() == "datetime" )
		return diffProperty(boost::any_cast<Core::Time>(v_o1),
		                    boost::any_cast<Core::Time>(v_o2),
		                    logNode, childLogNode.get());
	if ( prop->type() == "boolean" )
		return diffProperty(boost::any_cast<bool>(v_o1),
		                    boost::any_cast<bool>(v_o2),
		                    logNode, childLogNode.get());

	throw Core::TypeException("unexpected type: " + prop->type());
}

bool compare(const Core::BaseObject *o1, const Core::BaseObject *o2,
             bool indexOnly, LogNode *logNode) {
	bool result = true;

	// compare className
	if ( o1->className() != o2->className() ){
		if ( logNode ) logNode->setMessage("type mismatch");
		return false;
	}

	for ( size_t i = 0; i < o1->meta()->propertyCount(); ++i ) {
		const Core::MetaProperty* prop = o1->meta()->property(i);

		// check if only index values should be compared
		if ( indexOnly && !prop->isIndex() ) continue;

		// only non array properties are compared
		if ( prop->isArray() ) continue;

		// on difference check if logging requires further traversal
		if ( !compareNonArrayProperty(prop, o1, o2, logNode) && result) {
			result = false;
			if ( indexOnly || !logNode || logNode->level() == LogNode::OPERATIONS )
				break;
		}
	}

	return result;
}


} // anonymous


Diff2::Diff2() {}


Diff2::~Diff2() {}


void Diff2::LogNode::write(ostream &os, int padding, int indent,
                           bool ignoreFirstPad) const {
	if ( !ignoreFirstPad )
		for ( int p = 0; p < padding; ++p ) os << " ";

	os << _title;
	if ( !_message.empty() ) os << " [ " << _message << " ]";
	os << endl;

	padding += indent;

	for ( uint i = 0; i < _children.size(); ++i )
		_children[i]->write(os, padding, indent);

	return;
}



void Diff2::diff(DataModel::Object* o1, DataModel::Object* o2,
                 const string& o1ParentID,
                 vector<DataModel::NotifierPtr>& notifiers,
                 LogNode *parentLogNode) {
	// Both objects empty, nothing to compare here
	if ( !o1 && !o2 ) return;

	size_t ns = notifiers.size();

	// Filter object
	if ( o1 && blocked(o1, parentLogNode, true) ) return;
	if ( o2 && blocked(o2, parentLogNode, false) ) return;

	// No element on the left -> ADD
	if ( !o1 ) {
		AppendNotifier(notifiers, DataModel::OP_ADD, o2, o1ParentID);
		if ( parentLogNode && notifiers.size() > ns )
			createLogNodes(parentLogNode, o1ParentID,
			               notifiers.begin()+ns, notifiers.end());
		return;
	}

	// No element on the right -> REMOVE
	if ( !o2 ) {
		AppendNotifier(notifiers, DataModel::OP_REMOVE, o1, o1ParentID);
		if ( notifiers.size() > ns )
			createLogNodes(parentLogNode, o1ParentID,
			               notifiers.begin()+ns, notifiers.end());
		return;
	}

	// UPDATE?
	bool updateAdded = false;
	LogNodePtr logNode;
	if ( parentLogNode )
		logNode = new LogNode(o2t(o1), parentLogNode->level());

	DataModel::PublicObject *o1PO = DataModel::PublicObject::Cast(o1);

	// Iterate over all properties
	for ( size_t i = 0; i < o1->meta()->propertyCount(); ++i ) {
		const Core::MetaProperty* prop = o1->meta()->property(i);

		// Non array property
		if ( !prop->isArray() ) {
			// property has to be compared if no difference was detected so far
			// or log level requires output
			if ( updateAdded &&
			     (!logNode || logNode->level() == LogNode::OPERATIONS) )
				continue;
			bool status = compareNonArrayProperty(prop, o1, o2, logNode.get());

			if ( !updateAdded && !status ) {
				notifiers.push_back(new DataModel::Notifier(o1ParentID, DataModel::OP_UPDATE, o2));
				updateAdded = true;
				if ( logNode ) logNode->setMessage(op2str(DataModel::OP_UPDATE));
			}

			continue;
		}

		// only PublicObjects contain array properties
		if ( !o1PO ) continue;

		// Array property:
		// The order of elements of a class array is arbitrary, hence
		// each element of one array must be searched among all elements
		// of the other array. PublicObjects are identified based on their
		// publicID, other Objects are compared by their index fields.
		map<string, DataModel::PublicObject*> o2POChilds;
		vector<DataModel::Object*> o2Childs;
		for ( size_t i_o2 = 0; i_o2 < prop->arrayElementCount(o2); ++i_o2 ) {
			Core::BaseObject* bo = const_cast<Core::BaseObject*>(prop->arrayObject(o2, i_o2));

			DataModel::PublicObject* po = DataModel::PublicObject::Cast(bo);
			if ( po )
				o2POChilds[po->publicID()] = po;
			else
				o2Childs.push_back(DataModel::Object::Cast(bo));
		}

		// For each element of o1 array search counterpart in o2
		for ( size_t i_o1 = 0; i_o1 < prop->arrayElementCount(o1); ++i_o1 ) {
			Core::BaseObject* bo = const_cast<Core::BaseObject*>(prop->arrayObject(o1, i_o1));
			DataModel::Object *o1Child = DataModel::Object::Cast(bo);
			DataModel::Object *o2Child = NULL;
			DataModel::PublicObject *po = DataModel::PublicObject::Cast(bo);
			if ( po ) {
				map<string, DataModel::PublicObject*>::iterator it = o2POChilds.find(po->publicID());
				if ( it != o2POChilds.end() ) {
					o2Child = it->second;
					o2POChilds.erase(it);
				}
			}
			else {
				for ( vector<DataModel::Object*>::iterator it = o2Childs.begin();
				      it != o2Childs.end(); ++it ) {
					if ( compare(o1Child, *it, true) ) {
						o2Child = *it;
						o2Childs.erase(it);
						break;
					}
				}
			}

			diff(o1Child, o2Child, o1PO->publicID(), notifiers, logNode.get());
		}

		// Add all elements of o2 array which have no counterpart in o1
		for ( map<string, DataModel::PublicObject*>::iterator it = o2POChilds.begin();
		      it != o2POChilds.end(); ++it )
			diff(NULL, it->second, o1PO->publicID(), notifiers, logNode.get());
		for ( vector<DataModel::Object*>::iterator it = o2Childs.begin();
		      it != o2Childs.end(); ++it )
			diff(NULL, *it, o1PO->publicID(), notifiers, logNode.get());
	}

	if ( parentLogNode && logNode &&
	     (logNode->level() == LogNode::ALL || logNode->childCount()) )
		parentLogNode->addChild(logNode.get());
}


NotifierMessage *Diff2::diff2Message(Seiscomp::DataModel::Object *o1,
                                     Seiscomp::DataModel::Object *o2,
                                     const std::string &o1ParentID, LogNode *logNode) {
	std::vector<NotifierPtr> diffList;
	LogNode log;
	log.setLevel(LogNode::DIFFERENCES);
	diff(o1, o2, o1ParentID, diffList, &log);

	if ( diffList.empty() ) return NULL;

	NotifierMessage *msg = new NotifierMessage;
	std::vector<NotifierPtr>::iterator it;

	for ( it = diffList.begin(); it != diffList.end(); ++it )
		msg->attach(*it);

	return msg;
}


std::string Diff2::o2t(const Core::BaseObject *o) const {
	stringstream title;
	title << o->className() << " ";

	// PublicObject -> use public ID as title
	const DataModel::PublicObject *po = DataModel::PublicObject::ConstCast(o);
	if ( po ) {
		title << po->publicID();
		return title.str();
	}

	// not a DataModel::PublicObject -> concat all index values
	for ( size_t i = 0; i < o->meta()->propertyCount(); ++i ) {
		const Core::MetaProperty* prop = o->meta()->property(i);

		if ( !prop->isIndex() )
			continue;

		if ( prop->isClass() )
			throw Core::TypeException(
					"violation of contract: property " +
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
			title << "[" << boost::any_cast<string>(value) << "]";

		if ( prop->type() == "datetime" )
			title << "[" << boost::any_cast<Core::Time>(value).iso() << "]";

		if ( prop->type() == "boolean" )
			title << "[" << boost::any_cast<bool>(value) << "]";

		if ( prop->type() == "ComplexArray") {
			Core::BaseObject* bo1 = boost::any_cast<Core::BaseObject*>(value);
			DataModel::ComplexArray *ca = DataModel::ComplexArray::Cast(bo1);
			title << "[ComplexArray of " << ca->content().size() << " elements]";
		}

		if ( prop->type() == "RealArray") {
			Core::BaseObject* bo1 = boost::any_cast<Core::BaseObject*>(value);
			DataModel::RealArray *ra = DataModel::RealArray::Cast(bo1);
			title << "[ComplexArray of " << ra->content().size() << " elements]";
		}

		if ( prop->type() == "Blob") {
			Core::BaseObject* bo1 = boost::any_cast<Core::BaseObject*>(value);
			DataModel::Blob *ba = DataModel::Blob::Cast(bo1);
			title << "[Blob: " << ba << "]";
		}
	}

	return title.str();
}


void Diff2::createLogNodes(LogNode *rootLogNode, const string &rootID,
                           Notifiers::const_iterator begin,
                           Notifiers::const_iterator end) {
	typedef map<string, LogNodePtr> PublicIDNodes;
	PublicIDNodes nodeMap;

	for ( ; begin != end; ++begin ) {
		LogNode *parentLogNode;
		DataModel::Notifier *n = begin->get();
		DataModel::Object *o = n->object();
		string op = op2str(n->operation());

		// get publicID of parent object
		string parentID;
		if ( o->parent() ) parentID = o->parent()->publicID();

		// search/create LogNode for parent object
		if ( parentID == rootID )
			// the root of the notifier tree was found
			parentLogNode = rootLogNode;
		else {
			PublicIDNodes::iterator it = nodeMap.find(parentID);
			if ( it == nodeMap.end() ) {
				parentLogNode = new LogNode();
				nodeMap[parentID] = parentLogNode;
			}
			else
				parentLogNode = it->second.get();
		}

		// check if node already exists
		DataModel::PublicObject *po = DataModel::PublicObject::Cast((*begin)->object());
		if ( po ) {
			const string &pID = po->publicID();
			PublicIDNodes::iterator it = nodeMap.find(pID);
			if ( it == nodeMap.end() )
				nodeMap[pID] = parentLogNode->addChild(o2t(o), op);
			else {
				it->second->setTitle(o2t(o));
				it->second->setMessage(op);
			}
		}
		else
			parentLogNode->addChild(o2t(o), op);
	}
}


bool Diff2::blocked(const Core::BaseObject *o, LogNode *node, bool local) {
	return false;
}


} // ns DataModel
} // ns Seiscomp
