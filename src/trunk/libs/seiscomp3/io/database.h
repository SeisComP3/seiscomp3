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


#ifndef __SEISCOMP_IO_DATABASE_H__
#define __SEISCOMP_IO_DATABASE_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/interfacefactory.h>
#include <seiscomp3/core/datetime.h>
#include <seiscomp3/core.h>
#include <vector>
#include <string>
#include <stdint.h>


namespace Seiscomp {
namespace IO {


DEFINE_SMARTPOINTER(DatabaseInterface);

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief An abstract database interface and factory

	This class provides an interface to databases such as MySQL, Oracle
	and so on.
	Database interface classes will be registered in its constructor in
	the global interface pool. They can only be created through the
	interface factory.

	Example to request a database interface
	\code
	DatabaseInterfacePtr db = DatabaseInterface::Create("mysql");
	\endcode

	To implement new interface, just derive from DatabaseInterface
	and register the instance with REGISTER_DB_INTERFACE
	\code
	class MyDatabaseInterface : Seiscomp::IO::DatabaseInterface {
		DECLARE_SC_CLASS(MyDatabaseInterface)

		public:
			MyDatabaseInterface(const char* serviceName)
			 : DatabaseInterface(serviceName) {}

		// Implement all virtual methods
	};

	IMPLEMENT_SC_CLASS_DERIVED(MyDatabaseInterface,
	                           Seiscomp::IO::DatabaseInterface,
	                           "MyDatabaseInterface")
	REGISTER_DB_INTERFACE(MyDatabaseInterface, "MyServiceName");
	\endcode
 */
 class SC_SYSTEM_CORE_API DatabaseInterface : public Seiscomp::Core::BaseObject {
	DECLARE_SC_CLASS(DatabaseInterface);


	// ------------------------------------------------------------------
	//  Public types
	// ------------------------------------------------------------------
	public:
		typedef uint64_t OID;
		static const OID INVALID_OID;


	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		//! Protected constructor
		DatabaseInterface();

	public:
		//! Destructor
		virtual ~DatabaseInterface();


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		//! Returns a database driver for the given database service
		//! @return A pointer to the database driver
		//!         NOTE: The returned pointer has to be deleted by the
		//!               caller!
		static DatabaseInterface *Create(const char* service);

		//! Opens a database source.
		//! @param url A source URL of format [service://]user:pwd@host/database,
		//!            e.g. mysql://user:pwd@localhost/mydb.
		//! @return A pointer to the database interface.
		//!         NOTE: The returned pointer has to be deleted by the
		//!               caller!
		static DatabaseInterface *Open(const char* uri);

		/** Opens a connection to the database server
		    @param connection The string containing the connection
		                      data. Its format is
		                      username:password@host:port/database?column_prefix=[prefix]
		                      Default values:
		                        username = ""
		                        password = ""
		                        host = "localhost"
		                        port = "0" = undefined
		                        database = ""
		                        column_prefix = ""
			@return The result of the connection
		  */
		virtual bool connect(const char* connection);

		//! Closes the connection to the database
		virtual void disconnect() = 0;

		//! Returns the current connection state.
		virtual bool isConnected() const = 0;

		//! Starts a transaction
		virtual void start() = 0;

		//! Ends a transaction.
		//! Everthing between begin and end transaction will
		//! be comittet atomically
		virtual void commit() = 0;

		//! Aborts a transaction and does a rollback of
		//! statements between begin and rollback
		virtual void rollback() = 0;

		//! Executes a SQL command without a expecting a result
		virtual bool execute(const char* command) = 0;

		/** Starts a SQL query. If a query has not been closed
		    with endQuery it will be done in the first place.
		    @return False, if the query has not been executed because
		            of errors.
		            True, the query has been executed and results can
		            be fetched.
		  */
		virtual bool beginQuery(const char* query) = 0;

		//! Ends a query after its results are not needed anymore
		virtual void endQuery() = 0;

		/** Returns the default value name for the 'insert into' statement.
		    This is needed because sqlite3 does not support
		    \code
		    insert into MyTable values(default);
		    \endcode
		    if there is just one attribute in MyTable (e.g. an
		    autoincrement ID). sqlite3 supports only NULL as default
		    value string. The default implementation returns "default".
		    @return The default value name
		  */
		virtual const char *defaultValue() const;

		/** Returns the last inserted id when using auto increment id's
		    @param table The table name for which the last generated id
		                 will be returned. In MySQL last_insert_id() is
		                 independant of the table name. PostgreSQL
		                 does not support autoincrement attributes but
		                 uses named sequences. To retrieve the last
		                 generated ID one has to use something like
		                 \code
		                 select currval('MyTable_seq');
		                 \endcode
		                 assuming the sequence names are created as
		                 [tablename]_seq.
		    @return The last generated ID > 0 or 0 of there hasn't been
		            created an ID yet.
		  */
		virtual OID lastInsertId(const char* table) = 0;

		/** Returns the number of rows affected by a SQL statement.
		    This function should only be used after a UPDATE or DELETE
		    statement.
		    @return An integer greater than zero indicates the number
		            of rows affected or retrieved. Zero indicates that
		            no records were updated for an UPDATE statement,
		            no rows matched the WHERE clause in the query or
		            that no query has yet been executed. 0xFFFFFFFF (-1)
		            indicates an error.
		  */
		virtual uint64_t numberOfAffectedRows() = 0;

		/** Fetches a row from the results of a query.
		    @return True, a row has been fetched
		            False, there is no row left to fetch
		  */
		virtual bool fetchRow() = 0;

		/** Returns the index of the column with name <name>
		    This method is valid for the most recent query.
		    @param name The name of the column
		    @return The index of the column, -1 if no column
		            with that name exists
		  */
		virtual int findColumn(const char* name) = 0;

		/** Returns the number of columns returned by a query.
		    @return The column count. This number can be less or equal
		            to zero in case of an error.
		  */
		virtual int getRowFieldCount() const = 0;

		/** Returns the name of an indexed field of a
		    fetched row.
		    @param index The field index (column)
		    @return The name of the field.
		  */
		virtual const char *getRowFieldName(int index) = 0;

		/** Returns the content of an indexed field of a
		    fetched row.
		    @param index The field index (column)
		    @return The content of the field. If the field
		            is NULL, NULL will be returned.
		  */
		virtual const void *getRowField(int index) = 0;

		/**
		 * Convenience method to return the string of a certain column
		 * @param index The field index (column)
		 * @return The content of the field as string. If the field
		            is NULL, an empty string will be returned
		 */
		std::string getRowFieldString(int index);

		/** Returns the size of an indexed field of a
		    fetched row.
		    @param index The field index
		    @return The size of the field data.
		  */
		virtual size_t getRowFieldSize(int index) = 0;

		//! Converts a time to a string representation used by
		//! the database.
		virtual std::string timeToString(const Seiscomp::Core::Time&);

		//! Parses a string containing time information and returns
		//! the corresponding time object.
		//! The format of the string is database dependant.
		virtual Seiscomp::Core::Time stringToTime(const char*);

		/**
		 * @brief Escapes an input string to a database specific escaped
		 *        string to be used in an SQL statement.
		 *
		 * The default implementation does C escaping and duplicates a
		 * single quote.
		 *
		 * Note that the output string will have the correct size but the
		 * capacity (string::reserve, string::capacity) is most likely much
		 * larger to fit the maximum number of escaped characters. If the
		 * output string should be used permanently and not just as temporary
		 * value then call out.reserve(0) afterwards to allow it to shrink to
		 * its size.
		 *
		 * @param to The output string
		 * @param from The input string.
		 * @return Success flag
		 */
		virtual bool escape(std::string &out, const std::string &in);

		//! Returns the used column prefix
		const std::string &columnPrefix() const;

		//! Prepends the column prefix to the name and returns it
		std::string convertColumnName(const std::string& name) const;


	// ------------------------------------------------------------------
	//  Protected interface
	// ------------------------------------------------------------------
	protected:
		//! This method can be implemented in an interface to handle
		//! additional URI parameters:
		//! mysql://sysop:sysop@localhost/test?param1=value1&param2=value2
		//! This method would be called for param1 and param2.
		//! If this method is reimplemented the base implementation should
		//! be called otherwise column_prefix and timeout are not handled
		//! by default.
		virtual bool handleURIParameter(const std::string &name,
		                                const std::string &value);

		//! This method has to be implemented in derived classes to
		//! connect to the database using the members _user, _password,
		//! _host, _port and _database
		virtual bool open() = 0;


	// ------------------------------------------------------------------
	//  Protected members
	// ------------------------------------------------------------------
	protected:
		std::string  _user;
		std::string  _password;
		std::string  _host;
		int          _port;
		unsigned int _timeout;
		std::string  _database;
		std::string  _columnPrefix;
};


DEFINE_INTERFACE_FACTORY(DatabaseInterface);

#define REGISTER_DB_INTERFACE(Class, Service) \
Seiscomp::Core::Generic::InterfaceFactory<Seiscomp::IO::DatabaseInterface, Class> __##Class##InterfaceFactory__(Service)

}
}


#endif

