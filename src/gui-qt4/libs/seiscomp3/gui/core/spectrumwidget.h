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


#ifndef __SEISCOMP_GUI_CORE_SPECTRUMWIDGET_H__
#define __SEISCOMP_GUI_CORE_SPECTRUMWIDGET_H__


#include <QWidget>
#include <seiscomp3/gui/plot/datay.h>
#include <seiscomp3/gui/plot/graph.h>

#ifndef Q_MOC_RUN
#include <seiscomp3/math/fft.h>
#include <seiscomp3/processing/response.h>
#endif


namespace Seiscomp {
namespace Gui {


class SC_GUI_API SpectrumWidget : public QWidget {
	Q_OBJECT

	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		SpectrumWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		void setMargin(int m) { _margin = qMax(0, m); }

		/**
		 * @brief Sets the spectrum values and the Nyquist frequency as the
		 *        frequency of the last spectrum value.
		 * @param freqNyquist The last spectrum value is measured for that frequency
		 * @param spec The spectrum values from 0Hz to [fNyquist]Hz
		 */
		void setSpectrum(double freqNyquist, const Math::ComplexArray &spec,
		                 Processing::Response *resp = NULL,
		                 QString exportBasename = QString());

		//! Access the x-axis
		Axis &xAxis() { return _xAxis; }

		//! Access the y-axis
		Axis &yAxis() { return _yAxis; }


	// ----------------------------------------------------------------------
	//  Public slots
	// ----------------------------------------------------------------------
	public slots:
		void setAmplitudeSpectrum(bool amplitudeSpectrum);
		void setPhaseSpectrum(bool phaseSpectrum);
		void setLogScaleX(bool logScale);
		void setLogScaleY(bool logScale);

		void setShowSpectrum(bool show);
		void setShowCorrected(bool show);
		void setShowResponse(bool show);

		/**
		 * @brief Exports all visible spectra into a simple ASCII file
		 */
		void exportSpectra();


	// ----------------------------------------------------------------------
	//  QWidget interface
	// ----------------------------------------------------------------------
	protected:
		void resizeEvent(QResizeEvent *e);
		void paintEvent(QPaintEvent *e);


	// ----------------------------------------------------------------------
	//  Private methods
	// ----------------------------------------------------------------------
	private:
		void updateData();
		void draw(QPainter &, const Graph *);
		void updateRanges();


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		QString                 _exportBasename;
		double                  _freqNyquist;
		Math::ComplexArray      _spec;
		Processing::ResponsePtr _resp;
		bool                    _phaseSpectrum;
		Graph                   _graphPowerSpectrum;
		Graph                   _graphResponseCorrectedPowerSpectrum;
		Graph                   _graphResponsePowerSpectrum;
		DataY                   _powerSpectrum;
		DataY                   _responseCorrectedPowerSpectrum;
		DataY                   _responsePowerSpectrum;
		int                     _margin;
		Axis                    _xAxis;
		Axis                    _yAxis;
		Axis                    _yAxis2;
};


}
}


#endif
