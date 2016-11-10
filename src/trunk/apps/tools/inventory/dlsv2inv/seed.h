/*==============================================================================
    Name:       seed.h

    Purpose:    define all seed control headers and the underlying blockettes

    Language:   C++, ANSI standard.

    Author:     Peter de Boer

    Revision:   0.1 initial  04-01-2008

==============================================================================*/
#ifndef SEED_H
#define SEED_H

#include <seiscomp3/core/baseobject.h>
#include "mystring.h"
#include "tmanip.h"
#include "define.h"


enum ParseResult {
	PR_Error = 0,
	PR_OK,
	PR_Partial,
	PR_NotSupported
};


DEFINE_SMARTPOINTER(Record);
class Record : public Seiscomp::Core::BaseObject {
	public:
		Record() {}
		~Record() {}

		//! Parse an record initially. Return PR_Partial if more data are
		//! required.
		virtual ParseResult Parse(std::string record) = 0;

		//! Continues a previous record because Parse return PR_Partial.
		//! This operation is not supported by default.
		virtual ParseResult Merge(std::string record) {
			return PR_NotSupported;
		}
};


#define DECLARE_BLOCKETTE(Name) DEFINE_SMARTPOINTER(Name);\
typedef std::vector<Name##Ptr> Name##List;\
class Name : public Record

#define DECLARE_BLOCKETTE2(Name, Base) DEFINE_SMARTPOINTER(Name);\
typedef std::vector<Name##Ptr> Name##List;\
class Name : public Base


class Control
{
	public:
		void SetRemains(const std::string&);
		void SetRemains(const std::string&, int);
		std::string GetRemains();
		bool SetBytesLeft(int, const std::string&);
		int GetBytesLeft();
		void SetBytes(int);

	private:
		std::string 	remains_of_record;
		int 		bytes_left;
};

// definition of all Volume Index Control Header blockettes
class FieldVolumeIdentifier
{
	public:
		FieldVolumeIdentifier(std::string);
		float GetVersionOfFormat(){return version_of_format;}
		int GetLogicalRecordLength(){return logical_record_length;}
		std::string GetBeginningOfVolume(){return beginning_of_volume;}

	private:
		float		version_of_format;
		int		logical_record_length;
		std::string	beginning_of_volume;
};

class TelemetryVolumeIdentifier
{
	public:
		TelemetryVolumeIdentifier(std::string);
		float GetVersionOfFormat(){return version_of_format;}
		int GetLogicalRecordLength(){return logical_record_length;}
		std::string GetStationIdentifier(){return station_identifier;}
		std::string GetChannelIdentifier(){return channel_identifier;}
		std::string GetLocationIdentifier(){return location_identifier;}
		std::string GetNetworkCode(){return network_code;}
		std::string GetBeginningOfVolume(){return beginning_of_volume;}
		std::string GetEndOfVolume(){return end_of_volume;}
		std::string GetStationInformationEffectiveDate(){return station_information_effective_date;}
		std::string GetChannelInformationEffectiveDate(){return channel_information_effective_date;}

	private:
		float		version_of_format;
		int		logical_record_length;
		std::string	station_identifier, channel_identifier;
		std::string	location_identifier, network_code;
		std::string	beginning_of_volume, end_of_volume;
		std::string	station_information_effective_date, channel_information_effective_date;
};

class VolumeIdentifier
{
	public:
		VolumeIdentifier(std::string);
		float GetVersionOfFormat(){return version_of_format;}
		int GetLogicalRecordLength(){return logical_record_length;}
		std::string GetBeginningTime(){return beginning_time;}
		std::string GetEndTime(){return end_time;}
		std::string GetVolumeTime(){return volume_time;}
		std::string GetOriginatingOrganization(){return originating_organization;}
		std::string GetLabel(){return label;}

	private:
		float		version_of_format;
		int		logical_record_length;
		std::string	beginning_time, end_time, volume_time;
		std::string	originating_organization, label;
};

class VolumeStationHeaderIndex
{
	public:
		VolumeStationHeaderIndex(std::string);
		int GetNumberOfStations(){return number_of_stations;}
		STATION_INFO GetStationInfo(){return station_info;}
	protected:
	private:
		int 	number_of_stations;
		// the station_info map contains the station_identifier_code and the sequence_number
		STATION_INFO 	station_info;
};

struct TimeSpan
{
	std::string 	beginning_of_span, end_of_span;
	int 	sequence_number;
};

class VolumeTimeSpanIndex
{
	public:
		VolumeTimeSpanIndex(std::string);
		int GetNumberOfSpans(){return number_of_spans;}
		const std::vector<TimeSpan> &GetTimespans(){return timespans;}

	private:
		int	number_of_spans;
		std::vector<TimeSpan> 	timespans;
};

class VolumeIndexControl : public Control
{
	public:
		void ParseVolumeRecord(std::string);
		void EmptyVectors();
		std::vector<FieldVolumeIdentifier> 	fvi;
		std::vector<TelemetryVolumeIdentifier> 	tvi;
		std::vector<VolumeIdentifier> 		vi;
		std::vector<VolumeStationHeaderIndex> 	vshi;
		std::vector<VolumeTimeSpanIndex> 	vtsi;

	private:
		Logging *log;
};

// definition of all Abbreviation Dictionary Control Headers blockettes
DECLARE_BLOCKETTE(DataFormatDictionary) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetDataFormatIdentifierCode(){return data_format_identifier_code;}
		int GetDataFamilyType(){return data_family_type;}
		int GetNumberOfDecoderKeys(){return number_of_decoder_keys;}
		std::string GetShortDescriptiveName(){return short_descriptive_name;}
		const std::vector<std::string> &GetDecoderKeys(){return decoder_keys;}

	private:
		int                      data_format_identifier_code, data_family_type, number_of_decoder_keys;
		std::string              short_descriptive_name;
		std::vector<std::string> decoder_keys;
};

DECLARE_BLOCKETTE(CommentDescription) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetCommentCodeKey(){return comment_code_key;}
		int GetUnits(){return units;}
		char GetCommentCodeClass(){return comment_class_code;}
		std::string GetDescriptionOfComment(){return description_of_comment;}

	private:
		int         comment_code_key, units;
		char        comment_class_code;
		std::string description_of_comment;
};

DECLARE_BLOCKETTE(CitedSourceDictionary) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetSourceLookupCode(){return source_lookup_code;}
		std::string GetNumberOfPublication(){return name_of_publication;}
		std::string GetDatePublished(){return date_published;}
		std::string GetPublisherName(){return publisher_name;}

	private:
		int         source_lookup_code;
		std::string name_of_publication, date_published, publisher_name;
};

DECLARE_BLOCKETTE(GenericAbbreviation) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetLookup(){return lookup_code;}
		std::string GetDescription(){return description;}

	private:
		int         lookup_code;
		std::string description;
};

DECLARE_BLOCKETTE(UnitsAbbreviations) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetLookup(){return lookup_code;}
		std::string GetName(){return name;}
		std::string GetDescription(){return description;}

	private:
		int         lookup_code;
		std::string name, description;
};

struct Component
{
	std::string	station, location, channel, subchannel, component_weight;
};

DECLARE_BLOCKETTE(BeamConfiguration) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetLookup(){return lookup_code;}
		int GetNumberOfComponents(){return number_of_components;}
		std::vector<Component> GetComponents(){return components;}

	private:
		int                    lookup_code, number_of_components;
		std::vector<Component> components;
};

class SequenceRecord : public Record {
	public:
		void SetStageSequenceNumber(int sn) { stage_sequence_number = sn; }
		int GetStageSequenceNumber(){return stage_sequence_number;}

	protected:
		int stage_sequence_number;
};

DECLARE_BLOCKETTE2(FIRResponse, SequenceRecord) {
	public:
		virtual ParseResult Parse(std::string record);
		virtual ParseResult Merge(std::string);

		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfCoefficients(){return number_of_coefficients;}
		char GetSymmetryCode(){return symmetry_code;}
		std::string GetName(){return response_name;}
		std::string GetCoefficients();

	protected:
		int                 number_of_coefficients;
		int                 signal_in_units, signal_out_units;
		char                symmetry_code;
		std::string         response_name;
		std::vector<double> coefficients;
};

struct CornerList {
	double frequency, slope;
};

DECLARE_BLOCKETTE2(FIRDictionary, FIRResponse) {
	public:
		virtual ParseResult Parse(std::string);
		virtual ParseResult Merge(std::string);

		int GetLookup() { return lookup_key; }

	private:
		int                 lookup_key;
};

struct Coefficient
{
	double coefficient, error;
};

DECLARE_BLOCKETTE2(ResponsePolynomial, SequenceRecord) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfPcoeff(){return number_of_pcoeff;}
		char GetTransferFunctionType(){return transfer_function_type;}
		char GetPolynomialApproximationType(){return polynomial_approximation_type;}
		char GetValidFrequencyUnits(){return valid_frequency_units;}
		double GetLowerValidFrequencyBound(){return lower_valid_frequency_bound;}
		double GetUpperValidFrequencyBound(){return upper_valid_frequency_bound;}
		double GetLowerBoundOfApproximation(){return lower_bound_of_approximation;}
		double GetUpperBoundOfApproximation(){return upper_bound_of_approximation;}
		double GetMaximumAbsoluteError(){return maximum_absolute_error;}
		std::string GetPolynomialCoefficients();

	protected:
		int    number_of_pcoeff;
		int    signal_in_units, signal_out_units;
		char   transfer_function_type, polynomial_approximation_type, valid_frequency_units;
		double lower_valid_frequency_bound, upper_valid_frequency_bound;
		double lower_bound_of_approximation, upper_bound_of_approximation, maximum_absolute_error;
		std::vector<Coefficient> polynomial_coefficients;
};

DECLARE_BLOCKETTE2(ResponsePolynomialDictionary, ResponsePolynomial) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetLookup(){return lookup_key;}
		std::string GetName(){return name;}

	private:
		int         lookup_key;
		std::string name;
};

struct Zeros
{
	double real_zero, imaginary_zero, real_zero_error, imaginary_zero_error;
};

struct Poles
{
	double real_pole, imaginary_pole, real_pole_error, imaginary_pole_error;
};

DECLARE_BLOCKETTE2(ResponsePolesZeros, SequenceRecord) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfPoles(){return number_of_poles;}
		int GetNumberOfZeros(){return number_of_zeros;}
		char GetTransferFunctionType(){return transfer_function_type;}
		double GetAoNormalizationFactor(){return ao_normalization_factor;}
		double GetNormalizationFrequency(){return normalization_frequency;}
		std::string GetComplexZeros();
		std::string GetComplexPoles();

	protected:
		char    transfer_function_type;
		int     number_of_zeros, number_of_poles;
		int     signal_in_units, signal_out_units;
		double  ao_normalization_factor, normalization_frequency;
		std::vector<Zeros> complex_zeros;
		std::vector<Poles> complex_poles;
};

DECLARE_BLOCKETTE2(ResponsePolesZerosDictionary, ResponsePolesZeros) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetLookup(){return lookup_key;}
		std::string GetName(){return name;}

	private:
		int         lookup_key;
		std::string name;
};

DECLARE_BLOCKETTE2(ResponseCoefficients, SequenceRecord) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfNumerators(){return number_of_numerators;}
		int GetNumberOfDenominators(){return number_of_denominators;}
		char GetResponseType(){return response_type;}
		std::string GetNumerators();
		std::string GetDenominators();

	protected:
		int	number_of_numerators, number_of_denominators;
		int  signal_in_units, signal_out_units;
		char response_type;
		std::vector<Coefficient> numerators;
		std::vector<Coefficient> denominators;
};

DECLARE_BLOCKETTE2(ResponseCoefficientsDictionary, ResponseCoefficients) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetLookup(){return lookup_key;}
		std::string GetName(){return name;}

	private:
		int             lookup_key;
		std::string     name;
};

struct ListedResponses
{
	double frequency, amplitude, amplitude_error, phase_angle, phase_error;
};

DECLARE_BLOCKETTE2(ResponseList, SequenceRecord) {
	public:
		virtual ParseResult Parse(std::string record);
		virtual ParseResult Merge(std::string);

		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfResponses(){return number_of_responses;}
		const std::vector<ListedResponses> &GetResponsesListed(){return responses_listed;}

	protected:
		int	number_of_responses;
		int	signal_in_units, signal_out_units;
		std::vector<ListedResponses> responses_listed;

	friend class StationControl;
};

DECLARE_BLOCKETTE2(ResponseListDictionary, ResponseList) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetLookup(){return lookup_key;}
		std::string GetName(){return name;}

 	private:
		int             lookup_key;
		std::string     name;
};

DECLARE_BLOCKETTE2(GenericResponse, SequenceRecord) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfCorners(){return number_of_corners;}
		const std::vector<CornerList> &GetCornersListed(){return corners_listed;}

	protected:
		int 	number_of_corners;
		int 	signal_in_units, signal_out_units;
		std::vector<CornerList> corners_listed;
};

DECLARE_BLOCKETTE2(GenericResponseDictionary, GenericResponse) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetLookup(){return lookup_key;}
		std::string GetName(){return name;}

	private:
		int             lookup_key;
		std::string     name;
};

DECLARE_BLOCKETTE2(Decimation, SequenceRecord) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetDecimationFactor(){return factor;}
		int GetDecimationOffset(){return offset;}
		double GetInputSampleRate(){return input_sample_rate;}
		double GetEstimatedDelay(){return estimated_delay;}
		double GetCorrectionApplied(){return correction_applied;}

	protected:
		int    factor, offset;
		double input_sample_rate, estimated_delay, correction_applied;
};

DECLARE_BLOCKETTE2(DecimationDictionary, Decimation) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetLookup(){return lookup_key;}
		std::string GetName(){return name;}

	private:
		int         lookup_key;
		std::string name;
};

struct HistoryValues
{
	double	sensitivity_for_calibration, frequency_of_calibration_sensitivity;
	std::string	time_of_calibration;
};

DECLARE_BLOCKETTE2(ChannelSensitivityGain, SequenceRecord) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetNumberOfHistoryValues(){return number_of_history_values;}
		double GetSensitivityGain(){return sensitivity_gain;}
		double GetFrequency(){return frequency;}
		const std::vector<HistoryValues> &GetHistoryValues(){return history_values;}

	protected:
		int    number_of_history_values;
		double sensitivity_gain, frequency;
		std::vector<HistoryValues> history_values;
};

DECLARE_BLOCKETTE2(ChannelSensitivityGainDictionary, ChannelSensitivityGain) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetLookup(){return lookup_key;}
		std::string GetName(){return name;}

	private:
		int             lookup_key;
		std::string     name;
};

class AbbreviationDictionaryControl : public Control
{
	public:
		void ParseVolumeRecord(std::string);
		void EmptyVectors();

		DataFormatDictionaryList             dfd;
		CommentDescriptionList               cd;
		CitedSourceDictionaryList            csd;
		GenericAbbreviationList              ga;
		UnitsAbbreviationsList               ua;
		BeamConfigurationList                bc;
		FIRDictionaryList                    fird;
		ResponsePolynomialDictionaryList     rpd;
		ResponsePolesZerosDictionaryList     rpzd;
		ResponseCoefficientsDictionaryList   rcd;
		ResponseListDictionaryList           rld;
		GenericResponseDictionaryList        grd;
		DecimationDictionaryList             dd;
		ChannelSensitivityGainDictionaryList csgd;
};

// definition of all Station Control Headers blockettes
DECLARE_BLOCKETTE(Comment) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetCommentCodeKey(){return comment_code_key;}
		int GetCommentLevel(){return comment_level;}
		std::string GetBeginningEffectiveTime(){return beginning_effective_time;}
		std::string GetEndEffectiveTime(){return end_effective_time;}
	protected:
	private:
		std::string	beginning_effective_time, end_effective_time;
		int 	comment_code_key, comment_level;
};


class ResponseReferenceStage
{
	public:
		int GetStageSequenceNumber() const {return stage_sequence_number;}
		int GetNumberOfResponses() const {return number_of_responses;}
		const std::vector<int> &GetResponseLookupKey() const {return response_lookup_key;}
	protected:
	private:
		int stage_sequence_number;
		int number_of_responses;
		std::vector<int> response_lookup_key;
	friend class ResponseReference;
};

DECLARE_BLOCKETTE(ResponseReference) {
	public:
		virtual ParseResult Parse(std::string record);

		int GetNumberOfStages() const {return number_of_stages;}
		const std::vector<ResponseReferenceStage> &GetStages() const {return stages;}
	protected:
	private:
		int number_of_stages;
		std::vector<ResponseReferenceStage> stages;
};

DECLARE_BLOCKETTE(ChannelIdentifier) {
	public:
		virtual ParseResult Parse(std::string record);

		void EmptyVectors();

		ResponsePolesZerosList             rpz;
		ResponseCoefficientsList           rc;
		ResponseListList                   rl;
		GenericResponseList                gr;
		DecimationList                     dec;
		ChannelSensitivityGainList         csg;
		CommentList                        cc;
		ResponseReferenceList              rr;
		FIRResponseList                    firr;
		ResponsePolynomialList             rp;

		char GetUpdateFlag(){return update_flag;}
		int GetSubchannel(){return subchannel;}
		int GetInstrument(){return instrument;}
		int GetDataRecordLength(){return data_record_length;}
		int GetNumberOfComments(){return number_of_comments;}
		int GetDataFormatIdentifierCode(){return data_format_identifier_code;}
		int GetUnitsOfSignalResponse(){return units_of_signal_response;}
		int GetUnitsOfCalibrationInput(){return units_of_calibration_input;}
		double GetLatitude(){return latitude;}
		double GetLongitude(){return longitude;}
		double GetElevation(){return elevation;}
		double GetLocalDepth(){return local_depth;}
		double GetAzimuth(){return azimuth;}
		double GetDip(){return dip;}
		double GetSampleRate(){return sample_rate;}
		double GetMaxClockDrift(){return max_clock_drift;}
		double GetMinimumInputDecimationSampleRate() const;
		double GetMaximumInputDecimationSampleRate() const;
		std::string GetLocation(){return location;}
		std::string GetChannel(){return channel;}
		std::string GetOptionalComment(){return optional_comment;}
		std::string GetFlags(){return flags;}
		std::string GetStartDate(){return start_date;}
		std::string GetEndDate(){return end_date;}
	protected:
	private:
		char	update_flag;
		int	subchannel, instrument, data_record_length, number_of_comments;
		int	data_format_identifier_code, units_of_signal_response, units_of_calibration_input;
		double 	latitude, longitude, elevation, local_depth, azimuth, dip;
		double	sample_rate, max_clock_drift;
		std::string	location, channel;
		std::string	optional_comment, flags, start_date, end_date;
};

DECLARE_BLOCKETTE(StationIdentifier) {
	public:
		virtual ParseResult Parse(std::string record);

		void EmptyVectors();
		CommentList           sc;
		ChannelIdentifierList ci;

		std::string GetStationCallLetters(){return station_call_letters;}
		std::string GetNetworkCode(){return network_code;}
		std::string GetSiteName(){return site_name;}
		std::string GetCity(){return city;}
		std::string GetProvince(){return province;}
		std::string GetCountry(){return country;}
		std::string GetStartDate(){return start_date;}
		std::string GetEndDate(){return end_date;}
		double GetLatitude(){return latitude;}
		double GetLongitude(){return longitude;}
		double GetElevation(){return elevation;}
		int GetWordOrder(){return word_order;}
		int GetShortOrder(){return short_order;}
		int GetNumberOfChannels(){return number_of_channels;}
		int GetNumberOfStationComments(){return number_of_station_comments;}
		int GetNetworkIdentifierCode(){return network_identifier_code;}
	protected:
	private:
		double 	latitude, longitude, elevation;
		int	word_order, short_order, number_of_channels, number_of_station_comments;
		int	network_identifier_code;
		char	update_flag;
		std::string	station_call_letters, network_code;
		std::string	site_name, city, province, country, start_date, end_date;
};

class StationControl : public Control
{
	public:
		void ParseVolumeRecord(std::string);
		void EmptyVectors();
		// Reading is finished, do post processing such as merging responses
		void Flush();

		StationIdentifierList si;

	private:
		Logging log;
		int last_blockette, rest_size, eos, eoc;
		std::string record_remains;
		int AddChannelToStation(ChannelIdentifierPtr);
};
#endif // SEED_H
