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

#include "GeoTessUtils.h"
#include "GeoTessProfileNPoint.h"
#include "GeoTessProfileType.h"
#include "GeoTessMetaData.h"
#include "IFStreamAscii.h"
#include "IFStreamBinary.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************


// **** _STATIC INITIALIZATIONS_************************************************

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

/**
 * Standard constructor.
 * Input radii and data values are copied from the input
 * variables into the new Profile object.
 *
 * @param rad Input array of radii.
 * @param dat Input array of Data objects.
 * @param n   Size of arrays.
 */
GeoTessProfileNPoint::GeoTessProfileNPoint(float* rad, const vector<GeoTessData*>& dat)
: GeoTessProfile(), nNodes(dat.size()), radii(NULL), data(NULL), y2(NULL), pointIndices(NULL)
{
  if (dat.size() < 2)
	{
		ostringstream os;
		os << endl << "ERROR in ProfileNPoint::ProfileNPoint" << endl
			 << "Input array entries must be > 2 ... Defined as "
			 << dat.size() << "." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 4305);
	}

  radii = new float[nNodes];
  data = new GeoTessData* [nNodes];
  for (int i = 0; i < nNodes; ++i)
  {
	  radii[i] = rad[i];
	  data[i] = dat[i];
  }
}

/**
 * Instantiates a new object from the provided file input stream.
 */
GeoTessProfileNPoint::GeoTessProfileNPoint(IFStreamAscii& ifs, GeoTessMetaData& gtmd)
: GeoTessProfile(), y2(NULL), pointIndices(NULL)
{
	// layer with 2 or more radii and one data object for each radius

	nNodes = ifs.readInteger();
	radii = new float [nNodes];
	data = new GeoTessData* [nNodes];
	for (int k = 0; k < nNodes; ++k)
	{
		radii[k] = ifs.readFloat();
		data[k] = GeoTessData::getData(ifs, gtmd);
	}
}

/**
 * Instantiates a new object from the provided file input stream.
 */
GeoTessProfileNPoint::GeoTessProfileNPoint(IFStreamBinary& ifs, GeoTessMetaData& gtmd)
: GeoTessProfile(), y2(NULL), pointIndices(NULL)
{
	// layer with 2 or more radii and one data object for each radius

	nNodes = ifs.readInt();
	radii = new float [nNodes];
	data = new GeoTessData* [nNodes];
	for (int k = 0; k < nNodes; ++k)
	{
		radii[k] = ifs.readFloat();
		data[k] = GeoTessData::getData(ifs, gtmd);
	}
}

/**
 * Destructor. Note that all Profile objects delete their data (and in this
 * case their radii). Data was created by input functionality.
 */
GeoTessProfileNPoint::~GeoTessProfileNPoint()
{
  // delete y2 if it was defined

	if (y2 != NULL)
	{
		for (int i = 0; i < data[0]->size(); ++i) delete [] y2[i];
		delete [] y2;
	}

	// delete arrays (and Data elements) if they are not null

	if (radii != NULL)
	{
		delete [] radii;
		for (int k = 0; k < nNodes; ++k) delete data[k];
		delete [] data;
	}

	// delete pointIndices if it was instantiated

	if (pointIndices != NULL)
		delete [] pointIndices;
}

/**
 * Resets the data object to the new input data
 */
void GeoTessProfileNPoint::setData(const vector<GeoTessData*>& inData)
{
  if ((int) inData.size() != nNodes)
  {
		ostringstream os;
		os << endl << "ERROR in ProfileNPoint::setData" << endl
				 << "Input data array length != nRadii" << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 4306);
  }

  // delete old data and assign new data

	for (int k = 0; k < nNodes; ++k)
	{
		delete data[k];
		data[k] = inData[k];
	}
}

/**
 * Writes this ProfileNPoint object to the provided output file stream.
 */
void GeoTessProfileNPoint::write(IFStreamBinary& ofs)
{
	ofs.writeByte((byte) GeoTessProfileType::NPOINT.ordinal());
	ofs.writeInt(nNodes);
	for (int i = 0; i < nNodes; ++i)
	{
		ofs.writeFloat(radii[i]);
		data[i]->write(ofs);
	}
}

/**
 * Writes this ProfileNPoint object to the provided output file stream.
 */
void GeoTessProfileNPoint::write(IFStreamAscii& ofs)
{
	ofs.writeInt(GeoTessProfileType::NPOINT.ordinal());
	ofs.writeString(" ");
	ofs.writeInt(nNodes);
	ofs.writeNL();
	for (int i = 0; i < nNodes; ++i)
	{
		ofs.writeFloat(radii[i]);
		data[i]->write(ofs);
		ofs.writeNL();
	}
}

/**
 * Defines the second derivatives at the input attribute index for all
 * radii in x using a cubic spline.
 */
double*	GeoTessProfileNPoint::spline(float* x, GeoTessData** y, int attributeIndex,
															double yp1, double ypn) const
{
	int i, k;
	double p, qn, sig, un;

	int n = nNodes;
	double* y2drv = new double [n];
	double* u  = new double [n - 1];

	if (yp1 > 0.99e30)
		y2drv[0] = u[0] = 0.0;
	else
	{
		y2drv[0] = -0.5;
		u[0] = (3.0 / (x[1] - x[0]))
				* ((y[1]->getDouble(attributeIndex) - y[0]->getDouble(attributeIndex))
						/ (x[1] - x[0]) - yp1);
	}
	for (i = 1; i < n - 1; i++)
	{
		sig = (x[i] - x[i - 1]) / (x[i + 1] - x[i - 1]);
		p = sig * y2drv[i - 1] + 2.0;
		y2drv[i] = (sig - 1.0) / p;
		u[i] =	(y[i + 1]->getDouble(attributeIndex) - y[i]->getDouble(attributeIndex)) /
						(x[i + 1] - x[i]) -
						(y[i]->getDouble(attributeIndex) - y[i - 1]->getDouble(attributeIndex)) /
						(x[i] - x[i - 1]);
		u[i] = (6.0 * u[i] / (x[i + 1] - x[i - 1]) - sig * u[i - 1]) / p;
	}
	if (ypn > 0.99e30)
		qn = un = 0.0;
	else
	{
		qn =	0.5;
		un =	(3.0 / (x[n - 1] - x[n - 2])) *
					(ypn - (y[n - 1]->getDouble(attributeIndex) -
									y[n - 2]->getDouble(attributeIndex)) /
									(x[n - 1] - x[n - 2]));
	}
	y2drv[n - 1] = (un - qn * u[n - 2]) / (qn * y2drv[n - 2] + 1.0);
	for (k = n - 2; k >= 0; --k) y2drv[k] = y2drv[k] * y2drv[k + 1] + u[k];

	delete [] u;
	return y2drv;
}

/**
 * find interpolation coefficient using linear interpolation.
 *
 * @param index
 *            the index in r such that radius is between r[index] and
 *            r[index+1].
 * @param radius
 *            the radius whose interpolation coefficient is desired.
 * @return c[index], the interpolation coefficient to be applied at
 *         r[index]. The interpolation coefficient for r[index+1] is
 *         1-c[index].
 */
double GeoTessProfileNPoint::getInterpolationCoefficient(int index, double radius,
		 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	 	  bool allowOutOfRange) const
{
	//TODO:  need this method to work for cubic spline interpolator.  It currently does not.
	if (!allowOutOfRange && (radius < radii[0] || radius > radii[nNodes-1]))
		return NaN_DOUBLE;

	if (radius <= radii[index])
		return 1.;
	if (radius >= radii[index + 1])
		return 0.;
	return ((double)radii[index + 1] - radius) /
			 	 ((double)radii[index + 1] - (double)radii[index]);
}

/**
 * Find index i such that x is >= xx[i] and < xx[i+1]. If x < xx[1] returns
 * 0. If x >= xx[xx.length-2] return xx.length-2.
 * <p>
 * This method is translation from Numerical Recipes in C++.
 *
 * @param radius
 * @param jlo			Initial estimate of desired index. If jlo < 0 or jlo >
 *                xx.length-2, then jlo is ignored and search proceeds
 *                directly to binary search phase.
 */
int	GeoTessProfileNPoint::getRadiusIndex(double radius, int jlo) const
{
	int jm, jhi, inc;

  // exit with lower or upper limit if radius does not lie within the bounds
	// of the profile radii

	if (radius < radii[1]) return 0;
	if (radius >= radii[nNodes - 2]) return nNodes - 2;

	// sb. commented out ascnd because it is always true
	// in this application
	// boolean ascnd=(xx[n-1] >= xx[0]);
	if (jlo < 0 || jlo > nNodes - 2)
	{
		jlo = -1;
		jhi = nNodes;
	}
	else
	{
		inc = 1;
		if (radius >= radii[jlo] /* == ascnd */)
		{
			// if (jlo == nRadii-1) return jlo;
			jhi = jlo + 1;
			while (radius >= radii[jhi] /* == ascnd */)
			{
				jlo = jhi;
				inc += inc;
				jhi = jlo + inc;
				if (jhi > nNodes - 1)
				{
					jhi = nNodes;
					break;
				}
			}
		}
		else
		{
			// if (jlo == 0) return -1;
			jhi = jlo--;
			while (radius < radii[jlo] /* == ascnd */)
			{
				jhi = jlo;
				inc <<= 1;
				if (inc >= jhi)
				{
					jlo = -1;
					break;
				}
				else
					jlo = jhi - inc;
			}
		}
	}

	while (jhi - jlo != 1)
	{
		jm = (jhi + jlo) >> 1;
		if (radius >= radii[jm] /* == ascnd */)
			jlo = jm;
		else
			jhi = jm;
	}

	return jlo;
}

} // end namespace geotess
