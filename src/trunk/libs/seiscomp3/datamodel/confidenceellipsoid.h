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


#ifndef __SEISCOMP_DATAMODEL_CONFIDENCEELLIPSOID_H__
#define __SEISCOMP_DATAMODEL_CONFIDENCEELLIPSOID_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ConfidenceEllipsoid);


/**
 * \brief This class represents a description of the location
 * \brief uncertainty as a confidence
 * \brief ellipsoid with arbitrary orientation in space. The
 * \brief orientation of a rigid
 * \brief body in three-dimensional Euclidean space can be described
 * \brief by three
 * \brief parameters. We use the convention of Euler angles, which
 * \brief can be interpreted
 * \brief as a composition of three elemental rotations (i.e.,
 * \brief rotations around a single axis).
 * \brief In the special case of Euler angles we use here, the angles
 * \brief are referred
 * \brief to as Tait-Bryan (or Cardan) angles. These angles may be
 * \brief familiar to
 * \brief the reader from their application in flight dynamics, and
 * \brief are referred
 * \brief to as heading (yaw, psi), elevation (attitude, pitch, phi),
 * \brief and bank (roll, theta).
 * \brief For a definition of the angles, see Figure 4. Through the
 * \brief three elemental
 * \brief rotations, a Cartesian system (x, y, z) centered at the
 * \brief epicenter, with
 * \brief the South-North direction x, the West-East direction y, and
 * \brief the downward
 * \brief vertical direction z, is transferred into a different
 * \brief Cartesian system
 * \brief (X, Y , Z) centered on the confidence ellipsoid. Here, X
 * \brief denotes the direction
 * \brief of the major axis, and Y denotes the direction of the minor
 * \brief axis of the
 * \brief ellipsoid. Note that Figure 4 can be interpreted as a
 * \brief hypothetical view
 * \brief from the interior of the Earth to the inner face of a shell
 * \brief representing
 * \brief Earth's surface. The three Tait-Bryan rotations are
 * \brief performed as follows:
 * \brief (i) a rotation about the Z axis with angle psi (heading, or
 * \brief azimuth);
 * \brief (ii) a rotation about the Y axis with angle phi (elevation,
 * \brief or plunge);
 * \brief and (iii) a rotation about the X axis with angle theta
 * \brief (bank). Note that in
 * \brief the case of Tait-Bryan angles, the rotations are performed
 * \brief about the
 * \brief ellipsoid's axes, not about the axes of the fixed (x, y, z)
 * \brief Cartesian system.
 * \brief In the following list the correspondence of the attributes
 * \brief of class
 * \brief ConfidenceEllipsoid to the respective Tait-Bryan angles is
 * \brief listed:
 * \brief majorAxisPlunge: elevation (pitch, phi), majorAxisAzimuth:
 * \brief heading (yaw, psi), majorAxisRotation: bank (roll, theta)
 */
class SC_SYSTEM_CORE_API ConfidenceEllipsoid : public Core::BaseObject {
	DECLARE_SC_CLASS(ConfidenceEllipsoid);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ConfidenceEllipsoid();

		//! Copy constructor
		ConfidenceEllipsoid(const ConfidenceEllipsoid& other);

		//! Destructor
		~ConfidenceEllipsoid();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		ConfidenceEllipsoid& operator=(const ConfidenceEllipsoid& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const ConfidenceEllipsoid& other) const;
		bool operator!=(const ConfidenceEllipsoid& other) const;

		//! Wrapper that calls operator==
		bool equal(const ConfidenceEllipsoid& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Largest uncertainty, corresponding to the semi-major axis
		//! of the confidence ellipsoid in meter.
		void setSemiMajorAxisLength(double semiMajorAxisLength);
		double semiMajorAxisLength() const;

		//! Smallest uncertainty, corresponding to the semi-minor axis
		//! of the confidence ellipsoid in meter.
		void setSemiMinorAxisLength(double semiMinorAxisLength);
		double semiMinorAxisLength() const;

		//! Uncertainty in direction orthogonal to major and minor axes
		//! of the confidence ellipsoid in meter.
		void setSemiIntermediateAxisLength(double semiIntermediateAxisLength);
		double semiIntermediateAxisLength() const;

		//! Plunge angle of major axis of confidence ellipsoid.
		//! Corresponds to Tait-Bryan angle phi
		//! in degrees.
		void setMajorAxisPlunge(double majorAxisPlunge);
		double majorAxisPlunge() const;

		//! Azimuth angle of major axis of confidence ellipsoid.
		//! Corresponds to Tait-Bryan angle psi
		//! in degrees.
		void setMajorAxisAzimuth(double majorAxisAzimuth);
		double majorAxisAzimuth() const;

		//! This angle describes a rotation about the confidence
		//! ellipsoid's major axis which is required
		//! to define the direction of the ellipsoid's minor axis.
		//! Corresponds to Tait-Bryan angle theta
		//! in degrees.
		void setMajorAxisRotation(double majorAxisRotation);
		double majorAxisRotation() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		double _semiMajorAxisLength;
		double _semiMinorAxisLength;
		double _semiIntermediateAxisLength;
		double _majorAxisPlunge;
		double _majorAxisAzimuth;
		double _majorAxisRotation;
};


}
}


#endif
