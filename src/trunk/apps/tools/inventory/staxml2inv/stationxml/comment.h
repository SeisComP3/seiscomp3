/***************************************************************************
 *   Copyright (C) 2009 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_STATIONXML_COMMENT_H__
#define __SEISCOMP_STATIONXML_COMMENT_H__


#include <stationxml/metadata.h>
#include <vector>
#include <string>
#include <stationxml/date.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace StationXML {

DEFINE_SMARTPOINTER(Contact);



DEFINE_SMARTPOINTER(Comment);


/**
 * \brief Container for a comment or log entry.
 */
class Comment : public Core::BaseObject {
	DECLARE_CASTS(Comment);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		Comment();

		//! Copy constructor
		Comment(const Comment& other);

		//! Destructor
		~Comment();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Comment& operator=(const Comment& other);
		bool operator==(const Comment& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: Value
		void setValue(const std::string& value);
		const std::string& value() const;

		//! XML tag: Date
		void setDate(const OPT(DateTime)& date);
		DateTime date() const throw(Seiscomp::Core::ValueException);

		//! XML tag: id
		void setId(int id);
		int id() const;

	
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
		bool addAuthor(Contact* obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeAuthor(Contact* obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeAuthor(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t authorCount() const;

		//! Index access
		//! @return The object at index i
		Contact* author(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _value;
		OPT(DateTime) _date;
		int _id;

		// Aggregations
		std::vector<ContactPtr> _authors;
};


}
}


#endif
