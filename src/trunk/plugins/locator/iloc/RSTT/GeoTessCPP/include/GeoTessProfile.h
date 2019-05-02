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

#ifndef PROFILE_OBJECT_H
#define PROFILE_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <map>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessProfileType.h"
#include "GeoTessData.h"
#include "GeoTessDataArray.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

class GeoTessInterpolatorType;
class GeoTessMetaData;
class IFStreamAscii;
class IFStreamBinary;


// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Abstract class that manages the radii and data values that span a single
 * layer associated with a single vertex in the model.
 *
 * An abstract class that manages the radii and data values that span a single
 * layer associated with a single vertex in the model.  The following Profile types are supported:
 *
 * <ul>
 * <li> ProfileNPoint is comprised on 2 or more monotonically increasing radius
 * values and an equal number of Data objects which define the distribution of
 * model values in the profile.
 * <li> ProfileConstant is comprised of two radius values, one defining the
 * bottom of the layer and the other the top of the layer.  There is a single
 * Data object which defines the model values for the entire profile.
 * <li> ProfileThin represents a profile with zero thickness.  It is comprised
 * of a single radius value and a single Data object.
 * <li> ProfileEmpty defines a profile with no Data.  It is comprised of two
 * radius values, one for the bottom of the layer and the other for the top of the
 * layer.  Any request for model values will return NaN_DOUBLE.
 * <li> ProfileSurface profiles are comprised of only a single Data object and
 * no radius values. They are used to support 2D models where the radius of the
 * Data values is not used.  ProfileSurface objects and all the other Profile
 * types are incompatible in the sense that they may not coexist in the same
 * model (a model is either 2D or 3D, but never both).
 * </ul>
 */
class GEOTESS_EXP_IMP GeoTessProfile
{
private:

	/**
	 * The static class instance count.
	 */
	static int						aClassCount;

public:

	/**
	 * Static factory method that instantiates a new Profile object of the appropriate
	 * type.
	 * <br> If radii.size() == 2 and data.size() == 0, creates a ProfileEmpty object.
	 * <br> If radii.size() == 1 and data.size() == 1, creates a ProfileThin object.
	 * <br> If radii.size() == 2 and data.size() == 1, creates a ProfileConstant object.
	 * <br> If radii.size() >= 2 and data.size() == nRadii, creates a ProfileNPoint object.
	 * <br> If radii.size() == 0 and data.size() == 1, creates a ProfileSurface object.
	 * <p>
	 * This method copies the radius values and Data* pointers out of the provided vectors
	 * into its owne structures.  Profile assumes ownership of the Data* pointers and will
	 * delete them when no longer needed.
	 *
	 * @param radii an array of zero or more radius values, in km.  Values must be
	 * monotonically increasing.
	 * @param data an array of pointers to Data objects.
	 * @return a new Profile object
	 * @throws GeoTessException
	 */
	static GeoTessProfile* newProfile(const vector<float>& radii, vector<GeoTessData*>& data);

	static GeoTessProfile* newProfile(const vector<float>& radii, vector<vector<double> >& data);
	static GeoTessProfile* newProfile(const vector<float>& radii, vector<vector<float> >& data);
	static GeoTessProfile* newProfile(const vector<float>& radii, vector<vector<LONG_INT> >& data);
	static GeoTessProfile* newProfile(const vector<float>& radii, vector<vector<int> >& data);
	static GeoTessProfile* newProfile(const vector<float>& radii, vector<vector<short> >& data);
	static GeoTessProfile* newProfile(const vector<float>& radii, vector<vector<byte> >& data);

	/**
	 * Static factory method that instantiates a new Profile object of the appropriate
	 * type.
	 * <br> If nRadii == 2 and nData == 0, creates a ProfileEmpty object.
	 * <br> If nRadii == 1 and nData == 1, creates a ProfileThin object.
	 * <br> If nRadii == 2 and nData == 1, creates a ProfileConstant object.
	 * <br> If nRadii >= 2 and nData == nRadii, creates a ProfileNPoint object.
	 * <br> If nRadii == 0 and nData == 1, creates a ProfileSurface object.
	 * <p>
	 * This method copies the radius values and Data* pointers out of the provided arrays
	 * into its own structures.  Profile assumes ownership of the Data* pointers and will
	 * delete them when no longer needed.  The callers retains ownership of the
	 * containing arrays however and should delete them when no longer needed.
	 *
	 * @param radii an array of zero or more radius values, in km.  Values must be
	 * monotonically increasing.
	 * @param nRadii the length of the radii array.
	 * @param data an array of pointers to Data objects.
	 * @param nData the length of the data array
	 * @return a new Profile object
	 * @throws GeoTessException
	 */
	static GeoTessProfile* newProfile(float* radii, const int& nRadii, GeoTessData** data, const int& nData);

	static GeoTessProfile* newProfile(float* radii, const int& nRadii, double** values, const int& nNodes, const int& nAttributes)
	{
		if (nNodes < 1 && nRadii == 2)
			return GeoTessProfile::newProfile(radii, nRadii, NULL, 0);

		if (nNodes == 1)
		{
			GeoTessData* d[1];
			d[0] = GeoTessData::getData(values[0], nAttributes);
			return GeoTessProfile::newProfile(radii, nRadii, d, nNodes);
		}

		GeoTessData** d = new GeoTessData*[nNodes];
		for (int i=0; i<nNodes; ++i) d[i] = GeoTessData::getData(values[i], nAttributes);
		GeoTessProfile* profile = GeoTessProfile::newProfile(radii, nRadii, d, nNodes);
		delete[] d;
		return profile;
	}

	static GeoTessProfile* newProfile(float* radii, const int& nRadii, float** values, const int& nNodes, const int& nAttributes)
	{
		if (nNodes < 1 && nRadii == 2)
			return GeoTessProfile::newProfile(radii, nRadii, NULL, 0);

		if (nNodes == 1)
		{
			GeoTessData* d[1];
			d[0] = GeoTessData::getData(values[0], nAttributes);
			return GeoTessProfile::newProfile(radii, nRadii, d, nNodes);
		}

		GeoTessData** d = new GeoTessData*[nNodes];
		for (int i=0; i<nNodes; ++i) d[i] = GeoTessData::getData(values[i], nAttributes);
		GeoTessProfile* profile = GeoTessProfile::newProfile(radii, nRadii, d, nNodes);
		delete[] d;
		return profile;
	}

	static GeoTessProfile* newProfile(float* radii, const int& nRadii, LONG_INT** values, const int& nNodes, const int& nAttributes)
	{
		if (nNodes < 1 && nRadii == 2)
			return GeoTessProfile::newProfile(radii, nRadii, NULL, 0);

		if (nNodes == 1)
		{
			GeoTessData* d[1];
			d[0] = GeoTessData::getData(values[0], nAttributes);
			return GeoTessProfile::newProfile(radii, nRadii, d, nNodes);
		}

		GeoTessData** d = new GeoTessData*[nNodes];
		for (int i=0; i<nNodes; ++i) d[i] = GeoTessData::getData(values[i], nAttributes);
		GeoTessProfile* profile = GeoTessProfile::newProfile(radii, nRadii, d, nNodes);
		delete[] d;
		return profile;
	}

	static GeoTessProfile* newProfile(float* radii, const int& nRadii, int** values, const int& nNodes, const int& nAttributes)
	{
		if (nNodes < 1 && nRadii == 2)
			return GeoTessProfile::newProfile(radii, nRadii, NULL, 0);

		if (nNodes == 1)
		{
			GeoTessData* d[1];
			d[0] = GeoTessData::getData(values[0], nAttributes);
			return GeoTessProfile::newProfile(radii, nRadii, d, nNodes);
		}

		GeoTessData** d = new GeoTessData*[nNodes];
		for (int i=0; i<nNodes; ++i) d[i] = GeoTessData::getData(values[i], nAttributes);
		GeoTessProfile* profile = GeoTessProfile::newProfile(radii, nRadii, d, nNodes);
		delete[] d;
		return profile;
	}

	static GeoTessProfile* newProfile(float* radii, const int& nRadii, short** values, const int& nNodes, const int& nAttributes)
	{
		if (nNodes < 1 && nRadii == 2)
			return GeoTessProfile::newProfile(radii, nRadii, NULL, 0);

		if (nNodes == 1)
		{
			GeoTessData* d[1];
			d[0] = GeoTessData::getData(values[0], nAttributes);
			return GeoTessProfile::newProfile(radii, nRadii, d, nNodes);
		}

		GeoTessData** d = new GeoTessData*[nNodes];
		for (int i=0; i<nNodes; ++i) d[i] = GeoTessData::getData(values[i], nAttributes);
		GeoTessProfile* profile = GeoTessProfile::newProfile(radii, nRadii, d, nNodes);
		delete[] d;
		return profile;
	}

	static GeoTessProfile* newProfile(float* radii, const int& nRadii, byte** values, const int& nNodes, const int& nAttributes)
	{
		if (nNodes < 1 && nRadii == 2)
			return GeoTessProfile::newProfile(radii, nRadii, NULL, 0);

		if (nNodes == 1)
		{
			GeoTessData* d[1];
			d[0] = GeoTessData::getData(values[0], nAttributes);
			return GeoTessProfile::newProfile(radii, nRadii, d, nNodes);
		}

		GeoTessData** d = new GeoTessData*[nNodes];
		for (int i=0; i<nNodes; ++i) d[i] = GeoTessData::getData(values[i], nAttributes);
		GeoTessProfile* profile = GeoTessProfile::newProfile(radii, nRadii, d, nNodes);
		delete[] d;
		return profile;
	}

	/**
	 * Returns the class name.
	 */
	static  string				class_name() { return "Profile"; };

	/**
	 * Returns the class size.
	 */
	virtual int					class_size() const { return (int) sizeof(GeoTessProfile); };

	virtual LONG_INT getMemory() = ABSTRACT;

	/**
	 * Returns the class instance count.
	 */
	static  int					class_count() { return aClassCount; };

	/**
	 * One of EMPTY, THIN, CONSTANT, NPOINT, SURFACE
	 *
	 * @return ProfileType
	 */
	virtual	const GeoTessProfileType&	getType() const = ABSTRACT;

	/**
	 * Return true if the input Profile object (p) profile type is the same as this
	 * Profile object.
	 */
	virtual bool operator == (const GeoTessProfile& p) const { return (getType() == p.getType()); }

	/**
	 * Retrieve the value of the specified attribute interpolated at the
	 * specified radius.
	 */
	virtual double				getValue(const GeoTessInterpolatorType& rInterpType,
			int attributeIndex, double radius, bool allowRadiusOutOfRange) const   = ABSTRACT;

	/**
	 * Retrieve the value of the specified attribute interpolated at the
	 * specified radius.
	 */
	double	getValue(const vector<int>& nodeIds,
			const vector<double>& coefficients, int attributeIndex) const
	{
		double value = 0;
		for (int i=0; i<(int)nodeIds.size(); ++i)
			value += getValue(attributeIndex, nodeIds[i])*coefficients[i];
		return value;
	}

	/**
	 * Retrieve the value of the specified attribute interpolated from this
	 * profile at the specified radius index
	 *
	 * @param attributeIndex
	 * @param nodeIndex
	 * @return double
	 */
	virtual double getValue(int attributeIndex, int nodeIndex) const  = ABSTRACT;

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
	virtual bool isNaN(int nodeIndex, int attributeIndex) = ABSTRACT;

	/**
	 * Retrieve the value of the specified attribute at the top of the layer.
	 *
	 * @param attributeIndex
	 * @return double
	 */
	virtual double		 	  getValueTop(int attributeIndex) const = ABSTRACT;

	/**
	 * Retrieve the value of the specified attribute at the bottom of the layer.
	 *
	 * @param attributeIndex
	 * @return double
	 */
	virtual double		 	  getValueBottom(int attributeIndex) const
	{
		return getValue(attributeIndex, 0);
	}

	/**
	 * Get the i'th radius value in this profile in km. Radii are in order of
	 * increasing radius.
	 */
	virtual float				getRadius(int i) const = ABSTRACT;

	/**
	 * Get the number of radii that comprise this profile.
	 */
	virtual int					getNRadii() const = ABSTRACT;

	/**
	 * Get the number of Data objects that comprise this profile.
	 */
	virtual int					getNData() const = ABSTRACT;

	/**
	 * Retrieve a copy of the array of radii values in km.
	 * Caller assumes ownership of the array and must delete it when done with it.
	 */
	virtual float*				getRadii() = ABSTRACT;

	/**
	 * Retrieve a shallow copy of the array of Data objects associated with this Profile.
	 * Caller assumes ownership of the array, but not the contents, and must delete the
	 * array when done with it.
	 */
	virtual GeoTessData**				getData() = ABSTRACT;

	/**
	 * Retrieve a reference the i'th Data object. This Profile retains
	 * ownership of the returned Data object; caller should not delete it.
	 */
	virtual GeoTessData*				getData(int i) = ABSTRACT;

	/**
	 * Retrieve a reference the i'th Data object
	 */
	virtual const GeoTessData&			getData(int i) const = ABSTRACT;

	/**
	 * Replace the Data currently associated with this Profile with new Data.
	 * The current Data objects are deleted.
	 * <p>This Profile assumes ownership of the input Data objects and will
	 * delete them when appropriate.
	 */
	virtual void				setData(int index, GeoTessData* data) = ABSTRACT;

	/**
	 * Replace the Data currently associated with this Profile with new Data.
	 * The current Data objects are deleted.
	 * <p>This Profile assumes ownership of the input Data objects and will
	 * delete them when appropriate.
	 */
	virtual void				setData(const vector<GeoTessData*>& inData) = ABSTRACT;

	/**
	 * Replace the radii currently associated with this Profile with new values.
	 * Radius values are copied from the supplied vector.  This Profile does NOT
	 * keep a reference to the input vector.
	 */
	virtual void				setRadii(const vector<float>& newRadii) = ABSTRACT;

	/**
	 * Replace the radius at the specified nodeIndex.
	 */
	virtual void				setRadius(int index, float radius) = ABSTRACT;

	/**
	 * Get the radius at the top of the profile, in km.
	 */
	virtual float				getRadiusTop() const = ABSTRACT;

	/**
	 * Get the Data object at the top of the profile.
	 */
	virtual const GeoTessData&			getDataTop() const = ABSTRACT;

	/**
	 * Get the Data object at the top of the profile.
	 */
	virtual GeoTessData*				getDataTop() = ABSTRACT;

	/**
	 * Get the radius at the bottom of the profile, in km.
	 */
	virtual float				getRadiusBottom() const = ABSTRACT;

	/**
	 * Get the Data object at the bottom of the profile.
	 */
	virtual const GeoTessData&			getDataBottom() const = ABSTRACT;

	/**
	 * Get the Data object at the bottom of the profile.
	 */
	virtual GeoTessData*				getDataBottom() = ABSTRACT;

	/**
	 * Return the thickness of the layer in km.
	 */
	double						getThickness() { return getRadiusTop() - getRadiusBottom(); }

	/**
	 * Find index i such that radius is >= radii[i] and < radii[i+1]. If radius
	 * < radii[1] returns 0. If radius >= radii[xx.length-2] return
	 * radii.length-2.
	 */
	int							getRadiusIndex(double radius) const
	{ return getRadiusIndex(radius, -1); }

	/**
	 * Find index i such that radius is >= radii[i] and < radii[i+1]. If radius
	 * < radii[1] returns 0. If radius >= radii[xx.length-2] return
	 * radii.length-2.
	 *
	 * @param radius
	 * @param jlo
	 *            index found by a previous search. Used as an initial guess to
	 *            improve performance of the search. Specify -1 if none
	 *            available.
	 */
	virtual int					getRadiusIndex(double radius, int jlo) const
	{
		// note that ProfileNPoint will override this method.
		return 0;
	}

	/**
	 * find interpolation coefficient.
	 *
	 * @param i the index in radii such that radius is between radii[index]
	 *            and radii[index+1].
	 * @param radius the radius whose interpolation coefficient is desired.
	 * @param allowOutOfRange if false and the supplied radius is out of range
	 * then this method returns NaN_DOUBLE.
	 *
	 * @return c[index], the interpolation coefficient to be applied at radii[index].
	 * The interpolation coefficient for radii[index+1] is 1-c[index].
	 */
	virtual double getInterpolationCoefficient(int i, double radius,
			bool allowOutOfRange) const
	{
		if (!allowOutOfRange && (radius < getRadiusBottom() || radius > getRadiusTop()))
			return NaN_DOUBLE;

		// note that ProfileNPoint will override this method.
		return 1.;
	}

	/**
	 * Get the pointIndex that corresponds to the supplied nodeIndex.
	 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
	 * the number of Data objects managed by a Profile.  There is a pointIndex for every
	 * Data object in the entire model, indexed from 0 to the number of Data objects in the
	 * model.
	 */
	virtual int					getPointIndex(int nodeIndex) const = ABSTRACT;

	/**
	 * Find the node index of the radius in this Profile that has radius closest
	 * to the supplied radius.
	 */
	virtual int					findClosestRadiusIndex(double radius) const = ABSTRACT;

	/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

	/**
	 * Add dkm * hcoefficient to the weight of this point index into the input map.
	 * This works for Profile types Constant, Thin and Surface since they only have
	 * a single node in the profile. It does not work for ProfileNPoint and
	 * ProfileEmpty so they override this method.
	 */
	virtual void getWeights(map<int, double>& weights,
			double dkm, double radius, double hcoefficient) const
	{
		// get the point index of the one-and-only node.

		int index = getPointIndex(0);

		// find the current weight of the point, if it exists.
		// either set the weight of pointIndex (if it does not already exist),
		// or add the new weight to the existing weight.

		map<int, double>::iterator it = weights.find(index);
		if (it == weights.end())
			weights[index] = dkm * hcoefficient;
		else
			it->second += dkm * hcoefficient;
	}

	/**
	 * Add entries to supplied map from pointIndex to interpolation coefficient.
	 * @param coefficients map from pointIndex to interpolation coefficient
	 * @param radius
	 * @param horizontalCoefficient
	 */
	virtual void				getCoefficients(map<int, double>& coefficients, double radius,
			double horizontalCoefficient) const
	{
		// this works for Profile types Constant, Thin and Surface since they only have a single node
		// in the profile.  It does not work for ProfileNPoint and ProfileEmpty so they override this method.

		coefficients[getPointIndex(0)] = horizontalCoefficient;
	}

	/**
	 * Default constructor.
	 */
	GeoTessProfile() { ++aClassCount; };

	/**
	 * Default destructor.
	 */
	virtual						~GeoTessProfile() { --aClassCount; };

	/**
	 * Static factory method to load and return a new Proflie object from disk.
	 */
	static GeoTessProfile*				newProfile(IFStreamBinary& ifs, GeoTessMetaData& gtmd);

	/**
	 * Static factory method to load and return a new Proflie object from disk.
	 */
	static GeoTessProfile*				newProfile(IFStreamAscii& ifs, GeoTessMetaData& gtmd);

	/**
	 * Write the radii and data values to binary file.
	 */
	virtual void					write(IFStreamBinary& ofs) = ABSTRACT;

	/**
	 * Write the radii and data values to binary file.
	 */
	virtual void					write(IFStreamAscii& ofs) = ABSTRACT;

	/**
	 * Set the pointIndex that corresponds to the supplied nodeIndex.
	 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
	 * the number of Data objects managed by a Profile.  There is a pointIndex for every
	 * Data object in the entire model, indexed from 0 to the number of Data objects in the
	 * model.
	 */
	virtual void					setPointIndex(int nodeIndex, int pointIndex) = ABSTRACT;

	/**
	 * Reset all the pointIndex values to -1.
	 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
	 * the number of Data objects managed by a Profile.  There is a pointIndex for every
	 * Data object in the entire model, indexed from 0 to the number of Data objects in the
	 * model.
	 */
	virtual void resetPointIndices() = ABSTRACT;

	virtual void setInterpolationCoefficients(const GeoTessInterpolatorType& interpType,
			vector<int>& nodeIndexes, vector<double>& coefficients,
			double& radius, bool& allowOutOfRange)
	{
		// this code works for Profiles constant, thin and surface.
		// ProfileNPoint and ProfileEmpty will override it.
		nodeIndexes.push_back(0);
		if (!allowOutOfRange && (radius < getRadiusBottom() || radius > getRadiusTop()))
			coefficients.push_back(NaN_DOUBLE);
		else
			coefficients.push_back(1.);
	}

	/**
	 * Returns a deep copy of this profile.  Caller assumes ownership.
	 */
	virtual GeoTessProfile*      copy() = ABSTRACT;

	///@endcond

}; // end class Profile

} // end namespace geotess

#endif  // PROFILE_OBJECT_H
