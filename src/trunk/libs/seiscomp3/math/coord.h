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


#ifndef __SEISCOMP_MATH_GEO_COORD_H__
#define __SEISCOMP_MATH_GEO_COORD_H__

#include <seiscomp3/core/baseobject.h>
#include <string>


namespace Seiscomp
{

namespace Math
{

namespace Geo
{

template<typename T>
struct Coord : public Core::BaseObject {
	DECLARE_SERIALIZATION;

	typedef T ValueType;

	Coord();
	Coord(T lat_, T lon_);

	void set(T lat_, T lon_);

	T latitude() const { return lat; }
	T longitude() const { return lon; }

	bool operator==(const Coord<T> &other) const;
	bool operator!=(const Coord<T> &other) const;

	T lat;
	T lon;
};

typedef Coord<float> CoordF;
typedef Coord<double> CoordD;


template<typename T>
class NamedCoord : public Coord<T> {
	public:
		NamedCoord();
		NamedCoord(const std::string& name, T lat_, T lon_);

		~NamedCoord();

		using Coord<T>::set;
		void set(const std::string& name, T lat_, T lon_);

	public:
		void setName(const std::string& name);
		const std::string& name() const;

		void serialize(Core::BaseObject::Archive& ar);

	private:
		std::string _name;
};


typedef NamedCoord<float> NamedCoordF;
typedef NamedCoord<double> NamedCoordD;


template<typename T>
class City : public NamedCoord<T> {
	public:
		City();
		City(const std::string& name,
		     T lat_, T lon_, size_t population);

		City(const std::string& name, const std::string& countryID,
		     T lat_, T lon_, size_t population);

		~City();

	public:
		//! Population in thousands
		void setPopulation(double population);
		double population() const;

		void setCountryID(const std::string &);
		const std::string &countryID() const;

		void setCategory(std::string &);
		const std::string &category() const;

		void serialize(Core::BaseObject::Archive& ar);

	private:
		std::string _countryID;
		double _population;
		std::string _category;
};


typedef City<float> CityF;
typedef City<double> CityD;


} // of ns  Geo

} // of ns  Math

} // of ns Seiscomp


#endif
