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



#ifndef _SEISCOMP_TTT_LOCSAT_H_
#define _SEISCOMP_TTT_LOCSAT_H_


#include <string>
#include <vector>
#include <seiscomp3/seismology/ttt.h>


namespace Seiscomp {
namespace TTT {


/**
 * TTTLibTau
 *
 * A class to compute seismic travel times for 1D models like "iasp91".
 */
class SC_SYSTEM_CORE_API Locsat : public TravelTimeTableInterface {
	public:
		struct Velocity {
			Velocity() {}
			Velocity(float z, float p, float s) : depth(z), vp(p), vs(s) {}
			float depth;
			float vp;
			float vs;
		};

	public:
		/**
		 * Construct a TravelTimeTable object for the specified model.
		 * Currently only "iasp91" is supported.
		 */
		Locsat();
		~Locsat();

		Locsat(const Locsat &other);
		Locsat &operator=(const Locsat &other);


	public:
		bool setModel(const std::string &model);
		const std::string &model() const;


		/**
		 * Compute the traveltime(s) for the branch selected using setBranch()
		 * @param dep1 The source depth in km
		 *
		 * @returns A TravelTimeList of travel times sorted by time.
		 *
		 * XXX However:
		 * XXX NEITHER ellipticity NOR altitude correction is currently
		 * XXX implemented! The respective parameters are ignored.
		 */
		TravelTimeList *compute(double lat1, double lon1, double dep1,
		                        double lat2, double lon2, double alt2 = 0.,
		                        int ellc = 1);


		/**
		 * Compute the traveltime for the branch selected using setBranch()
		 * and the first (fastest) phase.
		 * @param dep1 The source depth in km
		 *
		 * @returns A TravelTime
		 *
		 * XXX However:
		 * XXX NEITHER ellipticity NOR altitude correction is currently
		 * XXX implemented! The respective parameters are ignored.
		 */
		TravelTime computeFirst(double lat1, double lon1, double dep1,
		                        double lat2, double lon2, double alt2 = 0.,
		                        int ellc = 1);


	private:
		TravelTimeList *compute(double delta, double depth);
		TravelTime computeFirst(double delta, double depth);

		void InitPath(const std::string &model);

		static std::string _model;
		static int _tabinCount;

		//static std::vector<Velocity> _velocities;

		bool _initialized;
		int  _Pindex;
};


}
}


#endif
