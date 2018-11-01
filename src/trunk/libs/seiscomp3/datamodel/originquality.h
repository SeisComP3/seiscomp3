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


#ifndef __SEISCOMP_DATAMODEL_ORIGINQUALITY_H__
#define __SEISCOMP_DATAMODEL_ORIGINQUALITY_H__


#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(OriginQuality);


/**
 * \brief This type contains various attributes commonly used to
 * \brief describe the quality of an origin, e. g., errors, azimuthal
 * \brief coverage, etc. Origin objects have an optional attribute of
 * \brief the type OriginQuality.
 */
class SC_SYSTEM_CORE_API OriginQuality : public Core::BaseObject {
	DECLARE_SC_CLASS(OriginQuality);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		OriginQuality();

		//! Copy constructor
		OriginQuality(const OriginQuality& other);

		//! Destructor
		~OriginQuality();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		OriginQuality& operator=(const OriginQuality& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const OriginQuality& other) const;
		bool operator!=(const OriginQuality& other) const;

		//! Wrapper that calls operator==
		bool equal(const OriginQuality& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Number of associated phases, regardless of their use for
		//! origin computation.
		void setAssociatedPhaseCount(const OPT(int)& associatedPhaseCount);
		int associatedPhaseCount() const;

		//! Number of defining phases, i. e., phase observations that
		//! were actually used for computing
		//! the origin. Note that there may be more than one defining
		//! phase per station.
		void setUsedPhaseCount(const OPT(int)& usedPhaseCount);
		int usedPhaseCount() const;

		//! Number of stations at which the event was observed.
		void setAssociatedStationCount(const OPT(int)& associatedStationCount);
		int associatedStationCount() const;

		//! Number of stations from which data was used for origin
		//! computation.
		void setUsedStationCount(const OPT(int)& usedStationCount);
		int usedStationCount() const;

		//! Number of depth phases (typically pP, sometimes sP) used in
		//! depth computation.
		void setDepthPhaseCount(const OPT(int)& depthPhaseCount);
		int depthPhaseCount() const;

		//! RMS of the travel time residuals of the arrivals used for
		//! the origin computation
		//! in seconds.
		void setStandardError(const OPT(double)& standardError);
		double standardError() const;

		//! Largest azimuthal gap in station distribution as seen from
		//! epicenter
		//! in degrees.
		void setAzimuthalGap(const OPT(double)& azimuthalGap);
		double azimuthalGap() const;

		//! Secondary azimuthal gap in station distribution, i. e., the
		//! largest
		//! azimuthal gap a station closes in degrees.
		void setSecondaryAzimuthalGap(const OPT(double)& secondaryAzimuthalGap);
		double secondaryAzimuthalGap() const;

		//! String describing ground-truth level, e. g. GT0, GT5, etc.
		//! It has a maximum length of 16
		//! characters.
		void setGroundTruthLevel(const std::string& groundTruthLevel);
		const std::string& groundTruthLevel() const;

		//! Epicentral distance of station farthest from the epicenter
		//! in degrees.
		void setMaximumDistance(const OPT(double)& maximumDistance);
		double maximumDistance() const;

		//! Epicentral distance of station closest to the epicenter in
		//! degrees.
		void setMinimumDistance(const OPT(double)& minimumDistance);
		double minimumDistance() const;

		//! Median epicentral distance of used stations in degrees.
		void setMedianDistance(const OPT(double)& medianDistance);
		double medianDistance() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		OPT(int) _associatedPhaseCount;
		OPT(int) _usedPhaseCount;
		OPT(int) _associatedStationCount;
		OPT(int) _usedStationCount;
		OPT(int) _depthPhaseCount;
		OPT(double) _standardError;
		OPT(double) _azimuthalGap;
		OPT(double) _secondaryAzimuthalGap;
		std::string _groundTruthLevel;
		OPT(double) _maximumDistance;
		OPT(double) _minimumDistance;
		OPT(double) _medianDistance;
};


}
}


#endif
