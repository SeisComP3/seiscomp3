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


#ifndef __SEISCOMP_DATAMODEL_STRONGMOTION_RUPTURE_H__
#define __SEISCOMP_DATAMODEL_STRONGMOTION_RUPTURE_H__


#include <string>
#include <seiscomp3/datamodel/strongmotion/surfacerupture.h>
#include <seiscomp3/datamodel/strongmotion/literaturesource.h>
#include <seiscomp3/datamodel/realquantity.h>
#include <seiscomp3/datamodel/strongmotion/types.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/strongmotion/api.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


DEFINE_SMARTPOINTER(Rupture);

class StrongOriginDescription;


class SC_STRONGMOTION_API Rupture : public PublicObject {
	DECLARE_SC_CLASS(Rupture);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Rupture();

	public:
		//! Copy constructor
		Rupture(const Rupture& other);

		//! Constructor with publicID
		Rupture(const std::string& publicID);

		//! Destructor
		~Rupture();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Rupture* Create();
		static Rupture* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Rupture* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Rupture& operator=(const Rupture& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Rupture& other) const;
		bool operator!=(const Rupture& other) const;

		//! Wrapper that calls operator==
		bool equal(const Rupture& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setWidth(const OPT(RealQuantity)& width);
		RealQuantity& width() throw(Seiscomp::Core::ValueException);
		const RealQuantity& width() const throw(Seiscomp::Core::ValueException);

		void setDisplacement(const OPT(RealQuantity)& displacement);
		RealQuantity& displacement() throw(Seiscomp::Core::ValueException);
		const RealQuantity& displacement() const throw(Seiscomp::Core::ValueException);

		void setRiseTime(const OPT(RealQuantity)& riseTime);
		RealQuantity& riseTime() throw(Seiscomp::Core::ValueException);
		const RealQuantity& riseTime() const throw(Seiscomp::Core::ValueException);

		void setVtToVs(const OPT(RealQuantity)& vtToVs);
		RealQuantity& vtToVs() throw(Seiscomp::Core::ValueException);
		const RealQuantity& vtToVs() const throw(Seiscomp::Core::ValueException);

		void setShallowAsperityDepth(const OPT(RealQuantity)& shallowAsperityDepth);
		RealQuantity& shallowAsperityDepth() throw(Seiscomp::Core::ValueException);
		const RealQuantity& shallowAsperityDepth() const throw(Seiscomp::Core::ValueException);

		void setShallowAsperity(const OPT(bool)& shallowAsperity);
		bool shallowAsperity() const throw(Seiscomp::Core::ValueException);

		void setLiteratureSource(const OPT(LiteratureSource)& literatureSource);
		LiteratureSource& literatureSource() throw(Seiscomp::Core::ValueException);
		const LiteratureSource& literatureSource() const throw(Seiscomp::Core::ValueException);

		void setSlipVelocity(const OPT(RealQuantity)& slipVelocity);
		RealQuantity& slipVelocity() throw(Seiscomp::Core::ValueException);
		const RealQuantity& slipVelocity() const throw(Seiscomp::Core::ValueException);

		void setStrike(const OPT(RealQuantity)& strike);
		RealQuantity& strike() throw(Seiscomp::Core::ValueException);
		const RealQuantity& strike() const throw(Seiscomp::Core::ValueException);

		void setLength(const OPT(RealQuantity)& length);
		RealQuantity& length() throw(Seiscomp::Core::ValueException);
		const RealQuantity& length() const throw(Seiscomp::Core::ValueException);

		void setArea(const OPT(RealQuantity)& area);
		RealQuantity& area() throw(Seiscomp::Core::ValueException);
		const RealQuantity& area() const throw(Seiscomp::Core::ValueException);

		void setRuptureVelocity(const OPT(RealQuantity)& ruptureVelocity);
		RealQuantity& ruptureVelocity() throw(Seiscomp::Core::ValueException);
		const RealQuantity& ruptureVelocity() const throw(Seiscomp::Core::ValueException);

		void setStressdrop(const OPT(RealQuantity)& stressdrop);
		RealQuantity& stressdrop() throw(Seiscomp::Core::ValueException);
		const RealQuantity& stressdrop() const throw(Seiscomp::Core::ValueException);

		void setMomentReleaseTop5km(const OPT(RealQuantity)& momentReleaseTop5km);
		RealQuantity& momentReleaseTop5km() throw(Seiscomp::Core::ValueException);
		const RealQuantity& momentReleaseTop5km() const throw(Seiscomp::Core::ValueException);

		void setFwHwIndicator(const OPT(FwHwIndicator)& fwHwIndicator);
		FwHwIndicator fwHwIndicator() const throw(Seiscomp::Core::ValueException);

		void setRuptureGeometryWKT(const std::string& ruptureGeometryWKT);
		const std::string& ruptureGeometryWKT() const;

		void setFaultID(const std::string& faultID);
		const std::string& faultID() const;

		void setSurfaceRupture(const OPT(SurfaceRupture)& surfaceRupture);
		SurfaceRupture& surfaceRupture() throw(Seiscomp::Core::ValueException);
		const SurfaceRupture& surfaceRupture() const throw(Seiscomp::Core::ValueException);

		void setCentroidReference(const std::string& centroidReference);
		const std::string& centroidReference() const;

	
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

		//! Implement PublicObject interface
		bool updateChild(Object* child);

		void accept(Visitor*);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		OPT(RealQuantity) _width;
		OPT(RealQuantity) _displacement;
		OPT(RealQuantity) _riseTime;
		OPT(RealQuantity) _vtToVs;
		OPT(RealQuantity) _shallowAsperityDepth;
		OPT(bool) _shallowAsperity;
		OPT(LiteratureSource) _literatureSource;
		OPT(RealQuantity) _slipVelocity;
		OPT(RealQuantity) _strike;
		OPT(RealQuantity) _length;
		OPT(RealQuantity) _area;
		OPT(RealQuantity) _ruptureVelocity;
		OPT(RealQuantity) _stressdrop;
		OPT(RealQuantity) _momentReleaseTop5km;
		OPT(FwHwIndicator) _fwHwIndicator;
		std::string _ruptureGeometryWKT;
		std::string _faultID;
		OPT(SurfaceRupture) _surfaceRupture;
		std::string _centroidReference;

	DECLARE_SC_CLASSFACTORY_FRIEND(Rupture);
};


}
}
}


#endif
