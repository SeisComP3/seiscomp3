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


#ifndef __SC_CORE_ENUMERATION_H__
#define __SC_CORE_ENUMERATION_H__

#include <seiscomp3/core/io.h>

namespace Seiscomp {
namespace Core {



class SC_SYSTEM_CORE_API Enumeration {
	public:
		virtual ~Enumeration() {}

		/**
		 * Converts an enumeration to its string representation
		 * @return The enumeration value string
		 */
		virtual const char* toString() const = 0;

		/**
		 * Converts a string to an enumeration value.
		 * @param str The name of the enumeration value. This name is
		 *            case sensitive.
		 * @return The result of the conversion
		 */
		virtual bool fromString(const std::string& str) = 0;

		/**
		 * Converts an enumeration value to an integer
		 * @return The integer value
		 */
		virtual int toInt() const = 0;

		/**
		 * Converts an integer to an enumeration value
		 * @param value The integer value to be converted
		 * @return The result of the conversion
		 */
		virtual bool fromInt(int value) = 0;
};

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief An enumeration class that supports string conversion and
 *  \brief serialization
 *
 *  Native enumerations are difficult to serialize in string based
 *  archives like XML. This class implements automatic string
 *  conversion based on a name table created while compile time.
 *  To create an enumeration the preprocessor macros MAKEENUM should
 *  be used.
 *  MAKEENUM(Name, ValueList, NameList)
 *  \param Name The name of the enumeration
 *  \param ValueList The list of values created with EVALUES
 *  \param NameList The list of names created with ENAMES
 *  \code
 *  MAKEENUM(Type,
 *    EVALUES(
 *      AUTOMATIC,
 *      MANUAL
 *    ),
 *    ENAMES(
 *      "automatic",
 *      "manual"
 *    )
 *  );
 *  \endcode
 *
 *  The above example expands to:
 *  \code
 *  enum EType { AUTOMATIC = 0x00, MANUAL, ETypeQuantity };
 *  class ETypeNames {
 *  	public:
 *  		static const char* name(int i) {
 *  			static const char* names[] = { "automatic", "manual" };
 *  			return names[i];
 *  		}
 *  };
 *  typedef Enum<EType, AUTOMATIC, ETypeQuantity, ETypeNames> Type;
 *  \endcode
 *
 *  The class Type can be used like native enumerations.
 *  \code
 *  Type value = AUTOMATIC;
 *  assert(value == AUTOMATIC);
 *
 *  printf("value = %s", value.toString());
 *  value.fromString("manual"); // enumeration names are case sensitive
 *  assert(value == MANUAL);
 *  \endcode
 *
 *  NOTE: Because SWIG does not support nested classes (version 1.3.27)
 *        the MAKENUM macro should no be used inside of class definitions.
 *        However, in C++ it can be placed nearly anywhere.
 */
template <typename ENUMTYPE, ENUMTYPE END, typename NAMES>
class Enum : public Enumeration {
	// ------------------------------------------------------------------
	//  Typetraits
	// ------------------------------------------------------------------
	public:
		typedef ENUMTYPE Type;
		typedef NAMES NameDispatcher;
		static const ENUMTYPE First = (ENUMTYPE)0;
		static const ENUMTYPE End = (ENUMTYPE)(END - 1);
		static const ENUMTYPE Quantity = (ENUMTYPE)(END - 0);


	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! C'tor
		Enum(ENUMTYPE value = (ENUMTYPE)0);


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		operator ENUMTYPE() const;

		bool operator==(ENUMTYPE value) const;
		bool operator!=(ENUMTYPE value) const;


	// ------------------------------------------------------------------
	//  Serialization
	// ------------------------------------------------------------------
	public:
		void serialize(Archive& ar);


	// ------------------------------------------------------------------
	//  Conversion
	// ------------------------------------------------------------------
	public:
		const char* toString() const;
		bool fromString(const std::string& str);

		int toInt() const;
		bool fromInt(int value);


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		ENUMTYPE _value;
};


#define ENUMNAME(Name, ...) \
	class E##Name##Names { \
		public: \
			E##Name##Names() {} \
			static const char* name(int i) { \
				static const char* names[] = { __VA_ARGS__ }; \
				return names[i]; \
			} \
	}

#define ENUMX(Name, ...) \
	enum E##Name { \
		__VA_ARGS__, \
		E##Name##Quantity \
	}; \
	class E##Name##Names; \
	typedef Seiscomp::Core::Enum<E##Name, E##Name##Quantity, E##Name##Names> Name; \

#define ENUMXNAMES(Name, ...) \
	ENUMNAME(Name, __VA_ARGS__)


#define EVALUES(...) __VA_ARGS__
#define ENAMES(...) __VA_ARGS__

#define MAKEENUM(Name, DEFS, NAMES) \
	ENUMX(Name, DEFS) \
	ENUMXNAMES(Name, NAMES)


#include <seiscomp3/core/enumeration.inl>

}
}


#endif

