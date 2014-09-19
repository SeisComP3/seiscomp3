/***************************************************************************
* Copyright (C) 2012 by Jan Becker, gempa GmbH                             *
* EMail: jabe@gempa.de                                                     *
*                                                                          *
* This code has been developed for the SED/ETH Zurich and is               *
* released under the SeisComP Public License.                              *
***************************************************************************/

#define SEISCOMP_COMPONENT scenvelope

#include <seiscomp3/logging/log.h>
#include <seiscomp3/client/application.h>
#include <seiscomp3/io/archive/xmlarchive.h>

#include <seiscomp3/datamodel/vs/vs_package.h>


using namespace std;
using namespace Seiscomp;


class EnvelopeTest : public Client::Application {
	public:
		EnvelopeTest(int argc, char **argv) : Client::Application(argc, argv) {
			setMessagingEnabled(false);
			setDatabaseEnabled(false, false);
		}

	protected:
		bool run() {
			DataModel::VS::VSPtr vs = new DataModel::VS::VS;

			DataModel::CreationInfo ci;
			ci.setAgencyID(agencyID());
			ci.setAuthor(author());
			ci.setCreationTime(Core::Time::GMT());

			DataModel::VS::EnvelopePtr env = DataModel::VS::Envelope::Create();
			env->setCreationInfo(ci);
			env->setNetwork("CH");
			env->setStation("ZUR");

			DataModel::VS::EnvelopeChannelPtr cha = DataModel::VS::EnvelopeChannel::Create();
			cha->setName("Z");
			cha->setWaveformID(DataModel::WaveformStreamID("CH", "ZUR", "", "HGZ", ""));

			cha->add(new DataModel::VS::EnvelopeValue(0.3, "vel", Core::None));
			cha->add(new DataModel::VS::EnvelopeValue(0.2, "acc", Core::None));
			cha->add(new DataModel::VS::EnvelopeValue(0.1, "disp", Core::None));

			env->add(cha.get());

			vs->add(env.get());

			IO::XMLArchive ar;
			ar.create("-");
			ar.setFormattedOutput(true);
			ar << vs;
			return true;
		}
};


int main(int argc, char **argv) {
	EnvelopeTest app(argc, argv);
	return app.exec();
}

