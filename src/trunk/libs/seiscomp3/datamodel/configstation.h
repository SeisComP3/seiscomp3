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


#ifndef __SEISCOMP_DATAMODEL_CONFIGSTATION_H__
#define __SEISCOMP_DATAMODEL_CONFIGSTATION_H__


#include <vector>
#include <string>
#include <seiscomp3/datamodel/setup.h>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ConfigStation);
DEFINE_SMARTPOINTER(Setup);

class ConfigModule;


class SC_SYSTEM_CORE_API ConfigStationIndex {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		ConfigStationIndex();
		ConfigStationIndex(const std::string& networkCode,
		                   const std::string& stationCode);

		//! Copy constructor
		ConfigStationIndex(const ConfigStationIndex&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		bool operator==(const ConfigStationIndex&) const;
		bool operator!=(const ConfigStationIndex&) const;


	// ------------------------------------------------------------------
	//  Attributes
	// ------------------------------------------------------------------
	public:
		std::string networkCode;
		std::string stationCode;
};


class SC_SYSTEM_CORE_API ConfigStation : public PublicObject {
	DECLARE_SC_CLASS(ConfigStation);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		ConfigStation();

	public:
		//! Copy constructor
		ConfigStation(const ConfigStation& other);

		//! Constructor with publicID
		ConfigStation(const std::string& publicID);

		//! Destructor
		~ConfigStation();
	

	// ------------------------------------------------------------------
	//  Creators
	// ------------------------------------------------------------------
	public:
		static ConfigStation* Create();
		static ConfigStation* Create(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Lookup
	// ------------------------------------------------------------------
	public:
		static ConfigStation* Find(const std::string& publicID);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		ConfigStation& operator=(const ConfigStation& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const ConfigStation& other) const;
		bool operator!=(const ConfigStation& other) const;

		//! Wrapper that calls operator==
		bool equal(const ConfigStation& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setNetworkCode(const std::string& networkCode);
		const std::string& networkCode() const;

		void setStationCode(const std::string& stationCode);
		const std::string& stationCode() const;

		void setEnabled(bool enabled);
		bool enabled() const;


	// ------------------------------------------------------------------
	//  Index management
	// ------------------------------------------------------------------
	public:
		//! Returns the object's index
		const ConfigStationIndex& index() const;

		//! Checks two objects for equality regarding their index
		bool equalIndex(const ConfigStation* lhs) const;

	
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
		bool add(Setup* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(Setup* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeSetup(size_t i);
		bool removeSetup(const SetupIndex& i);

		//! Retrieve the number of objects of a particular class
		size_t setupCount() const;

		//! Index access
		//! @return The object at index i
		Setup* setup(size_t i) const;
		Setup* setup(const SetupIndex& i) const;

		//! Find an object by its unique attribute(s)

		ConfigModule* configModule() const;

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
		ConfigStationIndex _index;

		// Attributes
		bool _enabled;

		// Aggregations
		std::vector<SetupPtr> _setups;

	DECLARE_SC_CLASSFACTORY_FRIEND(ConfigStation);
};


}
}


#endif
