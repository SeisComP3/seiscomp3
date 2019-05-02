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

#ifndef GEOTESSException_H
#define GEOTESSException_H

// **** _SYSTEM INCLUDES_ ******************************************************

#include <sstream>

// **** _LOCAL INCLUDES_ *******************************************************

#include "CPPUtils.h"
#include "GeoTessUtils.h"

// **** _BEGIN GEOTESS NAMESPACE_ **********************************************

namespace geotess
{

// **** _FORWARD REFERENCES_ ***************************************************

// **** _CLASS DEFINITION_ *****************************************************

/**
 * \brief An exception class for all GeoTess objects.
 *
 * An exception class for all GeoTess objects.
 *
 * @author Sandy Ballard
 *
 */
class GEOTESS_EXP_IMP GeoTessException
{
public:

	/**
	 *
	 */
	string emessage;

	/**
	 * Public error code set to one of the error constantants defined in this file.
	 */
	int ecode;

	/**
	 * Parameterized constructor specifying the error message to be displayed.
	 */
	GeoTessException(std::string message, int code)
			: emessage(message), ecode(code)
	{
	}

	/**
	 * Standard Constructor taking a partially defined string stream, which contains
	 * the basic error message, and appending version, file, and line number (of error)
	 * to the stream before assigning its entire contents to the message of this
	 * exception. The error code is also assigned.
	 */
	GeoTessException(ostringstream& os, const string& file, int line, int code)
			: emessage(""), ecode(code)
	{
		os << "OS: " << CPPUtils::getOpSys() << ",  Version: "
				<< GeoTessUtils::getVersion() << ",  File: " << file
				<< ",  Line: " << line << endl << endl;
		emessage = os.str();
	}

	/**
	 * Standard Constructor taking a partially defined string stream, which contains
	 * the basic error message, and appending version, file, and line number (of error)
	 * to the stream before assigning its entire contents to the message of this
	 * exception. The error code is also assigned.
	 */
	GeoTessException(const string& msg, const string& file, int line, int code)
			: emessage(""), ecode(code)
	{
		emessage = msg + "\nOS: " + CPPUtils::getOpSys() + ",  Version: "
				+ GeoTessUtils::getVersion() + ",  File: " + file + ",  Line: "
				+ CPPUtils::itos(line) + "\n\n";
	}

	/**
	 * Destructor.
	 */
	virtual ~GeoTessException()
	{
	}

	/**
	 * Appends version, file, and line number information to the input string stream.
	 */
	static void appendInfo(ostringstream& os, const string& file, int line)
	{
		os << "Version: " << GeoTessUtils::getVersion() << ",  File: " << file
				<< ",  Line: " << line << endl << endl;
	}

	/**
	 * Appends version, file, and line number information to the input string stream.
	 */
	static void appendInfo(string& msg, const string& file, int line)
	{
		msg += "\nVersion: " + GeoTessUtils::getVersion() + ",  File: " + file
				+ ",  Line: " + CPPUtils::itos(line) + "\n\n";
	}

};
// end class GeoTessException

}// end geotess namespace

#endif // GEOTESSException_H
