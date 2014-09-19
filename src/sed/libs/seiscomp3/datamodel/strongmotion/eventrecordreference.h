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


#ifndef __SEISCOMP_DATAMODEL_STRONGMOTION_EVENTRECORDREFERENCE_H__
#define __SEISCOMP_DATAMODEL_STRONGMOTION_EVENTRECORDREFERENCE_H__


#include <string>
#include <seiscomp3/datamodel/realquantity.h>
#include <seiscomp3/datamodel/object.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/strongmotion/api.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


DEFINE_SMARTPOINTER(EventRecordReference);

class StrongOriginDescription;


class SC_STRONGMOTION_API EventRecordReference : public Object {
	DECLARE_SC_CLASS(EventRecordReference);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		EventRecordReference();

		//! Copy constructor
		EventRecordReference(const EventRecordReference& other);

		//! Custom constructor
		EventRecordReference(const std::string& recordID);
		EventRecordReference(const std::string& recordID,
		                     const OPT(RealQuantity)& campbellDistance,
		                     const OPT(RealQuantity)& ruptureToStationAzimuth,
		                     const OPT(RealQuantity)& ruptureAreaDistance,
		                     const OPT(RealQuantity)& JoynerBooreDistance,
		                     const OPT(RealQuantity)& closestFaultDistance,
		                     const OPT(double)& preEventLength,
		                     const OPT(double)& postEventLength);

		//! Destructor
		~EventRecordReference();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		EventRecordReference& operator=(const EventRecordReference& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const EventRecordReference& other) const;
		bool operator!=(const EventRecordReference& other) const;

		//! Wrapper that calls operator==
		bool equal(const EventRecordReference& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setRecordID(const std::string& recordID);
		const std::string& recordID() const;

		void setCampbellDistance(const OPT(RealQuantity)& campbellDistance);
		RealQuantity& campbellDistance() throw(Seiscomp::Core::ValueException);
		const RealQuantity& campbellDistance() const throw(Seiscomp::Core::ValueException);

		void setRuptureToStationAzimuth(const OPT(RealQuantity)& ruptureToStationAzimuth);
		RealQuantity& ruptureToStationAzimuth() throw(Seiscomp::Core::ValueException);
		const RealQuantity& ruptureToStationAzimuth() const throw(Seiscomp::Core::ValueException);

		void setRuptureAreaDistance(const OPT(RealQuantity)& ruptureAreaDistance);
		RealQuantity& ruptureAreaDistance() throw(Seiscomp::Core::ValueException);
		const RealQuantity& ruptureAreaDistance() const throw(Seiscomp::Core::ValueException);

		void setJoynerBooreDistance(const OPT(RealQuantity)& JoynerBooreDistance);
		RealQuantity& JoynerBooreDistance() throw(Seiscomp::Core::ValueException);
		const RealQuantity& JoynerBooreDistance() const throw(Seiscomp::Core::ValueException);

		void setClosestFaultDistance(const OPT(RealQuantity)& closestFaultDistance);
		RealQuantity& closestFaultDistance() throw(Seiscomp::Core::ValueException);
		const RealQuantity& closestFaultDistance() const throw(Seiscomp::Core::ValueException);

		void setPreEventLength(const OPT(double)& preEventLength);
		double preEventLength() const throw(Seiscomp::Core::ValueException);

		void setPostEventLength(const OPT(double)& postEventLength);
		double postEventLength() const throw(Seiscomp::Core::ValueException);

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		StrongOriginDescription* strongOriginDescription() const;

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
		// Attributes
		std::string _recordID;
		OPT(RealQuantity) _campbellDistance;
		OPT(RealQuantity) _ruptureToStationAzimuth;
		OPT(RealQuantity) _ruptureAreaDistance;
		OPT(RealQuantity) _joynerBooreDistance;
		OPT(RealQuantity) _closestFaultDistance;
		OPT(double) _preEventLength;
		OPT(double) _postEventLength;
};


}
}
}


#endif
