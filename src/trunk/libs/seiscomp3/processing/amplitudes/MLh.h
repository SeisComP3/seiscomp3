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



#ifndef __SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_MLH_H__
#define __SEISCOMP_PROCESSING_AMPLITUDEPROCESSOR_MLH_H__

#include <seiscomp3/processing/amplitudes/ML.h>


namespace Seiscomp {

namespace Processing {


//! Wrapper class that allows access to protected methods for
//! the proxy (see below).
class SC_SYSTEM_CLIENT_API AmplitudeProcessor_MLh : public AbstractAmplitudeProcessor_ML {
	public:
		AmplitudeProcessor_MLh();

	friend class AmplitudeProcessor_ML2h;
};


//! Proxy amplitude processor that holds to MLv processors to calculate
//! the amplitudes on both horizontals and then averages the result.
//! This class does not handle waveforms itself. It directs them to the
//! corresponding amplitude processors instead.
class SC_SYSTEM_CLIENT_API AmplitudeProcessor_ML2h : public AmplitudeProcessor {
	DECLARE_SC_CLASS(AmplitudeProcessor_ML2h);

	public:
		AmplitudeProcessor_ML2h();
		AmplitudeProcessor_ML2h(const Core::Time &trigger);

	public:
		int capabilities() const;
		IDList capabilityParameters(Capability cap) const;
		bool setParameter(Capability cap, const std::string &value);

		bool setup(const Settings &settings);

		void setTrigger(const Core::Time& trigger);

		void computeTimeWindow();

		void reset();
		void close();

		bool feed(const Record *record);

		bool computeAmplitude(const DoubleArray &data,
		                      size_t i1, size_t i2,
		                      size_t si1, size_t si2,
		                      double offset,
		                      AmplitudeIndex *dt, AmplitudeValue *amplitude,
		                      double *period, double *snr);

		const AmplitudeProcessor *componentProcessor(Component comp) const;
		const DoubleArray *processedData(Component comp) const;

		void reprocess(OPT(double) searchBegin, OPT(double) searchEnd);


	protected:
		double timeWindowLength(double distance) const;


	private:
		void newAmplitude(const AmplitudeProcessor *proc,
		                  const AmplitudeProcessor::Result &res);


	private:
		struct ComponentResult {
			AmplitudeValue value;
			AmplitudeTime  time;
		};

		enum CombinerProc {
			TakeMin,
			TakeMax,
			TakeAverage,
			TakeGeometricMean
		};

		mutable AmplitudeProcessor_MLh _ampE, _ampN;
		CombinerProc                   _combiner;
		OPT(ComponentResult)           _results[2];
};


}

}


#endif
