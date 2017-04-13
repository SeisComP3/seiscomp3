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


/* #if (SC_API_VERSION >= SC_API_VERSION_CHECK(10, 0, 0)) */
#define SC_API_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))


/* SC_API_VERSION is (major << 16) + (minor << 8) + patch. */
#define SC_API_VERSION 0x0A0000

#define SC_API_VERSION_MAJOR(v) (v >> 16)
#define SC_API_VERSION_MINOR(v) ((v >> 8) & 0xff)
#define SC_API_VERSION_PATCH(v) (v & 0xff)


/******************************************************************************
 API Changelog
 ******************************************************************************
 "10.0.0"   0x0A0000
   - Added Seiscomp::IO::GFArchive::getTravelTime(...)
   - Added Seiscomp::Math::WindowFunc and several implementations
   - Changed Seiscomp::Util::Bindings::getKeys to const
   - Added Seiscomp::Gui::Map:Canvas::prependLayer(...)
   - Added Seiscomp::Gui::Map:Canvas::insertLayerBefore(...)
   - Fixed bug in Seiscomp::Gui::Map::TextureCache that affected custom
     Seiscomp::Gui::Map::TileStore implementations
   - Added Seiscomp::Gui::RecordView::coveredTimeRange()

 "9.1.0"   0x090100
   - Added Seiscomp::Client::Application::Stage enum PLUGINS
   - Added Seiscomp::Gui::TensorRenderer::renderNP
   - Fixed parameter const'ness of Seiscomp::Math::Tensor2S

 "9.0.0"   0x090000
   - Added member creationInfo to class Seiscomp::DataModel::ConfigStation
     and increased datamodel version to 0.9
   - Added optional error code to Seiscomp::Communication class methods
   - Changed internal Seiscomp::Util::Timer API
   - Added Seiscomp::Util::Timer::setTimeout2
   - Added Seiscomp::Core::Archive::setStrictMode
   - Added Seiscomp::Core::Archive::isStrictMode
   - Added Seiscomp::IO::DatabaseInterface::numberOfAffectedRows
   - Added optional error code to Seiscomp::Inventory class
   - Added macro REREGISTER_CLASS which allows to overwrite class registrations
   - Added method Seiscomp::Gui::Alg::MapTreeNode::parent()
   - Allow Seiscomp::Gui::Map::TileStore::load to return null images
   - Increased TILESTORE version to 2

 "8.0.0"   0x080000
   - Added class Seiscomp::DataModel::ResponseFAP
   - Changed Seiscomp::IO::GFArchive::addRequest from 1D request signature to
     3D request signature (distance,depth) -> (source,receiver)
   - Made Seiscomp::RecordStream::SDSArchive private members and methods
     protected
   - Added GUI plotting library

 "7.0.0"   0x070000
   - Added support for httpmsgbus messaging protocol
   - Added Seiscomp::IO::HttpSocket
   - Added Seiscomp::Communication::NetworkInterface::setSequenceNumber
   - Added Seiscomp::Communication::NetworkInterface::getSequenceNumber
   - Added Seiscomp::Communication::SystemConnection::setSequenceNumber
   - Added Seiscomp::Communication::SystemConnection::getSequenceNumber
   - Modify Seiscomp::IO::Socket don't start timeout stopwatch automatically

 "6.1.0"   0x060100
   - Added Seiscomp::Gui::RecordWidget::setRecordStepFunction

 "6.0.0"   0x060000
   - Added virtual method Seiscomp::Record::clipMask()

 "5.1.0"   0x050100
   - Added Seiscomp::Core::BitSet

 "5.0.1"   0x050001
   - Renamed seiscomp3/math/filtering/rca.h to seiscomp3/math/filtering/average.h
     and made it a "real" average instead of an average of absolute values

 "5.0.0"   0x050000
   - Removed Seiscomp::Core::RecordSequence::continuousRecord(...)
   - Removed Seiscomp::Core::RecordSequence::fillGaps
   - Added template method Seiscomp::Core::RecordSequence::continuousRecord<T>(...)
   - Added Seiscomp::Processing::Picker::Config::noiseBegin
   - Added Seiscomp::Gui::Ruler::pixelPerUnit
   - Added Seiscomp::Gui::Ruler::scale

 "4.0.0"   0x040000
   - Added Seiscomp::System::ConfigDelegate::aboutToWrite
   - Added Seiscomp::System::ConfigDelegate::finishedWriting
   - Added Seiscomp::System::ConfigDelegate::hasWriteError
   - Added Seiscomp::System::ConfigDelegate::handleWriteTimeMismatch
   - Added delegate parameter to Seiscomp::System::*::writeConfig
   - Added Seiscomp::Gui::RecordStreamThread::setRecordHint
   - Added Seiscomp::Client::StreamApplication::setRecordDatatype
   - Added Seiscomp::Client::StreamApplication::recordDataType
   - Added Seiscomp::IO::DatabaseInterface::getRowFieldName virtual abstract
     method
   - Fixed Gui::RecordPolyline rendering with large offset
   - Added Seiscomp::Logging::Output::setUTCEnabled
   - Added Seiscomp::IO::QuakeLink::Response::disposed field

 "3.1.0"   0x030100
   - Change private to protected access in Seiscomp::IO::QuakeLink::Connection

 "3.0.0"   0x030000
   - Added Seiscomp::IO::RecordStream::filterRecord virtual method
   - Fixed bug in Seiscomp::IO::QuakeLink::Connection
   - Added Processing::Picker::Result::polarity

 "2.5.0"   0x020500
   - Added Seiscomp::Client::Application::configGetPath

 "2.4.0"   0x020400
   - Added Seiscomp::IO::BSONArchive
   - Added Seiscomp::IO::JSONArchive

 "2.3.0"   0x020300
   - Added Seiscomp::DataModel::Object::setLastModifiedInArchive
   - Added Seiscomp::DataModel::Object::lastModifiedInArchive
   - Populate Seiscomp::DataModel::Object::_lastModifiedInArchive
     with Seiscomp::DataModel::DatabaseArchive

 "2.2.0"   0x020200
   - Added optimization flag to Seiscomp::Gui::RecordPolyline::create
   - Added Seiscomp::Gui::RecordWidget::setRecordOptimization
   - Added Seiscomp::Gui::RecordWidget::traceInfo
   - Added Seiscomp::Gui::RecordWidget::areScaledValuesShown
   - Implement Seiscomp::Gui::Ruler::sizeHint for vertical layout

 "2.1.0"   0x020100
   - Removed Seiscomp::MultiComponentArray

 "2.0.0"   0x020000
   - Moved Processing::Parameters to Util::KeyValues
   - Renamed Processing::Parameters::readFrom to Util::KeyValues::init
   - Added Util::Bindings class

 "1.16.0"  0x011000
   - Added Seiscomp::IO::Spectralizer
   - Added Seiscomp::Gui::LUT
   - Added Seiscomp::Gui::StaticLUT
   - Added Seiscomp::Gui::StaticColorLUT
   - Added Seiscomp::Gui::SpectrogramRenderer
   - Added Seiscomp::Gui::SpectrogramWidget

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
