/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#include <fdsnxml/xml.h>
#include <fdsnxml/inttype.h>
#include <fdsnxml/floatnounittype.h>
#include <fdsnxml/floattype.h>
#include <fdsnxml/latitudetype.h>
#include <fdsnxml/longitudetype.h>
#include <fdsnxml/distancetype.h>
#include <fdsnxml/angletype.h>
#include <fdsnxml/diptype.h>
#include <fdsnxml/azimuthtype.h>
#include <fdsnxml/clockdrifttype.h>
#include <fdsnxml/unitstype.h>
#include <fdsnxml/sampleratetype.h>
#include <fdsnxml/samplerateratiotype.h>
#include <fdsnxml/countertype.h>
#include <fdsnxml/frequencytype.h>
#include <fdsnxml/site.h>
#include <fdsnxml/stringtype.h>
#include <fdsnxml/name.h>
#include <fdsnxml/agency.h>
#include <fdsnxml/email.h>
#include <fdsnxml/phone.h>
#include <fdsnxml/person.h>
#include <fdsnxml/operator.h>
#include <fdsnxml/externalreference.h>
#include <fdsnxml/datetype.h>
#include <fdsnxml/equipment.h>
#include <fdsnxml/output.h>
#include <fdsnxml/gain.h>
#include <fdsnxml/sensitivity.h>
#include <fdsnxml/decimation.h>
#include <fdsnxml/basefilter.h>
#include <fdsnxml/poleandzero.h>
#include <fdsnxml/polesandzeros.h>
#include <fdsnxml/coefficients.h>
#include <fdsnxml/responselistelement.h>
#include <fdsnxml/responselist.h>
#include <fdsnxml/numeratorcoefficient.h>
#include <fdsnxml/fir.h>
#include <fdsnxml/polynomialcoefficient.h>
#include <fdsnxml/polynomial.h>
#include <fdsnxml/responsestage.h>
#include <fdsnxml/response.h>
#include <fdsnxml/comment.h>
#include <fdsnxml/basenode.h>
#include <fdsnxml/channel.h>
#include <fdsnxml/station.h>
#include <fdsnxml/network.h>
#include <fdsnxml/fdsnstationxml.h>


namespace Seiscomp {
namespace FDSNXML {


//REGISTER_IMPORTER_INTERFACE(Importer, "fdsnxml");
//REGISTER_EXPORTER_INTERFACE(Exporter, "fdsnxml");


namespace {


struct IntTypeHandler : public IO::XML::TypedClassHandler<IntType> {
	IntTypeHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
	}
};


struct FloatNoUnitTypeHandler : public IO::XML::TypedClassHandler<FloatNoUnitType> {
	FloatNoUnitTypeHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
		addProperty("plusError", "", Optional, Attribute, "upperUncertainty");
		addProperty("minusError", "", Optional, Attribute, "lowerUncertainty");
	}
};


struct FloatTypeHandler : public IO::XML::TypedClassHandler<FloatType> {
	FloatTypeHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
		addProperty("plusError", "", Optional, Attribute, "upperUncertainty");
		addProperty("minusError", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct LatitudeTypeHandler : public IO::XML::TypedClassHandler<LatitudeType> {
	LatitudeTypeHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
		addProperty("plusError", "", Optional, Attribute, "upperUncertainty");
		addProperty("minusError", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
		addProperty("datum", "", Optional, Attribute, "datum");
	}
};


struct LongitudeTypeHandler : public IO::XML::TypedClassHandler<LongitudeType> {
	LongitudeTypeHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
		addProperty("plusError", "", Optional, Attribute, "upperUncertainty");
		addProperty("minusError", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
		addProperty("datum", "", Optional, Attribute, "datum");
	}
};


struct DistanceTypeHandler : public IO::XML::TypedClassHandler<DistanceType> {
	DistanceTypeHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
		addProperty("plusError", "", Optional, Attribute, "upperUncertainty");
		addProperty("minusError", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct AngleTypeHandler : public IO::XML::TypedClassHandler<AngleType> {
	AngleTypeHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
		addProperty("plusError", "", Optional, Attribute, "upperUncertainty");
		addProperty("minusError", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct DipTypeHandler : public IO::XML::TypedClassHandler<DipType> {
	DipTypeHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
		addProperty("plusError", "", Optional, Attribute, "upperUncertainty");
		addProperty("minusError", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct AzimuthTypeHandler : public IO::XML::TypedClassHandler<AzimuthType> {
	AzimuthTypeHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
		addProperty("plusError", "", Optional, Attribute, "upperUncertainty");
		addProperty("minusError", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct ClockDriftTypeHandler : public IO::XML::TypedClassHandler<ClockDriftType> {
	ClockDriftTypeHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
		addProperty("plusError", "", Optional, Attribute, "upperUncertainty");
		addProperty("minusError", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct UnitsTypeHandler : public IO::XML::TypedClassHandler<UnitsType> {
	UnitsTypeHandler() {
		addProperty("Name", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "Name");
		addProperty("Description", "http://www.fdsn.org/xml/station/1", Optional, Element, "Description");
	}
};


struct SampleRateTypeHandler : public IO::XML::TypedClassHandler<SampleRateType> {
	SampleRateTypeHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
		addProperty("plusError", "", Optional, Attribute, "upperUncertainty");
		addProperty("minusError", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct SampleRateRatioTypeHandler : public IO::XML::TypedClassHandler<SampleRateRatioType> {
	SampleRateRatioTypeHandler() {
		addProperty("NumberSamples", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "numberSamples");
		addProperty("NumberSeconds", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "numberSeconds");
	}
};


struct CounterTypeHandler : public IO::XML::TypedClassHandler<CounterType> {
	CounterTypeHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
	}
};


struct FrequencyTypeHandler : public IO::XML::TypedClassHandler<FrequencyType> {
	FrequencyTypeHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
		addProperty("plusError", "", Optional, Attribute, "upperUncertainty");
		addProperty("minusError", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct SiteHandler : public IO::XML::TypedClassHandler<Site> {
	SiteHandler() {
		addProperty("Name", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "name");
		addProperty("Description", "http://www.fdsn.org/xml/station/1", Optional, Element, "description");
		addProperty("Town", "http://www.fdsn.org/xml/station/1", Optional, Element, "town");
		addProperty("County", "http://www.fdsn.org/xml/station/1", Optional, Element, "county");
		addProperty("Region", "http://www.fdsn.org/xml/station/1", Optional, Element, "region");
		addProperty("Country", "http://www.fdsn.org/xml/station/1", Optional, Element, "country");
	}
};


struct StringTypeHandler : public IO::XML::TypedClassHandler<StringType> {
	StringTypeHandler() {
		addProperty("text", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "text");
	}
};


struct NameHandler : public IO::XML::TypedClassHandler<Name> {
	NameHandler() {
		addProperty("text", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "text");
	}
};


struct AgencyHandler : public IO::XML::TypedClassHandler<Agency> {
	AgencyHandler() {
		addProperty("text", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "text");
	}
};


struct EmailHandler : public IO::XML::TypedClassHandler<Email> {
	EmailHandler() {
		addProperty("text", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "text");
	}
};


struct PhoneHandler : public IO::XML::TypedClassHandler<Phone> {
	PhoneHandler() {
		addProperty("CountryCode", "http://www.fdsn.org/xml/station/1", Optional, Element, "countryCode");
		addProperty("AreaCode", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "areaCode");
		addProperty("PhoneNumber", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "phoneNumber");
		addProperty("description", "", Optional, Attribute, "description");
	}
};


struct PersonHandler : public IO::XML::TypedClassHandler<Person> {
	PersonHandler() {
		addChildProperty("Name", "http://www.fdsn.org/xml/station/1", "name");
		addChildProperty("Agency", "http://www.fdsn.org/xml/station/1", "agency");
		addChildProperty("Email", "http://www.fdsn.org/xml/station/1", "email");
		addChildProperty("Phone", "http://www.fdsn.org/xml/station/1", "phone");
	}
};


struct OperatorHandler : public IO::XML::TypedClassHandler<Operator> {
	OperatorHandler() {
		addChildProperty("Agency", "http://www.fdsn.org/xml/station/1", "agency");
		addChildProperty("Contact", "http://www.fdsn.org/xml/station/1", "contact");
		addChildProperty("WebSite", "http://www.fdsn.org/xml/station/1", "webSite");
	}
};


struct ExternalReferenceHandler : public IO::XML::TypedClassHandler<ExternalReference> {
	ExternalReferenceHandler() {
		addProperty("URI", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "URI");
		addProperty("Description", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "description");
	}
};


struct DateTypeHandler : public IO::XML::TypedClassHandler<DateType> {
	DateTypeHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
	}
};


struct EquipmentHandler : public IO::XML::TypedClassHandler<Equipment> {
	EquipmentHandler() {
		addProperty("Type", "http://www.fdsn.org/xml/station/1", Optional, Element, "Type");
		addProperty("Description", "http://www.fdsn.org/xml/station/1", Optional, Element, "Description");
		addProperty("Manufacturer", "http://www.fdsn.org/xml/station/1", Optional, Element, "Manufacturer");
		addProperty("Vendor", "http://www.fdsn.org/xml/station/1", Optional, Element, "Vendor");
		addProperty("Model", "http://www.fdsn.org/xml/station/1", Optional, Element, "Model");
		addProperty("SerialNumber", "http://www.fdsn.org/xml/station/1", Optional, Element, "SerialNumber");
		addProperty("InstallationDate", "http://www.fdsn.org/xml/station/1", Optional, Element, "InstallationDate");
		addProperty("RemovalDate", "http://www.fdsn.org/xml/station/1", Optional, Element, "RemovalDate");
		addChildProperty("CalibrationDate", "http://www.fdsn.org/xml/station/1", "CalibrationDate");
		addProperty("resourceId", "", Optional, Attribute, "resourceId");
	}
};


struct OutputHandler : public IO::XML::TypedClassHandler<Output> {
	OutputHandler() {
		addProperty("type", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "type");
	}
};


struct GainHandler : public IO::XML::TypedClassHandler<Gain> {
	GainHandler() {
		addProperty("Value", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "Value");
		addProperty("Frequency", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "Frequency");
	}
};


struct SensitivityHandler : public IO::XML::TypedClassHandler<Sensitivity> {
	SensitivityHandler() {
		addProperty("Value", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "Value");
		addProperty("Frequency", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "Frequency");
		// Element
		addProperty("InputUnits", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "InputUnits");
		// Element
		addProperty("OutputUnits", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "OutputUnits");
		addProperty("FrequencyStart", "http://www.fdsn.org/xml/station/1", Optional, Element, "FrequencyStart");
		addProperty("FrequencyEnd", "http://www.fdsn.org/xml/station/1", Optional, Element, "FrequencyEnd");
		addProperty("FrequencyDBVariation", "http://www.fdsn.org/xml/station/1", Optional, Element, "FrequencyDBVariation");
	}
};


struct DecimationHandler : public IO::XML::TypedClassHandler<Decimation> {
	DecimationHandler() {
		// Element
		addProperty("InputSampleRate", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "InputSampleRate");
		addProperty("Factor", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "Factor");
		addProperty("Offset", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "Offset");
		// Element
		addProperty("Delay", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "Delay");
		// Element
		addProperty("Correction", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "Correction");
	}
};


struct BaseFilterHandler : public IO::XML::TypedClassHandler<BaseFilter> {
	BaseFilterHandler() {
		addProperty("Description", "http://www.fdsn.org/xml/station/1", Optional, Element, "Description");
		// Element
		addProperty("InputUnits", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "InputUnits");
		// Element
		addProperty("OutputUnits", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "OutputUnits");
		addProperty("resourceId", "", Optional, Attribute, "resourceId");
		addProperty("name", "", Optional, Attribute, "name");
	}
};


struct PoleAndZeroHandler : public IO::XML::TypedClassHandler<PoleAndZero> {
	PoleAndZeroHandler() {
		// Element
		addProperty("Real", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "Real");
		// Element
		addProperty("Imaginary", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "Imaginary");
		addProperty("number", "", Mandatory, Attribute, "number");
	}
};


struct PolesAndZerosHandler : public IO::XML::TypedClassHandler<PolesAndZeros> {
	PolesAndZerosHandler() {
		addProperty("Description", "http://www.fdsn.org/xml/station/1", Optional, Element, "Description");
		// Element
		addProperty("InputUnits", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "InputUnits");
		// Element
		addProperty("OutputUnits", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "OutputUnits");
		addProperty("resourceId", "", Optional, Attribute, "resourceId");
		addProperty("name", "", Optional, Attribute, "name");
		addProperty("PzTransferFunctionType", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "PzTransferFunctionType");
		addProperty("NormalizationFactor", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "NormalizationFactor");
		// Element
		addProperty("NormalizationFrequency", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "NormalizationFrequency");
		addChildProperty("Zero", "http://www.fdsn.org/xml/station/1", "Zero");
		addChildProperty("Pole", "http://www.fdsn.org/xml/station/1", "Pole");
	}
};


struct CoefficientsHandler : public IO::XML::TypedClassHandler<Coefficients> {
	CoefficientsHandler() {
		addProperty("Description", "http://www.fdsn.org/xml/station/1", Optional, Element, "Description");
		// Element
		addProperty("InputUnits", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "InputUnits");
		// Element
		addProperty("OutputUnits", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "OutputUnits");
		addProperty("resourceId", "", Optional, Attribute, "resourceId");
		addProperty("name", "", Optional, Attribute, "name");
		addProperty("CfTransferFunctionType", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "CfTransferFunctionType");
		addChildProperty("Numerator", "http://www.fdsn.org/xml/station/1", "Numerator");
		addChildProperty("Denominator", "http://www.fdsn.org/xml/station/1", "Denominator");
	}
};


struct ResponseListElementHandler : public IO::XML::TypedClassHandler<ResponseListElement> {
	ResponseListElementHandler() {
		// Element
		addProperty("Frequency", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "Frequency");
		// Element
		addProperty("Amplitude", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "Amplitude");
		// Element
		addProperty("Phase", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "Phase");
	}
};


struct ResponseListHandler : public IO::XML::TypedClassHandler<ResponseList> {
	ResponseListHandler() {
		addProperty("Description", "http://www.fdsn.org/xml/station/1", Optional, Element, "Description");
		// Element
		addProperty("InputUnits", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "InputUnits");
		// Element
		addProperty("OutputUnits", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "OutputUnits");
		addProperty("resourceId", "", Optional, Attribute, "resourceId");
		addProperty("name", "", Optional, Attribute, "name");
		addChildProperty("ResponseListElement", "http://www.fdsn.org/xml/station/1", "element");
	}
};


struct NumeratorCoefficientHandler : public IO::XML::TypedClassHandler<NumeratorCoefficient> {
	NumeratorCoefficientHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
		addProperty("i", "", Mandatory, Attribute, "i");
	}
};


struct FIRHandler : public IO::XML::TypedClassHandler<FIR> {
	FIRHandler() {
		addProperty("Description", "http://www.fdsn.org/xml/station/1", Optional, Element, "Description");
		// Element
		addProperty("InputUnits", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "InputUnits");
		// Element
		addProperty("OutputUnits", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "OutputUnits");
		addProperty("resourceId", "", Optional, Attribute, "resourceId");
		addProperty("name", "", Optional, Attribute, "name");
		addProperty("Symmetry", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "Symmetry");
		addChildProperty("NumeratorCoefficient", "http://www.fdsn.org/xml/station/1", "NumeratorCoefficient");
	}
};


struct PolynomialCoefficientHandler : public IO::XML::TypedClassHandler<PolynomialCoefficient> {
	PolynomialCoefficientHandler() {
		addProperty("value", "http://www.fdsn.org/xml/station/1", Mandatory, CDATA, "value");
		addProperty("plusError", "", Optional, Attribute, "upperUncertainty");
		addProperty("minusError", "", Optional, Attribute, "lowerUncertainty");
		addProperty("number", "", Mandatory, Attribute, "number");
	}
};


struct PolynomialHandler : public IO::XML::TypedClassHandler<Polynomial> {
	PolynomialHandler() {
		addProperty("Description", "http://www.fdsn.org/xml/station/1", Optional, Element, "Description");
		// Element
		addProperty("InputUnits", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "InputUnits");
		// Element
		addProperty("OutputUnits", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "OutputUnits");
		addProperty("resourceId", "", Optional, Attribute, "resourceId");
		addProperty("name", "", Optional, Attribute, "name");
		addProperty("ApproximationType", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "ApproximationType");
		// Element
		addProperty("FrequencyLowerBound", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "FrequencyLowerBound");
		// Element
		addProperty("FrequencyUpperBound", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "FrequencyUpperBound");
		addProperty("ApproximationLowerBound", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "ApproximationLowerBound");
		addProperty("ApproximationUpperBound", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "ApproximationUpperBound");
		addProperty("MaximumError", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "MaximumError");
		addChildProperty("Coefficient", "http://www.fdsn.org/xml/station/1", "Coefficient");
	}
};


struct ResponseStageHandler : public IO::XML::TypedClassHandler<ResponseStage> {
	ResponseStageHandler() {
		// Element
		addProperty("PolesZeros", "http://www.fdsn.org/xml/station/1", Optional, Element, "PolesZeros");
		// Element
		addProperty("Coefficients", "http://www.fdsn.org/xml/station/1", Optional, Element, "Coefficients");
		// Element
		addProperty("ResponseList", "http://www.fdsn.org/xml/station/1", Optional, Element, "ResponseList");
		// Element
		addProperty("FIR", "http://www.fdsn.org/xml/station/1", Optional, Element, "FIR");
		// Element
		addProperty("Polynomial", "http://www.fdsn.org/xml/station/1", Optional, Element, "Polynomial");
		// Element
		addProperty("Decimation", "http://www.fdsn.org/xml/station/1", Optional, Element, "Decimation");
		// Element
		addProperty("StageGain", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "StageGain");
		addProperty("number", "", Mandatory, Attribute, "number");
		addProperty("resourceId", "", Optional, Attribute, "resourceId");
	}
};


struct ResponseHandler : public IO::XML::TypedClassHandler<Response> {
	ResponseHandler() {
		// Element
		addProperty("InstrumentSensitivity", "http://www.fdsn.org/xml/station/1", Optional, Element, "InstrumentSensitivity");
		// Element
		addProperty("InstrumentPolynomial", "http://www.fdsn.org/xml/station/1", Optional, Element, "InstrumentPolynomial");
		addChildProperty("Stage", "http://www.fdsn.org/xml/station/1", "Stage");
	}
};


struct CommentHandler : public IO::XML::TypedClassHandler<Comment> {
	CommentHandler() {
		addProperty("Value", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "value");
		addProperty("BeginEffectiveTime", "http://www.fdsn.org/xml/station/1", Optional, Element, "beginEffectiveTime");
		addProperty("EndEffectiveTime", "http://www.fdsn.org/xml/station/1", Optional, Element, "endEffectiveTime");
		addProperty("id", "", Optional, Attribute, "id");
		addChildProperty("Author", "http://www.fdsn.org/xml/station/1", "author");
	}
};


struct BaseNodeHandler : public IO::XML::TypedClassHandler<BaseNode> {
	BaseNodeHandler() {
		addProperty("Description", "http://www.fdsn.org/xml/station/1", Optional, Element, "description");
		addProperty("code", "", Mandatory, Attribute, "code");
		addProperty("startDate", "", Optional, Attribute, "startDate");
		addProperty("endDate", "", Optional, Attribute, "endDate");
		addProperty("restrictedStatus", "", Optional, Attribute, "restrictedStatus");
		addProperty("alternateCode", "", Optional, Attribute, "alternateCode");
		addProperty("historicCode", "", Optional, Attribute, "historicCode");
		addChildProperty("Comment", "http://www.fdsn.org/xml/station/1", "comment");
	}
};


struct ChannelHandler : public IO::XML::TypedClassHandler<Channel> {
	ChannelHandler() {
		addProperty("Description", "http://www.fdsn.org/xml/station/1", Optional, Element, "description");
		addProperty("code", "", Mandatory, Attribute, "code");
		addProperty("startDate", "", Optional, Attribute, "startDate");
		addProperty("endDate", "", Optional, Attribute, "endDate");
		addProperty("restrictedStatus", "", Optional, Attribute, "restrictedStatus");
		addProperty("alternateCode", "", Optional, Attribute, "alternateCode");
		addProperty("historicCode", "", Optional, Attribute, "historicCode");
		addChildProperty("Comment", "http://www.fdsn.org/xml/station/1", "comment");
		// Element
		addProperty("Latitude", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "latitude");
		// Element
		addProperty("Longitude", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "longitude");
		// Element
		addProperty("Elevation", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "elevation");
		// Element
		addProperty("Depth", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "depth");
		// Element
		addProperty("Azimuth", "http://www.fdsn.org/xml/station/1", Optional, Element, "azimuth");
		// Element
		addProperty("Dip", "http://www.fdsn.org/xml/station/1", Optional, Element, "dip");
		addChildProperty("Type", "http://www.fdsn.org/xml/station/1", "type");
		// Element
		addProperty("SampleRate", "http://www.fdsn.org/xml/station/1", Optional, Element, "SampleRate");
		// Element
		addProperty("SampleRateRatio", "http://www.fdsn.org/xml/station/1", Optional, Element, "SampleRateRatio");
		addProperty("StorageFormat", "http://www.fdsn.org/xml/station/1", Optional, Element, "StorageFormat");
		// Element
		addProperty("ClockDrift", "http://www.fdsn.org/xml/station/1", Optional, Element, "ClockDrift");
		// Element
		addProperty("CalibrationUnits", "http://www.fdsn.org/xml/station/1", Optional, Element, "CalibrationUnits");
		// Element
		addProperty("Sensor", "http://www.fdsn.org/xml/station/1", Optional, Element, "Sensor");
		// Element
		addProperty("PreAmplifier", "http://www.fdsn.org/xml/station/1", Optional, Element, "PreAmplifier");
		// Element
		addProperty("DataLogger", "http://www.fdsn.org/xml/station/1", Optional, Element, "DataLogger");
		// Element
		addProperty("Equipment", "http://www.fdsn.org/xml/station/1", Optional, Element, "Equipment");
		// Element
		addProperty("Response", "http://www.fdsn.org/xml/station/1", Optional, Element, "Response");
		addProperty("locationCode", "", Mandatory, Attribute, "locationCode");
	}
};


struct StationHandler : public IO::XML::TypedClassHandler<Station> {
	StationHandler() {
		addProperty("Description", "http://www.fdsn.org/xml/station/1", Optional, Element, "description");
		addProperty("code", "", Mandatory, Attribute, "code");
		addProperty("startDate", "", Optional, Attribute, "startDate");
		addProperty("endDate", "", Optional, Attribute, "endDate");
		addProperty("restrictedStatus", "", Optional, Attribute, "restrictedStatus");
		addProperty("alternateCode", "", Optional, Attribute, "alternateCode");
		addProperty("historicCode", "", Optional, Attribute, "historicCode");
		addChildProperty("Comment", "http://www.fdsn.org/xml/station/1", "comment");
		// Element
		addProperty("Latitude", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "latitude");
		// Element
		addProperty("Longitude", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "longitude");
		// Element
		addProperty("Elevation", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "elevation");
		// Element
		addProperty("Site", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "site");
		addProperty("Vault", "http://www.fdsn.org/xml/station/1", Optional, Element, "vault");
		addProperty("Geology", "http://www.fdsn.org/xml/station/1", Optional, Element, "geology");
		addChildProperty("Equipment", "http://www.fdsn.org/xml/station/1", "equipment");
		addChildProperty("Operator", "http://www.fdsn.org/xml/station/1", "operators");
		addProperty("CreationDate", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "creationDate");
		addProperty("TerminationDate", "http://www.fdsn.org/xml/station/1", Optional, Element, "terminationDate");
		// Element
		addProperty("TotalNumberChannels", "http://www.fdsn.org/xml/station/1", Optional, Element, "totalNumberChannels");
		// Element
		addProperty("SelectedNumberChannels", "http://www.fdsn.org/xml/station/1", Optional, Element, "selectedNumberChannels");
		addChildProperty("ExternalReference", "http://www.fdsn.org/xml/station/1", "externalReference");
		addChildProperty("Channel", "http://www.fdsn.org/xml/station/1", "channel");
	}
};


struct NetworkHandler : public IO::XML::TypedClassHandler<Network> {
	NetworkHandler() {
		addProperty("Description", "http://www.fdsn.org/xml/station/1", Optional, Element, "description");
		addProperty("code", "", Mandatory, Attribute, "code");
		addProperty("startDate", "", Optional, Attribute, "startDate");
		addProperty("endDate", "", Optional, Attribute, "endDate");
		addProperty("restrictedStatus", "", Optional, Attribute, "restrictedStatus");
		addProperty("alternateCode", "", Optional, Attribute, "alternateCode");
		addProperty("historicCode", "", Optional, Attribute, "historicCode");
		addChildProperty("Comment", "http://www.fdsn.org/xml/station/1", "comment");
		// Element
		addProperty("TotalNumberStations", "http://www.fdsn.org/xml/station/1", Optional, Element, "totalNumberOfStations");
		// Element
		addProperty("SelectedNumberStations", "http://www.fdsn.org/xml/station/1", Optional, Element, "selectedNumberStations");
		addChildProperty("Station", "http://www.fdsn.org/xml/station/1", "station");
	}
};


struct FDSNStationXMLHandler : public IO::XML::TypedClassHandler<FDSNStationXML> {
	FDSNStationXMLHandler() {
		addProperty("Source", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "source");
		addProperty("Sender", "http://www.fdsn.org/xml/station/1", Optional, Element, "sender");
		addProperty("Module", "http://www.fdsn.org/xml/station/1", Optional, Element, "module");
		addProperty("ModuleURI", "http://www.fdsn.org/xml/station/1", Optional, Element, "moduleURI");
		addProperty("Created", "http://www.fdsn.org/xml/station/1", Mandatory, Element, "created");
		addProperty("schemaVersion", "", Mandatory, Attribute, "schemaVersion");
		addChildProperty("Network", "http://www.fdsn.org/xml/station/1", "network");
	}
};


struct TypeMap : public Seiscomp::IO::XML::TypeMap {
	TypeMap();
};


TypeMap myTypeMap;


TypeMap::TypeMap() {
	static IntTypeHandler _IntTypeHandler;
	static FloatNoUnitTypeHandler _FloatNoUnitTypeHandler;
	static FloatTypeHandler _FloatTypeHandler;
	static LatitudeTypeHandler _LatitudeTypeHandler;
	static LongitudeTypeHandler _LongitudeTypeHandler;
	static DistanceTypeHandler _DistanceTypeHandler;
	static AngleTypeHandler _AngleTypeHandler;
	static DipTypeHandler _DipTypeHandler;
	static AzimuthTypeHandler _AzimuthTypeHandler;
	static ClockDriftTypeHandler _ClockDriftTypeHandler;
	static UnitsTypeHandler _UnitsTypeHandler;
	static SampleRateTypeHandler _SampleRateTypeHandler;
	static SampleRateRatioTypeHandler _SampleRateRatioTypeHandler;
	static CounterTypeHandler _CounterTypeHandler;
	static FrequencyTypeHandler _FrequencyTypeHandler;
	static SiteHandler _SiteHandler;
	static StringTypeHandler _StringTypeHandler;
	static NameHandler _NameHandler;
	static AgencyHandler _AgencyHandler;
	static EmailHandler _EmailHandler;
	static PhoneHandler _PhoneHandler;
	static PersonHandler _PersonHandler;
	static OperatorHandler _OperatorHandler;
	static ExternalReferenceHandler _ExternalReferenceHandler;
	static DateTypeHandler _DateTypeHandler;
	static EquipmentHandler _EquipmentHandler;
	static OutputHandler _OutputHandler;
	static GainHandler _GainHandler;
	static SensitivityHandler _SensitivityHandler;
	static DecimationHandler _DecimationHandler;
	static BaseFilterHandler _BaseFilterHandler;
	static PoleAndZeroHandler _PoleAndZeroHandler;
	static PolesAndZerosHandler _PolesAndZerosHandler;
	static CoefficientsHandler _CoefficientsHandler;
	static ResponseListElementHandler _ResponseListElementHandler;
	static ResponseListHandler _ResponseListHandler;
	static NumeratorCoefficientHandler _NumeratorCoefficientHandler;
	static FIRHandler _FIRHandler;
	static PolynomialCoefficientHandler _PolynomialCoefficientHandler;
	static PolynomialHandler _PolynomialHandler;
	static ResponseStageHandler _ResponseStageHandler;
	static ResponseHandler _ResponseHandler;
	static CommentHandler _CommentHandler;
	static BaseNodeHandler _BaseNodeHandler;
	static ChannelHandler _ChannelHandler;
	static StationHandler _StationHandler;
	static NetworkHandler _NetworkHandler;
	static FDSNStationXMLHandler _FDSNStationXMLHandler;

	registerMapping<IntType>("IntType", "http://www.fdsn.org/xml/station/1", &_IntTypeHandler);
	registerMapping<FloatNoUnitType>("FloatNoUnitType", "http://www.fdsn.org/xml/station/1", &_FloatNoUnitTypeHandler);
	registerMapping<FloatType>("FloatType", "http://www.fdsn.org/xml/station/1", &_FloatTypeHandler);
	registerMapping<LatitudeType>("LatitudeType", "http://www.fdsn.org/xml/station/1", &_LatitudeTypeHandler);
	registerMapping<LongitudeType>("LongitudeType", "http://www.fdsn.org/xml/station/1", &_LongitudeTypeHandler);
	registerMapping<DistanceType>("DistanceType", "http://www.fdsn.org/xml/station/1", &_DistanceTypeHandler);
	registerMapping<AngleType>("AngleType", "http://www.fdsn.org/xml/station/1", &_AngleTypeHandler);
	registerMapping<DipType>("DipType", "http://www.fdsn.org/xml/station/1", &_DipTypeHandler);
	registerMapping<AzimuthType>("AzimuthType", "http://www.fdsn.org/xml/station/1", &_AzimuthTypeHandler);
	registerMapping<ClockDriftType>("ClockDriftType", "http://www.fdsn.org/xml/station/1", &_ClockDriftTypeHandler);
	registerMapping<UnitsType>("UnitsType", "http://www.fdsn.org/xml/station/1", &_UnitsTypeHandler);
	registerMapping<SampleRateType>("SampleRateType", "http://www.fdsn.org/xml/station/1", &_SampleRateTypeHandler);
	registerMapping<SampleRateRatioType>("SampleRateRatioType", "http://www.fdsn.org/xml/station/1", &_SampleRateRatioTypeHandler);
	registerMapping<CounterType>("CounterType", "http://www.fdsn.org/xml/station/1", &_CounterTypeHandler);
	registerMapping<FrequencyType>("FrequencyType", "http://www.fdsn.org/xml/station/1", &_FrequencyTypeHandler);
	registerMapping<Site>("Site", "http://www.fdsn.org/xml/station/1", &_SiteHandler);
	registerMapping<StringType>("StringType", "http://www.fdsn.org/xml/station/1", &_StringTypeHandler);
	registerMapping<Name>("Name", "http://www.fdsn.org/xml/station/1", &_NameHandler);
	registerMapping<Agency>("Agency", "http://www.fdsn.org/xml/station/1", &_AgencyHandler);
	registerMapping<Email>("Email", "http://www.fdsn.org/xml/station/1", &_EmailHandler);
	registerMapping<Phone>("Phone", "http://www.fdsn.org/xml/station/1", &_PhoneHandler);
	registerMapping<Person>("Person", "http://www.fdsn.org/xml/station/1", &_PersonHandler);
	registerMapping<Operator>("Operator", "http://www.fdsn.org/xml/station/1", &_OperatorHandler);
	registerMapping<ExternalReference>("ExternalReference", "http://www.fdsn.org/xml/station/1", &_ExternalReferenceHandler);
	registerMapping<DateType>("DateType", "http://www.fdsn.org/xml/station/1", &_DateTypeHandler);
	registerMapping<Equipment>("Equipment", "http://www.fdsn.org/xml/station/1", &_EquipmentHandler);
	registerMapping<Output>("Output", "http://www.fdsn.org/xml/station/1", &_OutputHandler);
	registerMapping<Gain>("Gain", "http://www.fdsn.org/xml/station/1", &_GainHandler);
	registerMapping<Sensitivity>("Sensitivity", "http://www.fdsn.org/xml/station/1", &_SensitivityHandler);
	registerMapping<Decimation>("Decimation", "http://www.fdsn.org/xml/station/1", &_DecimationHandler);
	registerMapping<BaseFilter>("BaseFilter", "http://www.fdsn.org/xml/station/1", &_BaseFilterHandler);
	registerMapping<PoleAndZero>("PoleAndZero", "http://www.fdsn.org/xml/station/1", &_PoleAndZeroHandler);
	registerMapping<PolesAndZeros>("PolesAndZeros", "http://www.fdsn.org/xml/station/1", &_PolesAndZerosHandler);
	registerMapping<Coefficients>("Coefficients", "http://www.fdsn.org/xml/station/1", &_CoefficientsHandler);
	registerMapping<ResponseListElement>("ResponseListElement", "http://www.fdsn.org/xml/station/1", &_ResponseListElementHandler);
	registerMapping<ResponseList>("ResponseList", "http://www.fdsn.org/xml/station/1", &_ResponseListHandler);
	registerMapping<NumeratorCoefficient>("NumeratorCoefficient", "http://www.fdsn.org/xml/station/1", &_NumeratorCoefficientHandler);
	registerMapping<FIR>("FIR", "http://www.fdsn.org/xml/station/1", &_FIRHandler);
	registerMapping<PolynomialCoefficient>("PolynomialCoefficient", "http://www.fdsn.org/xml/station/1", &_PolynomialCoefficientHandler);
	registerMapping<Polynomial>("Polynomial", "http://www.fdsn.org/xml/station/1", &_PolynomialHandler);
	registerMapping<ResponseStage>("ResponseStage", "http://www.fdsn.org/xml/station/1", &_ResponseStageHandler);
	registerMapping<Response>("Response", "http://www.fdsn.org/xml/station/1", &_ResponseHandler);
	registerMapping<Comment>("Comment", "http://www.fdsn.org/xml/station/1", &_CommentHandler);
	registerMapping<BaseNode>("BaseNode", "http://www.fdsn.org/xml/station/1", &_BaseNodeHandler);
	registerMapping<Channel>("Channel", "http://www.fdsn.org/xml/station/1", &_ChannelHandler);
	registerMapping<Station>("Station", "http://www.fdsn.org/xml/station/1", &_StationHandler);
	registerMapping<Network>("Network", "http://www.fdsn.org/xml/station/1", &_NetworkHandler);
	registerMapping<FDSNStationXML>("FDSNStationXML", "http://www.fdsn.org/xml/station/1", &_FDSNStationXMLHandler);
}



}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Importer::Importer() {
	setRootName("");
	setTypeMap(&myTypeMap);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Exporter::Exporter() {
	setRootName("");
	setTypeMap(&myTypeMap);
	_defaultNsMap["http://www.fdsn.org/xml/station/1"] = "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
