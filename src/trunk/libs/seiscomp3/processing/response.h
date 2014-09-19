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


#ifndef __SEISCOMP_PROCESSING_RESPONSE_H__
#define __SEISCOMP_PROCESSING_RESPONSE_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/typedarray.h>
#include <seiscomp3/math/restitution/transferfunction.h>
#include <seiscomp3/client.h>

#include <vector>


namespace Seiscomp {
namespace Processing  {


DEFINE_SMARTPOINTER(Response);

class SC_SYSTEM_CLIENT_API Response : public Core::BaseObject {
	DECLARE_CASTS(Response);


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		// C'tor
		Response();
		// D'tor
		virtual ~Response();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		//! Deconvolves data in the frequency domain.
		//! This method incorporates the gain. If no transfer function
		//! can be retrieved false is returned.
		bool deconvolveFFT(int n, float *inout, double fsamp,
		                   double cutoff,
		                   double min_freq, double max_freq,
		                   int numberOfIntegrations = 0);

		bool deconvolveFFT(int n, double *inout, double fsamp,
		                   double cutoff,
		                   double min_freq, double max_freq,
		                   int numberOfIntegrations = 0);

		bool deconvolveFFT(FloatArray &inout, double fsamp,
		                   double cutoff,
		                   double min_freq, double max_freq,
		                   int numberOfIntegrations = 0);

		bool deconvolveFFT(DoubleArray &inout, double fsamp,
		                   double cutoff,
		                   double min_freq, double max_freq,
		                   int numberOfIntegrations = 0);

		//! Returns a transfer function that can be used to deconvolve the
		//! data. The transfer function does not incorporate the gain.
		//! @param numberOfIntegrations How often to integrate. In case of
		//!                             'poles and zeros' this will push n
		//!                             additional zeros to 'zeros'.
		virtual Math::Restitution::FFT::TransferFunction *
			getTransferFunction(int numberOfIntegrations = 0);
};


DEFINE_SMARTPOINTER(ResponsePAZ);

class SC_SYSTEM_CLIENT_API ResponsePAZ : public Response {
	DECLARE_CASTS(ResponsePAZ);


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		typedef std::vector<Math::Complex> ComplexArray;
		typedef ComplexArray Poles;
		typedef ComplexArray Zeros;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		// C'tor
		ResponsePAZ();
		// D'tor
		virtual ~ResponsePAZ();


	// ----------------------------------------------------------------------
	//  Public attributes
	// ----------------------------------------------------------------------
	public:
		void setNormalizationFactor(const OPT(double)& normalizationFactor);
		double normalizationFactor() const throw(Seiscomp::Core::ValueException);

		void setNormalizationFrequency(const OPT(double)& normalizationFrequency);
		double normalizationFrequency() const throw(Seiscomp::Core::ValueException);

		void setPoles(const Poles& poles);
		const Poles& poles() const;

		void setZeros(const Zeros& zeros);
		const Zeros& zeros() const;

		void convertFromHz();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		Math::Restitution::FFT::TransferFunction *
			getTransferFunction(int numberOfIntegrations = 0);


	// ----------------------------------------------------------------------
	//  Private interface
	// ----------------------------------------------------------------------
	private:
		OPT(double) _normalizationFactor;
		OPT(double) _normalizationFrequency;

		Poles       _poles;
		Zeros       _zeros;
};


}
}

#endif
