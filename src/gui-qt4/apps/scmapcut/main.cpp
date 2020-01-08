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

#include <seiscomp3/math/geo.h>
#include <seiscomp3/io/archive/xmlarchive.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/gui/core/application.h>
#include <seiscomp3/gui/map/projection.h>

#include "mapcut.h"


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::Gui;



void parseDim(const string &str, QSize *size) {
	size_t pos = str.find('x');

	if ( size ) *size = QSize(0,0);

	if ( pos == string::npos )
		return;

	int w,h;

	if ( !Core::fromString(w, str.substr(0,pos)) )
		return;

	size_t end = str.find('+', pos);
	if ( end == string::npos ) end = str.size();

	if ( !Core::fromString(h, str.substr(pos+1,end-pos-1)) )
		return;

	if ( size ) *size = QSize(w,h);
}


bool parseMargin(const string &str, QSizeF *size) {
	bool symmetric = false;

	size_t pos = str.find('x');

	if ( size ) *size = QSizeF(0,0);

	if ( pos == string::npos )
		symmetric = true;

	qreal w,h;

	if ( !Core::fromString(h, str.substr(0,pos)) )
		return false;

	if ( symmetric )
		w = h;
	else if ( !Core::fromString(w, str.substr(pos+1)) )
		return false;

	if ( size ) *size = QSizeF(w,h);
	return true;
}


void parseRegion(const string &str, QRectF *rect) {
	size_t pos = str.find('x');

	if ( rect ) *rect = QRectF();

	float w,h;
	float lat0, lon0;

	if ( pos == string::npos ) {
		// 4 values
		size_t end = min(str.find('+', 1), str.find('-', 1));
		if ( end == string::npos )
			return;

		if ( !Core::fromString(lat0, str.substr(0,end)) )
			return;

		size_t end2 = min(str.find('+', end+1), str.find('-', end+1));
		if ( end2 == string::npos )
			return;

		if ( !Core::fromString(lon0, str.substr(end,end2-end)) )
			return;

		end = end2;
		end2 = min(str.find('+', end+1), str.find('-', end+1));
		if ( end2 == string::npos )
			return;

		if ( !Core::fromString(w, str.substr(end,end2-end)) )
			return;

		end = end2;

		if ( !Core::fromString(h, str.substr(end)) )
			return;

		if ( rect ) *rect = QRectF(lon0, lat0, w-lon0, h-lat0);
		return;
	}

	if ( !Core::fromString(w, str.substr(0,pos)) )
		return;

	size_t end = min(str.find('+', pos), str.find('-', pos));
	if ( end == string::npos ) end = str.size();

	if ( !Core::fromString(h, str.substr(pos+1,end-pos-1)) )
		return;

	size_t end2 = min(str.find('+', end+1), str.find('-', end+1));
	if ( end2 == string::npos ) end2 = str.size();

	if ( !Core::fromString(lat0, str.substr(end,end2-end)) )
		return;

	if ( end2 == str.size() ) return;

	if ( !Core::fromString(lon0, str.substr(end2)) )
		return;

	if ( rect ) *rect = QRectF(lon0, lat0, h, w);
}


void setupView(Map::Canvas *canvas, const QRectF &rect) {
	canvas->displayRect(rect);

	/*
	QPointF center = rect.center();

	if ( canvas->width() <= 0 || canvas->height() <= 0 ) {
		canvas->setView(center, 1);
		return;
	}

	double dt = max(rect.width()/canvas->width(), rect.height()/canvas->height());
	if ( dt <= 0 ) {
		canvas->setView(center, 1);
		return;
	}

	canvas->setView(center, 360.0 / (canvas->width()*dt));
	*/
}


MapCut::MapCut(int& argc, char **argv, Seiscomp::Gui::Application::Type type)
: Application(argc, argv, 0, type) {
	setMessagingEnabled(false);
	setDatabaseEnabled(false, false);
	setRecordStreamEnabled(false);
	setLoadRegionsEnabled(true);

	_dim = QSize(0,0);
	_depth = 10;
	_magnitude = 0;
	_margins = QSizeF(0,0);
}

void MapCut::createCommandLineDescription() {
	commandline().addGroup("Options");
	commandline().addOption("Options", "region,r", "cut region ([lat_dim]x[lon_dim]+lat0+lon0 or +lat0+lon+lat1+lon1)", &_region, false);
	commandline().addOption("Options", "margin,m", "margin in degrees around origin (margin|margin_latxmargin_lon", &_margin, false);
	commandline().addOption("Options", "dimension,d", "output image dimension (wxh)", &_dimension, false);
	commandline().addOption("Options", "output,o", "output image", &_output, false);
	commandline().addOption("Options", "lat", "latitude of symbol", &_latitude, false);
	commandline().addOption("Options", "lon", "longitude of symbol", &_longitude, false);
	commandline().addOption("Options", "depth", "depth of event", &_depth, false);
	commandline().addOption("Options", "mag", "magnitude of event", &_magnitude, false);
	commandline().addOption("Options", "layers", "draw polygonal layers");
	commandline().addOption("Options", "ep", "EventParameters (XML) to load", &_epFile, false);
	commandline().addOption("Options", "event-id,E", "event to plot", &_eventID, false);
	commandline().addOption("Options", "html-area", "print html/area section");
}

bool MapCut::run() {
	SCScheme.map.vectorLayerAntiAlias = true;
	DataModel::EventParametersPtr ep;

	_htmlArea = commandline().hasOption("html-area");

	if ( !_epFile.empty() ) {
		IO::XMLArchive ar;
		if ( !ar.open(_epFile.c_str()) ) {
			cerr << "Unable to open file '" << _epFile << "'" << endl;
			return false;
		}

		ar >> ep;

		if ( ep == NULL ) {
			cerr << "File '" << _epFile << "' does not contain event parameters" << endl;
			return false;
		}
	}

	if ( !_eventID.empty() ) {
		if ( !ep ) {
			cerr << "No event parameters available, see --ep" << endl;
			return false;
		}

		DataModel::EventPtr evt = ep->findEvent(_eventID);
		if ( !evt ) {
			cerr << "Event '" << _eventID << "' not found" << endl;
			return false;
		}

		for ( size_t i = 0; i < ep->originCount(); ++i ) {
			if ( ep->origin(i)->publicID() == evt->preferredOriginID() ) {
				_org = ep->origin(i);
				break;
			}
		}

		if ( !_org ) {
			cerr << "Preferred origin for event '" << _eventID << "' not found" << endl;
			return false;
		}

		if ( !evt->preferredMagnitudeID().empty() ) {
			for ( size_t i = 0; i < _org->magnitudeCount(); ++i ) {
				if ( _org->magnitude(i)->publicID() == evt->preferredMagnitudeID() ) {
					try { _magnitude = _org->magnitude(i)->magnitude().value(); }
					catch ( ... ) {}
					break;
				}
			}
		}
	}
	// Grab first event
	else if ( ep ) {
		DataModel::EventPtr evt = ep->eventCount() > 0?ep->event(0):NULL;
		if ( !evt ) {
			cerr << "No event found in event parameters" << endl;
			return false;
		}

		for ( size_t i = 0; i < ep->originCount(); ++i ) {
			if ( ep->origin(i)->publicID() == evt->preferredOriginID() ) {
				_org = ep->origin(i);
				break;
			}
		}

		if ( !_org ) {
			cerr << "Preferred origin for event '" << _eventID << "' not found" << endl;
			return false;
		}

		if ( !evt->preferredMagnitudeID().empty() ) {
			for ( size_t i = 0; i < _org->magnitudeCount(); ++i ) {
				if ( _org->magnitude(i)->publicID() == evt->preferredMagnitudeID() ) {
					try { _magnitude = _org->magnitude(i)->magnitude().value(); }
					catch ( ... ) {}
					break;
				}
			}
		}
	}

	if ( commandline().hasOption("lat") && commandline().hasOption("lon") ) {
		_org = DataModel::Origin::Create();
		_org->setLatitude(_latitude);
		_org->setLongitude(_longitude);
		_org->setDepth(DataModel::RealQuantity(_depth));
	}

	if ( _output.empty() ) {
		cerr << "No output image given" << endl;
		return false;
	}

	parseDim(_dimension, &_dim);

	if ( _dim.isEmpty() ) {
		cerr << "Wrong output dimensions" << endl;
		return false;
	}

	if ( !_region.empty() ) {
		parseRegion(_region, &_reg);
		if ( _reg.isNull() || _reg.isEmpty() ) {
			cerr << "Invalid region: " << _region << endl;
			return false;
		}
	}

	if ( !_margin.empty() && !parseMargin(_margin, &_margins) ) {
		cerr << "Invalid margins: " << _margin << endl;
		return false;
	}

	if ( _region.empty() ) {
		if ( !_org ) {
			cerr << "No region and no origin given" << endl;
			return false;
		}

		if ( _margins.isEmpty() ) {
			cerr << "No region and no margins given" << endl;
			return false;
		}

		_reg = QRectF(_org->longitude()-_margins.width(),
		              _org->latitude()-_margins.height(),
		              _margins.width()*2, _margins.height()*2);
	}


	Map::ImageTreePtr mapTree = new Map::ImageTree(mapsDesc());

	_canvas = new Map::Canvas(mapTree.get());
	_canvas->setParent(this);
	_canvas->setSize(_dim.width(), _dim.height());

	setupView(_canvas, QRectF(_reg.left(),_reg.top(),_reg.width(),_reg.height()));

	_canvas->setPreviewMode(false);

	if ( commandline().hasOption("layers") )
		_canvas->setDrawLayers(true);

	_symbol = NULL;
	if ( _org ) {
		_symbol = new OriginSymbol(_org->latitude(), _org->longitude());
		_symbol->setDepth(_org->depth().value());
		_symbol->setPreferredMagnitudeValue(_magnitude);
		_canvas->symbolCollection()->add(_symbol);
	}

	connect(_canvas, SIGNAL(customLayer(QPainter*)),
	        this, SLOT(drawArrivals(QPainter*)));

	connect(_canvas, SIGNAL(updateRequested()), this, SLOT(renderCanvas()));
	renderCanvas();

	if ( !_canvas->renderingComplete() ) {
		connect(_canvas, SIGNAL(renderingCompleted()), this, SLOT(renderingCompleted()));
		cerr << "Requests in progress: waiting" << endl;

		return Application::run();
	}

	if ( !_canvas->buffer().save(_output.c_str(), NULL, 100) ) {
		cerr << "Saving the image failed" << endl;
		return false;
	}

	return true;
}


void MapCut::renderCanvas() {
	QPainter p(&_canvas->buffer());
	_canvas->draw(p);
}


void MapCut::renderingCompleted() {
	cerr << "Rendering finished" << endl;

	if ( !_canvas->buffer().save(_output.c_str(), NULL, 100) ) {
		cerr << "Saving the image failed" << endl;
		Client::Application::exit(1);
	}
	else
		Client::Application::quit();
}


void MapCut::drawArrivals(QPainter *p) {
	if ( !_org ) return;

	Map::Canvas *c = (Map::Canvas*)sender();

	QPoint originLocation;
	c->projection()->project(originLocation, QPointF(_symbol->longitude(), _symbol->latitude()));

	int cutOff = _symbol->size().width();
	if ( cutOff ) {
		p->setClipping(true);
		p->setClipRegion(QRegion(p->window()) -
		                 QRegion(QRect(originLocation.x() - cutOff/2, originLocation.y() - cutOff/2,
		                         cutOff, cutOff), QRegion::Ellipse));
	}

	p->setPen(SCScheme.colors.map.lines);
	for ( size_t i = 0; i < _org->arrivalCount(); ++i ) {
		DataModel::Arrival *arr = _org->arrival(i);

		try {
			double lat, lon;
			Math::Geo::delandaz2coord(arr->distance(), arr->azimuth(),
			                          _org->latitude(), _org->longitude(),
			                          &lat, &lon);

			c->drawLine(*p, QPointF(_symbol->longitude(), _symbol->latitude()), QPointF(lon,lat));
		}
		catch ( ... ) {}
	}

	if ( cutOff )
		p->setClipping(false);

	int r = SCScheme.map.stationSize/2;
	QRect screen = p->window().adjusted(-r,-r,r,r);

	p->setPen(SCScheme.colors.map.outlines);
	for ( int i = (int)_org->arrivalCount()-1; i >= 0; --i ) {
		DataModel::Arrival *arr = _org->arrival(i);

		try {
			double lat, lon;
			Math::Geo::delandaz2coord(arr->distance(), arr->azimuth(),
			                          _org->latitude(), _org->longitude(),
			                          &lat, &lon);

			bool enabled;
			try {
				enabled = arr->weight() > 0;
			}
			catch ( ... ) {
				enabled = true;
			}

			if ( enabled ) {
				try {
					p->setBrush(SCScheme.colors.arrivals.residuals.colorAt(arr->timeResidual()));
				}
				catch ( ... ) {
					p->setBrush(SCScheme.colors.arrivals.undefined);
				}
			}
			else
				p->setBrush(SCScheme.colors.arrivals.disabled);

			QPoint pp;
			if ( c->projection()->project(pp, QPointF(lon,lat)) ) {
				p->drawEllipse(pp.x()-r, pp.y()-r, SCScheme.map.stationSize, SCScheme.map.stationSize);

				if ( _htmlArea && screen.contains(pp) ) {
					cout << "<area shape=\"circle\" coords=\""
					     << pp.x() << "," << pp.y() << "," << r << "\" "
					     << "id=\"" << arr->pickID() << "\"/>" << endl;
				}
			}
		}
		catch ( ... ) {}
	}
}



int main(int argc, char ** argv) {
	Application::Type type = Application::Tty;
	// Qt 4.2.x crashes when rendering text with console
	// applications so we enable console application only if
	// at least Qt 4.3.0 is installed.
	type = Application::Tty;
	if ( !Application::minQtVersion("4.3.0") )
		type = Application::GuiClient;

	MapCut app(argc, argv,type);
	return app();
}
