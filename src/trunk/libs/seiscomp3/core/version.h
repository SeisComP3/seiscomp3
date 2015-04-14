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


#ifndef __SC_CORE_VERSION_H__
#define __SC_CORE_VERSION_H__


#include <string>
#include <seiscomp3/core.h>


namespace Seiscomp {
namespace Core {


/* #if (SC_API_VERSION >= SC_API_VERSION_CHECK(1, 15, 0)) */
#define SC_API_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))


/* SC_API_VERSION is (major << 16) + (minor << 10) + patch. */
#define SC_API_VERSION 0x010F00

#define SC_API_VERSION_MAJOR(v) (v >> 16)
#define SC_API_VERSION_MINOR(v) ((v >> 8) & 0xff)
#define SC_API_VERSION_PATCH(v) (v & 0xff)


/******************************************************************************
 API Changelog
 ******************************************************************************
 "1.15.0"  0x010F00
   - Added Seiscomp::Math::Geo::delazi_wgs84

 "1.14.0"  0x010E00
   - Changed return type of Seiscomp::DataModel::getThreeComponents to bool

 "1.13.0"  0x010D00
   - Changed Seiscomp::DataModel::DiffMerge::compareNonArrayProperty signature
     from Object* to BaseObject *
   - Added method Seiscomp::DataModel::DatabaseArchive::parentPublicID(...)

 "1.12.0"  0x010C00
   - Set Seiscomp::Seismology::LocatorInterface::locate(...) initTime to const

 "1.11.0"  0x010B00
   - Added const Record * as first parameter to
     Seiscomp::Processing::NCompsOperator<...>::Proc::operator()

 "1.10.0"  0x010A00
   - Derive Seiscomp::IO::QuakeLink::Connection from BaseObject

 "1.9.0"   0x010900
   - Renamed Seiscomp::Client::Application::version to
             Seiscomp::Client::Application::frameworkVersion
   - Added Seiscomp::Client::Application::version virtual method
   - Added Seiscomp::Client::Application::reloadInventory method

 "1.8.0"   0x010800
   - Added Seiscomp::Client::Application::reloadInventory method

 "1.7.0"   0x010700
   - Added Seiscomp::Processing::Picker::filterID method
   - Added Seiscomp::Processing::SecondaryPicker::filterID method

 "1.6.0"   0x010600
   - Added Seiscomp::IO::QuakeLink keep alive option

 "1.5.0"   0x010500
   - Added Seiscomp::Core::TimeSpan::MinTime and
           Seiscomp::Core::TimeSpan::MaxTime

 "1.4.0"   0x010400
   - Added clone method to Seiscomp::IO::RecordFilterInterface
   - Added Seiscomp::IO::RecordDemuxFilter

 "1.3.1"   0x010301
   - Fixed severe bug in Seiscomp::RecordStream::Decimation

 "1.3.0"   0x010300
   - Added Seiscomp::IO::RecordFilterInterface
     Added Seiscomp::IO::RecordIIRFilter
   - Added Seiscomp::IO::RecordResampler

 "1.2.0"   0x010200
   - Added Diff2 class to Seiscomp::DataModel
   - Added canPush, canPop methods to ThreadedQueue
   - Added acquititionError signal to RecordStreamThread

 "1.1.0"   0x010100
   - Added function DataModel::PublicObjectCache::cached()
   - Added function Gui::ImageTree::hasPendingRequests()
   - Added function Gui::Canvas::reload()
   - Added function Gui::Canvas::renderingComplete()
   - Added signal   Gui::Canvas::renderingCompleted()
   - Added function Gui::TextureCache::invalidateTexture(...)
 */


template <int major, int minor>
class VersionPacker {
	public:
		enum { Value = major << 0x10 | (minor & 0xFFFF) };
};


class SC_SYSTEM_CORE_API Version {
	// ----------------------------------------------------------------------
	// Traits
	// ----------------------------------------------------------------------
	public:
		typedef unsigned int   PackType;
		typedef unsigned short TagType;


	// ----------------------------------------------------------------------
	// Xstruction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		Version(PackType packed_version = 0) : packed(packed_version) {}
		Version(TagType major, TagType minor) {
			packed = pack(major, minor);
		}


	// ----------------------------------------------------------------------
	// Public interface
	// ----------------------------------------------------------------------
	public:
		TagType majorTag() const { return packed >> 0x10; }
		TagType minorTag() const { return packed & 0xFFFF; }

		std::string toString() const;
		bool fromString(const std::string &str);

		static PackType pack(TagType major, TagType minor) {
			return major << 0x10 | (minor & 0xFFFF);
		}


	// ----------------------------------------------------------------------
	// Operators
	// ----------------------------------------------------------------------
	public:
		// Operators
		bool operator==(const Version &other) const { return packed == other.packed; }
		bool operator!=(const Version &other) const { return packed != other.packed; }
		bool operator<(const Version &other) const { return packed < other.packed; }
		bool operator>(const Version &other) const { return packed > other.packed; }
		bool operator<=(const Version &other) const { return packed <= other.packed; }
		bool operator>=(const Version &other) const { return packed >= other.packed; }


	// ----------------------------------------------------------------------
	// Public members
	// ----------------------------------------------------------------------
	public:
		PackType packed;
};


class SC_SYSTEM_CORE_API FrameworkVersion {
	public:
		FrameworkVersion();

		//! Returns the version string
		std::string toString() const;

		//! Returns additional system build information
		std::string systemInfo() const;


	private:
		std::string _text;
};

extern FrameworkVersion CurrentVersion;


}
}


#endif
