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


#define SEISCOMP_COMPONENT DataModel
#include <seiscomp3/datamodel/strongmotion/literaturesource.h>
#include <seiscomp3/datamodel/metadata.h>
#include <seiscomp3/logging/log.h>


namespace Seiscomp {
namespace DataModel {
namespace StrongMotion {


IMPLEMENT_SC_CLASS(LiteratureSource, "LiteratureSource");


LiteratureSource::MetaObject::MetaObject(const Core::RTTI* rtti) : Seiscomp::Core::MetaObject(rtti) {
	addProperty(Core::simpleProperty("title", "string", false, false, false, false, false, false, NULL, &LiteratureSource::setTitle, &LiteratureSource::title));
	addProperty(Core::simpleProperty("firstAuthorName", "string", false, false, false, false, false, false, NULL, &LiteratureSource::setFirstAuthorName, &LiteratureSource::firstAuthorName));
	addProperty(Core::simpleProperty("firstAuthorForename", "string", false, false, false, false, false, false, NULL, &LiteratureSource::setFirstAuthorForename, &LiteratureSource::firstAuthorForename));
	addProperty(Core::simpleProperty("secondaryAuthors", "string", false, false, false, false, false, false, NULL, &LiteratureSource::setSecondaryAuthors, &LiteratureSource::secondaryAuthors));
	addProperty(Core::simpleProperty("doi", "string", false, false, false, false, false, false, NULL, &LiteratureSource::setDoi, &LiteratureSource::doi));
	addProperty(Core::simpleProperty("year", "int", false, false, false, false, true, false, NULL, &LiteratureSource::setYear, &LiteratureSource::year));
	addProperty(Core::simpleProperty("in_title", "string", false, false, false, false, false, false, NULL, &LiteratureSource::setInTitle, &LiteratureSource::inTitle));
	addProperty(Core::simpleProperty("editor", "string", false, false, false, false, false, false, NULL, &LiteratureSource::setEditor, &LiteratureSource::editor));
	addProperty(Core::simpleProperty("place", "string", false, false, false, false, false, false, NULL, &LiteratureSource::setPlace, &LiteratureSource::place));
	addProperty(Core::simpleProperty("language", "string", false, false, false, false, false, false, NULL, &LiteratureSource::setLanguage, &LiteratureSource::language));
	addProperty(Core::simpleProperty("tome", "int", false, false, false, false, true, false, NULL, &LiteratureSource::setTome, &LiteratureSource::tome));
	addProperty(Core::simpleProperty("page_from", "int", false, false, false, false, true, false, NULL, &LiteratureSource::setPageFrom, &LiteratureSource::pageFrom));
	addProperty(Core::simpleProperty("page_to", "int", false, false, false, false, true, false, NULL, &LiteratureSource::setPageTo, &LiteratureSource::pageTo));
}


IMPLEMENT_METAOBJECT(LiteratureSource)


LiteratureSource::LiteratureSource() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LiteratureSource::LiteratureSource(const LiteratureSource& other)
 : Core::BaseObject() {
	*this = other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LiteratureSource::LiteratureSource(const std::string& title)
 : _title(title) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LiteratureSource::LiteratureSource(const std::string& title,
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
                                   const OPT(int)& page_to)
 : _title(title),
   _firstAuthorName(firstAuthorName),
   _firstAuthorForename(firstAuthorForename),
   _secondaryAuthors(secondaryAuthors),
   _doi(doi),
   _year(year),
   _inTitle(in_title),
   _editor(editor),
   _place(place),
   _language(language),
   _tome(tome),
   _pageFrom(page_from),
   _pageTo(page_to) {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LiteratureSource::~LiteratureSource() {
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LiteratureSource::operator std::string&() {
	return _title;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LiteratureSource::operator const std::string&() const {
	return _title;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool LiteratureSource::operator==(const LiteratureSource& rhs) const {
	if ( !(_title == rhs._title) )
		return false;
	if ( !(_firstAuthorName == rhs._firstAuthorName) )
		return false;
	if ( !(_firstAuthorForename == rhs._firstAuthorForename) )
		return false;
	if ( !(_secondaryAuthors == rhs._secondaryAuthors) )
		return false;
	if ( !(_doi == rhs._doi) )
		return false;
	if ( !(_year == rhs._year) )
		return false;
	if ( !(_inTitle == rhs._inTitle) )
		return false;
	if ( !(_editor == rhs._editor) )
		return false;
	if ( !(_place == rhs._place) )
		return false;
	if ( !(_language == rhs._language) )
		return false;
	if ( !(_tome == rhs._tome) )
		return false;
	if ( !(_pageFrom == rhs._pageFrom) )
		return false;
	if ( !(_pageTo == rhs._pageTo) )
		return false;
	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool LiteratureSource::operator!=(const LiteratureSource& rhs) const {
	return !operator==(rhs);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool LiteratureSource::equal(const LiteratureSource& other) const {
	return *this == other;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LiteratureSource::setTitle(const std::string& title) {
	_title = title;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& LiteratureSource::title() const {
	return _title;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LiteratureSource::setFirstAuthorName(const std::string& firstAuthorName) {
	_firstAuthorName = firstAuthorName;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& LiteratureSource::firstAuthorName() const {
	return _firstAuthorName;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LiteratureSource::setFirstAuthorForename(const std::string& firstAuthorForename) {
	_firstAuthorForename = firstAuthorForename;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& LiteratureSource::firstAuthorForename() const {
	return _firstAuthorForename;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LiteratureSource::setSecondaryAuthors(const std::string& secondaryAuthors) {
	_secondaryAuthors = secondaryAuthors;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& LiteratureSource::secondaryAuthors() const {
	return _secondaryAuthors;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LiteratureSource::setDoi(const std::string& doi) {
	_doi = doi;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& LiteratureSource::doi() const {
	return _doi;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LiteratureSource::setYear(const OPT(int)& year) {
	_year = year;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int LiteratureSource::year() const throw(Seiscomp::Core::ValueException) {
	if ( _year )
		return *_year;
	throw Seiscomp::Core::ValueException("LiteratureSource.year is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LiteratureSource::setInTitle(const std::string& inTitle) {
	_inTitle = inTitle;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& LiteratureSource::inTitle() const {
	return _inTitle;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LiteratureSource::setEditor(const std::string& editor) {
	_editor = editor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& LiteratureSource::editor() const {
	return _editor;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LiteratureSource::setPlace(const std::string& place) {
	_place = place;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& LiteratureSource::place() const {
	return _place;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LiteratureSource::setLanguage(const std::string& language) {
	_language = language;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& LiteratureSource::language() const {
	return _language;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LiteratureSource::setTome(const OPT(int)& tome) {
	_tome = tome;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int LiteratureSource::tome() const throw(Seiscomp::Core::ValueException) {
	if ( _tome )
		return *_tome;
	throw Seiscomp::Core::ValueException("LiteratureSource.tome is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LiteratureSource::setPageFrom(const OPT(int)& pageFrom) {
	_pageFrom = pageFrom;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int LiteratureSource::pageFrom() const throw(Seiscomp::Core::ValueException) {
	if ( _pageFrom )
		return *_pageFrom;
	throw Seiscomp::Core::ValueException("LiteratureSource.pageFrom is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LiteratureSource::setPageTo(const OPT(int)& pageTo) {
	_pageTo = pageTo;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int LiteratureSource::pageTo() const throw(Seiscomp::Core::ValueException) {
	if ( _pageTo )
		return *_pageTo;
	throw Seiscomp::Core::ValueException("LiteratureSource.pageTo is not set");
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LiteratureSource& LiteratureSource::operator=(const LiteratureSource& other) {
	_title = other._title;
	_firstAuthorName = other._firstAuthorName;
	_firstAuthorForename = other._firstAuthorForename;
	_secondaryAuthors = other._secondaryAuthors;
	_doi = other._doi;
	_year = other._year;
	_inTitle = other._inTitle;
	_editor = other._editor;
	_place = other._place;
	_language = other._language;
	_tome = other._tome;
	_pageFrom = other._pageFrom;
	_pageTo = other._pageTo;
	return *this;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LiteratureSource::serialize(Archive& ar) {
	// Do not read/write if the archive's version is higher than
	// currently supported
	if ( ar.isHigherVersion<0,12>() ) {
		SEISCOMP_ERROR("Archive version %d.%d too high: LiteratureSource skipped",
		               ar.versionMajor(), ar.versionMinor());
		ar.setValidity(false);
		return;
	}

	ar & NAMED_OBJECT_HINT("title", _title, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("firstAuthorName", _firstAuthorName, Archive::XML_ELEMENT | Archive::XML_MANDATORY);
	ar & NAMED_OBJECT_HINT("firstAuthorForename", _firstAuthorForename, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("secondaryAuthors", _secondaryAuthors, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("doi", _doi, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("year", _year, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("in_title", _inTitle, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("editor", _editor, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("place", _place, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("language", _language, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("tome", _tome, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("page_from", _pageFrom, Archive::XML_ELEMENT);
	ar & NAMED_OBJECT_HINT("page_to", _pageTo, Archive::XML_ELEMENT);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
}
}
}
