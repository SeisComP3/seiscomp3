/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_COMMENT_H__
#define __SEISCOMP_FDSNXML_COMMENT_H__


#include <fdsnxml/metadata.h>
#include <vector>
#include <string>
#include <fdsnxml/date.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {

DEFINE_SMARTPOINTER(Person);



DEFINE_SMARTPOINTER(Comment);


/**
 * \brief Container for a comment or log entry. Corresponds to SEED
 * \brief blockettes 31, 51 and 59.
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
		Comment(const Comment &other);

		//! Destructor
		~Comment();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		Comment& operator=(const Comment &other);
		bool operator==(const Comment &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: Value
		void setValue(const std::string& value);
		const std::string& value() const;

		//! XML tag: BeginEffectiveTime
		void setBeginEffectiveTime(const OPT(DateTime)& beginEffectiveTime);
		DateTime beginEffectiveTime() const;

		//! XML tag: EndEffectiveTime
		void setEndEffectiveTime(const OPT(DateTime)& endEffectiveTime);
		DateTime endEffectiveTime() const;

		//! XML tag: id
		void setId(const OPT(int)& id);
		int id() const;

		//! A subject for this comment. Multiple comments with the same subject
		//! should be considered related.
		//! XML tag: subject
		void setSubject(const std::string& subject);
		const std::string& subject() const;

	
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
		bool addAuthor(Person *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeAuthor(Person *obj);

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
		Person* author(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _value;
		OPT(DateTime) _beginEffectiveTime;
		OPT(DateTime) _endEffectiveTime;
		OPT(int) _id;
		std::string _subject;

		// Aggregations
		std::vector<PersonPtr> _authors;
};


}
}


#endif
