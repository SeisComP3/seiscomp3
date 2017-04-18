/***************************************************************************
 * Copyright (C) 2016 by KNMI, gempa GmbH                                  *
 *                                                                         *
 * All Rights Reserved.                                                    *
 *                                                                         *
 * NOTICE: All information contained herein is, and remains                *
 * the property of gempa GmbH and its suppliers, if any. The intellectual  *
 * and technical concepts contained herein are proprietary to gempa GmbH   *
 * and its suppliers.                                                      *
 * Dissemination of this information or reproduction of this material      *
 * is strictly forbidden unless prior written permission is obtained       *
 * from gempa GmbH.                                                        *
 *                                                                         *
 * Author: Peter de Boer, KNMI                                             *
 * Date: 2008-01-17                                                        *
 *                                                                         *
 * Maintainance and improvements: Jan Becker                               *
 * Email: jabe@gempa.de                                                    *
 ***************************************************************************/


#include <complex>
#include <cstdio>
#include "inventory.h"

#include <seiscomp3/client/inventory.h>
#include <seiscomp3/utils/replace.h>
#include <seiscomp3/core/system.h>

#define SEISCOMP_COMPONENT sync_dlsv
#include <seiscomp3/logging/log.h>

#ifdef WIN32
#define snprintf _snprintf
#endif

#define LOG_STAGES 1


using namespace std;
using namespace Seiscomp;


namespace {


inline string strip(string s) {
	size_t i = 0, j = 0;

	for ( i = 0; i < s.length(); ++i ) {
		if ( s[i] != ' ' )
			break;
	}

	for ( j = s.length(); j; --j ) {
		if ( s[j-1] != ' ' )
			break;
	}

	return string(s, i, j);
}

string join(const vector<string> &list, const string &sep) {
	string result;
	for ( size_t i = 0; i < list.size(); ++i ) {
		if ( i ) result += sep;
		result += list[i];
	}
	return result;
}

inline DataModel::RealArray parseRealArray(const string &s) {
	DataModel::RealArray a;
	vector<double> v;

	if ( s.empty() )
		return a;

	if ( !Core::fromString(v, strip(s)) ) {
		SEISCOMP_ERROR("invalid real array: %s", s.c_str());
		return a;
	}

	a.setContent(v);
	return a;
}

inline DataModel::ComplexArray parseComplexArray(const string &s) {
	DataModel::ComplexArray a;
	vector<complex<double> > v;

	if ( s.empty() )
		return a;

	if ( !Core::fromString(v, strip(s)) ) {
		SEISCOMP_ERROR("invalid complex array: %s", s.c_str());
		return a;
	}

	a.setContent(v);
	return a;
}

inline string blob2str(const DataModel::Blob &b) {
	return b.content();
}

inline DataModel::Blob str2blob(const string &s) {
	DataModel::Blob b;
	b.setContent(s);
	return b;
}

inline bool _is_leap(int y) {
	return (y % 400 == 0 || (y % 4 == 0 && y % 100 != 0));
}

inline int _ldoy(int y, int m) {
	static const int DOY[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
	return (DOY[m] + (_is_leap(y) && m >= 2));
}

inline string date2str(const Core::Time& t) {
	int year, month, day, hour, minute, second;
	t.get(&year, &month, &day, &hour, &minute, &second);

	if ( month < 1 || month > 12 || day < 1 || day > 31 ) {
		SEISCOMP_ERROR("invalid date: month=%d, day=%d", month, day);
		month = 1;
		day = 0;
	}

	char buf[20];
	snprintf(buf, sizeof(buf)-1, "%d.%03d.%02d.%02d.%02d", year, _ldoy(year, month - 1) + day, hour, minute, second);
	return string(buf);
}

typedef pair<int,int> Fraction;

Fraction double2frac(double d) {
	double df = 1;
	Fraction::first_type top = d >= 2.0 ? d-1 : 1, ctop = top;
	Fraction::second_type bot = d <= 0.5 ? 1/d-1 : 1, cbot = bot;
	double error = fabs(df-d);
	double last_error = error*2;
	bool fixed_top = false;

	if ( fabs(d) < 1E-20 )
		return Fraction(0,1);

	while ( error < last_error ) {
		ctop = top;
		cbot = bot;

		//cerr << error << "  " << top << "/" << bot << endl;
		if ( df < d )
			++top;
		else {
			++bot;
			top = Fraction::first_type(d * bot);
		}

		df = (double)top / (double)bot;
		if ( top > 0 ) {
			last_error = error;
			error = fabs(df-d);
			fixed_top = false;
		}
		else if ( fixed_top ) {
			cbot = 1;
			break;
		}
		else
			fixed_top = true;

		if ( top < 0 || bot < 0 )
			return Fraction(0,0);
	}

	return Fraction(ctop,cbot);
}

inline void check_fir(DataModel::ResponseFIRPtr rf, int &errors) {
	vector<double> &v = rf->coefficients().content();
	int nc = (int)v.size();

	if ( rf->numberOfCoefficients() != nc ) {
		SEISCOMP_ERROR("expected %d coefficients, found %d", rf->numberOfCoefficients(), nc);
		rf->setNumberOfCoefficients(nc);
		++errors;
	}

	if ( nc == 0 || rf->symmetry() != "A" )
		return;

	int i = 0;
	for (; 2 * i < nc; ++i )
		if ( v[i] != v[nc - 1 - i]) break;

	if ( 2 * i > nc ) {
		rf->setNumberOfCoefficients(i);
		rf->setSymmetry("B");
		v.resize(i);
		SEISCOMP_DEBUG("A(%d) -> B(%d)", nc, i);
	}
	else if ( 2 * i == nc ) {
		rf->setNumberOfCoefficients(i);
		rf->setSymmetry("C");
		v.resize(i);
		SEISCOMP_DEBUG("A(%d) -> C(%d)", nc, i);
	}
	else
		SEISCOMP_DEBUG("A(%d) -> A(%d)", nc, nc);
}

inline void check_paz(DataModel::ResponsePAZPtr rp, int &errors) {
	if ( rp->numberOfPoles() != (int)rp->poles().content().size() ) {
		SEISCOMP_ERROR("expected %d poles, found %lu", rp->numberOfPoles(), (unsigned long)rp->poles().content().size());
		rp->setNumberOfPoles(rp->poles().content().size());
		++errors;
	}

	if ( rp->numberOfZeros() != (int)rp->zeros().content().size() ) {
		SEISCOMP_ERROR("expected %d zeros, found %lu", rp->numberOfZeros(), (unsigned long)rp->zeros().content().size());
		rp->setNumberOfZeros(rp->zeros().content().size());
		++errors;
	}
}


struct NetworkDescriptionResolver : public Util::VariableResolver {
	NetworkDescriptionResolver(DataModel::Network *n) : net(n) {}

	virtual bool resolve(std::string &variable) const {
		if ( variable == "code" )
			variable = net->code();
		else if ( variable == "start" )
			variable = net->start().toString("%F %T");
		else if ( variable == "end" ) {
			try {
				variable = net->end().toString("%F %T");
			}
			catch ( ... ) {
				variable = "";
			}
		}
		else if ( variable == "class" )
			variable = net->netClass();
		else if ( variable == "archive" )
			variable = net->archive();
		else
			variable = "";
		return true;
	}

	DataModel::Network *net;
};


bool isElectric(const string &unit) {
	return unit == AMPERE || unit == VOLTAGE;
}


bool isElectric(const UnitsAbbreviations &ua) {
	return isElectric(ua.GetName());
}


bool isAnalogDataloggerStage(AbbreviationDictionaryControl *adc,
                             const Inventory::StageItem &item) {
	return isElectric(adc->UnitName(item.inputUnit))
	    && isElectric(adc->UnitName(item.outputUnit));
}


bool isDigitalDataloggerStage(AbbreviationDictionaryControl *adc,
                              const Inventory::StageItem &item) {
	return !isElectric(adc->UnitName(item.outputUnit));
}

bool isADCStage(AbbreviationDictionaryControl *adc,
                const Inventory::StageItem &item) {
	return isElectric(adc->UnitName(item.inputUnit))
	   && !isElectric(adc->UnitName(item.outputUnit));
}


bool IsDummy(const ChannelIdentifier& ci, const Inventory::StageItem &item, double &stageGain) {
	stageGain = 1.0;

	for ( size_t i = 0; i< ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == (int)item.stage ) {
			stageGain = ci.csg[i]->GetSensitivityGain();
			break;
		}
	}

	switch ( item.type ) {
		case Inventory::RT_FIR:
			if ( ci.firr[item.index]->GetNumberOfCoefficients() == 0 ) {
				return true;
			}
			break;
		case Inventory::RT_RC:
			if ( ci.rc[item.index]->GetNumberOfNumerators() == 0 &&
			     ci.rc[item.index]->GetNumberOfDenominators() == 0 ) {
				return true;
			}
		case Inventory::RT_PAZ:
			if ( ci.rpz[item.index]->GetNumberOfPoles() == 0 &&
			     ci.rpz[item.index]->GetNumberOfZeros() == 0 ) {
				return true;
			}
			break;
		default:
			break;
	}

	return false;
}


template <typename C>
Inventory::SequenceNumber hasSensorStage(const C &objects, AbbreviationDictionaryControl *adc) {
	for ( size_t i = 0; i < objects.size(); ++i ) {
		int seq_in = -1, seq_out = -2;
		int siu = objects[i]->GetSignalInUnits();
		int sou = objects[i]->GetSignalOutUnits();

		for ( size_t j = 0; j < adc->ua.size(); ++j ) {
			UnitsAbbreviations ua = *adc->ua[j];
			if ( siu == ua.GetLookup() ) {
				if ( !isElectric(ua) )
					seq_in = i;
			}

			if ( sou == ua.GetLookup() ) {
				if ( isElectric(ua) )
					seq_out = i;
			}
		}

		if ( seq_in == seq_out )
			return objects[seq_in]->GetStageSequenceNumber();
	}

	return Core::None;
}


template <typename C>
Inventory::SequenceNumber getSensorStage(const C &objects, AbbreviationDictionaryControl *adc) {
	for ( size_t i = 0; i < objects.size(); ++i ) {
		int seq_in = -1, seq_out = -2;
		int siu = objects[i]->GetSignalInUnits();
		int sou = objects[i]->GetSignalOutUnits();
		for( size_t j = 0; j < adc->ua.size(); ++j ) {
			UnitsAbbreviations ua = *adc->ua[j];
			if ( siu == ua.GetLookup() ) {
				if ( !isElectric(ua) )
					seq_in = i;
			}

			if ( sou == ua.GetLookup() ) {
				if ( isElectric(ua) )
					seq_out = i;
			}
		}

		if ( seq_in == seq_out )
			return seq_in;
	}

	return Core::None;
}


template <typename C>
void populateStages(Inventory::Stages &stages, const C &objects, Inventory::ResponseType rt) {
	for ( size_t i = 0; i < objects.size(); ++i )
		stages.push_back(Inventory::StageItem(objects[i]->GetStageSequenceNumber(), i,
		                                      rt,
		                                      objects[i]->GetSignalInUnits(),
		                                      objects[i]->GetSignalOutUnits()));
}


bool bySequenceNumber(const Inventory::StageItem &i1, const Inventory::StageItem &i2) {
	return i1.stage < i2.stage;
}



}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory::Inventory(const std::string &dcid, const std::string &net_description,
	const std::string &net_type, const Seiscomp::Core::Time &net_start,
	const OPT(Seiscomp::Core::Time) &net_end, bool temporary, bool restricted, bool shared,
	DataModel::Inventory *inv)
: _dcid(dcid), _net_description(net_description), _net_type(net_type),
	_net_start(net_start), _net_end(net_end), _temporary(temporary),
	_restricted(restricted), _shared(shared), inventory(inv) {
	vector<string> steim1;
	steim1.push_back("F1 P4 W4 D0-31 C2 R1 P8 W4 D0-31 C2");
	steim1.push_back("P0 W4 N15 S2,0,1");
	steim1.push_back("T0 X N0 W4 D0-31 C2");
	steim1.push_back("T1 N0 W1 D0-7 C2 N1 W1 D0-7 C2 N2 W1 D0-7 C2 N3 W1 D0-7 C2");
	steim1.push_back("T2 N0 W2 D0-15 C2 N1 W2 D0-15 C2");
	steim1.push_back("T3 N0 W4 D0-31 C2");

	vector<string> steim2;
	steim2.push_back("F1 P4 W4 D C2 R1 P8 W4 D C2");
	steim2.push_back("P0 W4 N15 S2,0,1");
	steim2.push_back("T0 X W4");
	steim2.push_back("T1 Y4 W1 D C2");
	steim2.push_back("T2 W4 I D2");
	steim2.push_back("K0 X D30");
	steim2.push_back("K1 N0 D30 C2");
	steim2.push_back("K2 Y2 D15 C2");
	steim2.push_back("K3 Y3 D10 C2");
	steim2.push_back("T3 W4 I D2");
	steim2.push_back("K0 Y5 D6 C2");
	steim2.push_back("K1 Y6 D5 C2");
	steim2.push_back("K2 X D2 Y7 D4 C2");
	steim2.push_back("K3 X D30");

	vector<string> geoscope3bit;
	geoscope3bit.push_back("M0");
	geoscope3bit.push_back("W2 D0-11 A-2048");
	geoscope3bit.push_back("D12-14");
	geoscope3bit.push_back("E2:0:-1");

	vector<string> geoscope4bit;
	geoscope4bit.push_back("M0");
	geoscope4bit.push_back("W2 D0-11 A-2048");
	geoscope4bit.push_back("D12-15");
	geoscope4bit.push_back("E2:0:-1");

	encoding.insert(make_pair(steim1, string("Steim1")));
	encoding.insert(make_pair(steim2, string("Steim2")));
	encoding.insert(make_pair(geoscope3bit, string("mseed13")));
	encoding.insert(make_pair(geoscope4bit, string("mseed14")));

	_fixedErrors = 0;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::SynchronizeInventory() {
	_fixedErrors = 0;
	ProcessStation();
	SEISCOMP_INFO("Finished.");
	// CleanupDatabase();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::CleanupDatabase() {
	SEISCOMP_INFO("Cleaning up the database");

	set<pair<string, string> > stat_codes;
	for(unsigned int i=0; i<sc->si.size(); i++)
	{
		stat_codes.insert(make_pair(strip(sc->si[i]->GetNetworkCode()),
			strip(sc->si[i]->GetStationCallLetters())));
	}

	for(unsigned int i = 0; i < inventory->networkCount(); )
	{
		DataModel::NetworkPtr net = inventory->network(i);
		for(unsigned int j = 0; j < net->stationCount(); )
		{
			DataModel::StationPtr sta = net->station(j);
			if(stat_codes.find(make_pair(net->code(), sta->code())) == stat_codes.end())
			{
				++j;
				continue;
			}

			if(sta->archive() != _dcid)
			{
				SEISCOMP_INFO("skipping station %s %s", net->code().c_str(), sta->code().c_str());

				++j;
				continue;
			}

			if(stations.find(make_pair(make_pair(net->code(), sta->code()), sta->start())) == stations.end())
			{
				net->remove(sta.get());
				continue;
			}

			for(unsigned int k = 0; k < sta->sensorLocationCount(); )
			{
				DataModel::SensorLocationPtr loc = sta->sensorLocation(k);
				if(sensor_locations.find(make_pair(make_pair(make_pair(make_pair(net->code(), sta->code()), loc->code()), sta->start()), loc->start())) == sensor_locations.end())
				{
					sta->remove(loc.get());
					continue;
				}

				for(unsigned int l = 0; l < loc->streamCount(); )
				{
					DataModel::StreamPtr strm = loc->stream(l);
					if(seis_streams.find(make_pair(make_pair(make_pair(make_pair(make_pair(make_pair(net->code(), sta->code()), strm->code()), loc->code()), sta->start()), strm->start()), loc->start())) == seis_streams.end())
					{
						loc->remove(strm.get());
						continue;
					}

					++l;
				}

				for(unsigned int l = 0; l < loc->auxStreamCount(); )
				{
					DataModel::AuxStreamPtr strm = loc->auxStream(l);
					if(aux_streams.find(make_pair(make_pair(make_pair(make_pair(make_pair(make_pair(net->code(), sta->code()), strm->code()), loc->code()), sta->start()), strm->start()), loc->start())) == aux_streams.end())
					{
						loc->remove(strm.get());
						continue;
					}

					++l;
				}

				++k;
			}

			++j;
		}

		++i;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::GetComment(StationIdentifier& si) {
	DataModel::WaveformStreamID *wf = new DataModel::WaveformStreamID();
	wf->setNetworkCode(strip(si.GetNetworkCode()));
	wf->setStationCode(strip(si.GetStationCallLetters()));
	wf->setLocationCode("");
	wf->setChannelCode("");
	for(unsigned int noc=0; noc<si.sc.size(); noc++)
		GetStationComment(*si.sc[noc], wf);
	for(unsigned int component = 0; component < si.ci.size(); component++)
		GetChannelComment(*si.ci[component], wf);
	delete wf;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::GetStationComment(Comment &sc, DataModel::WaveformStreamID *wf) {
	int code_key = sc.GetCommentCodeKey();
	for(unsigned int j=0; j<adc->cd.size(); j++)
	{
		CommentDescription comm = *adc->cd[j];
		if(code_key == comm.GetCommentCodeKey())
		{
			if(comm.GetDescriptionOfComment().size()>1)
			{
				DataModel::QCLog *log = DataModel::QCLog::Create();
				log->setWaveformID(*wf);
				log->setMessage(comm.GetDescriptionOfComment());
				log->setStart(GetTime(sc.GetBeginningEffectiveTime()));
				if ( !sc.GetEndEffectiveTime().empty() )
					log->setEnd(GetTime(sc.GetEndEffectiveTime()));
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::GetChannelComment(ChannelIdentifier& ci, DataModel::WaveformStreamID *wf) {
	wf->setLocationCode(strip(ci.GetLocation()));
	wf->setChannelCode(strip(ci.GetChannel()));
	for(unsigned int noc=0; noc<ci.cc.size();noc++)
	{
		Comment comment = *ci.cc[noc];
		int code_key = comment.GetCommentCodeKey();
		for(unsigned int j=0; j<adc->cd.size(); j++)
		{
			CommentDescription comm = *adc->cd[j];
			if(code_key == comm.GetCommentCodeKey())
			{
				if(comm.GetDescriptionOfComment().size()>1)
				{
					DataModel::QCLog *log = DataModel::QCLog::Create();
					log->setWaveformID(*wf);
					log->setMessage(comm.GetDescriptionOfComment());
					log->setStart(GetTime(comment.GetBeginningEffectiveTime()));
					if(!comment.GetEndEffectiveTime().empty())
						log->setEnd(GetTime(comment.GetEndEffectiveTime()));
				}
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Core::Time Inventory::GetTime(string strTime, bool *ok) {
	int year=0, yday=0, month=0, day=0, hour=0, minute=0, second=0;
	int ondergrens, bovengrens;
	std::vector<std::string> date, time;
	stringstream ss;

	if ( ok ) *ok = true;

	Core::split(date, strip(strTime).c_str(), ",", false);

	if(date.size() > 0)
		sscanf(date[0].c_str(), "%d", &year);

	if(date.size() > 1)
	{
		sscanf(date[1].c_str(), "%d", &yday);
		for(int m=0;m<12;m++)
		{
			ondergrens = _ldoy(year, m);
			bovengrens = _ldoy(year, m+1);
			if(ondergrens < yday && yday <= bovengrens)
			{
				month = m+1;
				day = yday - _ldoy(year, m);
				m = 12;
			}
		}
	}

	if(date.size() > 2)
		Seiscomp::Core::split(time, date[2].c_str(), ":", false);

	if(time.size() > 0)
		sscanf(time[0].c_str(), "%d", &hour);

	if(time.size() > 1)
		sscanf(time[1].c_str(), "%d", &minute);

	if(time.size() > 2)
		sscanf(time[2].c_str(), "%d", &second);

	if ( year <= 1970 || year > 2037 ) {
		if ( ok ) *ok = false;
		return Core::Time();
	}

	return Core::Time(year, month, day, hour, minute, second);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
OPT(Core::Time) Inventory::GetOptTime(string strTime) {
	bool ok;
	Core::Time t = GetTime(strTime, &ok);
	if ( !ok ) return Core::None;
	return t;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::ProcessStation() {
	SEISCOMP_DEBUG("Start processing station information");

	for(unsigned int i = 0; i < inventory->networkCount(); ++i)
	{
		DataModel::NetworkPtr net = inventory->network(i);

		int net_year = 0;
		if(_temporary)
			_net_start.get(&net_year);

		networks[make_pair(net->code(), net_year)] = net;

		for(unsigned int j = 0; j < net->stationCount(); ++j)
		{
			DataModel::StationPtr sta = net->station(j);
			stations[make_pair(make_pair(net->code(), sta->code()), sta->start())] = sta;
		}
	}

	for(unsigned int i=0; i<sc->si.size(); i++)
	{
		string net_code = strip(sc->si[i]->GetNetworkCode());
		string sta_code = strip(sc->si[i]->GetStationCallLetters());
		string sta_start = strip(sc->si[i]->GetStartDate());

		int net_year = 0;
		if(_temporary)
			_net_start.get(&net_year);

		SEISCOMP_INFO("Processing station %s %s %s", net_code.c_str(), sta_code.c_str(), sta_start.c_str());

		map<pair<string, int>, DataModel::NetworkPtr>::iterator net_it = networks.find(make_pair(net_code, net_year));

		DataModel::NetworkPtr net;

		if(net_it != networks.end())
		{
			net = net_it->second;
			net->update();
		}
		else
		{
 			net = DataModel::Network::Create();
 			net->setCode(net_code);
			net->setStart(_net_start);
			net->setArchive(_dcid);
			net->setNetClass(_temporary? "t": "p");
			networks[make_pair(net_code, net_year)] = net;
			inventory->add(net.get());
 		}

		if ( _net_end )
			net->setEnd(*_net_end);
		else
			net->setEnd(Core::None);

		net->setType(_net_type);
		net->setRestricted(_restricted);
		net->setShared(_shared);

		if ( _net_description.empty() )
			net->setDescription(GetNetworkDescription(sc->si[i]->GetNetworkIdentifierCode()));
		else
			net->setDescription(Util::replace(_net_description, NetworkDescriptionResolver(net.get()), "${", "}", ""));

		map<pair<pair<string, string>, Core::Time>, DataModel::StationPtr>::iterator sta_it = \
			stations.find(make_pair(make_pair(net_code, sta_code), GetTime(sta_start)));

		if(sta_it != stations.end())
		{
			DataModel::StationPtr sta = sta_it->second;
			if(sta->archive() != _dcid)
			{
				SEISCOMP_WARNING("station %s (%s) belongs to datacenter %s, ignoring station %s",
					sta_code.c_str(), sta_start.c_str(), sta->archive().c_str(), sta_code.c_str());
				continue;
			}

			UpdateStation(*sc->si[i], sta);
			ProcessStream(*sc->si[i], sta);
		}
		else
		{
			DataModel::StationPtr sta = InsertStation(*sc->si[i], net);
			stations[make_pair(make_pair(net->code(), sta->code()), sta->start())] = sta;
			ProcessStream(*sc->si[i], sta);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::StationPtr Inventory::InsertStation(StationIdentifier& si, DataModel::NetworkPtr network) {
	SEISCOMP_DEBUG("Insert station");

	DataModel::StationPtr station = DataModel::Station::Create();
	station->setCode(strip(si.GetStationCallLetters()));

	string desc = si.GetSiteName();
	if(desc.empty())
		desc = si.GetStationCallLetters();

	station->setDescription(desc);
	station->setAffiliation(GetNetworkDescription(si.GetNetworkIdentifierCode()));
	station->setStart(GetTime(si.GetStartDate()));
	station->setEnd(GetOptTime(si.GetEndDate()));
	station->setLatitude(si.GetLatitude());
	station->setLongitude(si.GetLongitude());
	station->setElevation(si.GetElevation());
	station->setPlace(si.GetCity());
	station->setCountry(si.GetCountry());
	station->setRemark(Core::None);
	station->setRestricted(network->restricted());
	station->setShared(network->shared());
	station->setArchive(_dcid);
	GetComment(si);

	network->add(station.get());
	return station;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::UpdateStation(StationIdentifier& si, DataModel::StationPtr station) {
	SEISCOMP_DEBUG("Update station");

	string desc = si.GetSiteName();
	if(desc.empty())
		desc = si.GetStationCallLetters();

	station->setDescription(desc);
	station->setAffiliation(GetNetworkDescription(si.GetNetworkIdentifierCode()));
	station->setEnd(GetOptTime(si.GetEndDate()));
	station->setLatitude(si.GetLatitude());
	station->setLongitude(si.GetLongitude());
	station->setElevation(si.GetElevation());
	station->setPlace(si.GetCity());
	station->setCountry(si.GetCountry());
	station->setRemark(Core::None);
	GetComment(si);

	station->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::ProcessStream(StationIdentifier& si, DataModel::StationPtr station) {
	SEISCOMP_DEBUG("Start processing stream information");

	string net_code = strip(si.GetNetworkCode());
	string sta_code = strip(si.GetStationCallLetters());

	//cerr << net_code << "." << sta_code << endl;

	map<pair<pair<double, double>, string>, pair<Core::Time, OPT(Core::Time)> > loc_map;
	for ( size_t i = 0; i < si.ci.size(); ++i ) {
		ChannelIdentifier ci = *si.ci[i];
		string loc_code = strip(ci.GetLocation());
		Core::Time loc_start = GetTime(ci.GetStartDate());
		OPT(Core::Time) loc_end = GetOptTime(ci.GetEndDate());

		//cerr << "  + " << loc_code << ci.GetChannel() << "   " << loc_start.toString("%F %T") << endl;

		map<pair<pair<double, double>, string>, pair<Core::Time, OPT(Core::Time)> >::iterator p;
		if ( (p = loc_map.find(make_pair(make_pair(ci.GetLatitude(), ci.GetLongitude()), loc_code))) == loc_map.end() ) {
			p = loc_map.insert(make_pair(make_pair(make_pair(ci.GetLatitude(), ci.GetLongitude()), loc_code), make_pair(loc_start, loc_end))).first;
		}
		else {
			if ( p->second.first > loc_start )
				p->second.first = loc_start;

			if ( p->second.second && !loc_end )
				p->second.second = loc_end;
			else if ( p->second.second && loc_end && *p->second.second < *loc_end )
				p->second.second = *loc_end;
		}

		map<pair<pair<double, double>, string>, pair<Core::Time, OPT(Core::Time)> >::iterator p1 = loc_map.begin();
		while ( p1 != loc_map.end() ) {
			if ( p1 != p && p1->first.second == p->first.second && p1->second.first == p->second.first ) {
				SEISCOMP_ERROR((net_code + " " + sta_code + " " + ci.GetChannel() + " sensor location '" + loc_code + "' starting " + loc_start.toString("%Y-%m-%d %H:%M:%S") + " has conflicting coordinates: "
				               "%f/%f vs. %f/%f: increasing start time by 1 sec.").c_str(),
				               p->first.first.first, p->first.first.second,
				               p1->first.first.first, p1->first.first.second);
				p->second.first += Core::TimeSpan(1.0);
				p1 = loc_map.begin();
				++_fixedErrors;
				continue;
			}

			++p1;
		}
	}

	for( size_t i = 0; i < si.ci.size(); ++i ) {
		ChannelIdentifier ci = *si.ci[i];
		station_name = strip(si.GetNetworkCode()) + "." + strip(si.GetStationCallLetters()) + "." + date2str(GetTime(ci.GetStartDate()));
		channel_name = strip(si.GetNetworkCode()) + "." + strip(si.GetStationCallLetters()) + "." + strip(ci.GetLocation()) + "." + ci.GetChannel() + "." + date2str(GetTime(ci.GetStartDate()));

		string strm_code = ci.GetChannel();
		string loc_code = strip(ci.GetLocation());
		Core::Time sta_start = GetTime(si.GetStartDate());
		Core::Time strm_start = GetTime(ci.GetStartDate());
		Core::Time loc_start;
		OPT(Core::Time) loc_end;

		map<pair<pair<double, double>, string>, pair<Core::Time, OPT(Core::Time)> >::iterator p;
		if ( (p = loc_map.find(make_pair(make_pair(ci.GetLatitude(), ci.GetLongitude()), loc_code))) != loc_map.end() ) {
			loc_start = p->second.first;
			loc_end = p->second.second;
		}
		else {
			SEISCOMP_ERROR("cannot find location (should not happen)");
			exit(1);
		}

		sensor_locations.insert(make_pair(make_pair(make_pair(make_pair(net_code, sta_code), loc_code), sta_start), loc_start));

		DataModel::SensorLocationPtr loc = station->sensorLocation(DataModel::SensorLocationIndex(loc_code, loc_start));

		if ( !loc )
			loc = InsertSensorLocation(ci, station, loc_start, loc_end);
		else
			UpdateSensorLocation(ci, loc, loc_start, loc_end);

		// Resolve references and copy them into the channel info
		for ( size_t i=0; i<ci.rr.size(); ++i ) {
			int stages = ci.rr[i]->GetNumberOfStages();
			for ( int s = 0; s < stages; ++s ) {
				const ResponseReferenceStage &stage = ci.rr[i]->GetStages()[s];
				for ( int r = 0; r < stage.GetNumberOfResponses(); ++r ) {
					stage.GetResponseLookupKey()[r];

					// Copy FIRR
					for ( size_t l = 0; l < adc->fird.size(); ++l ) {
						if ( adc->fird[l]->GetLookup() != stage.GetResponseLookupKey()[r] ) continue;
						ci.firr.push_back(new FIRResponse(*adc->fird[l]));
						ci.firr.back()->SetStageSequenceNumber(stage.GetStageSequenceNumber());
					}

					// Copy Polys
					for ( size_t l = 0; l < adc->rpd.size(); ++l ) {
						if ( adc->rpd[l]->GetLookup() != stage.GetResponseLookupKey()[r] ) continue;
						ci.rp.push_back(new ResponsePolynomial(*adc->rpd[l]));
						ci.rp.back()->SetStageSequenceNumber(stage.GetStageSequenceNumber());
					}

					// Copy PAZ
					for ( size_t l = 0; l < adc->rpzd.size(); ++l ) {
						if ( adc->rpzd[l]->GetLookup() != stage.GetResponseLookupKey()[r] ) continue;
						ci.rpz.push_back(new ResponsePolesZeros(*adc->rpzd[l]));
						ci.rpz.back()->SetStageSequenceNumber(stage.GetStageSequenceNumber());
					}

					// Copy Coefficients
					for ( size_t l = 0; l < adc->rcd.size(); ++l ) {
						if ( adc->rcd[l]->GetLookup() != stage.GetResponseLookupKey()[r] ) continue;
						ci.rc.push_back(new ResponseCoefficients(*adc->rcd[l]));
						ci.rc.back()->SetStageSequenceNumber(stage.GetStageSequenceNumber());
					}

					// Copy Lists
					for ( size_t l = 0; l < adc->rld.size(); ++l ) {
						if ( adc->rld[l]->GetLookup() != stage.GetResponseLookupKey()[r] ) continue;
						ci.rl.push_back(new ResponseList(*adc->rld[l]));
						ci.rl.back()->SetStageSequenceNumber(stage.GetStageSequenceNumber());
					}

					// Copy Generics
					for ( size_t l = 0; l < adc->grd.size(); ++l ) {
						if ( adc->grd[l]->GetLookup() != stage.GetResponseLookupKey()[r] ) continue;
						ci.gr.push_back(new GenericResponse(*adc->grd[l]));
						ci.gr.back()->SetStageSequenceNumber(stage.GetStageSequenceNumber());
					}

					// Copy Decimations
					for ( size_t l = 0; l < adc->dd.size(); ++l ) {
						if ( adc->dd[l]->GetLookup() != stage.GetResponseLookupKey()[r] ) continue;
						ci.dec.push_back(new Decimation(*adc->dd[l]));
						ci.dec.back()->SetStageSequenceNumber(stage.GetStageSequenceNumber());
					}

					// Copy Sensitivities
					for ( size_t l = 0; l < adc->csgd.size(); ++l ) {
						if ( adc->csgd[l]->GetLookup() != stage.GetResponseLookupKey()[r] ) continue;
						ci.csg.push_back(new ChannelSensitivityGain(*adc->csgd[l]));
						ci.csg.back()->SetStageSequenceNumber(stage.GetStageSequenceNumber());
					}
				}
			}
		}

		SequenceNumber sensorStage;
		ResponseType srt = GetSensorResponseType(ci, sensorStage);
		seis_streams.insert(make_pair(make_pair(make_pair(make_pair(make_pair(make_pair(net_code, sta_code), strm_code), loc_code), sta_start), strm_start), loc_start));

		DataModel::StreamPtr strm = loc->stream(DataModel::StreamIndex(strm_code, strm_start));

		if ( !strm )
			strm = InsertStream(ci, loc, station->restricted(), station->shared());
		else
			UpdateStream(ci, strm, station->restricted(), station->shared());

#if LOG_STAGES
		cerr << "[" << strm_code << "]" << endl;
		if ( srt != RT_None )
			cerr << " + S " << srt.toString() << endl;
#endif

		ProcessDatalogger(ci, strm);

		switch ( srt ) {
			case RT_None:
				break;
			case RT_PAZ:
				ProcessPAZSensor(ci, strm, *sensorStage);
				break;
			case RT_Poly:
				ProcessPolySensor(ci, strm, *sensorStage);
				break;
			case RT_FAP:
				ProcessFAPSensor(ci, strm, *sensorStage);
				break;
			default:
				SEISCOMP_ERROR("Sensor response type is %s, but only %s, %s and %s are "
				               "currently supported: ignore response",
				               srt.toString(),
				               ResponseType(RT_PAZ).toString(),
				               ResponseType(RT_Poly).toString(),
				               ResponseType(RT_FAP).toString());
				break;
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::SensorLocationPtr
Inventory::InsertSensorLocation(ChannelIdentifier& ci, DataModel::StationPtr station,
                                const Core::Time& loc_start, const OPT(Core::Time)& loc_end) {
	SEISCOMP_DEBUG("Insert sensor location information (%s)", ci.GetChannel().c_str());

	DataModel::SensorLocationPtr loc = DataModel::SensorLocation::Create();
	loc->setCode(strip(ci.GetLocation()));
	loc->setStart(loc_start);
	loc->setEnd(loc_end);
	loc->setLatitude(ci.GetLatitude());
	loc->setLongitude(ci.GetLongitude());
	loc->setElevation(ci.GetElevation());

	station->add(loc.get());
	return loc;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::UpdateSensorLocation(ChannelIdentifier& ci,
                                     DataModel::SensorLocationPtr loc,
                                     const Core::Time& loc_start,
                                     const OPT(Core::Time)& loc_end)
{
	SEISCOMP_DEBUG("Update sensor location information (%s)", ci.GetChannel().c_str());

	loc->setEnd(loc_end);
	loc->setLatitude(ci.GetLatitude());
	loc->setLongitude(ci.GetLongitude());
	loc->setElevation(ci.GetElevation());

	loc->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::StreamPtr
Inventory::InsertStream(ChannelIdentifier& ci, DataModel::SensorLocationPtr loc,
                        bool restricted, bool shared) {
	Fraction samprate = double2frac(ci.GetSampleRate());
	if ( samprate.first == 0 || samprate.second == 0 ) {
		SEISCOMP_WARNING("%s: invalid sample rate %.2f -> checking for valid decimations",
		                 ci.GetChannel().c_str(), ci.GetSampleRate());

		samprate = double2frac(ci.GetMinimumInputDecimationSampleRate());
		if ( samprate.first == 0 || samprate.second == 0 ) {
			SEISCOMP_WARNING("%s: invalid sample rate %.2f, keeping it",
			                 ci.GetChannel().c_str(), ci.GetSampleRate());
		}
	}

	SEISCOMP_DEBUG("Insert seisstream information (%s, %d/%d sps)",
	               ci.GetChannel().c_str(), samprate.first, samprate.second);

	// Adjust strm_start if loc_start was adjusted earlier
	Core::Time strm_start = GetTime(ci.GetStartDate());
	if(strm_start < loc->start())
		strm_start = loc->start();

	DataModel::StreamPtr strm = new DataModel::Stream();
	strm->setCode(ci.GetChannel());
	strm->setStart(strm_start);
	strm->setEnd(GetOptTime(ci.GetEndDate()));
	strm->setDataloggerSerialNumber("xxxx");
	strm->setSensorSerialNumber("yyyy");

	strm->setSampleRateNumerator(samprate.first);
	strm->setSampleRateDenominator(samprate.second);

	if(ci.GetChannel().substr(2,1)=="N" || ci.GetChannel().substr(2,1)=="1")
	{
		strm->setDataloggerChannel(1);
		strm->setSensorChannel(1);
	}
	else if(ci.GetChannel().substr(2,1)=="E" || ci.GetChannel().substr(2,1)=="2")
	{
		strm->setDataloggerChannel(2);
		strm->setSensorChannel(2);
	}
	else
	{
		strm->setDataloggerChannel(0);
		strm->setSensorChannel(0);
	}

	strm->setDepth(ci.GetLocalDepth());
	strm->setAzimuth(ci.GetAzimuth());
	strm->setDip(ci.GetDip());

	strm->setGain(Core::None);
	strm->setGainFrequency(Core::None);

	for( size_t i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == 0 ) {
			strm->setGain(ci.csg[i]->GetSensitivityGain());
			strm->setGainFrequency(ci.csg[i]->GetFrequency());

			/*
			if ( ci.csg[i]->GetSensitivityGain() < 0 ) {
				if ( ci.GetAzimuth() < 180.0 )
					strm->setAzimuth(ci.GetAzimuth() + 180.0);
				else
					strm->setAzimuth(ci.GetAzimuth() - 180.0);

				strm->setDip(-ci.GetDip());
			}
			*/

			break;
		}
	}

	strm->setFlags(ci.GetFlags());
	//strm->setFormat("Steim2");

	int identifier_code = ci.GetDataFormatIdentifierCode();
	for ( size_t i=0; i<adc->dfd.size(); ++i ) {
		DataFormatDictionary dataformat = *adc->dfd[i];
		if ( identifier_code == dataformat.GetDataFormatIdentifierCode() ) {
			const std::vector<std::string> &keys = dataformat.GetDecoderKeys();
			/*
			for ( size_t k = 0; k < keys.size(); ++k )
				cerr << "  " << keys[k];
			cerr << endl;
			*/
			map<vector<string>, string>::iterator p;
			if ( (p = encoding.find(keys)) != encoding.end() )
				strm->setFormat(p->second);
			else
				SEISCOMP_WARNING("No mapping to a name from decoder keys known: %s",
				                 join(keys, "; ").c_str());

			break;
		}
	}

	strm->setRestricted(restricted);
	strm->setShared(shared);

	loc->add(strm.get());
	return strm;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::UpdateStream(ChannelIdentifier& ci, DataModel::StreamPtr strm,
                             bool restricted, bool shared) {
	SEISCOMP_DEBUG("Update seisstream information (%s)", ci.GetChannel().c_str());

	strm->setEnd(GetOptTime(ci.GetEndDate()));
	strm->setDataloggerSerialNumber("xxxx");
	strm->setSensorSerialNumber("yyyy");

	Fraction samprate = double2frac(ci.GetSampleRate());
	strm->setSampleRateNumerator(samprate.first);
	strm->setSampleRateDenominator(samprate.second);

	if(ci.GetChannel().substr(2,1)=="N" || ci.GetChannel().substr(2,1)=="1")
	{
		strm->setDataloggerChannel(1);
		strm->setSensorChannel(1);
	}
	else if(ci.GetChannel().substr(2,1)=="E" || ci.GetChannel().substr(2,1)=="2")
	{
		strm->setDataloggerChannel(2);
		strm->setSensorChannel(2);
	}
	else
	{
		strm->setDataloggerChannel(0);
		strm->setSensorChannel(0);
	}

	strm->setDepth(ci.GetLocalDepth());
	strm->setAzimuth(ci.GetAzimuth());
	strm->setDip(ci.GetDip());
	strm->setGain(Core::None);
	strm->setGainFrequency(Core::None);

	for ( size_t i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == 0 ) {
			strm->setGain(ci.csg[i]->GetSensitivityGain());
			strm->setGainFrequency(ci.csg[i]->GetFrequency());
			break;
		}
	}

	strm->setFlags(ci.GetFlags());
	strm->setFormat("Steim2");

	int identifier_code = ci.GetDataFormatIdentifierCode();
	for ( size_t i = 0; i < adc->dfd.size(); ++i ) {
		DataFormatDictionary dataformat = *adc->dfd[i];
		if ( identifier_code == dataformat.GetDataFormatIdentifierCode()) {
			const std::vector<std::string> &keys = dataformat.GetDecoderKeys();
			map<vector<string>, string>::iterator p;
			if ( (p = encoding.find(keys)) != encoding.end() )
				strm->setFormat(p->second);
			else
				SEISCOMP_WARNING("No mapping to a name from decoder keys known: %s",
				                 join(keys, "; ").c_str());

			break;
		}
	}

	strm->setRestricted(restricted);
	strm->setShared(shared);

	strm->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::AuxStreamPtr
Inventory::InsertAuxStream(ChannelIdentifier& ci,
                           DataModel::SensorLocationPtr loc,
                           bool restricted, bool shared) {
	SEISCOMP_DEBUG("Insert auxstream information");

	// Adjust strm_start if loc_start was adjusted earlier
	Core::Time strm_start = GetTime(ci.GetStartDate());
	if(strm_start < loc->start())
		strm_start = loc->start();

	DataModel::AuxStreamPtr strm = new DataModel::AuxStream();
	strm->setCode(ci.GetChannel());
	strm->setStart(strm_start);
	strm->setEnd(GetOptTime(ci.GetEndDate()));
	strm->setDevice(station_name + "." + GetInstrumentName(ci.GetInstrument()));
	strm->setFlags(ci.GetFlags());
	strm->setFormat("Steim2");
	strm->setRestricted(restricted);
	strm->setShared(shared);

	loc->add(strm.get());
	return strm;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::UpdateAuxStream(ChannelIdentifier& ci, DataModel::AuxStreamPtr strm,
                                bool restricted, bool shared) {
	SEISCOMP_DEBUG("Update auxstream information");

	strm->setEnd(GetTime(ci.GetEndDate()));
	strm->setDevice(station_name + "." + GetInstrumentName(ci.GetInstrument()));
	strm->setFlags(ci.GetFlags());
	strm->setFormat("Steim2");
	strm->setRestricted(restricted);
	strm->setShared(shared);

	strm->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::ProcessDatalogger(ChannelIdentifier& ci, DataModel::StreamPtr strm) {
	SEISCOMP_DEBUG("Start processing datalogger information ");

	string dataloggerName = strm->sensorLocation()->station()->network()->code() + "." +
	                        strm->sensorLocation()->station()->code() + "." + strm->sensorLocation()->code() +
	                        strm->code() + "." +
	                        date2str(strm->start());
#if LOG_STAGES
	cerr << " + DL " << dataloggerName << endl;
#endif

	DataModel::DataloggerPtr dlg = inventory->datalogger(DataModel::DataloggerIndex(dataloggerName));
	if ( !dlg )
		dlg = InsertDatalogger(ci, strm, dataloggerName);
	else
		UpdateDatalogger(ci, dlg, strm);

	strm->setDatalogger(dlg->publicID());

	ProcessDecimation(ci, dlg, strm);

	DataModel::DecimationPtr deci = dlg->decimation(DataModel::DecimationIndex(strm->sampleRateNumerator(), strm->sampleRateDenominator()));
	if ( !deci ) {
		SEISCOMP_ERROR("decimation %d/%d Hz of %s not found",
		               strm->sampleRateNumerator(),
		               strm->sampleRateDenominator(), dlg->name().c_str());
		return;
	}

	Stages stages;
	GetStages(stages, ci);

	string analogueChain, digitalChain;
	bool hasDigitizerGain = false;
	SequenceNumber lastStage;

	for ( size_t i = 0; i < stages.size(); ++i ) {
		bool isAnalogue;
		if ( isAnalogDataloggerStage(adc, stages[i]) )
			isAnalogue = true;
		else if ( isDigitalDataloggerStage(adc, stages[i]) )
			isAnalogue = false;
		else
			continue;

		// Same sequence numbers -> this is most likely a continuation
		// blockette that should have been handled by the reader. Issue an
		// error
		if ( lastStage && *lastStage == stages[i].stage ) {
			SEISCOMP_ERROR("Same stage number in subsequent responses: this must be checked!");
			throw Core::GeneralException("Undefined behaviour expected, conversion aborted");
		}

		response_index = stages[i].index;

		double stageGain = 0;
		for ( size_t j = 0; j< ci.csg.size(); ++j ) {
			if ( ci.csg[j]->GetStageSequenceNumber() == (int)stages[i].stage ) {
				stageGain = ci.csg[j]->GetSensitivityGain();
				break;
			}
		}

#if LOG_STAGES
		cerr << " + D " << stages[i].type.toString() << " "
		     << adc->UnitName(stages[i].inputUnit) << " "
		     << adc->UnitName(stages[i].outputUnit) << " "
		     << stageGain;
#endif

		if ( IsDummy(ci, stages[i], stageGain) ) {
			bool ignoreStage = false;

			// Ignore dummy stages without a defining gain
			if ( stageGain == 1.0 ) {
#if LOG_STAGES
				cerr << " (dummy)";
				ignoreStage = true;
#endif
			}

			// Potential preamplifier gain
			if ( !hasDigitizerGain && isADCStage(adc, stages[i]) ) {
				hasDigitizerGain = true;
				dlg->setGain(stageGain);
#if LOG_STAGES
				cerr << " (digitizer gain)";
				ignoreStage = true;
#endif
			}

			if ( ignoreStage ) {
#if LOG_STAGES
				cerr << endl;
#endif
				continue;
			}
		}

#if LOG_STAGES
		cerr << endl;
#endif

		string instr = channel_name + ".stage_" + ToString<int>(stages[i].stage);
		string responseID;

		switch ( stages[i].type ) {
			case RT_FIR:
			{
				DataModel::ResponseFIRPtr fir = inventory->responseFIR(DataModel::ResponseFIRIndex(instr));
				if ( !fir )
					fir = InsertResponseFIR(ci, instr);
				else
					UpdateResponseFIR(ci, fir);
				responseID = fir->publicID();
				break;
			}
			case RT_RC:
			{
				DataModel::ResponseFIRPtr fir = inventory->responseFIR(DataModel::ResponseFIRIndex(instr));
				if ( !fir )
					fir = InsertRespCoeff(ci, instr);
				else
					UpdateRespCoeff(ci, fir);
				responseID = fir->publicID();
				break;
			}
			case RT_PAZ:
			{
				DataModel::ResponsePAZPtr rp = inventory->responsePAZ(DataModel::ResponsePAZIndex(instr));
				if ( !rp )
					rp = InsertResponsePAZ(ci, instr);
				else
					UpdateResponsePAZ(ci, rp);
				responseID = rp->publicID();
				break;
			}
			case RT_Poly:
			{
				DataModel::ResponsePolynomialPtr poly = inventory->responsePolynomial(DataModel::ResponsePolynomialIndex(instr));
				if ( !poly )
					poly = InsertResponsePolynomial(ci, instr);
				else
					UpdateResponsePolynomial(ci, poly);
				responseID = poly->publicID();
				break;
			}
			case RT_FAP:
			{
				DataModel::ResponseFAPPtr fap = inventory->responseFAP(DataModel::ResponseFAPIndex(instr));
				if ( !fap )
					fap = InsertResponseFAP(ci, instr);
				else
					UpdateResponseFAP(ci, fap);
				responseID = fap->publicID();
				break;
			}
			default:
				SEISCOMP_ERROR("Invalid response type");
				continue;
		}

		if ( isAnalogue ) {
			if ( !analogueChain.empty() ) analogueChain += " ";
			analogueChain += responseID;
		}
		else {
			if ( !digitalChain.empty() ) digitalChain += " ";
			digitalChain += responseID;
		}

		response_index = Core::None;
		lastStage = stages[i].stage;
	}

	if ( !analogueChain.empty() )
		deci->setAnalogueFilterChain(str2blob(analogueChain));

	if ( !digitalChain.empty() )
		deci->setDigitalFilterChain(str2blob(digitalChain));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::DataloggerPtr Inventory::InsertDatalogger(ChannelIdentifier& ci, DataModel::StreamPtr strm, const string& name) {
	SEISCOMP_DEBUG("Voeg nieuwe datalogger toe");

	double drift = ci.GetMaxClockDrift() * \
		double(strm->sampleRateNumerator()) / double(strm->sampleRateDenominator());

	DataModel::DataloggerPtr dlg = DataModel::Datalogger::Create();
	dlg->setName(name);
	dlg->setMaxClockDrift(drift);
	dlg->setGain(1.0);

	inventory->add(dlg.get());

	return dlg;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::UpdateDatalogger(ChannelIdentifier& ci, DataModel::DataloggerPtr dlg, DataModel::StreamPtr strm) {
	SEISCOMP_DEBUG("wijzig datalogger");

	double drift = ci.GetMaxClockDrift() * (double)strm->sampleRateNumerator() / (double)strm->sampleRateDenominator();

	dlg->setMaxClockDrift(drift);
	dlg->setGain(1.0);

	dlg->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::ProcessDecimation(ChannelIdentifier& ci, DataModel::DataloggerPtr dlg, DataModel::StreamPtr strm) {
	SEISCOMP_DEBUG("Start processing decimation for %d/%d sps", strm->sampleRateNumerator(), strm->sampleRateDenominator());

	DataModel::DecimationPtr deci = dlg->decimation(DataModel::DecimationIndex(strm->sampleRateNumerator(), strm->sampleRateDenominator()));
	if(!deci)
		InsertDecimation(ci, dlg, strm);
	else
		UpdateDecimation(ci, deci, strm);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::InsertDecimation(ChannelIdentifier& ci, DataModel::DataloggerPtr dlg, DataModel::StreamPtr strm) {
	SEISCOMP_DEBUG("Voeg nieuwe decimation toe");

	DataModel::DecimationPtr deci = new DataModel::Decimation();
	deci->setSampleRateNumerator(strm->sampleRateNumerator());
	deci->setSampleRateDenominator(strm->sampleRateDenominator());
	deci->setAnalogueFilterChain(Core::None);
	deci->setDigitalFilterChain(Core::None);

	dlg->add(deci.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::UpdateDecimation(ChannelIdentifier& ci, DataModel::DecimationPtr deci, DataModel::StreamPtr strm) {
	SEISCOMP_DEBUG("Wijzig decimation");

	deci->setAnalogueFilterChain(Core::None);
	deci->setDigitalFilterChain(Core::None);

	deci->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::ResponseFIRPtr Inventory::InsertRespCoeff(ChannelIdentifier &ci, const string &name) {
	int seq = (int)*response_index;
	int seqnum = ci.rc[seq]->GetStageSequenceNumber();
	int non = 0, number_of_loops = 0;
	string numerators;

	SEISCOMP_DEBUG("Insert response fir, for stage %d", seqnum);

	DataModel::ResponseFIRPtr rf = DataModel::ResponseFIR::Create();

	rf->setName(name);
	rf->setGain(Core::None);
	rf->setDecimationFactor(Core::None);
	rf->setDelay(Core::None);
	rf->setCorrection(Core::None);

	for ( size_t i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == seqnum )
			rf->setGain(ci.csg[i]->GetSensitivityGain());
	}

	for ( size_t i = 0; i < ci.dec.size(); ++i ) {
		if ( ci.dec[i]->GetStageSequenceNumber() == seqnum ) {
			rf->setDecimationFactor(ci.dec[i]->GetDecimationFactor());
			rf->setDelay(ci.dec[i]->GetEstimatedDelay() * ci.dec[i]->GetInputSampleRate());
			rf->setCorrection(ci.dec[i]->GetCorrectionApplied() * ci.dec[i]->GetInputSampleRate());
		}
	}

	while ( seq < (int)ci.rc.size() && seqnum == ci.rc[seq]->GetStageSequenceNumber() ) {
		non += ci.rc[seq]->GetNumberOfNumerators();
		numerators += ci.rc[seq]->GetNumerators();
		++seq;
		++number_of_loops;
	}

	rf->setNumberOfCoefficients(non);
	rf->setSymmetry("A");
	rf->setCoefficients(parseRealArray(numerators));
	check_fir(rf, _fixedErrors);

	inventory->add(rf.get());

	return rf;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::UpdateRespCoeff(ChannelIdentifier& ci, DataModel::ResponseFIRPtr rf) {
	int seq = (int)*response_index;
	int seqnum = ci.rc[seq]->GetStageSequenceNumber();
	int non = 0, number_of_loops = 0;
	string numerators;

	SEISCOMP_DEBUG("Update response fir, for sequence number: %d", seqnum);

	rf->setGain(Core::None);
	rf->setDecimationFactor(Core::None);
	rf->setDelay(Core::None);
	rf->setCorrection(Core::None);

	for ( size_t i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == seqnum )
			rf->setGain(ci.csg[i]->GetSensitivityGain());
	}

	for ( size_t i = 0; i < ci.dec.size(); ++i ) {
		if ( ci.dec[i]->GetStageSequenceNumber() == seqnum ) {
			rf->setDecimationFactor(ci.dec[i]->GetDecimationFactor());
			rf->setDelay(ci.dec[i]->GetEstimatedDelay() * ci.dec[i]->GetInputSampleRate());
			rf->setCorrection(ci.dec[i]->GetCorrectionApplied() * ci.dec[i]->GetInputSampleRate());
		}
	}

	while ( seq < (int)ci.rc.size() && seqnum == ci.rc[seq]->GetStageSequenceNumber() ) {
		non += ci.rc[seq]->GetNumberOfNumerators();
		numerators += ci.rc[seq]->GetNumerators();
		++seq;
		++number_of_loops;
	}

	rf->setNumberOfCoefficients(non);
	rf->setSymmetry("A");
	rf->setCoefficients(parseRealArray(numerators));
	check_fir(rf, _fixedErrors);

	rf->update();
}

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::ResponseFIRPtr Inventory::InsertResponseFIR(ChannelIdentifier& ci, const string &name) {
	int seq = (int)*response_index;
	int seqnum = ci.firr[seq]->GetStageSequenceNumber();
	int non = 0, number_of_loops = 0;
	string numerators;

	SEISCOMP_DEBUG("Insert response fir for stage %d", seqnum);

	DataModel::ResponseFIRPtr rf = DataModel::ResponseFIR::Create();

	rf->setName(name);
	rf->setGain(Core::None);
	rf->setDecimationFactor(Core::None);
	rf->setDelay(Core::None);
	rf->setCorrection(Core::None);

	char sc = ci.firr[seq]->GetSymmetryCode();

	for ( size_t i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == seqnum )
			rf->setGain(ci.csg[i]->GetSensitivityGain());
	}

	for ( size_t i = 0; i < ci.dec.size(); ++i ) {
		if ( ci.dec[i]->GetStageSequenceNumber() == seqnum ) {
			rf->setDecimationFactor(ci.dec[i]->GetDecimationFactor());
			rf->setDelay(ci.dec[i]->GetEstimatedDelay() * ci.dec[i]->GetInputSampleRate());
			rf->setCorrection(ci.dec[i]->GetCorrectionApplied() * ci.dec[i]->GetInputSampleRate());
		}
	}

	while ( seq < (int)ci.firr.size() && seqnum == ci.firr[seq]->GetStageSequenceNumber() ) {
		non += ci.firr[seq]->GetNumberOfCoefficients();
		numerators += ci.firr[seq]->GetCoefficients();
		++seq;
		++number_of_loops;
	}

	rf->setNumberOfCoefficients(non);
	rf->setSymmetry(string(&sc, 1));
	rf->setCoefficients(parseRealArray(numerators));
	check_fir(rf, _fixedErrors);

	inventory->add(rf.get());

	return rf;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::UpdateResponseFIR(ChannelIdentifier& ci, DataModel::ResponseFIRPtr rf) {
	int seq = (int)*response_index;
	int seqnum = ci.firr[seq]->GetStageSequenceNumber();
	int non = 0, number_of_loops = 0;
	string numerators;

	SEISCOMP_DEBUG("Update response fir, for sequence number: %d", seqnum);

	rf->setGain(Core::None);
	rf->setDecimationFactor(Core::None);
	rf->setDelay(Core::None);
	rf->setCorrection(Core::None);

	char sc = ci.firr[seq]->GetSymmetryCode();

	for ( size_t i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == seqnum )
			rf->setGain(ci.csg[i]->GetSensitivityGain());
	}

	for ( size_t i = 0; i < ci.dec.size(); ++i ) {
		if ( ci.dec[i]->GetStageSequenceNumber() == seqnum ) {
			rf->setDecimationFactor(ci.dec[i]->GetDecimationFactor());
			rf->setDelay(ci.dec[i]->GetEstimatedDelay() * ci.dec[i]->GetInputSampleRate());
			rf->setCorrection(ci.dec[i]->GetCorrectionApplied() * ci.dec[i]->GetInputSampleRate());
		}
	}

	while ( seq < (int)ci.firr.size() && seqnum == ci.firr[seq]->GetStageSequenceNumber() ) {
		non += ci.firr[seq]->GetNumberOfCoefficients();
		numerators += ci.firr[seq]->GetCoefficients();
		++seq;
		++number_of_loops;
	}

	rf->setNumberOfCoefficients(non);
	rf->setSymmetry(string(&sc, 1));
	rf->setCoefficients(parseRealArray(numerators));
	check_fir(rf, _fixedErrors);

	rf->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::ProcessPAZSensor(ChannelIdentifier& ci, DataModel::StreamPtr strm, int stageSequenceNumber) {
	response_index = getSensorStage(ci.rpz, adc);
	if ( response_index )
		strm->setGainUnit(adc->UnitName(ci.rpz[*response_index]->GetSignalInUnits()));

	SEISCOMP_DEBUG("Start processing paz sensor information of stage %d",
	               response_index ? ci.rpz[*response_index]->GetStageSequenceNumber() : -1);

	string sensorName = strm->sensorLocation()->station()->network()->code() + "." +
	                    strm->sensorLocation()->station()->code() + "." + strm->sensorLocation()->code() +
	                    strm->code() + "." +
	                    date2str(strm->start());

	DataModel::SensorPtr sm = inventory->sensor(DataModel::SensorIndex(sensorName));
	if ( !sm )
		sm = InsertSensor(ci, strm, strm->gainUnit(), sensorName);
	else
		UpdateSensor(ci, sm, strm->gainUnit());

	strm->setSensor(sm->publicID());

	ProcessSensorPAZ(ci, sm);
	ProcessSensorCalibration(ci, sm, strm, stageSequenceNumber);

	DataModel::ResponsePAZPtr rp = inventory->responsePAZ(DataModel::ResponsePAZIndex(sm->name()));
	if ( !rp )
		SEISCOMP_ERROR("poles & zeros response of sensor %s not found", strm->sensor().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::ProcessPolySensor(ChannelIdentifier& ci, DataModel::StreamPtr strm, int stageSequenceNumber) {
	SEISCOMP_DEBUG("Start processing poly sensor information ");

	response_index = getSensorStage(ci.rp, adc);
	if ( response_index )
		strm->setGainUnit(adc->UnitName(ci.rp[*response_index]->GetSignalInUnits()));

	SEISCOMP_DEBUG("Start processing poly sensor information of stage %d",
	               response_index ? ci.rp[*response_index]->GetStageSequenceNumber() : -1);

	string sensorName = station_name + "." + ci.GetChannel().substr(1,2) + strip(ci.GetLocation());

	DataModel::SensorPtr sm = inventory->sensor(DataModel::SensorIndex(sensorName));
	if ( !sm )
		sm = InsertSensor(ci, strm, strm->gainUnit(), sensorName);
	else
		UpdateSensor(ci, sm, strm->gainUnit());

	strm->setSensor(sm->publicID());

	ProcessSensorPolynomial(ci, sm);
	ProcessSensorCalibration(ci, sm, strm, stageSequenceNumber);

	DataModel::ResponsePolynomialPtr rp = inventory->responsePolynomial(DataModel::ResponsePolynomialIndex(sm->name()));
	if ( !rp )
		SEISCOMP_ERROR("polynomial response of sensor %s not found", strm->sensor().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::ProcessFAPSensor(ChannelIdentifier& ci, DataModel::StreamPtr strm, int stageSequenceNumber) {
	SEISCOMP_DEBUG("Start processing fap sensor information");

	response_index = getSensorStage(ci.rl, adc);
	if ( response_index )
		strm->setGainUnit(adc->UnitName(ci.rl[*response_index]->GetSignalInUnits()));

	SEISCOMP_DEBUG("Start processing fap sensor information of stage %d",
	               response_index ? ci.rl[*response_index]->GetStageSequenceNumber() : -1);

	string sensorName = station_name + "." + ci.GetChannel().substr(1,2) + strip(ci.GetLocation());

	DataModel::SensorPtr sm = inventory->sensor(DataModel::SensorIndex(sensorName));
	if ( !sm )
		sm = InsertSensor(ci, strm, strm->gainUnit(), sensorName);
	else
		UpdateSensor(ci, sm, strm->gainUnit());

	strm->setSensor(sm->publicID());

	ProcessSensorFAP(ci, sm);
	ProcessSensorCalibration(ci, sm, strm, stageSequenceNumber);

	DataModel::ResponseFAPPtr rp = inventory->responseFAP(DataModel::ResponseFAPIndex(sm->name()));
	if ( !rp )
		SEISCOMP_ERROR("response list of sensor %s not found", strm->sensor().c_str());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::SensorPtr Inventory::InsertSensor(ChannelIdentifier& ci, DataModel::StreamPtr strm, const string &unit, const string &name) {
	SEISCOMP_DEBUG("Insert sensor");

	int instr = ci.GetInstrument();

	DataModel::SensorPtr sm = DataModel::Sensor::Create();

	sm->setName(name);
	sm->setDescription(GetInstrumentDescription(instr));
	sm->setModel(GetInstrumentName(instr));
	sm->setManufacturer(GetInstrumentManufacturer(instr));
	sm->setType(GetInstrumentType(instr));
	sm->setUnit(unit);
	//sm->setResponse("paz:" + strm->sensor());

	inventory->add(sm.get());

	return sm;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::UpdateSensor(ChannelIdentifier& ci, DataModel::SensorPtr sm, const string &unit) {
	SEISCOMP_DEBUG("Update sensor");

	int instr = ci.GetInstrument();

	sm->setDescription(GetInstrumentDescription(instr));
	sm->setModel(GetInstrumentName(instr));
	sm->setManufacturer(GetInstrumentManufacturer(instr));
	sm->setType(GetInstrumentType(instr));
	sm->setUnit(unit);
	//sm->setResponse("paz:" + sm->name());

	sm->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::ProcessSensorCalibration(ChannelIdentifier& ci, DataModel::SensorPtr sm, DataModel::StreamPtr strm, int stageSequenceNumber) {
	/*
	 * Actually a sensor calibration should not be created automatically. That
	 * be used from the historic values of the sensitivity blockette.
	*/

	/*
	SEISCOMP_DEBUG("Process sensor calibration");

	DataModel::SensorCalibrationPtr cal = sm->sensorCalibration(DataModel::SensorCalibrationIndex(strm->sensorSerialNumber(), strm->sensorChannel(), strm->start()));
	if ( !cal )
		InsertSensorCalibration(ci, sm, strm, stageSequenceNumber);
	else
		UpdateSensorCalibration(ci, cal, strm, stageSequenceNumber);
	*/
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::InsertSensorCalibration(ChannelIdentifier& ci, DataModel::SensorPtr sm, DataModel::StreamPtr strm, int stageSequenceNumber) {
	SEISCOMP_DEBUG("Insert sensor calibration");

	DataModel::SensorCalibrationPtr cal = new DataModel::SensorCalibration();
	cal->setSerialNumber(strm->sensorSerialNumber());
	cal->setChannel(strm->sensorChannel());
	cal->setStart(strm->start());

	try {
		cal->setEnd(strm->end());
	}
	catch ( Core::ValueException ) {
		cal->setEnd(Core::None);
	}

	cal->setGain(Core::None);
	cal->setGainFrequency(Core::None);

	bool found = false;
	for ( size_t i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == stageSequenceNumber ) {
			cal->setGain(ci.csg[i]->GetSensitivityGain());
			cal->setGainFrequency(ci.csg[i]->GetFrequency());
			found = true;
			break;
		}
	}

	if ( found )
		sm->add(cal.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::UpdateSensorCalibration(ChannelIdentifier& ci, DataModel::SensorCalibrationPtr cal, DataModel::StreamPtr strm, int stageSequenceNumber) {
	SEISCOMP_DEBUG("Update sensor calibration");

	try {
		cal->setEnd(strm->end());
	}
	catch ( Core::ValueException ) {
		cal->setEnd(Core::None);
	}

	cal->setGain(Core::None);
	cal->setGainFrequency(Core::None);

	bool found = false;
	for ( size_t i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == stageSequenceNumber ) {
			cal->setGain(ci.csg[i]->GetSensitivityGain());
			cal->setGainFrequency(ci.csg[i]->GetFrequency());
			found = true;
			break;
		}
	}

	if ( found )
		cal->update();
	else
		cal->detach();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::ProcessSensorPAZ(ChannelIdentifier& ci, DataModel::SensorPtr sm) {
	SEISCOMP_DEBUG("Start processing response poles & zeros for stage %d",
	               response_index ? ci.rpz[*response_index]->GetStageSequenceNumber() : -1);

	if ( response_index ) {
		DataModel::ResponsePAZPtr rp = inventory->responsePAZ(DataModel::ResponsePAZIndex(sm->name()));
		if ( !rp )
			rp = InsertResponsePAZ(ci, sm->name());
		else
			UpdateResponsePAZ(ci, rp);

		sm->setResponse(rp->publicID());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::ResponsePAZPtr Inventory::InsertResponsePAZ(ChannelIdentifier& ci, const string &name) {
	int seq = (int)*response_index;
	int seqnum = ci.rpz[seq]->GetStageSequenceNumber();

	SEISCOMP_DEBUG("Insert response paz, for sequence number: %d", seqnum);

	DataModel::ResponsePAZPtr rp = DataModel::ResponsePAZ::Create();

	rp->setName(name);

	char c = ci.rpz[seq]->GetTransferFunctionType();
	rp->setType(string(&c, 1));

	for ( size_t i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == seqnum ) {
			rp->setGain(ci.csg[i]->GetSensitivityGain());
			rp->setGainFrequency(ci.csg[i]->GetFrequency());
		}
	}

	rp->setNormalizationFactor(ci.rpz[seq]->GetAoNormalizationFactor());
	rp->setNormalizationFrequency(ci.rpz[seq]->GetNormalizationFrequency());
	rp->setNumberOfZeros(ci.rpz[seq]->GetNumberOfZeros());
	rp->setNumberOfPoles(ci.rpz[seq]->GetNumberOfPoles());
	rp->setZeros(parseComplexArray(ci.rpz[seq]->GetComplexZeros()));
	rp->setPoles(parseComplexArray(ci.rpz[seq]->GetComplexPoles()));
	check_paz(rp, _fixedErrors);

	inventory->add(rp.get());

	return rp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::UpdateResponsePAZ(ChannelIdentifier& ci, DataModel::ResponsePAZPtr rp) {
	int seq = (int)*response_index;
	int seqnum = ci.rpz[seq]->GetStageSequenceNumber();

	SEISCOMP_DEBUG("Update response poles & zeros");

	char c = ci.rpz[seq]->GetTransferFunctionType();
	rp->setType(string(&c, 1));
	rp->setGain(Core::None);
	rp->setGainFrequency(Core::None);

	for ( size_t i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == seqnum ) {
			rp->setGain(ci.csg[i]->GetSensitivityGain());
			rp->setGainFrequency(ci.csg[i]->GetFrequency());
		}
	}

	rp->setNormalizationFactor(ci.rpz[seq]->GetAoNormalizationFactor());
	rp->setNormalizationFrequency(ci.rpz[seq]->GetNormalizationFrequency());
	rp->setNumberOfZeros(ci.rpz[seq]->GetNumberOfZeros());
	rp->setNumberOfPoles(ci.rpz[seq]->GetNumberOfPoles());
	rp->setZeros(parseComplexArray(ci.rpz[seq]->GetComplexZeros()));
	rp->setPoles(parseComplexArray(ci.rpz[seq]->GetComplexPoles()));
	check_paz(rp, _fixedErrors);

	rp->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::ProcessSensorFAP(ChannelIdentifier& ci, DataModel::SensorPtr sm) {
	SEISCOMP_DEBUG("Start processing response list, for sequence: %d",
	               response_index ? ci.rl[*response_index]->GetStageSequenceNumber() : -1);

	if ( response_index ) {
		DataModel::ResponseFAPPtr rp = inventory->responseFAP(DataModel::ResponseFAPIndex(sm->name()));
		if ( !rp )
			rp = InsertResponseFAP(ci, sm->name());
		else
			UpdateResponseFAP(ci, rp);

		sm->setResponse(rp->publicID());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::ResponseFAPPtr Inventory::InsertResponseFAP(ChannelIdentifier &ci, const string &name) {
	int seq = (int)*response_index;

	SEISCOMP_DEBUG("Insert response list");

	DataModel::ResponseFAPPtr rp = DataModel::ResponseFAP::Create();

	rp->setName(name);
	rp->setNumberOfTuples(ci.rl[seq]->GetNumberOfResponses());

	rp->setTuples(DataModel::RealArray());
	DataModel::RealArray &tuples = rp->tuples();

	const std::vector<ListedResponses> &rl = ci.rl[seq]->GetResponsesListed();
	for ( size_t i = 0; i < rl.size(); ++i ) {
		tuples.content().push_back(rl[i].frequency);
		tuples.content().push_back(rl[i].amplitude);
		tuples.content().push_back(rl[i].phase_angle);
	}

	for( size_t i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == ci.rl[seq]->GetStageSequenceNumber() ) {
			rp->setGain(ci.csg[i]->GetSensitivityGain());
			rp->setGainFrequency(ci.csg[i]->GetFrequency());
		}
	}

	inventory->add(rp.get());

	return rp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::UpdateResponseFAP(ChannelIdentifier &ci, Seiscomp::DataModel::ResponseFAPPtr rp) {
	int seq = (int)*response_index;

	SEISCOMP_DEBUG("Update response list");

	rp->setGain(Core::None);
	rp->setGainFrequency(Core::None);
	rp->setNumberOfTuples(ci.rl[seq]->GetNumberOfResponses());

	rp->setTuples(DataModel::RealArray());
	DataModel::RealArray &tuples = rp->tuples();

	const std::vector<ListedResponses> &rl = ci.rl[seq]->GetResponsesListed();
	for ( size_t i = 0; i < rl.size(); ++i ) {
		tuples.content().push_back(rl[i].frequency);
		tuples.content().push_back(rl[i].amplitude);
		tuples.content().push_back(rl[i].phase_angle);
	}

	for ( size_t i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == ci.rl[seq]->GetStageSequenceNumber() ) {
			rp->setGain(ci.csg[i]->GetSensitivityGain());
			rp->setGainFrequency(ci.csg[i]->GetFrequency());
		}
	}

	rp->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::ProcessSensorPolynomial(ChannelIdentifier& ci, DataModel::SensorPtr sm) {
	SEISCOMP_DEBUG("Start processing response polynomial, for sequence: %d",
	               response_index ? ci.rp[*response_index]->GetStageSequenceNumber() : -1);

	if ( response_index ) {
		DataModel::ResponsePolynomialPtr rp = inventory->responsePolynomial(DataModel::ResponsePolynomialIndex(sm->name()));
		if(!rp)
			rp = InsertResponsePolynomial(ci, sm->name());
		else
			UpdateResponsePolynomial(ci, rp);

		sm->setResponse(rp->publicID());
		sm->setLowFrequency(ci.rp[*response_index]->GetLowerValidFrequencyBound());
		sm->setHighFrequency(ci.rp[*response_index]->GetUpperValidFrequencyBound());
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
DataModel::ResponsePolynomialPtr Inventory::InsertResponsePolynomial(ChannelIdentifier& ci, const string &name) {
	int seq = (int)*response_index;

	SEISCOMP_DEBUG("Voeg nieuwe response polynomial");

	DataModel::ResponsePolynomialPtr rp = DataModel::ResponsePolynomial::Create();

	rp->setName(name);

	for ( size_t i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == ci.rp[seq]->GetStageSequenceNumber() ) {
			rp->setGain(ci.csg[i]->GetSensitivityGain());
			rp->setGainFrequency(ci.csg[i]->GetFrequency());
		}
	}

	char a;
	a = ci.rp[seq]->GetValidFrequencyUnits();
	rp->setFrequencyUnit(string(&a, 1));
	a = ci.rp[seq]->GetPolynomialApproximationType();
	rp->setApproximationType(string(&a, 1));
	rp->setApproximationLowerBound(ci.rp[seq]->GetLowerBoundOfApproximation());
	rp->setApproximationUpperBound(ci.rp[seq]->GetUpperBoundOfApproximation());
	rp->setApproximationError(ci.rp[seq]->GetMaximumAbsoluteError());
	rp->setNumberOfCoefficients(ci.rp[seq]->GetNumberOfPcoeff());
	rp->setCoefficients(parseRealArray(ci.rp[seq]->GetPolynomialCoefficients()));

	inventory->add(rp.get());

	return rp;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::UpdateResponsePolynomial(ChannelIdentifier& ci, DataModel::ResponsePolynomialPtr rp) {
	int seq = (int)*response_index;

	rp->setGain(Core::None);
	rp->setGainFrequency(Core::None);

	for ( size_t i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == ci.rp[seq]->GetStageSequenceNumber() ) {
			rp->setGain(ci.csg[i]->GetSensitivityGain());
			rp->setGainFrequency(ci.csg[i]->GetFrequency());
		}
	}

	char a;
	a = ci.rp[seq]->GetValidFrequencyUnits();
	rp->setFrequencyUnit(string(&a, 1));
	a = ci.rp[seq]->GetPolynomialApproximationType();
	rp->setApproximationLowerBound(ci.rp[seq]->GetLowerBoundOfApproximation());
	rp->setApproximationUpperBound(ci.rp[seq]->GetUpperBoundOfApproximation());
	rp->setApproximationError(ci.rp[seq]->GetMaximumAbsoluteError());
	rp->setNumberOfCoefficients(ci.rp[seq]->GetNumberOfPcoeff());
	rp->setCoefficients(parseRealArray(ci.rp[seq]->GetPolynomialCoefficients()));

	rp->update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string Inventory::GetNetworkDescription(int lookup) {
	SEISCOMP_DEBUG("Getting the description of the network");

	string desc;
	for ( size_t i = 0; i < adc->ga.size(); ++i ) {
		GenericAbbreviation genabb = *adc->ga[i];
		if ( lookup == genabb.GetLookup() ) {
			desc = genabb.GetDescription();
			break;
		}
	}

	return desc;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string Inventory::GetInstrumentDescription(int lookup) {
	for ( size_t i = 0; i < adc->ga.size(); ++i ) {
		GenericAbbreviation genabb = *adc->ga[i];
		if ( lookup == genabb.GetLookup() ) {
			return genabb.GetDescription();
		}
	}

	return string();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string Inventory::GetInstrumentName(int lookup) {
	SEISCOMP_DEBUG("Getting the name of the instrument");

	string name;
	for ( size_t i = 0; i < adc->ga.size(); ++i ) {
		GenericAbbreviation genabb = *adc->ga[i];

		if ( lookup == genabb.GetLookup() ) {
			name = genabb.GetDescription();
			vector<string> instrument_info = SplitStrings(name, LINE_SEPARATOR);
			int size = instrument_info.size();
			if ( size == 1 ) {
				vector<string> extra = SplitStrings(name, ':');
				if ( extra.size() == 1 )
					name = instrument_info[0];
				else
					name = extra[0];
			}
			else if ( size == 3 )
				name = SplitString(instrument_info[1], '/');
			else if ( size > 1 )
				name = instrument_info[1];
		}
	}

	return name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string Inventory::GetInstrumentType(int lookup) {
	SEISCOMP_DEBUG("Getting the type of the instrument");

	string name;
	for ( size_t i = 0; i < adc->ga.size(); ++i ) {
		GenericAbbreviation genabb = *adc->ga[i];

		if ( lookup == genabb.GetLookup() ) {
			name = genabb.GetDescription();
			vector<string> instrument_info = SplitStrings(name, LINE_SEPARATOR);
			int size = instrument_info.size();
			if ( size == 3 ) {
				vector<string> name_type = SplitStrings(instrument_info[1], '/');
				size = name_type.size();
				if ( size == 2 )
					name = name_type[1];
				else
					name = "";
			}
			else
				name = "";
		}
	}

	return name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string Inventory::GetInstrumentManufacturer(int lookup) {
	SEISCOMP_DEBUG("Getting the manufacturer of the instrument");

	string name;
	for ( size_t i = 0; i < adc->ga.size(); ++i ) {
		GenericAbbreviation genabb = *adc->ga[i];

		if ( lookup == genabb.GetLookup() ) {
			name = genabb.GetDescription();
			vector<string> instrument_info = SplitStrings(name, LINE_SEPARATOR);
			int size = instrument_info.size();

			if ( size > 2 )
				name = instrument_info[0];
			else
				name = "";
		}
	}

	return name;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
string Inventory::GetStationInstrument(int lookup) {
	string instr;

	if ( !GetInstrumentName(lookup).empty() )
		instr = SplitString(station_name, LINE_SEPARATOR)+"."+ GetInstrumentName(lookup);
	else
		instr = SplitString(station_name, LINE_SEPARATOR);

	return instr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory::SequenceNumber Inventory::GetPAZSequence(ChannelIdentifier& ci, string in, string out) {
	for ( size_t i = 0; i < ci.rpz.size(); ++i ) {
		int seq_in = -1, seq_out = -2;
		int siu = ci.rpz[i]->GetSignalInUnits();
		int sou = ci.rpz[i]->GetSignalOutUnits();

		for( size_t j = 0; j < adc->ua.size(); ++j ) {
			UnitsAbbreviations ua = *adc->ua[j];
			if ( siu == ua.GetLookup() ) {
				if ( ua.GetName() == in )
					seq_in = i;
			}

			if ( sou == ua.GetLookup() ) {
				if ( ua.GetName() == out )
					seq_out = i;
			}
		}

		if ( seq_in == seq_out )
			return seq_in;
	}

	return Core::None;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory::SequenceNumber Inventory::GetPolySequence(ChannelIdentifier& ci, string in, string out) {
	for ( size_t i = 0; i < ci.rp.size(); ++i ) {
		int seq_in = -1, seq_out = -2;
		int siu = ci.rp[i]->GetSignalInUnits();
		int sou = ci.rp[i]->GetSignalOutUnits();
		for ( size_t j = 0; j < adc->ua.size(); ++j ) {
			UnitsAbbreviations ua = *adc->ua[j];
			if ( siu == ua.GetLookup() ) {
				if ( ua.GetName() == in )
					seq_in = i;
			}

			if ( sou == ua.GetLookup() ) {
				if ( ua.GetName() == out )
					seq_out = i;
			}
		}

		if ( seq_in == seq_out )
			return seq_in;
	}

	return Core::None;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory::SequenceNumber Inventory::GetFAPSequence(ChannelIdentifier& ci, string in, string out) {
	for ( size_t i = 0; i < ci.rl.size(); ++i ) {
		int seq_in = -1, seq_out = -2;
		int siu = ci.rl[i]->GetSignalInUnits();
		int sou = ci.rl[i]->GetSignalOutUnits();
		for( size_t j = 0; j < adc->ua.size(); ++j ) {
			UnitsAbbreviations ua = *adc->ua[j];
			if ( siu == ua.GetLookup() ) {
				if ( ua.GetName() == in )
					seq_in = i;
			}

			if ( sou == ua.GetLookup() ) {
				if ( ua.GetName() == out )
					seq_out = i;
			}
		}

		if ( seq_in == seq_out )
			return seq_in;
	}

	return Core::None;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Inventory::ResponseType Inventory::GetSensorResponseType(const ChannelIdentifier &ci, SequenceNumber &stageSequenceNumber) {
	if ( stageSequenceNumber = hasSensorStage(ci.rc, adc) )
		return RT_RC;

	if ( stageSequenceNumber = hasSensorStage(ci.firr, adc) )
		return RT_FIR;

	if ( stageSequenceNumber = hasSensorStage(ci.rpz, adc) )
		return RT_PAZ;

	if ( stageSequenceNumber = hasSensorStage(ci.rp, adc) )
		return RT_Poly;

	if ( stageSequenceNumber = hasSensorStage(ci.rl, adc) )
		return RT_FAP;

	stageSequenceNumber = Core::None;
	return RT_None;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Inventory::GetStages(Stages &stages, const ChannelIdentifier &ci) {
	stages.clear();

	populateStages(stages, ci.rpz, RT_PAZ);
	populateStages(stages, ci.rp, RT_Poly);
	populateStages(stages, ci.rl, RT_FAP);
	populateStages(stages, ci.rc, RT_RC);
	populateStages(stages, ci.firr, RT_FIR);

	sort(stages.begin(), stages.end(), bySequenceNumber);

#if 0
	Stages::iterator it = stages.begin();
	for ( ; it != stages.end(); ++it ) {
		cerr << it->stage << "  " << it->type.toString() << "  "
		     << adc->UnitName(it->inputUnit) << "  "
		     << adc->UnitName(it->outputUnit) << endl;
	}
#endif
}
