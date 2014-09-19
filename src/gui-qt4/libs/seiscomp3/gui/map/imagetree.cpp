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


#define SEISCOMP_COMPONENT Gui::ImageTree

#include <seiscomp3/gui/map/imagetree.h>
#include <seiscomp3/core/interfacefactory.ipp>
#include <seiscomp3/logging/log.h>

#include <cstdio>


IMPLEMENT_INTERFACE_FACTORY(Seiscomp::Gui::Map::TileStore, SC_GUI_API);


namespace Seiscomp {
namespace Gui {
namespace Map {


namespace {


QString generateID(int level, int column, int row) {
	QString id = "";

	for ( int i = 0; i < level; ++i ) {
		int ofs = 1 << (level-i-1);
		int x,y,c;

		if ( column >= ofs ) {
			x = 1;
			column -= ofs;
		}
		else
			x = 0;

		if ( row >= ofs ) {
			y = 1;
			row -= ofs;
		}
		else
			y = 0;

		if ( x )
			c = y?3:0;
		else
			c = y?2:1;

		id += char('0' + c);
	}

	return id;
}


QString generatePath(int level, int col, int row,
                         const std::string &pattern) {
	QString path;

	for ( size_t i = 0; i < pattern.size(); ++i ) {
		if ( pattern[i] != '%' )
			path += pattern[i];
		else {
			++i;
			int len = 0;
			while ( i < pattern.size() ) {
				if ( pattern[i] >= '0' && pattern[i] <= '9' ) {
					len *= 10;
					len += int(pattern[i] - '0');
					++i;
					continue;
				}
				else if ( pattern[i] == '%' )
					path += pattern[i];
				else if ( pattern[i] == 's' )
					path += generateID(level, col, row);
				else if ( pattern[i] == 'l' )
					path += QString::number(level);
				else if ( pattern[i] == 'c' )
					path += QString::number(col);
				else if ( pattern[i] == 'r' )
					path += QString::number(row);
				break;
			}
		}
	}

	return path;
}




char buffer[1024+1];
const char* generateID(const std::string& pattern, const std::string& id) {
	snprintf(buffer, 1024, pattern.c_str(), id.c_str());
	return buffer;
}



class TileDirectory : public TileStore {
	public:
		TileDirectory() {}

		bool open(MapsDesc &desc) {
			_filePattern = desc.location.toStdString();

			if ( validate(0, 0, 0) ) {
				QString id = generatePath(0, 0, 0, _filePattern);

				_level = 0;
				_row = 0;
				_column = 0;

				QImage img(id);
				_tilesize = img.size();
				_mapsize = _tilesize * (1 << depth());

				return true;
			}

			return false;
		}

		bool load(QImage &img, Alg::MapTreeNode *node) {
			return img.load(getID(node));
		}

		QString getID(const MapTreeNode *node) const {
			return generatePath(node->level(), node->column(), node->row(), _filePattern);
		}

		// Tiles are never loaded asynchronously
		bool hasPendingRequests() const {
			return false;
		}

		void refresh() {}


	protected:
		bool validate(int level, int column, int row) const {
			QString id = generatePath(level, column, row, _filePattern);

			FILE* fp = fopen(id.toAscii(), "rb");
			if ( fp == NULL ) return false;
			fclose(fp);

			return true;
		}

	private:
		std::string _filePattern;
};


}


TileStore::TileStore()
: _tree(NULL) {

}


void TileStore::setImageTree(ImageTree *tree) {
	_tree = tree;
}


void TileStore::finishedLoading(QImage &img, Alg::MapTreeNode *node) {
	if ( _tree != NULL )
		_tree->finishedLoading(img, node);
}


void TileStore::invalidate(Alg::MapTreeNode *node) {
	if ( _tree != NULL )
		_tree->invalidate(node);
}


ImageTree::ImageTree(const MapsDesc &meta) {
	if ( meta.type.isEmpty() )
		_store = new TileDirectory;
	else
		_store = TileStoreFactory::Create(meta.type.toAscii());

	if ( _store ) {
		_store->setImageTree(this);

		MapsDesc desc(meta);

		if ( !_store->open(desc) ) {
			SEISCOMP_ERROR("Failed to open tile store at %s",
			               (const char*)desc.location.toAscii());
			_store = NULL;
		}
		else {
			_isMercatorProjected = desc.isMercatorProjected;
			_cacheSize = desc.cacheSize;
		}
	}
	else {
		SEISCOMP_ERROR("Could not create tile store: %s",
		               (const char*)meta.type.toAscii());
	}
}


ImageTree::~ImageTree() {
	_cache = NULL;
	_store = NULL;
}


TextureCache *ImageTree::getCache() {
	if ( !_cache && _store ) {
		_cache = new TextureCache(_store.get(), _isMercatorProjected);
		if ( _cacheSize > 0 )
			_cache->setCacheLimit(_cacheSize);
	}

	return _cache.get();
}


void ImageTree::refresh() {
	if ( _store ) _store->refresh();
	if ( _cache ) _cache->clear();
}


void ImageTree::finishedLoading(QImage &img, Alg::MapTreeNode *node) {
	if ( !_cache ) return;
	_cache->setTexture(img, node);

	tilesUpdated();

	if ( !hasPendingRequests() )
		tilesComplete();
}


void ImageTree::invalidate(Alg::MapTreeNode *node) {
	if ( _cache ) _cache->invalidateTexture(node);
}


}
}
}
