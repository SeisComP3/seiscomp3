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


#ifndef __SEISCOMP_DATAMODEL_ARRIVAL_H__
#define __SEISCOMP_DATAMODEL_ARRIVAL_H__


#include <seiscomp3/datamodel/creationinfo.h>
#include <string>
#include <seiscomp3/datamodel/phase.h>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Arrival);

class Origin;


class SC_SYSTEM_CORE_API ArrivalIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ArrivalIndex();
		ArrivalIndex(const std::string& pickID);

		//! Copy constructor
		ArrivalIndex(const ArrivalIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const ArrivalIndex&) const;
		bool operator!=(const ArrivalIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string pickID;
};


/**
 * \brief Successful association of a pick with an origin qualifies
 * \brief this pick as
 * \brief an arrival. An arrival thus connects a pick with an origin
 * \brief and provides
 * \brief additional attributes that describe this relationship.
 * \brief Usually qualification
 * \brief of a pick as an arrival for a given origin is a hypothesis,
 * \brief which is
 * \brief based on assumptions about the type of arrival (phase) as
 * \brief well as
 * \brief observed and (on the basis of an earth model) computed
 * \brief arrival times,
 * \brief or the residual, respectively. Additional pick attributes
 * \brief like the
 * \brief horizontal slowness and backazimuth of the observed
 * \brief wave-especially if
 * \brief derived from array data-may further constrain the nature of
 * \brief the arrival.
 */
class SC_SYSTEM_CORE_API Arrival : public Object {
	DECLARE_SC_CLASS(Arrival);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Arrival();

		//! Copy constructor
		Arrival(const Arrival& other);

		//! Destructor
		~Arrival();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Arrival& operator=(const Arrival& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Arrival& other) const;
		bool operator!=(const Arrival& other) const;

		//! Wrapper that calls operator==
		bool equal(const Arrival& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Refers to a publicID of a Pick.
		void setPickID(const std::string& pickID);
		const std::string& pickID() const;

		//! Phase identification. For possible values, please refer to
		//! the description of the Phase type.
		void setPhase(const Phase& phase);
		Phase& phase();
		const Phase& phase() const;

		//! Time correction value. Usually, a value characteristic for
		//! the station
		//! at which the pick was detected, sometimes also
		//! characteristic for the
		//! phase type or the slowness in seconds.
		void setTimeCorrection(const OPT(double)& timeCorrection);
		double timeCorrection() const;

		//! Azimuth of station as seen from the epicenter in degrees.
		void setAzimuth(const OPT(double)& azimuth);
		double azimuth() const;

		//! Epicentral distance in degrees.
		void setDistance(const OPT(double)& distance);
		double distance() const;

		//! Angle of emerging ray at the source, measured against the
		//! downward
		//! normal direction in degrees.
		void setTakeOffAngle(const OPT(double)& takeOffAngle);
		double takeOffAngle() const;

		//! Residual between observed and expected arrival time
		//! assuming proper
		//! phase identification and given the earthModelID of the
		//! Origin,
		//! taking into account the timeCorrection in seconds.
		void setTimeResidual(const OPT(double)& timeResidual);
		double timeResidual() const;

		//! Residual of horizontal slowness and the expected slowness
		//! given the
		//! current origin (refers to attribute horizontalSlowness of
		//! class Pick)
		//! in s/deg.
		void setHorizontalSlownessResidual(const OPT(double)& horizontalSlownessResidual);
		double horizontalSlownessResidual() const;

		//! Residual of backazimuth and the backazimuth computed for
		//! the current
		//! origin (refers to attribute backazimuth of class Pick) in
		//! degrees.
		void setBackazimuthResidual(const OPT(double)& backazimuthResidual);
		double backazimuthResidual() const;

		void setTimeUsed(const OPT(bool)& timeUsed);
		bool timeUsed() const;

		//! Weight of the horizontal slowness for computation of the
		//! associated Origin.
		//! Note that the sum of all weights is not required to be
		//! unity.
		void setHorizontalSlownessUsed(const OPT(bool)& horizontalSlownessUsed);
		bool horizontalSlownessUsed() const;

		void setBackazimuthUsed(const OPT(bool)& backazimuthUsed);
		bool backazimuthUsed() const;

		//! Weight of the arrival time for computation of the
		//! associated Origin.
		//! Note that the sum of all weights is not required to be
		//! unity.
		void setWeight(const OPT(double)& weight);
		double weight() const;

		//! Earth model which is used for the association of Arrival to
		//! Pick and computation of the
		//! residuals.
		void setEarthModelID(const std::string& earthModelID);
		const std::string& earthModelID() const;

		//! Indicates if the arrival is preliminary.
		void setPreliminary(const OPT(bool)& preliminary);
		bool preliminary() const;

		//! CreationInfo for the Arrival object.
		void setCreationInfo(const OPT(CreationInfo)& creationInfo);
		CreationInfo& creationInfo();
		const CreationInfo& creationInfo() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const ArrivalIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const Arrival* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		Origin* origin() const;

		//! Implement Object interface
		bool assign(Object* other);
		bool attachTo(PublicObject* parent);
		bool detachFrom(PublicObject* parent);
		bool detach();

		//! Creates a clone
		Object* clone() const;

		void accept(Visitor*);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Index
		ArrivalIndex _index;

		// Attributes
		Phase _phase;
		OPT(double) _timeCorrection;
		OPT(double) _azimuth;
		OPT(double) _distance;
		OPT(double) _takeOffAngle;
		OPT(double) _timeResidual;
		OPT(double) _horizontalSlownessResidual;
		OPT(double) _backazimuthResidual;
		OPT(bool) _timeUsed;
		OPT(bool) _horizontalSlownessUsed;
		OPT(bool) _backazimuthUsed;
		OPT(double) _weight;
		std::string _earthModelID;
		OPT(bool) _preliminary;
		OPT(CreationInfo) _creationInfo;
};


}
}


#endif
