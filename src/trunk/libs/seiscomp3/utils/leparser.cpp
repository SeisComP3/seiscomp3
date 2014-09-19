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

#include "leparser.h"

#include <seiscomp3/core/strings.h>


namespace Seiscomp {
namespace Utils {


std::map<LeParserTypes::Operator, std::string> LeParserTypes::OperatorMap;
LeParserTypes LeParserTypes::__parserTypes__;





// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
LeTokenizer::LeTokenizer(const std::string& expr) : _expr(expr)
{
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::vector<std::string>& LeTokenizer::tokens() const
{
	return _tokens;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool LeTokenizer::error() const
{
	return _error;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
const std::string& LeTokenizer::what() const
{
	return _errorStr;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LeTokenizer::init()
{
	_error = false;
	_errorStr.clear();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool LeTokenizer::tokenize()
{
	int parenthesesCounter = 0;

	std::string::const_iterator previous = _expr.begin();
	std::string::const_iterator current  = _expr.begin();
	std::string::const_iterator next     = _expr.begin();

	std::string token;
	for ( ; current != _expr.end(); ++current)
	{
		if (current != _expr.begin())
			previous = current - 1;

		if (current + 1 != _expr.end())
			next = current + 1;

		// Evaluate character
		if (std::string(1, *current) == LeParserTypes::OperatorMap[LeParserTypes::AND])
		{
			if (current == _expr.begin()) break;

			pushToken(token);
			_tokens.push_back(std::string(1, *current));
		}
		else if (std::string(1, *current) == LeParserTypes::OperatorMap[LeParserTypes::OR])
		{
			if (current == _expr.begin()) break;

			pushToken(token);
			_tokens.push_back(std::string(1, *current));
		}
		else if (std::string(1, *current) == LeParserTypes::OperatorMap[LeParserTypes::NOT])
		{
			pushToken(token);
			_tokens.push_back(std::string(1, *current));
		}
		else if (std::string(1, *current) == LeParserTypes::OperatorMap[LeParserTypes::POPEN])
		{
			++parenthesesCounter;

			pushToken(token);
			_tokens.push_back(std::string(1, *current));
		}
		else if (std::string(1, *current) == LeParserTypes::OperatorMap[LeParserTypes::PCLOSE])
		{
			--parenthesesCounter;

			pushToken(token);
			_tokens.push_back(std::string(1, *current));
		}
		else if (Core::WHITESPACE.find(*current) != std::string::npos)
		{
			pushToken(token);
		}
		else
		{
			token.push_back(*current);
		}
	}
	pushToken(token);

	if (parenthesesCounter < 0)
	{
		_errorStr = "Too many closing parentheses in " + _expr;
		_error = true;
		return false;
	}
	else if (parenthesesCounter > 0)
	{
		_errorStr = "Too many opening parentheses in " + _expr;
		_error = true;
		return false;
	}

 	return true;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
bool LeTokenizer::tokenize(const std::string& expr)
{
	_expr = expr;
	return tokenize();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void LeTokenizer::pushToken(std::string& token)
{
	if (token.size() > 0)
	{
		_tokens.push_back(token);
		token.clear();
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




} // namespace Utils
} // namespace Seiscomp
