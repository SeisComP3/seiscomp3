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
//- Program:       CrustalProfileStore
//- Module:        $RCSfile: CrustalProfileStore.cc,v $
//- Revision:      $Revision: 1.9 $
//- Last Modified: $Date: 2011/10/07 13:14:55 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#include "CrustalProfileStore.h"
#include "SLBMGlobals.h"
#include "SLBMException.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

// **** _FUNCTION DESCRIPTION_ *************************************************
//
// Parameterized CrustalProfileStore Constructor
//
// *****************************************************************************
CrustalProfileStore::CrustalProfileStore(Grid& _grid, const int& _maxSize)
: grid(_grid),
  maxSize(_maxSize),
  locString("12345678901234567890123456789012")
{
}  // END CrustalProfileStore Default Constructor


// **** _FUNCTION DESCRIPTION_ *************************************************
//
// CrustalProfileStore Destructor
//
// *****************************************************************************
CrustalProfileStore::~CrustalProfileStore()
{
	clear();
}  

void CrustalProfileStore::clear()
{
	for (map<string, CrustalProfile*>::iterator it = profiles.begin();
		it != profiles.end(); ++it)
			//pool.push_back(it->second);
			delete it->second;

	profiles.clear();
	priority.clear();

	while (!pool.empty())
	{
		delete pool.front();
		pool.pop_front();
	}
}

CrustalProfile* CrustalProfileStore::getCrustalProfile(const int& phase, 
		const double& lat, const double& lon, const double& depth)
{
	CrustalProfile* profile = NULL;

	// create a string out of the phase, lat,lon,depth information 
	*((int*)    &locString[0])  = phase;
	*((double*) &locString[8])  = lat;
	*((double*) &locString[16]) = lon;
	*((double*) &locString[24]) = depth;

	// see if CrustalProfileStore already knows about a crustal profile at this 
	// phase/location.
	map<string, CrustalProfile*>::iterator it = profiles.find(locString);

	if (it == profiles.end())
	{
		// do not have a reference to a profile that matches this description.
		// Have to create one.

		// see if the priority list is max size
		if ((int)priority.size() >= maxSize)
		{
			// priority list is maxSize so it can't grow anymore.
			// Remove the lowest priority CrustalProfile object.

			// get the string at back of list (lowest priority)
			string oldString = priority.back();

			// find iterator to oldString->crustalProfile pair
			it = profiles.find(oldString);

			// return the CrustalProfile object to pool of unused profiles.
			pool.push_back(it->second);

			// remove the oldString->crustalProfile pair from the map
			profiles.erase(it);

			// pop last element off priority list
			priority.pop_back();
		}
		
		// get a new crustal profile object at this phase/location.
		if (pool.size() > 0)
		{
			profile = pool.front();
			pool.pop_front();
		}
		else
			profile = new CrustalProfile();

		profile->setup(grid, phase, lat, lon, depth);

		// add a map entry that relates this phase/location to the new
		// crustal profile.
		profiles[locString] = profile;

		// push the new locString onto the front of the priority list
		priority.push_front(locString);
	}
	else
	{
		// there is already a crustal profile defined at this phase/location.
		// return a pointer to the existing crustal profile object.
		profile = it->second;

		// find locString in the priority list, remove it from the list,
		// and add it to the front of the list.
		if (locString != priority.front())
		{
			list<string>::iterator lit = priority.begin();
			while (*lit != locString) ++lit;
			priority.erase(lit);
			priority.push_front(locString);
		}
	}
	
	return profile;
}

} // end slbm namespace
