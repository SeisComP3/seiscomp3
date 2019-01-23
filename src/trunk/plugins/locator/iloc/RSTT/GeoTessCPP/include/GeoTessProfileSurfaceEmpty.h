//- ****************************************************************************
//-
//- Copyright 2009 Sandia Corporation. Under the terms of Contract
//- DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government
//- retains certain rights in this software.
//-
//- BSD Open Source License.
//- All rights reserved.
//-
//- Redistribution and use in source and binary forms, with or without
//- modification, are permitted provided that the following conditions are met:
//-
//-    * Redistributions of source code must retain the above copyright notice,
//-      this list of conditions and the following disclaimer.
//-    * Redistributions in binary form must reproduce the above copyright
//-      notice, this list of conditions and the following disclaimer in the
//-      documentation and/or other materials provided with the distribution.
//-    * Neither the name of Sandia National Laboratories nor the names of its
//-      contributors may be used to endorse or promote products derived from
//-      this software without specific prior written permission.
//-
//- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//- AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//- IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//- ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//- LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//- CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//- SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//- INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//- CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//- ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//- POSSIBILITY OF SUCH DAMAGE.
//-
//- ****************************************************************************

#ifndef PROFILESURFACE_EMPTY_OBJECT_H
#define PROFILESURFACE_EMPTY_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>
#include <fstream>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessUtils.h"
#include "GeoTessData.h"
#include "GeoTessProfile.h"
#include "GeoTessProfileType.h"
#include "IFStreamAscii.h"
#include "IFStreamBinary.h"
#include "GeoTessDataValue.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

class GeoTessMetaData;

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief A Profile object that defines a single Data object and no radius value.
 *
 * A Profile object used to support 2D models.  It represents a single Data object
 * and no radius value.  ProfileSurfaceEmpty objects are incompatible with all other
 * Profile types in the sense that a model that contains any ProfileSurfaceEmpty objects
 * must be made up entirely of ProfileSurfaceEmpty objects.
 *
 */
class GEOTESS_EXP_IMP GeoTessProfileSurfaceEmpty: virtual public GeoTessProfile {
public:

	GeoTessProfileSurfaceEmpty() : GeoTessProfile() { }

	/**
	 * Returns the class name.
	 */
	static string class_name() { return "ProfileSurfaceEmpty"; }

	/**
	 * Returns the class size.
	 */
	virtual int class_size() const { return (int) sizeof(GeoTessProfileSurfaceEmpty); }

	virtual LONG_INT getMemory() { return (LONG_INT)sizeof(GeoTessProfileSurfaceEmpty); }

	/**
	 * Returns ProfileType (SURFACE).
	 *
	 * @return ProfileType (SURFACE).
	 */
	virtual const GeoTessProfileType& getType() const { return GeoTessProfileType::SURFACE_EMPTY; }

	/**
	 * Return true if the input Profile object (p) equals this Profile object.
	 */
	virtual bool operator ==(const GeoTessProfile& p) const { return p.getType() == GeoTessProfileType::SURFACE_EMPTY; }

	/**
	 * Retrieve the value of the specified attribute from this
	 * profile at the specified node index
	 *
	 * @param attributeIndex
	 * @param nodeIndex
	 * @return double
	 */
	virtual double getValue(int attributeIndex, int nodeIndex) const { return NaN_DOUBLE; }

	/**
	 * Retrieve the value of the specified attribute at the top of the layer.
	 *
	 * @param attributeIndex
	 * @return double
	 */
	virtual double getValueTop(int attributeIndex) const { return NaN_DOUBLE; }

	/**
	 * Return true if the specified Data value is NaN.
	 * For doubles and floats, this means not NaN.
	 * For bytes, shorts, ints and longs, always returns false
	 * since there is no value that is NaN.
	 *
	 * @param nodeIndex
	 * @param attributeIndex
	 * @return true if the specified Data value is NaN.
	 */
	virtual bool isNaN(int nodeIndex, int attributeIndex) { return true; }

	/**
	 * Retrieve the value of the specified attribute interpolated at the
	 * specified radius. Ignore allowRadiusOutOfRange.
	 */
	virtual double getValue(const GeoTessInterpolatorType& rInterpType,
			int attributeIndex, double radius,
			bool allowRadiusOutOfRange) const
	{ return NaN_DOUBLE; }

	/**
	 * Resets the data object to the new input data
	 */
	virtual void setData(const vector<GeoTessData*>& inData) { }

	/**
	 * Resets the data object at index to the new input data.
	 */
	virtual void setData(int index, GeoTessData* inData) { }

	/**
	 * Retrieve a reference to all of the Data obects associated with this
	 * Profile.
	 */
	virtual GeoTessData** getData()
	{
		ostringstream os;
		os << endl << "ERROR in ProfileSurfaceEmpty::getData" << endl
			 << "Unsupported method call." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 4201);
		return (GeoTessData**) NULL;
	}

	/**
	 * Retrieve a reference the i'th Data object
	 */
	virtual GeoTessData* getData(int i)
	{
		ostringstream os;
		os << endl << "ERROR in ProfileSurfaceEmpty::getData" << endl
			 << "Unsupported method call." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 4201);
		return (GeoTessData*) NULL;
	}

	/**
	 * Retrieve a reference the i'th Data object
	 */
	virtual const GeoTessData& getData(int i) const
	{
		ostringstream os;
		os << endl << "ERROR in ProfileSurfaceEmpty::getData" << endl
			 << "Unsupported method call." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 4201);
		return *(new GeoTessDataValue<int>());
	}
	/**
	 * Get the Data object at the top of the profile.
	 */
	virtual GeoTessData* getDataTop()
	{
		ostringstream os;
		os << endl << "ERROR in ProfileSurfaceEmpty::getDataBottom" << endl
			 << "Unsupported method call." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 4201);
		return (GeoTessData*) NULL;
	}

	/**
	 * Get the Data object at the top of the profile.
	 */
	virtual const GeoTessData& getDataTop() const
	{
		ostringstream os;
		os << endl << "ERROR in ProfileSurfaceEmpty::getDataTop" << endl
			 << "Unsupported method call." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 4201);
		return *(new GeoTessDataValue<int>());
	}

	/**
	 * Get the Data object at the bottom of the profile.
	 */
	virtual GeoTessData* getDataBottom()
	{
		ostringstream os;
		os << endl << "ERROR in ProfileSurfaceEmpty::getDataBottom" << endl
			 << "Unsupported method call." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 4201);
		return (GeoTessData*) NULL;
	}

	/**
	 * Get the Data object at the bottom of the profile.
	 */
	virtual const GeoTessData& getDataBottom() const
	{
		ostringstream os;
		os << endl << "ERROR in ProfileSurfaceEmpty::getDataBottom" << endl
			 << "Unsupported method call." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 4201);
		return *(new GeoTessDataValue<int>());
	}

	/**
	 * Get the i'th radius value in this profile in km. Radii are in order of
	 * increasing radius.
	 */
	virtual float getRadius(int i) const { return NaN_FLOAT; }

	/**
	 * Replace the radii currently associated with this Profile with new values.
	 */
	virtual void setRadii(const vector<float>& newRadii) { }

	virtual void setRadius(int index, float radius)
	{  /* do nothing */  }

	/**
	 * Get the radius at the top of the profile, in km.
	 */
	virtual float getRadiusTop() const { return NaN_FLOAT; }

	/**
	 * Get the radius at the bottom of the profile, in km.
	 */
	virtual float getRadiusBottom() const { return NaN_FLOAT; }

	/**
	 * Get the number of radii that comprise this profile.
	 */
	virtual int getNRadii() const { return 0; }

	/**
	 * Get the number of Data objects that comprise this profile.
	 */
	virtual int getNData() const { return 0; }

	/**
	 * Returns NULL
	 */
	virtual float* getRadii() { return (float*) NULL; }

	/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

	/**
	 * Standard constructor that reads data from the provided input file stream.
	 */
	GeoTessProfileSurfaceEmpty(IFStreamBinary& ifs, GeoTessMetaData& gtmd) : GeoTessProfile() { }

	/**
	 * Standard constructor that reads data from the provided input file stream.
	 */
	GeoTessProfileSurfaceEmpty(IFStreamAscii& ifs, GeoTessMetaData& gtmd) : GeoTessProfile() { }

	/**
	 * Default destructor. Note that all Profile objects delete their data. Data was
	 * created by input functionality.
	 */
	virtual ~GeoTessProfileSurfaceEmpty() { }

	/**
	 * Write the radii and data values to binary file.
	 */
	virtual void write(IFStreamBinary& ofs)
	{ ofs.writeByte((byte) GeoTessProfileType::SURFACE_EMPTY.ordinal()); }

	/**
	 * Write the radii and data values to binary file.
	 */
	virtual void write(IFStreamAscii& ofs)
	{ ofs.writeInt(GeoTessProfileType::SURFACE_EMPTY.ordinal()); ofs.writeNL();}

	/**
	 * Find the index of the node in this Profile that has radius closest to the
	 * supplied radius.
	 *
	 * @param radius in km
	 * @return The index of the node in this Profile that has radius closest to the
	 *         supplied radius.
	 */
	virtual int findClosestRadiusIndex(double radius) const { return -1; }

	/**
	 * Set the pointIndex that corresponds to the supplied nodeIndex.
	 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
	 * the number of Data objects managed by a Profile.  There is a pointIndex for every
	 * Data object in the entire model, indexed from 0 to the number of Data objects in the
	 * model.
	 */
	virtual void setPointIndex(int nodeIndex, int pntIndex) { }

	/**
	 * Reset all the pointIndex values to -1.
	 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
	 * the number of Data objects managed by a Profile.  There is a pointIndex for every
	 * Data object in the entire model, indexed from 0 to the number of Data objects in the
	 * model.
	 */
	virtual void resetPointIndices() { }

	/**
	 * Get the pointIndex that corresponds to the supplied nodeIndex.
	 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
	 * the number of Data objects managed by a Profile.  There is a pointIndex for every
	 * Data object in the entire model, indexed from 0 to the number of Data objects in the
	 * model.
	 */
	virtual int getPointIndex(int nodeIndex) const { return -1; }

	/**
	 * Returns a deep copy of this profile.
	 */
	virtual GeoTessProfile* copy() { return new GeoTessProfileSurfaceEmpty(); }

	///@endcond

};
// end class ProfileSurfaceEmpty

}// end namespace geotess

#endif  // PROFILESURFACE_EMPTY_OBJECT_H
