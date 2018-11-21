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


#include <seiscomp3/gui/core/spectrogramrenderer.h>


namespace Seiscomp {
namespace Gui {
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SpectrogramRenderer::SpectrogramRenderer() {
	_tmin = _tmax = 0;
	_scale = 1.0;
	_ampMin = -15;
	_ampMax = -5;
	_imageFormat = QImage::Format_RGB32;

	Gradient gradient;
	gradient.setColorAt(0.0, QColor(255,   0, 255)); // pink
	gradient.setColorAt(0.2, QColor(  0,   0, 255)); // blue
	gradient.setColorAt(0.4, QColor(  0, 255, 255)); // cyan
	gradient.setColorAt(0.6, QColor(  0, 255,   0)); // green
	gradient.setColorAt(0.8, QColor(255, 255,   0)); // yellow
	gradient.setColorAt(1.0, QColor(255,   0,   0)); // red

	setGradient(gradient);

	_normalize = false;
	_logarithmic = false;
	_smoothTransform = true;
	_dirty = false;

	_renderedFmin = _renderedFmax = -1;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setGradient(const Gradient &gradient) {
	Gradient::const_iterator it;
	bool hasAlphaChannel = false;

	for ( it = gradient.begin(); it != gradient.end(); ++it ) {
		if ( it.value().first.alpha() < 255 ) {
			hasAlphaChannel = true;
			break;
		}
	}

	_imageFormat = hasAlphaChannel ? QImage::Format_ARGB32 : QImage::Format_RGB32;

	_gradient.generateFrom(gradient);
	_gradient.setRange(_ampMin, _ampMax);

	setDirty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SpectrogramRenderer::setOptions(const IO::Spectralizer::Options &opts) {
	if ( !_spectralizer )
		_spectralizer = new IO::Spectralizer;

	// Reset data
	reset();

	_options = opts;
	return _spectralizer->setOptions(_options);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setScale(double scale) {
	_scale = scale;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setGradientRange(double lowerBound, double upperBound) {
	_ampMin = lowerBound;
	_ampMax = upperBound;

	_gradient.setRange(lowerBound, upperBound);

	if ( !_spectralizer ) return;

	setDirty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::reset() {
	if ( !_spectralizer ) return;

	_spectra.clear();
	_images.clear();

	_spectralizer = new IO::Spectralizer;
	_spectralizer->setOptions(_options);

	_renderedFmin = _renderedFmax = -1;

	setDirty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SpectrogramRenderer::feed(const Record *rec) {
	if ( !_spectralizer ) return false;

	// Record clipped
	if ( _timeWindow.startTime().valid() && rec->endTime() <= _timeWindow.startTime() )
		return false;

	// Record clipped
	if ( _timeWindow.endTime().valid() && rec->startTime() >= _timeWindow.endTime() )
		return false;

	if ( _spectralizer->push(rec) ) {
		IO::SpectrumPtr spec;

		while ( (spec = _spectralizer->pop()) ) {
			if ( !spec->isValid() ) continue;

			// Deconvolution
			if ( _transferFunction ) {
				Seiscomp::ComplexDoubleArray *data = spec->data();
				double df = spec->maximumFrequency() / (data->size()-1);
				_transferFunction->deconvolve(data->size()-1, data->typedData()+1, df, df);
			}

			_spectra.push_back(spec);
			addSpectrum(spec.get());
			setDirty();
		}
	}

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SpectrogramRenderer::feedSequence(const RecordSequence *seq) {
	if ( seq == NULL ) return true;

	RecordSequence::const_iterator it;
	bool result = false;
	for ( it = seq->begin(); it != seq->end(); ++it )
		if ( feed(it->get()) ) result = true;

	setDirty();

	return result;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setRecords(const RecordSequence *seq) {
	reset();
	feedSequence(seq);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setAlignment(const Core::Time &align) {
	_alignment = align;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setTimeRange(double tmin, double tmax) {
	_tmin = tmin;
	_tmax = tmax;

	if ( _tmin > _tmax )
		std::swap(_tmin, _tmax);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setTimeWindow(const Core::TimeWindow& tw) {
	_timeWindow = tw;

	// Trim spectra
	if ( _timeWindow.startTime().valid() || _timeWindow.endTime().valid() ) {
		Spectra::iterator it;
		bool needUpdate = false;

		for ( it = _spectra.begin(); it != _spectra.end(); ) {
			IO::Spectrum *spec = it->get();
			if ( _timeWindow.startTime().valid() && spec->endTime() <= _timeWindow.startTime() ) {
				it = _spectra.erase(it);
				needUpdate = true;
			}
			else if ( _timeWindow.endTime().valid() && spec->startTime() >= _timeWindow.endTime() ) {
				it = _spectra.erase(it);
				needUpdate = true;
			}
			else
				++it;
		}

		if ( needUpdate ) setDirty();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setFrequencyRange(OPT(double) fmin, OPT(double) fmax) {
	_fmin = fmin;
	_fmax = fmax;

	if ( _normalize ) setDirty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setNormalizeAmplitudes(bool f) {
	if ( _normalize == f ) return;
	_normalize = f;

	if ( !_spectralizer ) return;

	setDirty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setLogScale(bool f) {
	if ( _logarithmic == f ) return;
	_logarithmic = f;

	if ( !_spectralizer ) return;

	setDirty();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setSmoothTransform(bool st) {
	_smoothTransform = st;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setTransferFunction(Math::Restitution::FFT::TransferFunction *tf) {
	_transferFunction = tf;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::setDirty() {
	_dirty = true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::renderSpectrogram() {
	Spectra::iterator it;

	_images.clear();

	for ( it = _spectra.begin(); it != _spectra.end(); ++it )
		addSpectrum(it->get());

	_dirty = false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::addSpectrum(IO::Spectrum *spec) {
	Seiscomp::ComplexDoubleArray *data = spec->data();

	if ( _images.empty() ) {
		SpecImage img;
		img.minimumFrequency = spec->minimumFrequency();
		img.maximumFrequency = spec->maximumFrequency();
		img.startTime = spec->center();
		img.dt = spec->dt();

		img.data = QImage(1, data->size(), _imageFormat);

		fillRow(img.data, data, spec->maximumFrequency(), 0);

		_images.append(img);
	}
	else {
		SpecImage &img = _images.back();
		Core::Time newTime = spec->center();

		// Do more checks on gaps and so on

		double dt = (double)img.dt;
		Core::Time currentEndTime = img.startTime + Core::TimeSpan(img.data.width()*dt);

		bool needNewImage = (fabs((double)(newTime - currentEndTime)) > dt*0.5)
		                 || (img.data.height() != data->size())
		                 || (img.minimumFrequency != spec->minimumFrequency())
		                 || (img.maximumFrequency != spec->maximumFrequency())
		                 || (img.dt != spec->dt());

		// Gap, overlap or different meta data -> start new image
		if ( needNewImage ) {
			SpecImage newImg;
			newImg.minimumFrequency = spec->minimumFrequency();
			newImg.maximumFrequency = spec->maximumFrequency();
			newImg.startTime = spec->center();
			newImg.dt = spec->dt();

			newImg.data = QImage(1, data->size(), _imageFormat);
			fillRow(newImg.data, data, spec->maximumFrequency(), 0);

			_images.append(newImg);
		}
		else {
			// Extent image by one column
			int col = img.data.width();
			img.data = img.data.copy(0,0,col+1,img.data.height());

			// Fill colors for column
			fillRow(img.data, data, spec->maximumFrequency(), col);
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::fillRow(QImage &img, Seiscomp::ComplexDoubleArray *spec,
                                  double maxFreq, int column) {
	QRgb *rgb = (QRgb*)img.bits();
	int ofs = img.width();
	int n = spec->size();

	// Goto nth column
	rgb += column;

	if ( _normalize ) {
		double amin = -1, amax = -1;
		double fmin = 0.0, fmax = maxFreq;
		if ( _fmin ) fmin = *_fmin;
		if ( _fmax ) fmax = *_fmax;

		// Compute min/max amplitude
		double f = maxFreq;
		double df = maxFreq / (spec->size()-1);

		for ( int i = n; i; --i, f -= df ) {
			if ( f < fmin || f > fmax ) continue;

			Seiscomp::ComplexDoubleArray::Type &v = (*spec)[i-1];

			double sr = v.real()*_scale;
			double si = v.imag()*_scale;
			double ps = sr*sr + si*si;
			if ( ps > 0 ) {
				if ( amin < 0 || amin > ps )
					amin = ps;
				if ( amax < 0 || amax < ps )
					amax = ps;
			}
		}

		double ascale = 1.0;

		if ( amin > 0 && amax > 0 ) {
			amin = log10(amin);
			amax = log10(amax);

			double arange = amax-amin;
			if ( arange > 0 )
				ascale = 1.0/arange;
		}
		else {
			amin = 0;
			ascale = 0;
		}

		if ( _logarithmic ) {
			double logTo = log10(n);
			double logFrom = 0;
			double logRange = logTo - logFrom;

			for ( int i = n; i; --i ) {
				double li = pow(10.0, (i-1)*logRange/(n-1) + logFrom) - 1;
				int i0 = (int)li;
				double t = li-i0;

				Seiscomp::ComplexDoubleArray::Type v;

				if ( t == 0 )
					v = (*spec)[i0];
				else
					v = (*spec)[i0]*(1-t)+(*spec)[i0+1]*t;

				double sr = v.real()*_scale;
				double si = v.imag()*_scale;
				double ps = sr*sr + si*si;
				double amp = ps > 0?log10(ps):_gradient.lowerBound();
				amp = (amp-amin)*ascale;

				*rgb = _gradient.valueAtNormalizedIndex(amp);
				rgb += ofs;
			}
		}
		else {
			for ( int i = n; i; --i ) {
				Seiscomp::ComplexDoubleArray::Type &v = (*spec)[i-1];

				double sr = v.real()*_scale;
				double si = v.imag()*_scale;
				double ps = sr*sr + si*si;
				double amp = ps > 0?log10(ps):_gradient.lowerBound();
				amp = (amp-amin)*ascale;

				*rgb = _gradient.valueAtNormalizedIndex(amp);
				rgb += ofs;
			}
		}
	}
	else {
		// Go from highest to lowest frequency
		if ( _logarithmic ) {
			double logTo = log10(n);
			double logFrom = 0;
			double logRange = logTo - logFrom;

			for ( int i = n; i; --i ) {
				double li = pow(10.0, (i-1)*logRange/(n-1) + logFrom) - 1;
				int i0 = (int)li;
				double t = li-i0;

				Seiscomp::ComplexDoubleArray::Type v;

				if ( t == 0 )
					v = (*spec)[i0];
				else
					v = (*spec)[i0]*(1-t)+(*spec)[i0+1]*t;

				double sr = v.real()*_scale;
				double si = v.imag()*_scale;
				double ps = sr*sr + si*si;
				double amp = ps > 0?log10(ps):_gradient.lowerBound();

				*rgb = _gradient.valueAt(amp);
				rgb += ofs;
			}
		}
		else {
			for ( int i = n; i; --i ) {
				Seiscomp::ComplexDoubleArray::Type &v = (*spec)[i-1];

				double sr = v.real()*_scale;
				double si = v.imag()*_scale;
				double ps = sr*sr + si*si;
				double amp = ps > 0?log10(ps):_gradient.lowerBound();

				*rgb = _gradient.valueAt(amp);
				rgb += ofs;
			}
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::render(QPainter &p, const QRect &rect,
                                 bool labelLeftAlign, bool renderLabels) {
	SpecImageList::iterator it;
	double fmin = -1;
	double fmax = -1;
	double frange;

	_renderedFmin = fmin;
	_renderedFmax = fmax;

	int w = rect.width();
	int h = rect.height();

	if ( (h <= 0) || (w <= 0) ) return;

	if ( _dirty ) renderSpectrogram();
	if ( _images.empty() ) return;

	if ( !_fmax ) {
		bool first = true;
		for ( it = _images.begin(); it != _images.end(); ++ it ) {
			if ( first ) {
				fmax = it->maximumFrequency;
				first = false;
			}
			else if ( it->maximumFrequency > fmax )
				fmax = it->maximumFrequency;
		}
	}
	else
		fmax = *_fmax;

	if ( !_fmin ) {
		bool first = true;
		for ( it = _images.begin(); it != _images.end(); ++ it ) {
			if ( first ) {
				fmin = it->minimumFrequency;
				first = false;
			}
			else if ( it->minimumFrequency < fmin )
				fmin = it->minimumFrequency;
		}
	}
	else
		fmin = *_fmin;

	if ( fmin > fmax ) std::swap(fmin, fmax);

	frange = fmax - fmin;

	_renderedFmin = fmin;
	_renderedFmax = fmax;

	Core::Time t0 = _alignment + Core::TimeSpan(_tmin);
	Core::Time t1 = _alignment + Core::TimeSpan(_tmax);
	double xlen = t1-t0;

	// Nothing to draw
	if ( xlen <= 0 ) return;

	double dx = 1.0/xlen;

	p.save();
	if ( _smoothTransform )
		p.setRenderHint(QPainter::SmoothPixmapTransform);

	// Draw images
	for ( it = _images.begin(); it != _images.end(); ++ it ) {
		SpecImage &img = *it;

		// Clip by min frequency
		if ( img.minimumFrequency >= fmax ) continue;

		// Clip by max frequency
		if ( img.maximumFrequency <= fmin ) continue;

		Core::Time startTime = img.startTime - Core::TimeSpan((double)img.dt*0.5);
		Core::Time endTime   = startTime + Core::TimeSpan((double)img.dt * img.data.width());

		// Clip by start time
		if ( startTime >= t1 ) continue;

		// Clip by end time
		if ( endTime <= t0 ) continue;

		double x0 = startTime - t0;
		double x1 = endTime - t0;

		double ifw = img.maximumFrequency-img.minimumFrequency;

		// Convert start and end time into widget coordinates
		int ix0 = (int)(x0*dx*w);
		int ix1 = (int)(x1*dx*w);

		// Source lines to plot
		int sy0 = 0;
		int sy1 = img.data.height();

		int ty0 = 0;
		int ty1 = h;

		if ( _logarithmic ) {
			if ( fmin < img.minimumFrequency )
				ty0 = (int)(log10(((img.minimumFrequency-fmin)/frange)*h)/log(h)*h);
			else
				sy0 = (int)(log10(((fmin-img.minimumFrequency)/ifw)*img.data.height())/log10(img.data.height())*img.data.height());

			if ( fmax > img.maximumFrequency )
				ty1 = (int)(log10((1.0-(fmax-img.maximumFrequency)/frange)*h)/log10(h)*h);
			else
				sy1 = (int)(log10((1.0-(img.maximumFrequency-fmax)/ifw)*img.data.height())/log10(img.data.height())*img.data.height());
		}
		else {
			if ( fmin < img.minimumFrequency )
				ty0 = (int)((img.minimumFrequency-fmin)/frange * h);
			else
				sy0 = (int)((fmin-img.minimumFrequency)/ifw * img.data.height());

			if ( fmax > img.maximumFrequency )
				ty1 = (int)((1.0-(fmax-img.maximumFrequency)/frange) * h);
			else
				sy1 = (int)((1.0-(img.maximumFrequency-fmax)/ifw) * img.data.height());
		}

		QRect sourceRect(0,img.data.height()-sy1,img.data.width(),sy1-sy0),
		      targetRect(rect.left()+ix0,rect.top()+h-ty1,ix1-ix0,ty1-ty0);

		p.drawImage(targetRect, img.data, sourceRect);
	}

	if ( renderLabels && (fmin >= 0) && (fmax >= 0) ) {
		QFont font = p.font();
		QFontInfo fi(font);
		int fontSize = std::min(h/2, fi.pixelSize());

		font.setPixelSize(fontSize);

		p.setFont(font);

		QString minFreq = QString("%1Hz").arg(fmin);
		QString maxFreq = QString("%1Hz").arg(fmax);

		QRect minR = p.fontMetrics().boundingRect(minFreq);
		QRect maxR = p.fontMetrics().boundingRect(maxFreq);

		minR.adjust(-2,-2,2,2);
		maxR.adjust(-2,-2,2,2);

		if ( labelLeftAlign )
			minR.moveBottomLeft(rect.bottomLeft()-QPoint(0,1));
		else
			minR.moveBottomRight(rect.bottomRight()-QPoint(0,1));

		p.drawRect(minR);
		p.drawText(minR, Qt::AlignCenter, minFreq);

		if ( labelLeftAlign )
			maxR.moveTopLeft(rect.topLeft());
		else
			maxR.moveTopRight(rect.topRight());

		p.drawRect(maxR);
		p.drawText(maxR, Qt::AlignCenter, maxFreq);
	}

	p.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SpectrogramRenderer::renderAxis(QPainter &p, const QRect &rect,
                                     bool leftAlign, int paddingOuter,
                                     int paddingInner) {
	if ( (_renderedFmin < 0) || (_renderedFmax < 0) ) return;

	int w = rect.width();
	int h = rect.height();

	if ( (h <= 0) || (w <= 0) ) return;

	QFont font = p.font();
	QFontInfo fi(font);
	int fontSize = std::min(h/2, fi.pixelSize());

	font.setPixelSize(fontSize);

	p.setFont(font);

	double frange = _renderedFmax - _renderedFmin;

	int fontHeight = p.fontMetrics().height();
	int tickLength = fontHeight/2+1;
	int tickSpacing = tickLength*3/2-tickLength;

	QString minFreq = QString("%1").arg(_renderedFmin);
	QString maxFreq = QString("%1").arg(_renderedFmax);

	QRect minR = p.fontMetrics().boundingRect(minFreq);
	QRect maxR = p.fontMetrics().boundingRect(maxFreq);

	QString axisLabel = "f [1/T] in Hz";
	QRect axisLabelR = p.fontMetrics().boundingRect(axisLabel);

	if ( axisLabelR.width() >= h-4 ) {
		axisLabel = "Hz";
		axisLabelR = p.fontMetrics().boundingRect(axisLabel);
	}

	minR.adjust(0,-1,0,1);
	maxR.adjust(0,-1,0,1);

	int wAxis = std::max(minR.width(), maxR.width()) + tickLength + tickSpacing + paddingInner;
	QRect axis(0,0,wAxis,h), area;

	if ( leftAlign ) {
		axis.moveTopLeft(rect.topLeft());
		axis.translate(axisLabelR.height()+2,0);
		area = axis.adjusted(-axisLabelR.height()-2,0,0,0);
	}
	else {
		axis.moveTopRight(rect.topRight());
		axis.translate(-axisLabelR.height()-2,0);
		area = axis.adjusted(0,0,axisLabelR.height()+2,0);
	}

	p.fillRect(area, p.brush());

	if ( leftAlign ) {
		axis.adjust(0,0,-paddingInner,0);
		p.drawLine(axis.topRight(), axis.bottomRight());
	}
	else {
		axis.adjust(paddingInner,0,0,0);
		p.drawLine(axis.topLeft(), axis.bottomLeft());
	}

	p.save();
	p.translate(area.right()-2, area.center().y()+axisLabelR.width()/2);
	p.rotate(-90);
	p.drawText(axisLabelR, Qt::AlignHCenter | Qt::AlignTop, axisLabel);
	p.restore();

	if ( leftAlign ) {
		p.drawText(axis.adjusted(0,0,-tickLength-tickSpacing,0), Qt::AlignRight | Qt::AlignTop, maxFreq);
		p.drawText(axis.adjusted(0,0,-tickLength-tickSpacing,0), Qt::AlignRight | Qt::AlignBottom, minFreq);
	}
	else {
		p.drawText(axis.adjusted(tickLength+tickSpacing,0,0,0), Qt::AlignLeft | Qt::AlignTop, maxFreq);
		p.drawText(axis.adjusted(tickLength+tickSpacing,0,0,0), Qt::AlignLeft | Qt::AlignBottom, minFreq);
	}

	// Reduce axis rect to free area
	QRect labels = axis.adjusted(0,maxR.height()-2,0,-minR.height()-2);
	if ( leftAlign )
		labels.adjust(0,0,-tickLength-tickSpacing,0);
	else
		labels.adjust(tickLength+tickSpacing,0,0,0);

	int steps = 10;
	double vSpacing;

	if ( _logarithmic ) {
		vSpacing = double(log10(axis.height())) / steps;
	}
	else {
		vSpacing = double(axis.height()) / steps;
		if ( vSpacing < 4 ) {
			steps = 5;
			vSpacing = double(axis.height()) / steps;
		}
	}

	double fSpacing = frange / steps;

	int xPos = leftAlign ? axis.right()-tickLength : axis.left();
	int sxPos = leftAlign ? axis.right()-tickLength/2 : axis.left();
	int subSteps = 10;
	while ( vSpacing*2 < fontHeight*subSteps )
		subSteps /= 2;

	p.save();
	p.setBrush(Qt::NoBrush);

	int alignmentFlags = (leftAlign ? Qt::AlignRight : Qt::AlignLeft) | Qt::AlignVCenter;

	double f = _renderedFmax;
	double yPos = 0;

	for ( int i = 0; i <= steps; ++i, yPos += vSpacing, f -= fSpacing ) {
		int y = axis.top() + (int)(_logarithmic ? pow(10.0, yPos) : yPos);
		if ( y > axis.bottom() ) y = axis.bottom();
		p.drawLine(xPos, y, xPos+tickLength, y);

		for ( int j = 1; j < subSteps; ++j ) {
			int sy = y + j*vSpacing/subSteps;
			p.drawLine(sxPos, sy, sxPos+tickLength/2, sy);
		}

		// Try to render label
		QString freq = QString("%1").arg(f);
		QRect freqR = p.fontMetrics().boundingRect(freq);
		freqR.adjust(0,1,0,1);

		if ( leftAlign )
			freqR.moveRight(labels.right());
		else
			freqR.moveLeft(labels.left());

		freqR.moveTop(y-freqR.height()/2);

		if ( labels.contains(freqR) ) {
			p.drawText(freqR, alignmentFlags, freq);
			// Shrink the labels area
			labels.setTop(freqR.bottom());
		}
	}

	p.restore();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
