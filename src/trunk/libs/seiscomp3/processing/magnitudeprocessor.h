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



#ifndef __SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_H__
#define __SEISCOMP_PROCESSING_MAGNITUDEPROCESSOR_H__

#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/core/enumeration.h>
#include <seiscomp3/processing/processor.h>
#include <seiscomp3/client.h>


namespace Seiscomp {

namespace Processing {


DEFINE_SMARTPOINTER(MagnitudeProcessor);

class SC_SYSTEM_CLIENT_API MagnitudeProcessor : public Processor {
	DECLARE_SC_CLASS(MagnitudeProcessor);

	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		MAKEENUM(
			Status,
			EVALUES(
				//! No error
				OK,
				//! Given amplitude is out of range
				AmplitudeOutOfRange,
				//! Given depth is out of range to continue processing
				DepthOutOfRange,
				//! Given distance is out of range to continue processing
				DistanceOutOfRange,
				//! Given period is out of range to continue processing
				PeriodOutOfRange,
				//! The estimation of the Mw magnitude is not supported
				MwEstimationNotSupported,
				//! Unspecified error
				Error
			),
			ENAMES(
				"OK",
				"amplitude out of range",
				"depth out of range",
				"distance out of range",
				"period out of range",
				"Mw estimation not supported",
				"error"
			)
		);


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		MagnitudeProcessor();
		MagnitudeProcessor(const std::string& type);

		//! D'tor
		~MagnitudeProcessor();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		//! Returns the type of magnitude to be calculated
		const std::string& type() const;

		//! Returns the type of the Mw estimation
		//! The default implementation returns "Mw($type)"
		virtual std::string typeMw() const;

		/**
		 * Returns the amplitude type used as input for this magnitude
		 * processor.
		 * The default implementation returns type()
		 * @return 
		 */
		virtual std::string amplitudeType() const;

		virtual bool setup(const Settings &settings);

		//! Sets the trigger used to compute the timewindow to calculate
		virtual Status computeMagnitude(
			double amplitude,   // in micrometers per second
			double period,      // in seconds
			double delta,       // in degrees
			double depth,       // in kilometers
			double &value) = 0; // resulting magnitude value

		//! Estimates the Mw magnitude of a given magnitude
		//! The default implementation returns MwEstimationNotSupported
		virtual Status estimateMw(
			double magnitude,   // input magnitude
			double &estimation, // output Mw estimation
			double &stdError);  // output standard error

		void setCorrectionCoefficients(double a, double b);

		//! Corrects the magnitudes based on a a configured
		//! linear and constant correction:
		//! out = a*val + b
		//! where a = linearCorrection and b = constantCorrection
		double correctMagnitude(double val) const;


	private:
		double _linearCorrection;
		double _constantCorrection;
		std::string _type;
};


DEFINE_INTERFACE_FACTORY(MagnitudeProcessor);


}

}


#define REGISTER_MAGNITUDEPROCESSOR(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Processing::MagnitudeProcessor, Class> __##Class##InterfaceFactory__(Service)


#endif
