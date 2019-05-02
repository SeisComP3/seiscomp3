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
//- Module:        $RCSfile: CrustalProfileStore.h,v $
//- Revision:      $Revision: 1.6 $
//- Last Modified: $Date: 2011/08/23 20:56:50 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef CrustalProfileStore_H
#define CrustalProfileStore_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <map>
#include <list>

using namespace std;

// **** _LOCAL INCLUDES_ *******************************************************

#include "SLBMGlobals.h"
#include "CrustalProfile.h"
#include "Location.h"
// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

//! \brief CrustalProfileStore supports pool allocation for
//! CrustalProfile objects.  
//!
//! CrustalProfileStore supports pool allocation for
//! CrustalProfile objects.  Grid owns two CrustalProfileStore
//! objects, one for seismic sources and a separate one for
//! seismic receivers.  When a GreatCircle object is created, it
//! needs two CrustalProfile objects, one for the source and one
//! for the receiver.  It requests these CrustalProfile objects
//! from the Grid object and Grid requests them from the two
//! CrustalProfileStore objects.  A Grid object requests a
//! CrustalProfile object from a CrustalProfileStore by calling
//! CrustalProfileStore::getCrustalProfile(phase, lat, lon,
//! depth).  When it receives such a request CrustalProfileStore
//! searches its internal containers to see if it already has a
//! CrustalProfile object for that phase/location.  If it does,
//! it returns a handle to the existing CrustalProfile object
//! and promotes the priority of that CrustalProfile object to
//! the highest level.  If it does not already have a
//! CrustalProfile object for the specified phase/location, it
//! pops a reference to a previously used CrustalProfile object
//! that it keeps in a pool of previously instantiated but
//! currently unused objects and resets the data in the
//! CrustalProfile object for the new phase/location.  If the
//! pool is empty, it constructs a new CrustalProfile object. 
//! It then pushes the Crustal Profile object onto the head of
//! the priority list.  If the priority list reaches its maximum
//! size, then CrustalProfileStore pops the CrustalProfile
//! object with the lowest priority off the back of the priority
//! list and returns it to the pool of unused CrustalProfile
//! objects. 
//! 
//! There is an important assumption being made here and that is
//! that when CrustalProfileStore::getCrustalProfile(phase, lat,
//! lon, depth) is called, there are no valid references to any
//! CrustalProfile objects out in the application.  This is
//! necessary so that CrustalProfileStore can reset the
//! information contained in CrustalProfile that it pops off the
//! pool of unused objects.  As currently implemented this
//! condition is satisfied in SLBM.  CrustalProfile objects are
//! owned only by GreatCircle objects.  SlbmInterface objects
//! maintain a reference to a single GreatCircle object at a
//! time.  When an application requests a new GreatCircle
//! object, SlbmInterface deletes the current GreatCircle object
//! (if it currently has one) and then instantiates a new
//! GreatCircle object.  Since only one GreatCircle exists at a
//! time, there can be no valid references to any CrustalProfile
//! at the time that
//! CrustalProfileStore::getCrustalProfile(phase, lat, lon,
//! depth) is called.
class SLBM_EXP_IMP CrustalProfileStore
{

public:

	//! Constructor.
	CrustalProfileStore(Grid& grid, const int& maxSize);

	//! Destructor.
	~CrustalProfileStore();

	void clear();

	CrustalProfile* getCrustalProfile(const int& phase, 
		const double& lat, const double& lon, const double& depth);

	int getNCrustalProfiles() { return (int)profiles.size(); };

	size_t memSize();

private:

	Grid& grid;

	int maxSize;

	map<string, CrustalProfile*> profiles;

	list<string> priority;

	list<CrustalProfile*> pool;

	string locString;

};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
//  INLINE FUNCTIONS 
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

inline size_t CrustalProfileStore::memSize() 
{ 
	size_t n = 0;
	for (map<string, CrustalProfile*>::iterator 
		it  = profiles.begin();
		it != profiles.end(); 
		it++)
			n += it->second->memSize();
	return (int)n;
}

} // end slbm namespace

#endif // CrustalProfileStore.h
