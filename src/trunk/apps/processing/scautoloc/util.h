/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/




#include <string>
#include <vector>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/seismology/ttt.h>

#include "datamodel.h"

namespace Autoloc {

double distance(const Station* s1, const Station* s2);
std::string printDetailed(const Origin*);
std::string printOneliner(const Origin*);
bool automatic(const Pick*);
bool ignored(const Pick*);
bool manual(const Pick*);
char statusFlag(const Pick*);
bool hasAmplitude(const Pick*);


double meandev(const Origin* origin);

double avgfn(double x);

std::string printOrigin(const Origin *origin, bool=false);
int numberOfDefiningPhases(const Origin &origin);

typedef Seiscomp::TravelTime TravelTime;
bool travelTimeP (double lat1, double lon1, double dep1, double lat2, double lon2, double alt2, double delta, TravelTime&);

// 1st arrival P incl. Pdiff up to 130 deg, no PKP
bool travelTimeP1(double lat1, double lon1, double dep1, double lat2, double lon2, double alt2, double delta, TravelTime&);

// 1st arrival PK* incl. PKP*, PKiKP
bool travelTimePK(double lat1, double lon1, double dep1, double lat2, double lon2, double alt2, double delta, TravelTime&);


TravelTime travelTimePP(double lat1, double lon1, double dep1, double lat2, double lon2, double alt2, double delta);

std::string time2str(const Time &t);

namespace Utils {

StationDB *readStationLocations(const std::string &fname);
//bool readStationConfig(StationDB *stations, const std::string &fname);
PickDB readPickFile();
Pick*  readPickLine();
Pick::Status status(const Seiscomp::DataModel::Pick *pick);

}

}






namespace Seiscomp {
namespace Math {
namespace Statistics {

double rms(const std::vector<double> &v, double offset = 0);

} // namespace Statistics
} // namespace Math
} // namespace Seiscomp

