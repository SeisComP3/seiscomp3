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

#ifndef __SCARCHIVE_BSON_H__
#define __SCARCHIVE_BSON_H__

#include <seiscomp3/core/io.h>
#include <seiscomp3/core.h>


namespace Seiscomp {
namespace IO {


// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief An archive using BSON streams
 */
class SC_SYSTEM_CORE_API BSONArchive : public Seiscomp::Core::Archive {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		BSONArchive();

		//! Constructor with predefined buffer and mode
		BSONArchive(std::streambuf* buf, bool isReading = true,
			   int forceWriteVersion = -1);

		//! Destructor
		~BSONArchive();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		//! Opens an archive reading from a streambuf
		bool open(std::streambuf*);

		//! Implements derived virtual method
		virtual bool open(const char* filename);

		//! Creates an archive writing to a streambuf
		bool create(std::streambuf* buf, bool writeVersion = true);

		//! Implements derived virtual method
		virtual bool create(const char* filename, bool writeVersion = true);

		//! Implements derived virtual method
		virtual void close();

		/**
		 * Enables/Disables zip compression
		 * @param enable The state of this flag
		 */
		void setCompression(bool enable);

		/**
		 * Enables/Disables JSON format
		 * @param enable The state of this flag
		 */
		void setJSON(bool enable);


	// ----------------------------------------------------------------------
	//  Read methods
	// ----------------------------------------------------------------------
	public:
		//! Reads an integer
		virtual void read(int& value);
		//! Reads a float
		virtual void read(float& value);
		//! Reads a double
		virtual void read(double& value);

		virtual void read(char& value);

		//! Reads a vector of native types
		virtual void read(std::vector<char>& value);
		virtual void read(std::vector<int>& value);
		virtual void read(std::vector<float>& value);
		virtual void read(std::vector<double>& value);
		virtual void read(std::vector<std::string>& value);

		//! Reads a complex float
		virtual void read(std::complex<float>& value);
		//! Reads a complex double
		virtual void read(std::complex<double>& value);
		//! Reads a boolean
		virtual void read(bool& value);

		//! Reads a vector of complex doubles
		virtual void read(std::vector<std::complex<double> >& value);

		//! Reads a string
		virtual void read(std::string& value);

		//! Reads a time
		virtual void read(time_t& value);
		virtual void read(Seiscomp::Core::Time& value);


	// ----------------------------------------------------------------------
	//  Write methods
	// ----------------------------------------------------------------------
	public:
		//! Writes an integer
		virtual void write(int value);
		//! Writes a float
		virtual void write(float value);
		//! Writes a double
		virtual void write(double value);

		virtual void write(char& value);

		//! Writes a vector of native types
		virtual void write(std::vector<char>& value);
		virtual void write(std::vector<int>& value);
		virtual void write(std::vector<float>& value);
		virtual void write(std::vector<double>& value);
		virtual void write(std::vector<std::string>& value);

		//! Writes a complex float
		virtual void write(std::complex<float>& value);
		//! Writes a complex double
		virtual void write(std::complex<double>& value);
		//! Writes a boolean
		virtual void write(bool value);

		//! Writes a vector of complex doubles
		virtual void write(std::vector<std::complex<double> >& value);

		//! Writes a string
		virtual void write(std::string& value);

		//! Writes a time
		virtual void write(time_t value);
		virtual void write(Seiscomp::Core::Time& value);


	// ----------------------------------------------------------------------
	//  Protected interface
	// ----------------------------------------------------------------------
	protected:
		void setValidity(bool v);

		//! Implements derived virtual method
		bool locateObjectByName(const char* name, const char* targetClass, bool nullable);
		bool locateNextObjectByName(const char* name, const char* targetClass);

		//! Implements derived virtual method
		void readSequence();
		void writeSequence(int size);

		//! Implements derived virtual method
		std::string determineClassName();

		//! Implements derived virtual method
		virtual void setClassName(const char* className);

		//! Implements derived virtual method
		void serialize(RootType* object);

		//! Implements derived virtual method
		void serialize(SerializeDispatcher&);


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		bool open();
		bool create(bool writeVersion);

		template<typename T>
		void readVector(std::vector<T>& value);

		template<typename T>
		void readComplex(std::complex<T>& value);

		template<typename T>
		void writeVector(std::vector<T>& value);

		template<typename T>
		void writeComplex(std::complex<T>& value);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		DEFINE_SMARTPOINTER(BSONImpl);

		BSONImplPtr            _impl;

		std::string            _className;
		std::string            _attribName;
		int                    _siblingCount;
		bool                   _startSequence;
		bool                   _validObject;

		std::streambuf        *_buf;
		bool                   _deleteOnClose;

		bool                   _compression;
		bool                   _json;
		int                    _forceWriteVersion;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<


}
}

#endif
