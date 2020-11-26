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


#ifndef __SEISCOMP_DATAMODEL_STRONGMOTION_LITERATURESOURCE_H__
#define __SEISCOMP_DATAMODEL_STRONGMOTION_LITERATURESOURCE_H__


#include <string>
#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/exceptions.h>
#include <seiscomp3/datamodel/strongmotion/api.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


DEFINE_SMARTPOINTER(LiteratureSource);


class SC_STRONGMOTION_API LiteratureSource : public Core::BaseObject {
	DECLARE_SC_CLASS(LiteratureSource);
	DECLARE_SERIALIZATION;
	DECLARE_METAOBJECT;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		//! Constructor
		LiteratureSource();

		//! Copy constructor
		LiteratureSource(const LiteratureSource& other);

		//! Custom constructor
		LiteratureSource(const std::string& title);
		LiteratureSource(const std::string& title,
		                 const std::string& firstAuthorName,
		                 const std::string& firstAuthorForename,
		                 const std::string& secondaryAuthors,
		                 const std::string& doi,
		                 const OPT(int)& year,
		                 const std::string& in_title,
		                 const std::string& editor,
		                 const std::string& place,
		                 const std::string& language,
		                 const OPT(int)& tome,
		                 const OPT(int)& page_from,
		                 const OPT(int)& page_to);

		//! Destructor
		~LiteratureSource();
	

	// ------------------------------------------------------------------
	//  Operators
	// ------------------------------------------------------------------
	public:
		operator std::string&();
		operator const std::string&() const;

		//! Copies the metadata of other to this
		LiteratureSource& operator=(const LiteratureSource& other);
		//! Checks for equality of two objects. Childs objects
		//! are not part of the check.
		bool operator==(const LiteratureSource& other) const;
		bool operator!=(const LiteratureSource& other) const;

		//! Wrapper that calls operator==
		bool equal(const LiteratureSource& other) const;


	// ------------------------------------------------------------------
	//  Setters/Getters
	// ------------------------------------------------------------------
	public:
		void setTitle(const std::string& title);
		const std::string& title() const;

		void setFirstAuthorName(const std::string& firstAuthorName);
		const std::string& firstAuthorName() const;

		void setFirstAuthorForename(const std::string& firstAuthorForename);
		const std::string& firstAuthorForename() const;

		void setSecondaryAuthors(const std::string& secondaryAuthors);
		const std::string& secondaryAuthors() const;

		void setDoi(const std::string& doi);
		const std::string& doi() const;

		void setYear(const OPT(int)& year);
		int year() const;

		void setInTitle(const std::string& inTitle);
		const std::string& inTitle() const;

		void setEditor(const std::string& editor);
		const std::string& editor() const;

		void setPlace(const std::string& place);
		const std::string& place() const;

		void setLanguage(const std::string& language);
		const std::string& language() const;

		void setTome(const OPT(int)& tome);
		int tome() const;

		void setPageFrom(const OPT(int)& pageFrom);
		int pageFrom() const;

		void setPageTo(const OPT(int)& pageTo);
		int pageTo() const;


	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		// Attributes
		std::string _title;
		std::string _firstAuthorName;
		std::string _firstAuthorForename;
		std::string _secondaryAuthors;
		std::string _doi;
		OPT(int) _year;
		std::string _inTitle;
		std::string _editor;
		std::string _place;
		std::string _language;
		OPT(int) _tome;
		OPT(int) _pageFrom;
		OPT(int) _pageTo;
};


}
}
}


#endif
