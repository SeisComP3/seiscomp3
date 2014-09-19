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

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
template <typename T>
void Config::add(const std::string& name, const T& value)
{
	Symbol symbol;
	symbol.name = name;
	symbol.values.push_back(Private::toString(value));
	symbol.uri = "";

	_symbolTable->add(symbol);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
template <>
void Config::add<std::string>(const std::string& name, const std::string& value)
{
	Symbol symbol;
	symbol.name = name;
	symbol.values.push_back(value);
	symbol.uri = "";

	_symbolTable->add(symbol);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
template <typename T>
void Config::add(const std::string& name, const std::vector<T>& values)
{
	Symbol symbol;
	symbol.name = name;
	for (size_t i = 0; i < values.size(); ++i)
		symbol.values.push_back(Private::toString(values[i]));
	symbol.uri = "";

	_symbolTable->add(symbol);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
template <>
void Config::add<std::string>(const std::string& name, const std::vector<std::string>& values)
{
	Symbol symbol;
	symbol.name = name;
	for (size_t i = 0; i < values.size(); ++i)
		symbol.values.push_back(values[i]);
	symbol.uri = "";

	_symbolTable->add(symbol);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
template <typename T>
bool Config::set(const std::string& name, const T& value)
{
	Symbol* symbol = _symbolTable->get(name);
	if (!symbol)
	{
		add<T>(name, value);
		return true;
	}

	symbol->values.clear();
	symbol->values.push_back(Private::toString(value));
	symbol->uri = "";

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
template <typename T>
bool Config::set(const std::string& name, const std::vector<T>& values)
{
	Symbol* symbol = _symbolTable->get(name);
	if (!symbol)
	{
		add<T>(name, values);
		return true;
	}

	symbol->values.clear();
	for (size_t i = 0; i < values.size(); ++i)
		symbol->values.push_back(Private::toString(values[i]));

	symbol->uri = "";

	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
template <typename T>
T Config::get(const std::string& name) const
throw(Exception)
{
	const Symbol* symbol = _symbolTable->get(name);
	if (!symbol)
		throw OptionNotFoundException(name);

	T value = T();
	if (!Private::fromString(value, symbol->values[0]))
		throw TypeConversionException(symbol->values[0]);

	return value;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
template <typename T>
std::vector<T> Config::getVec(const std::string& name) const
throw(Exception)
{
	const Symbol* symbol = _symbolTable->get(name);
	if (!symbol)
		throw OptionNotFoundException(name);

	std::vector<T> values;
	for (size_t i = 0; i < symbol->values.size(); ++i)
	{
		T tmp = T();
		if (!Private::fromString(tmp, symbol->values[i]))
			throw TypeConversionException(symbol->values[i]);
		values.push_back(tmp);
	}

	return values;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
template <typename T>
T Config::get(const std::string& name, bool* error) const
{
	*error = false;
	try {
		return get<T>(name);
	}
	catch (...) {
		*error = true;
		return T();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
template <typename T>
bool Config::get(T& value, const std::string& name) const
{
	try {
		value = get<T>(name);
		return true;
	} catch (...) {
		return false;
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<





// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
template <typename T>
std::vector<T> Config::getVec(const std::string& name, bool* error) const
{
	*error = false;
	try {
		return getVec<T>(name);
	}
	catch (...) {
		 *error = true;
		return std::vector<T>();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
