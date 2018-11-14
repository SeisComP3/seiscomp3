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

#include <seiscomp3/config/symboltable.h>

#include <sstream>
#include <ctype.h>
#include <algorithm>
#include <iostream>


namespace Seiscomp {
namespace Config {


namespace {


std::string toupper(const std::string &s) {
	std::string tmp;
	std::string::const_iterator it;
	for ( it = s.begin(); it != s.end(); ++it )
		tmp += ::toupper(*it);
	return tmp;
}


}



Symbol::Symbol(const std::string& name, const std::string &ns,
               const std::vector<std::string>& values,
               const std::string& uri,
               const std::string& comment,
               int s)
: name(name)
, ns(ns)
, values(values)
, uri(uri)
, comment(comment)
, stage(s)
, line(-1) {}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Symbol::Symbol() : stage(-1), line(-1) {}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void Symbol::set(const std::string& name, const std::string &ns,
                 const std::vector<std::string>& values,
                 const std::string& uri,
                 const std::string& comment,
                 int stage) {
	this->name    = name;
	this->ns      = ns;
	this->values  = values;
	this->uri     = uri;
	this->comment = comment;
	this->stage   = stage;
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool Symbol::operator ==(const Symbol& symbol) const {
	if (name == symbol.name)
		return true;
	return false;
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string Symbol::toString() const {
	std::stringstream ss;
	if (!comment.empty())
		ss << comment;
	ss << name << " = ";
	Values::const_iterator valueIt = values.begin();
	for ( ; valueIt != values.end(); ++valueIt ) {
		if ( valueIt != values.begin() )
			ss << ", ";
		ss << *valueIt;
	}
	ss << " in " << uri;
	return ss.str();
}
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SymbolTable::SymbolTable() : _csCheck(false), _objectCount(0), _logger(NULL) {}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SymbolTable::setCaseSensitivityCheck(bool f) {
	_csCheck = f;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SymbolTable::setLogger(Logger *l) {
	_logger = l;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SymbolTable::add(const std::string& name,
                      const std::string& ns,
                      const std::string& content,
                      const std::vector<std::string>& values,
                      const std::string& uri,
                      const std::string& comment,
                      int stage, int line) {
	std::pair<Symbols::iterator, bool> itp;
	itp = _symbols.insert(Symbols::value_type(name, Symbol()));
	if ( itp.second ) {
		Symbol &newSymbol = itp.first->second;
		newSymbol = Symbol(name, ns, values, uri, comment, stage);
		newSymbol.content = content;
		_symbolOrder.push_back(&newSymbol);
	}
	else {
		itp.first->second.set(name, ns, values, uri, comment, stage);
		itp.first->second.content = content;
	}

	// Update the last line in the parsed content
	itp.first->second.line = line;

	// Register mapping to case-sensitive name
	_cisymbols[toupper(name)] = itp.first;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SymbolTable::add(const Symbol& symbol) {
	std::pair<Symbols::iterator, bool> itp;
	itp = _symbols.insert(Symbols::value_type(symbol.name, Symbol()));

	if ( itp.second ) {
		Symbol &newSymbol = itp.first->second;
		newSymbol = symbol;
		_symbolOrder.push_back(&newSymbol);
	}
	else
		itp.first->second = symbol;

	// Register mapping to case-sensitive name
	_cisymbols[toupper(symbol.name)] = itp.first;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SymbolTable::remove(const std::string& name) {
	CISymbols::iterator ci_it;
	ci_it = _cisymbols.find(toupper(name));
	if ( ci_it != _cisymbols.end() )
		_cisymbols.erase(ci_it);

	Symbols::iterator it;
	it = _symbols.find(name);
	if ( it != _symbols.end() ) {
		SymbolOrder::iterator ito = std::find(_symbolOrder.begin(), _symbolOrder.end(), &it->second);
		if ( ito != _symbolOrder.end() )
			_symbolOrder.erase(ito);
		_symbols.erase(it);
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
Symbol* SymbolTable::get(const std::string& name) {
	Symbols::iterator it = _symbols.find(name);
	if ( it != _symbols.end() ) {
		if ( _csCheck && checkCI(name, &it->second) )
			return NULL;

		return &it->second;
	}

	if ( _csCheck ) checkCI(name, NULL);

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const Symbol* SymbolTable::get(const std::string& name) const {
	Symbols::const_iterator it = _symbols.find(name);
	if ( it != _symbols.end() ) {
		if ( _csCheck && checkCI(name, &it->second) )
			return NULL;

		return &it->second;
	}

	if ( _csCheck ) checkCI(name, NULL);

	return NULL;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SymbolTable::checkCI(const std::string &name, const Symbol *s) const {
	CISymbols::const_iterator it = _cisymbols.find(toupper(name));
	// No upper case entry found
	if ( it == _cisymbols.end() ) return false;

	if ( s == NULL ) {
		int _line = it->second->second.line;
		const std::string &_fileName = it->second->second.uri;

		// Issue warning
		CONFIG_WARNING("%s should define %s which is not defined itself: names are case-sensitive!",
		               it->second->second.name.c_str(),
		               name.c_str());
		return true;
	}

	const Symbol *cisym = &it->second->second;
	// Defined at a later stage?
	if ( s->stage >= 0 && (cisym->stage > s->stage || cisym->line > s->line) ) {
		int _line = cisym->line;
		const std::string &_fileName = cisym->uri;

		// Issue warning
		CONFIG_WARNING("%s should override %s but does not: names are case-sensitive!",
		               cisym->name.c_str(), name.c_str());
		return true;
	}

	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SymbolTable::incrementObjectCount() {
	return ++_objectCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SymbolTable::decrementObjectCount() {
	return --_objectCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int SymbolTable::objectCount() const {
	return _objectCount;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
std::string SymbolTable::toString() const {
	std::stringstream ss;
	SymbolOrder::const_iterator symbolIt = _symbolOrder.begin();
	for ( ; symbolIt != _symbolOrder.end(); ++symbolIt)
		ss << (*symbolIt)->toString() << std::endl;

	return ss.str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool SymbolTable::hasFileBeenIncluded(const std::string& fileName) {
	IncludedFiles::iterator it = _includedFiles.find(fileName);
	if (it != _includedFiles.end())
		return true;
	return false;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void SymbolTable::addToIncludedFiles(const std::string& fileName) {
	_includedFiles.insert(fileName);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SymbolTable::file_iterator SymbolTable::includesBegin() {
	return _includedFiles.begin();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SymbolTable::file_iterator SymbolTable::includesEnd() {
	return _includedFiles.end();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SymbolTable::iterator SymbolTable::begin() {
	return _symbolOrder.begin();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
SymbolTable::iterator SymbolTable::end() {
	return _symbolOrder.end();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



} // namespace Config
} // namespace Seiscomp
