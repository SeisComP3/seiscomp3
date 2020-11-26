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


#ifndef __SEISCOMP_DATAMODEL_STRONGMOTION_RECORD_H__
#define __SEISCOMP_DATAMODEL_STRONGMOTION_RECORD_H__


#include <seiscomp3/datamodel/creationinfo.h>
#include <string>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/datamodel/strongmotion/fileresource.h>
#include <vector>
#include <seiscomp3/datamodel/timequantity.h>
#include <seiscomp3/datamodel/strongmotion/contact.h>
#include <seiscomp3/datamodel/strongmotion/simplefilterchainmember.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/strongmotion/api.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


DEFINE_SMARTPOINTER(Record);
DEFINE_SMARTPOINTER(SimpleFilterChainMember);
DEFINE_SMARTPOINTER(PeakMotion);

class StrongMotionParameters;


class SC_STRONGMOTION_API Record : public PublicObject {
	DECLARE_SC_CLASS(Record);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Record();

	public:
		//! Copy constructor
		Record(const Record& other);

		//! Constructor with publicID
		Record(const std::string& publicID);

		//! Destructor
		~Record();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Record* Create();
		static Record* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Record* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Record& operator=(const Record& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Record& other) const;
		bool operator!=(const Record& other) const;

		//! Wrapper that calls operator==
		bool equal(const Record& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setCreationInfo(const OPT(CreationInfo)& creationInfo);
		CreationInfo& creationInfo();
		const CreationInfo& creationInfo() const;

		void setGainUnit(const std::string& gainUnit);
		const std::string& gainUnit() const;

		void setDuration(const OPT(double)& duration);
		double duration() const;

		void setStartTime(const TimeQuantity& startTime);
		TimeQuantity& startTime();
		const TimeQuantity& startTime() const;

		void setOwner(const OPT(Contact)& owner);
		Contact& owner();
		const Contact& owner() const;

		void setResampleRateNumerator(const OPT(int)& resampleRateNumerator);
		int resampleRateNumerator() const;

		void setResampleRateDenominator(const OPT(int)& resampleRateDenominator);
		int resampleRateDenominator() const;

		void setWaveformID(const WaveformStreamID& waveformID);
		WaveformStreamID& waveformID();
		const WaveformStreamID& waveformID() const;

		void setWaveformFile(const OPT(FileResource)& waveformFile);
		FileResource& waveformFile();
		const FileResource& waveformFile() const;

	
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
		bool add(SimpleFilterChainMember* obj);
		bool add(PeakMotion* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(SimpleFilterChainMember* obj);
		bool remove(PeakMotion* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeSimpleFilterChainMember(size_t i);
		bool removeSimpleFilterChainMember(const SimpleFilterChainMemberIndex& i);
		bool removePeakMotion(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t simpleFilterChainMemberCount() const;
		size_t peakMotionCount() const;

		//! Index access
		//! @return The object at index i
		SimpleFilterChainMember* simpleFilterChainMember(size_t i) const;
		SimpleFilterChainMember* simpleFilterChainMember(const SimpleFilterChainMemberIndex& i) const;
		PeakMotion* peakMotion(size_t i) const;

		//! Find an object by its unique attribute(s)
		PeakMotion* findPeakMotion(PeakMotion* peakMotion) const;

		StrongMotionParameters* strongMotionParameters() const;

		//! Implement Object interface
		bool assign(Object* other);
		bool attachTo(PublicObject* parent);
		bool detachFrom(PublicObject* parent);
		bool detach();

		//! Creates a clone
		Object* clone() const;

		//! Implement PublicObject interface
		bool updateChild(Object* child);

		void accept(Visitor*);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		OPT(CreationInfo) _creationInfo;
		std::string _gainUnit;
		OPT(double) _duration;
		TimeQuantity _startTime;
		OPT(Contact) _owner;
		OPT(int) _resampleRateNumerator;
		OPT(int) _resampleRateDenominator;
		WaveformStreamID _waveformID;
		OPT(FileResource) _waveformFile;

		// Aggregations
		std::vector<SimpleFilterChainMemberPtr> _simpleFilterChainMembers;
		std::vector<PeakMotionPtr> _peakMotions;

	DECLARE_SC_CLASSFACTORY_FRIEND(Record);
};


}
}
}


#endif
