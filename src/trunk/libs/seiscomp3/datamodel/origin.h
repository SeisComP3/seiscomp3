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

// This file was created by a source code generator.
// Do not modify the contents. Change the definition and run the generator
// again!


#ifndef __SEISCOMP_DATAMODEL_ORIGIN_H__
#define __SEISCOMP_DATAMODEL_ORIGIN_H__


#include <seiscomp3/datamodel/creationinfo.h>
#include <seiscomp3/datamodel/originuncertainty.h>
#include <string>
#include <seiscomp3/datamodel/originquality.h>
#include <seiscomp3/datamodel/types.h>
#include <vector>
#include <seiscomp3/datamodel/timequantity.h>
#include <seiscomp3/datamodel/realquantity.h>
#include <seiscomp3/datamodel/comment.h>
#include <seiscomp3/datamodel/arrival.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Origin);
DEFINE_SMARTPOINTER(Comment);
DEFINE_SMARTPOINTER(CompositeTime);
DEFINE_SMARTPOINTER(Arrival);
DEFINE_SMARTPOINTER(StationMagnitude);
DEFINE_SMARTPOINTER(Magnitude);

class EventParameters;


/**
 * \brief This class represents the focal time and geographical
 * \brief location of an
 * \brief earthquake hypocenter, as well as additional
 * \brief meta-information. Origin
 * \brief can have objects of type OriginUncertainty and Arrival as
 * \brief child elements.
 */
class SC_SYSTEM_CORE_API Origin : public PublicObject {
	DECLARE_SC_CLASS(Origin);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Origin();

	public:
		//! Copy constructor
		Origin(const Origin& other);

		//! Constructor with publicID
		Origin(const std::string& publicID);

		//! Destructor
		~Origin();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Origin* Create();
		static Origin* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Origin* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Origin& operator=(const Origin& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Origin& other) const;
		bool operator!=(const Origin& other) const;

		//! Wrapper that calls operator==
		bool equal(const Origin& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Focal time as TimeQuantity.
		void setTime(const TimeQuantity& time);
		TimeQuantity& time();
		const TimeQuantity& time() const;

		//! Hypocenter longitude, with respect to the World Geodetic
		//! System 1984 (WGS84) reference system
		//! (National Imagery and Mapping Agency 2000) in degrees. The
		//! uncertainties are given in kilometers.
		void setLatitude(const RealQuantity& latitude);
		RealQuantity& latitude();
		const RealQuantity& latitude() const;

		//! Hypocenter latitude, with respect to the WGS84 reference
		//! system in degrees.
		//! The uncertainties are given in kilometers.
		void setLongitude(const RealQuantity& longitude);
		RealQuantity& longitude();
		const RealQuantity& longitude() const;

		//! Depth of hypocenter with respect to the nominal sea level
		//! given by the
		//! WGS84 geoid (Earth Gravitational Model, EGM96, Lemoine et
		//! al. 1998).
		//! Positive values indicate hypocenters below sea level. For
		//! shallow
		//! hypocenters, the depth value can be negative. The depth is
		//! defined
		//! in km.
		void setDepth(const OPT(RealQuantity)& depth);
		RealQuantity& depth();
		const RealQuantity& depth() const;

		//! Type of depth determination.
		void setDepthType(const OPT(OriginDepthType)& depthType);
		OriginDepthType depthType() const;

		//! Boolean flag. True if focal time was kept fixed for
		//! computation of the Origin.
		void setTimeFixed(const OPT(bool)& timeFixed);
		bool timeFixed() const;

		//! Boolean flag. True if epicenter was kept fixed for
		//! computation of Origin.
		void setEpicenterFixed(const OPT(bool)& epicenterFixed);
		bool epicenterFixed() const;

		//! Identifies the reference system used for hypocenter
		//! determination. This is only necessary if
		//! a modified version of the standard (with local extensions)
		//! is used that provides a non-standard coordinate
		//! system.
		void setReferenceSystemID(const std::string& referenceSystemID);
		const std::string& referenceSystemID() const;

		//! Identifies the method used for locating the event.
		void setMethodID(const std::string& methodID);
		const std::string& methodID() const;

		//! Identifies the earth model used in methodID.
		void setEarthModelID(const std::string& earthModelID);
		const std::string& earthModelID() const;

		//! Additional parameters describing the quality of an Origin
		//! determination.
		void setQuality(const OPT(OriginQuality)& quality);
		OriginQuality& quality();
		const OriginQuality& quality() const;

		//! Additional parameters describing the uncertainty of an
		//! Origin determination.
		void setUncertainty(const OPT(OriginUncertainty)& uncertainty);
		OriginUncertainty& uncertainty();
		const OriginUncertainty& uncertainty() const;

		//! Describes the Origin type.
		void setType(const OPT(OriginType)& type);
		OriginType type() const;

		//! Evaluation mode of Origin.
		void setEvaluationMode(const OPT(EvaluationMode)& evaluationMode);
		EvaluationMode evaluationMode() const;

		//! Evaluation status of Origin.
		void setEvaluationStatus(const OPT(EvaluationStatus)& evaluationStatus);
		EvaluationStatus evaluationStatus() const;

		//! CreationInfo for the Origin object.
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
		bool add(CompositeTime* obj);
		bool add(Arrival* obj);
		bool add(StationMagnitude* obj);
		bool add(Magnitude* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(Comment* obj);
		bool remove(CompositeTime* obj);
		bool remove(Arrival* obj);
		bool remove(StationMagnitude* obj);
		bool remove(Magnitude* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeComment(size_t i);
		bool removeComment(const CommentIndex& i);
		bool removeCompositeTime(size_t i);
		bool removeArrival(size_t i);
		bool removeArrival(const ArrivalIndex& i);
		bool removeStationMagnitude(size_t i);
		bool removeMagnitude(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t commentCount() const;
		size_t compositeTimeCount() const;
		size_t arrivalCount() const;
		size_t stationMagnitudeCount() const;
		size_t magnitudeCount() const;

		//! Index access
		//! @return The object at index i
		Comment* comment(size_t i) const;
		Comment* comment(const CommentIndex& i) const;
		CompositeTime* compositeTime(size_t i) const;

		Arrival* arrival(size_t i) const;
		Arrival* arrival(const ArrivalIndex& i) const;
		StationMagnitude* stationMagnitude(size_t i) const;
		Magnitude* magnitude(size_t i) const;

		//! Find an object by its unique attribute(s)
		CompositeTime* findCompositeTime(CompositeTime* compositeTime) const;
		StationMagnitude* findStationMagnitude(const std::string& publicID) const;
		Magnitude* findMagnitude(const std::string& publicID) const;

		EventParameters* eventParameters() const;

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
		TimeQuantity _time;
		RealQuantity _latitude;
		RealQuantity _longitude;
		OPT(RealQuantity) _depth;
		OPT(OriginDepthType) _depthType;
		OPT(bool) _timeFixed;
		OPT(bool) _epicenterFixed;
		std::string _referenceSystemID;
		std::string _methodID;
		std::string _earthModelID;
		OPT(OriginQuality) _quality;
		OPT(OriginUncertainty) _uncertainty;
		OPT(OriginType) _type;
		OPT(EvaluationMode) _evaluationMode;
		OPT(EvaluationStatus) _evaluationStatus;
		OPT(CreationInfo) _creationInfo;

		// Aggregations
		std::vector<CommentPtr> _comments;
		std::vector<CompositeTimePtr> _compositeTimes;
		std::vector<ArrivalPtr> _arrivals;
		std::vector<StationMagnitudePtr> _stationMagnitudes;
		std::vector<MagnitudePtr> _magnitudes;

	DECLARE_SC_CLASSFACTORY_FRIEND(Origin);
};


}
}


#endif
