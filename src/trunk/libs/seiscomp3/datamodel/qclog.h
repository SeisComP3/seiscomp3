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


#ifndef __SEISCOMP_DATAMODEL_QCLOG_H__
#define __SEISCOMP_DATAMODEL_QCLOG_H__


#include <seiscomp3/core/datetime.h>
#include <string>
#include <seiscomp3/datamodel/waveformstreamid.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(QCLog);

class QualityControl;


class SC_SYSTEM_CORE_API QCLogIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		QCLogIndex();
		QCLogIndex(Seiscomp::Core::Time start,
		           const WaveformStreamID& waveformID);

		//! Copy constructor
		QCLogIndex(const QCLogIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const QCLogIndex&) const;
		bool operator!=(const QCLogIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		Seiscomp::Core::Time start;
		WaveformStreamID waveformID;
};


class SC_SYSTEM_CORE_API QCLog : public PublicObject {
	DECLARE_SC_CLASS(QCLog);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		QCLog();

	public:
		//! Copy constructor
		QCLog(const QCLog& other);

		//! Constructor with publicID
		QCLog(const std::string& publicID);

		//! Destructor
		~QCLog();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static QCLog* Create();
		static QCLog* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static QCLog* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		QCLog& operator=(const QCLog& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const QCLog& other) const;
		bool operator!=(const QCLog& other) const;

		//! Wrapper that calls operator==
		bool equal(const QCLog& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setWaveformID(const WaveformStreamID& waveformID);
		WaveformStreamID& waveformID();
		const WaveformStreamID& waveformID() const;

		void setCreatorID(const std::string& creatorID);
		const std::string& creatorID() const;

		void setCreated(Seiscomp::Core::Time created);
		Seiscomp::Core::Time created() const;

		void setStart(Seiscomp::Core::Time start);
		Seiscomp::Core::Time start() const;

		void setEnd(Seiscomp::Core::Time end);
		Seiscomp::Core::Time end() const;

		void setMessage(const std::string& message);
		const std::string& message() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const QCLogIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const QCLog* lhs) const;

	
	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		QualityControl* qualityControl() const;

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
		// Index
		QCLogIndex _index;

		// Attributes
		std::string _creatorID;
		Seiscomp::Core::Time _created;
		Seiscomp::Core::Time _end;
		std::string _message;

	DECLARE_SC_CLASSFACTORY_FRIEND(QCLog);
};


}
}


#endif
