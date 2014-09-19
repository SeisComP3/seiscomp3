/************************************************************************
 *                                                                      *
 * Copyright (C) 2012 OVSM/IPGP                                         *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * This program is part of 'Projet TSUAREG - INTERREG IV Caraïbes'.     *
 * It has been co-financed by the European Union and le Ministère de    *
 * l'Ecologie, du Développement Durable, des Transports et du Logement. *
 *                                                                      *
 ************************************************************************/


#ifndef __IPGP_MD_PLUGIN_H__
#define __IPGP_MD_PLUGIN_H__


#include <seiscomp3/core/plugin.h>
#include <seiscomp3/processing/amplitudeprocessor.h>
#include <seiscomp3/processing/magnitudeprocessor.h>


using namespace Seiscomp;
using namespace Seiscomp::Processing;

class SC_SYSTEM_CLIENT_API AmplitudeProcessor_Md : public AmplitudeProcessor {

	DECLARE_SC_CLASS(AmplitudeProcessor_Md);

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		AmplitudeProcessor_Md();
		AmplitudeProcessor_Md(const Core::Time& trigger);

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		virtual void initFilter(double fsamp);
		virtual bool setup(const Settings& settings);
		virtual int capabilities() const;
		virtual IDList capabilityParameters(Capability cap) const;
		virtual bool setParameter(Capability cap, const std::string& value);

	protected:
		// ------------------------------------------------------------------
		//  Protected interface
		// ------------------------------------------------------------------
		bool deconvolveData(Response* resp, DoubleArray& data,
		                    int numberOfIntegrations);

		/**
		 * @brief Computes the amplitude of data in the range[i1, i2],
		 *        calculates Coda phase by using the difference value between
		 *        SNR from the start and SNR from the end of the window.
		 * @param data the waveform data
		 * @param i1 start index in data (trigger + config.signalBegin)
		 * @param i2 end index in data (trigger + config.signalEnd)
		 * @param si1 start index of the amplitude search window
		 * @param si2 end index of the amplitude search window
		 * @param offset the computed noise offset
		 * @param dt the picked data index (can be a subindex if required)
		 *		     the dt.begin and dt.end are the begin/end of the timewindow
		 *		     in samples relativ to the picked index. dt.begin and dt.end
		 *		     do not need to be in order, they are ordered afterwards
		 *		     automatically. The default values for begin/end are 0.
		 * @param amplitude the picked amplitude value
		 * @param period the period in seconds between the P and Coda phase
		 * @param snr signal noise ratio
		 * @return true if no error
		 **/
		bool computeAmplitude(const DoubleArray& data, size_t i1, size_t i2,
		                      size_t si1, size_t si2, double offset,
		                      AmplitudeIndex* dt, AmplitudeValue* amplitude,
		                      double* period, double* snr);

		double timeWindowLength(double distance) const;

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		bool _computeAbsMax;
		bool _isInitialized;
};

class SC_SYSTEM_CLIENT_API MagnitudeProcessor_Md : public MagnitudeProcessor {

	DECLARE_SC_CLASS(MagnitudeProcessor_Md);

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		MagnitudeProcessor_Md();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		bool setup(const Settings& settings);
		Status computeMagnitude(double amplitude, double period,
		                        double delta, double depth, double& value);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		double _linearCorrection;
		double _constantCorrection;
};

#endif
