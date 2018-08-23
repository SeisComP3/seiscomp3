/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_CUSTOM_AMPLITUDE_NUTTLI_H__
#define __SEISCOMP_CUSTOM_AMPLITUDE_NUTTLI_H__


#include <seiscomp3/math/filter.h>
#include <seiscomp3/processing/amplitudeprocessor.h>
#include <seiscomp3/seismology/ttt.h>


namespace {


/**
 * @brief The MNAmplitude class implements the AmplitudeProcessor
 *        interface which computes an amplitude from waveform data.
 */
class MNAmplitude : public Seiscomp::Processing::AmplitudeProcessor {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		MNAmplitude();


	// ----------------------------------------------------------------------
	//  AmplitudeProcessor interface
	// ----------------------------------------------------------------------
	public:
		virtual void setHint(ProcessingHint hint, double value);

		virtual void setEnvironment(const Seiscomp::DataModel::Origin *hypocenter,
		                            const Seiscomp::DataModel::SensorLocation *receiver,
		                            const Seiscomp::DataModel::Pick *pick);

		virtual void finalizeAmplitude(Seiscomp::DataModel::Amplitude *amplitude) const;


	protected:
		/**
		 * @brief Sets up the processor by reading configuration parameters
		 *        from the settings object. If false is returned then
		 *        processing will be aborted.
		 * @param settings The settings object
		 * @return true on success, false otherwise
		 */
		virtual bool setup(const Seiscomp::Processing::Settings &settings);

		/**
		 * @brief Overrides AmplitudeProcessor::prepareData
		 *
		 * The default implementation converts almost any data to velocity
		 * by applying the full responses. According to the specification
		 * only sensitivity correction should be applied and the final
		 * amplitude value should be corrected with the response at the
		 * computed period. This function will be called prior to
		 * @computeAmplitude.
		 *
		 * @param data The time window data for the amplitude
		 */
		virtual void prepareData(Seiscomp::DoubleArray &data);

		/**
		 * @brief Computes the noise of data in the range [i1,i2] and returns
		 * the offfset and the amplitude in 'offset' and 'amplitude'.
		 * The amplitude is calculated in the same way as the signal amplitude.
		 * @param data The complete data of the requested time window
		 * @param i1 The start index of the noise window
		 * @param i2 The end index of the noise window
		 * @param offset The data offset returned to the caller
		 * @param amplitude The amplitude returned to the caller
		 */
		virtual bool computeNoise(const Seiscomp::DoubleArray &data,
		                          int i1, int i2,
		                          double *offset, double *amplitude);

		/**
		 * @brief Computes the amplitude, period and snr for a given time window.
		 *
		 * This function can be called to just compute a portion of the initially
		 * requested data. E.g. scolv will do that when the user selects a
		 * portion of the time window.
		 * @param data The complete data of the requested time window.
		 * @param i1 The start index in data (trigger + config.signalBegin)
		 * @param i2 The end index in data (trigger + config.signalEnd)
		 * @param si1 The start index of the amplitude search window
		 * @param si2 The end index of the amplitude search window
		 * @param offset The computed noise offset
		 * @param dt The result index of the computed amplitude.
		 * @param amplitude The amplitude with uncertainties
		 * @param period The period in s
		 * @param snr The signal-to-noise ratio
		 * @return true if success, false otherwise
		 */
		virtual bool computeAmplitude(const Seiscomp::DoubleArray &data,
		                              size_t i1, size_t i2,
		                              size_t si1, size_t si2,
		                              double offset,
		                              AmplitudeIndex *dt,
		                              AmplitudeValue *amplitude,
		                              double *period, double *snr);


	// ----------------------------------------------------------------------
	//  Private types and methods
	// ----------------------------------------------------------------------
	private:
		MAKEENUM(
			PhaseOrVelocity,
			EVALUES(
				PoV_Undefined,
				PoV_Pg,
				PoV_Pn,
				PoV_P,
				PoV_Sg,
				PoV_Sn,
				PoV_S,
				PoV_Lg,
				PoV_Rg,
				PoV_Vmin,
				PoV_Vmax
			),
			ENAMES(
				"",
				"Pg",
				"Pn",
				"P",
				"Sg",
				"Sn",
				"S",
				"Lg",
				"Rg",
				"Vmin",
				"Vmax"
			)
		);

		typedef Seiscomp::Core::SmartPointer<Filter>::Impl FilterPtr;

		void setDefaults();

		bool readPriorities(PhaseOrVelocity *priorities,
		                    const Seiscomp::Processing::Settings &settings,
		                    const std::string &parameter);

		OPT(double) getDefinedOnset(const PhaseOrVelocity *,
		                            double lat0, double lon0, double depth,
		                            double lat1, double lon1, double dist,
		                            bool left) const;

		OPT(double) getEarliestOnset(double lat0, double lon0, double depth,
		                             double lat1, double lon1, double dist) const;


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		static Seiscomp::TravelTimeTableInterfacePtr _travelTimeTable;

		std::string     _networkCode;
		std::string     _stationCode;
		std::string     _locationCode;

		bool            _useRMS; //!< Whether to use RMS for SNR or not
		double          _Vmin, _Vmax;
		double          _snrWindowSeconds;
		double          _noiseWindowPreSeconds;
		PhaseOrVelocity _signalStartPriorities[EPhaseOrVelocityQuantity];
		PhaseOrVelocity _signalEndPriorities[EPhaseOrVelocityQuantity];
};


}


#endif
