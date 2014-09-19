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


#ifndef __SEISCOMP_GUI_CORE_TENSORRENDERER_H__
#define __SEISCOMP_GUI_CORE_TENSORRENDERER_H__


#include <QImage>
#include <QColor>

#include <seiscomp3/math/matrix3.h>
#include <seiscomp3/math/tensor.h>
#include <seiscomp3/gui/qt4.h>


namespace Seiscomp {
namespace Gui {


class SC_GUI_API TensorRenderer {
	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		TensorRenderer();


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		void setShadingEnabled(bool);
		bool isShadingEnabled() const { return _shading; }

		void setMaterial(float ambient, float diffuse);

		void setMargin(int);
		void setProjectMargin(int);

		void setBorderColor(QColor);
		void setTColor(QColor);
		void setPColor(QColor);

		QPoint project(Math::Vector3f &v) const;
		QPoint project(Math::Vector3d &v) const;
		QPoint project(double azimuth, double dist = 1.0) const;
		QPoint projectMargin(double azimuth, int margin, double dist = 1.0) const;

		// Unprojects a point in screen coordinates. Returns false if the
		// point is not on the sphere (x,y is then valid on the plane and
		// z is undefined), true otherwise
		bool unproject(Math::Vector3d &v, const QPointF &p) const;

		void renderBorder(QImage& img);

		// strike, dip and slip are expected to be in degrees
		void render(QImage& img, double strike, double dip, double slip);
		void render(QImage& img, const Math::Matrix3f &m);
		void render(QImage& img, const Math::Matrix3d &m);
		void render(QImage& img, const Math::Tensor2Sd &t, const Math::Matrix3f &m);
		void render(QImage& img, const Math::Tensor2Sd &t);


	private:
		QColor _colorT;
		QColor _colorP;
		QColor _border;

		bool   _shading;
		QPoint _center;
		int    _radius;
		int    _ballRadius;
		int    _projectRadius;
		int    _margin;
		int    _projectMargin;
		float  _materialAmbient;
		float  _materialDiffuse;
};


}
}

#endif
