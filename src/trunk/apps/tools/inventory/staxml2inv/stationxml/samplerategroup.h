/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_SAMPLERATEGROUP_H__
#define __SEISCOMP_STATIONXML_SAMPLERATEGROUP_H__


#include <stationxml/metadata.h>
#include <vector>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {

DEFINE_SMARTPOINTER(SampleRateType);
DEFINE_SMARTPOINTER(SampleRateRatioType);



DEFINE_SMARTPOINTER(SampleRateGroup);


/**
 * \brief This is a group of elements that represent sample rate. If this
 * \brief group is included, then SampleRate, which is the sample rate in
 * \brief samples per second, is required. SampleRateRatio, which is
 * \brief expressed as a ratio of number of samples in a number of seconds,
 * \brief is optional. If both are included, SampleRate should be considered
 * \brief more definitive.
 */
class SampleRateGroup : public Core::BaseObject {
	DECLARE_CASTS(SampleRateGroup);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SampleRateGroup();

		//! Copy constructor
		SampleRateGroup(const SampleRateGroup& other);

		//! Destructor
		~SampleRateGroup();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		SampleRateGroup& operator=(const SampleRateGroup& other);
		bool operator==(const SampleRateGroup& other) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		/**
		 * Add an object.
		 * @param obj The object pointer
		 * @return true The object has been added
		 * @return false The object has not been added
		 *               because it already exists in the list
		 *               or it already has another parent
		 */
		bool addSampleRate(SampleRateType* obj);
		bool addSampleRateRatio(SampleRateRatioType* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeSampleRate(SampleRateType* obj);
		bool removeSampleRateRatio(SampleRateRatioType* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeSampleRate(size_t i);
		bool removeSampleRateRatio(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t sampleRateCount() const;
		size_t sampleRateRatioCount() const;

		//! Index access
		//! @return The object at index i
		SampleRateType* sampleRate(size_t i) const;
		SampleRateRatioType* sampleRateRatio(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Aggregations
		std::vector<SampleRateTypePtr> _sampleRates;
		std::vector<SampleRateRatioTypePtr> _sampleRateRatios;
};


}
}


#endif
