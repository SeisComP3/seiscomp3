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

// This file was created by a source code generator.
// Do not modify the contents. Change the definition and run the generator
// again!


#ifndef __SEISCOMP_DATAMODEL_STRONGMOTION_SURFACERUPTURE_H__
#define __SEISCOMP_DATAMODEL_STRONGMOTION_SURFACERUPTURE_H__


#include <string>
#include <seiscomp3/datamodel/strongmotion/literaturesource.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/strongmotion/api.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


DEFINE_SMARTPOINTER(SurfaceRupture);


class SC_STRONGMOTION_API SurfaceRupture : public Core::BaseObject {
	DECLARE_SC_CLASS(SurfaceRupture);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SurfaceRupture();

		//! Copy constructor
		SurfaceRupture(const SurfaceRupture& other);

		//! Destructor
		~SurfaceRupture();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		SurfaceRupture& operator=(const SurfaceRupture& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const SurfaceRupture& other) const;
		bool operator!=(const SurfaceRupture& other) const;

		//! Wrapper that calls operator==
		bool equal(const SurfaceRupture& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setObserved(bool observed);
		bool observed() const;

		void setEvidence(const std::string& evidence);
		const std::string& evidence() const;

		void setLiteratureSource(const OPT(LiteratureSource)& literatureSource);
		LiteratureSource& literatureSource();
		const LiteratureSource& literatureSource() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		bool _observed;
		std::string _evidence;
		OPT(LiteratureSource) _literatureSource;
};


}
}
}


#endif
