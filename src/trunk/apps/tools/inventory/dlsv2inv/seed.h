/*==============================================================================
    Name:       seed.h

    Purpose:    define all seed control headers and the underlying blockettes

    Language:   C++, ANSI standard.

    Author:     Peter de Boer

    Revision:   0.1 initial  04-01-2008

==============================================================================*/
#ifndef SEED_H
#define SEED_H

#include "mystring.h"
#include "tmanip.h"
#include "define.h"

class Control
{
	public:
		void SetRemains(const std::string&);
		void SetRemains(const std::string&, int);
		std::string GetRemains();
		bool SetBytesLeft(int, const std::string&);
		int GetBytesLeft();
		void SetBytes(int);
	protected:
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
	protected:
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
	protected:
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
	protected:
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
		std::vector<TimeSpan> GetTimespans(){return timespans;}
	protected:
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
	protected:
	private:
		Logging *log;
};

// definition of all Abbreviation Dictionary Control Headers blockettes
class DataFormatDictionary
{
	public:
		DataFormatDictionary(std::string);
		int GetDataFormatIdentifierCode(){return data_format_identifier_code;}
		int GetDataFamilyType(){return data_family_type;}
		int GetNumberOfDecoderKeys(){return number_of_decoder_keys;}
		std::string GetShortDescriptiveName(){return short_descriptive_name;}
		std::vector<std::string> GetDecoderKeys(){return decoder_keys;}
	protected:
	private:
		int		data_format_identifier_code, data_family_type, number_of_decoder_keys;
		std::string	short_descriptive_name;
		std::vector<std::string> 	decoder_keys;
};

class CommentDescription
{
	public:
		CommentDescription(std::string);
		int GetCommentCodeKey(){return comment_code_key;}
		int GetUnits(){return units;}
		char GetCommentCodeClass(){return comment_class_code;}
		std::string GetDescriptionOfComment(){return description_of_comment;}
	protected:
	private:
		int		comment_code_key, units;
		char		comment_class_code;
		std::string	description_of_comment;
};

class CitedSourceDictionary
{
	public:
		CitedSourceDictionary(std::string);
		int GetSourceLookupCode(){return source_lookup_code;}
		std::string GetNumberOfPublication(){return name_of_publication;}
		std::string GetDatePublished(){return date_published;}
		std::string GetPublisherName(){return publisher_name;}
	protected:
	private:
		int		source_lookup_code;
		std::string	name_of_publication, date_published, publisher_name;
};

class GenericAbbreviation
{
	public:	
		GenericAbbreviation(std::string);
		int GetLookup(){return lookup_code;}
		std::string GetDescription(){return description;}
	protected:
	private:
		int		lookup_code;
		std::string	description;
};

class UnitsAbbreviations
{
	public:
		UnitsAbbreviations(std::string);
		int GetLookup(){return lookup_code;}
		std::string GetName(){return name;}
		std::string GetDescription(){return description;}
	protected:
	private:
		int		lookup_code;
		std::string	name, description;
};

struct Component
{
	std::string	station, location, channel, subchannel, component_weight;
};

class BeamConfiguration
{
	public:
		BeamConfiguration(std::string);
		int GetLookup(){return lookup_code;}
		int GetNumberOfComponents(){return number_of_components;}
		std::vector<Component> GetComponents(){return components;}
	protected:
	private:
		int	lookup_code, number_of_components;
		std::vector<Component> 	components;
};

class FIRDictionary
{
	public:
		FIRDictionary(std::string);
		int GetLookup(){return lookup_key;}
		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfFactors(){return number_of_factors;}
		char GetSymmetryCode(){return symmetry_code;}
		std::string GetName(){return name;}
		std::vector<double> GetCoefficients(){return coefficients;}
	protected:
	private:
		int     lookup_key, signal_in_units, signal_out_units, number_of_factors;
		char	symmetry_code;
		std::string     name;
		std::vector<double>	coefficients;
};

struct Coefficient
{
	double coefficient, error;
};

class ResponsePolynomialDictionary
{
	public:
		ResponsePolynomialDictionary(std::string);
		int GetLookup(){return lookup_key;}
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
		std::string GetName(){return name;}
		std::vector<Coefficient> GetPolynomialCoefficients(){return polynomial_coefficients;}
	protected:
	private:
		int     lookup_key, signal_in_units, signal_out_units, number_of_pcoeff;
		char	transfer_function_type, polynomial_approximation_type, valid_frequency_units;
		double	lower_valid_frequency_bound, upper_valid_frequency_bound;
		double	lower_bound_of_approximation, upper_bound_of_approximation;
		double	maximum_absolute_error;
		std::string     name;
		std::vector<Coefficient> polynomial_coefficients;
};

struct Zeros
{
	double real_zero, imaginary_zero, real_zero_error, imaginary_zero_error;
};

struct Poles
{
	double real_pole, imaginary_pole, real_pole_error, imaginary_pole_error;
}; 

class ResponsePolesZerosDictionary
{
	public:
		ResponsePolesZerosDictionary(std::string);
		int GetLookup(){return lookup_key;}
		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfPoles(){return number_of_poles;}
		int GetNumberOfZeros(){return number_of_zeros;}
		char GetResponseType(){return response_type;}
		double GetAoNormalizationFactor(){return ao_normalization_factor;}
		double GetNormalizationFrequency(){return normalization_frequency;}
		std::string GetName(){return name;}
		std::vector<Zeros> GetComplexZeros(){return complex_zeros;}
		std::vector<Poles> GetComplexPoles(){return complex_poles;}
	protected:
	private:
		int     lookup_key, signal_in_units, signal_out_units, number_of_zeros, number_of_poles;
		char	response_type;
		double	ao_normalization_factor, normalization_frequency;
		std::string     name;
		std::vector<Zeros> complex_zeros;
		std::vector<Poles> complex_poles;
};

class ResponseCoefficientsDictionary
{
	public:
		ResponseCoefficientsDictionary(std::string);
		int GetLookup(){return lookup_key;}
		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfNumerators(){return number_of_numerators;}
		int GetNumberOfDenominators(){return number_of_denominators;}
		char GetResponseType(){return response_type;}
		std::string GetName(){return name;}
		std::vector<Coefficient> GetNumerators(){return numerators;}
		std::vector<Coefficient> GetDenominators(){return denominators;}
	protected:
	private:
		int	lookup_key, signal_in_units, signal_out_units, number_of_numerators, number_of_denominators;
		char	response_type;
		std::string     name;
		std::vector<Coefficient> numerators;
		std::vector<Coefficient> denominators;
};

struct ListedResponses
{
	double frequency, amplitude, amplitude_error, phase_angle, phase_error;
};

class ResponseListDictionary
{
	public:
		ResponseListDictionary(std::string);
		int GetLookup(){return lookup_key;}
		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfResponses(){return number_of_responses;}
		std::string GetName(){return name;}
		std::vector<ListedResponses> GetResponsesListed(){return responses_listed;}
	protected:
 	private:
		int             lookup_key, signal_in_units, signal_out_units, number_of_responses;
		std::string     name;
		std::vector<ListedResponses> responses_listed;
};

struct CornerList
{
	double frequency, slope;
};

class GenericResponseDictionary
{
	public:
		GenericResponseDictionary(std::string);
		int GetLookup(){return lookup_key;}
		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfCorners(){return number_of_corners;}
		std::string GetName(){return name;}
		std::vector<CornerList> GetCornersListed(){return corners_listed;}
	protected:
	private:
		int             lookup_key, signal_in_units, signal_out_units, number_of_corners;
		std::string     name;
		std::vector<CornerList> corners_listed;
};

class DecimationDictionary
{
	public:
		DecimationDictionary(std::string);
		int GetLookup(){return lookup_key;}
		int GetDecimationFactor(){return decimation_factor;}
		int GetDecimationOffset(){return decimation_offset;}
		double GetInputSampleRate(){return input_sample_rate;}
		double GetEstimatedDelay(){return estimated_delay;}
		double GetCorrectionApplied(){return correction_applied;}
		std::string GetName(){return name;}
	protected:
	private:
		int	lookup_key, decimation_factor, decimation_offset;
		double	input_sample_rate, estimated_delay, correction_applied;
		std::string 	name;
};

struct HistoryValues
{
	double	sensitivity_for_calibration, frequency_of_calibration_sensitivity;
	std::string	time_of_calibration; 
};

class ChannelSensitivityGainDictionary
{
	public:
		ChannelSensitivityGainDictionary(std::string);
		int GetLookup(){return lookup_key;}
		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfHistoryValues(){return number_of_history_values;}
		double GetSensitivityGain(){return sensitivity_gain;}
		double GetFrequency(){return frequency;}
		std::string GetName(){return name;}
		std::vector<HistoryValues> GetHistoryValues(){return history_values;}
	protected:
	private:
		int     lookup_key, signal_in_units, signal_out_units, number_of_history_values;
		double	sensitivity_gain, frequency;
		std::string     name;
		std::vector<HistoryValues> history_values;
};

class AbbreviationDictionaryControl : public Control
{
	public:
		void ParseVolumeRecord(std::string);
		void EmptyVectors();
		std::vector<DataFormatDictionary>		dfd;
		std::vector<CommentDescription>			cd;
		std::vector<CitedSourceDictionary>		csd;
		std::vector<GenericAbbreviation>		ga;
		std::vector<UnitsAbbreviations>			ua;
		std::vector<BeamConfiguration>			bc;
		std::vector<FIRDictionary>			fird;
		std::vector<ResponsePolynomialDictionary>	rpd;
		std::vector<ResponsePolesZerosDictionary>	rpzd;
		std::vector<ResponseCoefficientsDictionary>	rcd;
		std::vector<ResponseListDictionary>		rld;
		std::vector<GenericResponseDictionary>		grd;
		std::vector<DecimationDictionary>		dd;
		std::vector<ChannelSensitivityGainDictionary>	csgd;
	protected:
	private: 
		Logging *log;
};

// definition of all Station Control Headers blockettes
class Comment
{
	public:
		Comment(std::string);
		int GetCommentCodeKey(){return comment_code_key;}
		int GetCommentLevel(){return comment_level;}
		std::string GetBeginningEffectiveTime(){return beginning_effective_time;}
		std::string GetEndEffectiveTime(){return end_effective_time;}
	protected:
	private:
		std::string	beginning_effective_time, end_effective_time;	
		int 	comment_code_key, comment_level;
};

class ResponsePolesZeros
{
	public:
		ResponsePolesZeros(std::string);
		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfPoles(){return number_of_poles;}
		int GetNumberOfZeros(){return number_of_zeros;}
		int GetStageSequenceNumber(){return stage_sequence_number;}
		char GetTransferFunctionType(){return transfer_function_type;}
		double GetAoNormalizationFactor(){return ao_normalization_factor;}
		double GetNormalizationFrequency(){return normalization_frequency;}
		std::string GetComplexZeros();
		std::string GetComplexPoles();
	protected:
	private:
		char 	transfer_function_type;
		int		number_of_zeros, number_of_poles, stage_sequence_number;
		int		signal_in_units, signal_out_units;
		double	ao_normalization_factor, normalization_frequency;
		std::vector<Zeros> complex_zeros;
		std::vector<Poles> complex_poles;
};

class ResponseCoefficients
{
	public:
		ResponseCoefficients(std::string);
		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfNumerators(){return number_of_numerators;}
		int GetNumberOfDenominators(){return number_of_denominators;}
		int GetStageSequenceNumber(){return stage_sequence_number;}
		char GetResponseType(){return response_type;}
		std::string GetNumerators();
		std::string GetDenominators();
	protected:
	private:
		int	number_of_numerators, number_of_denominators, stage_sequence_number;
		int 	signal_in_units, signal_out_units;
		char	response_type;
		std::vector<Coefficient> numerators;
		std::vector<Coefficient> denominators;
};

class ResponseList
{
	public:
		ResponseList(std::string);
		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfResponses(){return number_of_responses;}
		int GetStageSequenceNumber(){return stage_sequence_number;}
		std::vector<ListedResponses> GetResponsesListed(){return responses_listed;}
	protected:
 	private:
		int	stage_sequence_number, number_of_responses;
		int	signal_in_units, signal_out_units;
		std::vector<ListedResponses> responses_listed;
};

class GenericResponse
{
	public:
		GenericResponse(std::string);
		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfCorners(){return number_of_corners;}
		int GetStageSequenceNumber(){return stage_sequence_number;}
		std::vector<CornerList> GetCornersListed(){return corners_listed;}
	protected:
	private:
		int 	stage_sequence_number, number_of_corners;
		int 	signal_in_units, signal_out_units;
		std::vector<CornerList> corners_listed;
};

class Decimation
{
	public:
		Decimation(std::string);
		int GetDecimationFactor(){return factor;}
		int GetDecimationOffset(){return offset;}
		int GetStageSequenceNumber(){return stage_sequence_number;}
		double GetInputSampleRate(){return input_sample_rate;}
		double GetEstimatedDelay(){return estimated_delay;}
		double GetCorrectionApplied(){return correction_applied;}
	protected:
	private:
		int	stage_sequence_number, factor, offset;
		double	input_sample_rate, estimated_delay, correction_applied;
};

class ChannelSensitivityGain
{
	public:	
		ChannelSensitivityGain(std::string);
		int GetNumberOfHistoryValues(){return number_of_history_values;}
		int GetStageSequenceNumber(){return stage_sequence_number;}
		double GetSensitivityGain(){return sensitivity_gain;}
		double GetFrequency(){return frequency;}
		std::vector<HistoryValues> GetHistoryValues(){return history_values;}
	protected:
	private:
		int 	stage_sequence_number, number_of_history_values;
		double	sensitivity_gain, frequency;
		std::vector<HistoryValues> history_values;
};

class ResponseReference
{
	public:
		ResponseReference(std::string);
		int GetNumberOfStages(){return number_of_stages;}
		int GetNumberOfResponses(){return number_of_responses;}
		std::vector<int> GetStageSequenceNumber(){return stage_sequence_number;}
		std::vector<int> GetResponseLookupKey(){return response_lookup_key;}
	protected:
	private:
		int number_of_stages, number_of_responses;
		std::vector<int> stage_sequence_number, response_lookup_key;
};

class FIRResponse
{
	public:
		FIRResponse(std::string);
		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfCoefficients(){return number_of_coefficients;}
		int GetStageSequenceNumber(){return stage_sequence_number;}
		char GetSymmetryCode(){return symmetry_code;}
		std::string GetName(){return response_name;}
		std::string GetCoefficients();
	protected:
	private:
		int	stage_sequence_number, number_of_coefficients;
		int 	signal_in_units, signal_out_units;
		char	symmetry_code;
		std::string	response_name;
		std::vector<double>	coefficients;
};

class ResponsePolynomial
{
	public:
		ResponsePolynomial(std::string);
		int GetSignalInUnits(){return signal_in_units;}
		int GetSignalOutUnits(){return signal_out_units;}
		int GetNumberOfPcoeff(){return number_of_pcoeff;}
		int GetStageSequenceNumber(){return stage_sequence_number;}
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
	private:
		int	stage_sequence_number, number_of_pcoeff;
		int 	signal_in_units, signal_out_units;
		char	transfer_function_type, polynomial_approximation_type, valid_frequency_units;
		double	lower_valid_frequency_bound, upper_valid_frequency_bound;
		double	lower_bound_of_approximation, upper_bound_of_approximation, maximum_absolute_error;
		std::vector<Coefficient> polynomial_coefficients;
};

class ChannelIdentifier
{
	public:
		ChannelIdentifier(std::string);
		void EmptyVectors();
		std::vector<ResponsePolesZeros>		rpz;
		std::vector<ResponseCoefficients>	rc;
		std::vector<ResponseList>		rl;
		std::vector<GenericResponse>		gr;
		std::vector<Decimation>			dec;
		std::vector<ChannelSensitivityGain>	csg;
		std::vector<Comment>			cc;
		std::vector<ResponseReference>		rr;
		std::vector<FIRResponse>		firr;
		std::vector<ResponsePolynomial>		rp;
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

class StationIdentifier
{	
	public:
		StationIdentifier(std::string);
		void EmptyVectors();
		std::vector<Comment>		sc;
		std::vector<ChannelIdentifier>	ci;
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
		std::vector<StationIdentifier> si;
 	protected:
	private:
		Logging log;
		int last_blockette, rest_size, eos, eoc;
		std::string record_remains;
		int AddChannelToStation(ChannelIdentifier);
};
#endif // SEED_H
