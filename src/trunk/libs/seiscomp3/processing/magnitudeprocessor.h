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
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/sensorlocation.h>
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
				//! Either the origin or the sensor location hasn't been set
				//! in call to computeMagnitude
				MetaDataRequired,
				//! The epicentre is out of supported regions
				EpicentreOutOfRegions,
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
				"meta data required",
				"epicentre out of regions",
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

		/**
		 * @brief Computes the magnitude from an amplitude. The method signature
		 *        has changed with API version >= 11. Prior to that version,
		 *        @hypocenter and @receiver were not present.
		 * @param amplitude The amplitude value without unit. The unit is
		 *                  implicitly defined by the requested amplitude
		 *                  type.
		 * @param period The measured period of the amplitude in seconds.
		 * @param delta The distance from the epicenter in degrees.
		 * @param depth The depth of the hypocenter in kilometers.
		 * @param hypocenter The optional origin which describes the hypocenter.
		 * @param receiver The sensor location meta-data of the receiver.
		 * @param value The return value, the magnitude.
		 * @return The status of the computation.
		 */
		virtual Status computeMagnitude(double amplitude, double period,
		                                double delta, double depth,
		                                const DataModel::Origin *hypocenter,
		                                const DataModel::SensorLocation *receiver,
		                                double &value) = 0;

		/**
		 * @brief Estimates the Mw magnitude from a given magnitude. The
		 *        default implementation returns MwEstimationNotSupported.
		 * @param magnitude Input magnitude value.
		 * @param estimation Resulting Mw estimation.
		 * @param stdError Resulting standard error.
		 * @return The status of the computation.
		 */
		virtual Status estimateMw(double magnitude, double &estimation,
		                          double &stdError);

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
