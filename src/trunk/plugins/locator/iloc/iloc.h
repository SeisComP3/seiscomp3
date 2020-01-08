/***************************************************************************
 *   Copyright (C) gempa GmbH                                              *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/


#include <seiscomp3/core/version.h>


#ifndef __GEMPA_LOCATOR_ILOC_H__
#define __GEMPA_LOCATOR_ILOC_H__


#include <seiscomp3/seismology/locatorinterface.h>
#include <string>

extern "C" {

#include "sciLocInterface.h"

}


namespace Seiscomp {
namespace Plugins {


class ILoc : public Seismology::LocatorInterface {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		//! C'tor
		ILoc();

		//! D'tor
		~ILoc();


	// ----------------------------------------------------------------------
	//  Locator interface implementation
	// ----------------------------------------------------------------------
	public:
		//! Initializes the locator.
		virtual bool init(const Config::Config &config);

		//! Returns supported parameters to be changed.
		virtual IDList parameters() const;

		//! Returns the value of a parameter.
		virtual std::string parameter(const std::string &name) const;

		//! Sets the value of a parameter.
		virtual bool setParameter(const std::string &name,
		                          const std::string &value);

		//! Returns a list of supported profiles.
		virtual IDList profiles() const;

		//! Specify the Earth model to be used, e.g. "iasp91"
		virtual void setProfile(const std::string &name);

		//! Returns the locators capabilities which is an int or'ed from
		//! Capability enums.
		virtual int capabilities() const;

		virtual DataModel::Origin *locate(PickList &pickList)
		;

		virtual DataModel::Origin *locate(PickList &pickList,
		                                  double initLat, double initLon, double initDepth,
		                                  const Core::Time &initTime);
		virtual DataModel::Origin *relocate(const DataModel::Origin *origin);

		virtual std::string lastMessage(MessageType) const;


	// ----------------------------------------------------------------------
	//  Private methods
	// ----------------------------------------------------------------------
	private:
		void prepareAuxFiles();
		void initProfiles(const Config::Config *config, const std::string &auxdir);
		DataModel::Origin *fromPicks(PickList &picks);


	// ----------------------------------------------------------------------
	//  Private members
	// ----------------------------------------------------------------------
	private:
		static IDList _allowedParameters;

		struct AuxData {
			AuxData();
			~AuxData();

			void read(const iLocConfig *config);
			void free();

			ILOC_PHASEIDINFO  infoPhaseId;
			ILOC_TTINFO       infoTT;        /* global velocity model info and phase list */
			ILOC_TTINFO       infoLocalTT;   /* local velocity model info and phase list */
			ILOC_TT_TABLE    *tablesTT;      /* global travel-time tables */
			ILOC_TT_TABLE    *tablesLocalTT; /* local travel-time tables */
			ILOC_EC_COEF     *ec;            /* ellipticity correction coefficients */
			ILOC_VARIOGRAM    variogram;     /* generic variogram model */
			ILOC_FE           fe;            /* Flinn-Engdahl geographic region numbers */
			ILOC_DEFAULTDEPTH defaultDepth;  /* default depth and etopo structures */

			bool         useRSTT;
			bool         valid;
		};

		std::vector<ILOC_CONF> _profileConfigs;
		ILOC_CONF             *_currentConfig;
		AuxData                _aux;
		bool                   _auxDirty;

		IDList                 _profiles;
		//! Minimum arrival weight to regard an arrival as defining,
		//! default is 0.5
		double                 _minArrivalWeight;
		double                 _defaultPickUncertainty;
		bool                   _usePickUncertainties;
		bool                   _fixTime;
		bool                   _fixLocation;

		std::string            _tablePrefix;
		std::string            _tableDir;
};


}
}


#endif
