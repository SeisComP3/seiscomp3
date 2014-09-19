/*==============================================================================
    Name:       inventory.h

    Language:   C++, ANSI standard.

    Author:     Peter de Boer, KNMI
                Adopted by Jan Becker, gempa GmbH

    Revision:   2012-05-02

==============================================================================*/
#ifndef MYINVENTORY_H
#define MYINVENTORY_H

#include <seiscomp3/datamodel/inventory.h>
#include <seiscomp3/datamodel/qualitycontrol.h>
#include <sstream>
#include <set>

#include "define.h"
#include "tmanip.h"
#include "mystring.h"
#include "seed.h"

class Inventory
{
	public:
		Inventory(INIT_MAP& init, Seiscomp::DataModel::Inventory *inv);
		Inventory(const std::string &dcid, const std::string &net_description,
		const std::string &net_type, const Seiscomp::Core::Time &net_start,
		const OPT(Seiscomp::Core::Time) &net_end, bool temporary, bool restricted, bool shared,
		Seiscomp::DataModel::Inventory *inv);
		VolumeIndexControl *vic;
		AbbreviationDictionaryControl *adc;
		StationControl *sc;
		void SynchronizeInventory();

		int fixedErrors() const { return _fixedErrors; }

	protected:
	private:
		//std::stringstream command, output;
		std::string station_name;
		std::string channel_name;
		std::map<std::pair<std::string, int>, Seiscomp::DataModel::NetworkPtr> networks;
		std::map<std::pair<std::pair<std::string, std::string>, Seiscomp::Core::Time>, Seiscomp::DataModel::StationPtr> stations;
		std::set<std::pair<std::pair<std::pair<std::pair<std::string, std::string>, std::string>, Seiscomp::Core::Time>, Seiscomp::Core::Time > > sensor_locations;
		std::set<std::pair<std::pair<std::pair<std::pair<std::pair<std::pair<std::string, std::string>, std::string>, std::string>, Seiscomp::Core::Time>, Seiscomp::Core::Time>, Seiscomp::Core::Time> > seis_streams;
		std::set<std::pair<std::pair<std::pair<std::pair<std::pair<std::pair<std::string, std::string>, std::string>, std::string>, Seiscomp::Core::Time>, Seiscomp::Core::Time>, Seiscomp::Core::Time> > aux_streams;
		std::string _dcid;
		std::string _net_description;
		std::string _net_type;
		Seiscomp::Core::Time _net_start;
		OPT(Seiscomp::Core::Time) _net_end;
		bool _temporary;
		bool _restricted;
		bool _shared;
		int _fixedErrors;
		Seiscomp::DataModel::Inventory *inventory;
		std::map<std::vector<std::string>, std::string> encoding;
		//STATION_INFO info;
		//Logging *log;
		int sequence_number;
		void ProcessStation();
		void CleanupDatabase();
		void GetComment(StationIdentifier&);
		void GetStationComment(Comment&, Seiscomp::DataModel::WaveformStreamID *);
		void GetChannelComment(ChannelIdentifier&, Seiscomp::DataModel::WaveformStreamID *);
		OPT(Seiscomp::Core::Time) GetOptTime(std::string);
		Seiscomp::Core::Time GetTime(std::string, bool *ok = NULL);
		void ProcessStream(StationIdentifier&, Seiscomp::DataModel::StationPtr);
		void ProcessComponent(ChannelIdentifier&, Seiscomp::DataModel::StreamPtr); 
		void ProcessDatalogger(ChannelIdentifier&, Seiscomp::DataModel::StreamPtr);
		void ProcessDecimation(ChannelIdentifier&, Seiscomp::DataModel::DataloggerPtr, Seiscomp::DataModel::StreamPtr);
		void ProcessDataloggerCalibration(ChannelIdentifier&, Seiscomp::DataModel::DataloggerPtr, Seiscomp::DataModel::StreamPtr);
		void ProcessDataloggerFIR(ChannelIdentifier&, Seiscomp::DataModel::DataloggerPtr, Seiscomp::DataModel::StreamPtr strm);
		void ProcessDataloggerPAZ(ChannelIdentifier&, Seiscomp::DataModel::DataloggerPtr, Seiscomp::DataModel::StreamPtr strm);
		void ProcessPAZSensor(ChannelIdentifier&, Seiscomp::DataModel::StreamPtr);
		void ProcessPolySensor(ChannelIdentifier&, Seiscomp::DataModel::StreamPtr);
		void ProcessSensorCalibration(ChannelIdentifier&, Seiscomp::DataModel::SensorPtr, Seiscomp::DataModel::StreamPtr);
		void ProcessSensorPAZ(ChannelIdentifier& ci, Seiscomp::DataModel::SensorPtr sm);
		void ProcessSensorPolynomial(ChannelIdentifier& ci, Seiscomp::DataModel::SensorPtr sm);
		Seiscomp::DataModel::StationPtr InsertStation(StationIdentifier&, Seiscomp::DataModel::NetworkPtr);
		Seiscomp::DataModel::SensorLocationPtr InsertSensorLocation(ChannelIdentifier& ci, Seiscomp::DataModel::StationPtr station, const Seiscomp::Core::Time& loc_start, const OPT(Seiscomp::Core::Time)& loc_end);
		Seiscomp::DataModel::StreamPtr InsertStream(ChannelIdentifier&, Seiscomp::DataModel::SensorLocationPtr, bool restricted, bool shared);
		Seiscomp::DataModel::AuxStreamPtr InsertAuxStream(ChannelIdentifier&, Seiscomp::DataModel::SensorLocationPtr, bool restricted, bool shared);
		Seiscomp::DataModel::DataloggerPtr InsertDatalogger(ChannelIdentifier&, Seiscomp::DataModel::StreamPtr, const std::string& name);
		void InsertDecimation(ChannelIdentifier&, Seiscomp::DataModel::DataloggerPtr, Seiscomp::DataModel::StreamPtr);
		void InsertDataloggerCalibration(ChannelIdentifier&, Seiscomp::DataModel::DataloggerPtr, Seiscomp::DataModel::StreamPtr);
		Seiscomp::DataModel::ResponseFIRPtr InsertRespCoeff(ChannelIdentifier&, unsigned int&);
		Seiscomp::DataModel::ResponseFIRPtr InsertResponseFIRr(ChannelIdentifier&, unsigned int&);
		Seiscomp::DataModel::SensorPtr InsertSensor(ChannelIdentifier&, Seiscomp::DataModel::StreamPtr, const char* unit, const std::string& name);
		void InsertSensorCalibration(ChannelIdentifier&, Seiscomp::DataModel::SensorPtr, Seiscomp::DataModel::StreamPtr);
		Seiscomp::DataModel::ResponsePAZPtr InsertResponsePAZ(ChannelIdentifier&, std::string);
		Seiscomp::DataModel::ResponsePolynomialPtr InsertResponsePolynomial(ChannelIdentifier&, std::string);
		void UpdateStation(StationIdentifier&, Seiscomp::DataModel::StationPtr);
		void UpdateSensorLocation(ChannelIdentifier& ci, Seiscomp::DataModel::SensorLocationPtr loc, const Seiscomp::Core::Time& loc_start, const OPT(Seiscomp::Core::Time)& loc_end);
		void UpdateStream(ChannelIdentifier&, Seiscomp::DataModel::StreamPtr, bool restricted, bool shared);
		void UpdateAuxStream(ChannelIdentifier&, Seiscomp::DataModel::AuxStreamPtr, bool restricted, bool shared);
		void UpdateDatalogger(ChannelIdentifier&, Seiscomp::DataModel::DataloggerPtr, Seiscomp::DataModel::StreamPtr);
		void UpdateDecimation(ChannelIdentifier&, Seiscomp::DataModel::DecimationPtr, Seiscomp::DataModel::StreamPtr);
		void UpdateDataloggerCalibration(ChannelIdentifier&, Seiscomp::DataModel::DataloggerCalibrationPtr, Seiscomp::DataModel::StreamPtr);
		void UpdateRespCoeff(ChannelIdentifier&, Seiscomp::DataModel::ResponseFIRPtr, unsigned int&);
		void UpdateResponseFIRr(ChannelIdentifier&, Seiscomp::DataModel::ResponseFIRPtr, unsigned int&);
		void UpdateSensor(ChannelIdentifier&, Seiscomp::DataModel::SensorPtr, const char* unit);
		void UpdateSensorCalibration(ChannelIdentifier&, Seiscomp::DataModel::SensorCalibrationPtr, Seiscomp::DataModel::StreamPtr);
		void UpdateResponsePAZ(ChannelIdentifier&, Seiscomp::DataModel::ResponsePAZPtr);
		void UpdateResponsePolynomial(ChannelIdentifier&, Seiscomp::DataModel::ResponsePolynomialPtr);
		std::string GetNetworkDescription(int);
		std::string GetInstrumentName(int);
		std::string GetInstrumentManufacturer(int);
		std::string GetInstrumentType(int);
		std::string GetStationInstrument(int);
		int GetPAZSequence(ChannelIdentifier&, std::string, std::string);
		int GetPolySequence(ChannelIdentifier&, std::string, std::string);
		bool IsPAZStream(ChannelIdentifier& ci);
		bool IsPolyStream(ChannelIdentifier& ci);
};
#endif /* MYINVENTORY_H */
