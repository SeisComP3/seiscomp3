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



#ifndef __SEISCOMP3_CORE_GREENSFUNCTION_H__
#define __SEISCOMP3_CORE_GREENSFUNCTION_H__


#include <seiscomp3/core/array.h>
#include <seiscomp3/core/enumeration.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/core.h>


namespace Seiscomp {
namespace Core {


MAKEENUM(
	GreensFunctionComponent,
	EVALUES(
		ZSS,
		ZDS,
		ZDD,
		RSS,
		RDS,
		RDD,
		TSS,
		TDS,
		ZEP,
		REP
	),
	ENAMES(
		"ZSS",
		"ZDS",
		"ZDD",
		"RSS",
		"RDS",
		"RDD",
		"TSS",
		"TDS",
		"ZEP",
		"REP"
	)
);


DEFINE_SMARTPOINTER(GreensFunction);

class SC_SYSTEM_CORE_API GreensFunction : public Core::BaseObject {
	DECLARE_SC_CLASS(GreensFunction);
	DECLARE_SERIALIZATION;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		GreensFunction();
		GreensFunction(const std::string &model, double distance,
		               double depth, double fsamp, double timeOffset);

		virtual ~GreensFunction();

	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		void setId(const std::string &id);
		const std::string &id() const;

		void setModel(const std::string &model);
		const std::string &model() const;

		void setDistance(double);
		double distance() const;

		void setDepth(double);
		double depth() const;

		void setSamplingFrequency(double);
		double samplingFrequency() const;

		void setTimeOffset(double);
		double timeOffset() const;

		//! Returns the length in seconds
		double length(GreensFunctionComponent) const;

		void setData(GreensFunctionComponent, Array *);
		Array *data(GreensFunctionComponent) const;

		void setData(int, Array *);
		Array *data(int) const;


	// ------------------------------------------------------------------
	//  Private members
	// ------------------------------------------------------------------
	private:
		std::string _id;
		std::string _model;
		double      _distance;
		double      _depth;
		double      _samplingFrequency;
		double      _timeOffset;
		ArrayPtr    _components[GreensFunctionComponent::Quantity];
};


}
}


#endif
