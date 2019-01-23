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


/*
 *
 * Copyright (c) 1990-1999 Science Applications International Corporation.
 *
 * NAME
 *
 * FILE slbmshell.cc
 *
 * DESCRIPTION
 * SLBM- Thin "C" Shell Interface
 *
 */

#include "SlbmInterface.h"
#include "slbm_C_shell.h"
#include "SLBMException.h"
#include <stdio.h>
#include <string>

using namespace slbm;

static SlbmInterface* slbm_handle = NULL;

string errortext;

//==============================================================================
int slbm_shell_getVersion( char* str )
{
	int retval = 1;		errortext = "";
	try
	{
		string temp = slbm_handle->getVersion();
		for( int i = 0; i < (int)temp.length(); i++ )
		{
			*str++ = temp[i];
		}
		*str = '\0';
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getErrorMessage( char* str )
{
	if (errortext.length() == 0)
		errortext = "An unrecognized error has occurred in slbmshell.cc";

	for( int i = 0; i < (int)errortext.length(); i++ )
	{
		*str++ = errortext[i];
	}
	return 0;
}
//==============================================================================
int slbm_shell_create ()
{
	int retval = 1;		errortext = "";
	// If the handle already exists, then simply return
	if (slbm_handle != ( SlbmInterface* ) NULL) 
	{
		retval = 1;
	}

	try
	{
		slbm_handle = new SlbmInterface (); 
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
} /* END slbm_shell_create */


//==============================================================================
int slbm_shell_create_fixedEarthRadius ( double* radius )
{
	int retval = 1;		errortext = "";
	// If the handle already exists, then simply return
	if (slbm_handle != (SlbmInterface*) NULL) 
	{
		retval = 1;
	}

	try
	{
		slbm_handle = new SlbmInterface ( *radius ); 
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_loadVelocityModel( const char* modelPath )
{
	int retval = 1;		errortext = "";
	try
	{
		string modelPath_cc( modelPath );
		slbm_handle->loadVelocityModel( modelPath_cc );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}

//==============================================================================
int slbm_shell_saveVelocityModel( const char* modelFileName )
{
	int retval = 1;		errortext = "";
	try
	{
		string modelFileName_cc( modelFileName );
		slbm_handle->saveVelocityModel( modelFileName_cc );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}

//==============================================================================
int slbm_shell_saveVelocityModelFormat( const char* modelFileName, int format )
{
	int retval = 1;		errortext = "";
	try
	{
		string modelFileName_cc( modelFileName );
		slbm_handle->saveVelocityModel( modelFileName_cc, format );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}

//==============================================================================
int slbm_shell_delete ()
{
	int retval = 1;		errortext = "";
	try
	{
		if (slbm_handle != (SlbmInterface*) NULL)
		{
			delete slbm_handle;
			slbm_handle = (SlbmInterface *) NULL;
		}
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
} // END slbm_shell_delete



//==============================================================================
int slbm_shell_createGreatCircle ( char* phase, 
		double* sourceLat,
		double* sourceLon,
		double* sourceDepth,
		double* receiverLat,
		double* receiverLon,
		double* receiverDepth)
{	
	int retval = 1;		errortext = "";
	try
	{
		const string phase_cc(phase);
		slbm_handle->createGreatCircle( phase_cc,
				*sourceLat,
				*sourceLon,
				*sourceDepth,
				*receiverLat,
				*receiverLon,
				*receiverDepth);
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}

//==============================================================================
int slbm_shell_isValid ()
{
	int retval = 1;		errortext = "";
	try
	{
		bool result = slbm_handle->isValid();
		if( result == true )
		{
			retval = 0;
		}
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_clear ()
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->clear();
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		char *errmessage( new char[ex.emessage.length() + 1] );
		copy( ex.emessage.begin(), ex.emessage.end(), errmessage );
		errmessage[ex.emessage.length()] = '\0';
		return 1;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getDistance ( double* distance ) 
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getDistance(*distance);
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getSourceDistance ( double* dist )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getSourceDistance( *dist );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getReceiverDistance ( double* dist )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getReceiverDistance( *dist );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getHeadwaveDistance ( double* dist )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getHeadwaveDistance ( *dist );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getHeadwaveDistanceKm ( double* dist )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getHeadwaveDistanceKm ( *dist );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getTravelTime ( double* travelTime ) 
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getTravelTime ( *travelTime );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getTravelTimeComponents ( double* tTotal, double* tSource, double* tReceiver, double* tHeadwave, double* tGradient )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getTravelTimeComponents ( *tTotal, *tSource, *tReceiver, *tHeadwave, *tGradient );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getWeights ( int nodeId[], double weight[], int* nweights ) 
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getWeights ( nodeId, weight, *nweights );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getWeightsSource ( int nodeids[], double weights[], int* nWeights)
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getWeightsSource ( nodeids, weights, *nWeights );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getWeightsReceiver ( int nodeids[], double weights[], int* nWeights )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getWeightsReceiver ( nodeids, weights, *nWeights );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_toString ( char* str, int verbosity ) 
{
	int retval = 1;		errortext = "";
	try
	{
		string strOut = slbm_handle->toString ( verbosity );
		for( int i = 0; i < (int)strOut.length(); i++ )
		{
			*str++ = strOut[i];
		}
		*str = '\0';
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getNGridNodes ( int* numGridNodes )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getNGridNodes ( *numGridNodes );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getGridData ( int* nodeId, double* latitude, double* longitude, double* depth, double* pvelocity,
		double* svelocity, double* gradient )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getGridData( *nodeId, 
				*latitude,
				*longitude,
				depth,
				pvelocity,
				svelocity,
				gradient );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_setGridData ( int* nodeId, double* depth, double* pvelocity, double* svelocity, double* gradient )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->setGridData ( *nodeId, depth, pvelocity, svelocity, gradient );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getNHeadWavePoints ( int* npoints )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getNHeadWavePoints ( *npoints );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getGreatCircleData ( char* phase, double* path_increment, double sourceDepth[], double sourceVelocity[],
		double receiverDepth[],	double receiverVelocity[], int* npoints, double headWaveVelocity[], double gradient[] )
{
	int retval = 1;		errortext = "";
	try
	{
		string phase_cc;
		slbm_handle->getGreatCircleData ( phase_cc, *path_increment, sourceDepth, sourceVelocity, receiverDepth,
				receiverVelocity, *npoints, headWaveVelocity, gradient );

		for( int i = 0; i < (int)phase_cc.length(); i++ )
		{
			phase[i] = phase_cc[i];
		}
		phase[2] = '\0';
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getGreatCircleNodeInfo( int** neighbors, double** coefficients, const int* maxpoints,
		const int* maxnodes, int* npoints, int* nnodes )
{
	int retval = 1;		errortext = "";
	try
	{
		string phase_cc;
		slbm_handle->getGreatCircleNodeInfo ( neighbors, coefficients, *maxpoints, *maxnodes, *npoints, nnodes );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getInterpolatedPoint ( double* lat, double* lon,
		int* nodeIds, double* coefficients, int* nnodes, double* depth, double* pvelocity,
		double* svelocity, double* pgradient, double* sgradient )
{ 
	int retval = 1;		errortext = "";
	try
	{
		const double& lat_cc = *lat; 
		const double& lon_cc = *lon;
		slbm_handle->getInterpolatedPoint ( lat_cc, 
				lon_cc,
				nodeIds,
				coefficients,
				*nnodes,
				depth,
				pvelocity,
				svelocity,
				*pgradient,
				*sgradient );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
} 
//==============================================================================
int slbm_shell_getInterpolatedTransect ( double lat[], double lon[], int* nLatLon, int** nodeIds,
		double** coefficients, int* nnodes, double depth[][NLAYERS], double pvelocity[][NLAYERS],
		double svelocity[][NLAYERS], double pgradient[], double sgradient[], int* npoints )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getInterpolatedTransect ( lat, lon, *nLatLon, nodeIds, coefficients, nnodes, depth, pvelocity,	svelocity,
				pgradient, sgradient, *npoints );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_initializeActiveNodes ( double* latmin, double* lonmin, double* latmax, double* lonmax )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->initializeActiveNodes ( *latmin, *lonmin, *latmax, *lonmax );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}

//==============================================================================
int slbm_shell_initActiveNodesFile(char* polygonFileName)
{
	int retval = 1;		errortext = "";
	try
	{
		string polygonFileName_cc ( polygonFileName );
		slbm_handle->initializeActiveNodes(polygonFileName_cc);
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}

//==============================================================================
int slbm_shell_initActiveNodesPoints(double* lat, double* lon, int* npoints, int* inDegrees)
{
	int retval = 1;		errortext = "";
	try
	{
		const int n = *npoints;
		slbm_handle->initializeActiveNodes(lat, lon, n, (inDegrees != 0));
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}

//==============================================================================
int slbm_shell_clearActiveNodes()
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->clearActiveNodes();
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;

}

//==============================================================================
int slbm_shell_getNActiveNodes ( int* nNodes )
{
	int retval = 1;		errortext = "";
	try
	{
		*nNodes = slbm_handle->getNActiveNodes();
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getGridNodeId ( int activeNodeId, int* gridNodeId )
{
	int retval = 1;		errortext = "";
	try
	{
		*gridNodeId = slbm_handle->getGridNodeId ( activeNodeId );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getActiveNodeId ( int gridNodeId, int* activeNodeId  )
{
	int retval = 1;		errortext = "";
	try
	{
		*activeNodeId = slbm_handle->getActiveNodeId ( gridNodeId );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getNodeHitCount ( int* nodeId, int* hitCount )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getNodeHitCount ( *nodeId, *hitCount );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getNodeNeighbors ( int* nid, int neighbors[], int* nNeighbors )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getNodeNeighbors ( *nid, neighbors, *nNeighbors );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getNodeNeighborInfo ( int* nid, int neighbors[], double distance[], double azimuth[], int* nNeighbors )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getNodeNeighborInfo ( *nid, neighbors, distance, azimuth, *nNeighbors );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}//==============================================================================
int slbm_shell_getNodeSeparation ( int* node1, int* node2, double* distance )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getNodeSeparation ( *node1, *node2, *distance );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getNodeAzimuth ( int* node1, int* node2, double* azimuth )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getNodeAzimuth ( *node1, *node2, *azimuth );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getTravelTimeUncertainty( int* phase, double* distance, double* uncertainty )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getTravelTimeUncertainty( *phase, *distance, *uncertainty );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getTTUncertainty(double* uncertainty)
{ 
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getTravelTimeUncertainty ( *uncertainty );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
		*uncertainty = NA_VALUE;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getZhaoParameters ( double* Vm, double* Gm, double* H, double* C, double* Cm, int* udSign )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getZhaoParameters( *Vm, *Gm, *H, *C, *Cm, *udSign );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}

//==============================================================================
int slbm_shell_getActiveNodeWeights ( int nodeId[], double weight[], int* nWeights )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getActiveNodeWeights ( nodeId, weight, *nWeights );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getActiveNodeWeightsSource ( int nodeids[], double weights[], int* nWeights )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getActiveNodeWeightsSource ( nodeids, weights, *nWeights );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getActiveNodeWeightsReceiver ( int nodeids[], double weights[], int* nWeights )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getActiveNodeWeightsReceiver ( nodeids, weights, *nWeights );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getActiveNodeNeighbors ( int* nid, int neighbors[], int* nNeighbors )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getActiveNodeNeighbors ( *nid, neighbors, *nNeighbors );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getActiveNodeNeighborInfo ( int* nid, int neighbors[], double distance[],
		double azimuth[], int* nNeighbors )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getActiveNodeNeighborInfo ( *nid, neighbors, distance,
				azimuth, *nNeighbors );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getActiveNodeData (   int* nodeId,
		double* latitude,
		double* longitude,
		double depth[NLAYERS],
		double pvelocity[NLAYERS],
		double svelocity[NLAYERS],
		double gradient[2] )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getActiveNodeData (	*nodeId,
				*latitude,
				*longitude,
				depth,
				pvelocity,
				svelocity,
				gradient );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_setActiveNodeData (	int* nodeId,
		double depth[NLAYERS],
		double pvelocity[NLAYERS],
		double svelocity[NLAYERS],
		double gradient[2] )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->setActiveNodeData(	*nodeId,
				depth,
				pvelocity,
				svelocity,
				gradient );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_setCHMax ( double* chMax )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->setCHMax ( *chMax );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getCHMax ( double* chMax )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getCHMax ( *chMax );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getAverageMantleVelocity ( int* type, double* velocity )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getAverageMantleVelocity ( *type, *velocity );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_setAverageMantleVelocity ( int* type, double* velocity )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->setAverageMantleVelocity ( *type, *velocity );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_loadVelocityModelBinary ( const char* modelDirectory )
{
	int retval = 1;		errortext = "";
	try
	{
		string modelDirectory_cc ( modelDirectory );
		slbm_handle->loadVelocityModelBinary ( modelDirectory_cc );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_specifyOutputDirectory ( const char* directoryName )
{
	int retval = 1;		errortext = "";
	try
	{
		string directoryName_cc ( directoryName );
		slbm_handle->specifyOutputDirectory ( directoryName_cc );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_saveVelocityModelBinary()
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->saveVelocityModelBinary();
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getTessId ( char* tessId )
{
	int retval = 1;		errortext = "";
	try
	{
		string tessId_cc;
		slbm_handle->getTessId ( tessId_cc );
		for( int i = 0; i < (int)tessId_cc.length(); i++ )
		{
			*tessId++ = tessId_cc[i];
		}
		*tessId = '\0';
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getFractionActive ( double* fractionActive )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getFractionActive ( *fractionActive );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_setMaxDistance ( const double* maxDistance ) 
{ 
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->setMaxDistance( *maxDistance );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getMaxDistance ( double* maxDistance ) 
{

	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getMaxDistance ( *maxDistance );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_setMaxDepth ( const double* maxDepth ) 
{ 
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->setMaxDepth ( *maxDepth );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getMaxDepth ( double* maxDepth ) 
{ 
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getMaxDepth ( *maxDepth );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getPgLgComponents(double* tTotal, double* tTaup, 
		double* tHeadwave, double* pTaup, double* pHeadwave, double* trTaup, double* trHeadwave )
{ 
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getPgLgComponents ( *tTotal, *tTaup, *tHeadwave, *pTaup, *pHeadwave, *trTaup, *trHeadwave);
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================


// new gtb 16Jan2009
//==============================================================================
int slbm_shell_get_dtt_dlat(double* dtt_dlat)
{ 
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->get_dtt_dlat ( *dtt_dlat );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_get_dtt_dlon(double* dtt_dlon)
{ 
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->get_dtt_dlon ( *dtt_dlon );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_get_dtt_ddepth(double* dtt_ddepth)
{ 
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->get_dtt_ddepth ( *dtt_ddepth );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
////==============================================================================
//int slbm_shell_get_dsh_dlat(double* dsh_dlat)
//{ 
// 	int retval = 1;		errortext = "";
//	try
//	{
//		slbm_handle->get_dsh_dlat ( *dsh_dlat );
//		retval = 0;
//	}
//	catch( SLBMException& ex )
//	{
// 		errortext = ex.emessage;
//		retval = ex.ecode;
//	}
//	return retval;
//}
////==============================================================================
//int slbm_shell_get_dsh_dlon(double* dsh_dlon)
//{ 
// 	int retval = 1;		errortext = "";
//	try
//	{
//		slbm_handle->get_dsh_dlon ( *dsh_dlon );
//		retval = 0;
//	}
//	catch( SLBMException& ex )
//	{
// 		errortext = ex.emessage;
//		retval = ex.ecode;
//	}
//	return retval;
//}
////==============================================================================
//int slbm_shell_get_dsh_ddepth(double* dsh_ddepth)
//{ 
// 	int retval = 1;		errortext = "";
//	try
//	{
//		slbm_handle->get_dsh_ddepth ( *dsh_ddepth );
//		retval = 0;
//	}
//	catch( SLBMException& ex )
//	{
// 		errortext = ex.emessage;
//		retval = ex.ecode;
//	}
//	return retval;
//}
////==============================================================================
//int slbm_shell_get_dsh_ddist(double* dsh_ddist)
//{ 
// 	int retval = 1;		errortext = "";
//	try
//	{
//		slbm_handle->get_dsh_ddist ( *dsh_ddist );
//		retval = 0;
//	}
//	catch( SLBMException& ex )
//	{
// 		errortext = ex.emessage;
//		retval = ex.ecode;
//	}
//	return retval;
//}

//==============================================================================
int slbm_shell_getSlowness(double* slowness)
{ 
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getSlowness ( *slowness );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getSlownessUncertainty( int* phase, double* distance, double* slownessUncertainty )
{ 
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getSlownessUncertainty ( *phase, *distance, *slownessUncertainty );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getSHUncertainty(double* slownessUncertainty)
{ 
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getSlownessUncertainty ( *slownessUncertainty );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================

// new sb 8/2011  version 2.7.0
//==============================================================================
int slbm_shell_getPiercePointSource(double* lat, double* lon, double* depth)
{ 
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getPiercePointSource (*lat, *lon, *depth);
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getPiercePointReceiver(double* lat, double* lon, double* depth)
{ 
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getPiercePointReceiver (*lat, *lon, *depth);
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getDelDistance(double* delDistance){
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getDelDistance( *delDistance );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getDelDepth(double* delDepth){
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getDelDepth( *delDepth );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_setDelDistance( double delDistance)
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->setDelDistance( delDistance );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_setDelDepth( double delDepth)
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->setDelDepth( delDepth );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}


//==============================================================================
int slbm_shell_getGreatCircleLocations ( double latitude[], double longitude[] , 
		double depth[], int* npoints )
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getGreatCircleLocations (latitude, longitude, depth, *npoints );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}

//==============================================================================
int slbm_shell_getGreatCirclePoints (double sourceLat, double sourceLon, 
		double receiverLat, double receiverLon,int npoints, 
		double latitude[], double longitude[]) 
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getGreatCirclePoints(
				sourceLat,
				sourceLon,
				receiverLat,
				receiverLon,
				npoints,
				latitude,
				longitude );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getGreatCirclePointsOnCenters (double sourceLat, double sourceLon, 
		double receiverLat, double receiverLon,int npoints, 
		double latitude[], double longitude[] ) 
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getGreatCirclePointsOnCenters(
				sourceLat,
				sourceLon,
				receiverLat,
				receiverLon,
				npoints,
				latitude,
				longitude );
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}
//==============================================================================
int slbm_shell_getDistAz(double aLat, double aLon, double bLat, double bLon, 
		double* distance, double* azimuth, double naValue)
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->getDistAz (aLat, aLon, bLat, bLon, *distance, *azimuth, naValue);
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}


//==============================================================================
int slbm_shell_movePoint(double aLat, double aLon, double distance, double azimuth,
		double* bLat, double* bLon)
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->movePoint (aLat, aLon, distance, azimuth, *bLat, *bLon);
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;
}

//==============================================================================
int slbm_shell_setInterpolatorType(char* interpolatorType)
{
	int retval = 1;		errortext = "";
	try
	{
		slbm_handle->setInterpolatorType(interpolatorType);
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;

}

//==============================================================================
int slbm_shell_getInterpolatorType(char* interpolatorType)
{
	int retval = 1;		errortext = "";
	try
	{
		string temp = slbm_handle->getInterpolatorType();
		for( int i = 0; i < (int)temp.length(); i++ )
		{
			*interpolatorType++ = temp[i];
		}
		*interpolatorType = '\0';
		retval = 0;
		retval = 0;
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;

}

//==============================================================================
int slbm_shell_getModelString(char* modelString, int* allocatedSize)
{
	int retval = 1;		errortext = "";
	try
	{
		string temp = slbm_handle->getModelString();
		if ((int)temp.length() >= *allocatedSize)
		{
			*modelString = '\0';
			ostringstream os;
			os << endl << "ERROR in slbm_shell_getModelString" << endl
					<< "Allocated size of argument uncertaintyTable (" << allocatedSize << ")  "
					<< "is less than required size (" << temp.length()+1 << ")." << endl
					<< "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			errortext = os.str();
			retval = -1;
		}
		else
		{
			for( int i = 0; i < (int)temp.length(); i++ )
			{
				*modelString++ = temp[i];
			}
			*modelString = '\0';
			retval = 0;
		}
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;

}

//==============================================================================
int slbm_shell_getUncertaintyTable(int* phaseIndex, int* attributeIndex, char* uncertaintyTable,
		int* allocatedSize)
{
	int retval = 1;		errortext = "";
	try
	{
		const int phase = *phaseIndex;
		const int attribute = *attributeIndex;
		string temp = slbm_handle->getUncertaintyTable(phase, attribute);
		if ((int)temp.length() >= *allocatedSize)
		{
			*uncertaintyTable = '\0';
			ostringstream os;
			os << endl << "ERROR in slbm_shell_getUncertaintyTable" << endl
					<< "Allocated size of argument uncertaintyTable (" << allocatedSize << ")  "
					<< "is less than required size (" << temp.length()+1 << ")." << endl
					<< "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			errortext = os.str();
			retval = -1;
		}
		else
		{
			for( int i = 0; i < (int)temp.length(); i++ )
			{
				*uncertaintyTable++ = temp[i];
			}
			*uncertaintyTable = '\0';
			retval = 0;
		}
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;

}

//==============================================================================
int slbm_shell_getUncertaintyFileFormat(int* phaseIndex, int* attributeIndex, char* uncertaintyTable,
		int* allocatedSize)
{
	int retval = 1;		errortext = "";
	try
	{
		const int phase = *phaseIndex;
		const int attribute = *attributeIndex;
		string temp = slbm_handle->getUncertaintyFileFormat(phase, attribute);
		if ((int)temp.length() >= *allocatedSize)
		{
			*uncertaintyTable = '\0';
			ostringstream os;
			os << endl << "ERROR in slbm_shell_getUncertaintyFileFormat" << endl
					<< "Allocated size of argument uncertaintyTable (" << allocatedSize << ")  "
					<< "is less than required size (" << temp.length()+1 << ")." << endl
					<< "  File " << __FILE__ << " line " << __LINE__ << endl << endl;
			errortext = os.str();
			retval = -1;

		}
		else
		{
			for( int i = 0; i < (int)temp.length(); i++ )
			{
				*uncertaintyTable++ = temp[i];
			}
			*uncertaintyTable = '\0';
			retval = 0;
		}
	}
	catch( SLBMException& ex )
	{
		errortext = ex.emessage;
		retval = ex.ecode;
	}
	return retval;

}

