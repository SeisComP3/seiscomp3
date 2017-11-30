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



#ifndef __SEISCOMP_DATAMODEL_PUBLICOBJECT_H__
#define __SEISCOMP_DATAMODEL_PUBLICOBJECT_H__


#include <seiscomp3/datamodel/object.h>
#include <boost/thread/tss.hpp>
#include <string>
#include <map>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(PublicObject);

namespace _private { struct Resolver; }


class SC_SYSTEM_CORE_API PublicObject : public Object {
	DECLARE_SC_CLASS(PublicObject);
	DECLARE_SERIALIZATION;

	// ------------------------------------------------------------------
	//  Public types
	// ------------------------------------------------------------------
	public:
		typedef std::map<std::string, PublicObject*> PublicObjectMap;
		typedef PublicObjectMap::const_iterator Iterator;


	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Constructors
		PublicObject();
		PublicObject(const std::string& publicID);

	public:
		//! Destructor
		~PublicObject();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	protected:
		//! Assigns 'this' the publicID of other if and only if 'this'
		//! does not have a valid publicID.
		//! No registration will be done, so 'this' cannot be found
		//! in the global instance pool via Find(publicID).
		//! If 'this' has been registered already this method
		//! does nothing.
		PublicObject& operator=(const PublicObject& other);

	public:
		bool operator==(const PublicObject&) const;
		bool operator!=(const PublicObject&) const;


	// ------------------------------------------------------------------
	//  Getters
	// ------------------------------------------------------------------
	public:
		//! Returns the objects publicID
		const std::string& publicID() const;

		//! Sets the publicID for an PublicObject. Usually it
		//! is done automatically by an data reader or within
		//! PublicObject::Create. Anyway to support custom data
		//! readers it is necessary to enable setting the
		//! publicID from outside.
		//! This function returns true when the object is registered
		//! after changing the publicID and false otherwise.
		bool setPublicID(const std::string &);

		//! Returns whether the object is registered or not
		//! If it is not registered, it cannot be found when
		//! using Find(publicID).
		//! There are two reasons why an object is not registered:
		//!  1. Its publicID is invalid (empty)
		//!  2. Another object with the same publicID has been
		//!     registered already
		bool registered() const;

		/**
		 * @brief Registers this instances publicID and links it with
		 *        this instance that it can be found with Find(publicID).
		 * @return success flag
		 */
		bool registerMe();

		/**
		 * @brief Deregisters this instances publicID and unlinks it with
		 *        this instance that it cannot be found with Find(publicID).
		 * @return success flag
		 */
		bool deregisterMe();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		//! Returns whether the object has a valid publicID or not
		bool validId() const;

		/**
		 * Returns the object with the given 'publicID'.
		 * The returned object must not be deleted.
		 * If no object can be found with the given Id, NULL is
		 * returned.
		 */
		static PublicObject* Find(const std::string& publicID);

		/**
		 * Returns the size of the static PublicObject registration map
		 */
		static size_t ObjectCount();

		/**
		 * Returns an iterator to the first element of
		 * the static PublicObject registration map
		 */
		static Iterator Begin();

		/**
		 * Returns an iterator behind the last element of
		 * the static PublicObject registration map
		 */
		static Iterator End();

		/**
		 * Enables/disabled the automatic publicID generation during
		 * serialization.
		 * This feature is useful to generate publicID when no publicID
		 * is given in an older archive.
		 */
		static void SetIdGeneration(bool);

		/**
		 * Sets the pattern used to generate a publicID.
		 * There are several placeholder that can be used
		 * to create a hopefully unique Id. Each placeholder
		 * has to be enclosed with '@'.
		 * Placeholders:
		 *  classname  - The classname of the object
		 *  id  - The number of PublicObject instances created since
		 *        program start
		 *  globalid  - The number of existing Core::BaseObject instances
		 *  time  - The current GMT time. The used format can be appended
		 *          after a slash.
		 * Example:
		 *
		 *  smi://de.gfz-potsdam.sc/@classname@#@time/%Y%m%d%H%M%S.%f@.@id@
		 *
		 *  results in
		 *
		 *  smi://de.gfz-potsdam.sc/Pick#20061213121443.138624.1204
		 *
		 * @param pattern The used pattern
		 */
		static void SetIdPattern(const std::string& pattern);

		/**
		 * Generates a publicID for an object.
		 * @param object The object thats publicID is going to be
		 *               generated.
		 * @return The unchanged object pointer passed as parameter.
		 */
		static PublicObject* GenerateId(PublicObject* object);

		/**
		 * Generates a publicID for an object.
		 * @param object The object thats publicID is going to be
		 *               generated.
		 * @param pattern The generation pattern to be used.
		 * @return The unchanged object pointer passed as parameter.
		 */
		static PublicObject* GenerateId(PublicObject* object,
		                                const std::string &pattern);

		/**
		 * Enables/Disables the registration of PublicObjects with their
		 * publicID in a global registration map. This influences only
		 * objects created after this call.
		 * @param enable true or false
		 */
		static void SetRegistrationEnabled(bool enable);
		static bool IsRegistrationEnabled();

		//! Updates a child object
		//! a parent, the method returns false.
		virtual bool updateChild(Object*) = 0;

		//! Visitor interface
		virtual void accept(Visitor*) = 0;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		void generateId(const std::string &pattern);


	private:
		std::string _publicID;
		bool _registered;

		static PublicObjectMap _publicObjects;

		static bool _generateIds;
		static std::string _idPattern;
		static unsigned long _publicObjectId;

		//static bool _registerObjects;
		static boost::thread_specific_ptr<bool> _registerObjects;

	friend class Object;
	friend struct _private::Resolver;
};


}
}


#endif
