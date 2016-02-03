/***************************************************************************
 *   Copyright (C) by gempa GmbH                                           *
 *   Author: Jan Becker, gempa GmbH                                        *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#ifndef __SEISCOMP_GUI_CORE_SPECTROGRAMRENDERER_H__
#define __SEISCOMP_GUI_CORE_SPECTROGRAMRENDERER_H__


#ifndef Q_MOC_RUN
#include <seiscomp3/core/recordsequence.h>
#include <seiscomp3/math/restitution/transferfunction.h>
#include <seiscomp3/io/recordfilter/spectralizer.h>
#endif
#include <seiscomp3/gui/core/lut.h>

#include <QPainter>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API SpectrogramRenderer {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		SpectrogramRenderer();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		//! Sets the spectrogram options and calls reset().
		bool setOptions(const IO::Spectralizer::Options &opts);
		const IO::Spectralizer::Options &options() const { return _options; }

		//! Sets the scale of the raw stream data which is 1/gain to convert
		//! to sensor units
		void setScale(double scale);

		//! Sets the color gradient. Key range is normalized to [0,1].
		void setGradient(const Gradient &gradient);

		//! Sets the gradient range
		void setGradientRange(double lowerBound, double upperBound);

		double gradientLowerBound() const { return _gradient.lowerBound(); }
		double gradientUpperBound() const { return _gradient.upperBound(); }

		//! Resets the spectrogram and deletes all data
		void reset();

		//! Feeds a record for processing. Records must be timely ordered
		//! otherwise gaps are produced.
		bool feed(const Record *rec);
		bool feedSequence(const RecordSequence *seq);

		//! Resets the view and feeds the sequence
		void setRecords(const RecordSequence *seq);

		void setAlignment(const Core::Time &align);
		void setTimeRange(double tmin, double tmax);

		//! Sets the current time window of the data
		void setTimeWindow(const Core::TimeWindow &tw);

		//! Sets the frequency range to be displayed. A value lower or equal to
		//! zero refers to the global minimum or the global maximum
		//! respectively.
		void setFrequencyRange(OPT(double) fmin, OPT(double) fmax);

		const OPT(double) &frequencyLowerBound() const { return _fmin; }
		const OPT(double) &frequencyUpperBound() const { return _fmax; }

		void setNormalizeAmplitudes(bool f);
		bool normalizeAmplitudes() const { return _normalize; }

		void setLogScale(bool f);
		bool logScale() const { return _logarithmic; }

		//! Sets the transfer function for deconvolution
		void setTransferFunction(Math::Restitution::FFT::TransferFunction *tf);

		bool isDirty() const { return _dirty; }

		//! Creates the spectrogram. This is usually done in render if the
		//! spectrogram is dirty but can called from outside.
		void renderSpectrogram();

		//! Renders the spectrogram with the given painter into the given rect.
		void render(QPainter &p, const QRect &rect, bool labelLeftAlign = true,
		            bool renderLabels = false);

		//! Renders the y axis. This call must precede a call to render otherwise
		//! the frequency range can by out of sync.
		void renderAxis(QPainter &p, const QRect &rect, bool leftAlign = true);


	// ----------------------------------------------------------------------
	//  Private Interface
	// ----------------------------------------------------------------------
	private:
		void setDirty();
		void addSpectrum(IO::Spectrum *);
		void fillRow(QImage &img, Seiscomp::ComplexDoubleArray *spec,
		             double maxFreq, int column);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		typedef QList<IO::SpectrumPtr> Spectra;

		struct SpecImage {
			QImage         data;
			Core::Time     startTime;
			Core::TimeSpan dt;
			double         minimumFrequency;
			double         maximumFrequency;
		};

		typedef QList<SpecImage> SpecImageList;
		typedef StaticColorLUT<512> Gradient512;
		typedef Math::Restitution::FFT::TransferFunctionPtr TransferFunctionPtr;

		QImage::Format            _imageFormat;
		TransferFunctionPtr       _transferFunction;
		Core::TimeWindow          _timeWindow;
		Core::Time                _alignment;
		double                    _tmin, _tmax;
		double                    _scale;
		OPT(double)               _fmin, _fmax;
		double                    _ampMin, _ampMax;
		IO::Spectralizer::Options _options;
		IO::SpectralizerPtr       _spectralizer;
		Spectra                   _spectra;
		SpecImageList             _images;
		Gradient512               _gradient;
		bool                      _normalize;
		bool                      _logarithmic;
		bool                      _dirty;
		double                    _renderedFmin;
		double                    _renderedFmax;
};


}
}


#endif
