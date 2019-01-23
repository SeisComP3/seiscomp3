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
//- Program:       SLBMException
//- Module:        $RCSfile: SLBMException.h,v $
//- Revision:      $Revision: 1.7 $
//- Last Modified: $Date: 2011/08/23 20:56:53 $
//- Last Check-in: $Author: sballar $
//-
//- ****************************************************************************
#ifndef SLBMException_H
#define SLBMException_H

// **** _SYSTEM INCLUDES_ ******************************************************
#include <string>

#include "SLBMGlobals.h"

// **** _BEGIN SLBM NAMESPACE_ **************************************************

namespace slbm {

class SLBM_EXP_IMP SLBMException
//! \brief An Exception class for Grid and related objects.
//!
//! An Exception class for Grid and related objects.
//! 
{
	public:
	string emessage;
	int ecode;

	//! \brief Parameterized constructor specifying the error message
	//! to be displayed.
	//! 
	//! Parameterized constructor specifying the error message
	//! to be displayed.
	SLBMException(string message, int code)  {emessage = message; ecode = code; };

};

} // end slbm namespace

#endif
