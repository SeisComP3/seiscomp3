/***************************************************************************
* Copyright (C) 2010 by Jan Becker, gempa GmbH                             *
* EMail: jabe@gempa.de                                                     *
*                                                                          *
* This code has been developed for the SED/ETH Zurich and is               *
* released under the SeisComP Public License.                              *
***************************************************************************/

#define SEISCOMP_COMPONENT Org2NLL

#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/application.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/datamodel/network.h>
#include <seiscomp3/datamodel/station.h>
#include <seiscomp3/datamodel/sensorlocation.h>
#include <seiscomp3/datamodel/utils.h>


#include <iostream>
#include <iomanip>


using namespace std;
using namespace Seiscomp;
using namespace Seiscomp::DataModel;


namespace {


double timeError(const TimeQuantity &t, double defaultValue) {
	try {
		return quantityUncertainty(t);
	}
	catch ( ... ) {
		return defaultValue;
	}
}


}


class Org2NLLApp : public Client::Application {
	public:
		Org2NLLApp(int argc, char **argv) : Client::Application(argc, argv) {
			setMessagingEnabled(false);
			setDatabaseEnabled(true, true);
			setLoadStationsEnabled(true);
			setLoggingToStdErr(true);

			_defaultError = 0.5;
		}

	protected:
		void createCommandLineDescription() {
			commandline().addGroup("Input");
			commandline().addOption("Input", "origin-id,O", "origin id to convert into NLL observations", &_originID, false);
			commandline().addOption("Input", "default-pick-error", "if no pick error is defined use this as a default", &_defaultError);
		}

		bool run() {
			if ( _originID.empty() ) {
				cerr << "ERROR: OriginID is empty" << endl;
				return false;
			}

			Client::Inventory *inv = Client::Inventory::Instance();
			if ( !inv ) {
				cerr << "ERROR: No inventory found" << endl;
				return false;
			}

			PublicObjectPtr obj = query()->getObject(Origin::TypeInfo(), _originID);
			OriginPtr org = Origin::Cast(obj);
			if ( org == NULL ) {
				cerr << "ERROR: Origin with id '" << _originID << "' has not been found" << endl;
				return false;
			}

			query()->loadArrivals(org.get());

			// Iterate over all arrival and find picks and the sensor locations.
			//
			// The output is defined by NonLinLoc with one arrival per line:
			//
			// --- NLL obs format ---
			// Station name (char*6)
			//   station name or code
			// Instrument (char*4)
			//   instument identification for the trace for which the time pick corresponds (i.e. SP, BRB, VBB)
			// Component (char*4)
			//   component identification for the trace for which the time pick corresponds (i.e. Z, N, E, H)
			// P phase onset (char*1)
			//   description of P phase arrival onset; i, e
			// Phase descriptor (char*6)
			//   Phase identification (i.e. P, S, PmP)
			// First Motion (char*1)
			//   first motion direction of P arrival; c, C, u, U = compression; d, D = dilatation; +, -, Z, N; . or ? = not readable.
			// Date (yyyymmdd) (int*6)
			//   year (with century), month, day
			// Hour/minute (hhmm) (int*4)
			//   Hour, min
			// Seconds (float*7.4)
			//   seconds of phase arrival
			// Err (char*3)
			//   Error/uncertainty type; GAU
			// ErrMag (expFloat*9.2)
			//   Error/uncertainty magnitude in seconds
			// Coda duration (expFloat*9.2)
			//   coda duration reading
			// Amplitude (expFloat*9.2)
			//   Maxumim peak-to-peak amplitude 
			// Period (expFloat*9.2)
			//   Period of amplitude reading

			for ( size_t i = 0; i < org->arrivalCount(); ++i ) {
				Arrival *ar = org->arrival(i);
				PickPtr pick = Pick::Find(ar->pickID());
				if ( !pick ) {
					pick = Pick::Cast(query()->getObject(Pick::TypeInfo(), ar->pickID()));
					if ( !pick ) {
						cerr << "WARNING: pick '" << ar->pickID() << "' for arrival[" << i << "] not found: ignoring" << endl;
						continue;
					}

					SensorLocation *sloc = inv->getSensorLocation(
						pick->waveformID().networkCode(),
						pick->waveformID().stationCode(),
						pick->waveformID().locationCode(),
						pick->time().value());

					if ( sloc == NULL ) {
						cerr << "WARNING: sensor location for arrival[" << i << "] and for id "
						     << pick->waveformID().networkCode() << "."
						     << pick->waveformID().networkCode() << "."
						     << pick->waveformID().networkCode() << " not found: ignoring" << endl;
						continue;
					}

					cout << setprecision(2);
					cout.setf(ios_base::scientific, ios_base::floatfield);

					cout // Station code
					     << left << setw(6) << pick->waveformID().stationCode() << internal << setw(0) << " "
					     // Instrument
					     << "? "
					     // Component
					     << (pick->waveformID().channelCode().size() > 2
					         ?
					           pick->waveformID().channelCode().substr(pick->waveformID().channelCode().size()-1)
					         :
					           "?"
					         ) << " "
					     // P phase onset (i, e)
					     << "? "
					     // Phase descriptor
					     << pick->phaseHint().code() << " "
					     // First motion
					     << "? "
					     // Date
					     << pick->time().value().toString("%Y%m%d") << " "
					     // Hour Minute
					     << pick->time().value().toString("%H%M") << " "
					     // Seconds
					     << pick->time().value().toString("%S.%4f") << " "
					     // Error type
					     << "GAU "
					     // Error magnitude
					     << timeError(pick->time(), _defaultError) << " "
					     // Coda duration
					     << -1.0 << " "
					     // Amplitude
					     << -1.0 << " "
					     // Period
					     << -1.0 << " "
					     // PriorWt
					     << 1.0 << endl;
				}
			}

			return true;
		}

	private:
		std::string _originID;
		double      _defaultError;
};


int main(int argc, char **argv) {
	int retCode = EXIT_SUCCESS;

	// Create an own block to make sure the application object
	// is destroyed when printing the overall objectcount
	{
		Org2NLLApp app(argc, argv);
		retCode = app.exec();
	}

	return retCode;
}
