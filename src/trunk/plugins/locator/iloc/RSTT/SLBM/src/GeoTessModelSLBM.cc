//- ****************************************************************************
//-
//- Copyright 2012 Sandia Corporation. Under the terms of Contract
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

// **** _SYSTEM INCLUDES_ ******************************************************

#include <sstream>
#include <cmath>

// **** _LOCAL INCLUDES_ *******************************************************

#include "GeoTessData.h"
#include "GeoTessProfile.h"
#include "GeoTessModel.h"
#include "GeoTessModelSLBM.h"
#include "IFStreamBinary.h"
#include "GeoTessInterpolatorType.h"
#include "SLBMException.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace slbm {

// **** _EXPLICIT TEMPLATE INSTANTIATIONS_ *************************************

// **** _STATIC INITIALIZATIONS_************************************************

// **** _FUNCTION IMPLEMENTATIONS_ *********************************************

GeoTessModelSLBM::GeoTessModelSLBM(vector<vector<Uncertainty*> >& uncert,
		const GeoTessOptimizationType* optimization)
: GeoTessModel(optimization), uncertainty(uncert), ioUncertainty(true)
{
	init();
}

GeoTessModelSLBM::GeoTessModelSLBM(const string& modelInputFile, const string& relativeGridPath,
		vector<vector<Uncertainty*> >& uncert, const GeoTessOptimizationType* optimization)
: GeoTessModel(optimization), uncertainty(uncert), ioUncertainty(true)
{
	init();
	loadModel(modelInputFile, relativeGridPath);
}

GeoTessModelSLBM::GeoTessModelSLBM(const string& modelInputFile,
		vector<vector<Uncertainty*> >& uncert, const GeoTessOptimizationType* optimization)
: GeoTessModel(optimization), uncertainty(uncert), ioUncertainty(true)
{
	init();
	loadModel(modelInputFile, ".");
}

GeoTessModelSLBM::GeoTessModelSLBM(const string& gridFileName, GeoTessMetaData* metaData,
		vector<vector<Uncertainty*> >& uncert, double* avgMantleVel)
: GeoTessModel(gridFileName, metaData), uncertainty(uncert), ioUncertainty(true)
{
	init();
	averageMantleVelocity[0] = avgMantleVel[0];
	averageMantleVelocity[1] = avgMantleVel[1];
}


GeoTessModelSLBM::GeoTessModelSLBM(GeoTessGrid* grid, GeoTessMetaData* metaData,
		vector<vector<Uncertainty*> >& uncert, double* avgMantleVel)
: GeoTessModel(grid, metaData), uncertainty(uncert), ioUncertainty(true)
{
	init();
	averageMantleVelocity[0] = avgMantleVel[0];
	averageMantleVelocity[1] = avgMantleVel[1];
}

/**
 * Destructor.
 */
GeoTessModelSLBM::~GeoTessModelSLBM()
{
}

void GeoTessModelSLBM::init()
{
	averageMantleVelocity[0]=averageMantleVelocity[1]=0;
}

void GeoTessModelSLBM::loadModelAscii(IFStreamAscii& input, const string& inputDirectory,
		const string& relGridFilePath)
{
	// read all the information from the standard GeoTessModel object, including the
	// model values of P and S velocity
	GeoTessModel::loadModelAscii(input, inputDirectory, relGridFilePath);

	string s;
	if (!input.readLine(s) || s != "SLBM")
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModelSLBM::loadModelAscii()." << endl <<
				"File " << getMetaData().getInputModelFile() << endl <<
				"While this file is a GeoTessModel file, it does not appear to be GeoTessModelSLBM file. Extra SLBM data is missing." << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),105);
	}
	int version = input.readInteger();
	if (version < 1 || version > 2)
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModelSLBM::loadModelAscii()." << endl <<
				"File " << getMetaData().getInputModelFile() << endl <<
				"SLBM IO Version number is " << version << " but that is not a supported version number." << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),105);
	}
	averageMantleVelocity[0] = input.readFloat();
	averageMantleVelocity[1] = input.readFloat();

	// read uncertainty information.
	// np is the number of phases (Pn, Sn, Pg, Lg); always 4
	// na is the number of attribute (TT, SH, AZ); always 3
	int np = input.readInteger();
	int na = input.readInteger();
	int phase, attribute;

	for (int p=0; p<np; ++p)
		for (int a=0; a<na; ++a)
		{
			phase = Uncertainty::getPhase(input.readString());
			attribute = Uncertainty::getAttribute(input.readString());
			uncertainty[phase][attribute] = Uncertainty::getUncertainty(input, phase, attribute);
		}



	if (version > 1)
	{
		float vn[2], vg[2];
		GeoTessData *dn, *dg;
		vector<string> layerNames;

		getMetaData().getLayerNames(layerNames);
		layerNames[3] = "middle_crust_G";
		layerNames[4] = "middle_crust_N";
		getMetaData().setLayerNames(layerNames);
		for (int v = 0; v < getNVertices(); ++v)
		{
			dn = getProfile(v, 3)->getData(0);
			dg = getProfile(v, 4)->getData(0);

			dn->getValues(vn, 2);
			dg->getValues(vg, 2);
			dn->setValue(0, vg[0]);
			dn->setValue(1, vg[1]);
			dg->setValue(0, vn[0]);
			dg->setValue(1, vn[1]);
		}
	}
}

void GeoTessModelSLBM::writeModelAscii(IFStreamAscii& output, const string& gridFileName)
{
	float vn[2], vg[2];
	GeoTessData *dn, *dg;
	vector<string> layerNames;
	getMetaData().getLayerNames(layerNames);
	layerNames[3] = "middle_crust_N";
	layerNames[4] = "middle_crust_G";
	getMetaData().setLayerNames(layerNames);

	for (int v = 0; v < getNVertices(); ++v)
	{
		dn = getProfile(v, 3)->getData(0);
		dg = getProfile(v, 4)->getData(0);

		dn->getValues(vn, 2);
		dg->getValues(vg, 2);
		dn->setValue(0, vg[0]);
		dn->setValue(1, vg[1]);
		dg->setValue(0, vn[0]);
		dg->setValue(1, vn[1]);
	}

	GeoTessModel::writeModelAscii(output, gridFileName);

	getMetaData().getLayerNames(layerNames);
	layerNames[3] = "middle_crust_G";
	layerNames[4] = "middle_crust_N";
	getMetaData().setLayerNames(layerNames);
	for (int v = 0; v < getNVertices(); ++v)
	{
		dn = getProfile(v, 3)->getData(0);
		dg = getProfile(v, 4)->getData(0);

		dn->getValues(vn, 2);
		dg->getValues(vg, 2);
		dn->setValue(0, vg[0]);
		dn->setValue(1, vg[1]);
		dg->setValue(0, vn[0]);
		dg->setValue(1, vn[1]);
	}

	output.writeStringNL("SLBM");
	output.writeIntNL(2); // slbm-specific io version number
	output.writeFloatNL((float)averageMantleVelocity[0]);
	output.writeFloatNL((float)averageMantleVelocity[1]);

	if (ioUncertainty)
	{
		// uncertainty
		output.writeStringNL("4 3");
		for (int p=0; p<4; ++p)
			for (int a=0; a<3; ++a)
			{
				output.writeString(Uncertainty::getPhase(p));
				output.writeString(" ");
				output.writeStringNL(Uncertainty::getAttribute(a));
				if (uncertainty[p][a] == NULL)
					output.writeStringNL("0 0");
				else
					output.writeString(uncertainty[p][a]->toStringFile());
			}
	}
	else
		output.writeStringNL("0");

}

void GeoTessModelSLBM::loadModelBinary(IFStreamBinary& input, const string& inputDirectory,
		const string& relGridFilePath)
{
	GeoTessModel::loadModelBinary(input, inputDirectory, relGridFilePath);

	string s;
	input.readString(s);
	if (s != "SLBM")
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModelSLBM::loadModelBinary()." << endl <<
				"File " << getMetaData().getInputModelFile() << endl <<
				"While this file is a GeoTessModel file, it does not appear to be GeoTessModelSLBM file. Extra SLBM data is missing." << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),105);
	}
	int version = input.readInt();
	if (version > 2)
	{
		ostringstream os;
		os << endl << "ERROR in GeoTessModelSLBM::loadModelBinary()." << endl <<
				"File " << getMetaData().getInputModelFile() << endl <<
				"SLBM IO Version number is " << version << " but that is not a supported version number." << endl
				<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
		throw SLBMException(os.str(),105);
	}
	averageMantleVelocity[0] = input.readFloat();
	averageMantleVelocity[1] = input.readFloat();

	int np = input.readInt();
	int na = input.readInt();
	if (na > 0 && np > 0)
	{
		if (na != 3 || np != 4)
		{
			ostringstream os;
			os << endl << "ERROR in GeoTessModelSLBM::loadModelBinary()." << endl <<
					"File " << getMetaData().getInputModelFile() << endl <<
					"Expecting uncertainty information for 3 attributes and 4 phases." << endl
					<< "Version " << SlbmVersion << "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			throw SLBMException(os.str(),105);
		}
		for (int p=0; p<np; ++p)
			for (int a=0; a<na; ++a)
			{
				int phase = Uncertainty::getPhase(input.readString());
				int attribute = Uncertainty::getAttribute(input.readString());
				// will be null if numDistances is zero
				uncertainty[phase][attribute] = Uncertainty::getUncertainty(input, phase, attribute);
			}
	}

	if (version > 1)
	{
		float vn[2], vg[2];
		GeoTessData *dn, *dg;
		vector<string> layerNames;

		getMetaData().getLayerNames(layerNames);
		layerNames[3] = "middle_crust_G";
		layerNames[4] = "middle_crust_N";
		getMetaData().setLayerNames(layerNames);
		for (int v = 0; v < getNVertices(); ++v)
		{
			dn = getProfile(v, 3)->getData(0);
			dg = getProfile(v, 4)->getData(0);

			dn->getValues(vn, 2);
			dg->getValues(vg, 2);
			dn->setValue(0, vg[0]);
			dn->setValue(1, vg[1]);
			dg->setValue(0, vn[0]);
			dg->setValue(1, vn[1]);
		}
	}
}

void GeoTessModelSLBM::writeModelBinary(IFStreamBinary& output, const string& gridFileName)
{
	float vn[2], vg[2];
	GeoTessData *dn, *dg;
	vector<string> layerNames;
	getMetaData().getLayerNames(layerNames);
	layerNames[3] = "middle_crust_N";
	layerNames[4] = "middle_crust_G";
	getMetaData().setLayerNames(layerNames);

	for (int v = 0; v < getNVertices(); ++v)
	{
		dn = getProfile(v, 3)->getData(0);
		dg = getProfile(v, 4)->getData(0);

		dn->getValues(vn, 2);
		dg->getValues(vg, 2);
		dn->setValue(0, vg[0]);
		dn->setValue(1, vg[1]);
		dg->setValue(0, vn[0]);
		dg->setValue(1, vn[1]);
	}

	GeoTessModel::writeModelBinary(output, gridFileName);

	getMetaData().getLayerNames(layerNames);
	layerNames[3] = "middle_crust_G";
	layerNames[4] = "middle_crust_N";
	getMetaData().setLayerNames(layerNames);
	for (int v = 0; v < getNVertices(); ++v)
	{
		dn = getProfile(v, 3)->getData(0);
		dg = getProfile(v, 4)->getData(0);

		dn->getValues(vn, 2);
		dg->getValues(vg, 2);
		dn->setValue(0, vg[0]);
		dn->setValue(1, vg[1]);
		dg->setValue(0, vn[0]);
		dg->setValue(1, vn[1]);
	}

	output.writeString("SLBM");
	output.writeInt(2); // slbm-specific io version number
	output.writeFloat((float)averageMantleVelocity[0]);
	output.writeFloat((float)averageMantleVelocity[1]);

	if (ioUncertainty)
	{
		// uncertainty
		output.writeInt(4);  // 4 phases: Pn, Sn, Pg, Lg
		output.writeInt(3);  // 3 attributes: TT, AZ, SH
		for (int p=0; p<4; ++p)
			for (int a=0; a<3; ++a)
			{
				output.writeString(Uncertainty::getPhase(p));
				output.writeString(Uncertainty::getAttribute(a));
				if (uncertainty[p][a] != NULL)
					uncertainty[p][a]->writeFile(output);
				else
				{
					// number of distances and number of depths both = 0
					output.writeInt(0); output.writeInt(0);
				}
			}
	}
	else { output.writeInt(0); output.writeInt(0); }

}

/**
 * Write the model currently in memory to a DataBuffer.  GeoTessModel does
 * not have a method to write itself to a DataBuffer so this method will
 * write the entire model, including all the information from the base class
 * (GeoTessModel) and all the extra data from the derived class (GeoTessModelSLBM).
 */
void GeoTessModelSLBM::writeModelDataBuffer(util::DataBuffer& buffer)
{
	// write out file type identifier ("GEOTESSMODEL"), format version,
	// code version, and data stamp

	buffer.writeCharArray("GEOTESSMODEL", 12);

	buffer.writeInt32(1);

	buffer.writeString(metaData->getOptimizationType().toString());

	buffer.writeString(metaData->getModelSoftwareVersion());
	buffer.writeString(metaData->getModelGenerationDate());

	buffer.writeString(metaData->getDescription());

	buffer.writeString(metaData->getAttributeNamesString());
	buffer.writeString(metaData->getAttributeUnitsString());
	buffer.writeString(metaData->getLayerNamesString());

	buffer.writeString(metaData->getDataType().toString());
	buffer.writeInt32(grid->getNVertices());

	// tessellation ids
	for (int i = 0; i < metaData->getNLayers(); ++i)
		buffer.writeInt32(metaData->getTessellation(i));

	for (int i = 0; i < grid->getNVertices(); ++i)
		for (int j = 0; j < metaData->getNLayers(); ++j)
		{
			const GeoTessProfile* p = getProfile(i,j);
			buffer.writeByte((uByte)p->getType().ordinal());
			if (p->getType() == GeoTessProfileType::CONSTANT)
			{
				buffer.writeFloat(p->getRadiusBottom());
				buffer.writeFloat(p->getRadiusTop());
				buffer.writeFloat(p->getData(0).getFloat(0));
				buffer.writeFloat(p->getData(0).getFloat(1));
			}
			else if (p->getType() == GeoTessProfileType::EMPTY)
			{
				buffer.writeFloat(p->getRadiusBottom());
				buffer.writeFloat(p->getRadiusTop());
			}
			else if (p->getType() == GeoTessProfileType::THIN)
			{
				buffer.writeFloat(p->getRadiusTop());
				buffer.writeFloat(p->getData(0).getFloat(0));
				buffer.writeFloat(p->getData(0).getFloat(1));
			}
			else if (p->getType() == GeoTessProfileType::NPOINT)
			{
				buffer.writeInt32(p->getNRadii());
				for (int k=0; k<p->getNRadii(); ++k)
				{
					buffer.writeFloat(p->getRadius(k));
					buffer.writeFloat(p->getData(k).getFloat(0));
					buffer.writeFloat(p->getData(k).getFloat(1));
				}
			}
			else if (p->getType() == GeoTessProfileType::SURFACE)
			{
				buffer.writeFloat(p->getData(0).getFloat(0));
				buffer.writeFloat(p->getData(0).getFloat(1));
			}
			else if (p->getType() == GeoTessProfileType::SURFACE_EMPTY)
			{
			}
		}

	buffer.writeString((string)"*");
	buffer.writeString(grid->getGridID());

	// Done with GeoTessModel information.  Now write GeoTessGrid info.

	// write out file type identifier ("GEOTESSGRID"), format version,
	// code version, and data stamp

	buffer.writeCharArray("GEOTESSGRID", 11);
	buffer.writeInt32(2);

	buffer.writeString(grid->getGridInputFile());
	buffer.writeString(grid->getGridOutputFile());

	buffer.writeString(grid->getGridSoftwareVersion());
	buffer.writeString(grid->getGridGenerationDate());

	buffer.writeString(grid->getGridID());

	buffer.writeInt32(grid->getNTessellations());
	buffer.writeInt32(grid->getNLevels());
	buffer.writeInt32(grid->getNTriangles());
	buffer.writeInt32(grid->getNVertices());

	buffer.writeIntArray(grid->getTessellations()[0], grid->getNTessellations()*2);

	buffer.writeIntArray(grid->getLevels()[0], grid->getNLevels()*2);

	buffer.writeDoubleArray(grid->getVertices()[0], grid->getNVertices()*3);

	buffer.writeIntArray(grid->getTriangles()[0], grid->getNTriangles()*3);

	// Done with GeoTessGrid.  Now write GeoTessModelSLBM
	// 'extra' data.

	buffer.writeString((string)"SLBM");
	buffer.writeInt32(1); // slbm-specific io version number is 1
	buffer.writeFloat((float)averageMantleVelocity[0]);
	buffer.writeFloat((float)averageMantleVelocity[1]);

	// uncertainty
	buffer.writeInt32(4);  // 4 phases: Pn, Sn, Pg, Lg
	buffer.writeInt32(3);  // 3 attributes: TT, AZ, SH
	for (int p=0; p<4; ++p)
		for (int a=0; a<3; ++a)
		{
			buffer.writeString(Uncertainty::getPhase(p));
			buffer.writeString(Uncertainty::getAttribute(a));
			if (uncertainty[p][a] != NULL)
				uncertainty[p][a]->serialize(buffer);
			else
				buffer.writeInt32(-1);
		}
}

int  GeoTessModelSLBM::getBufferSize()
{
	int bsiz = 12;

	bsiz += sizeof(int);

	bsiz += sizeof(int)+metaData->getModelSoftwareVersion().size();
	bsiz += sizeof(int)+metaData->getModelGenerationDate().size();

	bsiz += sizeof(int)+metaData->getDescription().size();

	bsiz += sizeof(int)+metaData->getAttributeNamesString().size();
	bsiz += sizeof(int)+metaData->getAttributeUnitsString().size();
	bsiz += sizeof(int)+metaData->getLayerNamesString().size();

	bsiz += sizeof(int)+metaData->getDataType().toString().size();

	bsiz += sizeof(int);  // nvertices

	bsiz += metaData->getNLayers()*sizeof(int);

	// tessellation ids
	bsiz += metaData->getNLayers()*sizeof(int);


	for (int i = 0; i < grid->getNVertices(); ++i)
		for (int j = 0; j < metaData->getNLayers(); ++j)
		{
			const GeoTessProfile* p = getProfile(i,j);

			bsiz += sizeof(uByte);
			if (p->getType() == GeoTessProfileType::CONSTANT)
				bsiz += 4*sizeof(float);
			else if (p->getType() == GeoTessProfileType::EMPTY)
				bsiz += 2*sizeof(float);
			else if (p->getType() == GeoTessProfileType::THIN)
				bsiz += 3*sizeof(float);
			else if (p->getType() == GeoTessProfileType::NPOINT)
				bsiz += sizeof(int) + p->getNRadii()*3*sizeof(float);
			else if (p->getType() == GeoTessProfileType::SURFACE)
				bsiz += 2*sizeof(float);
		}

	bsiz += sizeof(int)+1;
	bsiz += sizeof(int)+grid->getGridID().size();

	// Done with GeoTessModel information.  Now write GeoTessGrid info.

	// write out file type identifier ("GEOTESSGRID"), format version,
	// code version, and data stamp

	bsiz += 11;
	bsiz += sizeof(int);

	bsiz += sizeof(int)+grid->getGridSoftwareVersion().size();
	bsiz += sizeof(int)+grid->getGridGenerationDate().size();

	bsiz += sizeof(int)+grid->getGridID().size();

	bsiz += 4*sizeof(int);

	bsiz += grid->getNTessellations()*2*sizeof(int);

	bsiz += grid->getNLevels()*2*sizeof(int);

	// vertices
	bsiz += grid->getNVertices()*3*sizeof(double);

	// triangles
	bsiz += grid->getNTriangles()*3*sizeof(int);

	// Done with GeoTessGrid.  Now write GeoTessModelSLBM
	// 'extra' data.

	bsiz += sizeof(int)+4;  // (string)"SLBM"

	bsiz += sizeof(int); // slbm-specific io version number is 1
	bsiz += 2*sizeof(float); // average P and S velocities

	bsiz += 2*sizeof(int);
	//if (ioUncertainty)
		for (int p=0; p<4; ++p)
			for (int a=0; a<3; ++a)
			{
				bsiz += sizeof(int)+Uncertainty::getPhase(p).size();
				bsiz += sizeof(int)+Uncertainty::getAttribute(a).size();

				if (uncertainty[p][a] != NULL)
					bsiz += uncertainty[p][a]->getBufferSize();
				else
					bsiz += sizeof(int)+4; // "NULL"
			}
	return bsiz;
}

} // end namespace slbm

