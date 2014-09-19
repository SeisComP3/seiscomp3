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



#ifndef __SEISCOMP_PROCESSING_SECONDARYPICKER_H__
#define __SEISCOMP_PROCESSING_SECONDARYPICKER_H__


#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/processing/timewindowprocessor.h>
#include <boost/function.hpp>



namespace Seiscomp {

namespace Processing {


DEFINE_SMARTPOINTER(SecondaryPicker);


class SC_SYSTEM_CLIENT_API SecondaryPicker : public TimeWindowProcessor {
	DECLARE_SC_CLASS(SecondaryPicker);

	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		//! Configuration structure to store processing dependent
		//! settings
		struct Config {
			double noiseBegin;  // start of the noise window to initialize filters
			double signalBegin; // start time relative to P pick
			double signalEnd;   // end time relative to P pick
		};

		struct Result {
			const Record *record;
			double        snr;
			Core::Time    time;
			double        timeLowerUncertainty;
			double        timeUpperUncertainty;
			std::string   phaseCode;
			std::string   filterID;
			OPT(double)   slowness;
			OPT(double)   backAzimuth;
		};

		struct Trigger {
			Trigger()
			: onsetLowerUncertainty(-1), onsetUpperUncertainty(-1), snr(-1) {}

			Core::Time    onset;
			double        onsetLowerUncertainty;
			double        onsetUpperUncertainty;
			OPT(double)   slowness;
			OPT(double)   backAzimuth;
			double        snr;
		};


		typedef boost::function<void (const SecondaryPicker*,
		                              const Result &)> PublishFunc;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		SecondaryPicker();

		//! D'tor
		~SecondaryPicker();


	// ----------------------------------------------------------------------
	//  Configuration Interface
	// ----------------------------------------------------------------------
	public:
		//! Returns the methodID of the method used to determine
		//! the secondary pick.
		virtual const std::string &methodID() const = 0;

		//! Set the start of the noise window relative to the trigger
		void setNoiseStart(double start) { _config.noiseBegin = start; }

		//! Set the start of the signal window relative to the trigger.
		void setSignalStart(double start)  { _config.signalBegin = start; }

		//! Set the end of the signal window relative to the trigger.
		void setSignalEnd(double end)  { _config.signalEnd = end; }

		//! Returns the current configuration
		const Config &config() const { return _config; }

		//! This method has to be called when all configuration
		//! settings has been set to calculate the timewindow.
		void computeTimeWindow();

		void reset();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		//! Sets the trigger used to compute the timewindow to calculate
		//! the amplitude.
		//! Once a trigger has been set all succeeding calls will fail.
		void setTrigger(const Trigger& trigger) throw(Core::ValueException);
		const Trigger &trigger() const { return _trigger; }

		void setPublishFunction(const PublishFunc& func);

		void setReferencingPickID(const std::string&);
		const std::string& referencingPickID() const;


	// ----------------------------------------------------------------------
	//  Protected Interface
	// ----------------------------------------------------------------------
	protected:
		//! Zero gap tolerance!
		//! The default implementation does not tolerate gaps and does not
		//! handle them.
		bool handleGap(Filter *filter, const Core::TimeSpan& span,
		               double lastSample, double nextSample,
		               size_t missingSamples);


		//! This method is called when a pick has to be published
		void emitPick(const Result &result);


	// ----------------------------------------------------------------------
	//  Protected Members
	// ----------------------------------------------------------------------
	protected:
		Trigger _trigger;

		// config
		Config _config;


	// ----------------------------------------------------------------------
	//  Private Members
	// ----------------------------------------------------------------------
	private:
		PublishFunc _func;
		std::string _pickID;
};


DEFINE_INTERFACE_FACTORY(SecondaryPicker);


}

}


#define REGISTER_SECONDARYPICKPROCESSOR(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::Processing::SecondaryPicker, Class> __##Class##InterfaceFactory__(Service)


#endif
