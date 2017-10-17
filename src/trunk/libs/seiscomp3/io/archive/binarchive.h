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


#ifndef __SCARCHIVE_BIN_H__
#define __SCARCHIVE_BIN_H__

#include <seiscomp3/core/io.h>
#include <seiscomp3/core.h>
#include <streambuf>

namespace Seiscomp {
namespace IO {



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
/** \brief An archive using binary streams
 */
class SC_SYSTEM_CORE_API BinaryArchive : public Seiscomp::Core::Archive {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		BinaryArchive();

		//! Constructor with predefined buffer and mode
		BinaryArchive(std::streambuf* buf, bool isReading = true);

		//! Destructor
		~BinaryArchive();


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		bool open(const char* file);
		bool open(std::streambuf*);

		bool create(const char* file);
		bool create(std::streambuf*);

		//! Implements derived virtual method
		virtual void close();


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

		virtual void read(std::vector<char>& value);
		virtual void read(std::vector<int>& value);
		virtual void read(std::vector<float>& value);
		virtual void read(std::vector<double>& value);
		virtual void read(std::vector<std::string>& value);
		virtual void read(std::vector<Core::Time>& value);

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
		
		virtual void write(std::vector<char>& value);
		virtual void write(std::vector<int>& value);
		virtual void write(std::vector<float>& value);
		virtual void write(std::vector<double>& value);
		virtual void write(std::vector<std::string>& value);
		virtual void write(std::vector<Core::Time>& value);

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
		//! Implements derived virtual method
		bool locateObjectByName(const char* name, const char* targetClass, bool nullable);
		bool locateNextObjectByName(const char* name, const char* targetClass);
		void locateNullObjectByName(const char* name, const char* targetClass, bool first);

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

		int writeBytes(const void*, int);


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		int classId(const std::string& classname);


	protected:
		std::streambuf* _buf;

	private:
		bool _deleteOnClose;
		bool _nullable;
		bool _usedObject;
		std::string _classname;

		int _sequenceSize;

		typedef std::vector<std::string> ClassList;
		ClassList _classes;
};
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<



// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class SC_SYSTEM_CORE_API VBinaryArchive : public BinaryArchive {
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		VBinaryArchive(int forceWriteVersion = -1);

		//! Constructor with predefined buffer and mode
		VBinaryArchive(std::streambuf* buf, bool isReading = true,
		               int forceWriteVersion = -1);


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		void setWriteVersion(int version);

		bool open(const char* file);
		bool open(std::streambuf*);

		bool create(const char* file);
		bool create(std::streambuf*);

		void close();

		const char *errorMsg() const;


	// ----------------------------------------------------------------------
	//  Implementation
	// ----------------------------------------------------------------------
	private:
		void writeHeader();
		bool readHeader();


	private:
		int         _forceWriteVersion;
		std::string _error;
};


}
}

#endif
