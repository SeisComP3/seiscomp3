/*===========================================================================================================================
    Name:       inventory.C

    Purpose:  	synchronising inventory information

    Language:   C++, ANSI standard.

    Author:     Peter de Boer, KNMI

    Revision:	2008-01-17	0.1	initial version

===========================================================================================================================*/
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

using namespace std;
using namespace Seiscomp;

inline string strip(string s)
{
	unsigned int i = 0, j = 0;

	for(i = 0; i < s.length(); ++i)
	{
		if(s[i] != ' ')
			break;
	}

	for(j = s.length(); j; --j)
	{
		if(s[j-1] != ' ')
			break;
	}

	return string(s, i, j);
}

inline DataModel::RealArray parseRealArray(const string &s)
{
	DataModel::RealArray a;
	vector<double> v;

	if(s.empty())
		return a;

	if(!Core::fromString(v, strip(s)))
	{
		SEISCOMP_ERROR("invalid real array: %s", s.c_str());
		return a;
	}

	a.setContent(v);
	return a;
}

inline DataModel::ComplexArray parseComplexArray(const string &s)
{
	DataModel::ComplexArray a;
	vector<complex<double> > v;

	if(s.empty())
		return a;

	if(!Core::fromString(v, strip(s)))
	{
		SEISCOMP_ERROR("invalid complex array: %s", s.c_str());
		return a;
	}

	a.setContent(v);
	return a;
}

inline string blob2str(const DataModel::Blob &b)
{
	return b.content();
}

inline DataModel::Blob str2blob(const string &s)
{
	DataModel::Blob b;
	b.setContent(s);
	return b;
}

inline bool _is_leap(int y)
{
	return (y % 400 == 0 || (y % 4 == 0 && y % 100 != 0));
}

inline int _ldoy(int y, int m)
{
	static const int DOY[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
	return (DOY[m] + (_is_leap(y) && m >= 2));
}

inline string date2str(const Core::Time& t)
{
	int year, month, day;
	t.get(&year, &month, &day);

	if(month < 1 || month > 12 || day < 1 || day > 31)
	{
		SEISCOMP_ERROR("invalid date: month=%d, day=%d", month, day);
		month = 1;
		day = 0;
	}

	char buf[10];
	snprintf(buf, 9, "%d.%03d", year, _ldoy(year, month - 1) + day);
	return string(buf);
}

inline pair<int,int> float2rational(double x) // to be improved
{
	for(int n = 1; n <= 10000; n *= 10)
	{
		int k = int(n * x + 0.5);
		if(k >= 1) return make_pair(k, n);
	}

	return make_pair(0, 1);
}

inline void check_fir(DataModel::ResponseFIRPtr rf, int &errors)
{
	vector<double> &v = rf->coefficients().content();
	int nc = v.size();

	if(rf->numberOfCoefficients() != nc)
	{
		SEISCOMP_ERROR("expected %d coefficients, found %d", rf->numberOfCoefficients(), nc);
		rf->setNumberOfCoefficients(nc);
		++errors;
	}

	if(nc == 0 || rf->symmetry() != "A")
		return;

	int i = 0;
	for(; 2 * i < nc; ++i)
		if(v[i] != v[nc - 1 - i]) break;

	if(2 * i > nc)
	{
		rf->setNumberOfCoefficients(i);
		rf->setSymmetry("B");
		v.resize(i);
		SEISCOMP_DEBUG("A(%d) -> B(%d)", nc, i);
	}
	else if(2 * i == nc)
	{
		rf->setNumberOfCoefficients(i);
		rf->setSymmetry("C");
		v.resize(i);
		SEISCOMP_DEBUG("A(%d) -> C(%d)", nc, i);
	}
	else
	{
		SEISCOMP_DEBUG("A(%d) -> A(%d)", nc, nc);
	}
}

inline void check_paz(DataModel::ResponsePAZPtr rp, int &errors)
{
	if(rp->numberOfPoles() != (int)rp->poles().content().size())
	{
		SEISCOMP_ERROR("expected %d poles, found %lu", rp->numberOfPoles(), (unsigned long)rp->poles().content().size());
		rp->setNumberOfPoles(rp->poles().content().size());
		++errors;
	}

	if(rp->numberOfZeros() != (int)rp->zeros().content().size())
	{
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

/*******************************************************************************
* Function:     SynchronizeInventory                                           *
* Parameters:   none                                                           *
* Returns:      nothing                                                        *
* Description:  start synchronizing the dataless with the inventory database   *
*******************************************************************************/
void Inventory::SynchronizeInventory()
{
	_fixedErrors = 0;
	ProcessStation();
	SEISCOMP_INFO("Finished.");
	// CleanupDatabase();
}

/*******************************************************************************
* Function:     CleanupDatabase                                                *
* Parameters:   none                                                           *
* Returns:      nothing                                                        *
* Description:  start synchronizing the dataless with the inventory database   *
*******************************************************************************/
void Inventory::CleanupDatabase()
{
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

/*******************************************************************************
* Function:     GetComment                                                     *
* Parameters:   StationIdentifier si                                           *
* Returns:      nothing                                                        *
* Description:  adds station comment to qclog                                  *
*******************************************************************************/
void Inventory::GetComment(StationIdentifier& si)
{
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

void Inventory::GetStationComment(Comment &sc, DataModel::WaveformStreamID *wf)
{
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

void Inventory::GetChannelComment(ChannelIdentifier& ci, DataModel::WaveformStreamID *wf)
{
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

Core::Time Inventory::GetTime(string strTime, bool *ok)
{
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

OPT(Core::Time) Inventory::GetOptTime(string strTime)
{
	bool ok;
	Core::Time t = GetTime(strTime, &ok);
	if ( !ok ) return Core::None;
	return t;
}

/*******************************************************************************
* Function:     ProcessStation                                                 *
* Parameters:   none                                                           *
* Returns:      nothing                                                        *
* Description:  check if the current station is under control of ODC, if yes   *
*               than check the start- and endtimes                             *
*******************************************************************************/
void Inventory::ProcessStation()
{
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

/*******************************************************************************
* Function:     InsertStation												    *
* Parameters:   si      	- StationIdentifier with information of the station to check (from dataless)		    *
* Return:	nothing													    *
* Description:	insert the station information in the database with that of the dataless				    *
*******************************************************************************/
DataModel::StationPtr Inventory::InsertStation(StationIdentifier& si, DataModel::NetworkPtr network)
{
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

/*******************************************************************************
* Function:     UpdateStation												    *
* Parameters:   si      	- StationIdentifier with information of the station to check (from dataless)		    *
*		db_start	- starttime that is currently in the database						    *
* Return:	nothing													    *
* Description:	update the station information in the database with that of the dataless				    *
*******************************************************************************/
void Inventory::UpdateStation(StationIdentifier& si, DataModel::StationPtr station)
{
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

/*******************************************************************************
* Function:	ProcessStream												    *
* Parameters:   si 	- information about the station being processed							    *
*		station - information of the station being processed now in the database				    *
* Returns:      nothing                                                                                                     *
* Description:  - select all seisstreams currently present in the database for a given station                              *
*		- loop through all returned seisstreams and compare starttime, endtime and location code with that from	    *
*		  the dataless												    *
*******************************************************************************/
void Inventory::ProcessStream(StationIdentifier& si, DataModel::StationPtr station)
{
	SEISCOMP_DEBUG("Start processing stream information");

	string net_code = strip(si.GetNetworkCode());
	string sta_code = strip(si.GetStationCallLetters());

	//cerr << net_code << "." << sta_code << endl;

	map<pair<pair<double, double>, string>, pair<Core::Time, OPT(Core::Time)> > loc_map;
	for(unsigned int i=0; i<si.ci.size(); i++)
	{
		ChannelIdentifier ci = *si.ci[i];
		string loc_code = strip(ci.GetLocation());
		Core::Time loc_start = GetTime(ci.GetStartDate());
		OPT(Core::Time) loc_end = GetOptTime(ci.GetEndDate());

		//cerr << "  + " << loc_code << ci.GetChannel() << "   " << loc_start.toString("%F %T") << endl;

		map<pair<pair<double, double>, string>, pair<Core::Time, OPT(Core::Time)> >::iterator p;
		if((p = loc_map.find(make_pair(make_pair(ci.GetLatitude(), ci.GetLongitude()), loc_code))) == loc_map.end())
		{
			p = loc_map.insert(make_pair(make_pair(make_pair(ci.GetLatitude(), ci.GetLongitude()), loc_code), make_pair(loc_start, loc_end))).first;
		}
		else
		{
			if(p->second.first > loc_start)
				p->second.first = loc_start;

			if(p->second.second && !loc_end)
				p->second.second = loc_end;
			else if (p->second.second && loc_end && *p->second.second < *loc_end)
				p->second.second = *loc_end;
		}

		map<pair<pair<double, double>, string>, pair<Core::Time, OPT(Core::Time)> >::iterator p1 = loc_map.begin();
		while(p1 != loc_map.end())
		{
			if(p1 != p && p1->first.second == p->first.second && p1->second.first == p->second.first)
			{
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

	for(unsigned int i=0; i<si.ci.size(); i++)
	{
		ChannelIdentifier ci = *si.ci[i];
		station_name = strip(si.GetStationCallLetters()) + "." + date2str(GetTime(ci.GetStartDate()));
		channel_name = strip(si.GetStationCallLetters()) + "." + strip(ci.GetLocation()) + "." + ci.GetChannel() + "." + date2str(GetTime(ci.GetStartDate()));

		string strm_code = ci.GetChannel();
		string loc_code = strip(ci.GetLocation());
		Core::Time sta_start = GetTime(si.GetStartDate());
		Core::Time strm_start = GetTime(ci.GetStartDate());
		Core::Time loc_start;
		OPT(Core::Time) loc_end;

		map<pair<pair<double, double>, string>, pair<Core::Time, OPT(Core::Time)> >::iterator p;
		if((p = loc_map.find(make_pair(make_pair(ci.GetLatitude(), ci.GetLongitude()), loc_code))) != loc_map.end())
		{
			loc_start = p->second.first;
			loc_end = p->second.second;
		}
		else
		{
			SEISCOMP_ERROR("cannot find location (should not happen)");
			exit(1);
		}

		sensor_locations.insert(make_pair(make_pair(make_pair(make_pair(net_code, sta_code), loc_code), sta_start), loc_start));

		DataModel::SensorLocationPtr loc = station->sensorLocation(DataModel::SensorLocationIndex(loc_code, loc_start));

		if(!loc)
			loc = InsertSensorLocation(ci, station, loc_start, loc_end);
		else
			UpdateSensorLocation(ci, loc, loc_start, loc_end);

		// Resolve references and copy them into the channel info
		for(unsigned int i=0; i<ci.rr.size(); i++)
		{
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

// For debugging reasons
#if 0
		cerr << "[" << strm_code << "]" << endl;
		for(unsigned int i=0; i<ci.rpz.size(); i++)
		{
			int siu = ci.rpz[i]->GetSignalInUnits();
			int sou = ci.rpz[i]->GetSignalOutUnits();
			int seq_in = -1, seq_out = -2;
			cerr << " + PAZ ";
			for(unsigned int j=0; j<adc->ua.size(); j++)
			{
				UnitsAbbreviations ua = *adc->ua[j];
				if(siu == ua.GetLookup())
				{
					seq_in = j;
				}
				if(sou == ua.GetLookup())
				{
					seq_out = j;
				}
			}
			if ( seq_in >= 0 )
				cerr << adc->ua[seq_in]->GetName();
			else
				cerr << "-";
			cerr << ":";
			if ( seq_out >= 0 )
				cerr << adc->ua[seq_out]->GetName();
			else
				cerr << "-";
			cerr << endl;
		}
		for(unsigned int i=0; i<ci.rp.size(); i++)
		{
			int siu = ci.rp[i]->GetSignalInUnits();
			int sou = ci.rp[i]->GetSignalOutUnits();
			int seq_in = -1, seq_out = -2;
			cerr << " + POLY ";
			for(unsigned int j=0; j<adc->ua.size(); j++)
			{
				UnitsAbbreviations ua = *adc->ua[j];
				if(siu == ua.GetLookup())
				{
					seq_in = j;
				}
				if(sou == ua.GetLookup())
				{
					seq_out = j;
				}
			}
			if ( seq_in >= 0 )
				cerr << adc->ua[seq_in]->GetName();
			else
				cerr << "-";
			cerr << ":";
			if ( seq_out >= 0 )
				cerr << adc->ua[seq_out]->GetName();
			else
				cerr << "-";
			cerr << endl;
		}
		for(unsigned int i=0; i<ci.firr.size(); i++)
		{
			int siu = ci.firr[i]->GetSignalInUnits();
			int sou = ci.firr[i]->GetSignalOutUnits();
			int seq_in = -1, seq_out = -2;
			cerr << " + FIRR ";
			for(unsigned int j=0; j<adc->ua.size(); j++)
			{
				UnitsAbbreviations ua = *adc->ua[j];
				if(siu == ua.GetLookup())
				{
					seq_in = j;
				}
				if(sou == ua.GetLookup())
				{
					seq_out = j;
				}
			}
			if ( seq_in >= 0 )
				cerr << adc->ua[seq_in]->GetName();
			else
				cerr << "-";
			cerr << ":";
			if ( seq_out >= 0 )
				cerr << adc->ua[seq_out]->GetName();
			else
				cerr << "-";
			cerr << endl;
		}
#endif

		if(IsPAZStream(ci) || IsPolyStream(ci) || IsFAPStream(ci))
		{
			/* pair<set<pair<pair<pair<pair<pair<string, string>, string>, string>, Core::Time>, Core::Time> >::iterator, bool> ins = \
			*/
			seis_streams.insert(make_pair(make_pair(make_pair(make_pair(make_pair(make_pair(net_code, sta_code), strm_code), loc_code), sta_start), strm_start), loc_start));

			DataModel::StreamPtr strm = loc->stream(DataModel::StreamIndex(strm_code, strm_start));

			if(!strm)
				strm = InsertStream(ci, loc, station->restricted(), station->shared());
			else
				UpdateStream(ci, strm, station->restricted(), station->shared()); //, ins.second);

			// ProcessComponent(ci, strm);
			ProcessDatalogger(ci, strm);

			if(IsPAZStream(ci))
				ProcessPAZSensor(ci, strm);
			else if (IsPolyStream(ci))
				ProcessPolySensor(ci, strm);
			else
				ProcessFAPSensor(ci, strm);
		}
		else
		{
			aux_streams.insert(make_pair(make_pair(make_pair(make_pair(make_pair(make_pair(net_code, sta_code), strm_code), loc_code), sta_start), strm_start), loc_start));

			DataModel::AuxStreamPtr strm = loc->auxStream(DataModel::AuxStreamIndex(strm_code, strm_start));

			if(!strm)
				strm = InsertAuxStream(ci, loc, station->restricted(), station->shared());
			else
				UpdateAuxStream(ci, strm, station->restricted(), station->shared());
		}
	}
}

/*******************************************************************************
* Function:     InsertSensorLocation											    *
* Parameters:   ci      - information of a channel as it is stored in the dataless					    *
*		station	- information of the station being processed now in the database				    *
* Returns:	nothing													    *
* Description:	insert a new channel(seisstream)									    *
*******************************************************************************/
DataModel::SensorLocationPtr Inventory::InsertSensorLocation(ChannelIdentifier& ci, DataModel::StationPtr station, const Core::Time& loc_start, const OPT(Core::Time)& loc_end)
{
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

/*******************************************************************************
* Function:     UpdateSensorLocation											    *
* Parameters:   ci      - information of a channel as it is stored in the dataless                                          *
*               channel - information of a channel as it is stored in the database                                          *
* Returns:	nothing													    *
* Description:	update the current channel with information from dataless						    *
*******************************************************************************/
void Inventory::UpdateSensorLocation(ChannelIdentifier& ci, DataModel::SensorLocationPtr loc, const Core::Time& loc_start, const OPT(Core::Time)& loc_end)
{
	SEISCOMP_DEBUG("Update sensor location information (%s)", ci.GetChannel().c_str());

	loc->setEnd(loc_end);
	loc->setLatitude(ci.GetLatitude());
	loc->setLongitude(ci.GetLongitude());
	loc->setElevation(ci.GetElevation());

	loc->update();
}

/*******************************************************************************
* Function:     InsertStream											    *
* Parameters:   ci      - information of a channel as it is stored in the dataless					    *
*		station	- information of the station being processed now in the database				    *
* Returns:	nothing													    *
* Description:	insert a new channel(seisstream)									    *
*******************************************************************************/
DataModel::StreamPtr Inventory::InsertStream(ChannelIdentifier& ci, DataModel::SensorLocationPtr loc, bool restricted, bool shared)
{
	pair<int,int> samprate = float2rational(ci.GetSampleRate());
	if ( samprate.first == 0 || samprate.second == 0 ) {
		SEISCOMP_WARNING("%s: invalid sample rate %.2f -> checking for valid decimations",
		                 ci.GetChannel().c_str(), ci.GetSampleRate());

		samprate = float2rational(ci.GetMaximumInputDecimationSampleRate());
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
	strm->setGain(0.0);
	strm->setGainFrequency(0.0);

	for(unsigned int i = 0; i < ci.csg.size(); ++i)
	{
		if(ci.csg[i]->GetStageSequenceNumber() == 0)
		{
			strm->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
			strm->setGainFrequency(ci.csg[i]->GetFrequency());

			if(ci.csg[i]->GetSensitivityGain() < 0)
			{
				if(ci.GetAzimuth() < 180.0)
					strm->setAzimuth(ci.GetAzimuth() + 180.0);
				else
					strm->setAzimuth(ci.GetAzimuth() - 180.0);

				strm->setDip(-ci.GetDip());
			}

			break;
		}
	}

	strm->setFlags(ci.GetFlags());
	strm->setFormat("Steim2");

	int identifier_code = ci.GetDataFormatIdentifierCode();
	for(unsigned int i=0; i<adc->dfd.size(); i++)
	{
		DataFormatDictionary dataformat = *adc->dfd[i];
		if(identifier_code == dataformat.GetDataFormatIdentifierCode())
		{
			map<vector<string>, string>::iterator p;
			if((p = encoding.find(dataformat.GetDecoderKeys())) != encoding.end())
				strm->setFormat(p->second);

			break;
		}
	}

	strm->setRestricted(restricted);
	strm->setShared(shared);

	loc->add(strm.get());
	return strm;
}

/*******************************************************************************
* Function:     UpdateStream											    *
* Parameters:   ci      - information of a channel as it is stored in the dataless                                          *
*               channel - information of a channel as it is stored in the database                                          *
* Returns:	nothing													    *
* Description:	update the current channel with information from dataless						    *
*******************************************************************************/
void Inventory::UpdateStream(ChannelIdentifier& ci, DataModel::StreamPtr strm, bool restricted, bool shared) //, bool FirstComponent)
{
	SEISCOMP_DEBUG("Update seisstream information (%s)", ci.GetChannel().c_str());

	strm->setEnd(GetOptTime(ci.GetEndDate()));
	strm->setDataloggerSerialNumber("xxxx");
	strm->setSensorSerialNumber("yyyy");

	pair<int,int> samprate = float2rational(ci.GetSampleRate());
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
	strm->setGain(0.0);
	strm->setGainFrequency(0.0);

	for(unsigned int i = 0; i < ci.csg.size(); ++i)
	{
		if(ci.csg[i]->GetStageSequenceNumber() == 0)
		{
			strm->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
			strm->setGainFrequency(ci.csg[i]->GetFrequency());
			break;
		}
	}


	strm->setFlags(ci.GetFlags());
	strm->setFormat("Steim2");

	int identifier_code = ci.GetDataFormatIdentifierCode();
	for(unsigned int i=0; i<adc->dfd.size(); i++)
	{
		DataFormatDictionary dataformat = *adc->dfd[i];
		if(identifier_code == dataformat.GetDataFormatIdentifierCode())
		{
			map<vector<string>, string>::iterator p;
			if((p = encoding.find(dataformat.GetDecoderKeys())) != encoding.end())
				strm->setFormat(p->second);

			break;
		}
	}

	strm->setRestricted(restricted);
	strm->setShared(shared);

	strm->update();
}

/*******************************************************************************
* Function:     InsertAuxStream											    *
* Parameters:   ci      - information of a channel as it is stored in the dataless					    *
*		station	- information of the station being processed now in the database				    *
* Returns:	nothing													    *
* Description:	insert a new channel(auxstream)									    *
*******************************************************************************/
DataModel::AuxStreamPtr Inventory::InsertAuxStream(ChannelIdentifier& ci, DataModel::SensorLocationPtr loc, bool restricted, bool shared)
{
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

/*******************************************************************************
* Function:     UpdateAuxStream											    *
* Parameters:   ci      - information of a channel as it is stored in the dataless                                          *
*               channel - information of a channel as it is stored in the database                                          *
* Returns:	nothing													    *
* Description:	update the current channel with information from dataless						    *
*******************************************************************************/
void Inventory::UpdateAuxStream(ChannelIdentifier& ci, DataModel::AuxStreamPtr strm, bool restricted, bool shared)
{
	SEISCOMP_DEBUG("Update auxstream information");

	strm->setEnd(GetTime(ci.GetEndDate()));
	strm->setDevice(station_name + "." + GetInstrumentName(ci.GetInstrument()));
	strm->setFlags(ci.GetFlags());
	strm->setFormat("Steim2");
	strm->setRestricted(restricted);
	strm->setShared(shared);

	strm->update();
}

/*******************************************************************************
* Function:     ProcessDatalogger                                                                                           *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*               seis_stream     - information of a channel as it is stored in the database                                  *
* Returns:      nothing                                                                                                     *
* Description:  check whether a datalogger has to be added or updated                                                       *
*******************************************************************************/
void Inventory::ProcessDatalogger(ChannelIdentifier& ci, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("Start processing datalogger information ");

	string dataloggerName = station_name + "." + ci.GetChannel() + strip(ci.GetLocation());

	DataModel::DataloggerPtr dlg = inventory->datalogger(DataModel::DataloggerIndex(dataloggerName));
	if(!dlg)
		dlg = InsertDatalogger(ci, strm, dataloggerName);
	else
		UpdateDatalogger(ci, dlg, strm);

	strm->setDatalogger(dlg->publicID());

	ProcessDecimation(ci, dlg, strm);
	ProcessDataloggerCalibration(ci, dlg, strm);
	ProcessDataloggerFIR(ci, dlg, strm);
	ProcessDataloggerPAZ(ci, dlg, strm);
}

/*******************************************************************************
* Function:     InsertDatalogger											    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*               seis_stream     - information of a channel as it is stored in the database                                  *
* Returns:	nothing													    *
* Description:	add a datalogger											    *
*******************************************************************************/
DataModel::DataloggerPtr Inventory::InsertDatalogger(ChannelIdentifier& ci, DataModel::StreamPtr strm, const string& name)
{
	SEISCOMP_DEBUG("Voeg nieuwe datalogger toe");

	double drift = ci.GetMaxClockDrift() * \
		double(strm->sampleRateNumerator()) / double(strm->sampleRateDenominator());

	DataModel::DataloggerPtr dlg = DataModel::Datalogger::Create();
	dlg->setName(name);
	dlg->setDescription(name);
	dlg->setMaxClockDrift(drift);

	dlg->setGain(1.0);

	int sensitivity_stage = GetDataloggerSensitivity(ci);
	if ( sensitivity_stage >= 0 )
		dlg->setGain(fabs(ci.csg[sensitivity_stage]->GetSensitivityGain()));

	inventory->add(dlg.get());

	return dlg;
}

/*******************************************************************************
* Function:     UpdateDatalogger											    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*               datalogger      - the datalogger that is in the database                                                    *
* Returns:      nothing                                                                                                     *
* Description:	update a datalogger											    *
*******************************************************************************/
void Inventory::UpdateDatalogger(ChannelIdentifier& ci, DataModel::DataloggerPtr dlg, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("wijzig datalogger");

	double drift = ci.GetMaxClockDrift() * \
		double(strm->sampleRateNumerator()) / double(strm->sampleRateDenominator());

	dlg->setMaxClockDrift(drift);

	dlg->setGain(1.0);

	int sensitivity_stage = GetDataloggerSensitivity(ci);
	if ( sensitivity_stage >= 0 )
		dlg->setGain(fabs(ci.csg[sensitivity_stage]->GetSensitivityGain()));

	dlg->update();
}

/*******************************************************************************
* Function:     ProcessDecimation											    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*               datalogger      - the datalogger that is in the database                                                    *
*               seis_stream     - information of a channel as it is stored in the database                                  *
* Returns:      nothing                                                                                                     *
* Description:	check whether a decimation should be added or updated							    *
*******************************************************************************/
void Inventory::ProcessDecimation(ChannelIdentifier& ci, DataModel::DataloggerPtr dlg, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("Start processing decimation for %d/%d sps", strm->sampleRateNumerator(), strm->sampleRateDenominator());

	DataModel::DecimationPtr deci = dlg->decimation(DataModel::DecimationIndex(strm->sampleRateNumerator(), strm->sampleRateDenominator()));
	if(!deci)
		InsertDecimation(ci, dlg, strm);
	else
		UpdateDecimation(ci, deci, strm);
}

/*******************************************************************************
* Function:     InsertDecimation											    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*               datalogger      - the datalogger that is in the database                                                    *
*               seis_stream     - information of a channel as it is stored in the database                                  *
* Returns:      nothing                                                                                                     *
* Description:	add a new decimation											    *
*******************************************************************************/
void Inventory::InsertDecimation(ChannelIdentifier& ci, DataModel::DataloggerPtr dlg, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("Voeg nieuwe decimation toe");

	DataModel::DecimationPtr deci = new DataModel::Decimation();
	deci->setSampleRateNumerator(strm->sampleRateNumerator());
	deci->setSampleRateDenominator(strm->sampleRateDenominator());
	deci->setAnalogueFilterChain(Core::None);
	deci->setDigitalFilterChain(Core::None);

	dlg->add(deci.get());
}

/*******************************************************************************
* Function:     UpdateDecimation											    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*               decimation      - the decimation that is in the database                                                    *
*               seis_stream     - information of a channel as it is stored in the database                                  *
* Returns:      nothing                                                                                                     *
* Description:	update an existing decimation belonging to the datalogger in process					    *
*******************************************************************************/
void Inventory::UpdateDecimation(ChannelIdentifier& ci, DataModel::DecimationPtr deci, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("Wijzig decimation");

	deci->setAnalogueFilterChain(Core::None);
	deci->setDigitalFilterChain(Core::None);

	deci->update();
}

/*******************************************************************************
* Function:     ProcessDataloggerCalibration										    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*               dl      	- the datalogger that is in the database                                                    *
*               seis_stream     - information of a channel as it is stored in the database                                  *
* Returns:      nothing                                                                                                     *
* Description:	check whether a datalogger calibration should be added or updated					    *
*******************************************************************************/
void Inventory::ProcessDataloggerCalibration(ChannelIdentifier& ci, DataModel::DataloggerPtr dlg, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("start processing datalogger calibration");

	DataModel::DataloggerCalibrationPtr cal = dlg->dataloggerCalibration(DataModel::DataloggerCalibrationIndex(strm->dataloggerSerialNumber(), strm->dataloggerChannel(), strm->start()));
	if(!cal)
		InsertDataloggerCalibration(ci, dlg, strm);
	else
		UpdateDataloggerCalibration(ci, cal, strm);
}

/*******************************************************************************
* Function:     InsertDataloggerCalibration										    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*               datalogger      - the datalogger that is in the database                                                    *
*               seis_stream     - information of a channel as it is stored in the database                                  *
* Returns:      nothing                                                                                                     *
* Description:	add a new dataloggercalibration										    *
*******************************************************************************/
void Inventory::InsertDataloggerCalibration(ChannelIdentifier& ci, DataModel::DataloggerPtr dlg, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("Voeg datalogger calibration toe");

	DataModel::DataloggerCalibrationPtr cal = new DataModel::DataloggerCalibration();
	cal->setSerialNumber(strm->dataloggerSerialNumber());
	cal->setChannel(strm->dataloggerChannel());
	cal->setStart(strm->start());

	try {
		cal->setEnd(strm->end());
	}
	catch(Core::ValueException) {
		cal->setEnd(Core::None);
	}

	cal->setGain(1.0);
	cal->setGainFrequency(0.0);

	int sensitivity_stage = GetDataloggerSensitivity(ci);
	if ( sensitivity_stage >= 0 ) {
		cal->setGain(fabs(ci.csg[sensitivity_stage]->GetSensitivityGain()));
		cal->setGainFrequency(ci.csg[sensitivity_stage]->GetFrequency());
	}

	dlg->add(cal.get());
}

/*******************************************************************************
* Function:     UpdateDataloggerCalibration										    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*               datalogger      - the datalogger that is in the database                                                    *
*               seis_stream     - information of a channel as it is stored in the database                                  *
* Returns:      nothing                                                                                                     *
* Description:	update an existing dataloggercalibration								    *
*******************************************************************************/
void Inventory::UpdateDataloggerCalibration(ChannelIdentifier& ci, DataModel::DataloggerCalibrationPtr cal, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("Wijzig datalogger calibration");

	try {
		cal->setEnd(strm->end());
	}
	catch(Core::ValueException) {
		cal->setEnd(Core::None);
	}

	cal->setGain(1.0);
	cal->setGainFrequency(0.0);

	int sensitivity_stage = GetDataloggerSensitivity(ci);
	if ( sensitivity_stage >= 0 ) {
		cal->setGain(fabs(ci.csg[sensitivity_stage]->GetSensitivityGain()));
		cal->setGainFrequency(ci.csg[sensitivity_stage]->GetFrequency());
	}

	cal->update();

}

/*******************************************************************************
* Function:     ProcessDataloggerFIR											    	    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*               datalogger      - the datalogger that is in the database                                                    *
* Returns:      nothing                                                                                                     *
* Description:	check whether a new respfir has to be added or an existing should be updated				    *
*		after that the decimation table has to be updated with a new respfir name				    *
*******************************************************************************/
void Inventory::ProcessDataloggerFIR(ChannelIdentifier& ci, DataModel::DataloggerPtr dlg, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("Start processing ResponseFIR information");

	DataModel::DecimationPtr deci = dlg->decimation(DataModel::DecimationIndex(strm->sampleRateNumerator(), strm->sampleRateDenominator()));
	if(!deci)
	{
		SEISCOMP_ERROR("decimation %d/%d sps of %s not found", strm->sampleRateNumerator(),
			strm->sampleRateDenominator(), dlg->name().c_str());
		return;
	}

	unsigned int i=0;
	if(ci.rc.size() > 0 && IsDummy(*ci.rc[0]))
		++i;

	for(; i<ci.rc.size(); i++)
	{
		string instr = channel_name + ".stage_" + ToString<int>(ci.rc[i]->GetStageSequenceNumber());

		DataModel::ResponseFIRPtr rf = inventory->responseFIR(DataModel::ResponseFIRIndex(instr));
		if(!rf)
			rf = InsertRespCoeff(ci, i);
		else
			UpdateRespCoeff(ci, rf, i);

		bool add = true;
		string dfc;

		try { dfc = blob2str(deci->digitalFilterChain()); } catch(Core::ValueException) {}

		string new_dfc = rf->publicID();
		vector<string> digital = SplitStrings(dfc, LINE_SEPARATOR);
		for(unsigned int i=0; i<digital.size(); i++)
			if(digital[i]==new_dfc)
				add = false;
		if(add)
		{
			if(!dfc.empty())
				dfc += " ";

			dfc += new_dfc;
			deci->setDigitalFilterChain(str2blob(dfc));
		}
	}

	for(i = 0; i<ci.firr.size(); i++)
	{
		string instr = channel_name + ".stage_" + ToString<int>(ci.firr[i]->GetStageSequenceNumber());

		DataModel::ResponseFIRPtr rf = inventory->responseFIR(DataModel::ResponseFIRIndex(instr));
		if(!rf)
			rf = InsertResponseFIRr(ci, i);
		else
			UpdateResponseFIRr(ci, rf, i);

		bool add = true;
		string dfc;

		try { dfc = blob2str(deci->digitalFilterChain()); } catch(Core::ValueException) {}

		string new_dfc = rf->publicID();
		vector<string> digital = SplitStrings(dfc, LINE_SEPARATOR);
		for(unsigned int i=0; i<digital.size(); i++)
			if(digital[i]==new_dfc)
				add = false;
		if(add)
		{
			if(!dfc.empty())
				dfc += " ";

			dfc += new_dfc;
			deci->setDigitalFilterChain(str2blob(dfc));
		}
	}
}

/*******************************************************************************
* Function:     InsertRespCoeff												    *
* Parameters:   ci	- information of a channel as it is stored in the dataless	                                    *
* 		seq	- starting number of Response Coefficient							    *
* Returns:      nothing                                                                                                     *
* Description:	add a new respfir											    *
*******************************************************************************/
DataModel::ResponseFIRPtr Inventory::InsertRespCoeff(ChannelIdentifier& ci, unsigned int &seq)
{
	int seqnum = ci.rc[seq]->GetStageSequenceNumber();
	int non = 0, number_of_loops = 0;
	string numerators;

	SEISCOMP_DEBUG("Insert response fir, for sequence number: %d", seqnum);

	DataModel::ResponseFIRPtr rf = DataModel::ResponseFIR::Create();

	rf->setName(channel_name + ".stage_" + ToString<int>(seqnum));
	rf->setGain(0.0);
	rf->setDecimationFactor(1);
	rf->setDelay(0.0);
	rf->setCorrection(0.0);

	for(unsigned int i=0; i< ci.csg.size(); i++)
	{
		if(ci.csg[i]->GetStageSequenceNumber() == seqnum)
			rf->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
	}
	for(unsigned int i=0; i<ci.dec.size(); i++)
	{
		if(ci.dec[i]->GetStageSequenceNumber() == seqnum)
		{
			rf->setDecimationFactor(ci.dec[i]->GetDecimationFactor());
			rf->setDelay(ci.dec[i]->GetEstimatedDelay() * ci.dec[i]->GetInputSampleRate());
			rf->setCorrection(ci.dec[i]->GetCorrectionApplied() * ci.dec[i]->GetInputSampleRate());
		}
	}
	while(seq < ci.rc.size() && seqnum == ci.rc[seq]->GetStageSequenceNumber())
	{
		non += ci.rc[seq]->GetNumberOfNumerators();
		numerators += ci.rc[seq]->GetNumerators();
		++seq;
		++number_of_loops;
	}
	--seq;
	rf->setNumberOfCoefficients(non);
	rf->setSymmetry("A");
	rf->setCoefficients(parseRealArray(numerators));
	check_fir(rf, _fixedErrors);

	inventory->add(rf.get());

	return rf;
}

/*******************************************************************************
* Function:     UpdateRespCoeff												    *
* Parameters:   ci	- information of a channel as it is stored in the dataless 	                                    *
*               resp	- the respfir that is in the database 		                                                    *
*		seq	- starting number of the Response Coefficients							    *
* Returns:      nothing                                                                                                     *
* Description:	update the respfir											    *
*******************************************************************************/
void Inventory::UpdateRespCoeff(ChannelIdentifier& ci, DataModel::ResponseFIRPtr rf, unsigned int &seq)
{
	int seqnum = ci.rc[seq]->GetStageSequenceNumber();
	int non = 0, number_of_loops = 0;
	string numerators;

	SEISCOMP_DEBUG("Update response fir, for sequence number: %d", seqnum);

	rf->setGain(0.0);
	rf->setDecimationFactor(1);
	rf->setDelay(0.0);
	rf->setCorrection(0.0);

	for(unsigned int i=0; i< ci.csg.size(); i++)
	{
		if(ci.csg[i]->GetStageSequenceNumber() == seqnum)
			rf->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
	}
	for(unsigned int i=0; i<ci.dec.size(); i++)
	{
		if(ci.dec[i]->GetStageSequenceNumber() == seqnum)
		{
			rf->setDecimationFactor(ci.dec[i]->GetDecimationFactor());
			rf->setDelay(ci.dec[i]->GetEstimatedDelay() * ci.dec[i]->GetInputSampleRate());
			rf->setCorrection(ci.dec[i]->GetCorrectionApplied() * ci.dec[i]->GetInputSampleRate());
		}
	}
	while(seq < ci.rc.size() && seqnum == ci.rc[seq]->GetStageSequenceNumber())
	{
		non += ci.rc[seq]->GetNumberOfNumerators();
		numerators += ci.rc[seq]->GetNumerators();
		++seq;
		++number_of_loops;
	}
	--seq;
	rf->setNumberOfCoefficients(non);
	rf->setSymmetry("A");
	rf->setCoefficients(parseRealArray(numerators));
	check_fir(rf, _fixedErrors);

	rf->update();
}

/*******************************************************************************
* Function:     InsertResponseFIRr												    *
* Parameters:   ci	- information of a channel as it is stored in the dataless	                                    *
* 		seq	- starting number of Response Coefficient							    *
* Returns:      nothing                                                                                                     *
* Description:	add a new respfir											    *
*******************************************************************************/
DataModel::ResponseFIRPtr Inventory::InsertResponseFIRr(ChannelIdentifier& ci, unsigned int &seq)
{
	int seqnum = ci.firr[seq]->GetStageSequenceNumber();
	int non = 0, number_of_loops = 0;
	string numerators;

	SEISCOMP_DEBUG("Insert response fir, for sequence number: %d", seqnum);

	DataModel::ResponseFIRPtr rf = DataModel::ResponseFIR::Create();

	rf->setName(channel_name + ".stage_" + ToString<int>(seqnum));
	rf->setGain(0.0);
	rf->setDecimationFactor(1);
	rf->setDelay(0.0);
	rf->setCorrection(0.0);

	char sc = ci.firr[seq]->GetSymmetryCode();

	for(unsigned int i=0; i< ci.csg.size(); i++)
	{
		if(ci.csg[i]->GetStageSequenceNumber() == seqnum)
			rf->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
	}
	for(unsigned int i=0; i<ci.dec.size(); i++)
	{
		if(ci.dec[i]->GetStageSequenceNumber() == seqnum)
		{
			rf->setDecimationFactor(ci.dec[i]->GetDecimationFactor());
			rf->setDelay(ci.dec[i]->GetEstimatedDelay() * ci.dec[i]->GetInputSampleRate());
			rf->setCorrection(ci.dec[i]->GetCorrectionApplied() * ci.dec[i]->GetInputSampleRate());
		}
	}
	while(seq < ci.firr.size() && seqnum == ci.firr[seq]->GetStageSequenceNumber())
	{
		non += ci.firr[seq]->GetNumberOfCoefficients();
		numerators += ci.firr[seq]->GetCoefficients();
		++seq;
		++number_of_loops;
	}
	--seq;
	rf->setNumberOfCoefficients(non);
	rf->setSymmetry(string(&sc, 1));
	rf->setCoefficients(parseRealArray(numerators));
	check_fir(rf, _fixedErrors);

	inventory->add(rf.get());

	return rf;
}

/*******************************************************************************
* Function:     UpdateResponseFIRr												    *
* Parameters:   ci	- information of a channel as it is stored in the dataless 	                                    *
*               resp	- the respfir that is in the database 		                                                    *
*		seq	- starting number of the Response Coefficients							    *
* Returns:      nothing                                                                                                     *
* Description:	update the respfir											    *
*******************************************************************************/
void Inventory::UpdateResponseFIRr(ChannelIdentifier& ci, DataModel::ResponseFIRPtr rf, unsigned int &seq)
{
	int seqnum = ci.firr[seq]->GetStageSequenceNumber();
	int non = 0, number_of_loops = 0;
	string numerators;

	SEISCOMP_DEBUG("Update response fir, for sequence number: %d", seqnum);

	rf->setGain(0.0);
	rf->setDecimationFactor(1);
	rf->setDelay(0.0);
	rf->setCorrection(0.0);

	char sc = ci.firr[seq]->GetSymmetryCode();

	for(unsigned int i=0; i< ci.csg.size(); i++)
	{
		if(ci.csg[i]->GetStageSequenceNumber() == seqnum)
			rf->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
	}
	for(unsigned int i=0; i<ci.dec.size(); i++)
	{
		if(ci.dec[i]->GetStageSequenceNumber() == seqnum)
		{
			rf->setDecimationFactor(ci.dec[i]->GetDecimationFactor());
			rf->setDelay(ci.dec[i]->GetEstimatedDelay() * ci.dec[i]->GetInputSampleRate());
			rf->setCorrection(ci.dec[i]->GetCorrectionApplied() * ci.dec[i]->GetInputSampleRate());
		}
	}
	while(seq < ci.firr.size() && seqnum == ci.firr[seq]->GetStageSequenceNumber())
	{
		non += ci.firr[seq]->GetNumberOfCoefficients();
		numerators += ci.firr[seq]->GetCoefficients();
		++seq;
		++number_of_loops;
	}
	--seq;
	rf->setNumberOfCoefficients(non);
	rf->setSymmetry(string(&sc, 1));
	rf->setCoefficients(parseRealArray(numerators));
	check_fir(rf, _fixedErrors);

	rf->update();
}

/*******************************************************************************
* Function:     ProcessDataloggerPAZ											    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*		datalogger	- the datalogger that is in the database						    *
*               seis_stream     - information of a channel as it is stored in the database                                  *
* Returns:      nothing                                                                                                     *
* Description:	this function processes the update/addition of decimation, datalogger_calibration, datalogger_gain, 	    *
*		resp_fir and resp_paz											    *
*******************************************************************************/
void Inventory::ProcessDataloggerPAZ(ChannelIdentifier& ci, DataModel::DataloggerPtr dlg, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("Start processing datalogger analog filter chain");

	DataModel::DecimationPtr deci = dlg->decimation(DataModel::DecimationIndex(strm->sampleRateNumerator(), strm->sampleRateDenominator()));
	if(!deci)
	{
		SEISCOMP_ERROR("decimation %d/%d Hz of %s not found", strm->sampleRateNumerator(),
			strm->sampleRateDenominator(), dlg->name().c_str());
		return;
	}

	// get the analogue poles- and zero responses
	sequence_number = GetPAZSequence(ci, VOLTAGE, VOLTAGE);
	if(sequence_number != -1)
	{
		int in_unit = ci.rpz[sequence_number]->GetSignalInUnits();
		int out_unit = ci.rpz[sequence_number]->GetSignalOutUnits();

		for(; sequence_number<(int)ci.rpz.size(); ++sequence_number)
		{
			if(ci.rpz[sequence_number]->GetSignalInUnits() != in_unit || ci.rpz[sequence_number]->GetSignalOutUnits() != out_unit)
				break;

			string instr = channel_name + ".stage_" + ToString<int>(ci.rpz[sequence_number]->GetStageSequenceNumber());

			DataModel::ResponsePAZPtr rp = inventory->responsePAZ(DataModel::ResponsePAZIndex(instr));
			if(!rp)
				rp = InsertResponsePAZ(ci, instr);
			else
				UpdateResponsePAZ(ci, rp);

			bool add = true;
			string afc;

			try { afc = blob2str(deci->analogueFilterChain()); } catch(Core::ValueException) {}

			string new_afc = rp->publicID();
			vector<string> analog = SplitStrings(afc, LINE_SEPARATOR);
			for(unsigned int i=0; i<analog.size(); i++)
				if(analog[i]==new_afc)
					add = false;

			if(add)
			{
				if(!afc.empty())
					afc += " ";

				afc += new_afc;
				deci->setAnalogueFilterChain(str2blob(afc));
			}
		}
	}
	else {
		sequence_number = GetFAPSequence(ci, VOLTAGE, VOLTAGE);
		if(sequence_number != -1)
		{
			int in_unit = ci.rl[sequence_number]->GetSignalInUnits();
			int out_unit = ci.rl[sequence_number]->GetSignalOutUnits();

			for(; sequence_number<(int)ci.rl.size(); ++sequence_number)
			{
				if(ci.rl[sequence_number]->GetSignalInUnits() != in_unit || ci.rl[sequence_number]->GetSignalOutUnits() != out_unit)
					break;

				string instr = channel_name + ".stage_" + ToString<int>(ci.rl[sequence_number]->GetStageSequenceNumber());

				DataModel::ResponseFAPPtr rp = inventory->responseFAP(DataModel::ResponseFAPIndex(instr));
				if(!rp)
					rp = InsertResponseFAP(ci, instr);
				else
					UpdateResponseFAP(ci, rp);

				bool add = true;
				string afc;

				try { afc = blob2str(deci->analogueFilterChain()); } catch(Core::ValueException) {}

				string new_afc = rp->publicID();
				vector<string> analog = SplitStrings(afc, LINE_SEPARATOR);
				for(unsigned int i=0; i<analog.size(); i++)
					if(analog[i]==new_afc)
						add = false;

				if(add)
				{
					if(!afc.empty())
						afc += " ";

					afc += new_afc;
					deci->setAnalogueFilterChain(str2blob(afc));
				}
			}
		}
	}

	// get the digital poles- and zero responses
	sequence_number = GetPAZSequence(ci, DIGITAL, DIGITAL);
	if(sequence_number != -1)
	{
		int in_unit = ci.rpz[sequence_number]->GetSignalInUnits();
		int out_unit = ci.rpz[sequence_number]->GetSignalOutUnits();

		for(; sequence_number<(int)ci.rpz.size(); ++sequence_number)
		{
			if(ci.rpz[sequence_number]->GetSignalInUnits() != in_unit || ci.rpz[sequence_number]->GetSignalOutUnits() != out_unit)
				break;

			string instr = channel_name + ".stage_" + ToString<int>(ci.rpz[sequence_number]->GetStageSequenceNumber());
			DataModel::ResponsePAZPtr rp = inventory->responsePAZ(DataModel::ResponsePAZIndex(instr));
			if(!rp)
				rp = InsertResponsePAZ(ci, instr);
			else
				UpdateResponsePAZ(ci, rp);


			bool add = true;
			string dfc;

			try { dfc = blob2str(deci->digitalFilterChain()); } catch(Core::ValueException) {}

			string new_dfc = rp->publicID();
			vector<string> digital = SplitStrings(dfc, LINE_SEPARATOR);
			for(unsigned int i=0; i<digital.size(); i++)
				if(digital[i]==new_dfc)
					add = false;

			if(add)
			{
				if(!dfc.empty())
					dfc += " ";

				dfc += new_dfc;
				deci->setDigitalFilterChain(str2blob(dfc));
			}
		}
	}
	else {
		sequence_number = GetFAPSequence(ci, DIGITAL, DIGITAL);
		if(sequence_number != -1)
		{
			int in_unit = ci.rl[sequence_number]->GetSignalInUnits();
			int out_unit = ci.rl[sequence_number]->GetSignalOutUnits();

			for(; sequence_number<(int)ci.rl.size(); ++sequence_number)
			{
				if(ci.rl[sequence_number]->GetSignalInUnits() != in_unit || ci.rl[sequence_number]->GetSignalOutUnits() != out_unit)
					break;

				string instr = channel_name + ".stage_" + ToString<int>(ci.rl[sequence_number]->GetStageSequenceNumber());

				DataModel::ResponseFAPPtr rp = inventory->responseFAP(DataModel::ResponseFAPIndex(instr));
				if(!rp)
					rp = InsertResponseFAP(ci, instr);
				else
					UpdateResponseFAP(ci, rp);

				bool add = true;
				string dfc;

				try { dfc = blob2str(deci->digitalFilterChain()); } catch(Core::ValueException) {}

				string new_dfc = rp->publicID();
				vector<string> digital = SplitStrings(dfc, LINE_SEPARATOR);
				for(unsigned int i=0; i<digital.size(); i++)
					if(digital[i]==new_dfc)
						add = false;

				if(add)
				{
					if(!dfc.empty())
						dfc += " ";

					dfc += new_dfc;
					deci->setDigitalFilterChain(str2blob(dfc));
				}
			}
		}
	}
}

/*******************************************************************************
* Function:	ProcessPAZSensor											    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*               seis_stream     - information of a channel as it is stored in the database                                  *
* Returns:      nothing                                                                                                     *
* Description:  check whether a new sensor should be added or an existing updated					    *
*******************************************************************************/
void Inventory::ProcessPAZSensor(ChannelIdentifier& ci, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("Start processing sensor information ");

	const char* unit = VELOCITY;
	sequence_number = GetPAZSequence(ci, VELOCITY, VOLTAGE);
	if(sequence_number == -1)
	{
		unit = ACCEL1;
		sequence_number = GetPAZSequence(ci, ACCEL1, VOLTAGE);
	}
	if(sequence_number == -1)
	{
		unit = ACCEL1;
		sequence_number = GetPAZSequence(ci, ACCEL2, VOLTAGE);
	}
	if(sequence_number == -1)
	{
		unit = DISPLACE;
		sequence_number = GetPAZSequence(ci, DISPLACE, VOLTAGE);
	}
	if(sequence_number == -1)
	{
		unit = PRESSURE;
		sequence_number = GetPAZSequence(ci, PRESSURE, VOLTAGE);
	}
	if(sequence_number == -1)
	{
		unit = TEMPERATURE;
		sequence_number = GetPAZSequence(ci, TEMPERATURE, VOLTAGE);
	}
	if(sequence_number == -1)
	{
		unit = TEMPERATURE;
		sequence_number = GetPAZSequence(ci, TEMPERATURE2, VOLTAGE);
	}
	if(sequence_number == -1)
	{
		unit = VELOCITY;
		sequence_number = GetPAZSequence(ci, VELOCITY, DIGITAL);
	}
	if(sequence_number == -1)
	{
		unit = ACCEL1;
		sequence_number = GetPAZSequence(ci, ACCEL1, DIGITAL);
	}
	if(sequence_number == -1)
	{
		unit = ACCEL1;
		sequence_number = GetPAZSequence(ci, ACCEL2, DIGITAL);
	}
	if(sequence_number == -1)
	{
		unit = DISPLACE;
		sequence_number = GetPAZSequence(ci, DISPLACE, DIGITAL);
	}
	if(sequence_number == -1)
	{
		unit = PRESSURE;
		sequence_number = GetPAZSequence(ci, PRESSURE, DIGITAL);
	}
	if(sequence_number == -1)
	{
		unit = TEMPERATURE;
		sequence_number = GetPAZSequence(ci, TEMPERATURE, DIGITAL);
	}
	if(sequence_number == -1)
	{
		unit = TEMPERATURE;
		sequence_number = GetPAZSequence(ci, TEMPERATURE2, DIGITAL);
	}

	string sensorName = station_name + "." + ci.GetChannel().substr(1,2) + strip(ci.GetLocation());

	DataModel::SensorPtr sm = inventory->sensor(DataModel::SensorIndex(sensorName));
	if(!sm)
		sm = InsertSensor(ci, strm, unit, sensorName);
	else
		UpdateSensor(ci, sm, unit);

	strm->setSensor(sm->publicID());
	strm->setGainUnit(sm->unit());

	ProcessSensorCalibration(ci, sm, strm);
	//ProcessSensorGain(ci, sm, strm);
	ProcessSensorPAZ(ci, sm);
	//ProcessRespPoly(ci, strm->sensor());

	DataModel::ResponsePAZPtr rp = inventory->responsePAZ(DataModel::ResponsePAZIndex(sm->name()));
	if(!rp)
		SEISCOMP_ERROR("poles & zeros response of sensor %s not found", strm->sensor().c_str());
}

/*******************************************************************************
* Function:	ProcessPolySensor											    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*               seis_stream     - information of a channel as it is stored in the database                                  *
* Returns:      nothing                                                                                                     *
* Description:  check whether a new sensor should be added or an existing updated					    *
*******************************************************************************/
void Inventory::ProcessPolySensor(ChannelIdentifier& ci, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("Start processing sensor information ");

	const char* unit = PRESSURE;
	sequence_number = GetPolySequence(ci, PRESSURE, VOLTAGE);
	if(sequence_number == -1)
	{
		unit = TEMPERATURE;
		sequence_number = GetPolySequence(ci, TEMPERATURE, VOLTAGE);
	}
	if(sequence_number == -1)
	{
		unit = TEMPERATURE;
		sequence_number = GetPolySequence(ci, TEMPERATURE2, VOLTAGE);
	}
	if(sequence_number == -1)
	{
		unit = PRESSURE;
		sequence_number = GetPolySequence(ci, PRESSURE, DIGITAL);
	}
	if(sequence_number == -1)
	{
		unit = TEMPERATURE;
		sequence_number = GetPolySequence(ci, TEMPERATURE, DIGITAL);
	}
	if(sequence_number == -1)
	{
		unit = TEMPERATURE;
		sequence_number = GetPolySequence(ci, TEMPERATURE2, DIGITAL);
	}

	string sensorName = station_name + "." + ci.GetChannel().substr(1,2) + strip(ci.GetLocation());

	DataModel::SensorPtr sm = inventory->sensor(DataModel::SensorIndex(sensorName));
	if(!sm)
		sm = InsertSensor(ci, strm, unit, sensorName);
	else
		UpdateSensor(ci, sm, unit);

	strm->setSensor(sm->publicID());
	strm->setGainUnit(sm->unit());

	// ProcessSensorCalibration(ci, sm, strm);
	ProcessSensorPolynomial(ci, sm);

	DataModel::ResponsePolynomialPtr rp = inventory->responsePolynomial(DataModel::ResponsePolynomialIndex(sm->name()));
	if(!rp)
		SEISCOMP_ERROR("polynomial response of sensor %s not found", strm->sensor().c_str());
}

/*******************************************************************************
* Function:	ProcessFAPSensor											    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*               seis_stream     - information of a channel as it is stored in the database                                  *
* Returns:      nothing                                                                                                     *
* Description:  check whether a new sensor should be added or an existing updated					    *
*******************************************************************************/
void Inventory::ProcessFAPSensor(ChannelIdentifier& ci, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("Start processing sensor information");

	const char* unit = VELOCITY;
	sequence_number = GetFAPSequence(ci, VELOCITY, VOLTAGE);
	if(sequence_number == -1)
	{
		unit = ACCEL1;
		sequence_number = GetFAPSequence(ci, ACCEL1, VOLTAGE);
	}
	if(sequence_number == -1)
	{
		unit = ACCEL1;
		sequence_number = GetFAPSequence(ci, ACCEL2, VOLTAGE);
	}
	if(sequence_number == -1)
	{
		unit = DISPLACE;
		sequence_number = GetFAPSequence(ci, DISPLACE, VOLTAGE);
	}
	if(sequence_number == -1)
	{
		unit = PRESSURE;
		sequence_number = GetFAPSequence(ci, PRESSURE, VOLTAGE);
	}
	if(sequence_number == -1)
	{
		unit = TEMPERATURE;
		sequence_number = GetFAPSequence(ci, TEMPERATURE, VOLTAGE);
	}
	if(sequence_number == -1)
	{
		unit = TEMPERATURE;
		sequence_number = GetFAPSequence(ci, TEMPERATURE2, VOLTAGE);
	}
	if(sequence_number == -1)
	{
		unit = VELOCITY;
		sequence_number = GetFAPSequence(ci, VELOCITY, DIGITAL);
	}
	if(sequence_number == -1)
	{
		unit = ACCEL1;
		sequence_number = GetFAPSequence(ci, ACCEL1, DIGITAL);
	}
	if(sequence_number == -1)
	{
		unit = ACCEL1;
		sequence_number = GetFAPSequence(ci, ACCEL2, DIGITAL);
	}
	if(sequence_number == -1)
	{
		unit = DISPLACE;
		sequence_number = GetFAPSequence(ci, DISPLACE, DIGITAL);
	}
	if(sequence_number == -1)
	{
		unit = PRESSURE;
		sequence_number = GetFAPSequence(ci, PRESSURE, DIGITAL);
	}
	if(sequence_number == -1)
	{
		unit = TEMPERATURE;
		sequence_number = GetFAPSequence(ci, TEMPERATURE, DIGITAL);
	}
	if(sequence_number == -1)
	{
		unit = TEMPERATURE;
		sequence_number = GetFAPSequence(ci, TEMPERATURE2, DIGITAL);
	}

	string sensorName = station_name + "." + ci.GetChannel().substr(1,2) + strip(ci.GetLocation());

	DataModel::SensorPtr sm = inventory->sensor(DataModel::SensorIndex(sensorName));
	if(!sm)
		sm = InsertSensor(ci, strm, unit, sensorName);
	else
		UpdateSensor(ci, sm, unit);

	strm->setSensor(sm->publicID());
	strm->setGainUnit(sm->unit());

	ProcessSensorCalibration(ci, sm, strm);
	//ProcessSensorGain(ci, sm, strm);
	ProcessSensorFAP(ci, sm);
	//ProcessRespPoly(ci, strm->sensor());

	DataModel::ResponseFAPPtr rp = inventory->responseFAP(DataModel::ResponseFAPIndex(sm->name()));
	if(!rp)
		SEISCOMP_ERROR("response list of sensor %s not found", strm->sensor().c_str());
}

/*******************************************************************************
* Function:     InsertSensor											    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
* Returns:	nothing													    *
* Description:	add new sensor											    *
*******************************************************************************/
DataModel::SensorPtr Inventory::InsertSensor(ChannelIdentifier& ci, DataModel::StreamPtr strm, const char* unit, const string& name)
{
	SEISCOMP_DEBUG("Insert sensor");

	int instr = ci.GetInstrument();

	DataModel::SensorPtr sm = DataModel::Sensor::Create();

	sm->setName(name);
	sm->setDescription(GetInstrumentName(instr));
	sm->setModel(GetInstrumentName(instr));
	sm->setManufacturer(GetInstrumentManufacturer(instr));
	sm->setType(GetInstrumentType(instr));
	sm->setUnit(unit);
	//sm->setResponse("paz:" + strm->sensor());

	inventory->add(sm.get());

	return sm;
}

/*******************************************************************************
* Function:     UpdateSensor											    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*               sensor     - information of a sensor as it is stored in the database                              *
* Returns:	nothing													    *
* Description:	update an existing sensor										    *
*******************************************************************************/
void Inventory::UpdateSensor(ChannelIdentifier& ci, DataModel::SensorPtr sm, const char* unit)
{
	SEISCOMP_DEBUG("Update sensor");

	int instr = ci.GetInstrument();

	sm->setDescription(GetInstrumentName(instr));
	sm->setModel(GetInstrumentName(instr));
	sm->setManufacturer(GetInstrumentManufacturer(instr));
	sm->setType(GetInstrumentType(instr));
	sm->setUnit(unit);
	//sm->setResponse("paz:" + sm->name());

	sm->update();
}

/*******************************************************************************
* Function:     ProcessSensorCalibration										    *
* Parameters:   ci              - information of a channel as it is stored in the dataless                                  *
*		sensor	- information of a sensor as it is stored in the database				    *
*               seis_stream     - information of a channel as it is stored in the database                                  *
* Returns:	nothing													    *
* Description:	check whether a new sensorcalibration should be added or an existing updated			    *
*******************************************************************************/
void Inventory::ProcessSensorCalibration(ChannelIdentifier& ci, DataModel::SensorPtr sm, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("Process sensor calibration");

	DataModel::SensorCalibrationPtr cal = sm->sensorCalibration(DataModel::SensorCalibrationIndex(strm->sensorSerialNumber(), strm->sensorChannel(), strm->start()));
	if(!cal)
		InsertSensorCalibration(ci, sm, strm);
	else
		UpdateSensorCalibration(ci, cal, strm);
}

/*******************************************************************************
* Function:     InsertSensorCalibration										    *
* Parameters:   ci    	- information of a channel as it is stored in the dataless              	            	    *
*               seismo	- id of the sensor										    *
*               seisid	- sensor_id as it is stored in the seis_stream table	                                    *
* Returns:      nothing                                                                                                     *
* Description:  add of a new sensorcalibration                                                                         *
*******************************************************************************/
void Inventory::InsertSensorCalibration(ChannelIdentifier& ci, DataModel::SensorPtr sm, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("Insert sensor calibration");

	DataModel::SensorCalibrationPtr cal = new DataModel::SensorCalibration();
	cal->setSerialNumber(strm->sensorSerialNumber());
	cal->setChannel(strm->sensorChannel());
	cal->setStart(strm->start());

	try {
		cal->setEnd(strm->end());
	}
	catch(Core::ValueException) {
		cal->setEnd(Core::None);
	}

	cal->setGain(0.0);
	cal->setGainFrequency(0.0);

	for(unsigned int i=0; i< ci.csg.size(); i++)
	{
		bool found = false;

		for ( size_t j = 0; j < ci.rpz.size(); ++j ) {
			if ( ci.csg[i]->GetStageSequenceNumber() == ci.rpz[j]->GetStageSequenceNumber() ) {
				cal->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
				cal->setGainFrequency(ci.csg[i]->GetFrequency());
				found = true;
				break;
			}
		}

		if ( !found ) {
			for ( size_t j = 0; j < ci.rl.size(); ++j ) {
				if ( ci.csg[i]->GetStageSequenceNumber() == ci.rl[j]->GetStageSequenceNumber() ) {
					cal->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
					cal->setGainFrequency(ci.csg[i]->GetFrequency());
					found = true;
					break;
				}
			}
		}
	}

	sm->add(cal.get());
}

/*******************************************************************************
* Function:     UpdateSensorCalibration										    *
* Parameters:   ci      - information of a channel as it is stored in the dataless                          		    *
*               seiscal	- information of a sensorcalibration as it is stored in the database           		    *
*               seisid	- sensor_id as it is stored in the seis_stream table                           		    *
* Returns:	nothing													    *
* Description:	update of a sensorcalibration									    *
*******************************************************************************/
void Inventory::UpdateSensorCalibration(ChannelIdentifier& ci, DataModel::SensorCalibrationPtr cal, DataModel::StreamPtr strm)
{
	SEISCOMP_DEBUG("Update sensor calibration");

	try {
		cal->setEnd(strm->end());
	}
	catch(Core::ValueException) {
		cal->setEnd(Core::None);
	}

	cal->setGain(0.0);
	cal->setGainFrequency(0.0);

	for(unsigned int i=0; i< ci.csg.size(); i++)
	{
		bool found = false;

		for ( size_t j = 0; j < ci.rpz.size(); ++j ) {
			if ( ci.csg[i]->GetStageSequenceNumber() == ci.rpz[j]->GetStageSequenceNumber() ) {
				cal->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
				cal->setGainFrequency(ci.csg[i]->GetFrequency());
				found = true;
				break;
			}
		}

		if ( !found ) {
			for ( size_t j = 0; j < ci.rl.size(); ++j ) {
				if ( ci.csg[i]->GetStageSequenceNumber() == ci.rl[j]->GetStageSequenceNumber() ) {
					cal->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
					cal->setGainFrequency(ci.csg[i]->GetFrequency());
					found = true;
					break;
				}
			}
		}
	}

	cal->update();
}

/*******************************************************************************
* Function:     ProcessSensorPAZ												    *
* Parameters:   ci	- information of a channel as it is stored in the dataless					    *
*		instr	- name of instrument the response paz is created for						    *
* Returns:	nothing													    *
* Description:	check if a new resppaz should be added or an existing should be updated					    *
*******************************************************************************/
void Inventory::ProcessSensorPAZ(ChannelIdentifier& ci, DataModel::SensorPtr sm)
{
	SEISCOMP_DEBUG("Start processing response poles & zeros, for sequence: %d", sequence_number);

	if(sequence_number != -1)
	{
		DataModel::ResponsePAZPtr rp = inventory->responsePAZ(DataModel::ResponsePAZIndex(sm->name()));
		if(!rp)
			rp = InsertResponsePAZ(ci, sm->name());
		else
			UpdateResponsePAZ(ci, rp);

		sm->setResponse(rp->publicID());
	}
}

/*******************************************************************************
* Function:     InsertResponsePAZ											    	    *
* Parameters:   ci		- information of a channel as it is stored in the dataless				    *
*		instrument	- name of instrument the response paz is created for 					    *
* Returns:	nothing													    *
* Description:	add a new resppaz											    *
*******************************************************************************/
DataModel::ResponsePAZPtr Inventory::InsertResponsePAZ(ChannelIdentifier& ci, string instrument)
{
	SEISCOMP_DEBUG("Voeg nieuwe response poles & zeros");

	DataModel::ResponsePAZPtr rp = DataModel::ResponsePAZ::Create();

	rp->setName(instrument);

	char c = ci.rpz[sequence_number]->GetTransferFunctionType();
	rp->setType(string(&c, 1));
	rp->setGain(0.0);
	rp->setGainFrequency(0.0);

	for(unsigned int i=0; i<ci.csg.size(); i++)
	{
		if(ci.csg[i]->GetStageSequenceNumber() == ci.rpz[sequence_number]->GetStageSequenceNumber())
		{
			rp->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
			rp->setGainFrequency(ci.csg[i]->GetFrequency());
		}
	}

	rp->setNormalizationFactor(ci.rpz[sequence_number]->GetAoNormalizationFactor());
	rp->setNormalizationFrequency(ci.rpz[sequence_number]->GetNormalizationFrequency());
	rp->setNumberOfZeros(ci.rpz[sequence_number]->GetNumberOfZeros());
	rp->setNumberOfPoles(ci.rpz[sequence_number]->GetNumberOfPoles());
	rp->setZeros(parseComplexArray(ci.rpz[sequence_number]->GetComplexZeros()));
	rp->setPoles(parseComplexArray(ci.rpz[sequence_number]->GetComplexPoles()));
	check_paz(rp, _fixedErrors);

	inventory->add(rp.get());

	return rp;
}

/*******************************************************************************
* Function:     UpdateResponsePAZ											    	    *
* Parameters:   ci	- information of a channel as it is stored in the dataless					    *
*		paz	- existing resppaz from the database								    *
* Returns:	nothing													    *
* Description:	update of existing resppaz										    *
*******************************************************************************/
void Inventory::UpdateResponsePAZ(ChannelIdentifier& ci, DataModel::ResponsePAZPtr rp)
{
	SEISCOMP_DEBUG("Update response poles & zeros");

	char c = ci.rpz[sequence_number]->GetTransferFunctionType();
	rp->setType(string(&c, 1));
	rp->setGain(0.0);
	rp->setGainFrequency(0.0);

	for(unsigned int i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == ci.rpz[sequence_number]->GetStageSequenceNumber() ) {
			rp->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
			rp->setGainFrequency(ci.csg[i]->GetFrequency());
		}
	}

	rp->setNormalizationFactor(ci.rpz[sequence_number]->GetAoNormalizationFactor());
	rp->setNormalizationFrequency(ci.rpz[sequence_number]->GetNormalizationFrequency());
	rp->setNumberOfZeros(ci.rpz[sequence_number]->GetNumberOfZeros());
	rp->setNumberOfPoles(ci.rpz[sequence_number]->GetNumberOfPoles());
	rp->setZeros(parseComplexArray(ci.rpz[sequence_number]->GetComplexZeros()));
	rp->setPoles(parseComplexArray(ci.rpz[sequence_number]->GetComplexPoles()));
	check_paz(rp, _fixedErrors);

	rp->update();
}

/******************************************************************************
 * Function:    ProcessSensorFAP                                              *
 * Parameters:  ci - information of a channel as it is stored in the dataless *
 *              instr - name of instrument the response paz is created for    *
 * Returns:     nothing                                                       *
 * Description: check if a new resppaz should be added or an existing should  *
 *              be updated                                                    *
 ******************************************************************************/
void Inventory::ProcessSensorFAP(ChannelIdentifier& ci, DataModel::SensorPtr sm)
{
	SEISCOMP_DEBUG("Start processing response list, for sequence: %d", sequence_number);

	if(sequence_number != -1)
	{
		DataModel::ResponseFAPPtr rp = inventory->responseFAP(DataModel::ResponseFAPIndex(sm->name()));
		if(!rp)
			rp = InsertResponseFAP(ci, sm->name());
		else
			UpdateResponseFAP(ci, rp);

		sm->setResponse(rp->publicID());
	}
}

/******************************************************************************
 * Function:     InsertResponseFAP                                            *
 * Parameters:   ci - information of a channel as stored in the dataless      *
 *               instrument - name of instrument the response is created for  *
 * Returns:      nothing                                                      *
 * Description:	 add a new respfap                                            *
 ******************************************************************************/
DataModel::ResponseFAPPtr Inventory::InsertResponseFAP(ChannelIdentifier &ci, std::string instrument) {
	SEISCOMP_DEBUG("Insert response list");

	DataModel::ResponseFAPPtr rp = DataModel::ResponseFAP::Create();

	rp->setName(instrument);
	rp->setGain(0.0);
	rp->setGainFrequency(0.0);
	rp->setNumberOfTuples(ci.rl[sequence_number]->GetNumberOfResponses());

	rp->setTuples(DataModel::RealArray());
	DataModel::RealArray &tuples = rp->tuples();

	const std::vector<ListedResponses> &rl = ci.rl[sequence_number]->GetResponsesListed();
	for ( size_t i = 0; i < rl.size(); ++i ) {
		tuples.content().push_back(rl[i].frequency);
		tuples.content().push_back(rl[i].amplitude);
		tuples.content().push_back(rl[i].phase_angle);
	}

	for( unsigned int i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == ci.rl[sequence_number]->GetStageSequenceNumber() ) {
			rp->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
			rp->setGainFrequency(ci.csg[i]->GetFrequency());
		}
	}

	inventory->add(rp.get());

	return rp;
}

/******************************************************************************
 * Function:    UpdateResponseFAP                                             *
 * Parameters:  ci  - information of a channel as stored in the dataless      *
 *              paz - existing resppaz from the database                      *
 * Returns:     nothing                                                       *
 * Description: update of existing respfap                                    *
 ******************************************************************************/
void Inventory::UpdateResponseFAP(ChannelIdentifier &ci, Seiscomp::DataModel::ResponseFAPPtr rp) {
	SEISCOMP_DEBUG("Update response list");

	rp->setGain(0.0);
	rp->setGainFrequency(0.0);
	rp->setNumberOfTuples(ci.rl[sequence_number]->GetNumberOfResponses());

	rp->setTuples(DataModel::RealArray());
	DataModel::RealArray &tuples = rp->tuples();

	const std::vector<ListedResponses> &rl = ci.rl[sequence_number]->GetResponsesListed();
	for ( size_t i = 0; i < rl.size(); ++i ) {
		tuples.content().push_back(rl[i].frequency);
		tuples.content().push_back(rl[i].amplitude);
		tuples.content().push_back(rl[i].phase_angle);
	}

	for ( unsigned int i = 0; i < ci.csg.size(); ++i ) {
		if ( ci.csg[i]->GetStageSequenceNumber() == ci.rl[sequence_number]->GetStageSequenceNumber() ) {
			rp->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
			rp->setGainFrequency(ci.csg[i]->GetFrequency());
		}
	}

	rp->update();
}

/*******************************************************************************
* Function:     ProcessSensorPolynomial												    *
* Parameters:   ci	- information of a channel as it is stored in the dataless					    *
*		instr	- name of instrument the response paz is created for						    *
* Returns:	nothing													    *
* Description:	check if a new resppaz should be added or an existing should be updated					    *
*******************************************************************************/
void Inventory::ProcessSensorPolynomial(ChannelIdentifier& ci, DataModel::SensorPtr sm)
{
	SEISCOMP_DEBUG("Start processing response polynomial, for sequence: %d", sequence_number);

	if(sequence_number != -1)
	{
		DataModel::ResponsePolynomialPtr rp = inventory->responsePolynomial(DataModel::ResponsePolynomialIndex(sm->name()));
		if(!rp)
			rp = InsertResponsePolynomial(ci, sm->name());
		else
			UpdateResponsePolynomial(ci, rp);

		sm->setResponse(rp->publicID());
		sm->setLowFrequency(ci.rp[sequence_number]->GetLowerValidFrequencyBound());
		sm->setHighFrequency(ci.rp[sequence_number]->GetUpperValidFrequencyBound());
	}
}

/*******************************************************************************
* Function:     InsertResponsePolynomial											    	    *
* Parameters:   ci		- information of a channel as it is stored in the dataless				    *
*		instrument	- name of instrument the response paz is created for 					    *
* Returns:	nothing													    *
* Description:	add a new resppaz											    *
*******************************************************************************/
DataModel::ResponsePolynomialPtr Inventory::InsertResponsePolynomial(ChannelIdentifier& ci, string instrument)
{
	SEISCOMP_DEBUG("Voeg nieuwe response polynomial");

	DataModel::ResponsePolynomialPtr rp = DataModel::ResponsePolynomial::Create();

	rp->setName(instrument);

	rp->setGain(0.0);
	rp->setGainFrequency(0.0);

	for(unsigned int i=0; i<ci.csg.size(); i++)
	{
		if(ci.csg[i]->GetStageSequenceNumber() == ci.rp[sequence_number]->GetStageSequenceNumber())
		{
			rp->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
			rp->setGainFrequency(ci.csg[i]->GetFrequency());
		}
	}

	char a;
	a = ci.rp[sequence_number]->GetValidFrequencyUnits();
	rp->setFrequencyUnit(string(&a, 1));
	a = ci.rp[sequence_number]->GetPolynomialApproximationType();
	rp->setApproximationType(string(&a, 1));
	rp->setApproximationLowerBound(ci.rp[sequence_number]->GetLowerBoundOfApproximation());
	rp->setApproximationUpperBound(ci.rp[sequence_number]->GetUpperBoundOfApproximation());
	rp->setApproximationError(ci.rp[sequence_number]->GetMaximumAbsoluteError());
	rp->setNumberOfCoefficients(ci.rp[sequence_number]->GetNumberOfPcoeff());
	rp->setCoefficients(parseRealArray(ci.rp[sequence_number]->GetPolynomialCoefficients()));

	inventory->add(rp.get());

	return rp;
}

/*******************************************************************************
* Function:     UpdateResponsePolynomial											    	    *
* Parameters:   ci	- information of a channel as it is stored in the dataless					    *
*		paz	- existing resppaz from the database								    *
* Returns:	nothing													    *
* Description:	update of existing resppaz										    *
*******************************************************************************/
void Inventory::UpdateResponsePolynomial(ChannelIdentifier& ci, DataModel::ResponsePolynomialPtr rp)
{
	SEISCOMP_DEBUG("Wijzig response polynomial");

	rp->setGain(0.0);
	rp->setGainFrequency(0.0);

	for(unsigned int i=0; i<ci.csg.size(); i++)
	{
		if(ci.csg[i]->GetStageSequenceNumber() == ci.rp[sequence_number]->GetStageSequenceNumber())
		{
			rp->setGain(fabs(ci.csg[i]->GetSensitivityGain()));
			rp->setGainFrequency(ci.csg[i]->GetFrequency());
		}
	}

	char a;
	a = ci.rp[sequence_number]->GetValidFrequencyUnits();
	rp->setFrequencyUnit(string(&a, 1));
	a = ci.rp[sequence_number]->GetPolynomialApproximationType();
	rp->setApproximationLowerBound(ci.rp[sequence_number]->GetLowerBoundOfApproximation());
	rp->setApproximationUpperBound(ci.rp[sequence_number]->GetUpperBoundOfApproximation());
	rp->setApproximationError(ci.rp[sequence_number]->GetMaximumAbsoluteError());
	rp->setNumberOfCoefficients(ci.rp[sequence_number]->GetNumberOfPcoeff());
	rp->setCoefficients(parseRealArray(ci.rp[sequence_number]->GetPolynomialCoefficients()));

	rp->update();
}

/*******************************************************************************
* Function:     GetNetworkDescription											    *
* Parameters:   lookup	- key to lookup information in the Generic Abbreviation blockette				    *
* Returns:	description of network											    *
* Description:  get the network description from Generic Abbreviation with the lookup key provided			    *
*******************************************************************************/
string Inventory::GetNetworkDescription(int lookup)
{
	SEISCOMP_DEBUG("Getting the description of the network");

	string desc;
	for(unsigned int i=0; i<adc->ga.size(); i++)
	{
		GenericAbbreviation genabb = *adc->ga[i];
		if(lookup == genabb.GetLookup())
		{
			desc = genabb.GetDescription();
			break;
		}
	}

	return desc;
}

/*******************************************************************************
* Function:     GetInstrumentName											    *
* Parameters:   lookup	- key to lookup information in the Generic Abbreviation blockette				    *
* Returns:	name of instrument											    *
* Description:  get the instrument name from Generic Abbreviation with the lookup key provided				    *
*******************************************************************************/
string Inventory::GetInstrumentName(int lookup)
{
	SEISCOMP_DEBUG("Getting the name of the instrument");

	string name;
	for(unsigned int i=0; i<adc->ga.size(); i++)
	{
		GenericAbbreviation genabb = *adc->ga[i];
		if(lookup == genabb.GetLookup())
		{
			name = genabb.GetDescription();
			vector<string> instrument_info = SplitStrings(name, LINE_SEPARATOR);
			int size = instrument_info.size();
			if(size == 1)
			{
				vector<string> extra = SplitStrings(name, ':');
				if(extra.size() == 1)
					name = instrument_info[0];
				else
					name = extra[0];
			}
			else if(size == 3)
				name = SplitString(instrument_info[1], '/');
			else if (size>1)
				name = instrument_info[1];
		}
	}
	return name;
}

/*******************************************************************************
* Function:     GetInstrumentType											    *
* Parameters:   lookup  - key to lookup information in the Generic Abbreviation blockette                                   *
* Returns:      type of instrument                                                                                          *
* Description:  get the instrument type from Generic Abbreviation with the lookup key provided                              *
*******************************************************************************/
string Inventory::GetInstrumentType(int lookup)
{
	SEISCOMP_DEBUG("Getting the type of the instrument");

	string name;
	for(unsigned int i=0; i<adc->ga.size(); i++)
	{
		GenericAbbreviation genabb = *adc->ga[i];
		if(lookup == genabb.GetLookup())
		{
			name = genabb.GetDescription();
			vector<string> instrument_info = SplitStrings(name, LINE_SEPARATOR);
			int size = instrument_info.size();
			if(size == 3)
			{
				vector<string> name_type = SplitStrings(instrument_info[1], '/');
				size = name_type.size();
				if(size == 2)
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

/*******************************************************************************
* Function:     GetInstrumentManufacturer										    *
* Parameters:   lookup  - key to lookup information in the Generic Abbreviation blockette                                   *
* Returns:      name of instrument manufacturer                                                                             *
* Description:  get the instrument manufacturer from Generic Abbreviation with the lookup key provided                      *
*******************************************************************************/
string Inventory::GetInstrumentManufacturer(int lookup)
{
	SEISCOMP_DEBUG("Getting the manufacturer of the instrument");

	string name;
	for(unsigned int i=0; i<adc->ga.size(); i++)
	{
		GenericAbbreviation genabb = *adc->ga[i];
		if(lookup == genabb.GetLookup())
		{
			name = genabb.GetDescription();
			vector<string> instrument_info = SplitStrings(name, LINE_SEPARATOR);
			int size = instrument_info.size();
			if(size > 2)
				name = instrument_info[0];
			else
				name = "";
		}
	}
	return name;
}

/*******************************************************************************
* Function:     GetStationInstrument											    *
* Parameters:   lookup	- key for looking up the description of the instrument in de Generic Abbreviation Blockette	    *
* Return:	name of instrument 											    *
* Description:	each ChannelIdentifier blockette contains an lookup key for the instrument used. This function get the	    *
*		description and retrieves the instrument name out of it							    *
*******************************************************************************/
string Inventory::GetStationInstrument(int lookup)
{
	string instr;
	if(!GetInstrumentName(lookup).empty())
		instr = SplitString(station_name, LINE_SEPARATOR)+"."+ GetInstrumentName(lookup);
	else
		instr = SplitString(station_name, LINE_SEPARATOR);
	return instr;
}

/*******************************************************************************
* Function:     GetPAZSequence												    *
* Parameters:   none													    *
*******************************************************************************/
int Inventory::GetPAZSequence(ChannelIdentifier& ci, string in, string out)
{
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

	return -1;
}
/*******************************************************************************
* Function:     GetPolySequence												    *
* Parameters:   none													    *
*******************************************************************************/
int Inventory::GetPolySequence(ChannelIdentifier& ci, string in, string out)
{
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

	return -1;
}
/******************************************************************************
 * Function:     GetFAPSequence                                               *
 * Parameters:   none                                                         *
 ******************************************************************************/
int Inventory::GetFAPSequence(ChannelIdentifier& ci, string in, string out)
{
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

	return -1;
}
/*******************************************************************************
* Function:     GetDataloggerSensitivity                                       *
* Parameters:   none                                                           *
*******************************************************************************/
int Inventory::GetDataloggerSensitivity(ChannelIdentifier &ci) const
{
	if(ci.rc.size() > 0)
	{
		if(IsDummy(*ci.rc[0]))
		{
			for(unsigned int i=0; i< ci.csg.size(); i++)
			{
				if(ci.csg[i]->GetStageSequenceNumber() == ci.rc[0]->GetStageSequenceNumber())
					return i;
			}
		}
#if 0
		else
		{
			for(unsigned int i=0; i< ci.csg.size(); i++)
			{
				if(ci.csg[i].GetStageSequenceNumber() == ci.rc[0].GetStageSequenceNumber() - 1)
					return i;
			}
		}
#endif
	}
	else
	{
			for(unsigned int j=0; j< ci.rpz.size(); j++)
			{
					if(ci.rpz[j]->GetNumberOfPoles() == 0 && ci.rpz[j]->GetNumberOfZeros() == 0)
					{
							for(unsigned int i=0; i< ci.csg.size(); i++)
							{
									if(ci.csg[i]->GetStageSequenceNumber() == ci.rpz[j]->GetStageSequenceNumber())
										return i;
							}
					}
			}
	}

	return -1;
}
/*******************************************************************************
* Function:     IsDummy                                                        *
* Parameters:   none                                                           *
*******************************************************************************/
bool Inventory::IsDummy(ResponseCoefficients &rc) const
{
	return rc.GetNumberOfNumerators() == 0;
}
/*******************************************************************************
* Function:     IsPAZStream                                                    *
* Parameters:   none                                                           *
*******************************************************************************/
bool Inventory::IsPAZStream(ChannelIdentifier& ci)
{
	if(GetPAZSequence(ci, VELOCITY, VOLTAGE) != -1)
		return true;

	if(GetPAZSequence(ci, ACCEL1, VOLTAGE) != -1)
		return true;

	if(GetPAZSequence(ci, ACCEL2, VOLTAGE) != -1)
		return true;

	if(GetPAZSequence(ci, DISPLACE, VOLTAGE) != -1)
		return true;

	if(GetPAZSequence(ci, PRESSURE, VOLTAGE) != -1)
		return true;

	if(GetPAZSequence(ci, TEMPERATURE, VOLTAGE) != -1)
		return true;

	if(GetPAZSequence(ci, TEMPERATURE2, VOLTAGE) != -1)
		return true;

	if(GetPAZSequence(ci, VELOCITY, DIGITAL) != -1)
		return true;

	if(GetPAZSequence(ci, ACCEL1, DIGITAL) != -1)
		return true;

	if(GetPAZSequence(ci, ACCEL2, DIGITAL) != -1)
		return true;

	if(GetPAZSequence(ci, DISPLACE, DIGITAL) != -1)
		return true;

	if(GetPAZSequence(ci, PRESSURE, DIGITAL) != -1)
		return true;

	if(GetPAZSequence(ci, TEMPERATURE, DIGITAL) != -1)
		return true;

	if(GetPAZSequence(ci, TEMPERATURE2, DIGITAL) != -1)
		return true;

	return false;
}
/*******************************************************************************
* Function:     IsPolyStream                                                   *
* Parameters:   none                                                           *
*******************************************************************************/
bool Inventory::IsPolyStream(ChannelIdentifier& ci)
{
	if(GetPolySequence(ci, TEMPERATURE, VOLTAGE) != -1)
		return true;

	if(GetPolySequence(ci, TEMPERATURE2, VOLTAGE) != -1)
		return true;

	if(GetPolySequence(ci, PRESSURE, VOLTAGE) != -1)
		return true;

	if(GetPolySequence(ci, TEMPERATURE, DIGITAL) != -1)
		return true;

	if(GetPolySequence(ci, TEMPERATURE2, DIGITAL) != -1)
		return true;

	if(GetPolySequence(ci, PRESSURE, DIGITAL) != -1)
		return true;

	return false;
}
/******************************************************************************
 * Function:     IsFAPStream                                                  *
 * Parameters:   none                                                         *
 ******************************************************************************/
bool Inventory::IsFAPStream(ChannelIdentifier& ci)
{
	if(GetFAPSequence(ci, VELOCITY, VOLTAGE) != -1)
		return true;

	if(GetFAPSequence(ci, ACCEL1, VOLTAGE) != -1)
		return true;

	if(GetFAPSequence(ci, ACCEL2, VOLTAGE) != -1)
		return true;

	if(GetFAPSequence(ci, DISPLACE, VOLTAGE) != -1)
		return true;

	if(GetFAPSequence(ci, PRESSURE, VOLTAGE) != -1)
		return true;

	if(GetFAPSequence(ci, TEMPERATURE, VOLTAGE) != -1)
		return true;

	if(GetFAPSequence(ci, TEMPERATURE2, VOLTAGE) != -1)
		return true;

	if(GetFAPSequence(ci, VELOCITY, DIGITAL) != -1)
		return true;

	if(GetFAPSequence(ci, ACCEL1, DIGITAL) != -1)
		return true;

	if(GetFAPSequence(ci, ACCEL2, DIGITAL) != -1)
		return true;

	if(GetFAPSequence(ci, DISPLACE, DIGITAL) != -1)
		return true;

	if(GetFAPSequence(ci, PRESSURE, DIGITAL) != -1)
		return true;

	if(GetFAPSequence(ci, TEMPERATURE, DIGITAL) != -1)
		return true;

	if(GetFAPSequence(ci, TEMPERATURE2, DIGITAL) != -1)
		return true;

	return false;
}

