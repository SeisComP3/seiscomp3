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

#ifndef PROFILEEMPTY_OBJECT_H
#define PROFILEEMPTY_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>
#include <fstream>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessProfile.h"
#include "GeoTessProfileType.h"
#include "IFStreamAscii.h"
#include "IFStreamBinary.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

class GeoTessData;

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief A Profile object that defines two radii at the bottom and top of the
 * associated layer, and no Data.
 *
 * A Profile object that defines two radii, one at the bottom and other at the
 * top of the associated layer, and no Data.  Profiles of this type should
 * never be connected together by the connectivity of a tessellation.
 *
 */
class GEOTESS_EXP_IMP GeoTessProfileEmpty : virtual public GeoTessProfile
{
  private:

		/**
		 * Top radius.
		 */
		float									radiusTop;

		/**
		 * Bottom radius.
		 */
		float         				radiusBottom;

		/**
		 * Default constructor.
		 */
													GeoTessProfileEmpty() : GeoTessProfile(), radiusTop(0.0), radiusBottom(0.0) {};

	public:

		/**
		 * Default constructor.
		 */
													GeoTessProfileEmpty(float radBot, float radTop) : GeoTessProfile(),
																			radiusTop(radTop), radiusBottom(radBot)
													{}

		/**
		 * Standard constructor. Reads radii from the provided input file stream.
		 */
													GeoTessProfileEmpty(IFStreamBinary& ifs) : GeoTessProfile()
													{ radiusBottom = ifs.readFloat();
														radiusTop = ifs.readFloat();
													}

		/**
		 * Standard constructor. Reads radii from the provided input file stream.
		 */
													GeoTessProfileEmpty(IFStreamAscii& ifs) : GeoTessProfile()
													{ radiusBottom = ifs.readFloat();
														radiusTop = ifs.readFloat();
													}

		/**
		 * Standard constructor creates references into the supplied array of radii.
		 */
													GeoTessProfileEmpty(float radii[], int &rIndex) : GeoTessProfile()
													{ radiusBottom = radii[rIndex++];
														radiusTop = radii[rIndex++];
													}

		/**
		 * Default destructor.
		 */
		virtual								~GeoTessProfileEmpty() {};

		/**
		 * Returns the class name.
		 */
		static  string				class_name() { return "ProfileEmpty"; };

		/**
		 * Returns the class size.
		 */
		virtual int						class_size() const
													{ return (int) sizeof(GeoTessProfileEmpty); };

		virtual LONG_INT getMemory() { return (LONG_INT)(sizeof(GeoTessProfileEmpty)); };

		/**
		 * Returns ProfileType (EMPTY).
		 *
		 * @return ProfileType (EMPTY).
		 */
		virtual	const GeoTessProfileType&		 getType() const
													{ return GeoTessProfileType::EMPTY; };

		/**
		 * Return true if the input Profile object (p) equals this Profile object.
		 */
		virtual bool					operator == (const GeoTessProfile& p) const
													{	return (GeoTessProfile::operator==(p) &&
																		(radiusBottom == p.getRadiusBottom()) &&
																		(radiusTop == p.getRadiusTop()));
													}

		/**
		 * Retrieve the value of the specified attribute interpolated at the
		 * specified radius. Unsuppported for ProfileEmpty.
		 */
		virtual double getValue(const GeoTessInterpolatorType& rInterpType,
			int attributeIndex, double radius,
			bool allowRadiusOutOfRange) const;

		/**
		 * Retrieve the value of the specified attribute interpolated from this
		 * profile at the specified radius index
		 *
		 * @param attributeIndex
		 * @param nodeIndex
		 * @return double
		 */
		virtual double getValue(int attributeIndex, int nodeIndex) const
		{ return NaN_DOUBLE; }

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
			return true;
		}

		// *** TODO added 7/20/2012
		/**
		 * Retrieve the value of the specified attribute at the top of the layer.
		 *
		 * @param attributeIndex
		 * @return double
		 */
		virtual double		 	  getValueTop(int attributeIndex) const
		{
			return NaN_DOUBLE;
		}

		// *** TODO added 7/20/2012
		/**
		 * Retrieve the value of the specified attribute at the bottom of the layer.
		 *
		 * @param attributeIndex
		 * @return double
		 */
		virtual double		 	  getValueBottom(int attributeIndex) const
		{
			return NaN_DOUBLE;
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
		virtual int						getNData() const { return 0; };

		/**
		 * Retrieve a deeep copy of the radii values in km.
		 */
		virtual float*				getRadii()
		 { float* fa = new float [2]; fa[0] = radiusBottom; fa[1] = radiusTop; return fa; };

		/**
		 * Retrieve a reference to all of the Data obects associated with this
		 * Profile.
		 */
		virtual GeoTessData**				getData();

		/**
		 * Retrieve a reference the i'th Data object
		 */
		virtual GeoTessData*					getData(int i);

		/**
		 * Retrieve a reference the i'th Data object
		 */
		virtual const GeoTessData&		getData(int i) const;

		/**
		 * Resets the data object to the new input data
		 */
		virtual void					setData(const vector<GeoTessData*>& inData)
													{/* do nothing*/ };

		// TODO: added by sb 9/27/2012
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
		// *** TODO added 7/19/2012
		/**
		 * Resets the data object at index to the new input data.
		 */
		virtual void          setData(int index, GeoTessData* inData)
													{ /* do nothing*/ };

		/**
		 * Get the radius at the top of the profile, in km.
		 */
		virtual float					getRadiusTop() const { return radiusTop; };

		/**
		 * Get the Data object at the top of the profile.
		 */
		virtual const GeoTessData& 	getDataTop() const;

		/**
		 * Get the Data object at the top of the profile.
		 */
		virtual GeoTessData*				 	getDataTop() { return NULL; }

		/**
		 * Get the radius at the bottom of the profile, in km.
		 */
		virtual float					getRadiusBottom() const { return radiusBottom; };

		/**
		 * Get the Data object at the bottom of the profile.
		 */
		virtual const GeoTessData&		getDataBottom() const;

		/**
		 * Get the Data object at the bottom of the profile.
		 */
		virtual GeoTessData*				 	getDataBottom() { return NULL; }

		/**
		 * Write the radii and data values to binary file.
		 */
		virtual void					write(IFStreamBinary& ofs)
													{ ofs.writeByte((byte) GeoTessProfileType::EMPTY.ordinal());
														ofs.writeFloat(radiusBottom);
														ofs.writeFloat(radiusTop); };

		/**
		 * Write the radii and data values to binary file.
		 */
		virtual void					write(IFStreamAscii& ofs)
													{ ofs.writeInt(GeoTessProfileType::EMPTY.ordinal());
														ofs.writeString(" ");
														ofs.writeFloat(radiusBottom);
														ofs.writeString(" ");
														ofs.writeFloatNL(radiusTop); };

		// *** TODO added 7/20/2012
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

		// *** TODO added 7/20/2012
		/**
		 * Set the pointIndex that corresponds to the supplied nodeIndex.
		 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
		 * the number of Data objects managed by a Profile.  There is a pointIndex for every
		 * Data object in the entire model, indexed from 0 to the number of Data objects in the
		 * model.
		 */
		virtual void					setPointIndex(int nodeIndex, int pointIndex)
		{
			// do nothing.  empty profiles have no data, hence pointIndex is always -1.
		}

		// *** TODO added 10/14/2012
		/**
		 * Reset all the pointIndex values to -1.
		 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
		 * the number of Data objects managed by a Profile.  There is a pointIndex for every
		 * Data object in the entire model, indexed from 0 to the number of Data objects in the
		 * model.
		 */
		virtual void resetPointIndices() { /* empty profiles have no data, hence pointIndex is always -1. */ }

		// *** TODO added 7/20/2012
		/**
		 * Get the pointIndex that corresponds to the supplied nodeIndex.
		 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
		 * the number of Data objects managed by a Profile.  There is a pointIndex for every
		 * Data object in the entire model, indexed from 0 to the number of Data objects in the
		 * model.
		 */
		virtual int						getPointIndex(int nodeIndex) const
													{ return -1; }

		// *** TODO added 7/20/2012
		/**
		 * Add dkm * hcoefficient to the weight of this point index into the input map.
		 * This works for Profile types Constant, Thin and Surface since they only have
		 * a single node in the profile. It does not work for ProfileNPoint and
		 * ProfileEmpty so they override this method.
		 */
	  virtual void					getWeights(map<int, double>& weights,
	  		                             double dkm, double radius, double hcoefficient) const
		{
			// do nothing.
		}

		// *** TODO added 7/26/2012
		/**
		 *
		 */
		virtual void					getCoefficients(map<int, double>& coefficients, double radius,
				                                  double horizontalCoefficient) const
		{
			// do nothing.
		}

		virtual void setInterpolationCoefficients(const GeoTessInterpolatorType& interpType,
				vector<int>& nodeIndexes, vector<double>& coefficients,
				double& radius, bool& allowOutOfRange)
		{
			// POISON!
			nodeIndexes.push_back(0);
			coefficients.push_back(NaN_DOUBLE);
		}

		// *** TODO added 8/20/2012
		/**
		 * Returns a deep copy of this profile.
		 */
		virtual GeoTessProfile*      copy()
		{
			return new GeoTessProfileEmpty(radiusBottom, radiusTop);
		}

		/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

		///@endcond

}; // end class ProfileEmpty

} // end namespace geotess

#endif  // PROFILEEMPTY_OBJECT_H
