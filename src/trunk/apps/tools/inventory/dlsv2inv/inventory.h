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
		typedef OPT(size_t) SequenceNumber;

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


	public:
		MAKEENUM(ResponseType,
			EVALUES(
				RT_None,
				RT_FIR,
				RT_RC,
				RT_PAZ,
				RT_Poly,
				RT_FAP
			),
			ENAMES(
				"None",
				"FIR",
				"RC",
				"PAZ",
				"Poly",
				"FAP"
			)
		);

		struct StageItem {
			StageItem(size_t idx, size_t i, ResponseType rt, int iu, int ou)
			: stage(idx), index(i), type(rt), inputUnit(iu), outputUnit(ou) {}
			size_t       stage;
			size_t       index;
			ResponseType type;
			int          inputUnit;
			int          outputUnit;
		};

		typedef std::vector<StageItem> Stages;


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
		SequenceNumber response_index;
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
		void ProcessPAZSensor(ChannelIdentifier&, Seiscomp::DataModel::StreamPtr, int stageSequenceNumber);
		void ProcessPolySensor(ChannelIdentifier&, Seiscomp::DataModel::StreamPtr, int stageSequenceNumber);
		void ProcessFAPSensor(ChannelIdentifier&, Seiscomp::DataModel::StreamPtr, int stageSequenceNumber);
		void ProcessSensorCalibration(ChannelIdentifier&, Seiscomp::DataModel::SensorPtr, Seiscomp::DataModel::StreamPtr, int stageSequenceNumber);
		void ProcessSensorPAZ(ChannelIdentifier& ci, Seiscomp::DataModel::SensorPtr sm);
		void ProcessSensorPolynomial(ChannelIdentifier& ci, Seiscomp::DataModel::SensorPtr sm);
		void ProcessSensorFAP(ChannelIdentifier& ci, Seiscomp::DataModel::SensorPtr sm);
		Seiscomp::DataModel::StationPtr InsertStation(StationIdentifier&, Seiscomp::DataModel::NetworkPtr);
		Seiscomp::DataModel::SensorLocationPtr InsertSensorLocation(ChannelIdentifier& ci, Seiscomp::DataModel::StationPtr station, const Seiscomp::Core::Time& loc_start, const OPT(Seiscomp::Core::Time)& loc_end);
		Seiscomp::DataModel::StreamPtr InsertStream(ChannelIdentifier&, Seiscomp::DataModel::SensorLocationPtr, bool restricted, bool shared);
		Seiscomp::DataModel::DataloggerPtr InsertDatalogger(ChannelIdentifier&, Seiscomp::DataModel::StreamPtr, const std::string& name);
		void InsertDecimation(ChannelIdentifier&, Seiscomp::DataModel::DataloggerPtr, Seiscomp::DataModel::StreamPtr);

		Seiscomp::DataModel::ResponseFIRPtr InsertRespCoeff(ChannelIdentifier&, const std::string &name);
		Seiscomp::DataModel::ResponseFIRPtr InsertResponseFIR(ChannelIdentifier&, const std::string &name);
		Seiscomp::DataModel::ResponsePAZPtr InsertResponsePAZ(ChannelIdentifier&, const std::string &name);
		Seiscomp::DataModel::ResponseFAPPtr InsertResponseFAP(ChannelIdentifier&, const std::string &name);
		Seiscomp::DataModel::ResponsePolynomialPtr InsertResponsePolynomial(ChannelIdentifier&, const std::string &name);

		Seiscomp::DataModel::SensorPtr InsertSensor(ChannelIdentifier&, Seiscomp::DataModel::StreamPtr, const std::string &unit, const std::string& name);
		void InsertSensorCalibration(ChannelIdentifier&, Seiscomp::DataModel::SensorPtr, Seiscomp::DataModel::StreamPtr, int stageSequenceNumber);

		void UpdateStation(StationIdentifier&, Seiscomp::DataModel::StationPtr);
		void UpdateSensorLocation(ChannelIdentifier& ci, Seiscomp::DataModel::SensorLocationPtr loc, const Seiscomp::Core::Time& loc_start, const OPT(Seiscomp::Core::Time)& loc_end);
		void UpdateStream(ChannelIdentifier&, Seiscomp::DataModel::StreamPtr, bool restricted, bool shared);
		void UpdateDatalogger(ChannelIdentifier&, Seiscomp::DataModel::DataloggerPtr, Seiscomp::DataModel::StreamPtr);
		void UpdateDecimation(ChannelIdentifier&, Seiscomp::DataModel::DecimationPtr, Seiscomp::DataModel::StreamPtr);

		void UpdateRespCoeff(ChannelIdentifier&, Seiscomp::DataModel::ResponseFIRPtr);
		void UpdateResponseFIR(ChannelIdentifier&, Seiscomp::DataModel::ResponseFIRPtr);
		void UpdateResponsePAZ(ChannelIdentifier&, Seiscomp::DataModel::ResponsePAZPtr);
		void UpdateResponseFAP(ChannelIdentifier&, Seiscomp::DataModel::ResponseFAPPtr);
		void UpdateResponsePolynomial(ChannelIdentifier&, Seiscomp::DataModel::ResponsePolynomialPtr);

		void UpdateSensor(ChannelIdentifier&, Seiscomp::DataModel::SensorPtr, const std::string &unit);
		void UpdateSensorCalibration(ChannelIdentifier&, Seiscomp::DataModel::SensorCalibrationPtr, Seiscomp::DataModel::StreamPtr, int stageSequenceNumber);
		std::string GetNetworkDescription(int);
		std::string GetInstrumentDescription(int);
		std::string GetInstrumentName(int);
		std::string GetInstrumentManufacturer(int);
		std::string GetInstrumentType(int);
		std::string GetStationInstrument(int);
		SequenceNumber GetPAZSequence(ChannelIdentifier&, std::string, std::string);
		SequenceNumber GetFAPSequence(ChannelIdentifier&, std::string, std::string);
		SequenceNumber GetPolySequence(ChannelIdentifier&, std::string, std::string);
		ResponseType GetSensorResponseType(const ChannelIdentifier& ci, SequenceNumber &stageSequenceNumber);
		void GetStages(Stages &stages, const ChannelIdentifier &ci);
};
#endif /* MYINVENTORY_H */
