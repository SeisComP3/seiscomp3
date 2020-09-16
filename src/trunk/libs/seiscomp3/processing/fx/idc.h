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


#ifndef SEISCOMP_PROCESSING_FX_IDC_H
#define SEISCOMP_PROCESSING_FX_IDC_H


#include <seiscomp3/processing/fx.h>

#include <boost/array.hpp>


namespace Seiscomp {
namespace Processing {


/**
 * @brief The IDCFeatureExtraction class implements detection feature
 * extraction as done at IDC/CTBTO in DFX on three-component stations.
 *
 * The algorithm computes polarization attributes for a three-component station
 * using a modification to the Jurkevics [1] algorithm. Some of these
 * attributes are then used to determine detection azimuth
 * (seazp = P-type azimuth in degrees), detection slowness and azimuth/slowness
 * uncertainties (inang1 = emergence (incidence) angle and
 * rect = rectilinearity). The parts of the algorithm/code that calculates
 * S-type and LR-type polarization attributes (and the input only required to
 * calculate them) are skipped here as those attributes are are not used to
 * determine detection azimuth and detection slowness. The interval analysis
 * of the noise is also skipped as it is the basis for the three-component
 * amplitude which we are not interested in.
 *
 * [1] Jurkevics, Andy (1988), "Polarization Analysis of Three-Component
 *     Array Data", Bull. Seism. Soc. Am., 78, 1725-1743
 */
class IDCFeatureExtraction : public FX {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		IDCFeatureExtraction();


	// ----------------------------------------------------------------------
	//  Processor interface
	// ----------------------------------------------------------------------
	public:
		virtual bool setup(const Settings &settings);


	// ----------------------------------------------------------------------
	//  WaveformProcessor interface
	// ----------------------------------------------------------------------
	public:
		//! Intercepts feeding records and sorts each record into the
		//! corresponding component slot.
		virtual bool feed(const Record *rec);


	// ----------------------------------------------------------------------
	//  FX interface
	// ----------------------------------------------------------------------
	public:
		virtual void finalizePick(DataModel::Pick *pick) const;


	// ----------------------------------------------------------------------
	//  Private methods and members
	// ----------------------------------------------------------------------
	private:
		void extractFX(double *data[3], size_t n);
		void setDefault();


	private:
		enum { NCOMPS = 3 };

		struct Component {
			Component() : buffer(-1), complete(false), finished(false) {}
			RingBuffer buffer;
			bool complete;
			bool finished;
		};

		class ThreeC : public boost::array<Component, NCOMPS> {
			public:
				bool isComplete() const {
					for ( size_t i = 0; i < size(); ++i ) {
						if ( !at(i).complete )
							return false;
					}
					return true;
				}

				bool isFinished() const {
					for ( size_t i = 0; i < size(); ++i ) {
						if ( !at(i).finished )
							return false;
					}
					return true;
				}
		};

		struct Result {
			double rectiLinearity;
			double backAzimuth;
			double backAzimuthUncertainty;
			double slowness;
			double slownessUncertainty;
		};

		ThreeC      _threeC;
		int         _fOrder;
		double      _fLo, _fHi;
		double      _polarWindowLength;
		double      _polarWindowOverlap;
		double      _polarAlpha; // Default = 0.3
		double      _polarDs;    // Default = 0.03
		double      _polarDk;    // Default = 0.1
		bool        _dumpData;
		OPT(Result) _result;
};


}
}


#endif
