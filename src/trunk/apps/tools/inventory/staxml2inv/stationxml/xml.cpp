/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#include <stationxml/xml.h>
#include <stationxml/inttype.h>
#include <stationxml/floatnounittype.h>
#include <stationxml/floattype.h>
#include <stationxml/lattype.h>
#include <stationxml/lontype.h>
#include <stationxml/distancetype.h>
#include <stationxml/angletype.h>
#include <stationxml/diptype.h>
#include <stationxml/azimuthtype.h>
#include <stationxml/clockdrifttype.h>
#include <stationxml/sampleratetype.h>
#include <stationxml/samplerateratiotype.h>
#include <stationxml/samplerategroup.h>
#include <stationxml/countertype.h>
#include <stationxml/frequencytype.h>
#include <stationxml/rolltype.h>
#include <stationxml/passtype.h>
#include <stationxml/epoch.h>
#include <stationxml/site.h>
#include <stationxml/stringtype.h>
#include <stationxml/agencytype.h>
#include <stationxml/emailtype.h>
#include <stationxml/phonetype.h>
#include <stationxml/contact.h>
#include <stationxml/operator.h>
#include <stationxml/externaldocument.h>
#include <stationxml/datetype.h>
#include <stationxml/equipment.h>
#include <stationxml/offset.h>
#include <stationxml/output.h>
#include <stationxml/datalogger.h>
#include <stationxml/sensitivity.h>
#include <stationxml/decimation.h>
#include <stationxml/sa.h>
#include <stationxml/spectra.h>
#include <stationxml/comment.h>
#include <stationxml/poleandzero.h>
#include <stationxml/polesandzeros.h>
#include <stationxml/coefficients.h>
#include <stationxml/responselistelement.h>
#include <stationxml/responselist.h>
#include <stationxml/genericresponse.h>
#include <stationxml/numeratorcoefficient.h>
#include <stationxml/fir.h>
#include <stationxml/polynomialcoefficient.h>
#include <stationxml/polynomial.h>
#include <stationxml/response.h>
#include <stationxml/gaintype.h>
#include <stationxml/leastsignificantbittype.h>
#include <stationxml/voltagetype.h>
#include <stationxml/secondtype.h>
#include <stationxml/channelepoch.h>
#include <stationxml/channel.h>
#include <stationxml/stationepoch.h>
#include <stationxml/station.h>
#include <stationxml/network.h>
#include <stationxml/stamessage.h>


namespace Seiscomp {
namespace StationXML {


//REGISTER_IMPORTER_INTERFACE(Importer, "stationxml");
//REGISTER_EXPORTER_INTERFACE(Exporter, "stationxml");


namespace {


struct IntTypeHandler : public IO::XML::TypedClassHandler<IntType> {
	IntTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
	}
};


struct FloatNoUnitTypeHandler : public IO::XML::TypedClassHandler<FloatNoUnitType> {
	FloatNoUnitTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
	}
};


struct FloatTypeHandler : public IO::XML::TypedClassHandler<FloatType> {
	FloatTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct LatTypeHandler : public IO::XML::TypedClassHandler<LatType> {
	LatTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
		addProperty("datum", "", Optional, Attribute, "datum");
	}
};


struct LonTypeHandler : public IO::XML::TypedClassHandler<LonType> {
	LonTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
		addProperty("datum", "", Optional, Attribute, "datum");
	}
};


struct DistanceTypeHandler : public IO::XML::TypedClassHandler<DistanceType> {
	DistanceTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
		addProperty("datum", "", Optional, Attribute, "datum");
	}
};


struct AngleTypeHandler : public IO::XML::TypedClassHandler<AngleType> {
	AngleTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct DipTypeHandler : public IO::XML::TypedClassHandler<DipType> {
	DipTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct AzimuthTypeHandler : public IO::XML::TypedClassHandler<AzimuthType> {
	AzimuthTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct ClockDriftTypeHandler : public IO::XML::TypedClassHandler<ClockDriftType> {
	ClockDriftTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct SampleRateTypeHandler : public IO::XML::TypedClassHandler<SampleRateType> {
	SampleRateTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct SampleRateRatioTypeHandler : public IO::XML::TypedClassHandler<SampleRateRatioType> {
	SampleRateRatioTypeHandler() {
		addProperty("NumberSamples", "http://www.data.scec.org/xml/station/", Mandatory, Element, "NumberSamples");
		addProperty("NumberSeconds", "http://www.data.scec.org/xml/station/", Mandatory, Element, "NumberSeconds");
	}
};


struct SampleRateGroupHandler : public IO::XML::TypedClassHandler<SampleRateGroup> {
	SampleRateGroupHandler() {
		addChildProperty("SampleRate", "http://www.data.scec.org/xml/station/", "SampleRate");
		addChildProperty("SampleRateRatio", "http://www.data.scec.org/xml/station/", "SampleRateRatio");
	}
};


struct CounterTypeHandler : public IO::XML::TypedClassHandler<CounterType> {
	CounterTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
	}
};


struct FrequencyTypeHandler : public IO::XML::TypedClassHandler<FrequencyType> {
	FrequencyTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct RollTypeHandler : public IO::XML::TypedClassHandler<RollType> {
	RollTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct PassTypeHandler : public IO::XML::TypedClassHandler<PassType> {
	PassTypeHandler() {
		// Element
		addProperty("CornerFreq", "http://www.data.scec.org/xml/station/", Mandatory, Element, "CornerFreq");
		// Element
		addProperty("Roll", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Roll");
		addProperty("Damping", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Damping");
		// Element
		addProperty("Polenumber", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Polenumber");
	}
};


struct EpochHandler : public IO::XML::TypedClassHandler<Epoch> {
	EpochHandler() {
		addProperty("StartDate", "http://www.data.scec.org/xml/station/", Mandatory, Element, "start");
		addProperty("EndDate", "http://www.data.scec.org/xml/station/", Optional, Element, "end");
		addProperty("Comment", "http://www.data.scec.org/xml/station/", Optional, Element, "comment");
	}
};


struct SiteHandler : public IO::XML::TypedClassHandler<Site> {
	SiteHandler() {
		addProperty("Name", "http://www.data.scec.org/xml/station/", Mandatory, Element, "name");
		addProperty("Description", "http://www.data.scec.org/xml/station/", Optional, Element, "description");
		addProperty("Town", "http://www.data.scec.org/xml/station/", Optional, Element, "town");
		addProperty("County", "http://www.data.scec.org/xml/station/", Optional, Element, "county");
		addProperty("Region", "http://www.data.scec.org/xml/station/", Optional, Element, "region");
		addProperty("Country", "http://www.data.scec.org/xml/station/", Optional, Element, "country");
	}
};


struct StringTypeHandler : public IO::XML::TypedClassHandler<StringType> {
	StringTypeHandler() {
		addProperty("text", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "text");
	}
};


struct AgencyTypeHandler : public IO::XML::TypedClassHandler<AgencyType> {
	AgencyTypeHandler() {
		addProperty("text", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "text");
	}
};


struct EmailTypeHandler : public IO::XML::TypedClassHandler<EmailType> {
	EmailTypeHandler() {
		addProperty("text", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "text");
	}
};


struct PhoneTypeHandler : public IO::XML::TypedClassHandler<PhoneType> {
	PhoneTypeHandler() {
		addProperty("text", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "text");
	}
};


struct ContactHandler : public IO::XML::TypedClassHandler<Contact> {
	ContactHandler() {
		addProperty("Name", "http://www.data.scec.org/xml/station/", Mandatory, Element, "name");
		addChildProperty("Agency", "http://www.data.scec.org/xml/station/", "agency");
		addChildProperty("Email", "http://www.data.scec.org/xml/station/", "email");
		addChildProperty("Phone", "http://www.data.scec.org/xml/station/", "phone");
	}
};


struct OperatorHandler : public IO::XML::TypedClassHandler<Operator> {
	OperatorHandler() {
		addChildProperty("Agency", "http://www.data.scec.org/xml/station/", "agency");
		addChildProperty("Contact", "http://www.data.scec.org/xml/station/", "contact");
	}
};


struct ExternalDocumentHandler : public IO::XML::TypedClassHandler<ExternalDocument> {
	ExternalDocumentHandler() {
		addProperty("URI", "http://www.data.scec.org/xml/station/", Mandatory, Element, "URI");
		addProperty("Description", "http://www.data.scec.org/xml/station/", Mandatory, Element, "description");
	}
};


struct DateTypeHandler : public IO::XML::TypedClassHandler<DateType> {
	DateTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
	}
};


struct EquipmentHandler : public IO::XML::TypedClassHandler<Equipment> {
	EquipmentHandler() {
		addProperty("EquipType", "http://www.data.scec.org/xml/station/", Mandatory, Element, "EquipType");
		addProperty("Description", "http://www.data.scec.org/xml/station/", Optional, Element, "Description");
		addProperty("Manufacturer", "http://www.data.scec.org/xml/station/", Optional, Element, "Manufacturer");
		addProperty("Vendor", "http://www.data.scec.org/xml/station/", Optional, Element, "Vendor");
		addProperty("Model", "http://www.data.scec.org/xml/station/", Optional, Element, "Model");
		addProperty("SerialNumber", "http://www.data.scec.org/xml/station/", Optional, Element, "SerialNumber");
		addProperty("InstallationDate", "http://www.data.scec.org/xml/station/", Optional, Element, "InstallationDate");
		addProperty("RemovalDate", "http://www.data.scec.org/xml/station/", Optional, Element, "RemovalDate");
		addChildProperty("CalibrationDate", "http://www.data.scec.org/xml/station/", "CalibrationDate");
		addProperty("id", "", Optional, Attribute, "id");
	}
};


struct OffsetHandler : public IO::XML::TypedClassHandler<Offset> {
	OffsetHandler() {
		// Element
		addProperty("North", "http://www.data.scec.org/xml/station/", Mandatory, Element, "North");
		// Element
		addProperty("East", "http://www.data.scec.org/xml/station/", Mandatory, Element, "East");
		// Element
		addProperty("Vertical", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Vertical");
	}
};


struct OutputHandler : public IO::XML::TypedClassHandler<Output> {
	OutputHandler() {
		addProperty("type", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "type");
	}
};


struct DataloggerHandler : public IO::XML::TypedClassHandler<Datalogger> {
	DataloggerHandler() {
		addProperty("EquipType", "http://www.data.scec.org/xml/station/", Mandatory, Element, "EquipType");
		addProperty("Description", "http://www.data.scec.org/xml/station/", Optional, Element, "Description");
		addProperty("Manufacturer", "http://www.data.scec.org/xml/station/", Optional, Element, "Manufacturer");
		addProperty("Vendor", "http://www.data.scec.org/xml/station/", Optional, Element, "Vendor");
		addProperty("Model", "http://www.data.scec.org/xml/station/", Optional, Element, "Model");
		addProperty("SerialNumber", "http://www.data.scec.org/xml/station/", Optional, Element, "SerialNumber");
		addProperty("InstallationDate", "http://www.data.scec.org/xml/station/", Optional, Element, "InstallationDate");
		addProperty("RemovalDate", "http://www.data.scec.org/xml/station/", Optional, Element, "RemovalDate");
		addChildProperty("CalibrationDate", "http://www.data.scec.org/xml/station/", "CalibrationDate");
		addProperty("id", "", Optional, Attribute, "id");
		// Element
		addProperty("TotalChannels", "http://www.data.scec.org/xml/station/", Optional, Element, "TotalChannels");
		// Element
		addProperty("RecordedChannels", "http://www.data.scec.org/xml/station/", Optional, Element, "RecordedChannels");
	}
};


struct SensitivityHandler : public IO::XML::TypedClassHandler<Sensitivity> {
	SensitivityHandler() {
		addProperty("SensitivityValue", "http://www.data.scec.org/xml/station/", Mandatory, Element, "SensitivityValue");
		addProperty("Frequency", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Frequency");
		addProperty("SensitivityUnits", "http://www.data.scec.org/xml/station/", Mandatory, Element, "SensitivityUnits");
		addProperty("FrequencyStart", "http://www.data.scec.org/xml/station/", Optional, Element, "FrequencyStart");
		addProperty("FrequencyEnd", "http://www.data.scec.org/xml/station/", Optional, Element, "FrequencyEnd");
		addProperty("FrequencyDBVariation", "http://www.data.scec.org/xml/station/", Optional, Element, "FrequencyDBVariation");
	}
};


struct DecimationHandler : public IO::XML::TypedClassHandler<Decimation> {
	DecimationHandler() {
		// Element
		addProperty("InputSampleRate", "http://www.data.scec.org/xml/station/", Mandatory, Element, "InputSampleRate");
		addProperty("Factor", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Factor");
		addProperty("Offset", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Offset");
		// Element
		addProperty("Delay", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Delay");
		// Element
		addProperty("Correction", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Correction");
	}
};


struct SaHandler : public IO::XML::TypedClassHandler<Sa> {
	SaHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
		addProperty("period", "http://www.data.scec.org/xml/station/", Mandatory, Element, "period");
	}
};


struct SpectraHandler : public IO::XML::TypedClassHandler<Spectra> {
	SpectraHandler() {
		// Element
		addProperty("NumberPeriods", "http://www.data.scec.org/xml/station/", Mandatory, Element, "NumberPeriods");
		// Element
		addProperty("NumberDampingValues", "http://www.data.scec.org/xml/station/", Mandatory, Element, "NumberDampingValues");
		addChildProperty("Sa", "http://www.data.scec.org/xml/station/", "Sa");
	}
};


struct CommentHandler : public IO::XML::TypedClassHandler<Comment> {
	CommentHandler() {
		addProperty("Value", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Value");
		addChildProperty("Author", "http://www.data.scec.org/xml/station/", "Author");
		addProperty("Date", "http://www.data.scec.org/xml/station/", Optional, Element, "Date");
		addProperty("id", "", Mandatory, Attribute, "id");
	}
};


struct PoleAndZeroHandler : public IO::XML::TypedClassHandler<PoleAndZero> {
	PoleAndZeroHandler() {
		// Element
		addProperty("Real", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Real");
		// Element
		addProperty("Imaginary", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Imaginary");
		addProperty("number", "", Mandatory, Attribute, "number");
	}
};


struct PolesAndZerosHandler : public IO::XML::TypedClassHandler<PolesAndZeros> {
	PolesAndZerosHandler() {
		// Element
		addProperty("Comment", "http://www.data.scec.org/xml/station/", Optional, Element, "Comment");
		addProperty("InputUnits", "http://www.data.scec.org/xml/station/", Mandatory, Element, "InputUnits");
		addProperty("OutputUnits", "http://www.data.scec.org/xml/station/", Mandatory, Element, "OutputUnits");
		addProperty("PzTransferFunctionType", "http://www.data.scec.org/xml/station/", Mandatory, Element, "PzTransferFunctionType");
		addProperty("NormalizationFactor", "http://www.data.scec.org/xml/station/", Mandatory, Element, "NormalizationFactor");
		// Element
		addProperty("NormalizationFreq", "http://www.data.scec.org/xml/station/", Mandatory, Element, "NormalizationFreq");
		addChildProperty("Pole", "http://www.data.scec.org/xml/station/", "Pole");
		addChildProperty("Zero", "http://www.data.scec.org/xml/station/", "Zero");
	}
};


struct CoefficientsHandler : public IO::XML::TypedClassHandler<Coefficients> {
	CoefficientsHandler() {
		addProperty("InputUnits", "http://www.data.scec.org/xml/station/", Mandatory, Element, "InputUnits");
		addProperty("OutputUnits", "http://www.data.scec.org/xml/station/", Mandatory, Element, "OutputUnits");
		addProperty("CfTransferFunctionType", "http://www.data.scec.org/xml/station/", Mandatory, Element, "CfTransferFunctionType");
		addChildProperty("Numerator", "http://www.data.scec.org/xml/station/", "Numerator");
		addChildProperty("Denominator", "http://www.data.scec.org/xml/station/", "Denominator");
	}
};


struct ResponseListElementHandler : public IO::XML::TypedClassHandler<ResponseListElement> {
	ResponseListElementHandler() {
		// Element
		addProperty("Frequency", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Frequency");
		// Element
		addProperty("Amplitude", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Amplitude");
		// Element
		addProperty("Phase", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Phase");
	}
};


struct ResponseListHandler : public IO::XML::TypedClassHandler<ResponseList> {
	ResponseListHandler() {
		addProperty("InputUnits", "http://www.data.scec.org/xml/station/", Mandatory, Element, "InputUnits");
		addProperty("OutputUnits", "http://www.data.scec.org/xml/station/", Mandatory, Element, "OutputUnits");
		addChildProperty("ResponseListElement", "http://www.data.scec.org/xml/station/", "element");
	}
};


struct GenericResponseHandler : public IO::XML::TypedClassHandler<GenericResponse> {
	GenericResponseHandler() {
		addProperty("GenComment", "http://www.data.scec.org/xml/station/", Optional, Element, "GenComment");
		addProperty("InputUnits", "http://www.data.scec.org/xml/station/", Mandatory, Element, "InputUnits");
		addProperty("OutputUnits", "http://www.data.scec.org/xml/station/", Mandatory, Element, "OutputUnits");
		// Element
		addProperty("Sensitivity", "http://www.data.scec.org/xml/station/", Mandatory, Element, "Sensitivity");
		// Element
		addProperty("FreeFreq", "http://www.data.scec.org/xml/station/", Mandatory, Element, "FreeFreq");
		// Element
		addProperty("HighPass", "http://www.data.scec.org/xml/station/", Mandatory, Element, "HighPass");
		// Element
		addProperty("LowPass", "http://www.data.scec.org/xml/station/", Mandatory, Element, "LowPass");
	}
};


struct NumeratorCoefficientHandler : public IO::XML::TypedClassHandler<NumeratorCoefficient> {
	NumeratorCoefficientHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("i", "", Mandatory, Attribute, "i");
	}
};


struct FIRHandler : public IO::XML::TypedClassHandler<FIR> {
	FIRHandler() {
		addProperty("ResponseName", "http://www.data.scec.org/xml/station/", Optional, Element, "ResponseName");
		addProperty("Symmetry", "http://www.data.scec.org/xml/station/", Optional, Element, "Symmetry");
		addProperty("InputUnits", "http://www.data.scec.org/xml/station/", Mandatory, Element, "InputUnits");
		addProperty("OutputUnits", "http://www.data.scec.org/xml/station/", Mandatory, Element, "OutputUnits");
		addChildProperty("NumeratorCoefficient", "http://www.data.scec.org/xml/station/", "NumeratorCoefficient");
	}
};


struct PolynomialCoefficientHandler : public IO::XML::TypedClassHandler<PolynomialCoefficient> {
	PolynomialCoefficientHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("number", "", Mandatory, Attribute, "number");
	}
};


struct PolynomialHandler : public IO::XML::TypedClassHandler<Polynomial> {
	PolynomialHandler() {
		addProperty("InputUnits", "http://www.data.scec.org/xml/station/", Mandatory, Element, "InputUnits");
		addProperty("OutputUnits", "http://www.data.scec.org/xml/station/", Mandatory, Element, "OutputUnits");
		addProperty("ApproximationType", "http://www.data.scec.org/xml/station/", Mandatory, Element, "ApproximationType");
		// Element
		addProperty("FreqLowerBound", "http://www.data.scec.org/xml/station/", Mandatory, Element, "FreqLowerBound");
		// Element
		addProperty("FreqUpperBound", "http://www.data.scec.org/xml/station/", Mandatory, Element, "FreqUpperBound");
		// Element
		addProperty("ApproxLowerBound", "http://www.data.scec.org/xml/station/", Mandatory, Element, "ApproxLowerBound");
		// Element
		addProperty("ApproxUpperBound", "http://www.data.scec.org/xml/station/", Mandatory, Element, "ApproxUpperBound");
		// Element
		addProperty("MaxError", "http://www.data.scec.org/xml/station/", Mandatory, Element, "MaxError");
		addChildProperty("Coefficient", "http://www.data.scec.org/xml/station/", "Coefficient");
	}
};


struct ResponseHandler : public IO::XML::TypedClassHandler<Response> {
	ResponseHandler() {
		addChildProperty("PolesZeros", "http://www.data.scec.org/xml/station/", "PolesAndZeros");
		addChildProperty("Coefficients", "http://www.data.scec.org/xml/station/", "Coefficients");
		addChildProperty("ResponseList", "http://www.data.scec.org/xml/station/", "ResponseList");
		addChildProperty("Generic", "http://www.data.scec.org/xml/station/", "Generic");
		addChildProperty("FIR", "http://www.data.scec.org/xml/station/", "FIR");
		addChildProperty("Polynomial", "http://www.data.scec.org/xml/station/", "Polynomial");
		// Element
		addProperty("Decimation", "http://www.data.scec.org/xml/station/", Optional, Element, "Decimation");
		// Element
		addProperty("StageSensitivity", "http://www.data.scec.org/xml/station/", Mandatory, Element, "StageSensitivity");
		// Element
		addProperty("Spectra", "http://www.data.scec.org/xml/station/", Optional, Element, "Spectra");
		addProperty("stage", "", Mandatory, Attribute, "stage");
		addProperty("stage_description", "", Optional, Attribute, "stageDescription");
	}
};


struct GainTypeHandler : public IO::XML::TypedClassHandler<GainType> {
	GainTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
		addProperty("nominal", "", Mandatory, Attribute, "nominal");
	}
};


struct LeastSignificantBitTypeHandler : public IO::XML::TypedClassHandler<LeastSignificantBitType> {
	LeastSignificantBitTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct VoltageTypeHandler : public IO::XML::TypedClassHandler<VoltageType> {
	VoltageTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct SecondTypeHandler : public IO::XML::TypedClassHandler<SecondType> {
	SecondTypeHandler() {
		addProperty("value", "http://www.data.scec.org/xml/station/", Mandatory, CDATA, "value");
		addProperty("plus_error", "", Optional, Attribute, "upperUncertainty");
		addProperty("minus_error", "", Optional, Attribute, "lowerUncertainty");
		addProperty("unit", "", Optional, Attribute, "unit");
	}
};


struct ChannelEpochHandler : public IO::XML::TypedClassHandler<ChannelEpoch> {
	ChannelEpochHandler() {
		addProperty("StartDate", "http://www.data.scec.org/xml/station/", Mandatory, Element, "start");
		addProperty("EndDate", "http://www.data.scec.org/xml/station/", Optional, Element, "end");
		addProperty("Comment", "http://www.data.scec.org/xml/station/", Optional, Element, "comment");
		// Element
		addProperty("Lat", "http://www.data.scec.org/xml/station/", Mandatory, Element, "latitude");
		// Element
		addProperty("Lon", "http://www.data.scec.org/xml/station/", Mandatory, Element, "longitude");
		// Element
		addProperty("Elevation", "http://www.data.scec.org/xml/station/", Mandatory, Element, "elevation");
		// Element
		addProperty("Depth", "http://www.data.scec.org/xml/station/", Mandatory, Element, "depth");
		// Element
		addProperty("Azimuth", "http://www.data.scec.org/xml/station/", Optional, Element, "azimuth");
		// Element
		addProperty("Dip", "http://www.data.scec.org/xml/station/", Optional, Element, "dip");
		// Element
		addProperty("Offset", "http://www.data.scec.org/xml/station/", Optional, Element, "offset");
		addChildProperty("Output", "http://www.data.scec.org/xml/station/", "Output");
		// Element
		addProperty("SampleRate", "http://www.data.scec.org/xml/station/", Optional, Element, "SampleRate");
		// Element
		addProperty("SampleRateRatio", "http://www.data.scec.org/xml/station/", Optional, Element, "SampleRateRatio");
		addProperty("StorageFormat", "http://www.data.scec.org/xml/station/", Optional, Element, "StorageFormat");
		// Element
		addProperty("ClockDrift", "http://www.data.scec.org/xml/station/", Optional, Element, "ClockDrift");
		addProperty("CalibrationUnit", "http://www.data.scec.org/xml/station/", Optional, Element, "CalibrationUnit");
		// Element
		addProperty("DataLogger", "http://www.data.scec.org/xml/station/", Optional, Element, "datalogger");
		// Element
		addProperty("Sensor", "http://www.data.scec.org/xml/station/", Optional, Element, "Sensor");
		// Element
		addProperty("PreAmplifier", "http://www.data.scec.org/xml/station/", Optional, Element, "PreAmplifier");
		// Element
		addProperty("InstrumentSensitivity", "http://www.data.scec.org/xml/station/", Optional, Element, "InstrumentSensitivity");
		addChildProperty("Response", "http://www.data.scec.org/xml/station/", "Response");
		addProperty("DampingConstant", "http://www.data.scec.org/xml/station/", Optional, Element, "DampingConstant");
		// Element
		addProperty("NaturalFrequency", "http://www.data.scec.org/xml/station/", Optional, Element, "NaturalFrequency");
		// Element
		addProperty("LeastSignificantBit", "http://www.data.scec.org/xml/station/", Optional, Element, "LeastSignificantBit");
		// Element
		addProperty("FullScaleInput", "http://www.data.scec.org/xml/station/", Optional, Element, "FullScaleInput");
		// Element
		addProperty("FullScaleOutput", "http://www.data.scec.org/xml/station/", Optional, Element, "FullScaleOutput");
		// Element
		addProperty("FullScaleCapability", "http://www.data.scec.org/xml/station/", Optional, Element, "FullScaleCapability");
		// Element
		addProperty("PreTriggerTime", "http://www.data.scec.org/xml/station/", Optional, Element, "PreTriggerTime");
		// Element
		addProperty("PostDetriggerTime", "http://www.data.scec.org/xml/station/", Optional, Element, "PostDetriggerTime");
	}
};


struct ChannelHandler : public IO::XML::TypedClassHandler<Channel> {
	ChannelHandler() {
		addProperty("chan_code", "", Mandatory, Attribute, "code");
		addProperty("seed_chan_code", "", Optional, Attribute, "seedCode");
		addProperty("historical_chan_code", "", Optional, Attribute, "historicalCode");
		addProperty("loc_code", "", Optional, Attribute, "locationCode");
		addProperty("restricted", "", Optional, Attribute, "restricted");
		addProperty("CreationDate", "http://www.data.scec.org/xml/station/", Mandatory, Element, "creationDate");
		addProperty("TerminationDate", "http://www.data.scec.org/xml/station/", Optional, Element, "terminationDate");
		addProperty("Dataless", "http://www.data.scec.org/xml/station/", Optional, Element, "dataless");
		addChildProperty("ExternalReport", "http://www.data.scec.org/xml/station/", "externalReport");
		addChildProperty("Epoch", "http://www.data.scec.org/xml/station/", "epoch");
	}
};


struct StationEpochHandler : public IO::XML::TypedClassHandler<StationEpoch> {
	StationEpochHandler() {
		addProperty("StartDate", "http://www.data.scec.org/xml/station/", Mandatory, Element, "start");
		addProperty("EndDate", "http://www.data.scec.org/xml/station/", Optional, Element, "end");
		addProperty("Comment", "http://www.data.scec.org/xml/station/", Optional, Element, "comment");
		// Element
		addProperty("Lat", "http://www.data.scec.org/xml/station/", Mandatory, Element, "latitude");
		// Element
		addProperty("Lon", "http://www.data.scec.org/xml/station/", Mandatory, Element, "longitude");
		// Element
		addProperty("Elevation", "http://www.data.scec.org/xml/station/", Mandatory, Element, "elevation");
		// Element
		addProperty("Site", "http://www.data.scec.org/xml/station/", Mandatory, Element, "site");
		addProperty("Vault", "http://www.data.scec.org/xml/station/", Optional, Element, "vault");
		addProperty("Geology", "http://www.data.scec.org/xml/station/", Optional, Element, "geology");
		addProperty("Structure", "http://www.data.scec.org/xml/station/", Optional, Element, "structure");
		addChildProperty("Equipment", "http://www.data.scec.org/xml/station/", "equipment");
		addChildProperty("Operator", "http://www.data.scec.org/xml/station/", "operators");
		addProperty("CreationDate", "http://www.data.scec.org/xml/station/", Mandatory, Element, "creationDate");
		addProperty("TerminationDate", "http://www.data.scec.org/xml/station/", Optional, Element, "terminationDate");
		addProperty("NumberRecorders", "http://www.data.scec.org/xml/station/", Optional, Element, "numberRecorders");
		addProperty("TotalNumberChannels", "http://www.data.scec.org/xml/station/", Optional, Element, "totalNumberChannels");
		addProperty("SelectedNumberChannels", "http://www.data.scec.org/xml/station/", Optional, Element, "selectedNumberChannels");
		addChildProperty("ExternalReport", "http://www.data.scec.org/xml/station/", "externalReport");
		addChildProperty("Channel", "http://www.data.scec.org/xml/station/", "channel");
	}
};


struct StationHandler : public IO::XML::TypedClassHandler<Station> {
	StationHandler() {
		addProperty("net_code", "", Mandatory, Attribute, "netCode");
		addProperty("sta_code", "", Mandatory, Attribute, "code");
		addProperty("sta_number", "", Optional, Attribute, "sta_number");
		addProperty("agency_code", "", Optional, Attribute, "agency_code");
		addProperty("deployment_code", "", Optional, Attribute, "deployment_code");
		addChildProperty("StationEpoch", "http://www.data.scec.org/xml/station/", "epoch");
	}
};


struct NetworkHandler : public IO::XML::TypedClassHandler<Network> {
	NetworkHandler() {
		addProperty("StartDate", "http://www.data.scec.org/xml/station/", Optional, Element, "start");
		addProperty("EndDate", "http://www.data.scec.org/xml/station/", Optional, Element, "end");
		addProperty("Description", "http://www.data.scec.org/xml/station/", Optional, Element, "description");
		addProperty("TotalNumberStations", "http://www.data.scec.org/xml/station/", Optional, Element, "totalNumberOfStations");
		addChildProperty("Station", "http://www.data.scec.org/xml/station/", "station");
		addProperty("SelectedNumberStations", "http://www.data.scec.org/xml/station/", Optional, Element, "selectedNumberStations");
		addProperty("net_code", "", Mandatory, Attribute, "code");
	}
};


struct StaMessageHandler : public IO::XML::TypedClassHandler<StaMessage> {
	StaMessageHandler() {
		addProperty("Source", "http://www.data.scec.org/xml/station/", Mandatory, Element, "source");
		addProperty("Sender", "http://www.data.scec.org/xml/station/", Optional, Element, "sender");
		addProperty("Module", "http://www.data.scec.org/xml/station/", Optional, Element, "module");
		addProperty("ModuleURI", "http://www.data.scec.org/xml/station/", Optional, Element, "moduleURI");
		addProperty("SentDate", "http://www.data.scec.org/xml/station/", Mandatory, Element, "sentDate");
		addChildProperty("Network", "http://www.data.scec.org/xml/station/", "network");
		addChildProperty("Station", "http://www.data.scec.org/xml/station/", "station");
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
	static LatTypeHandler _LatTypeHandler;
	static LonTypeHandler _LonTypeHandler;
	static DistanceTypeHandler _DistanceTypeHandler;
	static AngleTypeHandler _AngleTypeHandler;
	static DipTypeHandler _DipTypeHandler;
	static AzimuthTypeHandler _AzimuthTypeHandler;
	static ClockDriftTypeHandler _ClockDriftTypeHandler;
	static SampleRateTypeHandler _SampleRateTypeHandler;
	static SampleRateRatioTypeHandler _SampleRateRatioTypeHandler;
	static SampleRateGroupHandler _SampleRateGroupHandler;
	static CounterTypeHandler _CounterTypeHandler;
	static FrequencyTypeHandler _FrequencyTypeHandler;
	static RollTypeHandler _RollTypeHandler;
	static PassTypeHandler _PassTypeHandler;
	static EpochHandler _EpochHandler;
	static SiteHandler _SiteHandler;
	static StringTypeHandler _StringTypeHandler;
	static AgencyTypeHandler _AgencyTypeHandler;
	static EmailTypeHandler _EmailTypeHandler;
	static PhoneTypeHandler _PhoneTypeHandler;
	static ContactHandler _ContactHandler;
	static OperatorHandler _OperatorHandler;
	static ExternalDocumentHandler _ExternalDocumentHandler;
	static DateTypeHandler _DateTypeHandler;
	static EquipmentHandler _EquipmentHandler;
	static OffsetHandler _OffsetHandler;
	static OutputHandler _OutputHandler;
	static DataloggerHandler _DataloggerHandler;
	static SensitivityHandler _SensitivityHandler;
	static DecimationHandler _DecimationHandler;
	static SaHandler _SaHandler;
	static SpectraHandler _SpectraHandler;
	static CommentHandler _CommentHandler;
	static PoleAndZeroHandler _PoleAndZeroHandler;
	static PolesAndZerosHandler _PolesAndZerosHandler;
	static CoefficientsHandler _CoefficientsHandler;
	static ResponseListElementHandler _ResponseListElementHandler;
	static ResponseListHandler _ResponseListHandler;
	static GenericResponseHandler _GenericResponseHandler;
	static NumeratorCoefficientHandler _NumeratorCoefficientHandler;
	static FIRHandler _FIRHandler;
	static PolynomialCoefficientHandler _PolynomialCoefficientHandler;
	static PolynomialHandler _PolynomialHandler;
	static ResponseHandler _ResponseHandler;
	static GainTypeHandler _GainTypeHandler;
	static LeastSignificantBitTypeHandler _LeastSignificantBitTypeHandler;
	static VoltageTypeHandler _VoltageTypeHandler;
	static SecondTypeHandler _SecondTypeHandler;
	static ChannelEpochHandler _ChannelEpochHandler;
	static ChannelHandler _ChannelHandler;
	static StationEpochHandler _StationEpochHandler;
	static StationHandler _StationHandler;
	static NetworkHandler _NetworkHandler;
	static StaMessageHandler _StaMessageHandler;

	registerMapping<IntType>("IntType", "http://www.data.scec.org/xml/station/", &_IntTypeHandler);
	registerMapping<FloatNoUnitType>("FloatNoUnitType", "http://www.data.scec.org/xml/station/", &_FloatNoUnitTypeHandler);
	registerMapping<FloatType>("FloatType", "http://www.data.scec.org/xml/station/", &_FloatTypeHandler);
	registerMapping<LatType>("LatType", "http://www.data.scec.org/xml/station/", &_LatTypeHandler);
	registerMapping<LonType>("LonType", "http://www.data.scec.org/xml/station/", &_LonTypeHandler);
	registerMapping<DistanceType>("DistanceType", "http://www.data.scec.org/xml/station/", &_DistanceTypeHandler);
	registerMapping<AngleType>("AngleType", "http://www.data.scec.org/xml/station/", &_AngleTypeHandler);
	registerMapping<DipType>("DipType", "http://www.data.scec.org/xml/station/", &_DipTypeHandler);
	registerMapping<AzimuthType>("AzimuthType", "http://www.data.scec.org/xml/station/", &_AzimuthTypeHandler);
	registerMapping<ClockDriftType>("ClockDriftType", "http://www.data.scec.org/xml/station/", &_ClockDriftTypeHandler);
	registerMapping<SampleRateType>("SampleRateType", "http://www.data.scec.org/xml/station/", &_SampleRateTypeHandler);
	registerMapping<SampleRateRatioType>("SampleRateRatioType", "http://www.data.scec.org/xml/station/", &_SampleRateRatioTypeHandler);
	registerMapping<SampleRateGroup>("SampleRateGroup", "http://www.data.scec.org/xml/station/", &_SampleRateGroupHandler);
	registerMapping<CounterType>("CounterType", "http://www.data.scec.org/xml/station/", &_CounterTypeHandler);
	registerMapping<FrequencyType>("FrequencyType", "http://www.data.scec.org/xml/station/", &_FrequencyTypeHandler);
	registerMapping<RollType>("RollType", "http://www.data.scec.org/xml/station/", &_RollTypeHandler);
	registerMapping<PassType>("PassType", "http://www.data.scec.org/xml/station/", &_PassTypeHandler);
	registerMapping<Epoch>("Epoch", "http://www.data.scec.org/xml/station/", &_EpochHandler);
	registerMapping<Site>("Site", "http://www.data.scec.org/xml/station/", &_SiteHandler);
	registerMapping<StringType>("StringType", "http://www.data.scec.org/xml/station/", &_StringTypeHandler);
	registerMapping<AgencyType>("AgencyType", "http://www.data.scec.org/xml/station/", &_AgencyTypeHandler);
	registerMapping<EmailType>("EmailType", "http://www.data.scec.org/xml/station/", &_EmailTypeHandler);
	registerMapping<PhoneType>("PhoneType", "http://www.data.scec.org/xml/station/", &_PhoneTypeHandler);
	registerMapping<Contact>("Contact", "http://www.data.scec.org/xml/station/", &_ContactHandler);
	registerMapping<Operator>("Operator", "http://www.data.scec.org/xml/station/", &_OperatorHandler);
	registerMapping<ExternalDocument>("ExternalDocument", "http://www.data.scec.org/xml/station/", &_ExternalDocumentHandler);
	registerMapping<DateType>("DateType", "http://www.data.scec.org/xml/station/", &_DateTypeHandler);
	registerMapping<Equipment>("Equipment", "http://www.data.scec.org/xml/station/", &_EquipmentHandler);
	registerMapping<Offset>("Offset", "http://www.data.scec.org/xml/station/", &_OffsetHandler);
	registerMapping<Output>("Output", "http://www.data.scec.org/xml/station/", &_OutputHandler);
	registerMapping<Datalogger>("Datalogger", "http://www.data.scec.org/xml/station/", &_DataloggerHandler);
	registerMapping<Sensitivity>("Sensitivity", "http://www.data.scec.org/xml/station/", &_SensitivityHandler);
	registerMapping<Decimation>("Decimation", "http://www.data.scec.org/xml/station/", &_DecimationHandler);
	registerMapping<Sa>("Sa", "http://www.data.scec.org/xml/station/", &_SaHandler);
	registerMapping<Spectra>("Spectra", "http://www.data.scec.org/xml/station/", &_SpectraHandler);
	registerMapping<Comment>("Comment", "http://www.data.scec.org/xml/station/", &_CommentHandler);
	registerMapping<PoleAndZero>("PoleAndZero", "http://www.data.scec.org/xml/station/", &_PoleAndZeroHandler);
	registerMapping<PolesAndZeros>("PolesAndZeros", "http://www.data.scec.org/xml/station/", &_PolesAndZerosHandler);
	registerMapping<Coefficients>("Coefficients", "http://www.data.scec.org/xml/station/", &_CoefficientsHandler);
	registerMapping<ResponseListElement>("ResponseListElement", "http://www.data.scec.org/xml/station/", &_ResponseListElementHandler);
	registerMapping<ResponseList>("ResponseList", "http://www.data.scec.org/xml/station/", &_ResponseListHandler);
	registerMapping<GenericResponse>("GenericResponse", "http://www.data.scec.org/xml/station/", &_GenericResponseHandler);
	registerMapping<NumeratorCoefficient>("NumeratorCoefficient", "http://www.data.scec.org/xml/station/", &_NumeratorCoefficientHandler);
	registerMapping<FIR>("FIR", "http://www.data.scec.org/xml/station/", &_FIRHandler);
	registerMapping<PolynomialCoefficient>("PolynomialCoefficient", "http://www.data.scec.org/xml/station/", &_PolynomialCoefficientHandler);
	registerMapping<Polynomial>("Polynomial", "http://www.data.scec.org/xml/station/", &_PolynomialHandler);
	registerMapping<Response>("Response", "http://www.data.scec.org/xml/station/", &_ResponseHandler);
	registerMapping<GainType>("GainType", "http://www.data.scec.org/xml/station/", &_GainTypeHandler);
	registerMapping<LeastSignificantBitType>("LeastSignificantBitType", "http://www.data.scec.org/xml/station/", &_LeastSignificantBitTypeHandler);
	registerMapping<VoltageType>("VoltageType", "http://www.data.scec.org/xml/station/", &_VoltageTypeHandler);
	registerMapping<SecondType>("SecondType", "http://www.data.scec.org/xml/station/", &_SecondTypeHandler);
	registerMapping<ChannelEpoch>("ChannelEpoch", "http://www.data.scec.org/xml/station/", &_ChannelEpochHandler);
	registerMapping<Channel>("Channel", "http://www.data.scec.org/xml/station/", &_ChannelHandler);
	registerMapping<StationEpoch>("StationEpoch", "http://www.data.scec.org/xml/station/", &_StationEpochHandler);
	registerMapping<Station>("Station", "http://www.data.scec.org/xml/station/", &_StationHandler);
	registerMapping<Network>("Network", "http://www.data.scec.org/xml/station/", &_NetworkHandler);
	registerMapping<StaMessage>("StaMessage", "http://www.data.scec.org/xml/station/", &_StaMessageHandler);
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
	_defaultNsMap["http://www.data.scec.org/xml/station/"] = "";
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Exporter::collectNamespaces(Core::BaseObject *) {
	// Just copy the defined default namespace map to avoid expensive
	// namespace collections
	_namespaces = _defaultNsMap;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
