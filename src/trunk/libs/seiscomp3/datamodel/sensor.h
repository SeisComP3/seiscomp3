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


#ifndef __SEISCOMP_DATAMODEL_SENSOR_H__
#define __SEISCOMP_DATAMODEL_SENSOR_H__


#include <seiscomp3/datamodel/blob.h>
#include <vector>
#include <string>
#include <seiscomp3/datamodel/sensorcalibration.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Sensor);
DEFINE_SMARTPOINTER(SensorCalibration);

class Inventory;


class SC_SYSTEM_CORE_API SensorIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		SensorIndex();
		SensorIndex(const std::string& name);

		//! Copy constructor
		SensorIndex(const SensorIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const SensorIndex&) const;
		bool operator!=(const SensorIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string name;
};


/**
 * \brief This type describes a sensor
 */
class SC_SYSTEM_CORE_API Sensor : public PublicObject {
	DECLARE_SC_CLASS(Sensor);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		Sensor();

	public:
		//! Copy constructor
		Sensor(const Sensor& other);

		//! Constructor with publicID
		Sensor(const std::string& publicID);

		//! Destructor
		~Sensor();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static Sensor* Create();
		static Sensor* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static Sensor* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Sensor& operator=(const Sensor& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Sensor& other) const;
		bool operator!=(const Sensor& other) const;

		//! Wrapper that calls operator==
		bool equal(const Sensor& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! Unique sensor name
		void setName(const std::string& name);
		const std::string& name() const;

		//! Sensor description
		void setDescription(const std::string& description);
		const std::string& description() const;

		//! Sensor model
		void setModel(const std::string& model);
		const std::string& model() const;

		//! Sensor manufacturer
		void setManufacturer(const std::string& manufacturer);
		const std::string& manufacturer() const;

		//! Sensor type (VBB, BB, SP, SM, OBS)
		void setType(const std::string& type);
		const std::string& type() const;

		//! Unit of measurement (M, M/S, M/S**2, RAD/S, V, A, PA, C)
		void setUnit(const std::string& unit);
		const std::string& unit() const;

		//! Lower corner frequency (optional)
		void setLowFrequency(const OPT(double)& lowFrequency);
		double lowFrequency() const;

		//! Higher corner frequency (optional)
		void setHighFrequency(const OPT(double)& highFrequency);
		double highFrequency() const;

		//! Reference to responsePAZ/@publicID or
		//! responsePolynomial/@publicID or responseFAP/@publicID
		void setResponse(const std::string& response);
		const std::string& response() const;

		void setRemark(const OPT(Blob)& remark);
		Blob& remark();
		const Blob& remark() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const SensorIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const Sensor* lhs) const;

	
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
		bool add(SensorCalibration* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(SensorCalibration* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeSensorCalibration(size_t i);
		bool removeSensorCalibration(const SensorCalibrationIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t sensorCalibrationCount() const;

		//! Index access
		//! @return The object at index i
		SensorCalibration* sensorCalibration(size_t i) const;
		SensorCalibration* sensorCalibration(const SensorCalibrationIndex& i) const;

		//! Find an object by its unique attribute(s)

		Inventory* inventory() const;

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
		SensorIndex _index;

		// Attributes
		std::string _description;
		std::string _model;
		std::string _manufacturer;
		std::string _type;
		std::string _unit;
		OPT(double) _lowFrequency;
		OPT(double) _highFrequency;
		std::string _response;
		OPT(Blob) _remark;

		// Aggregations
		std::vector<SensorCalibrationPtr> _sensorCalibrations;

	DECLARE_SC_CLASSFACTORY_FRIEND(Sensor);
};


}
}


#endif
