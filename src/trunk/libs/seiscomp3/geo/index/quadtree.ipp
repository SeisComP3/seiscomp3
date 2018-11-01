// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline QuadTree::Node::Node() : isLeaf(true) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
inline const GeoBoundingBox &QuadTree::bbox() const {
	return _root.bbox;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename VISITOR>
inline void QuadTree::accept(const VISITOR &visitor, bool topDown) {
	if ( topDown )
		_root.acceptTopDown(visitor);
	else
		_root.acceptButtomUp(visitor);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename VISITOR>
inline bool QuadTree::Node::acceptTopDown(const VISITOR &visitor) {
	if ( !visitor.accept(bbox) ) return true;

	for ( size_t i = 0; i < features.size(); ++i ) {
		if ( !visitor.visit(features[i]) )
			return false;
	}

	for ( size_t i = 0; i < 4; ++i ) {
		if ( children[i] ) {
			if ( !children[i]->acceptTopDown(visitor) )
				return false;
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename VISITOR>
inline bool QuadTree::Node::acceptButtomUp(const VISITOR &visitor) {
	if ( !visitor.accept(bbox) ) return true;

	for ( size_t i = 0; i < 4; ++i ) {
		if ( children[i] ) {
			if ( !children[i]->acceptButtomUp(visitor) )
				return false;
		}
	}

	for ( size_t i = features.size(); i > 0; --i ) {
		if ( !visitor.visit(features[i-1]) )
			return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
