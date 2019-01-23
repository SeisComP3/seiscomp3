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

#include <sstream>

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessException.h"
#include "GeoTessProfile.h"
#include "GeoTessProfileEmpty.h"
#include "GeoTessProfileSurfaceEmpty.h"
#include "GeoTessProfileThin.h"
#include "GeoTessProfileConstant.h"
#include "GeoTessProfileSurface.h"
#include "GeoTessProfileNPoint.h"
#include "IFStreamAscii.h"
#include "IFStreamBinary.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess
{

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

int GeoTessProfile::aClassCount = 0;

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

/**
 * Static factory method that instantiates a new Profile object of the appropriate
 * type.
 * <br> If nRadii  > 0 and nData == 0, creates a ProfileEmpty object.
 * <br> If nRadii == 1 and nData == 1, creates a ProfileThin object.
 * <br> If nRadii == 2 and nData == 1, creates a ProfileConstant object.
 * <br> If nRadii >= 2 and nData == nRadii, creates a ProfileNPoint object.
 * <br> If nRadii == 0 and nData == 1, creates a ProfileSurface object.
 *
 *<p>Input radii and data values are copied from the input
 * variables into the new Profile object.
 *
 * @param radii an array of zero or more radius values, in km.  Values must be
 * monotonically increasing.
 * @param nRadii the length of the radii array.
 * @param data an array of pointers to Data objects.
 * @param the length of the data array
 * @return a new Profile object
 * @throws GeoTessException
 */
GeoTessProfile* GeoTessProfile::newProfile(float* radii, const int& nRadii, GeoTessData** data, const int& nData)
{
	if (nRadii > 0 && nData == 0)
		// EMPTY layer defined by two radii and no data
		return new GeoTessProfileEmpty(radii[0], radii[nRadii-1]);
	if (nRadii == 1 && nData == 1)
		// THIN layer defined by one radius and one data
		return new GeoTessProfileThin(radii[0], data[0]);
	if (nRadii == 2 && nData == 1)
		// CONSTANT layer defined by two radii and one data object
		return new GeoTessProfileConstant(radii[0], radii[1], data[0]);
	if (nRadii >= 2 && nData == nRadii)
		// NPOINT layer with 2 or more radii and one data object for each
		// radius
		return new GeoTessProfileNPoint(radii, data, nRadii);
	if (nRadii == 0 && nData == 1)
		// SURFACE layer with 0 radii and one data object
		return new GeoTessProfileSurface(data[0]);

	ostringstream os;
	os << endl << "ERROR in Profile::newProfile" << endl
				<< "Cannot construct a Profile object with " << endl
				<< nRadii << " radii and "<< nData << " Data objects. " << endl
				<< " Options are (nRadii, nData) = (2,0), (1,1), (2,1), (0,1), (n>1, m=n)" << endl;
	throw GeoTessException(os, __FILE__, __LINE__, 4001);
}

/**
 * Static factory method that instantiates a new Profile object of the appropriate
 * type.
 * <br> If radii.size()  > 0 and data.size() == 0, creates a ProfileEmpty object.
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
GeoTessProfile* GeoTessProfile::newProfile(const vector<float>& radii, vector<GeoTessData*>& data)
{
	if (radii.size() > 0 && data.size() == 0)
		// EMPTY layer defined by two radii and no data
		return new GeoTessProfileEmpty(radii[0], radii[radii.size()-1]);
	if (radii.size() == 1 && data.size() == 1)
		// THIN layer defined by one radius and one data
		return new GeoTessProfileThin(radii[0], data[0]);
	if (radii.size() == 2 && data.size() == 1)
		// CONSTANT layer defined by two radii and one data object
		return new GeoTessProfileConstant(radii[0], radii[1], data[0]);
	if (radii.size() >= 2 && data.size() == radii.size())
		// NPOINT layer with 2 or more radii and one data object for each
		// radius
		return new GeoTessProfileNPoint(radii, data);
	if (radii.size() == 0 && data.size() == 1)
		// SURFACE layer with 0 radii and one data object
		return new GeoTessProfileSurface(data[0]);

	ostringstream os;
	os << endl << "ERROR in Profile::newProfile" << endl
				<< "Cannot construct a Profile object with " << endl
				<< radii.size() << " radii and "<< data.size() << " Data objects. " << endl
				<< " Options are (nRadii, nData) = (2,0), (1,1), (2,1), (0,1), (n>1, m=n)" << endl;
	throw GeoTessException(os, __FILE__, __LINE__, 4002);
}

/**
 * Static factory method that instantiates a new Profile object of the appropriate type.
 * <br> If nRadii == 2 and nData == 0, creates a ProfileEmpty object.
 * <br> If nRadii == 1 and nData == 1, creates a ProfileThin object.
 * <br> If nRadii == 2 and nData == 1, creates a ProfileConstant object.
 * <br> If nRadii >= 2 and nData == nRadii, creates a ProfileNPoint object.
 * <br> If nRadii == 0 and nData == 1, creates a ProfileSurface object.
 *
 *<p>Input radii and data values are copied from the input
 * variables into the new Profile object.
 *
 * @param radii an array of zero or more radius values, in km.  Values must be
 * monotonically increasing.
 * @param 2D array of data values, nNodes x nAttributes.
 * @return a new Profile object
 * @throws GeoTessException
 */
GeoTessProfile* GeoTessProfile::newProfile(const vector<float>& radii, vector<vector<double> >& data)
{
	vector<GeoTessData*> d((int)data.size());
	for (int i=0; i<(int)data.size(); ++i) d[i] = GeoTessData::getData(data[i]);
	return GeoTessProfile::newProfile(radii, d);
}

/**
 * Static factory method that instantiates a new Profile object of the appropriate type.
 * <br> If nRadii == 2 and nData == 0, creates a ProfileEmpty object.
 * <br> If nRadii == 1 and nData == 1, creates a ProfileThin object.
 * <br> If nRadii == 2 and nData == 1, creates a ProfileConstant object.
 * <br> If nRadii >= 2 and nData == nRadii, creates a ProfileNPoint object.
 * <br> If nRadii == 0 and nData == 1, creates a ProfileSurface object.
 *
 *<p>Input radii and data values are copied from the input
 * variables into the new Profile object.
 *
 * @param radii an array of zero or more radius values, in km.  Values must be
 * monotonically increasing.
 * @param 2D array of data values, nNodes x nAttributes.
 * @return a new Profile object
 * @throws GeoTessException
 */
GeoTessProfile* GeoTessProfile::newProfile(const vector<float>& radii, vector<vector<float> >& data)
{
	vector<GeoTessData*> d((int)data.size());
	for (int i=0; i<(int)data.size(); ++i) d[i] = GeoTessData::getData(data[i]);
	return GeoTessProfile::newProfile(radii, d);
}

/**
 * Static factory method that instantiates a new Profile object of the appropriate type.
 * <br> If nRadii == 2 and nData == 0, creates a ProfileEmpty object.
 * <br> If nRadii == 1 and nData == 1, creates a ProfileThin object.
 * <br> If nRadii == 2 and nData == 1, creates a ProfileConstant object.
 * <br> If nRadii >= 2 and nData == nRadii, creates a ProfileNPoint object.
 * <br> If nRadii == 0 and nData == 1, creates a ProfileSurface object.
 *
 *<p>Input radii and data values are copied from the input
 * variables into the new Profile object.
 *
 * @param radii an array of zero or more radius values, in km.  Values must be
 * monotonically increasing.
 * @param 2D array of data values, nNodes x nAttributes.
 * @return a new Profile object
 * @throws GeoTessException
 */
GeoTessProfile* GeoTessProfile::newProfile(const vector<float>& radii, vector<vector<LONG_INT> >& data)
{
	vector<GeoTessData*> d((int)data.size());
	for (int i=0; i<(int)data.size(); ++i) d[i] = GeoTessData::getData(data[i]);
	return GeoTessProfile::newProfile(radii, d);
}

/**
 * Static factory method that instantiates a new Profile object of the appropriate type.
 * <br> If nRadii == 2 and nData == 0, creates a ProfileEmpty object.
 * <br> If nRadii == 1 and nData == 1, creates a ProfileThin object.
 * <br> If nRadii == 2 and nData == 1, creates a ProfileConstant object.
 * <br> If nRadii >= 2 and nData == nRadii, creates a ProfileNPoint object.
 * <br> If nRadii == 0 and nData == 1, creates a ProfileSurface object.
 *
 *<p>Input radii and data values are copied from the input
 * variables into the new Profile object.
 *
 * @param radii an array of zero or more radius values, in km.  Values must be
 * monotonically increasing.
 * @param 2D array of data values, nNodes x nAttributes.
 * @return a new Profile object
 * @throws GeoTessException
 */
GeoTessProfile* GeoTessProfile::newProfile(const vector<float>& radii, vector<vector<int> >& data)
{
	vector<GeoTessData*> d((int)data.size());
	for (int i=0; i<(int)data.size(); ++i) d[i] = GeoTessData::getData(data[i]);
	return GeoTessProfile::newProfile(radii, d);
}

/**
 * Static factory method that instantiates a new Profile object of the appropriate type.
 * <br> If nRadii == 2 and nData == 0, creates a ProfileEmpty object.
 * <br> If nRadii == 1 and nData == 1, creates a ProfileThin object.
 * <br> If nRadii == 2 and nData == 1, creates a ProfileConstant object.
 * <br> If nRadii >= 2 and nData == nRadii, creates a ProfileNPoint object.
 * <br> If nRadii == 0 and nData == 1, creates a ProfileSurface object.
 *
 *<p>Input radii and data values are copied from the input
 * variables into the new Profile object.
 *
 * @param radii an array of zero or more radius values, in km.  Values must be
 * monotonically increasing.
 * @param 2D array of data values, nNodes x nAttributes.
 * @return a new Profile object
 * @throws GeoTessException
 */
GeoTessProfile* GeoTessProfile::newProfile(const vector<float>& radii, vector<vector<short> >& data)
{
	vector<GeoTessData*> d((int)data.size());
	for (int i=0; i<(int)data.size(); ++i) d[i] = GeoTessData::getData(data[i]);
	return GeoTessProfile::newProfile(radii, d);
}

/**
 * Static factory method that instantiates a new Profile object of the appropriate type.
 * <br> If nRadii == 2 and nData == 0, creates a ProfileEmpty object.
 * <br> If nRadii == 1 and nData == 1, creates a ProfileThin object.
 * <br> If nRadii == 2 and nData == 1, creates a ProfileConstant object.
 * <br> If nRadii >= 2 and nData == nRadii, creates a ProfileNPoint object.
 * <br> If nRadii == 0 and nData == 1, creates a ProfileSurface object.
 *
 *<p>Input radii and data values are copied from the input
 * variables into the new Profile object.
 *
 * @param radii an array of zero or more radius values, in km.  Values must be
 * monotonically increasing.
 * @param 2D array of data values, nNodes x nAttributes.
 * @return a new Profile object
 * @throws GeoTessException
 */
GeoTessProfile* GeoTessProfile::newProfile(const vector<float>& radii, vector<vector<byte> >& data)
{
	vector<GeoTessData*> d((int)data.size());
	for (int i=0; i<(int)data.size(); ++i) d[i] = GeoTessData::getData(data[i]);
	return GeoTessProfile::newProfile(radii, d);
}


/**
 * Static factory method to load and return a new Proflie object from disk.
 */
GeoTessProfile* GeoTessProfile::newProfile(IFStreamBinary& ifs, GeoTessMetaData& gtmd)
{
	// read ProfileType and return new Profile object.

	int profileType = ifs.readByte();
	switch (profileType)
	{
	case 0:
		// EMPTY layer defined by two radii and no data
		return new GeoTessProfileEmpty(ifs);
	case 1:
		// THIN layer defined by one radius and one data
		return new GeoTessProfileThin(ifs, gtmd);
	case 2:
		// CONSTANT layer defined by two radii and one data object
		return new GeoTessProfileConstant(ifs, gtmd);
	case 3:
		// NPOINT layer with 2 or more radii and one data object for each
		// radius
		return new GeoTessProfileNPoint(ifs, gtmd);
	case 4:
		// SURFACE layer with 0 radii and one data object
		return new GeoTessProfileSurface(ifs, gtmd);
	case 5:
		// SURFACE layer with 0 radii and one data object
		return new GeoTessProfileSurfaceEmpty(ifs, gtmd);
	default:
		ostringstream os;
		os << endl << "ERROR in Profile::newProfile" << endl
		<< profileType << " is not a recognized ProfileType." << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 4003);
	}
}

/**
 * Static factory method to load and return a new Proflie object from disk.
 */
GeoTessProfile* GeoTessProfile::newProfile(IFStreamAscii& ifs, GeoTessMetaData& gtmd)
{
	// read ProfileType and return new Profile object.

	int profileType = -1;
	ifs.readInteger(profileType);
	switch (profileType)
	{
	case 0:
		// EMPTY layer defined by two radii and no data
		return new GeoTessProfileEmpty(ifs);
	case 1:
		// THIN layer defined by one radius and one data
		return new GeoTessProfileThin(ifs, gtmd);
	case 2:
		// CONSTANT layer defined by two radii and one data object
		return new GeoTessProfileConstant(ifs, gtmd);
	case 3:
		// NPOINT layer with 2 or more radii and one data object for each
		// radius
		return new GeoTessProfileNPoint(ifs, gtmd);
	case 4:
		// SURFACE layer with 0 radii and one data object
		return new GeoTessProfileSurface(ifs, gtmd);
	case 5:
		// SURFACE_EMPTY layer with 0 radii and 0 data
		return new GeoTessProfileSurfaceEmpty(ifs, gtmd);
	default:
		ostringstream os;
		os << endl << "ERROR in Profile::newProfile" << endl << "Unrecognized ProfileType "
				<< profileType << endl;
		throw GeoTessException(os, __FILE__, __LINE__, 4004);
	}
}

} // end namespace geotess
