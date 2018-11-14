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


#ifndef __SEISCOMP_DATAMODEL_QUALITYCONTROL_H__
#define __SEISCOMP_DATAMODEL_QUALITYCONTROL_H__


#include <vector>
#include <seiscomp3/datamodel/qclog.h>
#include <seiscomp3/datamodel/waveformquality.h>
#include <seiscomp3/datamodel/outage.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(QualityControl);
DEFINE_SMARTPOINTER(QCLog);
DEFINE_SMARTPOINTER(WaveformQuality);
DEFINE_SMARTPOINTER(Outage);


class SC_SYSTEM_CORE_API QualityControl : public PublicObject {
	DECLARE_SC_CLASS(QualityControl);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		QualityControl();

		//! Copy constructor
		QualityControl(const QualityControl& other);

		//! Destructor
		~QualityControl();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		QualityControl& operator=(const QualityControl& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const QualityControl& other) const;
		bool operator!=(const QualityControl& other) const;

		//! Wrapper that calls operator==
		bool equal(const QualityControl& other) const;

	
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
		bool add(QCLog* obj);
		bool add(WaveformQuality* obj);
		bool add(Outage* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(QCLog* obj);
		bool remove(WaveformQuality* obj);
		bool remove(Outage* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeQCLog(size_t i);
		bool removeQCLog(const QCLogIndex& i);
		bool removeWaveformQuality(size_t i);
		bool removeWaveformQuality(const WaveformQualityIndex& i);
		bool removeOutage(size_t i);
		bool removeOutage(const OutageIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t qCLogCount() const;
		size_t waveformQualityCount() const;
		size_t outageCount() const;

		//! Index access
		//! @return The object at index i
		QCLog* qCLog(size_t i) const;
		QCLog* qCLog(const QCLogIndex& i) const;

		WaveformQuality* waveformQuality(size_t i) const;
		WaveformQuality* waveformQuality(const WaveformQualityIndex& i) const;

		Outage* outage(size_t i) const;
		Outage* outage(const OutageIndex& i) const;

		//! Find an object by its unique attribute(s)
		QCLog* findQCLog(const std::string& publicID) const;

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
		// Aggregations
		std::vector<QCLogPtr> _qCLogs;
		std::vector<WaveformQualityPtr> _waveformQualitys;
		std::vector<OutagePtr> _outages;

	DECLARE_SC_CLASSFACTORY_FRIEND(QualityControl);
};


}
}


#endif
