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

#ifndef GEOTESSPOSITION_OBJECT_H
#define GEOTESSPOSITION_OBJECT_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>

// use standard library objects
using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "GeoTessUtils.h"
#include "GeoTessModel.h"
#include "GeoTessGrid.h"
#include "GeoTessMetaData.h"
#include "CpuTimer.h"
#include "GeoTessProfile.h"
#include "GeoTessData.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _FORWARD REFERENCES_ ***************************************************

class GeoTessInterpolatorType;
class GeoTessProfile;

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief Information about an interpolated point at an arbitrary position in a model.
 *
 * Manages information about a single point at an arbitrary position in a triangular tessellation.
 * It provides access to:
 * <ul>
 * <li>the position of the point (latitude-longitude, or unit vector)
 * <li>the index of the triangle in which the point resides
 * <li>the indexes of the nodes used to interpolate data
 * <li>the interpolation coefficients used to interpolate data
 * </ul>
 *
 * <p>GeoTessModel has method getGeoTessPosition() that returns a pointer to a new
 * GeoTessPosition object.
 *
 * <p>GeoTessPosition supports separate interpolation algorithms in the geographic and
 * radial dimensions.  In the geographic dimensions, LINEAR and NATURAL_NEIGHBOR
 * interpolation algorithms are supported.  LINEAR and CUBIC_SPLINE interpolation are
 * supported in the radial dimension.
 *
 * <p>
 * GeoTessPosition is not thread-safe in that it's internal state is mutable. The design intention
 * is that single instances of a GeoTessGrid object and GeoTessData object can be shared among all
 * the threads in a multi-threaded application and each thread will have it's own instance of a
 * GeoTessPosition object that references the common GeoTessGrid + GeoTessData combination.
 *
 * @author Sandy Ballard
 *
 */
class GEOTESS_EXP_IMP GeoTessPosition
{
private:

	/**
	 * Reference count.
	 */
	int refCount;

	/**
	 * The walking triangle algorithm will search no higher than this level.
	 * Defaults to a huge number but applications can set this to something
	 * less than the largest tessellation level to limit the search.
	 */
	int*									maxTessLevel;

	/**
	 * Tessellation level. Usually the top level of the current tessellation,
	 * but not if maxTessLevel is smaller. There is a separate tessLevel stored
	 * for each tessellation in the model.
	 */
	int*									tessLevels;

	/**
	 * The index of the triangles in which current position resides. There is a
	 * separate triangle for each tessellation in the model. This variable is
	 * used to flag which tessellations are currently up-to-date and which need
	 * to be updated.  If the value of triangel[tessid] is < 0 it means that
	 * the triangle on tessellation tessid that contains current point has not
	 * been identified.
	 */
	int*									triangle;

	/**
	 * This is the value returned when an invalid interpolation result is
	 * obtained.  Defaults to NaN, but applications can change it to some other
	 * value if they choose.
	 */
	double								errorValue;

	/**
	 * Set the 2D position and ensure that the triangle, vertices and 2D
	 * interpolation coefficients for the tessellation that supports layerid
	 * have been updated. Sets radius = -1.  Must call updateRadius after
	 * this call to ensure that radius is set properly.
	 *
	 * @param layerid
	 * @param uVector
	 *
	 */
	void									updatePosition2D(int layerid, const double* const uVector);

	/**
	 * Ensure that vertices and coefficients have been computed for the
	 * specified tessellation, assuming that the 2D position has not changed.
	 *
	 * <p>
	 * This method won't do anything if the containing triangle in the
	 * specified tessellation has already been computed. What can happen
	 * however, is that layerId and unitVector are changed with a call to
	 * setPosition2D(layerid, uVector). That will update all the information
	 * about vertices and coefficients for the tessid that supports layerid
	 * but will nullify that information for other tessids. This method can
	 * be called to ensure that vertices and coefficients are up-to-date for
	 * any tessid.
	 *
	 * @param tid
	 *
	 */
	void									checkTessellation(const int& tid)
	{
		if (triangle[tid] < 0)
		{
			tessLevels[tid] = 0;
			triangle[tid] = grid.getTriangle(tid, 0, 0);
			getContainingTriangle(tid);
			update2D(tid);
		}
	}

	/**
	 * Update the layerId, radius and tessid of this position.
	 *
	 * @param layerId
	 * @param radius
	 *
	 */
	void	updateRadius(int lid, double rad)
	{
		if ((radius < 0.) || (rad != radius) || (lid != layerId))
		{
			radius = rad;
			layerId = lid;
			tessid = layerTessIds[lid];
			checkTessellation(tessid);
			clearRadialCoefficients();
		}
	}

	void clearRadialCoefficients()
	{
		for (int i=0; i<(int)radialIndexes.size(); ++i)
		{
			radialIndexes[i].clear();
			radialCoefficients[i].clear();
		}
	}

	/**
	 * Update the radial interpolation indexes and coefficients.
	 * This method should be called whenever the position is modified.
	 * Assumes that the proper tessid, layerId and radius are in effect.
	 *
	 */
	void updateRadialCoefficients()
	{
		// make sure dimensions of radialIndexes and radialCoefficients
		// are at least as big as the number of vertices involved in interpolation.
		if (radialIndexes.size() < vertices[tessid].size())
		{
			radialIndexes.resize(vertices[tessid].size());
			radialCoefficients.resize(vertices[tessid].size());
		}

		// if radial interpolation coefficients have already been computed
		// do nothing.
		if (radialIndexes[0].size() == 0)
		{
			vector<int>& v = vertices[tessid];
			for (int i = 0; i < (int)v.size(); ++i)
				modlProfiles[v[i]][layerId]->setInterpolationCoefficients(radialInterpolatorType,
						radialIndexes[i], radialCoefficients[i], radius, radiusOutOfRangeAllowed);
		}
	}

	/**
	 * Tolerance value used in triangle walk algorithm implemented in
	 * getContainingTriangle()
	 */
	static const double TWALK_TOLERANCE;

	/**
	 * Find the index of the triangle that contains position. Also computes the 3 interpolation
	 * coefficients that can be used to interpolate data stored on the vertices of the returned
	 * triangle.
	 * <p>
	 * A GeoTessPosition object has an attribute maxTessLevel that defaults to Integer.maxValue. The
	 * search is limited to that tessellation level. So the triangle identified by this method will
	 * reside either on x.maxTessLevel, or the largest tessellation level of the tessellation,
	 * whichever is smaller.
	 */
	void									getContainingTriangle(int tid);

	/**
	 * Compare two triangles and return the larger of the two.
	 * Compares the inverse of the sum of the dot products of the
	 * triangle's edges.
	 * @param t1 index of triangle 1
	 * @param t2 index of triangle 2
	 * @return the index of the larger triangle, either t1 or t2
	 */
	int biggerTriangle(int t1, int t2)
	{
		const int* tv = grid.getTriangleVertexIndexes(t1);
		double dot1 = GeoTessUtils::dot(grid.getVertex(tv[0]), grid.getVertex(tv[1]))
				+ GeoTessUtils::dot(grid.getVertex(tv[1]), grid.getVertex(tv[2]))
				+ GeoTessUtils::dot(grid.getVertex(tv[2]), grid.getVertex(tv[0]));

		tv = grid.getTriangleVertexIndexes(t2);
		double dot2 = GeoTessUtils::dot(grid.getVertex(tv[0]), grid.getVertex(tv[1]))
				+ GeoTessUtils::dot(grid.getVertex(tv[1]), grid.getVertex(tv[2]))
				+ GeoTessUtils::dot(grid.getVertex(tv[2]), grid.getVertex(tv[0]));

		return dot2 > dot1 ? t1 : t2;

	}

protected:

	/// @cond PROTECTED  Turn off doxygen documentation until 'endcond' is found

	/**
	 * Radius of current position, in km.
	 */
	double								radius;

	/**
	 * radius of the earth at the current position.
	 */
	double								earthRadius;

	/**
	 * an array of length nLayers+1, where layerRadii[i] is the radius of the bottom of layer i. The
	 * last (extra) element is the radius of the top of the last layer.
	 * <p>
	 * Lazy evaluation is used here. Values are set to model.NA_VALUE whenever this position is
	 * moved and then set to appropriate values whenever a value is requested.
	 */
	vector<double>				layerRadii;

	/**
	 * Index of the current layer
	 */
	int										layerId;

	/**
	 * The index of the tessellation of the current position.
	 */
	int										tessid;

	/**
	 * The indexes of vertices in the 2D grid that will be involved in the
	 * interpolation of data. There is a separate set of vertices for each
	 * tessellation in the model. When linear interpolation is used this
	 * will be an nTessellation x 3 array, but higher order interpolation
	 * methods may involve more than 3 vertices.
	 */
	vector< vector< int> >	vertices;

	/**
	 * The linear interpolation coefficients that apply to the containing triangles.
	 * nTessellations x 3. For each tessellation, the 3 elements are
	 * normalized to 1.
	 */
	vector< vector< double> >	linearCoefficients;

	/**
	 * The interpolation coefficients used when natural neighbor interpolation
	 * is to be used.  nTessellations x nVertices where nVertices is the
	 * number of vertices used in the interpolation (see vertices above).
	 * For each tessellation, the coefficients must sum to 1.
	 */
	//vector< vector< double> >		nnCoefficients;

	/**
	 * reference assigned to either linearCoefficients or nnCoefficients.
	 */
	vector< vector< double> >	hCoefficients;

	/**
	 * An nVertices x nNodes array where nVertices is the number of vertices
	 * involved in horizontal interpolation and nNodes is the number of nodes
	 * in the associated Profile that are involved in the radial interpolation.
	 * Each element is the index of the node in the Profile that should be
	 * queried to find a data value.
	 * <p>
	 * radialIndexes and radialCoefficients are very tightly linked.  They
	 * must be the same size in both dimensions and they are always modified
	 * together.  They are resized (the nVertices dimension) in updatePosition2D
	 * and then that dimension is never changed.  The second dimension (nNodes)
	 * is populated with calls to a Profile object that populates the
	 * radial interpolation information as appropriate.
	 */
	vector<vector<int> > radialIndexes;

	/**
	 * An nVertices x nNodes array where nVertices is the number of vertices
	 * involved in horizontal interpolation and nNodes is the number of nodes
	 * in the associated Profile that are involved in the radial interpolation.
	 * Each element is the radial interpolation coefficient that should be
	 * applied to the nodes in teh current Profile.  Note that these
	 * coefficients sum to one in each Profile.  They must be multiplied
	 * by the horizontal interpolation coefficient for the vertex to get
	 * the total weight of each node.
	 * <p>
	 * radialIndexes and radialCoefficients are very tightly linked.  They
	 * must the same size in both dimensions and they are always modified
	 * together.  They are resized (the nVertices dimension) in updatePosition2D
	 * and then that dimension is never changed.  The second dimension (nNodes)
	 * is populated with calls to a Profile object that populates the
	 * radial interpolation information as appropriate.
	 */
	vector<vector<double> > radialCoefficients;


	/**
	 * The interpolation algorithm to use in the radial direction.  Currently
	 * either InterpolationType.LINEAR or InterpolationType.CUBIC_SPLINE.
	 */
	const GeoTessInterpolatorType& radialInterpolatorType;

	/**
	 * A pointer to the 3D model that holds the model information that this position object will
	 * interrogate.
	 */
	GeoTessModel*		model;

	/**
	 * A reference to the GeoTessGrid.
	 */
	GeoTessGrid&		grid;

	/**
	 * Get references to grid objects that will be used a lot in the triangle walking algorithm.
	 */
	GeoTessProfile ***  modlProfiles;
	double const* const*          gridVertices;
	int const* const*             gridTriangles;
	int const*                    gridDescendants;
	const vector<vector<Edge*> >& gridEdges;
	const int*                    layerTessIds;
	int                           nLayers;

	/**
	 * Controls radius out-of-range behavior.  If position or radius is
	 * set with a specified layer index, and a radius is specified that
	 * is outside the bounds of that layer, and radiusOutOfRangeAllowed
	 * is true, then interpolated values will be computed using the values
	 * from the top or bottom of the specified layer as appropriate.
	 * If radiusOutOfRangeAllowed is false then errorValue is returned.
	 * <p>If position or radius is
	 * set without a specified layer index, and a radius is specified that
	 * is above the surface of the Earth, and radiusOutOfRangeAllowed
	 * is true, then interpolated values will be computed using the values
	 * from the top of the shallowest layer that has finite thickness.
	 * If radiusOutOfRangeAllowed is false then errorValue is returned.
	 */
	bool												radiusOutOfRangeAllowed;

	/**
	 * The earth-centered unit vector that corresponds to current position.
	 */
	double								unitVector[3];

	/**
	 * Return the triangle id of the input tess id (tid).
	 */
	int										getTriangle(int tid)
	{ checkTessellation(tid); return triangle[tid]; };

	/**
	 * Update the 2D vertices and horizontal interpolation coefficients.
	 * Different types of interpolators will handle this differently.
	 */
	virtual void					update2D(int tid) = ABSTRACT;

	/**
	 * Standard Constructor.
	 */
	GeoTessPosition(GeoTessModel* model, const GeoTessInterpolatorType& radialType);

	/**
	 * calculate memory needed by contents of int* arrays and vectors;
	 */
	long memory()
	{
		long m = 0;

		// memory for int* triangle, tessLevels and maxTessLevel.
		m += (long)(grid.getNTessellations() * 3 * sizeof(int));

		// vector<double> layerRadii;
		m += (long)(layerRadii.capacity() * sizeof(double));

		// vector< vector< int> > vertices;
		m += (long)(vertices.capacity()*sizeof(vector<int>));
		for (int i=0; i<(int)vertices.size(); ++i)
			m += (long)(vertices[i].capacity()*sizeof(int));

		// vector< vector< double> > linearCoefficients;
		m += (long)(linearCoefficients.capacity()*sizeof(vector<double>));
		for (int i=0; i<(int)linearCoefficients.size(); ++i)
			m += (long)(linearCoefficients[i].capacity()*sizeof(double));

		// vector< vector< double> > hCoefficients;
		m += (long)(hCoefficients.capacity()*sizeof(vector<double>));
		for (int i=0; i<(int)hCoefficients.size(); ++i)
			m += (long)(hCoefficients[i].capacity()*sizeof(double));

		// vector<vector<int> > radialIndexes;
		m += (long)(radialIndexes.capacity()*sizeof(vector<int>));
		for (int i=0; i<(int)radialIndexes.size(); ++i)
			m += (long)(radialIndexes[i].capacity()*sizeof(int));

		// vector<vector<double> > radialCoefficients;
		m += (long)(radialCoefficients.capacity()*sizeof(vector<double>));
		for (int i=0; i<(int)radialCoefficients.size(); ++i)
			m += (long)(radialCoefficients[i].capacity()*sizeof(double));

		return m;
	}

	/// @endcond

public:

	/**
	 * Static factory method to create a new GeoTessPosition object.
	 * Linear interpolation will be performed in both the horizontal and
	 * radial dimensions.
	 * <p>It is the caller's responsibility to delete this object when
	 * it is no longer needed.
	 * @param model pointer to the model that supports this position
	 * @return a GeoTessPosition object.
	 */
	static GeoTessPosition*	getGeoTessPosition(GeoTessModel* model);

	/**
	 * Static factory method to create a new GeoTessPosition object.
	 * If the horizontal InterpolatorType
	 * is LINEAR then the radial InterpolatorType will be LINEAR as well.
	 * If the horizontal InterpolatorType
	 * is NATUAL_NEIGHBOR then the radial InterpolatorType will be CUBIC_SPLINE.
	 * <p>It is the caller's responsibility to delete this object when
	 * it is no longer needed.
	 * @param model the GeoTessModel from which values will be interpolated.
	 * @param horizontalType the type of interpolation that is to be used
	 * for interpolation in the geographic dimensions;
	 * either InterpolatorType:LINEAR or InterpolatorType::NATURAL_NEIGHBOR
	 * @return a GeoTessPosition object.
	 */
	static GeoTessPosition*	getGeoTessPosition(GeoTessModel* model,
			const GeoTessInterpolatorType& horizontalType);

	/**
	 * Static factory method to create a new GeoTessPosition object.
	 * <p>It is the caller's responsibility to delete this object when
	 * it is no longer needed.
	 * @param model the GeoTessModel from which values will be interpolated.
	 * @param horizontalType the type of interpolation that is to be used
	 * for interpolation in the geographic dimensions;
	 * either InterpolatorType:LINEAR or InterpolatorType::NATURAL_NEIGHBOR
	 * @param radialType the type of interpolation that is to be used in the
	 * radia dimension;
	 * either InterpolatorType:LINEAR or InterpolatorType::CUBIC_SPLINE
	 * @return a GeoTessPosition object.
	 */
	static GeoTessPosition*	getGeoTessPosition(GeoTessModel* model,
			const GeoTessInterpolatorType& horizontalType, const GeoTessInterpolatorType& radialType);

	/**
	 * Destructor.
	 */
	virtual								~GeoTessPosition();

	/**
	 * Retrieve the type of interpolator being used for interpolation in
	 * geographic dimensions.
	 * @return either InterpolatorType::LINEAR or
	 * InterpolatorType::NATURAL_NEIGHBOR
	 */
	virtual const GeoTessInterpolatorType&	getInterpolatorType() const = ABSTRACT;

	/**
	 * Retrieve the amount of memory consumed by this GeoTessPosition object.
	 * @return memory in bytes.
	 */
	virtual long getMemory() = ABSTRACT;

	/**
	 * Retrieve an interpolated value of the specified model attribute.
	 * @param attribute index
	 * @return the value of the specified attribute interpolated at the
	 * current position
	 */
	virtual double				getValue(int attribute);

	/**
	 * Replace the model that currently supports this GeoTessPosition object
	 * with a new model. For this to work, the new model and the current model
	 * must use the same grid.
	 *
	 * <p>
	 * The benefit of calling this method is that if the application needs to
	 * interpolate a value at the same position in multiple models
	 * that share the same grid, then the walking triangle algorithm and the
	 * calculation of geographic interpolation coefficients do not have to be
	 * repeated.  The radial interpolation information is updated by this
	 * method as well so after the call to this method, GeoTessPosition is ready
	 * to interpolate values from the new model.
	 *
	 * @param newModel
	 *            model that is to replace the currently supported model.
	 * @throws GeoTessException
	 *             if the new model and current model do not have GeoTessGrids
	 *             that have the same gridID.
	 */
	void									setModel(GeoTessModel* newModel);

	/**
	 * Returns true if the current interpolation position has no empty profiles
	 * @return true if the current interpolation position has no empty profiles
	 */
	bool									noEmptyProfiles();

	/**
	 * Set the interpolation point to specified latitude and and longitude in
	 * degrees and depth in km below sea level. This method will perform a
	 * walking triangle search for the triangle in which the specified position
	 * is located and compute the associated interpolation coefficients.
	 * <p>
	 * This method is pretty expensive compared to the other version of
	 * setPosition where the position is specified as a unit vector and a
	 * radius.
	 * <p>
	 * Assumes GRS80 ellipsoid.
	 *
	 * @param lat
	 *            in degrees.
	 * @param lon
	 *            in degrees.
	 * @param depth
	 *            below sea level in km.
	 *
	 */
	void 									set(double lat, double lon,	double depth)
	{
		double uVector[3] = {0.0, 0.0, 0.0};
		GeoTessUtils::getVectorDegrees(lat, lon, uVector);
		double newRadius = GeoTessUtils::getEarthRadius(uVector) - depth;
		set(uVector, newRadius);
	}

	/**
	 * Set the interpolation point. This method will perform a walking triangle
	 * search for the triangle in which the specified position is located and
	 * compute the associated 2D and radial interpolation coefficients.
	 *
	 * @param uVector
	 *            the Earth-centered unit vector that defines the position that
	 *            is to be set.
	 * @param newRadius
	 *            the radius of the position, in km.
	 *
	 */
	void set(const double* const uVector, const double& newRadius)
	{
		updatePosition2D(nLayers - 1, uVector);

		int lid = getLayerId(newRadius);

		updatePosition2D(lid, uVector);
		updateRadius(lid, newRadius);
	}

	/**
	 * Set the interpolation point to specified latitude and longitude in degrees and depth in
	 * km below sea level. This method will perform a walking triangle search for the triangle in
	 * which the specified position is located and compute the associated interpolation
	 * coefficients.
	 * <p>
	 * This method is pretty expensive compared to the other version of setPosition where the
	 * position is specified as a unit vector and a radius.
	 * <p>
	 * Assumes GRS80 ellipsoid.
	 *
	 * @param layid the index of the layer of the model in which the position is to be constrained.
	 * @param lat in degrees.
	 * @param lon in degrees.
	 * @param depth below sea level in km.
	 *
	 */
	void set(int layid, double lat, double lon, double depth)
	{
		if (layid < 0)
			set(lat, lon, depth);
		else
		{
			double uVector[3];
			GeoTessUtils::getVectorDegrees(lat, lon, uVector);
			updatePosition2D(layid, uVector);
			updateRadius(layid, GeoTessUtils::getEarthRadius(uVector) - depth);
		}
	}

	/**
	 * Set the interpolation point. This method will perform a walking triangle search for the
	 * triangle in which the specified position is located and compute the associated 2D and radial
	 * interpolation coefficients.
	 *
	 * @param layid the index of the layer of the model in which the position is to be constrained.
	 * @param uVector the Earth-centered unit vector that defines the position that is to be set.
	 * @param rad the radius of the position, in km.
	 *
	 */
	void									set(int layid, const double* const uVector, double rad)
	{
		if (layid < 0)
			set(uVector, rad);
		else
		{
			updatePosition2D(layid, uVector);
			if (rad >= 0.0) updateRadius(layid, rad);
		}
	}

	/**
	 * Set the 2D position to unitVector and radius to the radius of the top of the specified layer.
	 *
	 * @param layid the index of the layer of the model in which the position is located.
	 * @param uVector the Earth-centered unit vector that defines the position that is to be set.
	 *
	 */
	void									setTop(int layid, const double* const uVector)
	{
		updatePosition2D(layid, uVector);
		updateRadius(layid, getRadiusTop(layid));
	}

	/**
	 * Set the 2D position to unitVector and radius to the radius of the bottom of the specified
	 * layer.
	 *
	 * @param layid the index of the layer of the model in which the position is located.
	 * @param uVector the Earth-centered unit vector that defines the position that is to be set.
	 *
	 */
	void									setBottom(int layid, const double* const uVector)
	{
		updatePosition2D(layid, uVector);
		updateRadius(layid, getRadiusBottom(layid));
	}

	/**
	 * Change the current radius without changing the geographic position.
	 *
	 * @param layid the index of the layer of the model in which the position is located.
	 * @param rad the new radius in km
	 *
	 */
	void									setRadius(int layid, double rad)
	{
		if (tessid < 0)
		{
			ostringstream os;
			os << endl << "ERROR in GeoTessPosition::setRadius" << endl
					<< "Geographic position has not been specified." << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 3001);
		}
		updateRadius(layid, rad);
	}

	/**
	 * Change the current radius without changing the geographic position.
	 *
	 * @param rad the new radius in km
	 *
	 */
	void setRadius(double rad)
	{
		if (tessid < 0)
			if (tessid < 0)
			{
				ostringstream os;
				os << endl << "ERROR in GeoTessPosition::setRadius" << endl
						<< "Geographic position has not been specified." << endl;
				throw GeoTessException(os, __FILE__, __LINE__, 3002);
			}
		updateRadius(getLayerId(rad), rad);
	}

	/**
	 * Change the current layer and/or depth without changing the geographic
	 * position.
	 *
	 * @param layer
	 *            the index of the layer of the model in which the position is
	 *            located.
	 * @param depth
	 * @return reference to this GeoTessPosition object.
	 *            the depth of the position, in km.
	 */
	void setDepth(int layer, double depth)
	{
		setRadius(layer, getEarthRadius()-depth);
	}

	/**
	 * Change the current depth without changing the geographic
	 * position.
	 *
	 * @param depth depth in km.
	 */
	void setDepth(double depth)
	{
		setRadius(getEarthRadius()-depth);
	}

	/**
	 * Set the radius to the radius of the top of the specified layer.
	 *
	 * @param layid the index of the layer of the model in which the position is located.
	 */
	void									setTop(int layid)
	{
		if (tessid < 0)
		{
			ostringstream os;
			os << endl << "ERROR in GeoTessPosition::setRadius" << endl
					<< "Geographic position has not been specified." << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 3003);
		}
		tessid = layerTessIds[layid];
		checkTessellation(tessid);
		updateRadius(layid, getRadiusTop(layid));
	}

	/**
	 * Set the radius to the radius of the bottom of the specified layer.
	 *
	 * @param layid the index of the layer of the model in which the position is located.
	 *
	 */
	void									setBottom(int layid)
	{
		if (tessid < 0)
		{
			ostringstream os;
			os << endl << "ERROR in GeoTessPosition::setRadius" << endl
					<< "Geographic position has not been specified." << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 3004);
		}
		tessid = layerTessIds[layid];
		checkTessellation(tessid);
		updateRadius(layid, getRadiusBottom(layid));
	}

	/**
	 * Retrieve an interpolated value of the radius of the top of the specified layer, in km.
	 *
	 * @param layid the index of the layer
	 * @return interpolated value of the radius, in km.
	 *
	 */
	double								getRadiusTop(int layid);

	/**
	 * Retrieve an interpolated value of the radius of the bottom of the specified layer, in km.
	 * @param layid the index of the layer
	 * @return interpolated value of the radius, in km.
	 *
	 */
	double								getRadiusBottom(int layid);

	/**
	 * Retrieve the radius of the Earth at this position, in km. Assumes GRS80 ellipsoid.
	 *
	 * @return the radius of the Earth at this position, in km.
	 */
	double								getEarthRadius()
	{
		if (earthRadius < 0.)
			earthRadius = GeoTessUtils::getEarthRadius(unitVector);
		return earthRadius;
	}

	/**
	 * Retrieve a reference to the 3 component unit vector that corresponds to the current position.
	 * Do not modify the values of this array.
	 *
	 * @return a reference to the 3 component unit vector that corresponds to the current position
	 */
	double*								getVector() { return unitVector; };

	/**
	 * Copy the contents of the unit vector that corresponds to the current position into
	 * the supplied double* which must have at least 3 elements.
	 *
	 * @param u a 3-element array into which the current position will be copied.
	 */
	void									copyVector(double* u)
	{ u[0] = unitVector[0]; u[1] = unitVector[1]; u[2] = unitVector[2]; };

	/**
	 * Retrieve the index of the triangle within which the current position is located
	 *
	 * @return the index of the triangle within which the current position is located
	 */
	int										getTriangle() { return triangle[tessid]; };

	/**
	 * Return the number of vertices (3 for LINEAR interpolation, more for natural neighbor).
	 * @return number of vertices involved in horizontal interpolation.
	 */
	int										getNVertices() { return vertices[tessid].size(); };

	/**
	 * Return a reference to the array containing the indexes of vertices in the 2D grid
	 * that will be involved in the interpolation of data.
	 *
	 * @return vector<int>& a reference to the array containing the indexes of vertices in the 2D grid
	 * that will be involved in the interpolation of data.
	 */
	const vector<int>& getVertices() const { return vertices[tessid]; };

	/**
	 * Retrieve the index of the highest interpolation coefficient.
	 * @return the index of the vertex that is closest to this
	 * position.
	 */
	int getIndexOfClosestVertex() const;

	/**
	 * Retrieve the unit vector of the vertex wiht the highest
	 * interpolation coefficient.
	 * @return a reference to the vertex that is closest to this
	 * position.
	 */
	const double*         getClosestVertex() const
	{
		return grid.getVertex(getIndexOfClosestVertex());
	}

	/**
	 * Return the index of one of the vertices used to interpolate data.
	 *
	 * @param index the index of the desired coefficient.
	 * @return the index of one of the vertices used to interpolate data.
	 */
	int										getVertex(int index)
	{	return vertices[tessid][index]; }

	/**
	 * Retrieve a map from pointIndex to interpolation coefficient. The
	 * returned coefficients sum to one.
	 * @param coefficients HashMap<Integer, Double> from pointIndex to interpolation coefficient
	 */
	void	getCoefficients(map<int, double>& coefficients)
	{
		coefficients.clear();
		vector<int>& vtx = vertices[tessid];
		vector<double>& hc = hCoefficients[tessid];
		for (int i = 0; i < (int) vtx.size(); ++i)
			modlProfiles[vtx[i]][layerId]->getCoefficients(coefficients, radius, hc[i]);
	}

	/**
	 * Set the maximum tessellation level such that the triangle that is found
	 * during a walking triangle search will be on a tessellation level that is
	 * no higher than the specified value. Default value is Integer.maxValue.
	 *
	 * @param layid
	 * @param maxTess
	 *
	 */
	void									setMaxTessLevel(int layid, int maxTess)
	{
		maxTessLevel[layerTessIds[layid]] = maxTess;
		triangle[layerTessIds[layid]] = -1;
		checkTessellation(tessid);
	}

	/**
	 * Retrieve the current value of maxTessLevel, which is the maximum
	 * tessellation level such that the triangle that is found during a
	 * walking triangle search will be on a tessellation level that is no
	 * higher than the specified value. Default value is Integer.maxValue-1.
	 *
	 * @param layid
	 * @return current value of maxTessLevel
	 */
	int										getMaxTessLevel(int layid)
	{	return maxTessLevel[layerTessIds[layid]]; }

	/**
	 * Retrieve the index of the tessellation level of the triangle that was found the last time
	 * that the walinkg triangle algorithm was executed.
	 *
	 * @return index of current tessellation level
	 */
	int										getTessLevel() const { return tessLevels[tessid]; };

	/**
	 * Retrieve the index of the tessellation level of the triangle that was
	 * found the last time that the walking triangle algorithm was executed.
	 *
	 * @param tId tessellation index
	 * @return index of current tessellation level, relative to the first
	 *         tessellation level in the current tessellation
	 */
	int	getTessLevel(const int& tId) { checkTessellation(tId); return tessLevels[tId]; }

	/**
	 * Retrieve an interpolated value of the radius of the top of the current layer.
	 *
	 * @return interpolated value of the radius of the top of the specified layer.
	 */
	double								getRadiusTop()
	{	return getRadiusTop(layerId); };

	/**
	 * Retrieve an interpolated value of the radius of the bottom of the current layer.
	 *
	 * @return an interpolated value of the radius of the bottom of the specified layer.
	 */
	double								getRadiusBottom()
	{	return getRadiusBottom(layerId); };

	/**
	 * Retrieve an interpolated value of the depth of the top of the current layer.
	 *
	 * @return interpolated value of the depth of the top of the current layer.
	 */
	double								getDepthTop()
	{
		return	getEarthRadius() - getRadiusTop(layerId);
	}

	/**
	 * Retrieve an interpolated value of the depth of the bottom of the current layer.
	 *
	 * @return interpolated value of the depth of the bottom of the current layer.
	 */
	double								getDepthBottom()
	{
		return	getEarthRadius() - getRadiusBottom(layerId);
	}

	/**
	 * Retrieve an interpolated value of the depth of the top of the current layer.
	 *
	 * @param layid
	 * @return interpolated value of the depth of the top of the current layer.
	 */
	double								getDepthTop(int layid)
	{
		return	getEarthRadius() - getRadiusTop(layid);
	}

	/**
	 * Retrieve an interpolated value of the depth of the bottom of the current layer.
	 *
	 * @param layid
	 * @return interpolated value of the depth of the bottom of the current layer.
	 */
	double								getDepthBottom(int layid)
	{
		return	getEarthRadius() - getRadiusBottom(layid);
	}

	/**
	 * Retrieve the thickness of specified layer, in km.
	 *
	 * @param layid layer index
	 * @return the thickness of specified layer, in km.
	 */
	double								getLayerThickness(int layid)
	{
		return	getRadiusTop(layid) - getRadiusBottom(layid);
	}

	/**
	 * Retrieve the thickness of specified layer, in km.
	 *
	 * @return the thickness of specified layer, in km.
	 */
	double								getLayerThickness()
	{
		return	getRadiusTop() - getRadiusBottom();
	}

	/**
	 * Retrieve the radius of the current position, in km.
	 *
	 * @return the radius of the current position, in km.
	 */
	double								getRadius() { return radius; };

	/**
	 * Retrieve the depth of the current position in km. Assumes GRS80 ellipsoid.
	 *
	 * @return the depth of the current position in km.
	 */
	double								getDepth()
	{	return getEarthRadius() - radius; };

	/**
	 * @return a pointer to the current model
	 */
	GeoTessModel* getModel() { return model; }

	/**
	 * @return the tessID
	 */
	int	getTessID() { return tessid; };

	/**
	 * Retrieve the index of the layer that contains the specified radius.
	 * If radius is less than bottom of model, returns 0.  If radius
	 * greater than top of model, returns index of shallowest layer that
	 * has finite thickness.  If radius greater than top of model and
	 * all layers have zero thickness, returns nLayers-1.
	 * @param rad radius in km
	 * @return
	 *
	 */
	int										getLayerId(double rad)
	{
		for (int i=0; i<nLayers; ++i)
			if (rad <= getRadiusTop(i))
				return i;

		for (int i=nLayers-1; i>=0; --i)
			if (getLayerThickness(i) > 0)
				return i;

		return nLayers-1;
	};

	/**
	 * @return the layerId
	 */
	int										getLayerId() { return layerId; };

	/**
	 * Returns position as a string.
	 * @return string
	 */
	string								toString();

	/**
	 * If any calculated value is Double.NaN, then functions like getValue() or
	 * getRadiusTop() or getRadiusBottom() will return this errorValue. The
	 * default is NaN, but it can be set by calling setErrorValue();
	 * @return errorValue
	 */
	double								getErrorValue()
	{
		return errorValue;
	}

	/**
	 * If any calculated value is NaN, then functions like getValue() or
	 * getRadiusTop() or getRadiusBottom() will return this value. The default
	 * is NaN, but it can be set by calling this function.
	 * @param errVal new value for errVal
	 */
	void									setErrorValue(double errVal)
	{
		errorValue = errVal;
	}

	/**
	 * If the position of this GeoTessPosition object is currently set to a
	 * location that coincides with one of the grid vertices, return the index
	 * of that vertex. Otherwise return -1;
	 *
	 * @return index of colocated vertex or -1.
	 */
	int									  getVertexIndex()
	{
		vector<int>& vtid = vertices[tessid];
		vector<double>& htid = hCoefficients[tessid];
		for (int v = 0; v < (int) vtid.size(); ++v)
			if (htid[v] > 0.999999999)
				return vtid[v];
		return -1;
	}

	/**
	 * Add the weights of the current interpolation position to the supplied map of weights.
	 * Weights is a map from a pointIndex to the 'weight' associated with
	 * that point.  For an individual point, the weight associated with that
	 * point is the product of the interpoation coefficient times the supplied
	 * value of dkm.
	 * <p>
	 * <ol>
	 * <li>Consider all the points in the model that contribute interpolation
	 * coefficients to the position where this GeoTessPosition object is
	 * currently located.
	 * <li>For each of those points,  look in the supplied weights and see
	 * if there is already an entry for the pointIndex.
	 * <li>If there is no entry for the current pointIndex, add an
	 * entry with weight = interpolation coefficient * dkm.
	 * <li>If there is already an entry for the current pointIndex,
	 * add interpolation coefficient * dkm to the weight that is
	 * currently associated with the pointIndex.
	 * </ol>
	 *
	 * @param weights a map from a point index to the weight assigned
	 * to that point.
	 * @param dkm the 'length' in km associated with each interpolation position.
	 */
	void getWeights(map<int, double>& weights, double dkm)
	{
		vector<int>& vtx = vertices[tessid];
		vector<double>& htid = hCoefficients[tessid];
		model->getPointMap();
		for (int i = 0; i < (int) vtx.size(); ++i)
			modlProfiles[vtx[i]][layerId]->getWeights(weights, dkm, radius, htid[i]);
	}

	/**
	 * Retrieve a reference to the horizontal interpolation coefficients associated with
	 * the vertices of the tessellation used to interpolate data.
	 *
	 * @return a reference to the horizontal interpolation coefficients associated with the
	 *         vertices of the tessellation used to interpolate data.
	 */
	const vector<double>& getHorizontalCoefficients() const
	{
		return hCoefficients[tessid];
	}

	/**
	 * Retrieve the interpolation coefficient associated with one of the
	 * vertices of the tessellation used to interpolate data.
	 *
	 * @param index
	 *            the index of the desired coefficient.
	 * @return one of the interpolation coefficients
	 */
	double getHorizontalCoefficient(int index) const
	{
		return hCoefficients[tessid][index];
	}

	/**
	 * Controls radius out-of-range behavior.  If position or radius is
	 * set with a specified layer index, and a radius is specified that
	 * is outside the bounds of that layer, and radiusOutOfRangeAllowed
	 * is true, then interpolated values will be computed using the values
	 * from the top or bottom of the specified layer as appropriate.
	 * If radiusOutOfRangeAllowed is false then errorValue is returned.
	 * <p>If position or radius is
	 * set without a specified layer index, and a radius is specified that
	 * is above the surface of the Earth, and radiusOutOfRangeAllowed
	 * is true, then interpolated values will be computed using the values
	 * from the top of the shallowest layer that has finite thickness.
	 * If radiusOutOfRangeAllowed is false then errorValue is returned.
	 * @return current setting of radiusOutOfRangeAllowed.
	 */
	bool isRadiusOutOfRangeAllowed()
	{
		return radiusOutOfRangeAllowed;
	}

	/**
	 * Controls radius out-of-range behavior.  If position or radius is
	 * set with a specified layer index, and a radius is specified that
	 * is outside the bounds of that layer, and radiusOutOfRangeAllowed
	 * is true, then interpolated values will be computed using the values
	 * from the top or bottom of the specified layer as appropriate.
	 * If radiusOutOfRangeAllowed is false then errorValue is returned.
	 * <p>If position or radius is
	 * set without a specified layer index, and a radius is specified that
	 * is above the surface of the Earth, and radiusOutOfRangeAllowed
	 * is true, then interpolated values will be computed using the values
	 * from the top of the shallowest layer that has finite thickness.
	 * If radiusOutOfRangeAllowed is false then errorValue is returned.
	 * @param allowed the new value for radiusOutOfRangeAllowed
	 */
	void setRadiusOutOfRangeAllowed(bool allowed)
	{
		if (allowed != radiusOutOfRangeAllowed)
			clearRadialCoefficients();

		radiusOutOfRangeAllowed = allowed;
	}

	/**
	 * Add reference count;
	 */
	void addReference() { ++refCount; }

	/**
	 * Remove reference count;
	 */
	void removeReference()
	{
		if (isNotReferenced())
		{
			ostringstream os;
			os << endl << "ERROR in Polygon::removeReference" << endl
					<< "Reference count (" << refCount << ") is already zero." << endl;
			throw GeoTessException(os, __FILE__, __LINE__, 3005);
		}

		--refCount;
	}

	/**
	 * Retrieve reference count - number of other objects that hold
	 * a reference to this position object.
	 * @return current reference count
	 */
	int getReferenceCount()
	{
		return refCount;
	}

	/**
	 * Returns true if reference count is zero.
	 * @return  true if reference count is zero.
	 */
	bool isNotReferenced() { return refCount == 0; }

}; // end class GeoTessModel

} // end namespace geotess

#endif  // GEOTESSPOSITION_OBJECT_H
