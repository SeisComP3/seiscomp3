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


#ifndef __SCARCHIVE_H__
#define __SCARCHIVE_H__

#include <string>
#include <vector>
#include <list>
#include <complex>
#include <time.h>

#include <boost/type_traits.hpp>
#include <boost/mpl/if.hpp>
#include <seiscomp3/core.h>
#include <seiscomp3/core/defs.h>
#include <seiscomp3/core/optional.h>
#include <seiscomp3/core/factory.h>
#include <seiscomp3/core/serialization.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core/version.h>


#define DECLARE_ROOT_SERIALIZATION(RootClass) \
	public: \
		typedef Seiscomp::Core::Generic::Archive<RootClass> Archive; \
		virtual void serialize(Archive&) {}

#define DECLARE_SERIALIZATION \
	public: \
		virtual void serialize(Archive& ar)


namespace Seiscomp {
namespace Core {
namespace Generic {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief A template archive interface

    An archive offers an interface to read from and write to datasources. 
 */
template <typename ROOT_TYPE>
class Archive {
	// ------------------------------------------------------------------
	//  Traits
	// ------------------------------------------------------------------
	public:
		typedef ROOT_TYPE RootType;


	// ------------------------------------------------------------------
	//  Public Types
	// ------------------------------------------------------------------
	public:
		//! Serialization hints
		enum {
			NONE = 0,
			STATIC_TYPE     = 0x01,
			//! Objects should not serialize their child elements
			IGNORE_CHILDS   = 0x02,
			//! Objects are stored as XML nodes not as XML attributes
			XML_ELEMENT     = 0x04,
			//! Objects are stored as XML cdata not as XML attributes
			XML_CDATA       = 0x08,
			//! This attribute is mandatory even if empty
			XML_MANDATORY   = 0x10,
			//! Objects are stored in a seperate database table and
			//! not in columns of the parent object table
			DB_TABLE        = 0x20,
			//! The time is stored in two records: time and microseconds
			SPLIT_TIME      = 0x40,
			//! This is just an informational flag used for database
			//! access mostly. This flag is the only one that will
			//! be kept alive when serializing child objects.
			INDEX_ATTRIBUTE = 0x80
		};


	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Constructor
		Archive();


	public:
		//! Destructor
		virtual ~Archive() {}


	// ------------------------------------------------------------------
	//  Public Interface
	// ------------------------------------------------------------------
	public:
		static int PackVersion(int major, int minor) { return major << 16 | (minor & 0xFFFF); }

		/** Opens an archive.
		    Every archive class interpretes the dataSource parameter
		    in its own way. It can either point to a named dataSource (file)
		    or to a block of memory containing the actual archive data.
		 */
		virtual bool open(const char* dataSource);

		//! Creates a new archive
		virtual bool create(const char* dataSource);
		
		virtual void close() = 0;

		/**
		 * @brief Sets strict reading mode. In strict mode optional attributes
		 *        must be parsed correctly otherwise the archive is not valid.
		 *        If strict mode is disabled then invalid optional attributes
		 *        or objects are set to None or NULL.
		 * @param strict Enabled or disabled
		 */
		void setStrictMode(bool strict);
		bool isStrictMode() const;

		//! Queries whether the archive is in reading mode or not
		bool isReading() const;

		//! Returns whether the last operation was successfull or not
		bool success() const;

		//! Returns the serialization hints to propose a special
		//! behaviour to serializable objects.
		int hint() const;

		//! Sets the current serialization hint
		void setHint(int);

		//! Sets the validity during serialization if needed
		void setValidity(bool);

		void setVersion(Version v) { _version = v; }
		Version version() const { return _version; }

		int versionMajor() const { return _version.majorTag(); }
		int versionMinor() const { return _version.minorTag(); }

		template <int major, int minor>
		bool isLowerVersion() const {
			return _version.packed <VersionPacker<major,minor>::Value;
		}

		template <int major, int minor>
		bool isVersion() const {
			return _version.packed == VersionPacker<major,minor>::Value;
		}

		template <int major, int minor>
		bool isHigherVersion() const {
			return _version.packed > VersionPacker<major,minor>::Value;
		}

		template <int major, int minor>
		bool supportsVersion() const {
			return _version.packed >= VersionPacker<major,minor>::Value;
		}


	// ------------------------------------------------------------------
	//  Read methods
	// ------------------------------------------------------------------
	public:
		//! Reads an integer
		virtual void read(int& value) = 0;
		//! Reads a float
		virtual void read(float& value) = 0;
		//! Reads a double
		virtual void read(double& value) = 0;
		//! Reads a float complex
		virtual void read(std::complex<float>& value) = 0;
		//! Reads a double complex
		virtual void read(std::complex<double>& value) = 0;
		//! Reads a boolean
		virtual void read(bool& value) = 0;

		//! Reads a vector of chars
		virtual void read(std::vector<char>& value) = 0;

		//! Reads a vector of ints
		virtual void read(std::vector<int>& value) = 0;

		//! Reads a vector of floats
		virtual void read(std::vector<float>& value) = 0;

		//! Reads a vector of doubles
		virtual void read(std::vector<double>& value) = 0;

		//! Reads a vector of complex doubles
		virtual void read(std::vector<std::complex<double> >& value) = 0;

		//! Reads a vector of strings
		virtual void read(std::vector<std::string>& value) = 0;

		//! Reads a string
		virtual void read(std::string& value) = 0;

		//! Reads a time
		virtual void read(time_t& value) = 0;
		virtual void read(Seiscomp::Core::Time& value) = 0;

		template <typename T>
		void read(T& object);

		void read(ROOT_TYPE& object);

		//! Reads a generic pointer.
		//! When the referenced type is not registered, nothing will be
		//! done at all.
		//! Reading pointer to non class types (int, float, ...) implies
		//! compiler errors.
		template <typename T>
		void read(T*& object);

		template <typename T>
		void read(::boost::intrusive_ptr<T>& object);

		template <typename T>
		void read(::boost::optional<T>& object);


	// ------------------------------------------------------------------
	//  Write methods
	// ------------------------------------------------------------------
	public:
		//! Writes an integer
		virtual void write(int value) = 0;
		//! Writes a float
		virtual void write(float value) = 0;
		//! Writes a double
		virtual void write(double value) = 0;
		//! Writes a float complex
		virtual void write(std::complex<float>& value) = 0;
		//! Writes a double complex
		virtual void write(std::complex<double>& value) = 0;
		//! Writes a boolean
		virtual void write(bool value) = 0;

		//! Writes a vector of chars
		virtual void write(std::vector<char>& value) = 0;

		//! Writes a vector of ints
		virtual void write(std::vector<int>& value) = 0;

		//! Writes a vector of floats
		virtual void write(std::vector<float>& value) = 0;

		//! Writes a vector of doubles
		virtual void write(std::vector<double>& value) = 0;

		//! Writes a vector of complex doubles
		virtual void write(std::vector<std::complex<double> >& value) = 0;

		//! Writes a vector of strings
		virtual void write(std::vector<std::string>& value) = 0;

		//! Writes a string
		virtual void write(std::string& value) = 0;

		//! Writes a time
		virtual void write(time_t value) = 0;
		virtual void write(Seiscomp::Core::Time& value) = 0;

		//! Writes an object
		template <typename T>
		void write(T& object);

		void write(ROOT_TYPE& object);

		//! Entry method for writing object pointers
		template <typename T>
		void write(T*);

		template <typename T>
		void write(::boost::intrusive_ptr<T>&);

		template <typename T>
		void write(::boost::optional<T>&);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Writes a C pointer into the archive
		template <typename T>
		Archive& operator<<(T*&);

		//! Writes a smartpointer into the archive
		template <typename T>
		Archive& operator<<(::boost::intrusive_ptr<T>&);
		
		//! Writes a named object into the archive
		template <typename T>
		Archive& operator<<(const ObjectNamer<T>&);

		/**
		   \brief Why did you implement different versions for each generic
		   \brief container type instead of using a template template
		   \brief parameter?

		   To support container serialization, each container has to be
		   implemented in an own overloaded version, because Microsoft
		   Visual Studio lacks template type parameter deduction for this
		   kind of declaration:

		   template <template <typename> class CONTAINER, typename T>
		   Archive& operator<<(const ObjectNamer<CONTAINER<T> >&);

		   To support this compiler as well, different operators for each
		   supported container type have been implemented.
		 */

		//! Writes a named vector into the archive
		template <typename T>
		Archive& operator<<(const ObjectNamer<std::vector<T> >&);

		//! Writes a named list into the archive
		template <typename T>
		Archive& operator<<(const ObjectNamer<std::list<T> >&);

		//! Reads a C pointer from the archive
		template <typename T>
		Archive& operator>>(T*&);

		//! Reads a smartpointer pointer from the archive
		template <typename T>
		Archive& operator>>(::boost::intrusive_ptr<T>&);

		//! Reads an object sequence from the archive.
		template <typename T>
		Archive& operator>>(const ObjectIterator<T>&);

		//! Reads a named object from the archive.
		//! The object name will no be read but used for locating
		//! the data in the archive.
		template <typename T>
		Archive& operator>>(const ObjectNamer<T>&);

		//! Reads a vector from the archive
		template <typename T>
		Archive& operator>>(const ObjectNamer<std::vector<T> >&);

		//! Reads a list from the archive
		template <typename T>
		Archive& operator>>(const ObjectNamer<std::list<T> >&);
		
		//! Stream operator that decides by means of the _isReading flag
		//! whether a the object has to be written or to be read.
		template <typename T>
		Archive& operator&(ObjectNamer<T>);
		


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		struct SerializeDispatcher {
			virtual ~SerializeDispatcher() {}
			virtual void operator()(Archive&) = 0;
		};

		template <typename T>
		struct TypedSerializeDispatcher : SerializeDispatcher {
			TypedSerializeDispatcher(T* t = NULL) : target(t) {}
			
			TypedSerializeDispatcher& operator=(T* t) {
				target = t;
				return *this;
			}

			TypedSerializeDispatcher* operator->() { return this; }
		
			virtual void operator()(Archive<ROOT_TYPE>& ar) {
				target->serialize(ar);
			}

			const char* className() { return NULL; }

			T* target;
		};

		bool findObject(const char* name, const char* targetClass, bool nullable);
	
		//! Locates an object inside the archive. A derived class
		//! must provide its specific location code.
		virtual bool locateObjectByName(const char* name, const char* targetClass, bool nullable) = 0;
		virtual bool locateNextObjectByName(const char* name, const char* targetClass) = 0;

		//! Whenever a NULL object has to be serialized this method will be called.
		//! It has a default implementation (which does nothing) and does not need
		//! to be implemented by derived classes. But sometimes this information
		//! maybe quite useful. This method gets never called while in read mode.
		virtual void locateNullObjectByName(const char* name, const char* targetClass, bool first);

		//! When a sequence is to be read this methods gets called
		virtual void readSequence();

		//! Whenever a sequence is to be written this method gets called with the
		//! size of the sequence. The default implementation does nothing.
		virtual void writeSequence(int size);

		/** Whenever a polymorphic object has to be read, its classname
		    must be known to construct the object. A derived class
		    must implement this method to retrieve the current object
		    classname.
		 */
		virtual std::string determineClassName() = 0;

		//! Sets the classname to be written
		virtual void setClassName(const char*) = 0;

		/** Method to serialize an polymorphic object
		    The default implementation simply calls
		    \code
		    object->serialize(*this);
		    \endcode

		    Derived classes can override this method to wrap
		    the serialization process.
		    \code
		    Do_something_nifty_before_serialization();
		    Archive<ROOT_TYPE>::serialize(object);
		    Do_really_cool_things_after_serialization();
		    \endcode
		 */
		virtual void serialize(ROOT_TYPE* object);
		virtual void serialize(SerializeDispatcher&);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		template <typename T>
		void readPtr(ROOT_TYPE*, T*& object);

		template <typename T>
		void readPtr(void*, T*& object);
		
		//! Helper function to distinguish between pointer and non pointer
		//! types to avoid NULL pointer serialization.
		template <typename T>
		void read(const char* name, T& object, const char* targetClass);

		template <typename T>
		void read(const char* name, T*& object, const char* targetClass);

		template <typename T>
		void read(const char* name, ::boost::intrusive_ptr<T>& object, const char* targetClass);

		template <typename T>
		void read(const char* name, ::boost::optional<T>& object, const char* targetClass);
		
		template <typename T>
		void write(const char* name, T& object, const char* targetClass);

		//! Helper function to distinguish between C pointer and SmartPointers
		template <typename T>
		void write(const char* name, T* object, const char* targetClass);

		//! Helper function to distinguish between C pointer and SmartPointers
		template <typename T>
		void write(const char* name, ::boost::intrusive_ptr<T>& object, const char* targetClass);

		//! Helper function to distinguish between C pointer and Optionals
		template <typename T>
		void write(const char* name, ::boost::optional<T>& object, const char* targetClass);

		int setChildHint(int h);

	protected:
		int     _hint;
		bool    _isReading;
		bool    _validObject;
		bool    _first;
		bool    _found;
		bool    _strict;
		Version _version;


	template <typename ROOT, typename T, int CLASS_TYPE>
	friend struct VectorReader;

	template <typename ROOT, typename T, int CLASS_TYPE>
	friend struct VectorWriter;

	template <typename ROOT, typename T, int CLASS_TYPE>
	friend struct ListReader;

	template <typename ROOT, typename T, int CLASS_TYPE>
	friend struct ListWriter;

	template <typename ROOT, typename T, int CLASS_TYPE>
	friend struct ContainerReader;

	template <typename ROOT, typename T, int CLASS_TYPE>
	friend struct ContainerWriter;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#include "archive.inl"
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}
}

#endif
