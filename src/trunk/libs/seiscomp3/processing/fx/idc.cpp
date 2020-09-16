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


#define SEISCOMP_COMPONENT FX/IDC
#include <seiscomp3/logging/log.h>
#include <seiscomp3/datamodel/pick.h>
#include <seiscomp3/math/filter.h>
#include <seiscomp3/math/filter/butterworth.h>
#include <seiscomp3/math/geo.h>
#include <seiscomp3/math/mean.h>
#include <seiscomp3/math/matrix3.h>
#include <seiscomp3/math/tensor.h>

#include <fstream>

#include "idc.h"


#define SETUP_PREFIX "fx.IDC."
#define IDC_DEBUG 0


using namespace std;


namespace Seiscomp {
namespace Processing {

namespace {

template <typename T>
T cov(size_t n, const T *data1, const T *data2) {
	T sum = T(0.0);
	for ( size_t i = 0; i < n; ++i, ++data1, ++data2 )
		sum += *data1 * *data2;
	return sum / T(n);
}

}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
IDCFeatureExtraction::IDCFeatureExtraction() {
	setDefault();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void IDCFeatureExtraction::setDefault() {
	setNoiseStart(-30);
	setNoiseEnd(-20.5);
	setSignalStart(-1.5);
	setSignalEnd(4);
	setUsedComponent(Any);
	setMargin(Core::TimeSpan(0,0));
	setFilter(NULL);

	_threeC = ThreeC();
	_dumpData = false;

	_fOrder = 3;
	_fLo = 1;
	_fHi = 3;
	_polarWindowLength = 1.5;
	_polarWindowOverlap = 0.5;
	_polarAlpha = 0.3;
	_polarDs = 0.03;
	_polarDk = 0.10;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool IDCFeatureExtraction::setup(const Settings &settings) {
	setDefault();

	if ( !FX::setup(settings) )
		return false;

	if ( _streamConfig[Vertical].gainUnit != _streamConfig[FirstHorizontal].gainUnit ) {
		SEISCOMP_ERROR("Inconsistent gain units: V(%s) != H1(%s)",
		               _streamConfig[Vertical].gainUnit.c_str(),
		               _streamConfig[FirstHorizontal].gainUnit.c_str());
		return false;
	}

	if ( _streamConfig[FirstHorizontal].gainUnit != _streamConfig[SecondHorizontal].gainUnit ) {
		SEISCOMP_ERROR("Inconsistent gain units: H1(%s) != H2(%s)",
		               _streamConfig[FirstHorizontal].gainUnit.c_str(),
		               _streamConfig[SecondHorizontal].gainUnit.c_str());
		return false;
	}

	settings.getValue(_fOrder, SETUP_PREFIX "filter.order");
	settings.getValue(_fLo, SETUP_PREFIX "filter.loFreq");
	settings.getValue(_fHi, SETUP_PREFIX "filter.hiFreq");

	if ( _fOrder <= 0 ) {
		SEISCOMP_ERROR("Invalid " SETUP_PREFIX "filter.order: must be positive and greater than zero: %d",
		               _fOrder);
		return false;
	}

	settings.getValue(_polarWindowLength, SETUP_PREFIX "polar.window");
	settings.getValue(_polarWindowOverlap, SETUP_PREFIX "polar.overlap");

	if ( _polarWindowLength <= 0 ) {
		SEISCOMP_ERROR("Empty " SETUP_PREFIX "polar.window: must be positive and greater than zero: %f",
		               _polarWindowLength);
		return false;
	}

	if ( _polarWindowOverlap < 0 ) {
		SEISCOMP_ERROR("Negative " SETUP_PREFIX "polar.overlap: must be positive: %f",
		               _polarWindowOverlap);
		return false;
	}

	if ( _polarWindowOverlap >= _polarWindowLength ) {
		SEISCOMP_ERROR(SETUP_PREFIX "polar.overlap must not be larger or equal "
		               SETUP_PREFIX "polar.window");
		return false;
	}

	settings.getValue(_polarAlpha, SETUP_PREFIX "polar.alpha");
	settings.getValue(_polarDs, SETUP_PREFIX "polar.ds");
	settings.getValue(_polarDk, SETUP_PREFIX "polar.dk");

	settings.getValue(_dumpData, SETUP_PREFIX "dump");

	Filter *f = 0;
	if ( _fLo > 0 && _fHi > 0 )
		f = new Math::Filtering::IIR::ButterworthBandpass<double>(_fOrder, _fLo, _fHi);
	else if ( _fLo > 0 )
		f = new Math::Filtering::IIR::ButterworthHighpass<double>(_fOrder, _fLo);
	else if ( _fHi > 0 )
		f = new Math::Filtering::IIR::ButterworthLowpass<double>(_fOrder, _fLo);

	if ( f ) setFilter(f);

	/* Do your setup here and read configuration parameters.
	 * Return false if something went wrong and amplitude calculation should
	 * be canceled or true in case of success.
	 */
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool IDCFeatureExtraction::feed(const Record *rec) {
	size_t component;

	if ( isFinished() ) return false;

	if ( _streamConfig[0].code() == rec->channelCode() )
		component = 0;
	else if ( _streamConfig[1].code() == rec->channelCode() )
		component = 1;
	else if ( _streamConfig[2].code() == rec->channelCode() )
		component = 2;
	else
		return false;

	if ( rec->timeWindow().overlaps(safetyTimeWindow()) ) {
		_threeC[component].buffer.feed(rec);
		if ( _threeC[component].buffer.back()->endTime() >= safetyTimeWindow().endTime() )
			_threeC[component].complete = true;

		if ( _threeC.isComplete() ) {
			GenericRecordPtr recs[NCOMPS];
			double samplingRate = -1;
			for ( size_t i = 0; i < NCOMPS; ++i ) {
				recs[i] = _threeC[i].buffer.continuousRecord<double>();
				if ( !recs[i] ) {
					setStatus(QCError, -1.0);
					return false;
				}

				if ( !recs[i]->timeWindow().overlaps(safetyTimeWindow()) ) {
					setStatus(QCError, i);
					return false;
				}

				if ( !i )
					samplingRate = recs[i]->samplingFrequency();
				else {
					if ( recs[i]->samplingFrequency() != samplingRate ) {
						setStatus(QCError, -2.0);
						return false;
					}
				}
			}

			// Get common time window and cut data
			size_t commonNumberOfSamples = size_t(safetyTimeWindow().length() * samplingRate) + 1;
			double *commonData[NCOMPS];

			for ( size_t i = 0; i < NCOMPS; ++i ) {
				DoubleArray *ar = DoubleArray::Cast(recs[i]->data());
				if ( !ar ) {
					setStatus(Error, -1.0);
					return false;
				}

				double *data = ar->typedData();
				Core::TimeSpan timeOffset = safetyTimeWindow().startTime() - recs[i]->startTime();
				double sampleOffset = floor(double(timeOffset) * samplingRate + 0.5);
				if ( sampleOffset < 0 ) {
					setStatus(Error, -2.0);
					return false;
				}

				size_t ofs = size_t(sampleOffset);
				if ( ofs + commonNumberOfSamples > size_t(ar->size()) ) {
					setStatus(Error, -3.0);
					return false;
				}

				commonData[i] = data + ofs;
			}

			_stream.initialized = true;
			_stream.dataTimeWindow = safetyTimeWindow();
			_stream.fsamp = samplingRate;
			_stream.lastRecord = rec;

			extractFX(commonData, commonNumberOfSamples);

			if ( !isFinished() )
				setStatus(Finished, 0.0);
		}

		return true;
	}
	else {
		if ( rec->startTime() >= safetyTimeWindow().endTime() )
			_threeC[component].finished = true;

		if ( _threeC.isFinished() )
			setStatus(Terminated, 0.0);
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void IDCFeatureExtraction::extractFX(double *data[3], size_t n) {
	/*
	 * The following methods can be used:
	 * - dataTimeWindow()
	 * - samplingFrequency()
	 */

	const int X = SecondHorizontal; // X = East
	const int Y = FirstHorizontal;  // Y = North
	const int Z = Vertical;         // Z = Vertical

	// -----------------------------------------------------------------------
	// Stage 1: Remove gain
	// -----------------------------------------------------------------------
	for ( size_t c = 0; c < NCOMPS; ++c ) {
		double *cdata = data[c];
		if ( _streamConfig[c].gain == 0.0 ) {
			setStatus(MissingGain, c);
			return;
		}

		double scale = 1E9 / _streamConfig[c].gain / 2 / M_PI;
		//double scale = 1.0 / _streamConfig[c].gain;
		for ( size_t i = 0; i < n; ++i ) {
			cdata[i] *= scale;
		}
	}

	// -----------------------------------------------------------------------
	// Stage 2: compute noise and signal windows
	// -----------------------------------------------------------------------
	Core::TimeSpan relativeTrigger = trigger() - dataTimeWindow().startTime();

	// All spans are relative to data time window start
	Core::TimeSpan noiseBegin  = relativeTrigger + Core::TimeSpan(_config.noiseBegin);
	Core::TimeSpan noiseEnd    = relativeTrigger + Core::TimeSpan(_config.noiseEnd);

	// Indices into data arrays
	size_t iNoiseBegin  = size_t(samplingFrequency() * double(noiseBegin));
	size_t iNoiseEnd    = size_t(samplingFrequency() * double(noiseEnd));

	// -----------------------------------------------------------------------
	// Stage 3: Demean w.r.t to noise window
	// -----------------------------------------------------------------------
	size_t noiseSamples = iNoiseEnd - iNoiseBegin;

	if ( noiseSamples  > 0 ) {
		for ( size_t c = 0; c < NCOMPS; ++c ) {
			double *cdata = data[c];
			double mean = Math::Statistics::mean(int(noiseSamples), cdata + iNoiseBegin);
			for ( size_t i = 0; i < n; ++i ) {
				cdata[i] -= mean;
			}
		}
	}

	// -----------------------------------------------------------------------
	// Stage 4: Taper and filter data
	// -----------------------------------------------------------------------
	for ( size_t c = 0; c < NCOMPS; ++c ) {
		double *cdata = data[c];

		// Taper the noise window
		Math::Filtering::cosRamp(n, cdata, iNoiseBegin, iNoiseEnd, n, n);

		if ( _stream.filter ) {
			Filter *filter = _stream.filter->clone();
			filter->setSamplingFrequency(samplingFrequency());
			filter->apply(int(n), cdata);
			delete filter;
		}
	}

	// -----------------------------------------------------------------------
	// Stage 5: Rotate into ZNE
	// -----------------------------------------------------------------------

	// Create rotation matrix to ZNE
	{
		Math::Matrix3d m;
		Math::Vector3d v;

		// Create transposed (inverse) of the orientation matrix
		v.fromAngles(+deg2rad(_streamConfig[X].azimuth),
		             -deg2rad(_streamConfig[X].dip)).normalize();
		m.setColumn(0, v);

		v.fromAngles(+deg2rad(_streamConfig[Y].azimuth),
		             -deg2rad(_streamConfig[Y].dip)).normalize();
		m.setColumn(1, v);

		v.fromAngles(+deg2rad(_streamConfig[Z].azimuth),
		             -deg2rad(_streamConfig[Z].dip)).normalize();
		m.setColumn(2, v);

		/*
		cerr << m << endl;
		*/

		for ( size_t i = 0; i < n; ++i ) {
			Math::Vector3d v = m * Math::Vector3d(data[X][i], data[Y][i], data[Z][i]);
			data[X][i] = v[0];
			data[Y][i] = v[1];
			data[Z][i] = v[2];
		}
	}

	// -----------------------------------------------------------------------
	// Stage 6: Extract features
	// -----------------------------------------------------------------------
	Core::TimeSpan signalBegin = relativeTrigger + Core::TimeSpan(_config.signalBegin);
	Core::TimeSpan signalEnd   = relativeTrigger + Core::TimeSpan(_config.signalEnd);

	size_t iSignalBegin = size_t(samplingFrequency() * double(signalBegin));
	size_t iSignalEnd   = size_t(samplingFrequency() * double(signalEnd)) + 1;
	if ( iSignalEnd > n ) iSignalEnd = n;

	size_t intervalLength = size_t(_polarWindowLength * samplingFrequency()) + 1;
	size_t intervalOverlap = size_t(_polarWindowOverlap * samplingFrequency()) + 1;

	size_t intervalStart = iSignalBegin;

	// Dump data
	if ( _dumpData ) {
		ofstream ofs;
		string fn =
			"fx-" +
			_stream.lastRecord->networkCode() + "." +
			_stream.lastRecord->stationCode() + "." +
			_stream.lastRecord->locationCode() + "." +
			_stream.lastRecord->channelCode().substr(0, _stream.lastRecord->channelCode().size() - 1) +
			"-" + dataTimeWindow().startTime().toString("%Y%m%d%H%M%S%6f") +
			".plot";
		ofs.open(fn.c_str());

		for ( size_t i = iSignalBegin; i < iSignalEnd; ++i ) {
			for ( size_t c = 0; c < NCOMPS; ++c ) {
				if ( c ) ofs << "\t";
				ofs << data[c][i];
			}
			ofs << endl;
		}

		ofs.close();
	}

	FX::Result res;
	res.component = _usedComponent;
	res.record = _stream.lastRecord.get();

	_result = Core::None;

#if IDC_DEBUG
	cerr << "[FX " << _stream.lastRecord->streamID() << "]" << endl;

	size_t maxIntervalStart = 0;
	size_t maxIntervalEnd = 0;
#endif

	ofstream ofs;

	if ( _dumpData ) {
		string fn =
			"fx-" +
			_stream.lastRecord->networkCode() + "." +
			_stream.lastRecord->stationCode() + "." +
			_stream.lastRecord->locationCode() + "." +
			_stream.lastRecord->channelCode().substr(0, _stream.lastRecord->channelCode().size() - 1) +
			"-" + dataTimeWindow().startTime().toString("%Y%m%d%H%M%S%6f") + "-params"
			".plot";
		ofs.open(fn.c_str());
	}

	for ( ; intervalStart < iSignalEnd; intervalStart += intervalLength, intervalStart -= intervalOverlap ) {
		size_t intervalEnd = intervalStart + intervalLength;
		if ( intervalEnd > iSignalEnd )
			intervalEnd = iSignalEnd;

		double *signalData[3];
		for ( size_t c = 0; c < NCOMPS; ++c ) {
			signalData[c] = data[c] + intervalStart;
		}

		// Compute covariance matrix
		Math::Tensor2Sd tCov;
		size_t currentIntervalLength = intervalEnd - intervalStart;

		if ( currentIntervalLength < intervalLength ) {
			// Last interval which is less than the requested interval
			// length. Do not continue processing.
			break;
		}

		tCov._11 = cov(currentIntervalLength, signalData[X], signalData[X]);
		tCov._12 = cov(currentIntervalLength, signalData[X], signalData[Y]);
		tCov._13 = cov(currentIntervalLength, signalData[X], signalData[Z]);
		tCov._22 = cov(currentIntervalLength, signalData[Y], signalData[Y]);
		tCov._23 = cov(currentIntervalLength, signalData[Y], signalData[Z]);
		tCov._33 = cov(currentIntervalLength, signalData[Z], signalData[Z]);

#if IDC_DEBUG
		cerr << "  [Interval " << (intervalStart-iSignalBegin) << ":" << (intervalEnd-iSignalBegin) << "]" << endl;

		cerr << "    > Covariance" << endl;
		cerr << tCov << endl;
#endif

		/*
		0  0/0  X/X
		1  1/0  Y/X
		2  1/1  Y/Y
		3  2/0  Z/X
		4  2/1  Z/Y
		5  2/2  Z/Z
		*/

		Math::Spectral2Sd eigen;

		if ( !eigen.spect(tCov) ) continue;

		eigen.sort();

#if IDC_DEBUG
		cerr << "    > Eigenvectors" << endl;
		cerr << "    " << eigen.n1 << endl;
		cerr << "    " << eigen.n2 << endl;
		cerr << "    " << eigen.n3 << endl;
		cerr << "    > Eigenvalues" << endl;
		cerr << "    " << Vector3d(eigen.a1, eigen.a2, eigen.a3) << endl;
#endif

		double rectiLinearity = 2.0 * eigen.a1;
		if ( fabs(rectiLinearity) < 1E-20 ) continue;

		// eigenvalues: eigen.a1 > eigen.a2 > eigen.a3
		// rectilinearity, Pp, p. 1728, Jurkevics (1988) for surface waves
#if IDC_DEBUG
		rectiLinearity = 1.0 - ((eigen.a2 + eigen.a3) / rectiLinearity);
		cerr << "    > Rectilinearity, Jurkevics: " << rectiLinearity << endl;
#endif

		// rectilinearity of polarization, Pp, p. 1875 in Flinn (1965)
#if IDC_DEBUG
		double rectiLinearityF = 1.0 - eigen.a2 / eigen.a1;
		cerr << "    > RectiLinearity, Flinn: " << rectiLinearityF << endl;
#endif

		// strength of polarization, Ps, eq. 8 in Vidale (1986)
#if IDC_DEBUG
		double strength = 1.0 - ((eigen.a2 + eigen.a3) / eigen.a1);
		cerr << "    > Strength, Vidale: " << strength << endl;
#endif

		// planarity of polarization, Pp, eq. 9 in Vidale (1986)
#if IDC_DEBUG
		double planarityV = 1.0 - eigen.a3 / eigen.a2;
		cerr << "    > Planarity, Vidale: " << planarityV << endl;
#endif

		// planarity of polarization, Pp, p. 1728, Jurkevics (1988) for surface waves
#if IDC_DEBUG
		double planarityJ = 1.0 - 2.0 * eigen.a3 / (eigen.a1 + eigen.a2);
		cerr << "    > Planarity, Jurkevics: " << planarityJ << endl;
#endif

		// Take the eigenvector with the largest eigenvalue and project it onto
		// the X-Y plane. Then the backazimuth is computed with the dot-product
		// of the vector pointing towards north (0,1,0) and adding 180 degrees (Pi).
		// Adding Pi is actually the same as using the negative of the dot product.
		Math::Vector3d eigenAz = eigen.n1;

		double radOfIncidence = acos(abs(eigenAz.z));
#if IDC_DEBUG
		double angleOfIncidence = rad2deg(radOfIncidence);
		cerr << "    > AoI: " << angleOfIncidence << "°" << endl;
#endif

		double slowness = _polarAlpha * sin(radOfIncidence * 0.5);
		double sloUncertainty = 0.0;
		if ( _polarDk > 0.0 )
			sloUncertainty = 0.5 * _polarDk * _polarDk * (1.0 - rectiLinearity);
		if ( _polarDs > 0.0 )
			sloUncertainty += _polarDs * _polarDs;

		//double backAzimuth = rad2deg(atan2(eigenAz.x, eigenAz.y));
		double bazUncertainty = -1.0;
		double sign = eigenAz.z < 0 ? -1.0 : 1.0;
		double backAzimuth = rad2deg(atan2(sign * eigenAz.x, sign * eigenAz.y)) + 180.0;
		if ( backAzimuth > 360. )
			backAzimuth = backAzimuth - 360.0;

		/*
		if ( eigenAz.z > 0 )
			backAzimuth -= 180;
		if ( backAzimuth < 0 ) backAzimuth += 360;
		if ( backAzimuth >= 360 ) backAzimuth -= 360;
		*/

		if ( sloUncertainty > 0.0 ) {
			sloUncertainty = sqrt(sloUncertainty);
			bazUncertainty = sloUncertainty / (2.0 * slowness);
			if ( bazUncertainty < 1.0 )
				bazUncertainty = rad2deg(2.0 * asin(bazUncertainty));
			else
				bazUncertainty = 180.0;
			sloUncertainty = Math::Geo::deg2km(sloUncertainty);
		}

		slowness = Math::Geo::deg2km(slowness);

#if IDC_DEBUG
		cerr << "    > Baz: " << backAzimuth << "° +/- " << bazUncertainty << "°" << endl;
		cerr << "    > Slo: " << slowness << " s/deg +/- " << sloUncertainty << endl;
#endif

		if ( _dumpData )
			ofs << intervalStart << " " << backAzimuth << " " << slowness << " " << eigenAz.x << " " << eigenAz.y << " " << eigenAz.z << endl;

		if ( rectiLinearity > res.snr ) {
			res.snr = rectiLinearity;

			if ( !_result )
				_result = Result();

			_result->rectiLinearity = rectiLinearity;
			_result->backAzimuth = backAzimuth;
			_result->backAzimuthUncertainty = bazUncertainty;
			_result->slowness = slowness;
			_result->slownessUncertainty = sloUncertainty;

#if IDC_DEBUG
			maxIntervalStart = intervalStart;
			maxIntervalEnd = intervalEnd;
#endif
		}
	}

	if ( !_result ) {
		setStatus(Error, -100);
		return;
	}

#if IDC_DEBUG
	cerr << "Max interval: [" << (maxIntervalStart - iSignalBegin) << ":" << (maxIntervalEnd - iSignalBegin) << "]" << endl;
#endif

	setStatus(Finished, 100.);

	emit(res);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void IDCFeatureExtraction::finalizePick(DataModel::Pick *pick) const {
	if ( !_result ) {
		SEISCOMP_WARNING("No result set, pick cannot be finalized");
		return;
	}

	pick->setBackazimuth(DataModel::RealQuantity(_result->backAzimuth, _result->backAzimuthUncertainty));
	pick->setHorizontalSlowness(DataModel::RealQuantity(_result->slowness, _result->slownessUncertainty));

	DataModel::CommentPtr comment(new DataModel::Comment());
	comment->setId("IDC:rectilinearity");
	comment->setText(Core::toString(_result->rectiLinearity));

	pick->add(comment.get());
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
REGISTER_FXPROCESSOR(IDCFeatureExtraction, "IDC");
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
