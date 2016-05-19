/*==============================================================================

Purpose:    parsing seed files to file SeisComp database

Language:   C++, ANSI standard.

Author:     Peter de Boer, Andres Heinloo, Jan Becker

Revision:   17-07-2013

==============================================================================*/
#include "seed.h"
#include <assert.h>
#define SEISCOMP_COMPONENT sync_dlsv
#include <seiscomp3/logging/log.h>

using namespace std;

/****************************************************************************************************************************
* Function:     FromJulianDay												    *
* Parameters:   date	- date in year/julian_day format								    *
* Returns:      date in year/month/day format										    *
****************************************************************************************************************************/
string FromJulianDay(string date)
{
	long jaar = 0, maand = 0, dag = 0;
	int jan, feb, mrt, apr, mei, jun, jul, aug, sep, okt, nov, dec;
	int pos1=0, pos2;

	jaar = FromString<long>(SplitString(date, ',', pos1, pos2));
	pos1 = ++pos2;
	dag = FromString<long>(SplitString(date, ',', pos1, pos2));

	// start initializing days of each month
	if((jaar % 4)==0)
	{
		if(jaar % 400)
			feb = 29;
		else if(jaar % 100)
			feb = 28;
		else
			feb = 29;
	}
	else
		feb = 28;

	jan = mrt = mei = jul = aug = okt = dec = 31;
	apr = jun = sep = nov = 30;

	if((dag -= jan)<=0)
	{
		maand = 1;
		dag += jan;
	}
	else if((dag -= feb)<=0)
	{
		maand = 2;
		dag += feb;
	}
	else if((dag -= mrt)<=0)
	{
		maand = 3;
		dag += mrt;
	}
	else if((dag -= apr)<=0)
	{
		maand = 4;
		dag += apr;
	}
	else if((dag -= mei)<=0)
	{
		maand = 5;
		dag += mei;
	}
	else if((dag -= jun)<=0)
	{
		maand = 6;
		dag += jun;
	}
	else if((dag -= jul)<=0)
	{
		maand = 7;
		dag += jul;
	}
	else if((dag -= aug)<=0)
	{
		maand = 8;
		dag += aug;
	}
	else if((dag -= sep)<=0)
	{
		maand = 9;
		dag += sep;
	}
	else if((dag -= okt)<=0)
	{
		maand = 10;
		dag += okt;
	}
	else if((dag -= nov)<=0)
	{
		maand = 11;
		dag += nov;
	}
	else if((dag -= dec)<=0)
	{
		maand = 12;
		dag += dec;
	}
	Date internal_date(jaar, maand, dag);
	stringstream jd;
	jd << internal_date << "T00:00:00.0000Z";
	return jd.str();
}


inline string substr(const string &str, size_t pos = 0, size_t len = string::npos)
{
	/*
	if ( pos >= str.size() ) {
		cerr << "substr(\"" << str << "\", " << pos << ", " << len << ") failed" << endl;
		assert(false);
	}
	*/
	return str.substr(pos, len);
}


namespace {

// Stores the type of the last blockette that was incomplete
int lastIncompleteBlockette = -1;

}


/****************************************************************************************************************************
* Function:     SetRemains												    *
* Parameters:   record	- remains of the record that has been processed							    *
* Returns:      nothing													    *
* Description:  save the remains of the current record to add to the new one						    *
****************************************************************************************************************************/
void Control::SetRemains(const string& record)
{
	remains_of_record = record;
}

/****************************************************************************************************************************
* Function:     SetRemains												    *
* Parameters:   record	- the buffer that contains the data that has been processed					    *
*		size	- number of bytes to be added to bytes_left							    *
* Returns:      nothing													    *
* Description:  gets the remains from record and saves it in remains_of_record						    *
****************************************************************************************************************************/
void Control::SetRemains(const string& record, int size)
{
	remains_of_record = substr(record, 0, (bytes_left+size));
}

/****************************************************************************************************************************
* Function:     GetRemains												    *
* Parameters:   none													    *
* Returns:      the remains of the previous record									    *
****************************************************************************************************************************/
string Control::GetRemains()
{
	return remains_of_record;
}

/****************************************************************************************************************************
* Function:     SetBytesLeft												    *
* Parameters:   blockette_size	- size of the blockette being processed							    *
* Returns:      true if blockette can be processed, false if else							    *
****************************************************************************************************************************/
bool Control::SetBytesLeft(int blockette_size, const string& record) {
	// check if last character is an ~, if so than add 1 to blockette_size
	if ( blockette_size < (int)record.size() && record[blockette_size] == '~' )
		blockette_size++;
	bytes_left -= blockette_size;
	if ( bytes_left >= 0 )
		return true;
	else
		return false;
}

/****************************************************************************************************************************
* Function:     SetBytes												    *
* Parameters:   size	- size to be set										    *
* Returns:      nothing													    *
* Description:  set the variable bytes_left to the initial size of the record that is processed				    *
****************************************************************************************************************************/
void Control::SetBytes(int size)
{
	bytes_left = size;
}

/****************************************************************************************************************************
* Function:     GetBytesLeft												    *
* Parameters:   none													    *
* Returns:      number of bytes that is left in the record								    *
****************************************************************************************************************************/
int Control::GetBytesLeft()
{
	return bytes_left;
}

/****************************************************************************************************************************
 * Function:     ParseVolumeRecord											    *
 * Parameters:   record	- the string that will be parsed								    *
 * Returns:      nothing												    *
 * Description:  splitting the record into defined blockettes								    *
 ****************************************************************************************************************************/
void VolumeIndexControl::ParseVolumeRecord(string record)
{
	unsigned int blockette, size;
	bool proceed = true;

	log = new Logging();
	SetBytes(LRECL-8);
	if(!GetRemains().empty())
	{
		record = GetRemains() + record;
		SetBytes(record.size());
		SetRemains("");
	}

	do
	{
		try
		{
			// get blockette number and size
			blockette = FromString<int>(substr(record, 0,3).c_str());
			size = FromString<int>(substr(record, 3, 4).c_str());

			if ( blockette == 0 ) break;

			// check if the complete blockette is within the current record
			if(SetBytesLeft(size, record))
			{
				switch(blockette)
				{
					case(5):
						fvi.push_back(FieldVolumeIdentifier(substr(record, 7, size)));
						break;
					case(8):
						tvi.push_back(TelemetryVolumeIdentifier(substr(record, 7, size)));
						break;
					case(10):
						vi.push_back(VolumeIdentifier(substr(record, 7, size)));
						break;
					case(11):
						vshi.push_back(VolumeStationHeaderIndex(substr(record, 7, size)));
						break;
					case(12):
						vtsi.push_back(VolumeTimeSpanIndex(substr(record, 7, size)));
						break;
					default:
						proceed = false;
						SEISCOMP_WARNING("Unhandled blockette: %d", blockette);
						break;
				}
				// must check if size of the record is smaller or equal to the size of the last blockette
				// else you'll get an segmentation fault
				if(record.size() <= size)
				{
					proceed = false;
				}
				else
				{
					// check if character on position is an ~,
					// if so than take substring with size+1
					if(record[size] == '~')
						record = substr(record, ++size);
					else
						record = substr(record, size);
				}
			}
			else
			{
				SetRemains(record, size);
				proceed = false;
			}
		}
		catch(BadConversion &o) {
			log->write(o.what());
			proceed = false;
		}
		catch ( std::out_of_range &o ) {
			log->write(string("VolumeIndexControl blockette: ") + o.what());
			SetBytes(0);
			SetRemains("");
			proceed = false;
			throw o;
		}

	}while(proceed);
}

/****************************************************************************************************************************
* Function:     EmptyVectors												    *
* Parameters:   none													    *
* Returns:      nothing													    *
* Description:  clear all vectors after the data has been written to inventory						    *
****************************************************************************************************************************/
void VolumeIndexControl::EmptyVectors()
{
	if(!fvi.empty())
		fvi.clear();
	if(!tvi.empty())
		tvi.clear();
	if(!vi.empty())
		vi.clear();
	if(!vshi.empty())
		vshi.clear();
	if(!vtsi.empty())
		vtsi.clear();
}

/****************************************************************************************************************************
* Function:     FieldVolumeIdentifier											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the FieldVolumeIdentifier									    *
****************************************************************************************************************************/
FieldVolumeIdentifier::FieldVolumeIdentifier(string record)
{
	version_of_format = FromString<float>(substr(record, 0, 4));
	logical_record_length = FromString<int>(substr(record, 4, 2));
	int pos1=6, pos2;
	beginning_of_volume = SplitString(record, SEED_SEPARATOR, pos1, pos2);
}

/****************************************************************************************************************************
* Function:     TelemetryVolumeIdentifier										    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the TelemetryVolumeIdentifier								    *
****************************************************************************************************************************/
TelemetryVolumeIdentifier::TelemetryVolumeIdentifier(string record)
{
	version_of_format = FromString<float>(substr(record, 0, 4));
	logical_record_length = FromString<int>(substr(record, 4, 2));
	station_identifier = substr(record, 6, 5);
	location_identifier = substr(record, 11, 2);
	channel_identifier = substr(record, 13, 3);
	int pos1=16, pos2;
	beginning_of_volume = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	end_of_volume = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	station_information_effective_date = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	channel_information_effective_date = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	network_code = substr(record, ++pos2, 2);
}

/****************************************************************************************************************************
* Function:     VolumeIdentifier											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the VolumeIdentifier										    *
****************************************************************************************************************************/
VolumeIdentifier::VolumeIdentifier(string record)
{
	version_of_format = FromString<float>(substr(record, 0, 4));
	logical_record_length = FromString<int>(substr(record, 4, 2));
	int pos1=6, pos2;
	beginning_time = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	end_time = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	volume_time = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	originating_organization = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	label = SplitString(record, SEED_SEPARATOR, pos1, pos2);
}

/****************************************************************************************************************************
* Function:     VolumeStationHeaderIndex										    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the VolumeStationHeaderIndex									    *
****************************************************************************************************************************/
VolumeStationHeaderIndex::VolumeStationHeaderIndex(string record)
{
	number_of_stations = FromString<int>(substr(record, 0, 3));
	int pos1=3, pos2=8;
	for ( int i = 0; i < number_of_stations; ++i ) {
		station_info[substr(record, pos1, 5)] = FromString<int>(substr(record, pos2, 6));
		pos1 += 11;
 		pos2 += 11;
	}
}

/****************************************************************************************************************************
* Function:     VolumeTimeSpanIndex											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the VolumeTimeSpanIndex									    *
****************************************************************************************************************************/
VolumeTimeSpanIndex::VolumeTimeSpanIndex(string record)
{
	number_of_spans = FromString<int>(substr(record, 0, 4));
	if ( number_of_spans == 0 ) return;
	int pos1=4, pos2;
	TimeSpan ts;
 	ts.beginning_of_span = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	ts.end_of_span = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	ts.sequence_number = FromString<int>(substr(record, ++pos2, 6));
	timespans.push_back(ts);
}


template <typename T, typename C>
bool Parse(int blockette, C &container, bool merge, std::string record) {
	ParseResult res = PR_OK;

	if ( merge ) {
		SEISCOMP_DEBUG("Blockette %d: continuation", blockette);
		res = container.back()->Merge(record);
	}
	else {
		typename Seiscomp::Core::SmartPointer<T>::Impl rec = new T;
		res = rec->Parse(record);
		if ( res == PR_Error ) {
			SEISCOMP_ERROR("Blockette %d: Parse error: %s", blockette, record.c_str());
			return false;
		}

		container.push_back(rec);
	}

	if ( res == PR_Partial ) {
		SEISCOMP_DEBUG("Blockette %d: requires more data, try to continue on next record", blockette);
		lastIncompleteBlockette = blockette;
	}
	else
		lastIncompleteBlockette = -1;

	return res != PR_Error;
}

#define PARSE(TYPE, CONTAINTER) \
	Parse<TYPE>(blockette, CONTAINTER, lastIncompleteBlockette == blockette, substr(record, 7, data_size))


/****************************************************************************************************************************
 * Function:     ParseVolumeRecord											    *
 * Parameters:   record	- the string that will be parsed								    *
 * Returns:      nothing												    *
 * Description:  splitting the record into defined blockettes								    *
 ****************************************************************************************************************************/
void AbbreviationDictionaryControl::ParseVolumeRecord(string record)
{
	int blockette;
	size_t size, position = 0;
	bool proceed = true;

	SetBytes(LRECL-8);
	if ( !GetRemains().empty() ) {
		record = GetRemains() + record;
		SetBytes(record.size());
		SetRemains("");
	}

	do {
		try {
			blockette = FromString<int>(substr(record, 0,3).c_str());
			size = FromString<int>(substr(record, 3, 4).c_str());
			//SEISCOMP_DEBUG("Read blockette %d %d", blockette, (int)size);

			if ( SetBytesLeft(size, record) ) {
				size_t data_size = size-7;

				switch ( blockette ) {
					case(30):
						PARSE(DataFormatDictionary, dfd);
						break;
					case(31):
						PARSE(CommentDescription, cd);
						break;
					case(32):
						PARSE(CitedSourceDictionary, csd);
						break;
					case(33):
						PARSE(GenericAbbreviation, ga);
						break;
					case(34):
						PARSE(UnitsAbbreviations, ua);
						break;
					case(35):
						PARSE(BeamConfiguration, bc);
						break;
					case(41):
						PARSE(FIRDictionary, fird);
						break;
					case(42):
						PARSE(ResponsePolynomialDictionary, rpd);
						break;
					case(43):
						PARSE(ResponsePolesZerosDictionary, rpzd);
						break;
					case(44):
						PARSE(ResponseCoefficientsDictionary, rcd);
						break;
					case(45):
						PARSE(ResponseListDictionary, rld);
						break;
					case(46):
						PARSE(GenericResponseDictionary, grd);
						break;
					case(47):
						PARSE(DecimationDictionary, dd);
						break;
					case(48):
						PARSE(ChannelSensitivityGainDictionary, csgd);
						break;
					default:
						proceed = false;
						break;
				}

				// must check if size of the record is smaller or equal to the size of the last blockette
				// else you'll get an segmentation fault
				if ( record.size() <= size )
					proceed = false;
				else {
					// check if character on position is an ~,
					// if so than take substring with size+1
					if ( record[size] == '~' )
						record = substr(record, ++size);
					else
						record = substr(record, size);
					position += size;
				}
			}
			else {
				SetRemains(record, size);
				proceed = false;
			}
		}
		catch ( BadConversion &o ) {
			SEISCOMP_WARNING("%s", o.what());
			proceed = false;
		}
		catch ( std::out_of_range &o ) {
			SEISCOMP_WARNING("AbbreviationDictionaryControl volume: %s", o.what());
			SetBytes(0);
			SetRemains("");
			proceed = false;
			throw o;
		}
	}
	while ( proceed );
}

/****************************************************************************************************************************
* Function:     EmptyVectors												    *
* Parameters:   none													    *
* Returns:      nothing													    *
* Description:  clear all vectors after the data has been written to inventory						    *
****************************************************************************************************************************/
void AbbreviationDictionaryControl::EmptyVectors()
{
	if ( !dfd.empty() )
		dfd.clear();
	if ( !cd.empty() )
		cd.clear();
	if ( !csd.empty() )
		csd.clear();
	if ( !ga.empty() )
		ga.clear();
	if ( !ua.empty() )
		ua.clear();
	if ( !bc.empty() )
		bc.clear();
	if ( !fird.empty() )
		fird.clear();
	if ( !rpd.empty() )
		rpd.clear();
	if ( !rpzd.empty() )
		rpzd.clear();
	if ( !rcd.empty() )
		rcd.clear();
	if ( !rld.empty() )
		rld.clear();
	if ( !grd.empty() )
		grd.clear();
	if ( !dd.empty() )
		dd.clear();
	if ( !csgd.empty() )
		csgd.clear();
}

/****************************************************************************************************************************
* Function:     DataFormatDictionary											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the DataFormatDictionary									    *
****************************************************************************************************************************/
ParseResult DataFormatDictionary::Parse(string record) {
	int pos1=0, pos2;
	short_descriptive_name = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	data_format_identifier_code = FromString<int>(substr(record, pos1, 4));
	pos1 += 4;
	data_family_type = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	number_of_decoder_keys = FromString<int>(substr(record, pos1, 2));
	pos1 += 2;
	for ( int i = 0; i < number_of_decoder_keys; ++i ) {
		decoder_keys.push_back(SplitString(record, SEED_SEPARATOR, pos1, pos2));
		pos1 = ++pos2;
	}
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     CommentDescription											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the CommentDescription									    *
****************************************************************************************************************************/
ParseResult CommentDescription::Parse(string record) {
	comment_code_key = FromString<int>(substr(record, 0, 4));
	comment_class_code = record[4];
	int pos1=5, pos2;
	description_of_comment = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	units = FromString<int>(substr(record, ++pos2, 3));
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     CitedSourceDictionary											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the CitedSourceDictionary									    *
****************************************************************************************************************************/
ParseResult CitedSourceDictionary::Parse(string record) {
	source_lookup_code = FromString<int>(substr(record, 0, 2));
	int pos1=2, pos2;
	name_of_publication = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	date_published = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	publisher_name = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     GenericAbbreviation											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the GenericAbbreviation									    *
****************************************************************************************************************************/
ParseResult GenericAbbreviation::Parse(string record) {
	lookup_code = FromString<int>(substr(record, 0, 3));
	int pos1=3, pos2;
	description = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     UnitsAbbreviations											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the UnitsAbbrevations									    *
****************************************************************************************************************************/
ParseResult UnitsAbbreviations::Parse(string record) {
	lookup_code = FromString<int>(substr(record, 0, 3));
	int pos1=3, pos2;
	name = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	description = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     BeamConfiguration											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the BeamConfiguration									    *
****************************************************************************************************************************/
ParseResult BeamConfiguration::Parse(string record) {
	lookup_code = FromString<int>(substr(record, 0, 3));
	number_of_components = FromString<int>(substr(record, 3, 4));
	int pos1=7;
	Component comp;
	for ( int i = 0; i < number_of_components; ++i ) {
		comp.station = substr(record, pos1, 5);
		pos1 += 5;
		comp.location = substr(record, pos1, 2);
		pos1 += 2;
		comp.channel = substr(record, pos1, 3);
		pos1 += 3;
		comp.subchannel = FromString<int>(substr(record, pos1, 4));
		pos1 += 4;
		comp.component_weight = FromString<int>(substr(record, pos1, 5));
		pos1 += 5;
		components.push_back(comp);
 	}
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     FIRDictionary												    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the FIRDictionary										    *
****************************************************************************************************************************/
ParseResult FIRDictionary::Parse(std::string record)
{
	lookup_key = FromString<int>(substr(record, 0, 4));
	int pos1=4, pos2;
	response_name = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;

	// Not merging
	if ( coefficients.empty() ) {
		symmetry_code = record[pos1++];
		signal_in_units = FromString<int>(substr(record, pos1, 3));
		pos1 += 3;
		signal_out_units = FromString<int>(substr(record, pos1, 3));
		pos1 += 3;
		number_of_coefficients = FromString<int>(substr(record, pos1, 4));
		pos1 += 4;
	}
	else
		pos1 += 3+3+4;

	int max_factors = (record.size()-pos1)/14;
	if ( max_factors > number_of_coefficients ) {
		SEISCOMP_WARNING("Blockette 41: size larger than specified content");
		max_factors = number_of_coefficients;
	}

	for ( int i = 0; i < max_factors; ++i ) {
		coefficients.push_back(FromString<double>(substr(record, pos1, 14)));
		pos1 += 14;
	}

	if ( (int)coefficients.size() < number_of_coefficients )
		return PR_Partial;

	return PR_OK;
}


ParseResult FIRDictionary::Merge(string record)
{
	return Parse(record);
}


/****************************************************************************************************************************
* Function:     ResponsePolynomialDictionary										    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the ResponsePolynomialDictionary								    *
****************************************************************************************************************************/
ParseResult ResponsePolynomialDictionary::Parse(string record) {
	lookup_key = FromString<int>(substr(record, 0, 4));
	int pos1=4, pos2;
	name = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	transfer_function_type  = record[pos1++];
	signal_in_units = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	signal_out_units = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	polynomial_approximation_type  = record[pos1++];
	valid_frequency_units  = record[pos1++];
	lower_valid_frequency_bound = FromString<double>(substr(record, pos1, 12));
	pos1 += 12;
	upper_valid_frequency_bound = FromString<double>(substr(record, pos1, 12));
	pos1 += 12;
	lower_bound_of_approximation = FromString<double>(substr(record, pos1, 12));
	pos1 += 12;
	upper_bound_of_approximation = FromString<double>(substr(record, pos1, 12));
	pos1 += 12;
	maximum_absolute_error = FromString<double>(substr(record, pos1, 12));
	pos1 += 12;
	number_of_pcoeff = FromString<int>(substr(record, pos1, 4));
	pos1 += 4;
	Coefficient pc;
	for ( int i = 0; i < number_of_pcoeff; ++i ) {
		pc.coefficient = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		pc.error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		polynomial_coefficients.push_back(pc);
	}
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     ResponsePolesZerosDictionary										    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the ResponsePolesZerosDictionary								    *
****************************************************************************************************************************/
ParseResult ResponsePolesZerosDictionary::Parse(string record) {
	lookup_key = FromString<int>(substr(record, 0, 4));
	int pos1=4, pos2;
	name = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	transfer_function_type = record[pos1++];
	signal_in_units = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	signal_out_units = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	ao_normalization_factor = FromString<double>(substr(record, pos1, 12));
	pos1 += 12;
	normalization_frequency = FromString<double>(substr(record, pos1, 12));
	pos1 += 12;
	number_of_zeros = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	Zeros cz;
	for ( int i = 0; i < number_of_zeros; ++i ) {
		cz.real_zero = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		cz.imaginary_zero = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		cz.real_zero_error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		cz.imaginary_zero_error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		complex_zeros.push_back(cz);
	}
	number_of_poles = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	Poles cp;
	for ( int i = 0; i < number_of_poles; ++i ) {
		cp.real_pole = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		cp.imaginary_pole = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		cp.real_pole_error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		cp.imaginary_pole_error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		complex_poles.push_back(cp);
	}
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     ResponseCoefficientsDictionary										    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the ResponseCoefficientsDictionary								    *
****************************************************************************************************************************/
ParseResult ResponseCoefficientsDictionary::Parse(string record) {
	lookup_key = FromString<int>(substr(record, 0, 4));
	int pos1=4, pos2;
	name = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	response_type  = record[pos1++];
	signal_in_units = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	signal_out_units = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	number_of_numerators = FromString<int>(substr(record, pos1, 4));
	pos1 += 4;
	Coefficient coeff;
	for ( int i = 0; i < number_of_numerators; ++i ) {
		coeff.coefficient = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		coeff.error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		numerators.push_back(coeff);
	}
	number_of_denominators = FromString<int>(substr(record, pos1, 4));
	pos1 += 4;
	for ( int i = 0; i < number_of_denominators; ++i ) {
		coeff.coefficient = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		coeff.error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		denominators.push_back(coeff);
	}
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     ResponseListDictionary											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the ResponseListDictionary									    *
****************************************************************************************************************************/
ParseResult ResponseListDictionary::Parse(string record) {
	lookup_key = FromString<int>(substr(record, 0, 4));
	int pos1=4, pos2;
	name = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	signal_in_units = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	signal_out_units = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	number_of_responses = FromString<int>(substr(record, pos1, 4));
	pos1 += 4;
	ListedResponses lr;
	for ( int i = 0; i < number_of_responses; ++i ) {
		lr.frequency = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		lr.amplitude= FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		lr.amplitude_error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		lr.phase_angle = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		lr.phase_error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		responses_listed.push_back(lr);
	}
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     GenericResponseDictionary										    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the GenericResponseDictionary								    *
****************************************************************************************************************************/
ParseResult GenericResponseDictionary::Parse(string record) {
	lookup_key = FromString<int>(substr(record, 0, 4));
	int pos1=4, pos2;
	name = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	signal_in_units = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	signal_out_units = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	number_of_corners = FromString<int>(substr(record, pos1, 4));
	pos1 += 4;
	CornerList cl;
	for(int i=0; i<number_of_corners; i++)
	{
		cl.frequency = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		cl.slope = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		corners_listed.push_back(cl);
	}
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     DecimationDictionary											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the DecimationDictionary									    *
****************************************************************************************************************************/
ParseResult DecimationDictionary::Parse(string record)
{
	lookup_key = FromString<int>(substr(record, 0, 4));
	int pos1=4, pos2;
	name = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	input_sample_rate = FromString<double>(substr(record, pos1, 10));
	pos1 += 10;
	factor = FromString<int>(substr(record, pos1, 5));
	pos1 += 5;
	offset = FromString<int>(substr(record, pos1, 5));
	pos1 += 5;
	estimated_delay = FromString<double>(substr(record, pos1, 11));
	pos1 += 11;
	correction_applied = FromString<double>(substr(record, pos1, 11));
	pos1 += 11;
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     ChannelSensitivityDictionary										    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the ChannelSensitivityDictionary								    *
****************************************************************************************************************************/
ParseResult ChannelSensitivityGainDictionary::Parse(string record)
{
	lookup_key = FromString<int>(substr(record, 0, 4));
	int pos1=4, pos2;
	name = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	sensitivity_gain = FromString<double>(substr(record, pos1, 12));
	pos1 += 12;
	frequency = FromString<double>(substr(record, pos1, 12));
	pos1 += 12;
	number_of_history_values = FromString<int>(substr(record, pos1, 2));
	pos1 += 2;
	HistoryValues hv;
	for(int i=0; i<number_of_history_values; i++)
	{
		hv.sensitivity_for_calibration = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		hv.frequency_of_calibration_sensitivity = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		hv.time_of_calibration = SplitString(record, SEED_SEPARATOR, pos1, pos2);
		pos1 = ++pos2;
		history_values.push_back(hv);
	}
	return PR_OK;
}

/****************************************************************************************************************************
 * Function:     ParseVolumeRecord											    *
 * Parameters:   record	- the string that will be parsed								    *
 * Returns:      nothing												    *
 * Description:  splitting the record into defined blockettes								    *
 ****************************************************************************************************************************/
void StationControl::ParseVolumeRecord(string record)
{
	int blockette;
	size_t size, position = 0;
	bool proceed = true;

	SetBytes(record.size());
	if(!GetRemains().empty())
	{
		record = GetRemains() + record;
 		SetBytes(record.size());
		SetRemains("");
	}

	do
	{
		try
		{
			int begin = 0;
			blockette = 0;
			while(blockette < 50 || blockette > 62)
			{
				blockette = FromString<int>(substr(record, begin++, 3).c_str());
				if ( begin >= (int)record.size() ) {
					SetBytes(0);
					SetRemains("");
					proceed = false;
					return;
				}
			}

			if ( !begin )
				begin += 3;
			else
				begin += 2;

			if ( !si.empty() ) {
				if(blockette < 52)
					eos = (int)si.size()-1;
				if(eos < (int)si.size() && !si[eos]->ci.empty()) {
					eoc = si[eos]->ci.size()-1;
				}
			}

			size = FromString<int>(substr(record, begin, 4).c_str());

			if ( SetBytesLeft(size, record) ) {
				begin += 4;

				if ( blockette > 50 && eos == -1 )
					log.write("received blockette " + ToString(blockette) + ", but no station info available");
				else if ( blockette > 52 && eoc == -1 )
					log.write("received blockette " + ToString(blockette) + ", but no channel info available");
				else if ( blockette > 52 && si[eos]->ci.empty() )
					log.write("ignoring channel blockette " + ToString(blockette) + ", station has no channels");
				else {
					size_t data_size = size-begin;
					switch ( blockette ) {
						case(50):
							PARSE(StationIdentifier, si);
							break;
						case(51):
							PARSE(Comment, si[eos]->sc);
							break;
						case(52):
						{
							ChannelIdentifierPtr ci = new ChannelIdentifier;
							if ( ci->Parse(substr(record, begin, data_size)) == PR_OK )
								eos = AddChannelToStation(ci);
	//						PARSE(ChannelIdentifier, si[eos].ci);
							break;
						}
						case(53):
							PARSE(ResponsePolesZeros, si[eos]->ci[eoc]->rpz);
							break;
						case(54):
							PARSE(ResponseCoefficients, si[eos]->ci[eoc]->rc);
							break;
						case(55):
							PARSE(ResponseList, si[eos]->ci[eoc]->rl);
							break;
						case(56):
							PARSE(GenericResponse, si[eos]->ci[eoc]->gr);
							break;
						case(57):
							PARSE(Decimation, si[eos]->ci[eoc]->dec);
							break;
						case(58):
							PARSE(ChannelSensitivityGain, si[eos]->ci[eoc]->csg);
							break;
						case(59):
							PARSE(Comment, si[eos]->ci[eoc]->cc);
							break;
						case(60):
							PARSE(ResponseReference, si[eos]->ci[eoc]->rr);
							break;
						case(61):
							PARSE(FIRResponse, si[eos]->ci[eoc]->firr);
							break;
						case(62):
							PARSE(ResponsePolynomial, si[eos]->ci[eoc]->rp);
							break;
						default:
							SEISCOMP_WARNING("Unhandled blockette: %d", blockette);
							proceed = false;
							break;
					}
				}
				// must check if size of the record is smaller or equal to the size of the last blockette
				// else you'll get an segmentation fault
				if(record.size() <= size)
				{
					proceed = false;
				}
				else
				{
					// check if character on position is an ~,
					// if so than take substring with size+1
					if(begin!=7)
					{
						size += (begin-7);
					}
					if(record[size] == '~')
						record = substr(record, ++size);
					else
						record = substr(record, size);
					position += size;
				}
			}
			else
			{
				SetRemains(record, size);
				proceed = false;
			}
		}
		catch(BadConversion &o)
		{
			log.write(o.what());
			proceed = false;
		}
		catch ( std::out_of_range &o ) {
			log.write(string("StationControl blockette: ") + o.what());
			SetBytes(0);
			SetRemains("");
			proceed = false;
			throw o;
		}
	}while(proceed);
}

/****************************************************************************************************************************
* Function:     Flush                                                                                                       *
* Parameters:   None                                                                                                        *
* Returns:      Nothing                                                                                                     *
* Description:  Merges channel responses after reading has finished                                                         *
****************************************************************************************************************************/
void StationControl::Flush()
{
	for ( size_t eos = 0; eos < si.size(); ++eos ) {
		for ( size_t eoc = 0; eoc < si[eos]->ci.size(); ++eoc ) {
			ResponseListList &rl = si[eos]->ci[eoc]->rl;
			for ( size_t i = 1; i < rl.size(); ) {
				ResponseList *last = rl[i-1].get();
				ResponseList *curr = rl[i].get();

				int lastStage = last->GetStageSequenceNumber();
				int stage = curr->GetStageSequenceNumber();
				if ( stage == lastStage ) {
					// Merge records
					last->number_of_responses += curr->number_of_responses;
					last->responses_listed.insert(last->responses_listed.end(),
					                              curr->responses_listed.begin(),
					                              curr->responses_listed.end());
					rl.erase(rl.begin()+i);
				}
				else
					++i;
			}
		}
	}
}

/****************************************************************************************************************************
* Function:     AddChannelToStation											    *
* Parameters:   chan	ChannelIdentifier										    *
* Returns:      number of station in vector										    *
* Description:  						    *
****************************************************************************************************************************/
int StationControl::AddChannelToStation(ChannelIdentifierPtr chan)
{
	for(int i=si.size()-1; i>-1; i--)
	{
		if(chan->GetEndDate().empty() || chan->GetEndDate() > si[i]->GetStartDate())
		{
			if(si[i]->GetEndDate().empty() || si[i]->GetEndDate() > chan->GetStartDate())
			{
				si[i]->ci.push_back(chan);
				return i;
			}
		}
	}

	si[si.size()-1]->ci.push_back(chan);
	return si.size()-1;
}

/****************************************************************************************************************************
* Function:     EmptyVectors												    *
* Parameters:   none													    *
* Returns:      nothing													    *
* Description:  clear all vectors after the data has been written to inventory						    *
****************************************************************************************************************************/
void StationControl::EmptyVectors()
{
	for(size_t i=0; i<si.size(); i++)
		si[i]->EmptyVectors();
	si.clear();
}

/****************************************************************************************************************************
* Function:     StationIdentifier											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the StationIdentifier									    *
****************************************************************************************************************************/
ParseResult StationIdentifier::Parse(string record) {
	station_call_letters = substr(record, 0, 5);
	latitude = FromString<double>(substr(record, 5, 10));
	longitude = FromString<double>(substr(record, 15, 11));
	elevation = FromString<double>(substr(record, 26, 7));
	number_of_channels = FromString<int>(substr(record, 33, 4));
	number_of_station_comments = FromString<int>(substr(record, 37, 3));
	int pos1=40, pos2;
	site_name = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	network_identifier_code = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	word_order = FromString<int>(substr(record, pos1, 4));
	pos1 += 4;
	short_order = FromString<int>(substr(record, pos1, 2));
	pos1 += 2;
	start_date = SplitString(record, SEED_SEPARATOR, pos1, pos2);
//	if(start_date != "")
//		start_date = FromJulianDay(start_date);
	pos1 = ++pos2;
	end_date = SplitString(record, SEED_SEPARATOR, pos1, pos2);
//	if(end_date != "")
//		end_date = FromJulianDay(end_date);
	pos1 = ++pos2;
	update_flag = record[pos1++];
	network_code = substr(record, pos1, 2);
	pos1 = 0;
	city = SplitString(site_name, ',', pos1, pos2);
	if(pos2 < (int)site_name.size())
	{
		pos1 = ++pos2;
		country = SplitString(site_name, ',', pos1, pos2);
		if(pos2 < (int)site_name.size())
		{
			province = country;
			pos1 = ++pos2;
			country = SplitString(site_name, ',', pos1, pos2);
		}
	}
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     EmptyVectors												    *
* Parameters:   none													    *
* Returns:      nothing													    *
* Description:  clear all vectors after the data has been written to inventory						    *
****************************************************************************************************************************/
void StationIdentifier::EmptyVectors()
{
	sc.clear();
	for(size_t i=0; i<ci.size(); i++)
		ci[i]->EmptyVectors();
	ci.clear();
}

/****************************************************************************************************************************
* Function:     Comment													    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the Comment											    *
****************************************************************************************************************************/
ParseResult Comment::Parse(string record)
{
	int pos1=0, pos2;
	beginning_effective_time = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	end_effective_time = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	comment_code_key = FromString<int>(substr(record, pos1, 4));
	pos1 += 4;
	comment_level = FromString<int>(substr(record, pos1, 6));
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     ChannelIdentifier											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the ChannelIdentifier									    *
****************************************************************************************************************************/
ParseResult ChannelIdentifier::Parse(string record)
{
	location = substr(record, 0, 2);
	channel = substr(record, 2, 3);
	subchannel = FromString<int>(substr(record, 5, 4));
	instrument = FromString<int>(substr(record, 9, 3));
	int pos1=12, pos2;
	optional_comment = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	units_of_signal_response = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	units_of_calibration_input = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	latitude = FromString<double>(substr(record, pos1, 10));
	pos1 += 10;
	longitude = FromString<double>(substr(record, pos1, 11));
	pos1 += 11;
	elevation = FromString<double>(substr(record, pos1, 7));
	pos1 += 7;
	local_depth = FromString<double>(substr(record, pos1, 5));
	pos1 += 5;
	azimuth = FromString<double>(substr(record, pos1, 5));
	pos1 += 5;
	dip = FromString<double>(substr(record, pos1, 5));
	pos1 += 5;
	data_format_identifier_code = FromString<int>(substr(record, pos1, 4));
	pos1 += 4;
	data_record_length = FromString<int>(substr(record, pos1, 2));
	pos1 += 2;
	sample_rate = FromString<double>(substr(record, pos1, 10));
	pos1 += 10;
	max_clock_drift = FromString<double>(substr(record, pos1, 10));
	pos1 += 10;
	number_of_comments = FromString<int>(substr(record, pos1, 4));
	pos1 += 4;
	flags = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;
	start_date = SplitString(record, SEED_SEPARATOR, pos1, pos2);
//	if(start_date != "")
//		start_date = FromJulianDay(start_date);
	pos1 = ++pos2;
	end_date = SplitString(record, SEED_SEPARATOR, pos1, pos2);
//	if(end_date != "")
//		end_date = FromJulianDay(end_date);
	pos1 = ++pos2;
	update_flag = record[pos1];
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     EmptyVectors												    *
* Parameters:   none													    *
* Returns:      nothing													    *
* Description:  clear all vectors after the data has been written to inventory						    *
****************************************************************************************************************************/
void ChannelIdentifier::EmptyVectors()
{
	rpz.clear();
	rc.clear();
	rl.clear();
	gr.clear();
	dec.clear();
	csg.clear();
	cc.clear();
	rr.clear();
	firr.clear();
	rp.clear();
}

/*******************************************************************************
* Function:     GetMaximumInputDecimationSampleRate                            *
* Parameters:   none                                                           *
* Returns:      The maximum sample rate of all decimation stages or 0          *
* Description:  clear all vectors after the data has been written to inventory *
********************************************************************************/
double ChannelIdentifier::GetMaximumInputDecimationSampleRate() const {
	int minStage = -1;
	double samprate = 0.0;
	for( size_t i = 0; i < dec.size(); ++i ) {
		if ( minStage < 0 || minStage > dec[i]->GetStageSequenceNumber() ) {
			minStage = dec[i]->GetStageSequenceNumber();
			samprate = dec[i]->GetInputSampleRate();
		}
	}
	return samprate;
}


/****************************************************************************************************************************
* Function:     ResponsePolesZeros											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the ResponsePolesZeros									    *
****************************************************************************************************************************/
ParseResult ResponsePolesZeros::Parse(string record)
{
	transfer_function_type  = record[0];
	stage_sequence_number = FromString<int>(substr(record, 1, 2));
	signal_in_units = FromString<int>(substr(record, 3, 3));
	signal_out_units = FromString<int>(substr(record, 6, 3));
	ao_normalization_factor = FromString<double>(substr(record, 9, 12));
	normalization_frequency = FromString<double>(substr(record, 21, 12));
	number_of_zeros = FromString<int>(substr(record, 33, 3));
	int pos1 = 36;
	Zeros cz;
	for(int i=0; i<number_of_zeros; i++)
	{
		cz.real_zero = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		cz.imaginary_zero = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		cz.real_zero_error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		cz.imaginary_zero_error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		complex_zeros.push_back(cz);
	}
	number_of_poles = FromString<int>(substr(record, pos1, 3));
	pos1 += 3;
	Poles cp;
	for(int i=0; i<number_of_poles; i++)
	{
		cp.real_pole = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		cp.imaginary_pole = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		cp.real_pole_error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		cp.imaginary_pole_error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		complex_poles.push_back(cp);
	}
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     GetComplexZeros												    *
* Parameters:   none													    *
* Returns:      string containing the zeros										    *
* Description:  get all the zeros from the ResponsePolesZeros and put them in a formatted string			    *
*		in the case of inventory the format is: ('real_zero', 'imaginary_zero'), etc.				    *
****************************************************************************************************************************/
string ResponsePolesZeros::GetComplexZeros()
{
	string zeros;
	for(int i=0; i<number_of_zeros; i++)
	{
		zeros += '(';
		zeros += ToString<double>(complex_zeros[i].real_zero);
		zeros += ',';
		zeros += ToString<double>(complex_zeros[i].imaginary_zero);
		zeros += ')';
		if(i<number_of_zeros-1)
			zeros += ' ';
	}
	return zeros;
}

/****************************************************************************************************************************
* Function:     GetComplexPoles												    *
* Parameters:   none													    *
* Returns:      string containing the poles										    *
* Description:  get all the poles from the ReponsePolesZeros and put them in a formatted string				    *
*		in the case of inventory the format is: ('real_pole', 'imaginary_pole'), etc.				    *
****************************************************************************************************************************/
string ResponsePolesZeros::GetComplexPoles()
{
	string poles;
	for(int i=0; i<number_of_poles; i++)
	{
		poles += '(';
		poles += ToString<double>(complex_poles[i].real_pole);
		poles += ',';
		poles += ToString<double>(complex_poles[i].imaginary_pole);
		poles += ')';
		if(i<number_of_poles-1)
			poles += ' ';
	}
	return poles;
}

/****************************************************************************************************************************
* Function:     ResponseCoefficients											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the ResponseCoefficients									    *
****************************************************************************************************************************/
ParseResult ResponseCoefficients::Parse(string record)
{
	response_type  = record[0];
	stage_sequence_number = FromString<int>(substr(record, 1, 2));
	signal_in_units = FromString<int>(substr(record, 3, 3));
	signal_out_units = FromString<int>(substr(record, 6, 3));
	number_of_numerators = FromString<int>(substr(record, 9, 4));
	int pos1 = 13;
	Coefficient coeff;

	int nn = (record.size()-pos1) / 24;
	if ( number_of_numerators > nn ) {
		cerr << "Invalid number of numerators (" << number_of_numerators
		     << ") with respect to blockette size in Coefficient Response, clip to "
		     << nn << endl;
		number_of_numerators = nn;
	}

	for(int i=0; i<number_of_numerators; i++)
	{
		coeff.coefficient = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		coeff.error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		numerators.push_back(coeff);
	}
	number_of_denominators = FromString<int>(substr(record, pos1, 4));
	pos1 += 4;

	int nd = (record.size()-pos1) / 24;
	if ( number_of_denominators > nd ) {
		cerr << "Invalid number of denominators (" << number_of_denominators
		     << ") with respect to blockette size in Coefficient Response, clip to "
		     << nd << endl;
		number_of_denominators = nd;
	}

	for(int i=0; i<number_of_denominators; i++)
	{
		coeff.coefficient = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		coeff.error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		denominators.push_back(coeff);
	}
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     GetNumerators												    *
* Parameters:   none													    *
* Returns:      string with all the numerators										    *
* Description:  get all the numerators from the ResponseCoefficients and put them in a formatted string			    *
****************************************************************************************************************************/
string ResponseCoefficients::GetNumerators()
{
	string num;
	for(int i=0; i<number_of_numerators; i++)
	{
		num += ToString<double>(numerators[i].coefficient);
		num += ' ';
	}
	return num;
}

/****************************************************************************************************************************
* Function:     GetDenominators												    *
* Parameters:   none													    *
* Returns:      string with all the denominators									    *
* Description:  get all the denominators from the ResponseCoefficients and put them in a formatted string		    *
****************************************************************************************************************************/
string ResponseCoefficients::GetDenominators()
{
	string denom;
	for(int i=0; i<number_of_numerators; i++)
	{
		denom += ToString<double>(denominators[i].coefficient);
		denom += ' ';
	}
	return denom;
}

/****************************************************************************************************************************
* Function:     ResponseList												    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the ResponseList										    *
****************************************************************************************************************************/
ParseResult ResponseList::Parse(string record)
{
	stage_sequence_number = FromString<int>(substr(record, 0, 2));

	// No merging
	if ( responses_listed.empty() ) {
		signal_in_units = FromString<int>(substr(record, 2, 3));
		signal_out_units = FromString<int>(substr(record, 5, 3));
		number_of_responses = FromString<int>(substr(record, 8, 4));
	}

	int pos1 = 12;
	ListedResponses lr;

	int items = std::min(number_of_responses, (int)((record.size() - pos1) / 60));

	for(int i=0; i<items; i++)
	{
		lr.frequency = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		lr.amplitude= FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		lr.amplitude_error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		lr.phase_angle = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		lr.phase_error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		responses_listed.push_back(lr);
	}

	if ( items < number_of_responses )
		return PR_Partial;

	return PR_OK;
}

ParseResult ResponseList::Merge(std::string record)
{
	return Parse(record);
}

/****************************************************************************************************************************
* Function:     GenericResponse												    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the GenericResponse										    *
****************************************************************************************************************************/
ParseResult GenericResponse::Parse(string record)
{
	stage_sequence_number = FromString<int>(substr(record, 0, 2));
	signal_in_units = FromString<int>(substr(record, 2, 3));
	signal_out_units = FromString<int>(substr(record, 5, 3));
	number_of_corners = FromString<int>(substr(record, 8, 4));
	int pos1 = 12;
	CornerList cl;
	for(int i=0; i<number_of_corners; i++)
	{
		cl.frequency = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		cl.slope = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		corners_listed.push_back(cl);
	}
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     Decimation												    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the Decimation										    *
****************************************************************************************************************************/
ParseResult Decimation::Parse(string record)
{
	stage_sequence_number = FromString<int>(substr(record, 0, 2));
	input_sample_rate = FromString<double>(substr(record, 2, 10));
	factor = FromString<int>(substr(record, 12, 5));
	offset = FromString<int>(substr(record, 17, 5));
	estimated_delay = FromString<double>(substr(record, 22, 11));
	correction_applied = FromString<double>(substr(record, 33, 11));
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     ChannelSensitivityGain											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the ChannelSensitivityGain									    *
****************************************************************************************************************************/
ParseResult ChannelSensitivityGain::Parse(string record)
{
	stage_sequence_number = FromString<int>(substr(record, 0, 2));
	sensitivity_gain = FromString<double>(substr(record, 2, 12));
	frequency = FromString<double>(substr(record, 14, 12));
	number_of_history_values = FromString<int>(substr(record, 26, 2));
	int pos1 = 28, pos2;
	HistoryValues hv;
	for(int i=0; i<number_of_history_values; i++)
	{
		hv.sensitivity_for_calibration = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		hv.frequency_of_calibration_sensitivity = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		hv.time_of_calibration = SplitString(record, SEED_SEPARATOR, pos1, pos2);
		pos1 = ++pos2;
		history_values.push_back(hv);
	}
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     ResponseReference											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the ResponseReference									    *
****************************************************************************************************************************/
ParseResult ResponseReference::Parse(string record)
{
	number_of_stages = FromString<int>(substr(record, 0,2));

	int pos=2;
	for(int i=0; i < number_of_stages; i++)
	{
		ResponseReferenceStage stage;
		stage.stage_sequence_number = FromString<int>(substr(record, pos, 2));
		pos += 2;

		stage.number_of_responses = FromString<int>(substr(record, pos, 2));
		pos += 2;

		for ( int j = 0; j < stage.number_of_responses; ++j ) {
			stage.response_lookup_key.push_back(FromString<int>(substr(record, pos, 4)));
			pos += 4;
		}

		stages.push_back(stage);
	}
	return PR_OK;
}

/****************************************************************************************************************************
* Function:     FIRResponse												    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the FIRResponse										    *
****************************************************************************************************************************/
ParseResult FIRResponse::Parse(string record)
{
	int seq_num = FromString<int>(substr(record, 0, 2));
	int pos1=2, pos2;

	response_name = SplitString(record, SEED_SEPARATOR, pos1, pos2);
	pos1 = ++pos2;

	// Not merging
	if ( coefficients.empty() ) {
		stage_sequence_number = seq_num;

		symmetry_code = record[pos1++];
		signal_in_units = FromString<int>(substr(record, pos1, 3));
		pos1 += 3;
		signal_out_units = FromString<int>(substr(record, pos1, 3));
		pos1 += 3;
		number_of_coefficients = FromString<int>(substr(record, pos1, 4));
		pos1 += 4;
	}
	else
		pos1 += 3+3+4;

	int max_factors = (record.size()-pos1)/14;
	if ( max_factors > number_of_coefficients ) {
		SEISCOMP_WARNING("Blockette 41: size larger than specified content");
		max_factors = number_of_coefficients;
	}

	for ( int i = 0; i < max_factors; ++i ) {
		coefficients.push_back(FromString<double>(substr(record, pos1, 14)));
		pos1 += 14;
	}

	if ( (int)coefficients.size() < number_of_coefficients )
		return PR_Partial;

	return PR_OK;
}

ParseResult FIRResponse::Merge(string record)
{
	return Parse(record);
}

string FIRResponse::GetCoefficients()
{
      string num;
      for(int i=0; i<number_of_coefficients; i++)
      {
              num += ToString<double>(coefficients[i]);
              num += ' ';
      }
      return num;
}

/****************************************************************************************************************************
* Function:     ResponsePolynomial											    *
* Parameters:   record	- string with data of this blockette								    *
* Description:  initialize the ResponsePolynomial									    *
****************************************************************************************************************************/
ParseResult ResponsePolynomial::Parse(string record)
{
	transfer_function_type  = record[0];
	stage_sequence_number = FromString<int>(substr(record, 1, 2));
	signal_in_units = FromString<int>(substr(record, 3, 3));
	signal_out_units = FromString<int>(substr(record, 6, 3));
	polynomial_approximation_type  = record[9];
	valid_frequency_units  = record[10];
	lower_valid_frequency_bound = FromString<double>(substr(record, 11, 12));
	upper_valid_frequency_bound = FromString<double>(substr(record, 23, 12));
	lower_bound_of_approximation = FromString<double>(substr(record, 35, 12));
	upper_bound_of_approximation = FromString<double>(substr(record, 47, 12));
	maximum_absolute_error = FromString<double>(substr(record, 59, 12));
	number_of_pcoeff = FromString<int>(substr(record, 71, 3));
	int pos1 = 74;
	Coefficient pc;
	for(int i=0; i<number_of_pcoeff; i++)
	{
		pc.coefficient = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		pc.error = FromString<double>(substr(record, pos1, 12));
		pos1 += 12;
		polynomial_coefficients.push_back(pc);
	}
	return PR_OK;
}

string ResponsePolynomial::GetPolynomialCoefficients()
{
      string num;
      for(int i=0; i<number_of_pcoeff; i++)
      {
              num += ToString<double>(polynomial_coefficients[i].coefficient);
              num += ' ';
      }
      return num;
}

