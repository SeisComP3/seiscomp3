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


#ifndef __SEISCOMP_DATAMODEL_MAGNITUDE_H__
#define __SEISCOMP_DATAMODEL_MAGNITUDE_H__


#include <seiscomp3/datamodel/creationinfo.h>
#include <vector>
#include <string>
#include <seiscomp3/datamodel/realquantity.h>
#include <seiscomp3/datamodel/types.h>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/datamodel/stationmagnitudecontribution.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Magnitude);
DEFINE_SMARTPOINTER(Comment);
DEFINE_SMARTPOINTER(StationMagnitudeContribution);

class Origin;


/**
 * \brief Describes a magnitude which can, but does not need to be
 * \brief associated with
 * \brief an origin. Association with an origin is expressed with the
 * \brief optional
 * \brief attribute originID. It is either a combination of different
 * \brief magnitude
 * \brief estimations, or it represents the reported magnitude for
 * \brief the given event.
 */
class SC_SYSTEM_CORE_API Magnitude : public PublicObject {
	DECLARE_SC_CLASS(Magnitude);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Magnitude();

	public:
		//! Copy constructor
		Magnitude(const Magnitude& other);

		//! Constructor with publicID
		Magnitude(const std::string& publicID);

		//! Destructor
		~Magnitude();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Magnitude* Create();
		static Magnitude* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Magnitude* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Magnitude& operator=(const Magnitude& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Magnitude& other) const;
		bool operator!=(const Magnitude& other) const;

		//! Wrapper that calls operator==
		bool equal(const Magnitude& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Resulting magnitude value from combining values of type
		//! StationMagnitude.
		//! If no estimations are available, this value can represent
		//! the reported magnitude.
		void setMagnitude(const RealQuantity& magnitude);
		RealQuantity& magnitude();
		const RealQuantity& magnitude() const;

		//! Describes the type of magnitude. This is a free-text field
		//! because
		//! it is impossible to cover all existing magnitude type
		//! designations
		//! with an enumeration. Possible values are unspecified
		//! magitude (M),
		//! local magnitude (ML), body wave magnitude (Mb),
		//! surface wave magnitude (MS), moment magnitude (Mw),
		//! duration magnitude (Md), coda magnitude (Mc), MH, Mwp, M50,
		//! M100, etc.
		void setType(const std::string& type);
		const std::string& type() const;

		//! Reference to an origin's publicID if the magnitude has an
		//! associated Origin.
		void setOriginID(const std::string& originID);
		const std::string& originID() const;

		//! Identifies the method of magnitude estimation. Users should
		//! avoid to
		//! give contradictory information in methodID and type.
		void setMethodID(const std::string& methodID);
		const std::string& methodID() const;

		//! Number of used stations for this magnitude computation.
		void setStationCount(const OPT(int)& stationCount);
		int stationCount() const;

		//! Azimuthal gap for this magnitude computation in degrees.
		void setAzimuthalGap(const OPT(double)& azimuthalGap);
		double azimuthalGap() const;

		//! Evaluation status of Magnitude.
		void setEvaluationStatus(const OPT(EvaluationStatus)& evaluationStatus);
		EvaluationStatus evaluationStatus() const;

		//! CreationInfo for the Magnitude object.
		void setCreationInfo(const OPT(CreationInfo)& creationInfo);
		CreationInfo& creationInfo();
		const CreationInfo& creationInfo() const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * Add an object.
		 * @param obj The object pointer
		 * @return true The object has been added
		 * @return false The object has not been added
		 *               because it already exists in the list
		 *               or it already has another parent
		 */
		bool add(Comment* obj);
		bool add(StationMagnitudeContribution* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(Comment* obj);
		bool remove(StationMagnitudeContribution* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeComment(size_t i);
		bool removeComment(const CommentIndex& i);
		bool removeStationMagnitudeContribution(size_t i);
		bool removeStationMagnitudeContribution(const StationMagnitudeContributionIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t commentCount() const;
		size_t stationMagnitudeContributionCount() const;

		//! Index access
		//! @return The object at index i
		Comment* comment(size_t i) const;
		Comment* comment(const CommentIndex& i) const;

		StationMagnitudeContribution* stationMagnitudeContribution(size_t i) const;
		StationMagnitudeContribution* stationMagnitudeContribution(const StationMagnitudeContributionIndex& i) const;

		//! Find an object by its unique attribute(s)

		Origin* origin() const;

		//! Implement Object interface
		bool assign(Object* other);
		bool attachTo(PublicObject* parent);
		bool detachFrom(PublicObject* parent);
		bool detach();

		//! Creates a clone
		Object* clone() const;

		//! Implement PublicObject interface
		bool updateChild(Object* child);

		void accept(Visitor*);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		RealQuantity _magnitude;
		std::string _type;
		std::string _originID;
		std::string _methodID;
		OPT(int) _stationCount;
		OPT(double) _azimuthalGap;
		OPT(EvaluationStatus) _evaluationStatus;
		OPT(CreationInfo) _creationInfo;

		// Aggregations
		std::vector<CommentPtr> _comments;
		std::vector<StationMagnitudeContributionPtr> _stationMagnitudeContributions;

	DECLARE_SC_CLASSFACTORY_FRIEND(Magnitude);
};


}
}


#endif
