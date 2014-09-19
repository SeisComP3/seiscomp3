/***************************************************************************
 * Copyright
 * ---------
 * This file is part of the Virtual Seismologist (VS) software package.
 * VS is free software: you can redistribute it and/or modify it under
 * the terms of the "SED Public License for Seiscomp Contributions"
 *
 * VS is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the SED Public License for Seiscomp
 * Contributions for more details.
 *
 * You should have received a copy of the SED Public License for Seiscomp
 * Contributions with VS. If not, you can find it at
 * http://www.seismo.ethz.ch/static/seiscomp_contrib/license.txt
 *
 * Authors of the Software: Michael Fischer and Yannik Behr
 * Copyright (C) 2006-2013 by Swiss Seismological Service
 ***************************************************************************/

// -*- C++ -*-
#ifndef NsSiteCondition_h__
#define NsSiteCondition_h__

#include <iostream>
#include <vector>

#include <seiscomp3/logging/log.h>

#include "timeline.h"

namespace ch {
namespace sed {

/**
 \brief Site conditions for VS.
 */
class VsSiteCondition {
public:

	/**
	 \brief Site Amplification Factor.
	 */
	class SAF {
	private:

		/**
		 Class name.
		 */
		std::string _cn;

		/**
		 Minimum velcoity the observed ground motion must exceed.
		 */
		float _vmax;

		/**
		 Correction values divided into four classes for maximum 
		 acceleration [cm/s/s] > 0, > 150, > 250, and > 350.
		 */
		float _corr[4];

		//
		SAF();

	public:

		//
		SAF(std::string cn, float vel, float c0, float c1, float c2, float c3);

		//
		virtual ~SAF();

		//
		virtual float getVmax();

		/**
		 Return the correction value specified by idx.
		 */
		virtual float getCorr(int idx);

		//
		virtual std::string toString();
	};

	/**
	 \brief List of site amplification factors divided into seven different
	 velocity classes.
	 */
	class SafList {
	private:

		/**
		 Correction values for acceleration.
		 */
		std::vector<SAF> _pgalist;

		/**
		 Same as above for velocity.
		 */
		std::vector<SAF> _pgvlist;

	public:

		/**
		 Constructor fills the two lists _pgalist and _pgvlist.
		 */
		SafList();

		//
		~SafList();

		//
		float getCorr(Seiscomp::ValueType valuetype, float vs30, float MA);

		//
		std::string toString();
	};
};

}
}

#endif

// eof
