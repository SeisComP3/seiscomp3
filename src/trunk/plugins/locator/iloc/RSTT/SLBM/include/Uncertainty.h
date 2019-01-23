//- ****************************************************************************
//- 
//- Copyright 2009 Sandia Corporation. Under the terms of Contract 
//- DE-AC04-94AL85000 with Sandia Corporation, the U.S. Government retains 
//- certain rights in this software.
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
//-
//- Program:       Uncertainty
//- Module:        $RCSfile: Uncertainty.h,v $
//- Revision:      $Revision: 1.24 $
//- Last Modified: $Date: 2013/08/14 13:21:39 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef Uncertainty_H
#define Uncertainty_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <string>
#include <vector>

#include "SLBMGlobals.h"
#include "DataBuffer.h"
#include "IFStreamAscii.h"
#include "IFStreamBinary.h"

using namespace std;

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _LOCAL INCLUDES_ *******************************************************

//!
//! \brief A Uncertainty object contains the raw data to calculate a modeling
//! error in seconds as a function of distance in radians.
//!
//! A Uncertainty object contains the raw data to calculate a modeling
//! error in seconds as a function of distance in radians.
//!
//! <p>Code includes functionality to store and compute uncertainty for
//! 2D uncertainty tables (distance and depth).  But all the tables included
//! with SLBM versions up to and including version 3.0 do not have any depth
//! information.  All uncertainties are a function of distance only.
//! Therefore, functionality to handle 2D uncertainty has not been tested and
//! it is considered unlikely that it will work as currently coded.
class SLBM_EXP Uncertainty
{

public:

	//! \brief Default constructor.
	//!
	//! Default constructor.
	Uncertainty();

	//! \brief Parameterized constructor that loads model error
	//! from a specified file.
	//!
	//! Parameterized constructor that loads model error data
	//! from a file. Uses an SLBM specific phase ordering index.
	Uncertainty(const int& phase, const int& attribute);

	//! \brief Parameterized constructor that loads model error
	//! from a specified file.
	//!
	//! Parameterized constructor that loads model error data
	//! from a file. Uses an SLBM specific phase ordering index.
	Uncertainty(const string& phase, const string& attribute);

	// \brief Parameterized constructor that loads model error
	//! from a specified file.
	//!
	//! Parameterized constructor that loads model error data
	//! from a file. Uses the input phase string to find the uncertainty
	//! data file and assigns a phase ordering index.
	Uncertainty(string modelPath, const string& phase, int phasenum);

	//! \brief Parameterized constructor that loads model error
	//! from a specified file.
	//!
	//! Parameterized constructor that loads model error data
	//! from a file. Uses an SLBM specific phase ordering index.
	Uncertainty(string modelPath, const int& phase, const int& attribute);

	//! \brief Parameterized constructor that loads uncertainty data
	//! from the input DataBuffer.
	//!
	//! Parameterized constructor that loads model error data
	//! from a file.
	Uncertainty(util::DataBuffer& buffer);

	//! \brief Copy constructor.
	//!
	//! Copy constructor.
	Uncertainty(const Uncertainty& u);

	/**
	 * Retrieve a new Uncertainty object for the specified phase and attribute,
	 * loaded from specified input source.
	 * @param input data source
	 * @param phase 0:Pn, 1:Sn, 2:Pg, 3:Lg
	 * @param attribute 0:TT, 1:SH, 2:AZ
	 * @return pointer to an Uncertainty object.  Will return null if the
	 * number of distances in the file is zero.
	 */
	static Uncertainty* getUncertainty(ifstream& input, const int& phase, const int& attribute);

	/**
	 * Retrieve a new Uncertainty object for the specified phase and attribute,
	 * loaded from specified input source.
	 * @param input data source
	 * @param phase 0:Pn, 1:Sn, 2:Pg, 3:Lg
	 * @param attribute 0:TT, 1:SH, 2:AZ
	 * @return pointer to an Uncertainty object.  Will return null if the
	 * number of distances in the file is zero.
	 */
	static Uncertainty* getUncertainty(ifstream& input, const string& phase, const string& attribute);

	/**
	 * Retrieve a new Uncertainty object for the specified phase and attribute,
	 * loaded from specified input source.
	 * @param input data source
	 * @param phase 0:Pn, 1:Sn, 2:Pg, 3:Lg
	 * @param attribute 0:TT, 1:SH, 2:AZ
	 * @return pointer to an Uncertainty object.  Will return null if the
	 * number of distances in the file is zero.
	 */
	static Uncertainty* getUncertainty(geotess::IFStreamAscii& input, const int& phase, const int& attribute);

	/**
	 * Retrieve a new Uncertainty object for the specified phase and attribute,
	 * loaded from specified input source.
	 * @param input data source
	 * @param phase 0:Pn, 1:Sn, 2:Pg, 3:Lg
	 * @param attribute 0:TT, 1:SH, 2:AZ
	 * @return pointer to an Uncertainty object.  Will return null if the
	 * number of distances in the file is zero.
	 */
	static Uncertainty* getUncertainty(geotess::IFStreamAscii& input, const string& phase, const string& attribute);

	/**
	 * Retrieve a new Uncertainty object for the specified phase and attribute,
	 * loaded from specified input source.
	 * @param input data source
	 * @param phase 0:Pn, 1:Sn, 2:Pg, 3:Lg
	 * @param attribute 0:TT, 1:SH, 2:AZ
	 * @return pointer to an Uncertainty object.  Will return null if the
	 * number of distances in the file is zero.
	 */
	static Uncertainty* getUncertainty(geotess::IFStreamBinary& input, const int& phase, const int& attribute);

	/**
	 * Retrieve a new Uncertainty object for the specified phase and attribute,
	 * loaded from specified input source.
	 * @param input data source
	 * @param phase 0:Pn, 1:Sn, 2:Pg, 3:Lg
	 * @param attribute 0:TT, 1:SH, 2:AZ
	 * @return pointer to an Uncertainty object.  Will return null if the
	 * number of distances in the file is zero.
	 */
	static Uncertainty* getUncertainty(geotess::IFStreamBinary& input, const string& phase, const string& attribute);

	/**
	 * Retrieve a new Uncertainty object for the specified phase and attribute,
	 * loaded from specified input source.
	 * @param input data source
	 * @param phase 0:Pn, 1:Sn, 2:Pg, 3:Lg
	 * @param attribute 0:TT, 1:SH, 2:AZ
	 * @return pointer to an Uncertainty object.  Will return null if the
	 * number of distances in the file is zero.
	 */
	static Uncertainty* getUncertainty(util::DataBuffer& input, const int& phase, const int& attribute);

	/**
	 * Retrieve a new Uncertainty object for the specified phase and attribute,
	 * loaded from specified input source.
	 * @param input data source
	 * @param phase 0:Pn, 1:Sn, 2:Pg, 3:Lg
	 * @param attribute 0:TT, 1:SH, 2:AZ
	 * @return pointer to an Uncertainty object.  Will return null if the
	 * number of distances in the file is zero.
	 */
	static Uncertainty* getUncertainty(util::DataBuffer& input, const string& phase, const string& attribute);

	/**
	 * Retrieve a new Uncertainty object for the specified phase and attribute,
	 * loaded from specified input source.
	 * @param directoryName data source
	 * @param phase 0:Pn, 1:Sn, 2:Pg, 3:Lg
	 * @param attribute 0:TT, 1:SH, 2:AZ
	 * @return pointer to an Uncertainty object.  Will return null if the
	 * number of distances in the file is zero.
	 */
	static Uncertainty* getUncertainty(const string& directoryName, const int& phase, const int& attribute);

	//! \brief Destructor.
	//!
	//! Destructor.
	~Uncertainty();

	//! \brief Assignment operator.
	//!
	//! Assignment operator.
	Uncertainty& operator=(const Uncertainty& u);

	/**
	 * Overloaded equality operator
	 * @param other reference to the other Uncertainty object to which
	 * this Uncertainty object is to be compared
	 * @return true if this and other are equal.
	 */
	bool operator==(const Uncertainty& other);

	/**
	 * Overloaded inequality operator
	 * @param other reference to the other Uncertainty object to which
	 * this Uncertainty object is to be compared
	 * @return true if this and other are not equal.
	 */
	bool operator!=(const Uncertainty& other) { return !(*this == other); }

	void readFile(ifstream& fin);

	void readFile(geotess::IFStreamAscii& fin);

	void readFile(geotess::IFStreamBinary& fin);

	void writeFile(geotess::IFStreamBinary& fout);

	//! \brief A public convenience accessor used to verify the error data for the 
	//! correct model phase is loaded in memory.
	//!
	//! A public convenience accessor used to verify the error data for the 
	//! correct model phase is loaded in memory.
	int getPhase();

	//! \brief A public convenience accessor used to verify the error data for the 
	//! correct model phase is loaded in memory.
	//!
	//! A public convenience accessor used to verify the error data for the 
	//! correct model phase is loaded in memory.
	string getPhaseStr();

	//! \brief A public convenience accessor used to verify the error data for the
	//! correct model phase is loaded in memory.
	//!
	//! A public convenience accessor used to verify the error data for the
	//! correct model phase is loaded in memory.
	int getAttribute();

	//! \brief A public convenience accessor used to verify the error data for the
	//! correct model phase is loaded in memory.
	//!
	//! A public convenience accessor used to verify the error data for the
	//! correct model phase is loaded in memory.
	string getAttributeStr();

	//! \brief Returns the model uncertainty as a function of angular distance 
	//! (radians) and depth.
	//!
	//! Returns the model uncertainty as a function of angular distance 
	//! (radians) and depth. Depth defaults to the surface (0.0).
	double getUncertainty(const double& distance, double depth = 0.0);

	//! \brief Returns the model variance as a function of angular distance 
	//! (radians) and depth.
	//!
	//! Returns the model variance as a function of angular distance 
	//! (radians) and depth. Depth defaults to the surface (0.0).
	double getVariance(const double& distance, double depth = 0.0);

	//! \brief A vector of doubles representing the angular distances (in degrees)
	// for which the model errors are defined.
	//!
	//! A vector of doubles representing the angular distances (in degrees)
	// for which the model errors are defined.
	vector<double>& getDistances() {return errDistances; };

	//! \brief A vector of doubles representing the depths
	// for which the model errors are defined, in km.
	//!
	//! A vector of doubles representing the depths
	// for which the model errors are defined, in km.
	vector<double>& getDepths() { return errDepths; }

	//! \brief A vector of doubles representing the model errors (in seconds)
	// corresponding to the defined depths (first index) and angular distances.
	//!
	//! A vector of doubles representing the model errors (in seconds)
	// corresponding to the defined depths (first index) and angular distances.
	vector<vector<double> >& getValues() { return errVal; }

	void writeFile(const string& directoryName);

	string toStringTable();

	string toStringFile();

	static string getPhase(const int& phaseIndex)
	{
		switch(phaseIndex)
		{
		case 0:
			return "Pn";
		case 1:
			return "Sn";
		case 2:
			return "Pg";
		case 3:
			return "Lg";
		default:
			return "XX";
		}
	}

	static string getAttribute(const int& attributeIndex)
	{
		switch(attributeIndex)
		{
		case 0:
			return "TT";
		case 1:
			return "Sh";
		case 2:
			return "Az";
		default:
			return "XX";
		}
	}

	static int getPhase(const string& phase)
	{
		if (phase == "Pn") return Pn;
		if (phase == "Sn") return Sn;
		if (phase == "Pg") return Pg;
		if (phase == "Lg") return Lg;
		return -1;
	}

	static int getAttribute(const string& attribute)
	{
		if (attribute == "TT") return TT;
		if (attribute == "Sh") return SH;
		if (attribute == "Az") return AZ;
		return -1;
	}

	//! \brief Returns the model uncertainty DataBuffer size storage requirement.
	//!
	//! Returns the model uncertainty DataBuffer size storage requirement.
	int getBufferSize();

	//! \brief Writes the uncertainty object into the input DataBuffer.
	//!
	//! Writes the uncertainty object into the input DataBuffer.
	void serialize(util::DataBuffer& buffer);

	//! \brief Reads the uncertainty object from the input DataBuffer.
	//!
	//! Reads the uncertainty object from the input DataBuffer.
	void deserialize(util::DataBuffer& buffer);

	//	/**
	//	 * Retrieve reference count;
	//	 */
	//	int getReferenceCount() { return refCount; }
	//
	//	/**
	//	 * Add reference count;
	//	 */
	//	void addReference() { ++refCount; }
	//
	//	/**
	//	 * Remove reference count.
	//	 * @return the reference count after decrement.
	//	 */
	//	int removeReference();
	//
	//	/**
	//	 * Returns true if reference count is zero.
	//	 */
	//	bool isNotReferenced() { return (refCount == 0); }

private:

	//int refCount;

	//! \brief Returns the distance interpolated uncertainty at depth,
	//! \em idepth, and fraction \em f between distance indices \em idist
	//! and idist+1.
	//!
	//! Returns the distance interpolated uncertainty at depth,
	//! \em idepth, and fraction \em f between distance indices \em idist
	//! and \em idist+1. For \em f = 0 errVal[idepth][idist] is returned.
	//! For \em f = 1 errVal[idepth][idist+1] is returned
	double getUncertainty(double f, int idist, int idepth);

	//! \brief Returns the distance interpolated variance at depth,
	//! \em idepth, and fraction \em f between distance indices \em idist
	//! and idist+1.
	//!
	//! Returns the distance interpolated variance at depth,
	//! \em idepth, and fraction \em f between distance indices \em idist
	//! and \em idist+1. For \em f = 0 errVal[idepth][idist]^2 is returned.
	//! For \em f = 1 errVal[idepth][idist+1]^2 is returned
	double getVariance(double f, int idist, int idepth);

	//! \brief Private function used to find the bracketing index for performing
	//! uncertainty / variance distance interpolation.
	//!
	//! Private function used to find the bracketing index for performing
	//! uncertainty / variance distance interpolation. The bracketing \em index
	//! is found in vector \em v such that
	//
	//!    \em v[index] <= \em x <= \em v[index+1].
	//
	//! The weight \em w is calculated as
	//
	//!    \em w = (x - v[index]) / (v[index+1] - v[index]);
	void getIndex(double x, const vector<double>& v, int& index, double& w);

	//! \brief A function called by the constructor to read model error data
	//! from an ASCII text file.
	//!
	//! \brief A function called by the constructor to read model error data
	//! from an ASCII text file.
	void readFile(const string& filename);

	//! \brief The path and filename for the currently loaded seismic phase
	//! modeling error file.
	//!
	//! The path and filename for the currently loaded seismic phase
	//! modeling error file.
	string fname;

	//! \brief The seismic phase for which modeling error data was loaded.
	//!
	//! The seismic phase for which modeling error data was loaded.
	int phaseNum;

	//! \brief The attribute (TT, Sh or Az) for which modeling error
	//! data was loaded.
	//!
	//! The attribute (TT, Sh or Az) for which modeling error
	//! data was loaded.
	int attributeNum;

	//! \brief A vector of doubles representing the angular distances (in degrees)
	// for which the model errors are defined.
	//!
	//! A vector of doubles representing the angular distances (in degrees)
	// for which the model errors are defined.
	vector<double> errDistances;

	//! \brief A vector of doubles representing the depths
	// for which the model errors are defined.
	//!
	//! A vector of doubles representing the depths
	// for which the model errors are defined.
	vector<double> errDepths;

	//! \brief A vector of doubles representing the model errors (in seconds)
	// corresponding to the defined depths (first index) and angular distances.
	//!
	//! A vector of doubles representing the model errors (in seconds)
	// corresponding to the defined depths (first index) and angular distances.
	vector<vector<double> > errVal;

};

inline int Uncertainty::getPhase()
{
	return phaseNum;
}

inline int Uncertainty::getAttribute()
{
	return attributeNum;
}

inline string Uncertainty::getPhaseStr()
{
	return getPhase(phaseNum);
}

inline string Uncertainty::getAttributeStr()
{
	return getAttribute(attributeNum);
}

inline double Uncertainty::getUncertainty(double f, int idist, int idepth)
{
	return (f * (errVal[idepth][idist+1] - errVal[idepth][idist]) +
			errVal[idepth][idist]);
}

inline double Uncertainty::getVariance(double f, int idist, int idepth)
{
	return (f * (errVal[idepth][idist+1] * errVal[idepth][idist+1] -
			errVal[idepth][idist] * errVal[idepth][idist]) +
			errVal[idepth][idist] * errVal[idepth][idist]);
}

} // end slbm namespace

#endif // Uncertainty_H
