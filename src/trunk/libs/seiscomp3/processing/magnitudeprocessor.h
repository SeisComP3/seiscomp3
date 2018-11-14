/***************************************************************************
 *   Copyright (C) by GFZ Potsdam, gempa GmbH                              *
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

namespace DataModel {

class Amplitude;
class StationMagnitude;
class Origin;
class SensorLocation;

}

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
				//! Given amplitude SNR is out of range to continue processing
				SNROutOfRange,
				//! Either the origin or the sensor location hasn't been set
				//! in call to computeMagnitude
				MetaDataRequired,
				//! The epicentre is out of supported regions
				EpicenterOutOfRegions,
				//! The receiver is out of supported regions
				ReceiverOutOfRegions,
				//! The entire raypath does not lie entirely in the supported
				//! regions
				RayPathOutOfRegions,
				//! The unit of the input amplitude was not understood
				InvalidAmplitudeUnit,
				//! The amplitude object was missing
				MissingAmplitudeObject,
				//! The estimation of the Mw magnitude is not supported
				MwEstimationNotSupported,
				//! Unspecified error
				IncompleteConfiguration,
				//! Unspecified error
				Error
			),
			ENAMES(
				"OK",
				"amplitude out of range",
				"depth out of range",
				"distance out of range",
				"period out of range",
				"signal-to-noise ratio out of range",
				"meta data required",
				"epicenter out of regions",
				"receiver out of regions",
				"ray path out of regions",
				"invalid amplitude unit",
				"missing amplitude object",
				"Mw estimation not supported",
				"configuration incomplete",
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
		const std::string &type() const;

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
		 *        @hypocenter, @receiver and @amplitude were not present.
		 * @param amplitudeValue The amplitude value without unit. The unit is
		                         implicitly defined by the requested amplitude
		 *                       type.
		 * @param unit The unit of the amplitude.
		 * @param period The measured period of the amplitude in seconds.
		 * @param snr The measured SNR of the amplitude.
		 * @param delta The distance from the epicenter in degrees.
		 * @param depth The depth of the hypocenter in kilometers.
		 * @param hypocenter The optional origin which describes the hypocenter.
		 * @param receiver The sensor location meta-data of the receiver.
		 * @param amplitude The optional amplitude object from which the values
		 *                  were extracted.
		 * @param value The return value, the magnitude.
		 * @return The status of the computation.
		 */
		virtual Status computeMagnitude(double amplitudeValue, const std::string &unit,
		                                double period, double snr,
		                                double delta, double depth,
		                                const DataModel::Origin *hypocenter,
		                                const DataModel::SensorLocation *receiver,
		                                const DataModel::Amplitude *amplitude,
		                                double &value) = 0;

		/**
		 * @brief When computeMagnitude return an error the computed magnitude
		 *        value might nevertheless contain a meaningful value. E.g. if
		 *        the distance is out of range according to the defined rules,
		 *        the computation for a lower distance might still result in
		 *        valid values. This function indicates whether the returned
		 *        value should be treated as valid magnitude or not, even if an
		 *        error was returned. This function's return value must not be
		 *        used when computeMagnitude returned OK. A valid return value
		 *        is only provided in case the computation failed. To make it
		 *        clear: this function can only be called after
		 *        computeMagnitude and only if computeMagnitude(...) != OK.
		 * @return Whether the computed magnitude is a valid value or has to be
		 *         ignored. The default implementation returns false.
		 */
		virtual bool treatAsValidMagnitude() const;

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

		/**
		 * @brief Allows to finalize a magnitude object as created by
		 *        client code.
		 *
		 * This method will usually be called right before the magnitude will
		 * be stored or sent and inside the emit handler. It allows processors
		 * to set specific attributes or to add comments.
		 * The default implementation does nothing.
		 * @param magnitude The magnitude to be finalized
		 */
		virtual void finalizeMagnitude(DataModel::StationMagnitude *magnitude) const;


	protected:
		/**
		 * @brief Converts an amplitude value in an input unit to a value in
		 *        an output unit, e.g. mm/s -> nm/s.
		 * @param amplitude The input value which will be changed
		 * @param amplitudeUnit The unit associated with the input amplitude value
		 * @param desiredAmplitudeUnit The desired amplitude unit of the output
		 *                             value.
		 * @return Success or not
		 */
		bool convertAmplitude(double &amplitude,
		                      const std::string &amplitudeUnit,
		                      const std::string &desiredAmplitudeUnit) const;


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
