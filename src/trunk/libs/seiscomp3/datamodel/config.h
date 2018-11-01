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


#ifndef __SEISCOMP_DATAMODEL_CONFIG_H__
#define __SEISCOMP_DATAMODEL_CONFIG_H__


#include <vector>
#include <seiscomp3/datamodel/notifier.h>
#include <seiscomp3/datamodel/publicobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(Config);
DEFINE_SMARTPOINTER(ParameterSet);
DEFINE_SMARTPOINTER(ConfigModule);


class SC_SYSTEM_CORE_API Config : public PublicObject {
	DECLARE_SC_CLASS(Config);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Config();

		//! Copy constructor
		Config(const Config& other);

		//! Destructor
		~Config();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		//! No changes regarding child objects are made
		Config& operator=(const Config& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const Config& other) const;
		bool operator!=(const Config& other) const;

		//! Wrapper that calls operator==
		bool equal(const Config& other) const;

	
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
		bool add(ParameterSet* obj);
		bool add(ConfigModule* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool remove(ParameterSet* obj);
		bool remove(ConfigModule* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeParameterSet(size_t i);
		bool removeConfigModule(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t parameterSetCount() const;
		size_t configModuleCount() const;

		//! Index access
		//! @return The object at index i
		ParameterSet* parameterSet(size_t i) const;
		ConfigModule* configModule(size_t i) const;

		//! Find an object by its unique attribute(s)
		ParameterSet* findParameterSet(const std::string& publicID) const;
		ConfigModule* findConfigModule(const std::string& publicID) const;

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
		std::vector<ParameterSetPtr> _parameterSets;
		std::vector<ConfigModulePtr> _configModules;

	DECLARE_SC_CLASSFACTORY_FRIEND(Config);
};


}
}


#endif
