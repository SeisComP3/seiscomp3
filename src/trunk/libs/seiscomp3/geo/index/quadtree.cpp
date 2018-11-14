/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#include <seiscomp3/geo/index/quadtree.h>
#include <iostream>


namespace Seiscomp {
namespace Geo {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

inline bool validBox(const GeoBoundingBox &container, const GeoBoundingBox &child) {
	/*
	bool ok = container.contains(child);
	std::cerr << container << "  <  " << child << " = " << ok << std::endl;
	return ok;
	*/
	return container.contains(child);
}

}

QuadTree::NodeIndex QuadTree::Node::findAndCreateNode(const GeoFeature *f) {
	const GeoBoundingBox &fbbox = f->bbox();
	GeoCoordinate center = bbox.center();
	GeoBoundingBox childBox;
	NodeIndex idx = InvalidIndex;

	if ( idx == InvalidIndex ) {
		// Upper right
		if ( children[UpperRight] ) {
			if ( children[UpperRight]->bbox.contains(fbbox) )
				return UpperRight;
		}
		else {
			// Upper right
			childBox = GeoBoundingBox(center.lat, center.lon, bbox.north, bbox.east);
			if ( validBox(childBox, fbbox) )
				idx = UpperRight;
		}
	}

	if ( idx == InvalidIndex ) {
		if ( children[UpperLeft] ) {
			if ( children[UpperLeft]->bbox.contains(fbbox) )
				return UpperLeft;
		}
		else {
			// Upper left
			childBox = GeoBoundingBox(center.lat, bbox.west, bbox.north, center.lon);
			if ( validBox(childBox, fbbox) )
				idx = UpperLeft;
		}
	}

	if ( idx == InvalidIndex ) {
		if ( children[LowerLeft] ) {
			if ( children[LowerLeft]->bbox.contains(fbbox) )
				return LowerLeft;
		}
		else {
			// Lower left
			childBox = GeoBoundingBox(bbox.south, bbox.west, center.lat, center.lon);
			if ( validBox(childBox, fbbox) )
				idx = LowerLeft;
		}
	}

	if ( idx == InvalidIndex ) {
		if ( children[LowerRight] ) {
			if ( children[LowerRight]->bbox.contains(fbbox) )
				return LowerRight;
		}
		else {
			// Lower right
			childBox = GeoBoundingBox(bbox.south, center.lon, center.lat, bbox.east);
			if ( validBox(childBox, fbbox) )
				idx = LowerRight;
		}
	}

	if ( idx != InvalidIndex ) {
		children[idx] = new Node();
		children[idx]->bbox = childBox;
	}

	return idx;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QuadTree::Node::addItem(const GeoFeature *f) {
	if ( isLeaf ) {
		if ( features.size() < 4 ) {
			features.push_back(f);
			return;
		}
		else {
			// Split existing features
			for ( size_t i = 0; i < features.size(); ) {
				NodeIndex idx = findAndCreateNode(features[i]);
				if ( idx != InvalidIndex ) {
					children[idx]->addItem(features[i]);
					features.erase(features.begin()+i);
					isLeaf = false;
				}
				else {
					// Stay in this node
					++i;
				}
			}
		}
	}

	NodeIndex idx = findAndCreateNode(f);
	if ( idx != InvalidIndex )
		children[idx]->addItem(f);
	else
		features.push_back(f);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {

struct indent {
	indent(int n) : nch(n) {}
	int nch;
};

std::ostream &operator<<(std::ostream &os, const indent &ind) {
	int i = ind.nch;
	while ( i-- ) os << " ";
	return os;
}

}


void QuadTree::Node::dump(std::ostream &os, int indent_) const {
	os << indent(indent_) << "[" << bbox << "]" << std::endl;
	for ( size_t i = 0; i < features.size(); ++i )
		os << indent(indent_) << "  + " << features[i]->name() << "   " << features[i]->bbox() << std::endl;

	for ( size_t i = 0; i < 4; ++i )
		if ( children[i] )
			children[i]->dump(os, indent_ + 2);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QuadTree::Node::visit(const GeoCoordinate &gc, const VisitFunc &func) const {
	if ( !bbox.contains(gc) ) return;

	for ( size_t i = 0; i < features.size(); ++i ) {
		if ( features[i]->bbox().contains(gc) && features[i]->contains(gc) ) {
			if ( !func(features[i]) )
				return;
		}
	}

	for ( size_t i = 0; i < 4; ++i ) {
		if ( children[i] )
			children[i]->visit(gc,func);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QuadTree::Node::visit(const GeoBoundingBox &bb, const VisitFunc &func,
                           bool clipOnlyNodes) const {
	if ( !bb.intersects(bbox) ) return;

	if ( clipOnlyNodes ) {
		for ( size_t i = 0; i < features.size(); ++i ) {
			if ( !func(features[i]) )
				return;
		}
	}
	else {
		for ( size_t i = 0; i < features.size(); ++i ) {
			if ( bb.intersects(features[i]->bbox()) ) {
				if ( !func(features[i]) )
					return;
			}
		}
	}

	for ( size_t i = 0; i < 4; ++i ) {
		if ( children[i] )
			children[i]->visit(bb,func, clipOnlyNodes);
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const GeoFeature *QuadTree::Node::findFirst(const GeoCoordinate &gc) {
	const GeoFeature *f;

	if ( !bbox.contains(gc) ) return NULL;

	for ( size_t i = 0; i < features.size(); ++i ) {
		f = features[i];
		if ( !f->closedPolygon() ) continue;
		if ( f->bbox().contains(gc) && f->contains(gc) )
			return f;
	}

	for ( size_t i = 0; i < 4; ++i ) {
		if ( children[i] ) {
			f = children[i]->findFirst(gc);
			if ( f != NULL )
				return f;
		}
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const GeoFeature *QuadTree::Node::findLast(const GeoCoordinate &gc) {
	const GeoFeature *f;

	if ( !bbox.contains(gc) ) return NULL;

	for ( size_t i = 4; i > 0; --i ) {
		if ( children[i-1] ) {
			f = children[i-1]->findLast(gc);
			if ( f != NULL )
				return f;
		}
	}

	for ( size_t i = features.size(); i > 0; --i ) {
		f = features[i-1];
		if ( !f->closedPolygon() ) continue;
		if ( f->bbox().contains(gc) && f->contains(gc) )
			return f;
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
QuadTree::QuadTree() {
	_root.bbox = GeoBoundingBox(-90, -180, 90, 180);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QuadTree::addItem(const GeoFeature *f) {
	_root.addItem(f);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QuadTree::add(const GeoFeatureSet &featureSet) {
	for ( size_t i = 0; i < featureSet.features().size(); ++i )
		addItem(featureSet.features()[i]);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QuadTree::query(const GeoCoordinate &gc, const VisitFunc &func) const {
	_root.visit(gc, func);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void QuadTree::query(const GeoBoundingBox &bb, const VisitFunc &func,
                     bool clipOnlyNodes) const {
	_root.visit(bb, func, clipOnlyNodes);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const GeoFeature *QuadTree::findFirst(const GeoCoordinate &gc) {
	return _root.findFirst(gc);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const GeoFeature *QuadTree::findLast(const GeoCoordinate &gc) {
	return _root.findLast(gc);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream &QuadTree::dump(std::ostream &os) const {
	_root.dump(os, 0);
	return os;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::ostream &operator<<(std::ostream &os, const Seiscomp::Geo::QuadTree &tree) {
	return tree.dump(os);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
