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

#ifndef PROFILENPOINT_OBJECT_H
#define PROFILENPOINT_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessUtils.h"
#include "GeoTessException.h"
#include "GeoTessData.h"
#include "GeoTessProfile.h"
#include "GeoTessProfileType.h"
#include "GeoTessInterpolatorType.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

class GeoTessMetaData;
class IFStreamAscii;
class IFStreamBinary;

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief A Profile object consisting of N monotonically increasing radii that
 * span the radial extent of a layer, and an equal number of Data objects
 * that define the model values at the corresponding positions.
 *
 * A Profile object that defines N monotonically increasing radii that
 * span the radial extent of a layer, and an equal number of Data objects
 * that define the model values at the corresponding positions.
 *
 */
class GEOTESS_EXP_IMP GeoTessProfileNPoint : virtual public GeoTessProfile
{
private:

	/**
	 * Total number of radii and Data objects defining the profile.
	 */
	int									nNodes;

	/**
	 * Array of radii defining the profile.
	 */
	float*							radii;

	/**
	 * Array of Data objects defined at each radii.
	 */
	GeoTessData**							data;

	/**
	 * nAttributes x nNodex array containing the second derivatives at the node
	 * points. Used when doing cubic spline interpolation. Lazy evaluation is
	 * used, so elements of this variable are only instantiated when requested
	 * the first time, then they are permanently stored.
	 */
	mutable double**		y2;

	/**
	 * Map from nodeIndex to pointIndex.
	 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
	 * the number of Data objects managed by a Profile.  There is a pointIndex for every
	 * Data object in the entire model, indexed from 0 to the number of Data objects in the
	 * model.
	 */
	int*								pointIndices;

	/**
	 * Default constructor.
	 */
	GeoTessProfileNPoint() : GeoTessProfile(), nNodes(0), radii(NULL), data(NULL),
			y2(NULL), pointIndices(NULL) {};

	/**
	 * Defines the second derivatives at the input attribute index for all
	 * radii in x using a cubic spline.
	 */
	double*							spline(float* x, GeoTessData** y, int attributeIndex,
			double yp1, double ypn) const;

	/**
	 * Populates the second derivative of the input attribute index. (Note:
	 * This function must be synchronized to support concurrency but the
	 * capability is only supported functionally in C++ as of C++11.)
	 */
	void							check(int attributeIndex) const;

public:

	/**
	 * Standard constructor.
	 * Input radii and data values are copied from the input
	 * variables into the new Profile object.
	 * For the Data** array, a shallow copy is made.  In other
	 * words, this method instantiates a new Data** and copies
	 * the Data* pointers into it.  No copies are made of the
	 * Data objects.
	 * @param r an array of radius values, in km
	 * @param dat an array of pointers to Data objects.
	 * @param size number of elements in radii and dat.
	 */
	GeoTessProfileNPoint(float* r, GeoTessData** dat, int size) : GeoTessProfile(), nNodes(size), radii(NULL),
	data(NULL), y2(NULL), pointIndices(NULL)
	{
		radii = new float[size];
		data = new GeoTessData*[size];
		for (int i=0; i<size; ++i)
		{
			radii[i] = r[i];
			data[i] = dat[i];
		}

		if (radii[0] > radii[size - 1])
		{
			ostringstream os;
			os << endl << "ERROR in ProfileNPoint::ProfileNPoint" << endl
					<< "Profile has negative thickness" << endl;
			os << "radii = ";
			for (int i=0; i<size; ++i)
				os << radii[i] << ", ";
			os << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 4301);
		}
	}

	/**
	 * Standard constructor.
	 * Input radii and data values are copied from the input
	 * variables into the new Profile object.
	 * For vector<Data*>, a shallow copy is made.  In other
	 * words, this method instantiates a new Data** and copies
	 * the Data* pointers into it.  No copies are made of the
	 * Data objects.
	 * @param r an array of radius values, in km
	 * @param d an array of pointers to Data objects.
	 */
	GeoTessProfileNPoint(const vector<float>& r, vector<GeoTessData*>& d)
	: GeoTessProfile(), nNodes(r.size()), radii(NULL),
	data(NULL), y2(NULL), pointIndices(NULL)
	{
		radii = new float[nNodes];
		data = new GeoTessData*[nNodes];
		for (int i=0; i<nNodes; ++i)
		{
			radii[i] = r[i];
			data[i] = d[i];
		}

		if (r.size() != d.size())
		{
			ostringstream os;
			os << endl << "ERROR in ProfileNPoint::ProfileNPoint" << endl
					<< "radii.size() != data.size()" << endl;
			os << "radii.size = " << r.size() << endl;
			os << "data.size   = " << d.size() << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 4302);
		}

		if (radii[0] > radii[nNodes - 1])
		{
			ostringstream os;
			os << endl << "ERROR in ProfileNPoint::ProfileNPoint" << endl
					<< "Profile has negative thickness" << endl;
			os << "radii = ";
			for (int i=0; i<nNodes; ++i)
				os << radii[i] << ", ";
			os << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 4303);
		}
	}

	/**
	 * Standard constructor.
	 * Input radii and data values are copied from the input
	 * variables into the new Profile object.
	 */
	GeoTessProfileNPoint(float* rad, const vector<GeoTessData*>& dat);

	/**
	 * Returns the class name.
	 */
	static  string			class_name() { return "ProfileNPoint"; };

	/**
	 * Returns the class size.
	 */
	virtual int					class_size() const
	{ return (int) sizeof(GeoTessProfileNPoint); };

	virtual LONG_INT getMemory()
	{
		LONG_INT sz = (LONG_INT)sizeof(GeoTessProfileNPoint);
		if (nNodes > 0)
		{
			// add memory for radii
			sz += nNodes*(LONG_INT)sizeof(float);

			// add memory for data.  assumption is made that
			// GeoTessData objects at all nodes have the same type and
			// number of attributes.
			sz += nNodes*(LONG_INT)sizeof(GeoTessData*);
			sz += nNodes*data[0]->getMemory();

			if (pointIndices)
				sz += nNodes * (LONG_INT)(sizeof(int));
			if (y2)
				sz += data[0]->size() * nNodes * (LONG_INT)sizeof(double);
		}
		return sz;
	}

	/**
	 * Returns ProfileType (NPOINT).
	 *
	 * @return ProfileType (NPOINT).
	 */
	virtual	const GeoTessProfileType&	getType() const
	{ return GeoTessProfileType::NPOINT; };

	/**
	 * Return true if the input Profile object (p) equals this Profile object.
	 */
	virtual bool				operator == (const GeoTessProfile& p) const
																				{
		if (!GeoTessProfile::operator==(p)) return false;

		if (nNodes != p.getNRadii()) return false;

		for (int i = 0; i < nNodes; ++i)
			if ((radii[i] != p.getRadius(i)) ||
					(!(*(data[i]) == p.getData(i))))
				return false;

		return true;
																				}

	/**
	 * Return true if the specified Data value is NaN.
	 *
	 * @param nodeIndex
	 * @param attributeIndex
	 * @return true if the specified Data value is NaN.
	 */
	bool								isNaN(int nodeIndex, int attributeIndex)
	{ return data[nodeIndex]->isNaN(attributeIndex); };

	/**
	 * Get the i'th radius value in this profile in km. Radii are in order of
	 * increasing radius.
	 */
	virtual float				getRadius(int i) const { return radii[i]; };

	/**
	 * Get the number of radii that comprise this profile.
	 */
	virtual int					getNRadii() const { return nNodes; };

	/**
	 * Get the number of Data objects that comprise this profile.
	 */
	virtual int					getNData() const { return nNodes; };

	/**
	 * Retrieve a copy of the array of radii values in km.
	 * Caller assumes ownership of the array and must delete it when done with it.
	 */
	virtual float*			getRadii()
	{
		float* fa = new float [nNodes];
		for (int i=0; i<nNodes; ++i) fa[i] = radii[i];
		return fa;
	}

	/**
	 * Retrieve a shallow copy of the array of Data objects associated with this Profile.
	 * Caller assumes ownership of the array, but not the contents, and must delete the
	 * array when done with it.
	 */
	virtual GeoTessData**			getData()
	{
		GeoTessData** da = new GeoTessData*[nNodes];
		for (int i=0; i<nNodes; ++i) da[i] = data[i];
		return da;
	}


	/**
	 * Retrieve a reference the i'th Data object
	 */
	virtual GeoTessData*				getData(int i) { return data[i]; };

	/**
	 * Retrieve a reference the i'th Data object
	 */
	virtual const GeoTessData&	getData(int i) const { return *(data[i]); };

	/**
	 * Resets the data object to the new input data
	 */
	virtual void				setData(const vector<GeoTessData*>& inData);

	/**
	 * Replace the radii currently associated with this Profile with new values.
	 */
	virtual void					setRadii(const vector<float>& newRadii)
	{ for (int i=0; i<nNodes; ++i) radii[i] = newRadii[i]; }

	virtual void setRadius(int index, float radius)
	{ if (index < nNodes) radii[index] = radius; }

	/**
	 * Resets the data object to the new input data
	 */
	virtual void				setData(int index, GeoTessData* inData)
	{ delete data[index]; data[index] = inData; }

	/**
	 * Get the radius at the top of the profile, in km.
	 */
	virtual float				getRadiusTop() const { return radii[nNodes - 1]; };

	/**
	 * Get the Data object at the top of the profile.
	 */
	virtual const GeoTessData&	getDataTop() const { return *data[nNodes - 1]; };

	/**
	 * Get the Data object at the top of the profile.
	 */
	virtual GeoTessData*				getDataTop() { return data[nNodes - 1]; };

	/**
	 * Get the radius at the bottom of the profile, in km.
	 */
	virtual float				getRadiusBottom() const { return radii[0]; };

	/**
	 * Get the Data object at the bottom of the profile.
	 */
	virtual const GeoTessData&	getDataBottom() const { return *data[0]; };

	/**
	 * Get the Data object at the bottom of the profile.
	 */
	virtual GeoTessData*				getDataBottom() { return data[0]; };


	/**
	 * Retrieve the value of the specified attributes at the specified
	 * radius index.
	 */
	virtual double getValue(int attributeIndex, int radiusIndex) const
	{	return data[radiusIndex]->getDouble(attributeIndex); }

	/**
	 * Retrieve the value of the specified attribute at the top of the layer.
	 *
	 * @param attributeIndex
	 * @return double
	 */
	virtual double getValueTop(int attributeIndex) const
	{	return data[nNodes-1]->getDouble(attributeIndex); }

	/**
	 * Finds interpolation coefficient at the iput radius constrained to lie
	 * between radii[index] and radii[index + 1].
	 */
	virtual double			getInterpolationCoefficient(int index, double radius) const;

	/**
	 * Retrieve the value of the specified attribute interpolated at the
	 * specified radius.
	 */
	virtual double			getValue(const GeoTessInterpolatorType& radialType,
			int attributeIndex, double radius,
			bool allowRadiusOutOfRange) const;

	/**
	 * Find index i such that x is >= xx[i] and < xx[i+1]. If x < xx[1] returns
	 * 0. If x >= xx[xx.length-2] return xx.length-2.
	 */
	virtual int					getRadiusIndex(double radius, int jlo) const;

	/**
	 * find interpolation coefficient using linear interpolation.
	 */
	virtual double			getInterpolationCoefficient(int index, double radius,
			bool allowOutOfRange) const;

	/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

	/**
	 * Standard constructor. Reads radii from the provided input file stream.
	 */
	GeoTessProfileNPoint(IFStreamBinary& ifs, GeoTessMetaData& gtmd);

	/**
	 * Standard constructor. Reads radii from the provided input file stream.
	 */
	GeoTessProfileNPoint(IFStreamAscii& ifs, GeoTessMetaData& gtmd);

	/**
	 * Default destructor.
	 */
	virtual							~GeoTessProfileNPoint();

	/**
	 * Write the radii and data values to binary file.
	 */
	virtual void				write(IFStreamBinary& ofs);

	/**
	 * Write the radii and data values to binary file.
	 */
	virtual void				write(IFStreamAscii& ofs);

	/**
	 * Find the index of the node in this Profile that has radius closest to the
	 * supplied radius.
	 *
	 * @param radius in km
	 * @return The index of the node in this Profile that has radius closest to the
	 *         supplied radius.
	 */
	virtual int					findClosestRadiusIndex(double radius) const
	{
		int i = GeoTessProfile::getRadiusIndex(radius);
		return abs(radii[i+1] - radius) < abs(radii[i] - radius) ? i+1 : i;
	}

	/**
	 * Set the pointIndex that corresponds to the supplied nodeIndex.
	 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
	 * the number of Data objects managed by a Profile.  There is a pointIndex for every
	 * Data object in the entire model, indexed from 0 to the number of Data objects in the
	 * model.
	 */
	virtual void				setPointIndex(int nodeIndex, int pointIndex)
	{
		if (pointIndices == NULL)
		{
			if (pointIndex < 0) return;

			pointIndices = new int [nNodes];
			for (int i = 0; i < nNodes; ++i) pointIndices[i] = -1;
		}
		pointIndices[nodeIndex] = pointIndex;
	}

	/**
	 * Reset all the pointIndex values to -1.
	 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
	 * the number of Data objects managed by a Profile.  There is a pointIndex for every
	 * Data object in the entire model, indexed from 0 to the number of Data objects in the
	 * model.
	 */
	virtual void resetPointIndices()
	{
		if (pointIndices != NULL)
			delete[] pointIndices;
		pointIndices = NULL;
	}

	/**
	 * Get the pointIndex that corresponds to the supplied nodeIndex.
	 * <p>There is a node index for each Data object in a profile and they are indexed from 0 to
	 * the number of Data objects managed by a Profile.  There is a pointIndex for every
	 * Data object in the entire model, indexed from 0 to the number of Data objects in the
	 * model.
	 */
	virtual int					getPointIndex(int nodeIndex) const
	{	return pointIndices == NULL ? -1 : pointIndices[nodeIndex]; }

	/**
	 * Add dkm * hcoefficient to the weight of this point index into the input map.
	 * This works for Profile types Constant, Thin and Surface since they only have
	 * a single node in the profile. It does not work for ProfileNPoint and
	 * ProfileEmpty so they override this method.
	 */
	virtual void				getWeights(map<int, double>& weights,
			double dkm, double radius, double hcoefficient) const
	{
		int node = GeoTessProfile::getRadiusIndex(radius);

		//TODO:  need getInterpolationCoefficient to work for cubic spline interpolator.  It currently does not.

		// get coefficient at node influencing position radius

		int pointIndex = pointIndices == NULL ? -1 : pointIndices[node];

		double c = getInterpolationCoefficient(node, radius, true);
		if (c > 0.0)
		{
			map<int, double>::iterator it = weights.find(pointIndex);
			if (it == weights.end())
				weights[pointIndex] = dkm * hcoefficient * c;
			else
				it->second += dkm * hcoefficient * c;
		}

		// now get coefficient at node+1  influencing position radius

		c = 1.0 - c;
		pointIndex = pointIndices == NULL ? -1 : pointIndices[node+1];
		if (c > 0.0)
		{
			map<int, double>::iterator it = weights.find(pointIndex);
			if (it == weights.end())
				weights[pointIndex] = dkm * hcoefficient * c;
			else
				it->second += dkm * hcoefficient * c;
		}
	}

	/**
	 * Add entries to supplied map from pointIndex to interpolation coefficient.
	 * @param coefficients map from pointIndex to interpolation coefficient
	 * @param radius
	 * @param horizontalCoefficient
	 */
	virtual void					getCoefficients(map<int, double>& coefficients, double radius,
			double horizontalCoefficient) const
	{
		int node = GeoTessProfile::getRadiusIndex(radius);

		//TODO:  need getInterpolationCoefficient to work for cubic spline interpolator.  It currently does not.
		double c = getInterpolationCoefficient(node, radius, true);

		int pointIndex = pointIndices == NULL ? -1 : pointIndices[node];

		if (c > 0.0)
			coefficients[pointIndex] = c * horizontalCoefficient;

		pointIndex = pointIndices == NULL ? -1 : pointIndices[node+1];

		if (c < 1.0)
			coefficients[pointIndex] = (1.0 - c) * horizontalCoefficient;
	}

	virtual void setInterpolationCoefficients(const GeoTessInterpolatorType& interpType,
			vector<int>& nodeIndexes, vector<double>& coefficients,
			double& radius, bool& allowOutOfRange)
	{
		//TODO:  need this method to work for cubic spline interpolator.  It currently does not.

		if (radius < radii[0])
		{
			nodeIndexes.push_back(0);
			coefficients.push_back(allowOutOfRange ? 1 : NaN_DOUBLE);
		}
		else if (radius > radii[nNodes-1])
		{
			nodeIndexes.push_back(nNodes-1);
			coefficients.push_back(allowOutOfRange ? 1 : NaN_DOUBLE);
		}
		else
		{
			int index = getRadiusIndex(radius, -1);
			double c = ((double)radii[index + 1] - radius) /
					((double)radii[index + 1] - (double)radii[index]);
			nodeIndexes.push_back(index);
			coefficients.push_back(c);
			if (c < 1.)
			{
				nodeIndexes.push_back(index+1);
				coefficients.push_back(1.-c);
			}
		}
	}

	/**
	 * Returns a deep copy of this profile.
	 */
	virtual GeoTessProfile*      copy()
	{
		GeoTessData** d = new GeoTessData* [nNodes];
		float* r = new float [nNodes];
		for (int i = 0; i < nNodes; ++i)
		{
			d[i] = data[i]->copy();
			r[i] = radii[i];
		}
		return new GeoTessProfileNPoint(r, d, nNodes);
	}

	///@endcond

}; // end class ProfileNPoint

/**
 * Finds interpolation coefficient at the iput radius constrained to lie
 * between radii[index] and radii[index + 1].
 *
 * @param index  The index in r such that radius is between r[index] and
 *               r[index+1].
 * @param radius The radius whose interpolation coefficient is desired.
 * @return c[index], the interpolation coefficient to be applied at
 *         r[index]. The interpolation coefficient for r[index+1] is
 *         1-c[index].
 */
inline double	GeoTessProfileNPoint::getInterpolationCoefficient(int index,  double radius) const
{
	if (radius <= radii[index]) return 1.0;
	if (radius >= radii[index + 1]) return 0.0;
	return (radii[index + 1] - radius) / (radii[index + 1] - radii[index]);
}

/**
 * Retrieve the value of the specified attribute interpolated at the
 * specified radius.
 * @param radialType the interpolator type, either InterpolatorType::LINEAR
 * or InterpolatorType::CUBIC_SPLINE.
 * @param attributeIndex
 * @param radius
 * @param allowRadiusOutOfRange if false and radius is out of range, returns
 * NaN_DOUBLE
 * @return double
 * @throws GeoTessException
 */
inline double GeoTessProfileNPoint::getValue(const GeoTessInterpolatorType& radialType,
		int attributeIndex, double radius, bool allowRadiusOutOfRange) const
{
	if (!allowRadiusOutOfRange &&
			((radius < (double)radii[0]) || (radius > (double)radii[nNodes-1])))
		return NaN_DOUBLE;

	int index = getRadiusIndex(radius, -1);

	double r0 = radii[index];
	double v0 = data[index]->getDouble(attributeIndex);

	if (radius <= r0) return v0;

	double r1 = radii[index + 1];
	double v1 = data[index + 1]->getDouble(attributeIndex);

	if (radius >= r1) return v1;

	double a = (r1 - radius) / (r1 - r0);

	double b = 1. - a;
	double v = a * v0 + b * v1;

	switch (radialType.ordinal())
	{
	case 0: // LINEAR
		return	v;

	case 2: // CUBIC_SPLINE
		check(attributeIndex);
		return v + ((a * a * a - a) * y2[attributeIndex][index]
		                                                 + (b * b * b - b) * y2[attributeIndex][index + 1])
		                                                 * (r1 - r0)	* (r1 - r0) / 6.0;

	default:
		ostringstream os;
		os << endl << "ERROR in ProfileNPoint::getValue" << endl
				<< "InterpolatorType: " << radialType.name()
				<< " cannot be applied to a Profile." << endl
				<< "Must specify LINEAR or SPLINE" << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 4304);
	}
}

/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

/**
 * Populates the second derivative of the input attribute index. (Note:
 * This function must be synchronized to support concurrency but the
 * capability is only supported functionally in C++ as of C++11.)
 */
inline void GeoTessProfileNPoint::check(int attributeIndex) const
{
	if (y2 == NULL)
	{
		y2 = new double* [data[0]->size()];
		for (int i=0; i<data[0]->size(); ++i)
			y2[i] = NULL;
	}
	if (y2[attributeIndex] == NULL)
		y2[attributeIndex] = spline(radii, data, attributeIndex, 1.0e30, 1.0e30);
}

///@endcond

} // end namespace geotess

#endif  // PROFILENPOINT_OBJECT_H
