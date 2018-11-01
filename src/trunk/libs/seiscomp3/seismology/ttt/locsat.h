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
		 * @brief Compute the traveltime(s) for the model selected using
		 *        setModel().
		 *
		 * Note that altitude correction is currently not implemented! The
		 * respective parameters are ignored.
		 * @param dep1 The source depth in km
		 *
		 * @returns A TravelTimeList of travel times sorted by time.
		 */
		virtual TravelTimeList *compute(double lat1, double lon1, double dep1,
		                                double lat2, double lon2, double alt2 = 0.,
		                                int ellc = 1);

		/**
		 * Compute the traveltime and a given phase. The default implementation
		 * computes the complete travel time list and searches for them
		 * requested phase.
		 * @param dep1 The source depth in km
		 *
		 * @returns A TravelTime
		 */
		virtual TravelTime compute(const char *phase,
		                           double lat1, double lon1, double dep1,
		                           double lat2, double lon2, double alt2=0.,
		                           int ellc = 1);

		/**
		 * @brief Compute the traveltime for the model selected using setModel()
		 *        and the first (fastest) phase.
		 *
		 * Note that altitude correction is currently not implemented! The
		 * respective parameters are ignored.
		 * @param dep1 The source depth in km
		 * @returns A TravelTime
		 */
		TravelTime computeFirst(double lat1, double lon1, double dep1,
		                        double lat2, double lon2, double alt2 = 0.,
		                        int ellc = 1);


	private:
		TravelTimeList *compute(double delta, double depth);
		TravelTime compute(const char *phase, double delta, double depth);
		TravelTime computeFirst(double delta, double depth);

		bool initTables();


	private:
		std::string _model;
		std::string _tablePrefix;
		int         _Pindex;
};


}
}


#endif
