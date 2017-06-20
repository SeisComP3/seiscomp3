/***************************************************************************
 *   Copyright (C) by GFZ Potsdam                                          *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

// This file was created by a source code generator.
// Do not modify the contents. Change the definition and run the generator
// again!


#ifndef __SEISCOMP_DATAMODEL_SOURCETIMEFUNCTION_H__
#define __SEISCOMP_DATAMODEL_SOURCETIMEFUNCTION_H__


#include <seiscomp3/datamodel/types.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(SourceTimeFunction);


/**
 * \brief Source time function used in moment-tensor inversion.
 */
class SC_SYSTEM_CORE_API SourceTimeFunction : public Core::BaseObject {
	DECLARE_SC_CLASS(SourceTimeFunction);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SourceTimeFunction();

		//! Copy constructor
		SourceTimeFunction(const SourceTimeFunction& other);

		//! Destructor
		~SourceTimeFunction();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		SourceTimeFunction& operator=(const SourceTimeFunction& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const SourceTimeFunction& other) const;
		bool operator!=(const SourceTimeFunction& other) const;

		//! Wrapper that calls operator==
		bool equal(const SourceTimeFunction& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Type of source time function. Values can be taken from the
		//! following:
		//! BOX_CAR, TRIANGLE, TRAPEZOID, UNKNOWN_FUNCTION.
		void setType(SourceTimeFunctionType type);
		SourceTimeFunctionType type() const;

		//! Source time function duration in seconds.
		void setDuration(double duration);
		double duration() const;

		//! Source time function rise time in seconds.
		void setRiseTime(const OPT(double)& riseTime);
		double riseTime() const;

		//! Source time function decay time in seconds.
		void setDecayTime(const OPT(double)& decayTime);
		double decayTime() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		SourceTimeFunctionType _type;
		double _duration;
		OPT(double) _riseTime;
		OPT(double) _decayTime;
};


}
}


#endif
