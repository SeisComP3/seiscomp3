/***************************************************************************
 *   Copyright (C) 2013 by gempa GmbH
 *
 *   Author: Jan Becker
 *   Email: jabe@gempa.de $
 *
 ***************************************************************************/


#ifndef __SEISCOMP_FDSNXML_BASENODE_H__
#define __SEISCOMP_FDSNXML_BASENODE_H__


#include <fdsnxml/metadata.h>
#include <vector>
#include <fdsnxml/types.h>
#include <string>
#include <fdsnxml/date.h>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>


namespace Seiscomp {
namespace FDSNXML {

DEFINE_SMARTPOINTER(Comment);
DEFINE_SMARTPOINTER(DataAvailability);



DEFINE_SMARTPOINTER(BaseNode);


/**
 * \brief A base node type for derivation from: Network, Station and Channel
 * \brief types.
 */
class BaseNode : public Core::BaseObject {
	DECLARE_CASTS(BaseNode);
	DECLARE_RTTI;
	DECLARE_METAOBJECT_DERIVED;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		BaseNode();

		//! Copy constructor
		BaseNode(const BaseNode &other);

		//! Destructor
		~BaseNode();


	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		//! Copies the metadata of other to this
		BaseNode& operator=(const BaseNode &other);
		bool operator==(const BaseNode &other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		//! XML tag: Description
		void setDescription(const std::string& description);
		const std::string& description() const;

		//! XML tag: code
		void setCode(const std::string& code);
		const std::string& code() const;

		//! XML tag: startDate
		void setStartDate(const OPT(DateTime)& startDate);
		DateTime startDate() const;

		//! XML tag: endDate
		void setEndDate(const OPT(DateTime)& endDate);
		DateTime endDate() const;

		//! XML tag: restrictedStatus
		void setRestrictedStatus(const OPT(RestrictedStatusType)& restrictedStatus);
		RestrictedStatusType restrictedStatus() const;

		//! A code used for display or association, alternate to the
		//! SEED-compliant code.
		//! XML tag: alternateCode
		void setAlternateCode(const std::string& alternateCode);
		const std::string& alternateCode() const;

		//! A previously used code if different from the current code.
		//! XML tag: historicCode
		void setHistoricCode(const std::string& historicCode);
		const std::string& historicCode() const;

	
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
		bool addComment(Comment *obj);
		bool addDataAvailability(DataAvailability *obj);

		/**
		 * Removes an object.
		 * @param obj The object pointer
		 * @return true The object has been removed
		 * @return false The object has not been removed
		 *               because it does not exist in the list
		 */
		bool removeComment(Comment *obj);
		bool removeDataAvailability(DataAvailability *obj);

		/**
		 * Removes an object of a particular class.
		 * @param i The object index
		 * @return true The object has been removed
		 * @return false The index is out of bounds
		 */
		bool removeComment(size_t i);
		bool removeDataAvailability(size_t i);

		//! Retrieve the number of objects of a particular class
		size_t commentCount() const;
		size_t dataAvailabilityCount() const;

		//! Index access
		//! @return The object at index i
		Comment* comment(size_t i) const;
		DataAvailability* dataAvailability(size_t i) const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _description;
		std::string _code;
		OPT(DateTime) _startDate;
		OPT(DateTime) _endDate;
		OPT(RestrictedStatusType) _restrictedStatus;
		std::string _alternateCode;
		std::string _historicCode;

		// Aggregations
		std::vector<CommentPtr> _comments;
		std::vector<DataAvailabilityPtr> _dataAvailabilitys;
};


}
}


#endif
