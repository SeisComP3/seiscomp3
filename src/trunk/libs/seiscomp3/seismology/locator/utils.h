/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef SEISCOMP3_SEISMOLOGY_LOCATOR_UTILS_H
#define SEISCOMP3_SEISMOLOGY_LOCATOR_UTILS_H


namespace Seiscomp{

namespace DataModel {

class Origin;
class OriginQuality;

}


/**
 * @brief Compiles an origin quality object from an origin.
 * Computed attributes:
 *  * minimum distance
 *  * median distance
 *  * maximum distance
 *  * azimuthal gap
 *  * associated phase count
 *  * used phase count
 *  * depth phase count
 *  * associated station count
 *  * used station count
 *
 * Preconditions are:
 *  * Arrivals must be present to generate phase counts
 *  * Arrivals must have azimuth and distance set to generate azimuthal gap and
 *    min/max/median distances
 *  * Picks must be resolvable (Pick::Find) to update station counts
 *
 * Values that cannot be determined will not be written into the output quality,
 * meaning that this function will never set an output attribute to None.
 *
 * @param quality The output quality
 * @param origin The input origin
 */
void compile(DataModel::OriginQuality &quality, const DataModel::Origin *origin);


/**
 * @brief Convenience function which populates the origin quality of an origin
 *        object.
 * If the origin holds already a quality object then it will be updated.
 * Otherwise a new quality object will be set.
 * @param origin The origin that will receive the quality object.
 */
void populateQuality(DataModel::Origin *origin);



}


#endif
