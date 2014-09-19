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

// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename Expression>
LeParser<Expression>::LeParser(
		const LeParserTypes::Tokens& tokens,
		ExpressionFactoryInterface<Expression>* factory) :
			_tokens(tokens), _factory(factory)
{
	init();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename Expression>
LeParser<Expression>::~LeParser()
{
	if (_error)
	{
		while (_stack.size() > 0)
		{
			delete _stack.top();
			_stack.pop();
		}
	}
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename Expression>
void LeParser<Expression>::init()
{
	_error = false;
	_errorStr.clear();
	_it = _tokens.begin();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename Expression>
Expression* LeParser<Expression>::parse()
{
	while (_it != _tokens.end())
	{
		if (*_it == LeParserTypes::OperatorMap[LeParserTypes::AND])
		{
			_stack.push(_factory->createAndExpression());
			_modeStack.push(LeParserTypes::AND);
		}
		else if (*_it == LeParserTypes::OperatorMap[LeParserTypes::OR])
		{
			_stack.push(_factory->createOrExpression());
			_modeStack.push(LeParserTypes::OR);
		}
		else if (*_it == LeParserTypes::OperatorMap[LeParserTypes::NOT])
		{
			_stack.push(_factory->createNotExpression());
			_modeStack.push(LeParserTypes::NOT);
		}
		else
		{
			if (*_it == LeParserTypes::OperatorMap[LeParserTypes::POPEN])
			{
				++_it;
				_modeStack.push(LeParserTypes::POPEN);
				parse();
			}
			else if (*_it== LeParserTypes::OperatorMap[LeParserTypes::PCLOSE])
			{
				_modeStack.pop();
				return _stack.top();
			}
			else
			{
				Expression* tmp = _factory->createExpression(*_it);
				if (!tmp)
				{
					_error = true;
					_errorStr = "Could not create Expression: " + *_it;
					return NULL;
				}

				_stack.push(tmp);
			}

			if (_modeStack.size() > 0)
			{
				if (_modeStack.top() == LeParserTypes::AND || _modeStack.top() == LeParserTypes::OR)
				{
					applyBinaryOperator();
				}
				else if (_modeStack.top() == LeParserTypes::NOT)
				{
					applyUnaryOperator();
					if (_stack.size() > 0)
						applyBinaryOperator();
				}
			}
		}
		++_it;
	}

	if (_stack.size() > 1)
	{
		std::stringstream ss;
		ss << "ERROR, stack size is: " << _stack.size() << std::endl;
		_errorStr = ss.str();
		_error = true;
		return NULL;
	}
	return _stack.top();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename Expression>
bool LeParser<Expression>::error() const
{
	return _error;
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename Expression>
const char* LeParser<Expression>::what() const
{
	return _errorStr.c_str();
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename Expression>
void LeParser<Expression>::applyBinaryOperator()
{
	_modeStack.pop();

	Expression* lhs = _stack.top();
	_stack.pop();

	Expression* op = _stack.top();
	_stack.pop();

	Expression* rhs = _stack.top();
	_stack.pop();

	static_cast<BinaryOperator<Expression>* >(op)->setOperants(lhs, rhs);
	_stack.push(op);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<




// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
template <typename Expression>
void LeParser<Expression>::applyUnaryOperator()
{
	_modeStack.pop();

	Expression* rhs = _stack.top();
	_stack.pop();

	Expression* op = _stack.top();
	_stack.pop();

	static_cast<UnaryOperator<Expression>* >(op)->setOperant(rhs);
	_stack.push(op);
}
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
