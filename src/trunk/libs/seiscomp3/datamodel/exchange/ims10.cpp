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


#define SEISCOMP_COMPONENT IMS10Exchange


#include "ims10.h"

#include <cstdio>
#include <seiscomp3/datamodel/event.h>
#include <seiscomp3/datamodel/eventdescription.h>
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/magnitude.h>
#include <seiscomp3/datamodel/stationmagnitude.h>
#include <seiscomp3/datamodel/amplitude.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/logging/log.h>
#include <seiscomp3/math/geo.h>


namespace Seiscomp {
namespace DataModel {


REGISTER_EXPORTER_INTERFACE(ExporterIMS10, "ims10");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
namespace {


std::string stringify(const char* fmt, ...) {
	int size = 512;
	char* buffer = 0;
	buffer = new char[size];
	va_list vl;
	va_start(vl, fmt);
	int nsize = vsnprintf(buffer, size, fmt, vl);

	if ( size <= nsize ) { //fail delete buffer and try again
		delete buffer;
		buffer = new char[nsize + 1]; //+1 for /0
		nsize = vsnprintf(buffer, size, fmt, vl);
	}

	std::string ret(buffer);
	va_end(vl);
	delete buffer;

	return ret;
}


}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
ExporterIMS10::ExporterIMS10() {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool ExporterIMS10::put(std::streambuf* buf, Core::BaseObject* obj) {
	if ( buf == NULL ) return false;
	if ( obj == NULL ) return false;
	EventParameters* ep = EventParameters::Cast(obj);
	if ( ep == NULL ) return false;

	std::ostream os(buf);
	Origin *o = NULL;
	Magnitude *m = NULL;

	// prettyPrint flag enables header output
	if ( _prettyPrint ) {
		os << "DATA_TYPE BULLETIN IMS1.0:short" << std::endl;
		os << "SeisComP3 Bulletin" << std::endl;
	}

	for ( size_t i = 0; i < ep->eventCount(); ++i ) {
		Event* e = ep->event(i);

		for ( size_t i = 0; i < ep->originCount(); ++i )
			if ( ep->origin(i)->publicID() == e->preferredOriginID() )
				o = ep->origin(i);

		for ( size_t i = 0; i < o->magnitudeCount(); ++i )
			if ( o->magnitude(i)->publicID() == e->preferredMagnitudeID() )
				m = o->magnitude(i);

		// Origin arrivals
		for (size_t j = 0; j < o->arrivalCount(); ++j) {
			Arrival* arr = o->arrival(j);
			if ( arr )
			    _arrivalList.push_back(arr);
		}

		// Origin station magnitude
		for (size_t j = 0; j < o->stationMagnitudeCount(); ++j) {
			StationMagnitude* sta = o->stationMagnitude(j);
			if ( sta )
			    _stationMagnitudeList.push_back(sta);
		}

		std::string date = o->time().value().toString("%Y/%m/%d");
		std::string hour = o->time().value().toString("%H:%M:%S.%2f");

		std::string timeErr;
		try {
			timeErr = stringify("%#5.2f", o->time().uncertainty());
		} catch ( ... ) {
			timeErr = "     ";
		}
		if ( timeErr.length() > 5 )
			timeErr = "     ";

		std::string rms;
		try {
			rms = stringify("%#5.2f", o->quality().standardError());
		} catch ( ... ) {
			rms = "     ";
		}
		if ( rms.length() > 5 )
			rms = "     ";

		std::string latitude = stringify("%#8.4f", o->latitude().value());

		std::string longitude = stringify("%#9.4f", o->longitude().value() );

		std::string erhM, erhm, erhStrike;
		try {
			erhM = stringify("%#4.1f", o->uncertainty().maxHorizontalUncertainty());
			erhm = stringify("%#5.1f", o->uncertainty().minHorizontalUncertainty());
			erhStrike = stringify("%3.0f", o->uncertainty().azimuthMaxHorizontalUncertainty());
		}
		catch ( ... ) {
			try {
				erhM = stringify("%#4.1f", o->latitude().uncertainty());
			} catch ( ... ) {
				erhM = "    ";
			}

			try {
				erhm = stringify("%#5.1f", o->longitude().uncertainty());
			} catch ( ... ) {
				erhm = "     ";
			}

			erhStrike = "  0";
		}
		if ( erhM.length() > 4 )
			erhM = "    ";
		if ( erhm.length() > 5 )
			erhm = "     ";
		if ( erhStrike.length() > 3 )
			erhStrike = "   ";


		std::string depth;
		try {
			depth = stringify("%#5.1f", o->depth().value());
		} catch ( ... ) {
			depth = "      ";
		}
		if ( depth.length() > 5 )
			depth = "     ";

		std::string depthErr;
		try {
			depthErr = stringify("%#4.1f", o->depth().uncertainty());
		} catch ( ... ) {
			depthErr = "    ";
		}
		if ( depthErr.length() > 4 )
			depthErr = "    ";

		std::string Nphases;
		try {
			Nphases = stringify("%4d", o->quality().usedPhaseCount());
		} catch ( ... ) {
			Nphases = "    ";
		}
		if ( Nphases.length() > 4 )
			Nphases = "    ";

		std::string Nstations;
		try {
			Nstations = stringify("%4d", o->quality().usedStationCount());
		} catch ( ... ) {
			Nstations = "    ";
		}
		if ( Nstations.length() > 4 )
			Nstations = "    ";

		std::string azigap;
		try {
			azigap = stringify("%3.0f", o->quality().azimuthalGap());
		} catch ( ... ) {
			azigap = "   ";
		}
		if ( azigap.length() > 3 )
			azigap = "   ";

		std::string dMin;
		try {
			dMin = stringify("%#6.2f", o->quality().minimumDistance());
		} catch ( ... ) {
			dMin = "      ";
		}
		if ( dMin.length() > 6 )
			dMin = "      ";

		std::string dMax;
		try {
			dMax = stringify("%#6.2f", o->quality().maximumDistance());
		} catch ( ... ) {
			dMax = "      ";
		}
		if ( dMax.length() > 6 )
			dMax = "      ";

		std::string author = stringify("%-9s", o->creationInfo().agencyID().substr(0, 9).c_str());

		// ** Event Title Block **
		os << "Event " << e->publicID() << " ";
		for ( size_t j = 0; j < e->eventDescriptionCount(); ++j ) {
			EventDescription* desc = e->eventDescription(j);
			if ( desc->type() == REGION_NAME ) {
				os << desc->text() << std::endl;
				break;
			}
		}

	// ** Origin Block **
		os << "   Date       Time        Err   RMS Latitude Longitude  Smaj  Smin  Az Depth   Err Ndef Nsta Gap  mdist  Mdist Qual   Author      OrigID" << std::endl;
		if ( o ) {
			// Date YYYY/MM/DD col 1-10
			os << date << " ";
			// Hour Minute Seconds HH:MM:SS.SS col 12-22
			os << hour;
			// Fixed time flag col 23
			try {
				if ( o->timeFixed() )
					os << "f";
				else
					os << " ";
			} catch ( ... ) {
				os << " ";
			}
			// Blank col 24
			os << " ";
			// Origin time error col 25-29
			os << timeErr << " ";
			// RMS travel time residual col 31-35
			os << rms << " ";
			// Latitude col 37-44
			os << latitude << " ";
			// Longitude col 46-54
			os << longitude;
			// Fixed solution flag col 55
			try {
				if ( o->epicenterFixed() )
					os << "f";
				else
					os << " ";
			} catch ( ... ) {
				os << " ";
			}
			// Blank col 56
			os << " ";
			// Semi-major axis ellipsoide (km) col 57-60
			os << erhM << " ";
			// Semi-minor axis ellipsoide (km) col 62-66
			os << erhm << " ";
			// Error ellipsoide strike col 68-70
			os << erhStrike << " ";
			// Depth col 72-76
			os << depth;
			// Fixed depth flag col 77
			os << " ";
			// Blank col 78
			os << " ";
			// Depth error col 79-82
			os << depthErr << " ";
			// Number of defining phases
			// becomes here the number of used phases col 84-87
			os << Nphases << " ";
			// Number of defining stations
			// becomes here the number of used stations col 89-92
			os << Nstations << " ";
			// Azimuthal gap col 94-96
			os << azigap << " ";
			// Distance to nearest station col 98-103
			os << dMin << " ";
			// Distance to furthest station col 105-110
			os << dMax << " ";
			// Analysis type (a for automatic, m for manual, g for guess) col 112
			try {
				os << o->evaluationMode().toString()[0] << " ";
			}
			catch ( ... ) {
				os << "g ";
			}
			// Location method (inversion always assumed) col 114
			os << "i ";
			// Event type col 116-117
			try {
				switch ( e->type() ) {
				case EARTHQUAKE:
					os << "ke";
					break;
				case ROCKSLIDE:
					os << "kr";
					break;
				case INDUCED_EARTHQUAKE:
					os << "ki";
					break;
				case MINE_COLLAPSE:
					os << "km";
					break;
				case EXPLOSION:
					os << "kx";
					break;
				case NUCLEAR_EXPLOSION:
					os << "kn";
					break;
				case LANDSLIDE:
					os << "ls";
					break;
				default:
					os << "uk";
				}
			} catch ( ... ) {
				os << "uk";
			}
			// Blank col 118
			os << " ";
			// Author of the origin col 119-127
			os << author << " ";
			// Origin ID
			os << e->preferredOriginID() << std::endl << std::endl;

	// ** Magnitude Sub-block **
			os << "Magnitude  Err Nsta Author      OrigID" << std::endl;

			if ( m ) {
				std::string magType = stringify("%-5s", m->type().substr(0, 5).c_str());
				std::string magnitude = stringify("%#4.1f", m->magnitude().value());
				if ( magnitude.length() > 4 )
					magnitude = "    ";

				std::string magErr;
				try {
					magErr = stringify("%#3.1f", (m->magnitude().lowerUncertainty() + m->magnitude().lowerUncertainty()) * 0.5);
				} catch ( ... ) {
					try {
						magErr = stringify("%#3.1f", m->magnitude().uncertainty());
					} catch ( ... ) {
						magErr = "   ";
					}
				}
				if ( magErr.length() > 3 )
					magErr = "   ";

				std::string NmagSta;
				try {
					NmagSta = stringify("%#4d", m->stationCount());
				} catch ( ... ) {
					NmagSta = "    ";
				}
				if ( NmagSta.length() > 4 )
					NmagSta = "    ";

				std::string author = stringify("%-9s", m->creationInfo().agencyID().substr(0, 9).c_str());

				// Magnitude code col 1-5
				os << magType;
				// Min Max indicator blank col 6
				os << " ";
				// Magnitude value col 7-10
				os << magnitude << " ";
				// Magnitude error col 12-14
				os << magErr << " ";
				// Number of station magnitudes col 16-19
				os << NmagSta << " ";
				// Magnitude author col 21-29
				os << author << " ";
				// Origin ID
				os << e->preferredOriginID() << std::endl;
			}
			os << std::endl;
	// ** Phase Block **
			os << "Sta     Dist  EvAz Phase        Time      TRes  Azim AzRes   Slow   SRes Def   SNR       Amp   Per Qual Magnitude    ArrID" << std::endl;
			for (size_t k = 0; k < o->arrivalCount(); ++k) {

				PickPtr pick = Pick::Find(o->arrival(k)->pickID());

				if ( !pick )
				    continue;

				std::string staCode = stringify("%-5s", pick->waveformID().stationCode().substr(0, 5).c_str());

				std::string distance;
				try {
					distance = stringify("%#6.2f", o->arrival(k)->distance());
				} catch ( ... ) {
					distance = "      ";
				}
				if ( distance.length() > 6 )
					distance = "      ";

				std::string azimuth;
				try {
					azimuth = stringify("%#5.1f", o->arrival(k)->azimuth());
				} catch ( ... ) {
					azimuth = "     ";
				}
				if ( azimuth.length() > 5 )
					azimuth = "     ";

				std::string phaseCode;
				try {
					phaseCode = stringify("%-8s", o->arrival(k)->phase().code().substr(0, 8).c_str());
				} catch ( ... ) {
					phaseCode = stringify("%-8s", pick->phaseHint().code().substr(0, 8).c_str());
				}

				std::string timeRes;
				try {
					timeRes = stringify("%#5.1f", o->arrival(k)->timeResidual());
				} catch ( ... ) {
					timeRes = "     ";
				}
				if ( timeRes.length() > 5 )
					timeRes = "     ";

				std::string backAzimuth;
				try {
					backAzimuth = stringify("%#5.1f", o->arrival(k)->takeOffAngle());
				} catch ( ... ) {
					backAzimuth = "     ";
				}
				if ( backAzimuth.length() > 5 )
					backAzimuth = "     ";

				std::string backAzRes;
				try {
					backAzRes = stringify("%#5.1f", o->arrival(k)->backazimuthResidual());
				} catch ( ... ) {
					backAzRes = "     ";
				}
				if ( backAzRes.length() > 5 )
					backAzRes = "     ";

				std::string horizSlow;
				try {
					horizSlow = stringify("%#6.1f", pick->horizontalSlowness().value());
				} catch ( ... ) {
					horizSlow = "      ";
				}
				if ( horizSlow.length() > 6 )
					horizSlow = "      ";

				std::string horizSlowRes;
				try {
					horizSlowRes = stringify("%#6.1f", pick->horizontalSlowness().confidenceLevel());
				} catch ( ... ) {
					horizSlowRes = "      ";
				}
				if ( horizSlowRes.length() > 6 )
					horizSlowRes = "      ";

				std::string snr =  "     ";
				std::string amplitude = "         ";
				std::string period = "     ";
				std::string magType = "     ";
				std::string magValue = "    ";
				if ( m ) {
					for (size_t k = 0; k < o->stationMagnitudeCount(); ++k) {
						StationMagnitudePtr staMag = o->stationMagnitude(k);
						if ( staMag->waveformID().stationCode() == pick->waveformID().stationCode()
								&& staMag->waveformID().networkCode() == pick->waveformID().networkCode()
								&& staMag->type() == m->type() ) {
							try {
								magType = stringify("%-5s", staMag->type().substr(0, 5).c_str());
							} catch ( ... ) {}

							try {
								magValue = stringify("%#4.1f", staMag->magnitude().value());
							} catch ( ... ) {}
							if ( magValue.length() > 4 )
								magValue = "    ";

							AmplitudePtr amp = Amplitude::Find(staMag->amplitudeID());
							if ( amp ) {
								try {
									snr = stringify("%#5.1f", amp->snr());
								} catch ( ... ) {}
								if ( snr.length() > 5 )
									snr = "     ";

								try {
									amplitude = stringify("%#9.1f", amp->amplitude().value());
								} catch ( ... ) {}
								if ( amplitude.length() > 9 )
									amplitude = "         ";

								try {
									period = stringify("%#5.2f", amp->period().value());
								} catch ( ... ) {}
								if ( period.length() > 5 )
									period = "     ";
							}
						}
					}
				}

				// Station code col 1-5
				os << staCode << " ";
				// Station to event distance col 7-12
				os << distance << " ";
				// Event to station azimuth col 14-18
				os << azimuth << " ";
				// Phase code col 20-27
				os << phaseCode << " ";
				// Arrival time col 29-40
				os << pick->time().value().toString("%H:%M:%S.%3f") << " ";
				// Time residual col 42-46
				os << timeRes << " ";
				// Observed azimuth col 48-52
				os << backAzimuth << " ";
				// Azimuth residual col 54-58
				os << backAzRes << " ";
				// Observed slowness col 60-65
				os << horizSlow << " ";
				// Slowness residual col 67-72
				os << horizSlowRes << " ";
				// Time defining flag col 74
				os << "_";
				// Azimuth defining flag col 75
				os << "_";
				// Slowness defining flag col 76
				os << "_";
				// Blank col 77
				os << " ";
				// Signal to noise ratio col 78-82
				os << snr << " ";
				// Amplitude col 84-92
				os << amplitude << " ";
				// Period col 94-98
				os << period << " ";
				// Pick type (automatic or manual) col 100
				try {
					os << pick->evaluationMode().toString()[0];
				}
				catch ( ... ) {
					os << "a";
				}
				// Direction of first motion col 101
				try {
					if (pick->polarity() == POSITIVE)
						os << "d";
					else if (pick->polarity() == NEGATIVE)
						os << "c";
					else
						os << "_";
				} catch ( ... ) {
					os << "_";
				}
				// Onset quality col 102
				try {
					os << pick->onset().toString()[0];
				}
				catch ( ... ) {
					os << "_";
				}
				os << " ";
				// Magnitude type col 104-108
				os << magType;
				// Min Max magnitude indicator col 109
				os << " ";
				// Magnitude value col 110-113
				os << magValue << " ";
				// Arrival ID col 115-122
				os << o->arrival(k)->pickID() << std::endl;
			}
			os << std::endl;
		}
	}
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
} // namespace DataModel
} // namesapce Seiscomp
