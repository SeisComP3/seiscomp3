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



#include "maptree.h"


#ifdef WIN32
#undef min
#undef max
#endif


namespace Seiscomp {
namespace Gui {
namespace Alg {


MapTreeNode::MapTreeNode()
 : _parent(NULL) {

	_level = -1;
	_row = -1;
	_column = -1;
}


MapTreeNode::~MapTreeNode() {
	for ( int i = 0; i < 4; ++i )
		setChildren(i, NULL);
}


void MapTreeNode::setChildren(int index, MapTreeNode* node) {
	_child[index].node = node;
	_child[index].initialized = true;
}


void MapTreeNode::resetChildren(int index) {
	_child[index].node = NULL;
	_child[index].initialized = false;
}


void MapTreeNode::loadChildren(MapTree *root, int index) {
	int level = _level + 1;
	int col = _column << 1;
	int row = _row << 1;

	switch ( index ) {
		case 0:
			col += 1;
			break;
		case 1:
			break;
		case 2:
			row += 1;
			break;
		case 3:
			col += 1;
			row += 1;
			break;
		default:
			return;
	}

	setChildren(index, Create(root, this, level, col, row, 0));
}


MapTree::MapTree()
 : MapTreeNode() {
}


int MapTreeNode::childrenCount() const {
	int count = 0;

	for ( int i = 0; i < 4; ++i )
		if ( _child[i].node != NULL )
			count += _child[i].node->childrenCount() + 1;

	return count;
}


int MapTreeNode::depth() const {
	/*
	int depth = 0;

	for ( int i = 0; i < 4; ++i )
		if ( _child[i].node != NULL )
			depth = std::max(depth, _child[i].node->depth()+1);

	return depth;
	*/
	return 31;
}


MapTreeNode* MapTreeNode::Create(MapTree* root, MapTreeNode *parent,
                                 int level, int column, int row,
                                 int subLevels) {
	if ( root->validate(level, column, row) ) {
		MapTreeNode* node = new MapTreeNode;
		node->_parent = parent;
		node->_level = level;
		node->_row = row;
		node->_column = column;

		if ( subLevels != 0 ) {
			row <<= 1;
			column <<= 1;
			--subLevels;

			node->setChildren(0, Create(root, node, level+1, column+1, row, subLevels));
			node->setChildren(1, Create(root, node, level+1, column  , row, subLevels));
			node->setChildren(2, Create(root, node, level+1, column  , row+1, subLevels));
			node->setChildren(3, Create(root, node, level+1, column+1, row+1, subLevels));
		}

		return node;
	}

	return NULL;
}


}
}
}
