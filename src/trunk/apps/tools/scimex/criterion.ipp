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

template <typename T>
CriterionFactory<T>::CriterionFactory(const std::string& sinkName, Client::Application* app) :
	_sinkName(sinkName), _app(app)
{}




template <typename T>
T* CriterionFactory<T>::createAndExpression()
{
	return new AndOperator;
}




template <typename T>
T* CriterionFactory<T>::createOrExpression()
{
	return new OrOperator;
}




template <typename T>
T* CriterionFactory<T>::createNotExpression()
{
	return new NotOperator;
}




template <typename T>
T* CriterionFactory<T>::createExpression(const std::string& name)
{
	Criterion* criterion = new Criterion;
	if (!configGetLatitude("criteria." + name, "latitude", criterion))
	{
		delete criterion;
		return NULL;
	}

	if (!configGetLongitude("criteria." + name, "longitude", criterion))
	{
		delete criterion;
		return NULL;
	}

	if (!configGetMagnitude("criteria." + name, "magnitude", criterion))
	{
		delete criterion;
		return NULL;
	}

	if (!configGetArrivalCount("criteria." + name, "arrivalCount", criterion))
	{
		delete criterion;
		return NULL;
	}

	if (!configGetAgencyID("criteria." + name, "agencyID", criterion))
	{
		delete criterion;
		return NULL;
	}

	return criterion;
}





template <typename T>
bool CriterionFactory<T>::configGetLatitude(const std::string& prefix, const std::string& name, Criterion* criterion)
{
	return getRangeFromConfig(prefix, name, boost::bind(&Criterion::setLatitudeRange, criterion, _1, _2));
}




template <typename T>
bool CriterionFactory<T>::configGetLongitude(const std::string& prefix, const std::string& name, Criterion* criterion)
{
	return getRangeFromConfig(prefix, name, boost::bind(&Criterion::setLongitudeRange, criterion, _1, _2));

}




template <typename T>
bool CriterionFactory<T>::configGetMagnitude(const std::string& prefix, const std::string& name, Criterion* criterion)
{
	return getRangeFromConfig(prefix, name, boost::bind(&Criterion::setMagnitudeRange, criterion, _1, _2));
}




template <typename T>
bool CriterionFactory<T>::configGetArrivalCount(const std::string& prefix, const std::string& name, Criterion* criterion)
{
	try {
		criterion->setArrivalCount(_app->configGetInt(prefix + ".arrivalcount"));
	}
	catch (Config::Exception& e) {
		std::cout << "(" << e.what() << ") " << _sinkName << std::endl;
		return false;
	}
	return true;
}




template <typename T>
bool CriterionFactory<T>::configGetAgencyID(const std::string& prefix, const std::string& name, Criterion* criterion)
{
	std::vector<std::string> tokens;
	try
	{
		std::string agencyIDs =  _app->configGetString(prefix + ".agencyID");
		if (!agencyIDs.empty())
		{
			Core::split(tokens, agencyIDs.c_str(), ",");
			std::vector<std::string>::iterator it = tokens.begin();
			for ( ; it != tokens.end(); ++it)
				Core::trim(*it);
			criterion->setAgencyIDs(tokens);
		}
	}
	catch (Config::Exception& e) {
		std::cout << "(" << e.what() << ") " << _sinkName << std::endl;
		return false;
	}
	return true;
}




template <typename T>
template <typename FuncObj>
bool CriterionFactory<T>::getRangeFromConfig(const std::string& prefix, const std::string& name, FuncObj funcObj)
{
	std::vector<std::string> tokens;
	try {
		double val0 = 0, val1 = 0;
		std::string range = _app->configGetString(prefix + "." + name);
		if (Core::split(tokens, range.c_str(), ":") == 2)
		{
			if (!Core::fromString(val0, tokens[0]))
			{
				std::cout << "(" << _sinkName.c_str() << ")" <<  "Malformed " << name << " : " <<  tokens[0] << " Bailing out" << std::endl;
				return false;
			}
			if (!Core::fromString(val1, tokens[1]))
			{
				std::cout << "(" << _sinkName.c_str() << ")" <<  "Malformed " << name << " : " <<  tokens[1] << " Bailing out" << std::endl;
				return false;
			}
			funcObj(val0, val1);
		}
		else
		{
			std::cout << "(" << _sinkName << ") Malformed " << name <<  " range: " << range << std::endl;
			return false;
		}
	}
	catch (Config::Exception& e) {
		std::cout << "(" << e.what() << ") " << _sinkName << std::endl;
		return false;
	}

	return true;
}

