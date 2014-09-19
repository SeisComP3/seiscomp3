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


#ifndef __SEISCOMP_PROCESSING_SENSOR_H__
#define __SEISCOMP_PROCESSING_SENSOR_H__


#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/processing/response.h>
#include <seiscomp3/client.h>


namespace Seiscomp {
namespace Processing  {


DEFINE_SMARTPOINTER(Sensor);

class SC_SYSTEM_CLIENT_API Sensor : public Core::BaseObject {
	// ----------------------------------------------------------------------
	//  X'truction
	// ----------------------------------------------------------------------
	public:
		Sensor();


	// ----------------------------------------------------------------------
	//  Public interface
	// ----------------------------------------------------------------------
	public:
		void setModel(const std::string& model);
		const std::string& model() const;

		void setManufacturer(const std::string& manufacturer);
		const std::string& manufacturer() const;

		void setType(const std::string& type);
		const std::string& type() const;

		void setUnit(const std::string& unit);
		const std::string& unit() const;

		void setLowFrequency(const OPT(double)& lowFrequency);
		double lowFrequency() const throw(Seiscomp::Core::ValueException);

		void setHighFrequency(const OPT(double)& highFrequency);
		double highFrequency() const throw(Seiscomp::Core::ValueException);

		Response *response() const;
		void setResponse(Response *response);


	// ----------------------------------------------------------------------
	//  Members
	// ----------------------------------------------------------------------
	private:
		ResponsePtr _response;

		std::string _model;
		std::string _manufacturer;
		std::string _type;
		std::string _unit;

		OPT(double) _lowFrequency;
		OPT(double) _highFrequency;
};


}
}

#endif
