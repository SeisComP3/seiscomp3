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


#include <seiscomp3/gui/core/spectrumwidget.h>

#include <math.h>
#include <fstream>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>


using namespace std;


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SpectrumWidget::SpectrumWidget(QWidget *parent, Qt::WindowFlags f)
: QWidget(parent, f)
, _graphPowerSpectrum(&_xAxis, &_yAxis)
, _graphResponseCorrectedPowerSpectrum(&_xAxis, &_yAxis)
, _graphResponsePowerSpectrum(&_xAxis, &_yAxis2)
{
	_mode = Amplitude;
	_graphPowerSpectrum.setData(&_powerSpectrum);
	_graphResponseCorrectedPowerSpectrum.setData(&_responseCorrectedPowerSpectrum);
	_graphResponsePowerSpectrum.setData(&_responsePowerSpectrum);

	setBackgroundRole(QPalette::Base);
	setAutoFillBackground(true);

	setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));

	_xAxis.setLabel(tr("Frequency in Hz (1/T)"));
	updateAxisLabels();

	_xAxis.setPosition(Axis::Bottom);
	_yAxis.setPosition(Axis::Left);
	_yAxis2.setPosition(Axis::Right);

	_xAxis.setLogScale(true);
	_yAxis.setLogScale(true);
	_yAxis2.setLogScale(true);

	_margin = 9;
	_freqNyquist = -1;

	QColor lineColor(114, 159, 207);
	_graphPowerSpectrum.setColor(lineColor);
	_graphResponseCorrectedPowerSpectrum.setColor(QColor( 52*lineColor.red()/114, 101*lineColor.green()/159, 164*lineColor.blue()/207));
	_graphResponsePowerSpectrum.setPen(QPen(lineColor, 2, Qt::DashLine));
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::setSpectrum(double freqNyquist,
                                 const Math::ComplexArray &spec,
                                 Processing::Response *resp,
                                 QString exportBasename) {
	_exportBasename = exportBasename;
	_freqNyquist = freqNyquist;
	_spec = spec;
	_resp = resp;

	updateData();
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::updateData() {
	_powerSpectrum.clear();
	_responseCorrectedPowerSpectrum.clear();
	_responsePowerSpectrum.clear();

	if ( (_spec.size() < 2) || (_freqNyquist <= 0) ) {
		_xAxis.setRange(_powerSpectrum.x);
		return;
	}

	// Ignore the first value, this is the offset
	_powerSpectrum.y.resize(_spec.size()-1);

	switch ( _mode ) {
		case Amplitude:
		{
			double norm = 0.5 / _freqNyquist;
			for ( size_t i = 1; i < _spec.size(); ++i )
				_powerSpectrum.y[i-1] = abs(_spec[i]) * norm;
			break;
		}

		case Power:
		{
			double norm = 0.5 / _freqNyquist;
			for ( size_t i = 1; i < _spec.size(); ++i ) {
				double v = abs(_spec[i]) * norm;
				_powerSpectrum.y[i-1] = v*v;
			}
			break;
		}

		case Phase:
			for ( size_t i = 1; i < _spec.size(); ++i )
				_powerSpectrum.y[i-1] = rad2deg(arg(_spec[i]));
			break;

		default:
			for ( size_t i = 1; i < _spec.size(); ++i )
				_powerSpectrum.y[i-1] = 0;
			break;
	}

	_powerSpectrum.x.lower = _freqNyquist / _powerSpectrum.count();
	_powerSpectrum.x.upper = _freqNyquist;

	if ( (_powerSpectrum.count() > 1) && _resp ) {
		Math::Restitution::FFT::TransferFunctionPtr tf = _resp->getTransferFunction();
		if ( tf ) {
			_responseCorrectedPowerSpectrum.y.resize(_spec.size()-1);
			_responseCorrectedPowerSpectrum.x = _powerSpectrum.x;

			_responsePowerSpectrum.y.resize(_spec.size()-1);
			_responsePowerSpectrum.x = _powerSpectrum.x;

			double dx = _responseCorrectedPowerSpectrum.x.length() / (_spec.size()-1);
			double px = _responseCorrectedPowerSpectrum.x.lower;

			switch ( _mode ) {
				case Amplitude:
				{
					double norm = 0.5 / _freqNyquist;
					for ( size_t i = 1; i < _spec.size(); ++i, px += dx ) {
						Math::Complex r;
						tf->evaluate(&r, 1, &px);
						_responsePowerSpectrum.y[i-1] = abs(r);
						_responseCorrectedPowerSpectrum.y[i-1] = abs(_spec[i] / r) * norm;
					}
					break;
				}

				case Power:
				{
					double norm = 0.5 / _freqNyquist;
					for ( size_t i = 1; i < _spec.size(); ++i, px += dx ) {
						Math::Complex r;
						tf->evaluate(&r, 1, &px);

						double resp = abs(r);
						double cresp = abs(_spec[i] / r) * norm;
						_responsePowerSpectrum.y[i-1] = resp*resp;
						_responseCorrectedPowerSpectrum.y[i-1] = cresp*cresp;
					}
					break;
				}

				case Phase:
					for ( size_t i = 1; i < _spec.size(); ++i, px += dx ) {
						Math::Complex r;
						tf->evaluate(&r, 1, &px);
						_responsePowerSpectrum.y[i-1] = rad2deg(arg(r));
						_responseCorrectedPowerSpectrum.y[i-1] = rad2deg(arg(_spec[i] / r));
					}
					break;

				default:
					break;
			}
		}
	}

	_xAxis.setRange(_powerSpectrum.x);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::setAmplitudeSpectrum() {
	if ( _mode == Amplitude ) return;
	_mode = Amplitude;

	updateAxisLabels();

	if ( _freqNyquist > 0 )
		updateData();

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::setPhaseSpectrum() {
	if ( _mode == Phase ) return;
	_mode = Phase;

	updateAxisLabels();

	if ( _freqNyquist > 0 )
		updateData();

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::setPowerSpectrum() {
	if ( _mode == Power ) return;
	_mode = Power;

	updateAxisLabels();

	if ( _freqNyquist > 0 )
		updateData();

	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::updateAxisLabels() {
	switch ( _mode ) {
		case Amplitude:
			_yAxis.setLabel(tr("Amplitude"));
			_yAxis2.setLabel(tr("Normalized response amplitude"));
			break;

		case Power:
			_yAxis.setLabel(tr("Amplitude^2"));
			_yAxis2.setLabel(tr("Normalized response amplitude^2"));
			break;

		case Phase:
			_yAxis.setLabel(tr("Phase in degree"));
			_yAxis2.setLabel(tr("Response phase in degree"));
			break;

		default:
			_yAxis.setLabel(QString());
			_yAxis2.setLabel(QString());
			break;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::setLogScaleX(bool logScale) {
	if ( _xAxis.logScale() == logScale )
		return;

	_xAxis.setLogScale(logScale);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::setLogScaleY(bool logScale) {
	if ( _yAxis.logScale() == logScale )
		return;

	_yAxis.setLogScale(logScale);
	_yAxis2.setLogScale(logScale);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::setShowSpectrum(bool show) {
	if ( show == _graphPowerSpectrum.isVisible() ) return;
	_graphPowerSpectrum.setVisible(show);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::setShowCorrected(bool show) {
	if ( show == _graphResponseCorrectedPowerSpectrum.isVisible() ) return;
	_graphResponseCorrectedPowerSpectrum.setVisible(show);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::setShowResponse(bool show) {
	if ( show == _graphResponsePowerSpectrum.isVisible() ) return;
	_graphResponsePowerSpectrum.setVisible(show);
	_yAxis2.setVisible(show);
	update();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::exportSpectra() {
	if ( (_spec.size() < 1) || (_freqNyquist <= 0) ) {
		QMessageBox::critical(this, tr("Save spectra"), tr("No data"));
		return;
	}

	QString fn = QFileDialog::getSaveFileName(this, tr("Save spectra"), "spec-" + _exportBasename + ".txt");
	if ( fn.isEmpty() ) return;

	ofstream ofs;
	ofs.open(fn.toLatin1());
	if ( !ofs.is_open() ) {
		QMessageBox::critical(this, tr("Save spectra"), tr("Failed to open/create %1").arg(fn));
		return;
	}

	ofs << "# This is an ASCII representation of a spectrum. The first column is" << endl
	    << "# the frequency is Hz. The second and third column represent the" << endl
	    << "# complex value (real, imag) at that frequency of the raw spectrum." << endl
	    << "# The amplitude and phase can be found in column four and five." << endl;

	Math::Restitution::FFT::TransferFunctionPtr tf = _resp ? _resp->getTransferFunction() : NULL;
	if ( (_spec.size() > 1) && tf ) {
		ofs
		<< "# The sixth and seventh column represent the complex value of the response" << endl
		<< "# corrected spectrum. Column eight and nine the amplitude and phase" << endl
		<< "# respectively." << endl;
	}
	ofs << "#" << endl
	    << "# Note that phases are output in degrees!" << endl;

	double f = 0;
	double df = _freqNyquist / (_spec.size()-1);

	for ( size_t i = 0; i < _spec.size(); ++i, f += df ) {
		ofs << f << "\t" << _spec[i].real() << "\t" << _spec[i].imag()
		    << "\t" << abs(_spec[i]) << "\t" << deg2rad(arg(_spec[i]));
		if ( tf ) {
			ofs << "\t";
			if ( i ) {
				Math::Complex r;
				tf->evaluate(&r, 1, &f);
				r = _spec[i] / r;
				ofs << r.real() << "\t" << r.imag() << "\t" << abs(r) << "\t" << deg2rad(arg(r));
			}
			else
				ofs << _spec[i].real() << "\t" << _spec[i].imag() << "\t" << abs(_spec[i]) << "\t" << deg2rad(arg(_spec[i]));
		}
		ofs << endl;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::resizeEvent(QResizeEvent *e) {
	QWidget::resizeEvent(e);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::paintEvent(QPaintEvent *e) {
	QPainter p(this);

#if QT_VERSION >= 0x040300
	int axisHeight = _xAxis.sizeHint(p);

	updateRanges();

	QRect yAxisRect(_margin,_margin,0,height()-axisHeight-_margin*2);
	_yAxis.updateLayout(p, yAxisRect);

	QRect yAxis2Rect(width()-1-_margin,_margin,0,height()-axisHeight-_margin*2);
	if ( _yAxis2.isVisible() )
		_yAxis2.updateLayout(p, yAxis2Rect);

	QRect xAxisRect(yAxisRect.right(),height()-1-_margin,yAxis2Rect.left()-yAxisRect.right()+1,0);
	_xAxis.updateLayout(p, xAxisRect);

	QRect plotRect(xAxisRect.left(), yAxisRect.top(), xAxisRect.width(), yAxisRect.height());

	p.setPen(QColor(192,192,192));
	_xAxis.drawGrid(p, plotRect, true, false);
	_yAxis.drawGrid(p, plotRect, true, false);

	p.setPen(QColor(224,224,224));
	_xAxis.drawGrid(p, plotRect, false, true);
	_yAxis.drawGrid(p, plotRect, false, true);

	p.setPen(Qt::black);
	_xAxis.draw(p, xAxisRect);
	_yAxis.draw(p, yAxisRect);
	_yAxis2.draw(p, yAxis2Rect);

	p.setClipRect(plotRect.adjusted(0, -_margin, 0, 0));
	p.translate(xAxisRect.left(), yAxisRect.bottom());
	draw(p, &_graphPowerSpectrum);
	draw(p, &_graphResponseCorrectedPowerSpectrum);
	draw(p, &_graphResponsePowerSpectrum);
#else
	p.drawText(rect(), Qt::AlignCenter, tr("Minimum required Qt version is 4.3.0 to render the plot.\nInstalled version: %1").arg(QT_VERSION_STR));
#endif
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::draw(QPainter &p, const Graph *g) {
	if ( g->isEmpty() || !g->isVisible() ) return;

	QPolygonF poly;
	g->unproject(poly);

	p.setRenderHint(QPainter::Antialiasing, g->antiAliasing());

	if ( g->dropShadow() ) {
		p.translate(2,2);
		p.setPen(QPen(QColor(128,128,128,128), g->lineWidth()));
		p.drawPolyline(poly);
		p.translate(-2,-2);
	}

	p.setPen(g->pen());
	p.drawPolyline(poly);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrumWidget::updateRanges() {
	Range range;
	bool first = true;

	if ( _graphPowerSpectrum.isVisible() && !_graphPowerSpectrum.isEmpty() ) {
		if ( first ) {
			first = false;
			range = _graphPowerSpectrum.getYRange();
		}
		else
			range.extend(_graphPowerSpectrum.getYRange());
	}

	if ( _graphResponseCorrectedPowerSpectrum.isVisible() && !_graphResponseCorrectedPowerSpectrum.isEmpty() ) {
		if ( first ) {
			first = false;
			range = _graphResponseCorrectedPowerSpectrum.getYRange();
		}
		else
			range.extend(_graphResponseCorrectedPowerSpectrum.getYRange());
	}

	if ( _graphResponsePowerSpectrum.isVisible() && !_graphResponsePowerSpectrum.isEmpty() ) {
		/*
		if ( first ) {
			first = false;
			range = _graphResponsePowerSpectrum.getYRange();
		}
		else
			range.extend(_graphResponsePowerSpectrum.getYRange());
		*/
		_yAxis2.setRange(_graphResponsePowerSpectrum.getYRange());
	}

	_yAxis.setRange(range);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
