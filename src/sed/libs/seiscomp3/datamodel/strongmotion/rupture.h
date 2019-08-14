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
		RealQuantity& width();
		const RealQuantity& width() const;

		void setDisplacement(const OPT(RealQuantity)& displacement);
		RealQuantity& displacement();
		const RealQuantity& displacement() const;

		void setRiseTime(const OPT(RealQuantity)& riseTime);
		RealQuantity& riseTime();
		const RealQuantity& riseTime() const;

		void setVtToVs(const OPT(RealQuantity)& vtToVs);
		RealQuantity& vtToVs();
		const RealQuantity& vtToVs() const;

		void setShallowAsperityDepth(const OPT(RealQuantity)& shallowAsperityDepth);
		RealQuantity& shallowAsperityDepth();
		const RealQuantity& shallowAsperityDepth() const;

		void setShallowAsperity(const OPT(bool)& shallowAsperity);
		bool shallowAsperity() const;

		void setLiteratureSource(const OPT(LiteratureSource)& literatureSource);
		LiteratureSource& literatureSource();
		const LiteratureSource& literatureSource() const;

		void setSlipVelocity(const OPT(RealQuantity)& slipVelocity);
		RealQuantity& slipVelocity();
		const RealQuantity& slipVelocity() const;

		void setStrike(const OPT(RealQuantity)& strike);
		RealQuantity& strike();
		const RealQuantity& strike() const;

		void setLength(const OPT(RealQuantity)& length);
		RealQuantity& length();
		const RealQuantity& length() const;

		void setArea(const OPT(RealQuantity)& area);
		RealQuantity& area();
		const RealQuantity& area() const;

		void setRuptureVelocity(const OPT(RealQuantity)& ruptureVelocity);
		RealQuantity& ruptureVelocity();
		const RealQuantity& ruptureVelocity() const;

		void setStressdrop(const OPT(RealQuantity)& stressdrop);
		RealQuantity& stressdrop();
		const RealQuantity& stressdrop() const;

		void setMomentReleaseTop5km(const OPT(RealQuantity)& momentReleaseTop5km);
		RealQuantity& momentReleaseTop5km();
		const RealQuantity& momentReleaseTop5km() const;

		void setFwHwIndicator(const OPT(FwHwIndicator)& fwHwIndicator);
		FwHwIndicator fwHwIndicator() const;

		void setRuptureGeometryWKT(const std::string& ruptureGeometryWKT);
		const std::string& ruptureGeometryWKT() const;

		void setFaultID(const std::string& faultID);
		const std::string& faultID() const;

		void setSurfaceRupture(const OPT(SurfaceRupture)& surfaceRupture);
		SurfaceRupture& surfaceRupture();
		const SurfaceRupture& surfaceRupture() const;

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
