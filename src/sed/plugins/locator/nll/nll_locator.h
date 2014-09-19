/***************************************************************************
* Copyright (C) 2010 by Jan Becker, gempa GmbH                             *
* EMail: jabe@gempa.de                                                     *
*                                                                          *
* This code has been developed for the SED/ETH Zurich and is               *
* released under the SeisComP Public License.                              *
***************************************************************************/


#include <seiscomp3/core/plugin.h>
#include <seiscomp3/seismology/locatorinterface.h>
#include <string>


namespace Seiscomp {

namespace Seismology {

namespace Plugins {


class NLLocator : public Seiscomp::Seismology::LocatorInterface {
	// ----------------------------------------------------------------------
	//  Public types
	// ----------------------------------------------------------------------
	public:
		DEFINE_SMARTPOINTER(Region);

		struct Region : public Core::BaseObject {
			virtual bool isGlobal() const { return false; }
			virtual bool init(const Config::Config &config, const std::string &prefix) = 0;
			virtual bool isInside(double lat, double lon) const = 0;
		};


	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		NLLocator();

		//! D'tor
		~NLLocator();


	// ----------------------------------------------------------------------
	//  Locator interface implementation
	// ----------------------------------------------------------------------
	public:
		//! Initializes the locator and reads at least the path of the
		//! default control file used for NLLoc.
		virtual bool init(const Config::Config &config);

		//! Returns supported parameters to be changed.
		virtual IDList parameters() const;

		//! Returns the value of a parameter.
		virtual std::string parameter(const std::string &name) const;

		//! Sets the value of a parameter.
		virtual bool setParameter(const std::string &name,
		                          const std::string &value);

		virtual IDList profiles() const;

		//! specify the Earth model to be used, e.g. "iasp91"
		virtual void setProfile(const std::string &name);

		virtual int capabilities() const;

		virtual DataModel::Origin* locate(PickList& pickList)
		        throw(Core::GeneralException);
		virtual DataModel::Origin* locate(PickList& pickList,
		                                  double initLat, double initLon, double initDepth,
		                                  Core::Time &initTime)
		        throw(Core::GeneralException);
		virtual DataModel::Origin* relocate(const DataModel::Origin* origin)
		        throw(Core::GeneralException);

		virtual std::string lastMessage(MessageType) const;


	// ----------------------------------------------------------------------
	//  Private methods
	// ----------------------------------------------------------------------
	private:
		void updateProfile(const std::string &name);

		bool NLL2SC3(DataModel::Origin *origin, std::string &locComment,
		             const void *node, const PickList &picks,
		             bool depthFixed);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		struct Profile {
			std::string name;
			std::string earthModelID;
			std::string methodID;
			std::string tablePath;
			std::string controlFile;
			RegionPtr   region;
		};

		typedef std::map<std::string, std::string> ParameterMap;
		typedef std::vector<std::string> TextLines;
		typedef std::list<Profile> Profiles;

		static IDList _allowedParameters;

		std::string   _publicIDPattern;
		std::string   _outputPath;
		std::string   _controlFilePath;
		std::string   _lastWarning;
		std::string   _SEDqualityTag;
		std::string   _SEDdiffMaxLikeExpectTag;
		TextLines     _controlFile;
		IDList        _profileNames;

		double        _fixedDepthGridSpacing;
		double        _defaultPickError;
		bool          _allowMissingStations;
		bool          _enableSEDParameters;
		bool          _enableNLLOutput;
		bool          _enableNLLSaveInput;

		ParameterMap  _parameters;
		Profiles      _profiles;
		Profile      *_currentProfile;
};


}

}

}
