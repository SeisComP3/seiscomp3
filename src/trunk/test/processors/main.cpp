#define SEISCOMP_COMPONENT procs

#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/application.h>
#include <seiscomp3/client/inventory.h>
#include <seiscomp3/processing/amplitudes/MLv.h>


using namespace std;
using namespace Seiscomp;


class App : public Client::Application {
	public:
		App(int argc, char **argv) : Client::Application(argc, argv) {
			setMessagingEnabled(false);
			setDatabaseEnabled(false, false);
			setLoadInventoryEnabled(false);
			setLoggingToStdErr(true);
		}


		bool test(Processing::Settings &settings,
		          Processing::WaveformProcessor &proc,
		          bool expectedResult,
		          bool expectedThresholdActivation = true,
		          double expectedThreshold = -1) {
			if ( proc.setup(settings) != expectedResult ) {
				cerr << "proc.setup returned unexpected result: "
				     << (!expectedResult) << " != " << expectedResult << endl;
				_checksOK = false;
				return false;
			}

			if ( expectedResult == false )
				return true;

			if ( proc.isSaturationCheckEnabled() != expectedThresholdActivation ) {
				cerr << "proc saturation enabled is not expected: "
				     << (!expectedThresholdActivation) << " != " << expectedThresholdActivation << endl;
				_checksOK = false;
				return false;
			}

			if ( expectedThresholdActivation == false )
				return true;

			if ( proc.saturationThreshold() != expectedThreshold ) {
				cerr << "proc saturation threshold is not expected: "
				     << proc.saturationThreshold() << " != " << expectedThreshold << endl;
				_checksOK = false;
				return false;
			}

			return true;
		}


		bool run() {
			string net = "XX", sta = "ABCD", loc = "", cha = "HHZ";

			_checksOK = true;

			Processing::AmplitudeProcessor_MLv proc;
			Processing::Settings settings(configModuleName(),
			                              net, sta, loc, cha,
			                              &_configuration, NULL);

			_configuration.setString("module.trunk.global.amplitudes.MLv.saturationThreshold", "123");
			test(settings, proc, true, true, 123);

			_configuration.setString("module.trunk.global.amplitudes.MLv.saturationThreshold", "0.5@4");
			test(settings, proc, true, true, 8);

			_configuration.setString("module.trunk.global.amplitudes.MLv.saturationThreshold", "50%@4");
			test(settings, proc, true, true, 8);

			_configuration.setString("module.trunk.global.amplitudes.MLv.saturationThreshold", "-1@-1");
			test(settings, proc, false);

			_configuration.setString("module.trunk.global.amplitudes.MLv.saturationThreshold", "%@2");
			test(settings, proc, false);

			_configuration.setString("module.trunk.global.amplitudes.MLv.saturationThreshold", "80@");
			test(settings, proc, false);

			_configuration.setString("module.trunk.global.amplitudes.MLv.saturationThreshold", "80%");
			test(settings, proc, false);

			_configuration.setString("module.trunk.global.amplitudes.saturationThreshold", "80%@4");
			_configuration.setString("module.trunk.global.amplitudes.MLv.saturationThreshold", "123");
			test(settings, proc, true, true, 123);

			_configuration.setString("module.trunk.global.amplitudes.saturationThreshold", "80%@4");
			_configuration.setString("module.trunk.global.amplitudes.MLv.saturationThreshold", "false");
			test(settings, proc, true, false);

			_configuration.setString("module.trunk.global.amplitudes.saturationThreshold", "false");
			_configuration.setString("module.trunk.global.amplitudes.MLv.saturationThreshold", "123");
			test(settings, proc, true, true, 123);

			if ( _checksOK )
				cerr << "All tests passed" << endl;

			return _checksOK;
		}


	private:
		bool _checksOK;
};


int main(int argc, char **argv) {
	App app(argc, argv);
	return app();
}
