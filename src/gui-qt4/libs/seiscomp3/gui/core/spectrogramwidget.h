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


#ifndef __SEISCOMP_GUI_CORE_SPECTROGRAMWIDGET_H__
#define __SEISCOMP_GUI_CORE_SPECTROGRAMWIDGET_H__


#include <QWidget>
#include <seiscomp3/gui/core/spectrogramrenderer.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API SpectrogramWidget : public QWidget {
	Q_OBJECT

	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		typedef IO::Spectralizer::Options SpectrumOptions;


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		SpectrogramWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		//! Sets the spectrogram options and calls reset().
		bool setSpectrumOptions(const SpectrumOptions &opts);

		const SpectrumOptions &spectrumOptions() const;

		//! Sets the scale of the raw stream data which is 1/gain
		void setScale(double scale);

		//! Sets the gradient range
		void setGradientRange(double lowerBound, double upperBound);

		double gradientLowerBound() const;
		double gradientUpperBound() const;

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

		const OPT(double) &frequencyLowerBound() const;
		const OPT(double) &frequencyUpperBound() const;

		void setNormalizeAmplitudes(bool f);
		bool normalizeAmplitudes() const;

		void setLogScale(bool f);
		bool logScale() const;

		//! Sets the transfer function for deconvolution
		void setTransferFunction(Math::Restitution::FFT::TransferFunction *tf);


	// ----------------------------------------------------------------------
	//  Public slots
	// ----------------------------------------------------------------------
	public slots:
		void updateSpectrogram();


	// ----------------------------------------------------------------------
	//  Protected Qt Interface
	// ----------------------------------------------------------------------
	protected:
		void resizeEvent(QResizeEvent *);
		void paintEvent(QPaintEvent *);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		SpectrogramRenderer _renderer;
};


}
}


#endif
