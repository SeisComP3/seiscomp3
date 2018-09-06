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


#ifndef __SEISCOMP_CONFIG_H__
#define __SEISCOMP_CONFIG_H__


#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <map>
#include <deque>

#include <seiscomp3/config/log.h>
#include <seiscomp3/config/exceptions.h>
#include <seiscomp3/config/symboltable.h>

namespace Seiscomp {
namespace Config {

/**
 * Mapping of configuration variable to type
 */
typedef std::map<std::string, std::string> Variables;


/**
 * This is a class for reading and writing configuration files. Currently the
 * following datatypes are supported: bool, int, double and std::string as well as
 * lists of the datatypes
 */
class SC_CONFIG_API Config {
	// ------------------------------------------------------------------------
	// X'struction
	// ------------------------------------------------------------------------
	public:
		Config();
		~Config();


	// ------------------------------------------------------------------------
	// Public interface
	// ------------------------------------------------------------------------
	public:
		/** When names are queried and this check is enabled, it will
		 * throw an exception if the same name is defined in a later stage
		 * with respect to case insensitive name comparison.
		 * This allows to check for parameter inconsistencies that are
		 * hard to track otherwise.
		 */
		void setCaseSensitivityCheck(bool);

		/** Reads the given configuration file.
		 * @param file name of the configuration files
		 * @param stage Optional stage value to be set to each read symbol
		 * @param raw Raw mode which does not resolv references like ${var}
		 * @return true on success
		 */
		bool readConfig(const std::string& file, int stage=-1, bool raw=false);

		/** Writes the configuration to the given configuration file.
		 * @param file name of the configuarion files
		 * @param localOnly write only value read from this file and
		 *                  new entries
		 * @return true on success
		 */
		bool writeConfig(const std::string& file, bool localOny = true,
		                 bool multilineLists = false);

		/** Writes the configuration to the file which was given to
		 * readConfing
		 * @return true on success
		 */
		bool writeConfig(bool localOnly = true);

		/** Sets the current logger. The ownership does not go to the config
		 * object. It is up to the caller to free resources.
		 * @param logger A logger implementation
		 */
		void setLogger(Logger *logger);

		/** Returns the symboltabel as string */
		std::string symbolsToString();

		/** Returns the names of parameters */
		std::vector<std::string> names() const;
		
		/** Returns the names of the visited files */
		std::string visitedFilesToString();

		//! Gets an integer from the configuration file
		//! @param name name of the element
		//! @return value
		int getInt(const std::string& name) const;
		int getInt(const std::string& name, bool* error) const;
		bool getInt(int& value, const std::string& name) const;

		bool setInt(const std::string& name, int value);

		/** Gets a double from the configuration file
		 * @param name name of the element
		 * @return double
		 */
		double getDouble(const std::string& name) const;
		double getDouble(const std::string& name, bool* error) const;
		bool getDouble(double& value, const std::string& name) const;

		bool setDouble(const std::string& name, double value);

		/** Gets an boolean from the configuration file
		 * @param name name of the element
		 * @return boolean
		 */
		bool getBool(const std::string& name) const;
		bool getBool(const std::string& name, bool* error) const;
		bool getBool(bool& value, const std::string& name) const;

		bool setBool(const std::string& name, bool value);

		/** Gets a string from the configuration file
		 * @param name name of the element
		 * @return string
		 */
		std::string getString(const std::string& name) const;
		std::string getString(const std::string& name, bool* error) const;
		bool getString(std::string& value, const std::string& name) const;

		bool setString(const std::string& name, const std::string& value);

		/** Removes the symbol with the given name from the symboltable.
		 * @param name Symbol to be removed
		 */
		bool remove(const std::string& name);

		std::vector<int> getInts(const std::string& name) const;

		std::vector<int> getInts(const std::string& name, bool* error) const;

		bool setInts(const std::string& name, const std::vector<int>& values);

		std::vector<double> getDoubles(const std::string& name) const;

		std::vector<double> getDoubles(const std::string& name, bool* error) const;

		bool setDoubles(const std::string& name, const std::vector<double>& values);

		std::vector<bool> getBools(const std::string& name) const;

		std::vector<bool> getBools(const std::string& name, bool* error) const;

		bool setBools(const std::string& name, const std::vector<bool>& values);

		std::vector<std::string> getStrings(const std::string& name) const;

		std::vector<std::string> getStrings(const std::string& name, bool* error) const;

		bool setStrings(const std::string& name, const std::vector<std::string>& values);

		SymbolTable *symbolTable() const;

		/** Evaluates a rvalue string and writes the output in result.
		 * The symbol table is taken from this instance.
		 * @param rvalue The value string to be parsed
		 * @param result The result string vector
		 * @param resolveReference Should references be resolved or not (eg
		 *                         environment variables).
		 * @return Success or error
		 */
		bool eval(const std::string &rvalue,
		          std::vector<std::string> &result,
		          bool resolveReferences = true,
		          std::string *errmsg = NULL);

		/** Evaluates a rvalue string and writes the output in result.
		 * The symbol table is taken from this instance.
		 * @param rvalue The value string to be parsed
		 * @param result The result string vector
		 * @param resolveReference Should references be resolved or not (eg
		 *                         environment variables).
		 * @param The symbol table to be used to resolve references if enabled.
		 * @return Success or error
		 */
		static bool Eval(const std::string &rvalue,
		                 std::vector<std::string> &result,
		                 bool resolveReferences = true,
		                 SymbolTable *symtab = NULL,
		                 std::string *errmsg = NULL);

		/** Writes the values of a symbol to an output stream. No new line
		 * is appended.
		 */
		static void writeValues(std::ostream &os, const Symbol *symbol,
		                        bool multilineLists = false);

		/** Writes the content of the symbol to an output stream. No new line
		 * is appended.
		 */
		static void writeContent(std::ostream &os, const Symbol *symbol,
		                         bool multilineLists = false);

		/** Writes a symbol to an output stream including the symbol
		 * name and a equal sign. A new line is appended.
		 */
		static void writeSymbol(std::ostream &os, const Symbol *symbol,
		                        bool multilineLists = false);

		/** Enables/disables tracking of configuration variables.
		 */
		void trackVariables(bool enabled);

		/** Returns all configuration variables read by an application mapped
		 * to a type
		 */
		const Variables& getVariables() const;

		/**
		 * @brief Escapes a string value that it can be stored in the
		 *        configuration file without further modifications.
		 * @return The escaped string inside double quotes if necessary
		 */
		std::string escape(const std::string &) const;


		// ----------------------------------------------------------------------
		// Protected interface
		// ----------------------------------------------------------------------
	protected:
		/** Parses the given file
		 * @return true on success false on failure
		 */
		bool parseFile(std::istream &is); // virtual candidate


		// ------------------------------------------------------------------------
		// Private interface
		// ------------------------------------------------------------------------
	private:
		void init();
		bool handleEntry(const std::string& entry, const std::string& comment);
		bool handleInclude(const std::string& fileName);
		void handleAssignment(const std::string& name, const std::string& content,
		                      std::vector<std::string>& values,
		                      const std::string& comment);
		std::vector<std::string> tokenize(const std::string& entry);
		static bool reference(const std::string &name,
		                      std::vector<std::string> &value,
		                      const SymbolTable *symtab);
		static bool parseRValue(const std::string& entry,
		                        std::vector<std::string>& parsedValues,
		                        const SymbolTable *symtab,
		                        bool resolveReferences,
		                        bool rawMode,
		                        std::string *errmsg);

		bool readInternalConfig(const std::string &file, SymbolTable *symbolTable,
		                        const std::string &namespacePrefix,
		                        int stage = -1, bool raw = false);

		template <typename T>
		T get(const std::string& name) const;

		template <typename T>
		T get(const std::string& name, bool* error) const;

		template <typename T>
		bool get(T& value, const std::string& name) const;

		template <typename T>
		std::vector<T> getVec(const std::string& name) const;

		template <typename T>
		std::vector<T> getVec(const std::string& name, bool* error) const;

		template <typename T>
		void add(const std::string& name, const T& value);

		template <typename T>
		void add(const std::string& name, const std::vector<T>& values);

		/** Sets an value in the configuration file
		 * @param element name of the element
		 * @param value value for the element */
		template <typename T>
		bool set(const std::string& name, const T& value);

		template <typename T>
		bool set(const std::string& name, const std::vector<T>& values);

		inline void addVariable(const std::string &name, const char *type) const;


		// ------------------------------------------------------------------------
		// Private data members
		// ------------------------------------------------------------------------
	private:
		typedef std::deque<std::string> Namespaces;
		int          _stage;
		int          _line;
		bool         _resolveReferences;
		std::string  _fileName;
		Namespaces   _namespaces;
		std::string  _namespacePrefix;
		std::string  _defaultNamespacePrefix;
		Logger      *_logger;

		SymbolTable *_symbolTable;
		bool         _trackVariables;
		Variables    _variables;
};


} // namespace Config
} // namespace Seiscomp

#endif
