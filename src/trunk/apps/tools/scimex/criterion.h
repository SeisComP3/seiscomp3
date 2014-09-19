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



#ifndef __CRITERION_H__
#define __CRITERION_H__

#include <utility>
#include <vector>
#include <string>

#include <seiscomp3/utils/leparser.h>
#include <seiscomp3/client/application.h>


namespace Seiscomp {
namespace Applications {




class CriterionInterface : public Utils::LeExpression {

	public:
		CriterionInterface() {}
		virtual ~CriterionInterface() {}

	public:
		virtual bool isInLatLonRange(double lat, double lon) = 0;
		virtual bool isInMagnitudeRange(double mag) = 0;
		virtual bool checkArrivalCount(size_t count) = 0;
		virtual bool checkAgencyID(const std::string& id) = 0;
};




class Criterion : public CriterionInterface {

	// ----------------------------------------------------------------------
	// Nested Types
	// ----------------------------------------------------------------------
	private:
		typedef std::pair<double, double> DoublePair;

	public:
		typedef DoublePair LatitudeRange;
		typedef DoublePair LongitudeRange;
		typedef DoublePair MagnitudeRange;
		typedef std::vector<std::string> AgencyIDs;


	// ----------------------------------------------------------------------
	// X'struction
	// ----------------------------------------------------------------------
	public:
		virtual ~Criterion() {};


	// ----------------------------------------------------------------------
	// Public member functions
	// ----------------------------------------------------------------------
	public:
		const LatitudeRange& latitudeRange() const;
		void setLatitudeRange(double lat0, double lat1);

		const LongitudeRange& longitudeRange() const;
		void setLongitudeRange(double lon0, double lon1);

		const MagnitudeRange& magnitudeRange() const;
		void setMagnitudeRange(double mag0, double mag1);

		size_t arrivalCount() const;
		void setArrivalCount(size_t count);

		const AgencyIDs& agencyIDs() const;
		void setAgencyIDs(const AgencyIDs& ids);

		virtual bool isInLatLonRange(double lat, double lon);
		virtual bool isInMagnitudeRange(double mag);
		virtual bool checkArrivalCount(size_t count);
		virtual bool checkAgencyID(const std::string& id);


	// ----------------------------------------------------------------------
	// Private data members
	// ----------------------------------------------------------------------
	private:
		LatitudeRange  _latitudeRange;
		LongitudeRange _longitudeRange;
		MagnitudeRange _magnitudeRange;
		size_t         _arrivalCount;
		AgencyIDs      _agencyIDs;

};


template <typename Criterion>
class BinaryOperator : public Criterion {
	public:
		BinaryOperator() : _lhs(NULL), _rhs(NULL) {}

		void setOperants(const Criterion* lhs, const Criterion* rhs) {
			_lhs = lhs;
			_rhs = rhs;
		}

		virtual ~BinaryOperator() {
			if (_lhs) delete _lhs;
			if (_rhs) delete _rhs;
		}

	protected:
		Criterion* _lhs;
		Criterion* _rhs;
};




template <typename Criterion>
class UnaryOperator : public Criterion {
	public:
		UnaryOperator() : _rhs(NULL) {}

		void setOperant(const Criterion* rhs) {
			_rhs = rhs;
		}

		virtual ~UnaryOperator() {
			if (_rhs) delete _rhs;
		}

	protected:
		Criterion* _rhs;
};



class AndOperator : public BinaryOperator<CriterionInterface> {
	public:
		virtual bool isInLatLonRange(double lat, double lon)  {
			_boolean = _lhs->isInLatLonRange(lat, lon) && _rhs->isInLatLonRange(lat, lon);
			if (!_boolean) {
				missmatch("lat", lat);
				missmatch("lon", lon);
			}
			return _boolean;
		}
		virtual bool isInMagnitudeRange(double mag) {
			_boolean = _lhs->isInMagnitudeRange(mag) && _rhs->isInMagnitudeRange(mag);
			if (!_boolean) missmatch("mag", mag);
			return _boolean;
		}
		virtual bool checkArrivalCount(size_t count) {
			_boolean = _lhs->checkArrivalCount(count) && _rhs->checkArrivalCount(count);
			if (!_boolean) missmatch("arrivalCount", count);
			return _boolean;
		}
		virtual bool checkAgencyID(const std::string& id) {
			_boolean = _lhs->checkAgencyID(id) && _rhs->checkAgencyID(id);
			if (!_boolean) missmatch("agencyID", id);
			return _boolean;
		}

	private:
		template <typename T>
		void missmatch(const std::string& text, const T& val) {
			if (_lhs->error())
				append(_lhs->what());
			if (_rhs->error())
				append(_rhs->what());
			std::ostringstream ss;
			ss << "lhs(" << text << " = " << val << ") "
			<< Utils::LeParserTypes::OperatorMap[Utils::LeParserTypes::AND] << " rhs(" << text << " = " << val << ") == false";
			append(ss.str());
		}
	private:
		bool _boolean;
};


class OrOperator : public BinaryOperator<CriterionInterface> {
	public:
		virtual bool isInLatLonRange(double lat, double lon) {
			_boolean = _lhs->isInLatLonRange(lat, lon) || _rhs->isInLatLonRange(lat, lon);
			if (!_boolean) {
				missmatch("lat", lat);
				missmatch("lon", lon);
			}
			return _boolean;
		}
		virtual bool isInMagnitudeRange(double mag) {
			_boolean = _lhs->isInMagnitudeRange(mag) || _rhs->isInMagnitudeRange(mag);
			if (!_boolean) missmatch("mag", mag);
			return _boolean;
		}
		virtual bool checkArrivalCount(size_t count) {
			_boolean = _lhs->checkArrivalCount(count) || _rhs->checkArrivalCount(count);
			if (!_boolean) missmatch("arrivalCount", count);
			return _boolean;
		}
		virtual bool checkAgencyID(const std::string& id) {
			_boolean = _lhs->checkAgencyID(id) || _rhs->checkAgencyID(id);
			if (!_boolean) missmatch("agencyID", id);
			return _boolean;
		}

	private:
		template <typename T>
		void missmatch(const std::string& text, const T& val) {
			if (_lhs->error())
				append(_lhs->what());
			if (_rhs->error())
				append(_rhs->what());
			std::ostringstream ss;
			ss << "lhs(" << text << " = " << val << ") "
			<< Utils::LeParserTypes::OperatorMap[Utils::LeParserTypes::OR] << " rhs(" << text << " = " << val << ") == false";
			append(ss.str());
		}
	private:
		bool _boolean;
};




class NotOperator : public UnaryOperator<CriterionInterface> {
	public:
		virtual bool isInLatLonRange(double lat, double lon) {
			_boolean = !_rhs->isInLatLonRange(lat, lon);
			if (!_boolean) {
				missmatch("lat", lat);
				missmatch("lon", lon);
			}
			return _boolean;
		}
		virtual bool isInMagnitudeRange(double mag) {
			_boolean = !_rhs->isInMagnitudeRange(mag);
			if (!_boolean) missmatch("mag", mag);
			return _boolean;
		}
		virtual bool checkArrivalCount(size_t count) {
			_boolean = !_rhs->checkArrivalCount(count);
			if (!_boolean) missmatch("arrivalCount", count);
			return _boolean;
		}
		virtual bool checkAgencyID(const std::string& id) {
			_boolean = !_rhs->checkAgencyID(id);
			if (!_boolean) missmatch("agencyID", id);
			return _boolean;
		}

	private:
		template <typename T>
		void missmatch(const std::string& text, const T& val) {
			if (_rhs->error())
				append(_rhs->what());
			std::ostringstream ss;

			ss << Utils::LeParserTypes::OperatorMap[Utils::LeParserTypes::NOT] << "rhs(" << text << " = " << val << ") == false ";
			append(ss.str());
		}
	private:
		bool _boolean;

};




template <typename T>
class CriterionFactory : public Utils::ExpressionFactoryInterface<T> {

	public:
		CriterionFactory(const std::string& sinkName, Client::Application* app);
		virtual ~CriterionFactory() {}

		virtual T* createAndExpression();
		virtual T* createOrExpression();
		virtual T* createNotExpression();
		virtual T* createExpression(const std::string& name);

	private:
		bool configGetLatitude(const std::string& prefix, const std::string& name, Criterion* criterion);
		bool configGetLongitude(const std::string& prefix, const std::string& name, Criterion* criterion);
		bool configGetMagnitude(const std::string& prefix, const std::string& name, Criterion* criterion);
		bool configGetArrivalCount(const std::string& prefix, const std::string& name, Criterion* criterion);
		bool configGetAgencyID(const std::string& prefix, const std::string& name, Criterion* criterion);

		template <typename FuncObj>
		bool getRangeFromConfig(const std::string& prefix, const std::string& name, FuncObj func);


	private:
		std::string _sinkName;
		Client::Application* _app;
};


#include "criterion.ipp"



} // namespace Applications
} // namespace Seiscomp

#endif
