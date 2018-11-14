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


#ifndef __SEISCOMP_DATAMODEL_ARCLINKREQUESTSUMMARY_H__
#define __SEISCOMP_DATAMODEL_ARCLINKREQUESTSUMMARY_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ArclinkRequestSummary);


class SC_SYSTEM_CORE_API ArclinkRequestSummary : public Core::BaseObject {
	DECLARE_SC_CLASS(ArclinkRequestSummary);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ArclinkRequestSummary();

		//! Copy constructor
		ArclinkRequestSummary(const ArclinkRequestSummary& other);

		//! Destructor
		~ArclinkRequestSummary();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ArclinkRequestSummary& operator=(const ArclinkRequestSummary& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const ArclinkRequestSummary& other) const;
		bool operator!=(const ArclinkRequestSummary& other) const;

		//! Wrapper that calls operator==
		bool equal(const ArclinkRequestSummary& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setOkLineCount(int okLineCount);
		int okLineCount() const;

		void setTotalLineCount(int totalLineCount);
		int totalLineCount() const;

		void setAverageTimeWindow(int averageTimeWindow);
		int averageTimeWindow() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		int _okLineCount;
		int _totalLineCount;
		int _averageTimeWindow;
};


}
}


#endif
