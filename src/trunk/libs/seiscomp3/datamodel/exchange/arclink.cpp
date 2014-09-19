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



#include "arclink.h"

#include <seiscomp3/datamodel/inventory_package.h>
#include <seiscomp3/datamodel/routing_package.h>
#include <seiscomp3/datamodel/arclinklog_package.h>
#include <iostream>


namespace Seiscomp {
namespace DataModel {


REGISTER_IMPORTER_INTERFACE(ImporterArclink, "arclink");
REGISTER_EXPORTER_INTERFACE(ExporterArclink, "arclink");



/* Example XML

<?xml version="1.0" encoding="utf-8"?>
<ns0:inventory xmlns:ns0="http://geofon.gfz-potsdam.de/ns/Inventory/1.0/">
  <ns0:network archive="ODC" code="BE" description="ROB" end=""
               institutions="" netClass="p" publicID="Network/BE"
               region="" restricted="false" shared="true"
               start="1980-01-01T00:00:00.0000Z" type="BB">
    <ns0:remark />
  </ns0:network>
</ns0:inventory>

*/


namespace {



struct TypeMap : public IO::XML::TypeMap {
	TypeMap();
};


TypeMap __typeMap;


template <typename T>
class PublicIDSetter : public IO::XML::MemberHandler {
	public:
		PublicIDSetter() {}

		std::string value(Core::BaseObject *obj) {
			T *target = T::Cast(obj);
			if ( !target ) return "";

			return target->publicID();
		}

		bool get(Core::BaseObject *object, void *n, IO::XML::NodeHandler *h) {
			T *target = T::Cast(object);
			if ( !target ) return false;

			target->setPublicID(h->content(n));
			return true;
		}
};



struct BlobHandler : public IO::XML::TypedClassHandler<Blob> {
	BlobHandler() {
		addProperty("content", "", Optional, CDATA, "content");
	}
};


struct RealArrayHandler : public IO::XML::TypedClassHandler<RealArray> {
	RealArrayHandler() {
		addProperty("content", "", Optional, CDATA, "content");
	}
};


struct ComplexArrayHandler : public IO::XML::TypedClassHandler<ComplexArray> {
	ComplexArrayHandler() {
		addProperty("content", "", Optional, CDATA, "content");
	}
};


struct StationReferenceHandler : public IO::XML::TypedClassHandler<StationReference> {
	StationReferenceHandler() {
		addProperty("stationID", "", Mandatory, Attribute, "stationID");
	}
};


struct StationGroupHandler : public IO::XML::TypedClassHandler<StationGroup> {
	StationGroupHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<StationGroup>());
		addProperty("type", "", Optional, Attribute, "type");
		addProperty("code", "", Optional, Attribute, "code");
		addProperty("start", "", Optional, Attribute, "start");
		addProperty("end", "", Optional, Attribute, "end");
		addProperty("description", "", Optional, Attribute, "description");
		addProperty("latitude", "", Optional, Attribute, "latitude");
		addProperty("longitude", "", Optional, Attribute, "longitude");
		addProperty("elevation", "", Optional, Attribute, "elevation");
		addChildProperty("stationReference", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "stationReference");
	}
};


struct AuxSourceHandler : public IO::XML::TypedClassHandler<AuxSource> {
	AuxSourceHandler() {
		addProperty("name", "", Mandatory, Attribute, "name");
		addProperty("description", "", Optional, Attribute, "description");
		addProperty("unit", "", Optional, Attribute, "unit");
		addProperty("conversion", "", Optional, Attribute, "conversion");
		addProperty("sampleRateNumerator", "", Optional, Attribute, "sampleRateNumerator");
		addProperty("sampleRateDenominator", "", Optional, Attribute, "sampleRateDenominator");
		addProperty("remark", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "remark");
	}
};


struct AuxDeviceHandler : public IO::XML::TypedClassHandler<AuxDevice> {
	AuxDeviceHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<AuxDevice>());
		addProperty("name", "", Mandatory, Attribute, "name");
		addProperty("description", "", Optional, Attribute, "description");
		addProperty("model", "", Optional, Attribute, "model");
		addProperty("manufacturer", "", Optional, Attribute, "manufacturer");
		addProperty("remark", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "remark");
		addChildProperty("source", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "source");
	}
};


struct SensorCalibrationHandler : public IO::XML::TypedClassHandler<SensorCalibration> {
	SensorCalibrationHandler() {
		addProperty("serialNumber", "", Mandatory, Attribute, "serialNumber");
		addProperty("channel", "", Mandatory, Attribute, "channel");
		addProperty("start", "", Mandatory, Attribute, "start");
		addProperty("end", "", Optional, Attribute, "end");
		addProperty("gain", "", Optional, Attribute, "gain");
		addProperty("gainFrequency", "", Optional, Attribute, "gainFrequency");
		addProperty("remark", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "remark");
	}
};


struct SensorHandler : public IO::XML::TypedClassHandler<Sensor> {
	SensorHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<Sensor>());
		addProperty("name", "", Mandatory, Attribute, "name");
		addProperty("description", "", Optional, Attribute, "description");
		addProperty("model", "", Optional, Attribute, "model");
		addProperty("manufacturer", "", Optional, Attribute, "manufacturer");
		addProperty("type", "", Optional, Attribute, "type");
		addProperty("unit", "", Optional, Attribute, "unit");
		addProperty("lowFrequency", "", Optional, Attribute, "lowFrequency");
		addProperty("highFrequency", "", Optional, Attribute, "highFrequency");
		addProperty("response", "", Optional, Attribute, "response");
		addProperty("remark", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "remark");
		addChildProperty("calibration", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "calibration");
	}
};


struct ResponsePAZHandler : public IO::XML::TypedClassHandler<ResponsePAZ> {
	ResponsePAZHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<ResponsePAZ>());
		addProperty("name", "", Optional, Attribute, "name");
		addProperty("type", "", Optional, Attribute, "type");
		addProperty("gain", "", Optional, Attribute, "gain");
		addProperty("gainFrequency", "", Optional, Attribute, "gainFrequency");
		addProperty("normalizationFactor", "", Optional, Attribute, "normalizationFactor");
		addProperty("normalizationFrequency", "", Optional, Attribute, "normalizationFrequency");
		addProperty("numberOfZeros", "", Optional, Attribute, "numberOfZeros");
		addProperty("numberOfPoles", "", Optional, Attribute, "numberOfPoles");
		addProperty("zeros", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "zeros");
		addProperty("poles", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "poles");
		addProperty("remark", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "remark");
	}
};


struct ResponsePolynomialHandler : public IO::XML::TypedClassHandler<ResponsePolynomial> {
	ResponsePolynomialHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<ResponsePolynomial>());
		addProperty("name", "", Optional, Attribute, "name");
		addProperty("gain", "", Optional, Attribute, "gain");
		addProperty("gainFrequency", "", Optional, Attribute, "gainFrequency");
		addProperty("frequencyUnit", "", Optional, Attribute, "frequencyUnit");
		addProperty("approximationType", "", Optional, Attribute, "approximationType");
		addProperty("approximationLowerBound", "", Optional, Attribute, "approximationLowerBound");
		addProperty("approximationUpperBound", "", Optional, Attribute, "approximationUpperBound");
		addProperty("approximationError", "", Optional, Attribute, "approximationError");
		addProperty("numberOfCoefficients", "", Optional, Attribute, "numberOfCoefficients");
		addProperty("coefficients", "", Optional, Attribute, "coefficients");
		addProperty("remark", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "remark");
	}
};


struct DataloggerCalibrationHandler : public IO::XML::TypedClassHandler<DataloggerCalibration> {
	DataloggerCalibrationHandler() {
		addProperty("serialNumber", "", Mandatory, Attribute, "serialNumber");
		addProperty("channel", "", Mandatory, Attribute, "channel");
		addProperty("start", "", Mandatory, Attribute, "start");
		addProperty("end", "", Optional, Attribute, "end");
		addProperty("gain", "", Optional, Attribute, "gain");
		addProperty("gainFrequency", "", Optional, Attribute, "gainFrequency");
		addProperty("remark", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "remark");
	}
};


struct DecimationHandler : public IO::XML::TypedClassHandler<Decimation> {
	DecimationHandler() {
		addProperty("sampleRateNumerator", "", Mandatory, Attribute, "sampleRateNumerator");
		addProperty("sampleRateDenominator", "", Mandatory, Attribute, "sampleRateDenominator");
		addProperty("analogueFilterChain", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "analogueFilterChain");
		addProperty("digitalFilterChain", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "digitalFilterChain");
	}
};


struct DataloggerHandler : public IO::XML::TypedClassHandler<Datalogger> {
	DataloggerHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<Datalogger>());
		addProperty("name", "", Optional, Attribute, "name");
		addProperty("description", "", Optional, Attribute, "description");
		addProperty("digitizerModel", "", Optional, Attribute, "digitizerModel");
		addProperty("digitizerManufacturer", "", Optional, Attribute, "digitizerManufacturer");
		addProperty("recorderModel", "", Optional, Attribute, "recorderModel");
		addProperty("recorderManufacturer", "", Optional, Attribute, "recorderManufacturer");
		addProperty("clockModel", "", Optional, Attribute, "clockModel");
		addProperty("clockManufacturer", "", Optional, Attribute, "clockManufacturer");
		addProperty("clockType", "", Optional, Attribute, "clockType");
		addProperty("gain", "", Optional, Attribute, "gain");
		addProperty("maxClockDrift", "", Optional, Attribute, "maxClockDrift");
		addProperty("remark", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "remark");
		addChildProperty("calibration", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "calibration");
		addChildProperty("decimation", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "decimation");
	}
};


struct ResponseFIRHandler : public IO::XML::TypedClassHandler<ResponseFIR> {
	ResponseFIRHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<ResponseFIR>());
		addProperty("name", "", Optional, Attribute, "name");
		addProperty("gain", "", Optional, Attribute, "gain");
		addProperty("decimationFactor", "", Optional, Attribute, "decimationFactor");
		addProperty("delay", "", Optional, Attribute, "delay");
		addProperty("correction", "", Optional, Attribute, "correction");
		addProperty("numberOfCoefficients", "", Optional, Attribute, "numberOfCoefficients");
		addProperty("symmetry", "", Optional, Attribute, "symmetry");
		addProperty("coefficients", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "coefficients");
		addProperty("remark", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "remark");
	}
};


struct AuxStreamHandler : public IO::XML::TypedClassHandler<AuxStream> {
	AuxStreamHandler() {
		addProperty("code", "", Mandatory, Attribute, "code");
		addProperty("start", "", Mandatory, Attribute, "start");
		addProperty("end", "", Optional, Attribute, "end");
		addProperty("device", "", Optional, Attribute, "device");
		addProperty("deviceSerialNumber", "", Optional, Attribute, "deviceSerialNumber");
		addProperty("source", "", Optional, Attribute, "source");
		addProperty("format", "", Optional, Attribute, "format");
		addProperty("flags", "", Optional, Attribute, "flags");
		addProperty("restricted", "", Optional, Attribute, "restricted");
		addProperty("shared", "", Optional, Attribute, "shared");
	}
};


struct StreamHandler : public IO::XML::TypedClassHandler<Stream> {
	StreamHandler() {
		addProperty("code", "", Mandatory, Attribute, "code");
		addProperty("start", "", Mandatory, Attribute, "start");
		addProperty("end", "", Optional, Attribute, "end");
		addProperty("datalogger", "", Optional, Attribute, "datalogger");
		addProperty("dataloggerSerialNumber", "", Optional, Attribute, "dataloggerSerialNumber");
		addProperty("dataloggerChannel", "", Optional, Attribute, "dataloggerChannel");
		addProperty("sensor", "", Optional, Attribute, "sensor");
		addProperty("sensorSerialNumber", "", Optional, Attribute, "sensorSerialNumber");
		addProperty("sensorChannel", "", Optional, Attribute, "sensorChannel");
		addProperty("clockSerialNumber", "", Optional, Attribute, "clockSerialNumber");
		addProperty("sampleRateNumerator", "", Optional, Attribute, "sampleRateNumerator");
		addProperty("sampleRateDenominator", "", Optional, Attribute, "sampleRateDenominator");
		addProperty("depth", "", Optional, Attribute, "depth");
		addProperty("azimuth", "", Optional, Attribute, "azimuth");
		addProperty("dip", "", Optional, Attribute, "dip");
		addProperty("gain", "", Optional, Attribute, "gain");
		addProperty("gainFrequency", "", Optional, Attribute, "gainFrequency");
		addProperty("gainUnit", "", Optional, Attribute, "gainUnit");
		addProperty("format", "", Optional, Attribute, "format");
		addProperty("flags", "", Optional, Attribute, "flags");
		addProperty("restricted", "", Optional, Attribute, "restricted");
		addProperty("shared", "", Optional, Attribute, "shared");
	}
};


struct SensorLocationHandler : public IO::XML::TypedClassHandler<SensorLocation> {
	SensorLocationHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<SensorLocation>());
		addProperty("code", "", Mandatory, Attribute, "code");
		addProperty("start", "", Mandatory, Attribute, "start");
		addProperty("end", "", Optional, Attribute, "end");
		addProperty("latitude", "", Optional, Attribute, "latitude");
		addProperty("longitude", "", Optional, Attribute, "longitude");
		addProperty("elevation", "", Optional, Attribute, "elevation");
		addChildProperty("auxStream", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "auxStream");
		addChildProperty("stream", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "stream");
	}
};


struct StationHandler : public IO::XML::TypedClassHandler<Station> {
	StationHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<Station>());
		addProperty("code", "", Mandatory, Attribute, "code");
		addProperty("start", "", Mandatory, Attribute, "start");
		addProperty("end", "", Optional, Attribute, "end");
		addProperty("description", "", Optional, Attribute, "description");
		addProperty("latitude", "", Optional, Attribute, "latitude");
		addProperty("longitude", "", Optional, Attribute, "longitude");
		addProperty("elevation", "", Optional, Attribute, "elevation");
		addProperty("place", "", Optional, Attribute, "place");
		addProperty("country", "", Optional, Attribute, "country");
		addProperty("affiliation", "", Optional, Attribute, "affiliation");
		addProperty("type", "", Optional, Attribute, "type");
		addProperty("archive", "", Optional, Attribute, "archive");
		addProperty("archiveNetworkCode", "", Optional, Attribute, "archiveNetworkCode");
		addProperty("restricted", "", Optional, Attribute, "restricted");
		addProperty("shared", "", Optional, Attribute, "shared");
		addProperty("remark", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "remark");
		addChildProperty("sensorLocation", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "sensorLocation");
	}
};


struct NetworkHandler : public IO::XML::TypedClassHandler<Network> {
	NetworkHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<Network>());
		addProperty("code", "", Mandatory, Attribute, "code");
		addProperty("start", "", Mandatory, Attribute, "start");
		addProperty("end", "", Optional, Attribute, "end");
		addProperty("description", "", Optional, Attribute, "description");
		addProperty("institutions", "", Optional, Attribute, "institutions");
		addProperty("region", "", Optional, Attribute, "region");
		addProperty("type", "", Optional, Attribute, "type");
		addProperty("netClass", "", Optional, Attribute, "netClass");
		addProperty("archive", "", Optional, Attribute, "archive");
		addProperty("restricted", "", Optional, Attribute, "restricted");
		addProperty("shared", "", Optional, Attribute, "shared");
		addProperty("remark", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", Optional, Element, "remark");
		addChildProperty("station", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "station");
	}
};


struct InventoryHandler : public IO::XML::TypedClassHandler<Inventory> {
	InventoryHandler() {
		addChildProperty("network", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "network");
		addChildProperty("stationGroup", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "stationGroup");
		addChildProperty("auxDevice", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "auxDevice");
		addChildProperty("datalogger", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "datalogger");
		addChildProperty("sensor", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "sensor");
		addChildProperty("responsePAZ", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "responsePAZ");
		addChildProperty("responseFIR", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "responseFIR");
		addChildProperty("responsePolynomial", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "responsePolynomial");
	}
};


struct RouteArclinkHandler : public IO::XML::TypedClassHandler<RouteArclink> {
	RouteArclinkHandler() {
		addProperty("address", "", Mandatory, Attribute, "address");
		addProperty("start", "", Mandatory, Attribute, "start");
		addProperty("end", "", Optional, Attribute, "end");
		addProperty("priority", "", Optional, Attribute, "priority");
	}
};


struct RouteSeedlinkHandler : public IO::XML::TypedClassHandler<RouteSeedlink> {
	RouteSeedlinkHandler() {
		addProperty("address", "", Mandatory, Attribute, "address");
		addProperty("priority", "", Optional, Attribute, "priority");
	}
};


struct RouteHandler : public IO::XML::TypedClassHandler<Route> {
	RouteHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<Route>());
		addProperty("networkCode", "", Mandatory, Attribute, "networkCode");
		addProperty("stationCode", "", Mandatory, Attribute, "stationCode");
		addProperty("locationCode", "", Mandatory, Attribute, "locationCode");
		addProperty("streamCode", "", Mandatory, Attribute, "streamCode");
		addChildProperty("arclink", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "arclink");
		addChildProperty("seedlink", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "seedlink");
	}
};


struct AccessHandler : public IO::XML::TypedClassHandler<Access> {
	AccessHandler() {
		addProperty("networkCode", "", Mandatory, Attribute, "networkCode");
		addProperty("stationCode", "", Mandatory, Attribute, "stationCode");
		addProperty("locationCode", "", Mandatory, Attribute, "locationCode");
		addProperty("streamCode", "", Mandatory, Attribute, "streamCode");
		addProperty("user", "", Mandatory, Attribute, "user");
		addProperty("start", "", Mandatory, Attribute, "start");
		addProperty("end", "", Optional, Attribute, "end");
	}
};


struct RoutingHandler : public IO::XML::TypedClassHandler<Routing> {
	RoutingHandler() {
		addChildProperty("route", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "route");
		addChildProperty("access", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "access");
	}
};



struct ArclinkUserHandler : public IO::XML::TypedClassHandler<ArclinkUser> {
	ArclinkUserHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<ArclinkUser>());
		addProperty("name", "", Mandatory, Attribute, "name");
		addProperty("email", "", Optional, Attribute, "email");
		addProperty("password", "", Optional, Attribute, "password");
	}
};


struct ArclinkStatusLineHandler : public IO::XML::TypedClassHandler<ArclinkStatusLine> {
	ArclinkStatusLineHandler() {
		addProperty("type", "", Mandatory, Attribute, "type");
		addProperty("status", "", Mandatory, Attribute, "status");
		addProperty("size", "", Optional, Attribute, "size");
		addProperty("message", "", Optional, Attribute, "message");
		addProperty("volumeID", "", Optional, Attribute, "volumeID");
	}
};


struct ArclinkRequestLineHandler : public IO::XML::TypedClassHandler<ArclinkRequestLine> {
	ArclinkRequestLineHandler() {
		addProperty("start", "", Mandatory, Attribute, "start");
		addProperty("end", "", Mandatory, Attribute, "end");
		addProperty("streamID", "", Mandatory, Attribute, "streamID");
		addProperty("restricted", "", Optional, Attribute, "restricted");
		addProperty("shared", "", Optional, Attribute, "shared");
		addProperty("netClass", "", Optional, Attribute, "netClass");
		addProperty("constraints", "", Optional, Attribute, "constraints");
		addProperty("status", "", Mandatory, Attribute, "status");
	}
};


struct ArclinkRequestSummaryHandler : public IO::XML::TypedClassHandler<ArclinkRequestSummary> {
	ArclinkRequestSummaryHandler() {
		addProperty("okLineCount", "", Mandatory, Attribute, "okLineCount");
		addProperty("totalLineCount", "", Mandatory, Attribute, "totalLineCount");
		addProperty("averageTimeWindow", "", Mandatory, Attribute, "averageTimeWindow");
	}
};


struct ArclinkRequestHandler : public IO::XML::TypedClassHandler<ArclinkRequest> {
	ArclinkRequestHandler() {
		addMember("publicID", "", Mandatory, Attribute, new PublicIDSetter<ArclinkRequest>());
		addProperty("requestID", "", Mandatory, Attribute, "requestID");
		addProperty("userID", "", Mandatory, Attribute, "userID");
		addProperty("userIP", "", Optional, Attribute, "userIP");
		addProperty("clientID", "", Optional, Attribute, "clientID");
		addProperty("clientIP", "", Optional, Attribute, "clientIP");
		addProperty("type", "", Mandatory, Attribute, "type");
		addProperty("created", "", Mandatory, Attribute, "created");
		addProperty("status", "", Mandatory, Attribute, "status");
		addProperty("message", "", Optional, Attribute, "message");
		addProperty("label", "", Optional, Attribute, "label");
		addProperty("header", "", Optional, Attribute, "header");
		addProperty("summary", "", Optional, Attribute, "summary");
		addChildProperty("statusLine", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "statusLine");
		addChildProperty("requestLine", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "requestLine");
	}
};


struct ArclinkLogHandler : public IO::XML::TypedClassHandler<ArclinkLog> {
	ArclinkLogHandler() {
		addChildProperty("arclinkRequest", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "arclinkRequest");
		addChildProperty("arclinkUser", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "arclinkUser");
	}
};



TypeMap::TypeMap() {
	static BlobHandler _BlobHandler;
	static RealArrayHandler _RealArrayHandler;
	static ComplexArrayHandler _ComplexArrayHandler;
	static StationReferenceHandler _StationReferenceHandler;
	static StationGroupHandler _StationGroupHandler;
	static AuxSourceHandler _AuxSourceHandler;
	static AuxDeviceHandler _AuxDeviceHandler;
	static SensorCalibrationHandler _SensorCalibrationHandler;
	static SensorHandler _SensorHandler;
	static ResponsePAZHandler _ResponsePAZHandler;
	static ResponsePolynomialHandler _ResponsePolynomialHandler;
	static DataloggerCalibrationHandler _DataloggerCalibrationHandler;
	static DecimationHandler _DecimationHandler;
	static DataloggerHandler _DataloggerHandler;
	static ResponseFIRHandler _ResponseFIRHandler;
	static AuxStreamHandler _AuxStreamHandler;
	static StreamHandler _StreamHandler;
	static SensorLocationHandler _SensorLocationHandler;
	static StationHandler _StationHandler;
	static NetworkHandler _NetworkHandler;
	static InventoryHandler _InventoryHandler;
	static RouteArclinkHandler _RouteArclinkHandler;
	static RouteSeedlinkHandler _RouteSeedlinkHandler;
	static RouteHandler _RouteHandler;
	static AccessHandler _AccessHandler;
	static RoutingHandler _RoutingHandler;
	static ArclinkUserHandler _ArclinkUserHandler;
	static ArclinkStatusLineHandler _ArclinkStatusLineHandler;
	static ArclinkRequestLineHandler _ArclinkRequestLineHandler;
	static ArclinkRequestSummaryHandler _ArclinkRequestSummaryHandler;
	static ArclinkRequestHandler _ArclinkRequestHandler;
	static ArclinkLogHandler _ArclinkLogHandler;

	registerMapping<Blob>("blob", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_BlobHandler);
	registerMapping<RealArray>("realArray", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_RealArrayHandler);
	registerMapping<ComplexArray>("complexArray", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_ComplexArrayHandler);
	registerMapping<StationReference>("stationReference", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_StationReferenceHandler);
	registerMapping("stationGroup", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "StationGroup", &_StationGroupHandler);
	registerMapping<AuxSource>("auxSource", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_AuxSourceHandler);
	registerMapping("auxDevice", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "AuxDevice", &_AuxDeviceHandler);
	registerMapping<SensorCalibration>("SensorCalibration", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_SensorCalibrationHandler);
	registerMapping("sensor", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "Sensor", &_SensorHandler);
	registerMapping("responsePAZ", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "ResponsePAZ", &_ResponsePAZHandler);
	registerMapping("responsePolynomial", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "ResponsePolynomial", &_ResponsePolynomialHandler);
	registerMapping<DataloggerCalibration>("dataloggerCalibration", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_DataloggerCalibrationHandler);
	registerMapping<Decimation>("decimation", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_DecimationHandler);
	registerMapping("datalogger", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "Datalogger", &_DataloggerHandler);
	registerMapping("responseFIR", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "ResponseFIR", &_ResponseFIRHandler);
	registerMapping<AuxStream>("auxStream", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_AuxStreamHandler);
	registerMapping<Stream>("stream", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_StreamHandler);
	registerMapping("sensorLocation", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "SensorLocation", &_SensorLocationHandler);
	registerMapping("station", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "Station", &_StationHandler);
	registerMapping("network", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "Network", &_NetworkHandler);
	registerMapping<Inventory>("inventory", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_InventoryHandler);
	registerMapping<RouteArclink>("routeArclink", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_RouteArclinkHandler);
	registerMapping<RouteSeedlink>("routeSeedlink", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_RouteSeedlinkHandler);
	registerMapping("route", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "Route", &_RouteHandler);
	registerMapping<Access>("access", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_AccessHandler);
	registerMapping<Routing>("routing", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_RoutingHandler);
	registerMapping("arclinkUser", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "ArclinkUser", &_ArclinkUserHandler);
	registerMapping<ArclinkStatusLine>("arclinkStatusLine", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_ArclinkStatusLineHandler);
	registerMapping<ArclinkRequestLine>("arclinkRequestLine", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_ArclinkRequestLineHandler);
	registerMapping<ArclinkRequestSummary>("arclinkRequestSummary", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_ArclinkRequestSummaryHandler);
	registerMapping("arclinkRequest", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", "ArclinkRequest", &_ArclinkRequestHandler);
	registerMapping<ArclinkLog>("arclinkLog", "http://geofon.gfz-potsdam.de/ns/Inventory/1.0/", &_ArclinkLogHandler);
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ImporterArclink::ImporterArclink() {
	setRootName("");
	setTypeMap(&__typeMap);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ExporterArclink::ExporterArclink() {
	setRootName("");
	setTypeMap(&__typeMap);
	_defaultNsMap["http://geofon.gfz-potsdam.de/ns/Inventory/1.0/"] = "ns0";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
