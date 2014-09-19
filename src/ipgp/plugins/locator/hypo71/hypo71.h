/************************************************************************
 *                                                                      *
 * Copyright (C) 2012 OVSM/IPGP                                         *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * This program is part of 'Projet TSUAREG - INTERREG IV Caraïbes'.     *
 * It has been co-financed by the European Union and le Ministère de    *
 * l'Ecologie, du Développement Durable, des Transports et du Logement. *
 *                                                                      *
 ************************************************************************/


#ifndef __IPGP_HYPO71_PLUGIN_H__
#define __IPGP_HYPO71_PLUGIN_H__



#include <seiscomp3/core/plugin.h>
#include <seiscomp3/seismology/locatorinterface.h>
#include <seiscomp3/config/config.h>
#include <string>



namespace Seiscomp {
namespace Seismology {

class Hypo71 : public LocatorInterface {

	public:
		// ------------------------------------------------------------------
		//  Nested types
		// ------------------------------------------------------------------
		struct Profile {
				std::string name;
				std::string earthModelID;
				std::string methodID;
				std::string controlFile;
				std::string velocityModelFile;
		};

		typedef std::map<std::string, std::string> StationMap;
		typedef std::vector<std::string> TextLines;
		typedef std::list<Profile> Profiles;

		enum GeographicPosition {
			gpLatitude = 0x01,
			gpLongitude = 0x02
		};

		enum StringType {
			stInteger = 0x01,
			stDouble = 0x02
		};

	public:
		// ------------------------------------------------------------------
		//  Instruction
		// ------------------------------------------------------------------
		explicit Hypo71();
		~Hypo71();

	public:
		// ------------------------------------------------------------------
		//  Public interface
		// ------------------------------------------------------------------
		/**
		 * @brief  Initializes the locator and reads at least the path of the default
		 *         control file used for Hypo71.
		 * @param  config the configuration file instance
		 * @return True on success, false otherwise
		 */
		bool init(const Config::Config& config);

		//! Returns supported parameters to be changed.
		IDList parameters() const;

		/**
		 * @brief  Returns the value of a parameter.
		 * @param  name parameter's name
		 * @return parameter value
		 */
		std::string parameter(const std::string& name) const;

		/**
		 * @brief  Sets the value of a parameter.
		 * @param  name parameter's name
		 * @param  value new parameter's value
		 * @return Ture on success, false otherwise
		 */
		bool setParameter(const std::string& name,
		                  const std::string& value);

		/**
		 * @brief  Generic way to get stored profiles from plugin instance
		 * @return Current stored profiles as a list
		 **/
		IDList profiles() const;

		/**
		 * @brief Specifies the earth model to be used\n
		 *        e.g. "Tectonic", "Volcanic", "Landslide", etc
		 * @param name the profile name
		 */
		void setProfile(const std::string& name);

		/**
		 * @brief  Evaluates capabilities by reaching out to the Locatorinterface
		 *         instance. In scolv, this translate by using fixed depth and/or
		 *         distance cutoff boolean values.
		 * @note   0 = true | 1 = false
		 * @return True/false
		 */
		int capabilities() const;

		/**
		 * @brief  Performs localization.
		 * @param  pickList object containing arrivals and phases
		 * @return New origin object
		 */
		DataModel::Origin*
		locate(PickList& pickList) throw (Core::GeneralException);

		/**
		 * @brief  Indirectly performs localization
		 * @param  pickList list of picks
		 * @param  initLat event initial latitude
		 * @param  initLon event initial longitude
		 * @param  initDepth event initial depth
		 * @param  initTime event initial time
		 * @return New origin object
		 */
		DataModel::Origin*
		locate(PickList& pickList, double initLat, double initLon,
		       double initDepth, Core::Time& initTime) throw (Core::GeneralException);

		/**
		 * @brief  Origin's relocator
		 * @param  origin origin object to relocate
		 * @return origin object
		 */
		DataModel::Origin*
		relocate(const DataModel::Origin* origin) throw (Core::GeneralException);

		/**
		 * @brief  Fetches last error message value
		 * @param  type the type of message to get
		 * @return "" or message value (string)
		 */
		std::string lastMessage(MessageType) const;

	private:
		// ------------------------------------------------------------------
		//  Private interface
		// ------------------------------------------------------------------
		std::string defaultParameter(const std::string& name) const;
		void setDefaultParameter(const std::string& name,
		                         const std::string& value);
		/**
		 * @brief Updates profile's parameters by reading control file values
		 * @param name as parameter's name in _profileName
		 */
		void updateProfile(const std::string& name);

		/**
		 * @brief  Generates extra blank characters if needed
		 * @param  toFormat string value to format
		 * @param  nb characters to achieve
		 * @param  pos 0 or 1 to add blank before or after passed string
		 * @return formated string
		 */
		std::string
		formatString(std::string toFormat, const size_t& nb, const size_t& pos,
		             const std::string& sender = "") throw (Core::GeneralException);

		/**
		 * @brief  Converts string parameter to double
		 * @param  s string to convert
		 * @return double value of passed string
		 */
		const double toDouble(const std::string& s);

		/**
		 * @brief  Converts passed string number into int number
		 * @param  str string to convert
		 * @return int value of passed string
		 */
		const int toInt(const std::string& str);

		/**
		 * @brief  Converts decimal latitude/longitudes into sexagesimal
		 *         equivalent fitting Hypo71 formalism
		 * @param  decimal value
		 * @param  Geographic situation gpLatitude/gpLongitude
		 * @return String of degmin.sec
		 */
		const std::string h71DecimalToSexagesimal(const double&,
		                                          const GeographicPosition&);

		/**
		 * @brief  Selects best ZTR value for seismic localization
		 * @param  pickList list of picks of a origin
		 * @return ZRT value as a string
		 **/
		const std::string
		getZTR(const PickList& pickList) throw (Core::GeneralException);

		/**
		 * @brief  Converts HYPO71 sexagesimal origin (73° 59.14’ W)
		 *         in a decimal origin (-73.9874°).
		 * @param  deg double degrees input value
		 * @param  min double decimated minute value
		 * @param  pol string polar value of sexagesimal input (E/W/N/S)
		 * @return (string) decimal value of sexagesimal input
		 **/
		const std::string SexagesimalToDecimalHypo71(const double& deg,
		                                             const double& min,
		                                             const std::string& pol);

		const std::string getSituation(const double&, const GeographicPosition&);

		/**
		 * @brief  Retrieves time value from picklist object
		 * @param  pickList the C++ object picklist to iterate
		 * @param  name string value of the station to locate
		 * @param  phase string value of the phase to locate
		 * @param  rtype return type value (0/1/2/3 ctime/hour/minute/sec.dec)
		 * @return time value as a double, if it has been found
		 **/
		const double getTimeValue(const PickList& pickList,
		                          const std::string& networkCode,
		                          const std::string& stationCode,
		                          const std::string& phaseCode,
		                          unsigned int rtype);

		/**
		 * @brief  Returns the polarity of a pick
		 * @param  pickList the picklist to iterate
		 * @param  networkCode the station network code
		 * @param  stationCode the station code
		 * @param  phaseCode the pick phase code
		 * @return U, D, or ' '
		 */
		const std::string getPickPolarity(const PickList& pickList,
		                                  const std::string& networkCode,
		                                  const std::string& stationCode,
		                                  const std::string& phaseCode);


		const bool stringIsOfType(const std::string&, const StringType&);

		/**
		 * @brief Explodes a string using separator as a cut point
		 * @param str string to explode
		 * @param separator cut point
		 * @param results vector of exploded string
		 */
		void stringExplode(std::string str, std::string separator,
		                   std::vector<string>* results);

		/**
		 * @brief  Removes white spaces from passed string
		 * @param  str string to strip whites paces
		 * @return stripped string of passed string
		 **/
		std::string stripSpace(string& str);

		/**
		 * @brief  Evaluates and converts weights from upper/lower time
		 *         uncertainties into Hypo71 1-4 weight value
		 * @param  pickList the list of picks
		 * @param  networkCode the station's network code
		 * @param  stationCode the station's code
		 * @param  phaseCode the station's phase code
		 * @param  max the maximum value of weight
		 * @return Hypo71 weight value as integer
		 */
		const int getH71Weight(const PickList& pickList,
		                       const std::string& networkCode,
		                       const std::string& stationCode,
		                       const std::string& phaseCode,
		                       const double& max);

		/**
		 * @brief This method adds a station into the mapped list by giving it
		 *        a unique translated name which will serve as id in Hypo71
		 *        input and output files.
		 * @note  SEED allows 5chars station names, but Hypo71 doesn't... This
		 *        method will make sure everything is set up to perform a Hypo71
		 *        localization with custom names for stations reading/writing.
		 * @param station's network code
		 * @param station's code
		 */
		void addNewStation(const std::string&, const std::string&);

		/**
		 * @brief  Iterates thru the station mapped list and recovers the unique
		 *         id of a station.
		 * @param  station's network
		 * @param  station's code
		 * @return station's mapped unique name or an empty string if nothing
		 *         has been found.
		 */
		const std::string getStationMappedCode(const std::string&,
		                                       const std::string&);

		/**
		 * @brief  Iterates thru the station mapped list and recovers the station
		 *         original name
		 * @param  station's mapped unique name
		 * @return station's original name NET.STATION or an empty string if
		 *         nothing has been found
		 */
		const std::string getOriginalStationCode(const std::string&);

	private:
		// ------------------------------------------------------------------
		//  Members
		// ------------------------------------------------------------------
		struct ResetList {
				std::string test01;
				std::string test02;
				std::string test03;
				std::string test04;
				std::string test05;
				std::string test06;
				std::string test07;
				std::string test08;
				std::string test09;
				std::string test10;
				std::string test11;
				std::string test12;
				std::string test13;
				std::string test15;
				std::string test20;
		};

		struct ControlCard {
				std::string ztr;
				std::string xnear;
				std::string xfar;
				std::string pos;
				std::string iq;
				std::string kms;
				std::string kfm;
				std::string ipun;
				std::string imag;
				std::string ir;
				std::string iprn;
				std::string ktest;
				std::string kaz;
				std::string ksort;
				std::string ksel;
		};

		struct FullControlCard {
				std::string ztr;
				std::string xnear;
				std::string xfar;
				std::string pos;
				std::string iq;
				std::string kms;
				std::string kfm;
				std::string ipun;
				std::string imag;
				std::string ir;
				std::string iprn;
				std::string ktest;
				std::string kaz;
				std::string ksort;
				std::string ksel;
				std::string lat1;
				std::string lat2;
				std::string lon1;
				std::string lon2;
		};

		struct InstructionCard {
				std::string ipro;
				std::string knst;
				std::string inst;
				std::string zres;
		};

		static IDList _allowedParameters;

		bool _allowMissingStations;
		bool _useLastOriginAsReference;
		bool _useHypo71PatternID;

		double _fixedDepthGridSpacing;
		double _defaultPickError;
		double _oLastLatitude;
		double _oLastLongitude;

		std::string _publicIDPattern;
		std::string _outputPath;
		std::string _h71inputFile;
		std::string _h71outputFile;
		std::string _controlFilePath;
		std::string _lastWarning;
		std::string _hypo71ScriptFile;
		std::string _logFile;
		std::string _currentOriginID;
		std::string velocityModel;
		std::string depthModel;

		TextLines _controlFile;
		IDList _profileNames;
		StationMap _stationMap;

		ParameterMap _parameters;
		Profiles _profiles;
		Profile* _currentProfile;
};

}
}

#endif

