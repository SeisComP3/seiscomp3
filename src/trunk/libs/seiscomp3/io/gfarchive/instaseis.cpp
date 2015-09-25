/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *   -------------------------------------------------------------------   *
 *   Author: Jan Becker <jabe@gempa.de>                                    *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#define SEISCOMP_COMPONENT Instaseis


#include <seiscomp3/logging/log.h>
#include <seiscomp3/core/strings.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/core/greensfunction.h>
#include <seiscomp3/core/system.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/io/gfarchive/instaseis.h>
#include <seiscomp3/io/records/mseedrecord.h>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>


using namespace std;


namespace {

const string DefaultHost = "localhost";
const string DefaultPort = "8765";

}


namespace Seiscomp {
namespace IO {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_GFARCHIVE(Instaseis, "instaseis");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Instaseis::Instaseis() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Instaseis::Instaseis(const string &url) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Instaseis::~Instaseis() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Instaseis::setSource(string source) {
	// Close socket
	_socket.close();

	_minDist = 0;
	_maxDist = 180;

	_minDepth = 0;
	_maxDepth = 700;

	_srcShift = 0;
	_maxLength = -1;
	_dt = 1;

	_timeout = 0;

	size_t pos = source.find('?');
	if ( pos != string::npos ) {
		_host = source.substr(0, pos);

		string params = source.substr(pos+1);
		vector<string> toks;
		Core::split(toks, params.c_str(), "&");
		if ( !toks.empty() ) {
			for ( vector<string>::iterator it = toks.begin();
			      it != toks.end(); ++it ) {
				string name, value;

				pos = it->find('=');
				if ( pos != string::npos ) {
					name = it->substr(0, pos);
					value = it->substr(pos+1);
				}
				else {
					name = *it;
					value = "";
				}

				if ( name == "model" )
					_model = value;
				else if ( name == "maxdepth" ) {
					if ( !Core::fromString(_maxDepth, value) ) {
						SEISCOMP_ERROR("Invalid value for 'maxdepth': %s", value.c_str());
						return false;
					}
				}
				else if ( name == "timeout" ) {
					if ( !Core::fromString(_timeout, value) ) {
						SEISCOMP_ERROR("Invalid value for 'timeout': %s", value.c_str());
						return false;
					}
				}
			}
		}
	}
	else
		_host = source;

	// set address defaults if necessary
	if ( _host.empty() || _host == ":" )
		_host = DefaultHost + ":" + DefaultPort;
	else {
		pos = _host.find(':');
		if ( pos == string::npos )
			_host += ":" + DefaultPort;
		else if ( pos == _host.length()-1 )
			_host += DefaultPort;
		else if ( pos == 0 )
			_host.insert(0, DefaultHost);
	}

	if ( _timeout > 0 )
		_socket.setTimeout(_timeout);

	try {
		_socket.open(_host);

		string request = "GET /info HTTP/1.1\r\n"
		                 "Host: " + _host + "\r\n"
		                 "\r\n";
		string response;

		_socket.write(request);

		response = _socket.readline();
		if ( response.compare(0, 9, "HTTP/1.1 ") ) {
			SEISCOMP_ERROR("Expected HTTP/1.1 response, got: %s", response.c_str());
			_socket.close();
			return false;
		}

		response.erase(response.begin(), response.begin() + 9);
		Core::trim(response);
		if ( response.compare(0, 3, "200") ) {
			SEISCOMP_ERROR("Expected status 200, got: %s", response.substr(0,3).c_str());
			_socket.close();
			return false;
		}

		int contentLength = -1;

		while ( true ) {
			response = _socket.readline();
			// End of header
			if ( response.empty() )
				break;
			else {
				Core::trim(response);
				size_t pos = response.find(':');
				if ( pos == string::npos ) {
					SEISCOMP_ERROR("Invalid response header: %s", response.c_str());
					_socket.close();
					return false;
				}

				if ( !response.compare(0, pos, "Content-Length") ) {
					response.erase(response.begin(), response.begin() + pos + 1);
					Core::trim(response);
					if ( !Core::fromString(contentLength, response) ) {
						SEISCOMP_ERROR("Invalid Content-Length header, expected numeric value, got: %s", response.c_str());
						_socket.close();
						return false;
					}
				}
			}
		}

		if ( contentLength <= 0 ) {
			SEISCOMP_ERROR("No content, Content-Length = %d", contentLength);
			_socket.close();
			return false;
		}

		//SEISCOMP_DEBUG("Content-Length = %d", contentLength);
		string json;

		while ( contentLength > 0 ) {
			string data = _socket.read(contentLength > BUFSIZE ? BUFSIZE : contentLength);
			json += data;
			contentLength -= (int)data.size();
		}

		_socket.close();

		rapidjson::Document doc;
		doc.ParseInsitu(&json[0]);
		if ( doc.HasParseError() ) {
			SEISCOMP_WARNING("Received invalid JSON");
			_socket.close();
			return false;
		}

		rapidjson::Value::ConstMemberIterator jitr;

		if ( (jitr = doc.FindMember("velocity_model")) != doc.MemberEnd() && jitr->value.IsString() )
			_model = jitr->value.GetString();

		if ( (jitr = doc.FindMember("min_d")) != doc.MemberEnd() && jitr->value.IsNumber() )
			_minDist = jitr->value.GetDouble();

		if ( (jitr = doc.FindMember("max_d")) != doc.MemberEnd() && jitr->value.IsNumber() )
			_maxDist = jitr->value.GetDouble();

		double planet_radius = -1, min_radius = -1, max_radius = -1;
		double dt = 1;

		if ( (jitr = doc.FindMember("planet_radius")) != doc.MemberEnd() && jitr->value.IsNumber() )
			planet_radius = jitr->value.GetDouble();

		if ( (jitr = doc.FindMember("min_radius")) != doc.MemberEnd() && jitr->value.IsNumber() )
			min_radius = jitr->value.GetDouble();

		if ( (jitr = doc.FindMember("max_radius")) != doc.MemberEnd() && jitr->value.IsNumber() )
			max_radius = jitr->value.GetDouble();

		if ( planet_radius >= 0 && max_radius >= 0 )
			_minDepth = (planet_radius - max_radius) * 1E-3;

		if ( planet_radius >= 0 && min_radius >= 0 )
			_maxDepth = (planet_radius - min_radius) * 1E-3;

		if ( (jitr = doc.FindMember("dt")) != doc.MemberEnd() && jitr->value.IsNumber() ) {
			dt = jitr->value.GetDouble();
			while ( _dt > dt )
				_dt *= 0.5;
		}

		if ( (jitr = doc.FindMember("src_shift")) != doc.MemberEnd() && jitr->value.IsNumber() )
			_srcShift = jitr->value.GetDouble();

		if ( (jitr = doc.FindMember("length")) != doc.MemberEnd() && jitr->value.IsNumber() )
			_maxLength = (int)(jitr->value.GetDouble() - _srcShift - 12*dt);

		SEISCOMP_DEBUG("Model: %s", _model.c_str());
		SEISCOMP_DEBUG("Distance range: %f deg - %f deg", _minDist, _maxDist);
		SEISCOMP_DEBUG("Depth range: %f km - %f km", _minDepth, _maxDepth);
		SEISCOMP_DEBUG("Max length: %ds", _maxLength);
		SEISCOMP_DEBUG("Dt: %fs, will request %fs", dt, _dt);

		_minDist = Math::Geo::deg2km(_minDist);
		_maxDist = Math::Geo::deg2km(_maxDist);
	}
	catch ( std::exception &e ) {
		SEISCOMP_ERROR("%s", e.what());
		_socket.close();
		return false;
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Instaseis::close() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
list<string> Instaseis::availableModels() const {
	list<string> models;
	models.push_back(_model);
	return models;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
list<double> Instaseis::availableDepths(const string &model) const {
	list<double> depths;

	if ( model != _model )
		return depths;

	double depth = _minDepth;
	while ( depth < _maxDepth ) {
		depths.push_back(depth);
		depth += 1.0;
	}
	depths.push_back(_maxDepth);

	// Populate list
	return depths;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Instaseis::setTimeSpan(const Core::TimeSpan &span) {
	_defaultTimespan = span;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Instaseis::addRequest(const string &id,
                           const string &model, double distance,
                           double depth) {
	if ( _model != model ) {
		SEISCOMP_DEBUG("Wrong model: %s", model.c_str());
		return false;
	}

	if ( (distance < _minDist) || (distance > _maxDist) ) {
		SEISCOMP_DEBUG("Depth out of range: %f", depth);
		return false;
	}

	if ( (depth < _minDepth) || (depth > _maxDepth) ) {
		SEISCOMP_DEBUG("Distance out of range: %f", distance);
		return false;
	}

	_requests.push_back(Request());
	_requests.back().id = id;
	_requests.back().distance = distance;
	_requests.back().depth = depth;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Instaseis::addRequest(const string &id,
                           const string &model, double distance,
                           double depth,
                           const Core::TimeSpan &span) {
	if ( _model != model ) {
		SEISCOMP_DEBUG("Wrong model: %s", model.c_str());
		return false;
	}

	if ( (distance < _minDist) || (distance > _maxDist) ) {
		SEISCOMP_DEBUG("Depth out of range: %f", depth);
		return false;
	}

	if ( (depth < _minDepth) || (depth > _maxDepth) ) {
		SEISCOMP_DEBUG("Distance out of range: %f", distance);
		return false;
	}

	_requests.push_back(Request());
	_requests.back().id = id;
	_requests.back().distance = distance;
	_requests.back().depth = depth;
	_requests.back().timeSpan = span;

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::GreensFunction *Instaseis::get() {
	while ( !_requests.empty() ) {
		Request req = _requests.front();
		_requests.pop_front();

		try {
			if ( !_socket.isOpen() || _socket.tryReconnect() )
				_socket.open(_host);

			double ts = _defaultTimespan;
			if ( req.timeSpan ) ts = req.timeSpan;

			if ( (_maxLength > 0) && (ts >= _maxLength) )
				ts = _maxLength;

			string timespan = ts > 0 ? ("&endtime=" + Core::toString((double)ts)) : "";
			string request = "GET /greens_function"
			                 "?sourcedepthinmeters=" + Core::toString(req.depth*1000) +
			                 "&sourcedistanceindegrees=" + Core::toString(Math::Geo::km2deg(req.distance)) +
			                 "&format=miniseed" + timespan +
			                 "&dt=" + Core::toString(_dt) +
			                 "&units=displacement HTTP/1.1\r\n"
			                 "Host: " + _host + "\r\n"
			                 "\r\n";
			string response;

			SEISCOMP_DEBUG("Request: depth = %f km, distance = %f km\n"
			               "HTTP\n%s",
			               req.depth, req.distance, request.c_str());

			_socket.write(request);

			response = _socket.readline();
			if ( response.compare(0, 9, "HTTP/1.1 ") ) {
				SEISCOMP_ERROR("Expected HTTP/1.1 response, got: %s", response.c_str());
				_socket.close();
				return NULL;
			}

			response.erase(response.begin(), response.begin() + 9);
			Core::trim(response);
			if ( response.compare(0, 3, "200") ) {
				SEISCOMP_ERROR("Expected status 200, got: %s", response.substr(0,3).c_str());
				_socket.close();
				return NULL;
			}

			int contentLength = -1;

			while ( true ) {
				response = _socket.readline();
				// End of header
				if ( response.empty() )
					break;
				else {
					Core::trim(response);
					size_t pos = response.find(':');
					if ( pos == string::npos ) {
						SEISCOMP_ERROR("Invalid response header: %s", response.c_str());
						_socket.close();
						return NULL;
					}

					if ( !response.compare(0, pos, "Content-Length") ) {
						response.erase(response.begin(), response.begin() + pos + 1);
						Core::trim(response);
						if ( !Core::fromString(contentLength, response) ) {
							SEISCOMP_ERROR("Invalid Content-Length header, expected numeric value, got: %s", response.c_str());
							_socket.close();
							return NULL;
						}
					}
				}
			}

			if ( contentLength <= 0 ) {
				SEISCOMP_ERROR("No content, Content-Length = %d", contentLength);
				_socket.close();
				return NULL;
			}

			//SEISCOMP_DEBUG("Content-Length = %d", contentLength);
			stringstream ss;

			while ( contentLength > 0 ) {
				string data = _socket.read(contentLength > BUFSIZE ? BUFSIZE : contentLength);
				ss.write(&data[0], data.size());
				contentLength -= (int)data.size();
			}

			_socket.close();

			Core::GreensFunction *gf = new Core::GreensFunction;

			try {
				while ( true ) {
					MSeedRecord rec;
					rec.read(ss);

					Core::GreensFunctionComponent comp;
					if ( comp.fromString(rec.channelCode()) ) {
						ArrayPtr recdata = rec.data()->copy(Array::FLOAT);
						if ( recdata ) {
							FloatArrayPtr array = (FloatArray*)gf->data(comp);
							if ( array == NULL ) {
								array = new FloatArray;
								gf->setData(comp, array.get());
							}

							array->append(recdata.get());
							gf->setSamplingFrequency(rec.samplingFrequency());
						}
						else
							SEISCOMP_WARNING("Could not convert to float for comp %s", comp.toString());
					}
				}
			}
			catch ( Core::EndOfStreamException &e ) {
				// Do nothing
			}
			catch ( std::exception &e ) {
				delete gf;
				throw e;
			}

			// Convert from m to cm
			for ( int i = 0; i < Core::GreensFunctionComponent::Quantity; ++i ) {
				FloatArray *data = (FloatArray*)gf->data(i);
				if ( data != NULL ) {
					// Convert to dyn-cm
					*data *= 1E15;
				}
			}

			gf->setDepth(req.depth);
			gf->setDistance(req.distance);
			gf->setId(req.id);
			gf->setTimeOffset(_srcShift);
			gf->setModel(_model);

			return gf;
		}
		catch ( std::exception &e ) {
			SEISCOMP_ERROR("%s", e.what());
			_socket.close();
			return NULL;
		}
	}

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
