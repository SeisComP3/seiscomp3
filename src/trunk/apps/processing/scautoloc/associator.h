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




#ifndef _SEISCOMP_AUTOLOC_ASSOCIATOR_
#define _SEISCOMP_AUTOLOC_ASSOCIATOR_

#include "datamodel.h"

namespace Autoloc {


typedef Arrival Association;

/*
class Association
{
  public:
	Association(const Pick *pick, const Origin *origin, double affinity=0, double residual=0)
		: pick(pick), origin(origin), affinity(affinity), residual(residual)
	{ }

  public:
	const Pick   *pick;
	const Origin *origin;

	double affinity, residual;
};
*/

class AssociationVector : public std::vector<Association>
{
  public:
};


// Simple pick-to-origin associator. Can only associate P phases.
//
// TODO: Also would be good to be able to associate all picks to one origin,
// also for other phases than P
class Associator
{
  public:
	class Phase;

  public:
	Associator();
	~Associator();

  public:
	void setStations(const StationMap *stations);
	void setOrigins(const OriginVector *origins);

  public:
	// Feed a pick and try to associate it with known origins, under the
	// assumption that this is a P pick.
	//
	// The associated origins are then retrieved using associations()
	bool feed(const Pick* pick);

	// Retrieve associations made during the last call of feed()
	const AssociationVector& associations() const;

	Association* associate(Origin *origin, const Pick *pick, const std::string &phase);

  public:
	void reset();
	void shutdown();

  protected:
	// these are not owned:
	const StationMap *_stations;
	const OriginVector  *_origins;

  private:
	AssociationVector _associations;
	std::vector<Phase> _phases;
};


class Associator::Phase
{
  public:
	Phase(const std::string&, double, double);
        std::string code;
	double dmin, dmax;
};

} // namespace Autoloc

#endif
