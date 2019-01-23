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

#ifndef PROFILECONSTANT_OBJECT_H
#define PROFILECONSTANT_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>
#include <fstream>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessData.h"
#include "GeoTessProfile.h"
#include "GeoTessProfileType.h"
#include "IFStreamAscii.h"
#include "IFStreamBinary.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

class GeoTessMetaData;

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief A Profile object that defines two radii at the bottom and top of the
 * associated layer, and a single Data object that represents the model values
 * throughout the layer.
 *
 * A Profile object that defines two radii, one at the bottom and other at the
 * top of the associated layer, and a single Data object that represents the
 * model values throughout the layer.
 *
 */
class GEOTESS_EXP_IMP GeoTessProfileConstant : virtual public GeoTessProfile
{
  private:

		/**
		 * Bottom radius.
		 */
		float									radiusBottom;

		/**
		 * Top radius.
		 */
		float									radiusTop;

		/**
		 * Attached Data object.
		 */
		GeoTessData*     						data;

		/**
		 * The point index of this grid point
		 */
		int										pointIndex;

		/**
		 * Default constructor.
		 */
													GeoTessProfileConstant() : GeoTessProfile(), radiusBottom(0.0), radiusTop(0.0),
																						data(NULL), pointIndex(-1) {};

	public:

		/**
		 * Default constructor.
		 */
													GeoTessProfileConstant(float radBot, float radTop, GeoTessData* dat) :
																	GeoTessProfile(), radiusBottom(radBot), radiusTop(radTop),
																	data(dat), pointIndex(-1) {};

		/**
		 * Returns the class name.
		 */
		static  string				class_name() { return "ProfileConstant"; };

		/**
		 * Returns the class size.
		 */
		virtual int						class_size() const
													{ return (int) sizeof(GeoTessProfileConstant); };

		virtual LONG_INT getMemory() { return (LONG_INT)(sizeof(GeoTessProfileConstant) + data->getMemory()); };

		/**
		 * Returns ProfileType (CONSTANT).
		 *
		 * @return ProfileType (CONSTANT).
		 */
		virtual	const GeoTessProfileType&		getType() const
													{ return GeoTessProfileType::CONSTANT; };

		/**
		 * Return true if the input Profile object (p) equals this Profile object.
		 */
		virtual bool					operator == (const GeoTessProfile& p) const
													{	return (GeoTessProfile::operator==(p) &&
																		(radiusBottom == p.getRadiusBottom()) &&
																		(radiusTop == p.getRadiusTop()) &&
																		(*data == p.getData(0)));
													}

		/**
		 * Get the i'th radius value in this profile in km. Radii are in order of
		 * increasing radius.
		 */
		virtual float					getRadius(int i) const
													{ return i == 0 ? radiusBottom : radiusTop; };

		/**
		 * Get the number of radii that comprise this profile.
		 */
		virtual int						getNRadii() const { return 2; };

		/**
		 * Get the number of Data objects that comprise this profile.
		 */
		virtual int						getNData() const { return 1; };

		/**
		 * Retrieve a deep copy of the radii values in km.
		 */
		virtual float*				getRadii()
		{ float* fa = new float [2]; fa[0] = radiusBottom; fa[1] = radiusTop; return fa; };

		/**
		 * Retrieve a shallow copy of all of the Data objects associated with this Profile.
		 */
		virtual GeoTessData**				getData()
		{ GeoTessData** da = new GeoTessData* [1]; da[0] = data; return da; };

		/**
		 * Retrieve a reference the i'th Data object
		 */
		virtual GeoTessData*					getData(int i) { return data; };

		/**
		 * Retrieve a reference the i'th Data object
		 */
		virtual const GeoTessData&		getData(int i) const { return *data; };

		/**
		 * Resets the data object to the new input data
		 */
		virtual void					setData(const vector<GeoTessData*>& inData)
													{ delete data; data = inData[0]; }

		/**
		 * Resets the data object at index to the new input data.
		 */
		virtual void          setData(int index, GeoTessData* inData)
													{ delete data; data = inData; }

		/**
		 * Retrieve the value of the specified attribute interpolated from this
		 * profile at the specified radius index
		 *
		 * @param attributeIndex
		 * @param nodeIndex
		 * @return double
		 */
		virtual double getValue(int attributeIndex, int nodeIndex) const
		{ return nodeIndex <= 1 ? data->getDouble(attributeIndex) : NaN_DOUBLE; }

		/**
		 * Retrieve the value of the specified attribute interpolated at the
		 * specified radius.
		 * @param rInterpType either Int
		 * @param attributeIndex
		 * @param radius radius in km
		 * @param allowRadiusOutOfRange
		 * @return interpolated attribute value
		 */
		virtual double getValue(const GeoTessInterpolatorType& rInterpType,
				int attributeIndex, double radius, bool allowRadiusOutOfRange) const
		{
			if (!allowRadiusOutOfRange &&
					((radius < getRadiusBottom()) || (radius > getRadiusTop())))
				return NaN_DOUBLE;

			// default behavior is to simply return the data value for the first
			// data object. This works for all the Profile classes that only support
			// a single Data object like ProfileConstant, ProfileSurface, and
			// ProfileThin. Profile classes for which the number of supported
			// Data objects is not equal to 1 must override this method.

			return getValue(attributeIndex, 0);
		};

		/**
		 * Retrieve the value of the specified attribute at the top of the layer.
		 *
		 * @param attributeIndex
		 * @return double
		 */
		virtual double getValueTop(int attributeIndex) const
		{	return data->getDouble(attributeIndex); }

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
		virtual bool isNaN(int nodeIndex, int attributeIndex)
		{
			return nodeIndex != 0 || data->isNaN(attributeIndex);
		}

		/**
		 * Replace the radii currently associated with this Profile with new values.
		 */
		virtual void					setRadii(const vector<float>& newRadii)
		{ radiusBottom = newRadii[0]; radiusTop = newRadii[1]; }

		virtual void setRadius(int index, float radius)
		{
			switch (index)
			{
			case 0:
				radiusBottom = radius;
				break;
			case 1:
				radiusTop = radius;
				break;
			}
		}

		/**
		 * Get the radius at the top of the profile, in km.
		 */
		virtual float					getRadiusTop() const { return radiusTop; };

		/**
		 * Get the Data object at the top of the profile.
		 */
		virtual const GeoTessData&		getDataTop() const { return *data; };

		/**
		 * Get the Data object at the top of the profile.
		 */
		virtual GeoTessData*					getDataTop() { return data; };

		/**
		 * Get the radius at the bottom of the profile, in km.
		 */
		virtual float					getRadiusBottom() const { return radiusBottom; };

		/**
		 * Get the Data object at the bottom of the profile.
		 */
		virtual const GeoTessData&		getDataBottom() const { return *data; };

		/**
		 * Get the Data object at the bottom of the profile.
		 */
		virtual GeoTessData*					getDataBottom() { return data; };

		/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

		/**
		 * Standard constructor that reads radii and data values from the provided input file stream.
		 */
													GeoTessProfileConstant(IFStreamBinary& ifs, GeoTessMetaData& gtmd) : GeoTessProfile()
													{ radiusBottom = ifs.readFloat(); radiusTop = ifs.readFloat();
														data = GeoTessData::getData(ifs, gtmd); };

		/**
		 * Standard constructor that reads radii and data values from the provided input file stream.
		 */
													GeoTessProfileConstant(IFStreamAscii& ifs, GeoTessMetaData& gtmd) : GeoTessProfile()
													{ radiusBottom = ifs.readFloat();
														radiusTop = ifs.readFloat();
														data = GeoTessData::getData(ifs, gtmd); };

		/**
		 * Default destructor. Note that all Profile objects delete their data. Data was
		 * created by input functionality.
		 */
		virtual								~GeoTessProfileConstant() { if (data != NULL) delete data; };

		/**
		 * Write the radii and data values to binary file.
		 */
		virtual void					write(IFStreamBinary& ofs)
													{
														ofs.writeByte((byte) GeoTessProfileType::CONSTANT.ordinal());
														ofs.writeFloat(radiusBottom);
														ofs.writeFloat(radiusTop);
														data->write(ofs);
													};

		/**
		 * Write the radii and data values to binary file.
		 */
		virtual void					write(IFStreamAscii& ofs)
													{ ofs.writeInt(GeoTessProfileType::CONSTANT.ordinal());
														ofs.writeString(" ");
														ofs.writeFloat(radiusBottom);
														ofs.writeString(" ");
														ofs.writeFloat(radiusTop);
														data->write(ofs);
														ofs.writeNL();
													};

		/**
		 * Find the index of the node in this Profile that has radius closest to the
		 * supplied radius.
		 *
		 * @param radius in km
		 * @return The index of the node in this Profile that has radius closest to the
		 *         supplied radius.
		 */
		virtual int						findClosestRadiusIndex(double radius) const
		{	return abs(radiusTop - radius) < abs(radiusBottom - radius) ? 1 : 0; }

		/**
		 * Set the pointIndex that corresponds to the supplied nodeIndex.
		 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
		 * the number of Data objects managed by a Profile.  There is a pointIndex for every
		 * Data object in the entire model, indexed from 0 to the number of Data objects in the
		 * model.
		 */
		virtual void					setPointIndex(int nodeIndex, int pntIndex)
													{ pointIndex = pntIndex; }

		/**
		 * Reset all the pointIndex values to -1.
		 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
		 * the number of Data objects managed by a Profile.  There is a pointIndex for every
		 * Data object in the entire model, indexed from 0 to the number of Data objects in the
		 * model.
		 */
		virtual void resetPointIndices() { pointIndex = -1; }

		/**
		 * Get the pointIndex that corresponds to the supplied nodeIndex.
		 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
		 * the number of Data objects managed by a Profile.  There is a pointIndex for every
		 * Data object in the entire model, indexed from 0 to the number of Data objects in the
		 * model.
		 */
		virtual int						getPointIndex(int nodeIndex) const
													{	return pointIndex; }

		/**
		 * Returns a deep copy of this profile.
		 */
		virtual GeoTessProfile*      copy()
		{
			return new GeoTessProfileConstant(radiusBottom, radiusTop, data->copy());
		}

		///@endcond

}; // end class ProfileConstant

} // end namespace geotess

#endif  // PROFILECONSTANT_OBJECT_H
