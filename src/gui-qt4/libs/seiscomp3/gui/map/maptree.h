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



#ifndef __SEISCOMP_GUI_ALG_MAPTREE_H__
#define __SEISCOMP_GUI_ALG_MAPTREE_H__

#include <string>
#include <QString>
#include <QImage>
#ifndef Q_MOC_RUN
#include <seiscomp3/core/baseobject.h>
#endif
#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {
namespace Gui {
namespace Alg {


class MapTree;


DEFINE_SMARTPOINTER(MapTreeNode);
class SC_GUI_API MapTreeNode : public Seiscomp::Core::BaseObject {
	public:
		MapTreeNode();
		~MapTreeNode();

		void setChildren(int index, MapTreeNode*);
		void resetChildren(int index);

		//! Loads the next child. This function does not load the complete
		//! subtree.
		void loadChildren(MapTree*, int index);

		MapTreeNode* children(int index) const { return _child[index].node.get(); }
		bool initialized(int index) const { return _child[index].initialized; }

		int childrenCount() const;
		int depth() const;

		int level() const { return _level; }
		int row() const { return _row; }
		int column() const { return _column; }

	protected:
		static MapTreeNode* Create(MapTree* root, MapTreeNode *parent,
		                           int level, int column, int row,
		                           int subLevels);

	protected:
		struct ChildItem {
			ChildItem() : initialized(false) {}
			MapTreeNodePtr node;
			bool           initialized;
		};

		ChildItem    _child[4];
		MapTreeNode *_parent;

		int _level;
		int _row;
		int _column;
};


DEFINE_SMARTPOINTER(MapTree);
class SC_GUI_API MapTree : public MapTreeNode {
	protected:
		MapTree();

	public:
		virtual ~MapTree() {}

	public:
		const QSize &tileSize() const { return _tilesize; }
		const QSize &mapSize() const { return _mapsize; }


	protected:
		//! Creates and validates an fileID
		virtual bool validate(int level, int column, int row) const = 0;

	protected:
		QSize       _tilesize;
		QSize       _mapsize;

	friend class MapTreeNode;
};


}
}
}


#endif
