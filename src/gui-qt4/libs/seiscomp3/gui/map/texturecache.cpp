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



#include <QHash>
#include <QMutex>
#include <iostream>

#include <seiscomp3/gui/map/texturecache.h>
#include <seiscomp3/gui/map/imagetree.h>



namespace Seiscomp {
namespace Gui {
namespace Map {


QMap<QString, TextureCache::CacheEntry> TextureCache::_images;
QMutex imageCacheMutex(QMutex::Recursive);


TextureCache *getTexelCache = NULL;

Texture::Texture() {
	data = NULL;
	w = 0;
	h = 0;
}


int Texture::numBytes() const {
	return image.numBytes();
}



bool Texture::load(TextureCache *cache, Alg::MapTreeNode *node) {
	data = NULL;
	if ( node == NULL || !cache->load(image, node) || image.isNull() ) {
		image = QImage(1,1, QImage::Format_RGB32);
		QRgb *bits = (QRgb*)image.bits();
		*bits = qRgb(224,224,224);
		isDummy = true;
	}

	if ( node ) {
		id.level = node->level();
		id.row = node->row();
		id.column = node->column();
	}
	else {
		id.level = 0;
		id.row = 0;
		id.column = 0;
	}

	w = image.width();
	h = image.height();
	data = (const QRgb*)image.bits();

	return true;
}


void Texture::setImage(QImage &img) {
	image = img;
	w = image.width();
	h = image.height();
	data = (const QRgb*)image.bits();
	isDummy = false;
}


TextureCache::TextureCache(TileStore *tree, bool mercatorProjected) {
	_mapTree = tree;
	_isMercatorProjected = mercatorProjected;
	_storedBytes = 0;
	_textureCacheLimit = 32*1024*1024; // 32mb cache limit
	_lastTile[0] = _lastTile[1] = NULL;
	_currentIndex = 0;
	_currentTick = 0;
}


TextureCache::~TextureCache() {
	// remove storage textures
	for ( Storage::iterator it = _storage.begin(); it != _storage.end(); ++it ) {
		Alg::MapTreeNode *node = it.key();
		if ( node != NULL )
			remove(_mapTree->getID(node));
		else
			std::cerr << "Warning: cached texture node is NULL" << std::endl;
	}
}


void TextureCache::beginPaint() {
}


void TextureCache::setCacheLimit(int limit) {
	_textureCacheLimit = limit;
}


int TextureCache::maxLevel() const {
	if ( _mapTree ) return _mapTree->depth();
	return 0;
}


int TextureCache::tileWidth() const {
	if ( _mapTree ) return _mapTree->tileSize().width();
	return 0;
}


int TextureCache::tileHeight() const {
	if ( _mapTree ) return _mapTree->tileSize().height();
	return 0;
}


bool TextureCache::load(QImage &img, Alg::MapTreeNode *node) {
	QMutexLocker lock(&imageCacheMutex);

	ImageCache::iterator it;
	QString id = _mapTree->getID(node);
	it = _images.find(id);
	if ( it == _images.end() ) {
		if ( !_mapTree->load(img, node) )
			return false;

		if ( img.format() != QImage::Format_RGB32 &&
		     img.format() != QImage::Format_ARGB32 )
			img = img.convertToFormat(QImage::Format_ARGB32);

		_images[id] = CacheEntry(img, 1);
	}
	else {
		img = it.value().first;
		++it.value().second;
	}

	/*
	for ( int i = 0; i < 5; ++i ) {
		for ( int j = 0; j < w; ++j ) {
			data[i*w + j] = qRgb(255,0,0);
			data[(h-1-i)*w + j] = qRgb(255,0,0);
		}
		for ( int j = 0; j < h; ++j ) {
			data[j*w + i] = qRgb(255,0,0);
			data[j*w + w-1-i] = qRgb(255,0,0);
		}
	}
	*/

	return true;
}


void TextureCache::checkResources(Texture *tex) {
	//if ( _textureCountLimit <= 0 || _storage.size () < _textureCountLimit ) return;
	if ( _storedBytes <= _textureCacheLimit ) return;

	qint64 min = _currentTick;
	Storage::iterator it, min_it = _storage.end();
	for ( it = _storage.begin(); it != _storage.end(); ++it ) {
		if ( (it.value()->lastUsed < min || min_it == _storage.end()) && it.value() != tex ) {
			min = it.value()->lastUsed;
			min_it = it;
		}
	}

	// Remove texture completely
	if ( min_it != _storage.end() ) {
		TexturePtr min_tex = min_it.value();
		remove(_mapTree->getID(min_it.key()));
		_storage.erase(min_it);
		_storedBytes -= min_tex->numBytes();

		for ( Lookup::iterator lit = _firstLevel.begin(); lit != _firstLevel.end(); ) {
			if ( lit.value() == min_tex ) {
				lit = _firstLevel.erase(lit);
			}
			else
				++lit;
		}

		if ( _lastTile[0] == min_tex.get() )
			_lastTile[0] = NULL;

		if ( _lastTile[1] == min_tex.get() )
			_lastTile[1] = NULL;
	}
}


void TextureCache::setTexture(QImage &img, Alg::MapTreeNode *node) {
	// Update image cache
	{
		QMutexLocker lock(&imageCacheMutex);

		ImageCache::iterator it;
		QString id = _mapTree->getID(node);
		it = _images.find(id);

		// Image not yet cached, do nothing
		if ( it == _images.end() ) return;

		it->first = img;
	}

	// Update texture cache
	{
		Storage::iterator it = _storage.find(node);
		if ( it != _storage.end() ) {
			Texture *tex = it.value().get();

			// Update storage size
			_storedBytes -= tex->numBytes();
			tex->setImage(img);
			_storedBytes += tex->numBytes();

			checkResources();
		}
	}
}


void TextureCache::invalidateTexture(Alg::MapTreeNode *node) {
	QString id = _mapTree->getID(node);
	remove(id);

	{
		Lookup::iterator it = _firstLevel.find(TextureID(node->level(), node->row(), node->column()));
		if ( it != _firstLevel.end() )
			_firstLevel.erase(it);
	}

	{
		// Remove node from texture cache
		Storage::iterator it = _storage.find(node);
		if ( it != _storage.end() ) {
			if ( _lastTile[0] == it.value().get() )
				_lastTile[0] = NULL;

			if ( _lastTile[1] == it.value().get() )
				_lastTile[1] = NULL;

			Texture *tex = it.value().get();
			// Update storage size
			_storedBytes -= tex->numBytes();
			_storage.erase(it);
		}
	}
}


void TextureCache::clear() {
	QMutexLocker lock(&imageCacheMutex);

	_firstLevel.clear();
	_storage.clear();
	_images.clear();
	_storedBytes = 0;
	_lastTile[0] = _lastTile[1] = NULL;
	_currentIndex = 0;
	_currentTick = 0;
}


void TextureCache::remove(const QString &name) {
	QMutexLocker lock(&imageCacheMutex);
	ImageCache::iterator it;
	it = _images.find(name);
	if ( it != _images.end() ) {
		--it.value().second;
		if ( it.value().second == 0 )
			_images.erase(it);
	}
}


Alg::MapTreeNode *TextureCache::getNode(Alg::MapTreeNode *node, const TextureID &id) const {
	if ( node->level() == id.level ) {
		if ( node->row() == id.row && node->column() == id.column )
			return node;
		return NULL;
	}

	int shift = id.level-node->level()-1;
	int cells = 1 << shift;

	for ( int i = 0; i < 4; ++i ) {
		Alg::MapTreeNode *child = node->children(i);
		if ( !child ) {
			if ( !node->initialized(i) ) {
				node->loadChildren(_mapTree, i);
				child = node->children(i);
				if ( !child ) continue;
			}
			else
				continue;
		}

		int row = child->row() << shift;
		int col = child->column() << shift;
		if ( id.row >= row && id.row < row + cells &&
		     id.column >= col && id.column < col + cells ) {
			child = getNode(child, id);
			if ( child )
				return child;
			return node;
		}
	}

	return node;
}


uint qHash(const TextureID &id) {
	return ::qHash(id.level << 28 | id.row << 14 | id.column);
}


Texture *TextureCache::get(const TextureID &id) {
	Texture *tex;
	Storage::iterator it;

	quint64 oldTick = _currentTick;
	++_currentTick;
	// Wrap. Reset lastUsed of all other tiles
	if ( _currentTick < oldTick ) {
		for ( it = _storage.begin(); it != _storage.end(); ++it )
			it.value()->lastUsed = _currentTick;
		++_currentTick;
	}

	Alg::MapTreeNode *node = NULL;
	Lookup::iterator lit = _firstLevel.find(id);
	if ( lit != _firstLevel.end() )
		tex = *lit;
	else {
		node = getNode(_mapTree, id);
		it = _storage.find(node);
		if ( it != _storage.end() )
			tex = it.value().get();
		else {
			tex = new Texture;
			tex->load(this, node);

			_storage[node] = tex;
			_storedBytes += tex->numBytes();

			checkResources(tex);
		}

		_firstLevel[id] = tex;
	}

	tex->lastUsed = _currentTick;

	// If its a dummy texture then travel up the parent chain to check
	// for valid textures
	if ( tex->isDummy ) {
		if ( node == NULL ) node = getNode(_mapTree, id);

		while ( (node = node->parent()) != NULL ) {
			it = _storage.find(node);
			if ( it == _storage.end() )
				continue;

			Texture *tmp = it.value().get();
			if ( !tmp->isDummy ) {
				tex = tmp;
				break;
			}
		}
	}

	return tex;
}


}
}
}
