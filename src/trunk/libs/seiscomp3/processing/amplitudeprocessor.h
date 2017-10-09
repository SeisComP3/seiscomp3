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



#ifndef __SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_H__
#define __SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_H__

#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/processing/timewindowprocessor.h>
#include <seiscomp3/math/filter/seismometers.h>
#include <seiscomp3/client.h>
#include <boost/function.hpp>


namespace Seiscomp {

namespace Processing {


DEFINE_SMARTPOINTER(AmplitudeProcessor);

class SC_SYSTEM_CLIENT_API AmplitudeProcessor : public TimeWindowProcessor {
	DECLARE_SC_CLASS(AmplitudeProcessor);

	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		//! Amplitude calculation capabilities
		enum Capability {
			//! No supported capabilities
			NoCapability  = 0x0000,
			//! Supports different amplitude measure types
			MeasureType   = 0x0001,
			//! Supports different amplitude combiner if
			//! the amplitude is measured on different
			//! components
			Combiner      = 0x0002,
			CapQuantity
		};

		//! Configuration structure to store processing dependent
		//! settings
		struct Config {
			double noiseBegin; /* default: -35 */
			double noiseEnd; /* default: -5 */
			double signalBegin; /* default: -5 */
			double signalEnd; /* default: 30 */
			double snrMin; /* default: 3 */

			double minimumDistance; /* default: 0 */
			double maximumDistance; /* default: 180 */
			double minimumDepth; /* default: 0 */
			double maximumDepth; /* default: 700 */

			double respTaper;
			double respMinFreq;
			double respMaxFreq;

			Math::SeismometerResponse::WoodAnderson::Config woodAndersonResponse;
		};

		struct AmplitudeIndex {
			double  index;
			double  begin;
			double  end;
		};

		struct AmplitudeTime {
			AmplitudeTime() : begin(0), end(0) {}
			AmplitudeTime(const Core::Time &ref)
			: reference(ref), begin(0), end(0) {}
			Core::Time  reference;
			double      begin;
			double      end;
		};

		struct AmplitudeValue {
			double      value;
			OPT(double) lowerUncertainty;
			OPT(double) upperUncertainty;
		};

		struct Result {
			StreamComponent component;
			const Record   *record;
			AmplitudeValue  amplitude;
			AmplitudeTime   time;
			double          period;
			double          snr;
		};

		typedef std::vector<std::string> IDList;

		typedef boost::function<void (const AmplitudeProcessor*,
		                              const Result &)> PublishFunc;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		AmplitudeProcessor();
		AmplitudeProcessor(const std::string& type);
		AmplitudeProcessor(const Core::Time& trigger);
		AmplitudeProcessor(const Core::Time& trigger, const std::string& type);

		//! D'tor
		~AmplitudeProcessor();


	// ----------------------------------------------------------------------
	//  Configuration Interface
	// ----------------------------------------------------------------------
	public:
		//! Set the start of the noise window relative to the trigger
		void setNoiseStart(double start) { _config.noiseBegin = start; }

		//! Set the end of the noise window relative to the trigger
		void setNoiseEnd(double end)  { _config.noiseEnd = end; }

		//! Set the start of the signal window relative to the trigger
		void setSignalStart(double start)  { _config.signalBegin = start; }

		//! Set the end of the signal window relative to the trigger
		void setSignalEnd(double end)  { _config.signalEnd = end; }

		void setMinSNR(double snr) { _config.snrMin = snr; }

		//! Sets the minimum distance to calculate amplitudes for
		void setMinDist(double dist) { _config.minimumDistance = dist; }

		//! Sets the maximum distance to calculate amplitudes for
		void setMaxDist(double dist) { _config.maximumDistance = dist; }

		//! Sets the minimum depth to calculate amplitudes for
		void setMinDepth(double depth) { _config.minimumDepth = depth; }

		//! Sets the maximum depth to calculate amplitudes for
		void setMaxDepth(double depth) { _config.maximumDepth = depth; }

		//! Sets a configuration
		void setConfig(const Config &config) { _config = config; }

		//! Returns the current configuration
		const Config &config() const { return _config; }

		//! Sets whether amplitude updates are enabled or not
		void setUpdateEnabled(bool);
		bool isUpdateEnabled() const;

		void setReferencingPickID(const std::string&);
		const std::string& referencingPickID() const;


	// ----------------------------------------------------------------------
	//  Query interface
	//  This interface is important to be implemented for interactive
	//  analysis.
	// ----------------------------------------------------------------------
	public:
		//! Returns a child processor for a specific component. The
		//! returned pointer must not be deleted. The default
		//! implementation returns 'this' if comp matched the usedComponent.
		virtual const AmplitudeProcessor *componentProcessor(Component comp) const;

		//! Returns the internally processed data which is used to
		//! measure the amplitude.
		//! The default implementation returns
		//! TimeWindowProcessor::continuousData.
		//! The returned pointer must not be managed by a smartpointer or
		//! deleted. It points to a static member of this class.
		virtual const DoubleArray *processedData(Component comp) const;

		//! Returns the implementations capabilities
		//! The default implementation returns NoCapability (0)
		virtual int capabilities() const;

		//! Queries for a capability.
		bool supports(Capability capability) const;

		//! Returns a list of tokens valid for a certain capability
		//! The default implementation returns an empty list
		virtual IDList capabilityParameters(Capability cap) const;

		//! Sets a processing parameter. Value must be part of the
		//! list returned by capabilityParameters(cap).
		//! The default implementation returns always false.
		virtual bool setParameter(Capability cap, const std::string &value);


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		//! Reprocesses the current data chunk and searches for amplitudes
		//! only in the given optional time window relative to trigger time
		//! (if supported by the implementation).
		virtual void reprocess(OPT(double) searchBegin = Core::None,
		                       OPT(double) searchEnd = Core::None);

		//! Resets the amplitude processor and deletes all data
		//! and noise amplitudes
		virtual void reset();

		//! This method has to be called when all configuration
		//! settings has been set to calculate the timewindow
		void computeTimeWindow();

		//! Sets up the amplitude processor. By default it reads whether
		//! to use response information or not.
		virtual bool setup(const Settings &settings);

		//! Sets the trigger used to compute the timewindow to calculate
		//! the amplitude
		//! Once a trigger has been set all succeeding calls will fail.
		virtual void setTrigger(const Core::Time& trigger);

		Core::Time trigger() const;

		void setPublishFunction(const PublishFunc& func);

		//! Returns the computed noise offset
		OPT(double) noiseOffset() const;

		//! Returns the computed noise amplitude
		OPT(double) noiseAmplitude() const;

		//! Returns the type of amplitude to be calculated
		const std::string& type() const;

		//! Returns the unit of amplitude to be calculated
		const std::string& unit() const;

		void setHint(ProcessingHint hint, double value);

		//! Dumps the record data into an ascii file
		void writeData() const;


	// ----------------------------------------------------------------------
	//  Protected Interface
	// ----------------------------------------------------------------------
	protected:
		//! Sets the unit of the computed amplitude.
		void setUnit(const std::string &unit);

		virtual void process(const Record *record);

		virtual bool handleGap(Filter *filter, const Core::TimeSpan&,
		                       double lastSample, double nextSample,
		                       size_t missingSamples);

		//! Method to prepare the available data just before the noise
		//! and amplitude calculation takes place. This method can be
		//! reimplemented to convert the data into velocity if the input
		//! signal unit is e.g. meter per second squared.
		virtual void prepareData(DoubleArray &data);

		//! Deconvolve the data using the sensor response. The default
		//! implementation simply calls:
		//! resp->deconvolveFFT(data, _fsamp, 60.0, 0.00833333, 0, numberOfIntegrations)
		//! which corresponds to a period lowpass (freq highpass) filter
		//! of 120 seconds.
		virtual bool deconvolveData(Response *resp, DoubleArray &data,
		                            int numberOfIntegrations);

		//! -----------------------------------------------------------------------
		//! Computes the amplitude of data in the range[i1, i2].
		//! -----------------------------------------------------------------------
		//! Input:
		//! -----------------------------------------------------------------------
		//!  - data: the waveform data
		//!  - offset: the computed noise offset
		//!  - i1: start index in data (trigger + config.signalBegin)
		//!  - i2: end index in data (trigger + config.signalEnd)
		//!  - si1: start index of the amplitude search window
		//!  - si2: end index of the amplitude search window
		//!  NOTE: si1 and si2 are guaranteed to be in range [i1,i2] if
		//!  the default AmplitudeProcessor::process method is called (especially
		//!  when reimplemented).
		//! -----------------------------------------------------------------------
		//! Output:
		//! -----------------------------------------------------------------------
		//!  - dt: the picked data index (can be a subindex if required)
		//!        the dt.begin and dt.end are the begin/end of the timewindow
		//!        in samples relativ to the picked index. dt.begin and dt.end
		//!        do not need to be in order, they are ordered afterwards
		//!        automatically. The default values for begin/end are 0.
		//!  - amplitude: the picked amplitude value with optional uncertainties.
		//!  - period: the period in samples and not seconds (-1 if not calculated)
		//!  - snr: signal-to-noise ratio
		//! -----------------------------------------------------------------------
		virtual bool computeAmplitude(const DoubleArray &data,
		                              size_t i1, size_t i2,
		                              size_t si1, size_t si2,
		                              double offset,
		                              AmplitudeIndex *dt,
		                              AmplitudeValue *amplitude,
		                              double *period, double *snr) = 0;

		//! Computes the noise of data in the range [i1,i2] and returns the offfset and
		//! the amplitude in 'offset' and 'amplitude'
		//! The default implementation takes the median of the data as offset and
		//! twice the rms regarding the offset as amplitude
		virtual bool computeNoise(const DoubleArray &data, int i1, int i2, double *offset, double *amplitude);

		//! Computes the timewindow length when a distance hint has been set.
		//! The default implementation return _config.signalEnd
		virtual double timeWindowLength(double distance) const;

		//! This method gets called when an amplitude has to be published
		void emitAmplitude(const Result &result);


	private:
		void init();
		void process(const Record *record, const DoubleArray &filteredData);


	// ----------------------------------------------------------------------
	//  Protected Members
	// ----------------------------------------------------------------------
	protected:
		Core::Time       _trigger;

		// User defined amplitude search window
		OPT(double)      _searchBegin, _searchEnd;

		// pre-arrival offset and rms
		OPT(double)      _noiseOffset, _noiseAmplitude, _lastAmplitude;

		double           _snrMax, _snrRMS;
		bool             _enableUpdates;
		bool             _enableResponses;

		// config
		Config           _config;

		std::string      _type;
		std::string      _unit;

		std::string      _pickID;

		bool             _responseApplied;

	// ----------------------------------------------------------------------
	//  Private Members
	// ----------------------------------------------------------------------
	private:
		PublishFunc _func;
};


DEFINE_INTERFACE_FACTORY(AmplitudeProcessor);

}

}


#define REGISTER_AMPLITUDEPROCESSOR(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Processing::AmplitudeProcessor, Class> __##Class##InterfaceFactory__(Service)


#endif
