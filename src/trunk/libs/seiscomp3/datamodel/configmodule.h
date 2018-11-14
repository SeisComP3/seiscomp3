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


#ifndef __SEISCOMP_DATAMODEL_CONFIGMODULE_H__
#define __SEISCOMP_DATAMODEL_CONFIGMODULE_H__


#include <vector>
#include <string>
#include <seiscomp3/datamodel/configstation.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ConfigModule);
DEFINE_SMARTPOINTER(ConfigStation);

class Config;


class SC_SYSTEM_CORE_API ConfigModule : public PublicObject {
	DECLARE_SC_CLASS(ConfigModule);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		ConfigModule();

	public:
		//! Copy constructor
		ConfigModule(const ConfigModule& other);

		//! Constructor with publicID
		ConfigModule(const std::string& publicID);

		//! Destructor
		~ConfigModule();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static ConfigModule* Create();
		static ConfigModule* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static ConfigModule* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		ConfigModule& operator=(const ConfigModule& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const ConfigModule& other) const;
		bool operator!=(const ConfigModule& other) const;

		//! Wrapper that calls operator==
		bool equal(const ConfigModule& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setName(const std::string& name);
		const std::string& name() const;

		void setParameterSetID(const std::string& parameterSetID);
		const std::string& parameterSetID() const;

		void setEnabled(bool enabled);
		bool enabled() const;

	
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
		bool add(ConfigStation* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(ConfigStation* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeConfigStation(size_t i);
		bool removeConfigStation(const ConfigStationIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t configStationCount() const;

		//! Index access
		//! @return The object at index i
		ConfigStation* configStation(size_t i) const;
		ConfigStation* configStation(const ConfigStationIndex& i) const;

		//! Find an object by its unique attribute(s)
		ConfigStation* findConfigStation(const std::string& publicID) const;

		Config* config() const;

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
		std::string _name;
		std::string _parameterSetID;
		bool _enabled;

		// Aggregations
		std::vector<ConfigStationPtr> _configStations;

	DECLARE_SC_CLASSFACTORY_FRIEND(ConfigModule);
};


}
}


#endif
