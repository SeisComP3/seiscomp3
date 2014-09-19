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

#ifndef __SEISCOMP_APPLICATIONS_MONITORFILTER_H__
#define __SEISCOMP_APPLICATIONS_MONITORFILTER_H__

//#define BOOST_SPIRIT_DEBUG
#include <boost/version.hpp>

#if BOOST_VERSION < 103800
#include <boost/spirit.hpp>
#include <boost/spirit/tree/ast.hpp>
namespace bs = boost::spirit;
#else
#include <boost/spirit/include/classic.hpp>
#include <boost/spirit/include/classic_ast.hpp>
namespace bs = boost::spirit::classic;
#endif

#include <seiscomp3/core/strings.h>


using namespace bs;


namespace Seiscomp {
namespace Applications {


// Definition of the parser grammar
struct MFilterParser : public bs::grammar<MFilterParser> {
	// Parser ids
	enum ParserID { rexpID = 1, notLexpID, groupID, expressionID };

	template <typename ScannerT>
	struct definition {
		definition(const MFilterParser& self) {
			TAG_TOKEN =
				bs::leaf_node_d[ (
					bs::str_p("privategroup") |
					bs::str_p("hostname") |
					bs::str_p("clientname") |
					bs::str_p("ips") |
					bs::str_p("programname") |
					bs::str_p("pid") |
					bs::str_p("cpuusage") |
					bs::str_p("totalmemory") |
					bs::str_p("clientmemoryusage") |
					bs::str_p("memoryusage") |
					bs::str_p("sentmessages") |
					bs::str_p("receivedmessages") |
					bs::str_p("messagequeuesize") |
					bs::str_p("summedmessagequeuesize") |
					bs::str_p("averagemessagequeuesize") |
					bs::str_p("summedmessagesize") |
					bs::str_p("averagemessagesize") |
					bs::str_p("objectcount") |
					bs::str_p("uptime") |
					bs::str_p("responsetime")
					) ]
				;

			ROP_TOKEN =
				  (
					bs::str_p("==")
				  | bs::str_p("!=")
				  | bs::str_p("<")
				  | bs::str_p(">")
				  | bs::str_p(">=")
				  | bs::str_p("<=")
				  )
				;

			rexp = TAG_TOKEN >> ROP_TOKEN >> bs::leaf_node_d[+(bs::alnum_p | bs::ch_p("_"))]
			;

			group =
				  bs::ch_p("(") >>
				  +expression >>
				  ch_p(")")
				;

			notLexp =
				bs::root_node_d[ bs::str_p("!") ] >> group
				;

			expression =
				(rexp | group | notLexp) >>
				*((bs::root_node_d[ str_p("&&") ] | bs::root_node_d[ str_p("||") ]) >> (rexp | group | notLexp))
				;

			BOOST_SPIRIT_DEBUG_NODE(TAG_TOKEN);
			BOOST_SPIRIT_DEBUG_NODE(ROP_TOKEN);
			BOOST_SPIRIT_DEBUG_NODE(rexp);
			BOOST_SPIRIT_DEBUG_NODE(group);
			BOOST_SPIRIT_DEBUG_NODE(expression);

		}


		const bs::rule<ScannerT, bs::parser_context<>, bs::parser_tag<expressionID> >&
		start() const {
			return expression;
		}

		bs::rule<ScannerT, bs::parser_context<>, bs::parser_tag<rexpID> >       rexp;
		bs::rule<ScannerT, bs::parser_context<>, bs::parser_tag<notLexpID> >    notLexp;
		bs::rule<ScannerT, bs::parser_context<>, bs::parser_tag<groupID> >      group;
		bs::rule<ScannerT, bs::parser_context<>, bs::parser_tag<expressionID> > expression;
		bs::rule<ScannerT> TAG_TOKEN, ROP_TOKEN, LOP_TOKEN;
	};

};



// Filter interface
template <typename Interface>
class MBinaryOperator : public Interface {
	public:
		MBinaryOperator() : _lhs(0), _rhs(0) {}

	public:
		void setOperants(void* lhs, void* rhs) {
			_lhs = lhs;
			_rhs = rhs;
		}

	protected:
		void* _lhs;
		void* _rhs;
};


template <typename Interface>
class MUnaryOperator : public Interface {
	public:
		MUnaryOperator() : _lhs(0) {}
	public:
		void setOperant(void* rhs) {
			_lhs = rhs;
		}
	protected:
		void* _lhs;
};


class MFilterInterface {
	public:
		virtual ~MFilterInterface() {}
		virtual bool eval(const ClientInfoData& clientData) const = 0;
};


#define DEFINE_LOPERATOR(name, op) \
class name : public MBinaryOperator<MFilterInterface> { \
	public: \
		virtual bool eval(const ClientInfoData& clientData) const { \
			if (!_lhs || !_rhs) \
				return false; \
			return ((MFilterInterface*)_lhs)->eval(clientData) op ((MFilterInterface*)_rhs)->eval(clientData); \
		} \
};

DEFINE_LOPERATOR(MAndOperator, &&)
DEFINE_LOPERATOR(MOrOperator, ||)

class MNotOperator : public MUnaryOperator<MFilterInterface> {
	public:
		virtual bool eval(const ClientInfoData& clientData) const {
			if (!_lhs)
				return false;
			return !((MFilterInterface*)_lhs)->eval(clientData);
		}
};


template <Communication::EConnectionInfoTag tag>
bool getClientData(const ClientInfoData& clientData, typename Communication::ConnectionInfoT<tag>::Type& val) {

	ClientInfoData::const_iterator it = clientData.find(tag);
	if (it == clientData.end())
		return false;
	if (!Core::fromString(val, it->second))
		return false;
	return true;
}


#define DEFINE_ROPERATOR(name, op) \
class name : public MBinaryOperator<MFilterInterface> { \
	public: \
		virtual bool eval(const ClientInfoData& clientData) const { \
			switch( *((Communication::ConnectionInfoTag*)_lhs) ) { \
				case Communication::PRIVATE_GROUP_TAG: { \
					Communication::ConnectionInfoT<Communication::PRIVATE_GROUP_TAG>::Type val; \
					if ( !getClientData<Communication::PRIVATE_GROUP_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::PRIVATE_GROUP_TAG>::Type*)_rhs); \
				} \
				case Communication::HOSTNAME_TAG: { \
					Communication::ConnectionInfoT<Communication::HOSTNAME_TAG>::Type val; \
					if ( !getClientData<Communication::HOSTNAME_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::HOSTNAME_TAG>::Type*)_rhs); \
				} \
				case Communication::CLIENTNAME_TAG: { \
					Communication::ConnectionInfoT<Communication::CLIENTNAME_TAG>::Type val; \
					if ( !getClientData<Communication::CLIENTNAME_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::CLIENTNAME_TAG>::Type*)_rhs); \
				} \
				case Communication::IPS_TAG: { \
					Communication::ConnectionInfoT<Communication::IPS_TAG>::Type val; \
					if ( !getClientData<Communication::IPS_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::IPS_TAG>::Type*)_rhs); \
				} \
				case Communication::PROGRAMNAME_TAG: { \
					Communication::ConnectionInfoT<Communication::PROGRAMNAME_TAG>::Type val; \
					if ( !getClientData<Communication::PROGRAMNAME_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::PROGRAMNAME_TAG>::Type*)_rhs); \
				} \
				case Communication::PID_TAG: { \
					Communication::ConnectionInfoT<Communication::PID_TAG>::Type val; \
					if ( !getClientData<Communication::PID_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::PID_TAG>::Type*)_rhs); \
				} \
				case Communication::CPU_USAGE_TAG: { \
					Communication::ConnectionInfoT<Communication::CPU_USAGE_TAG>::Type val; \
					if ( !getClientData<Communication::CPU_USAGE_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::CPU_USAGE_TAG>::Type*)_rhs); \
				} \
				case Communication::TOTAL_MEMORY_TAG: { \
					Communication::ConnectionInfoT<Communication::TOTAL_MEMORY_TAG>::Type val; \
					if ( !getClientData<Communication::TOTAL_MEMORY_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::TOTAL_MEMORY_TAG>::Type*)_rhs); \
				} \
				case Communication::CLIENT_MEMORY_USAGE_TAG: { \
					Communication::ConnectionInfoT<Communication::CLIENT_MEMORY_USAGE_TAG>::Type val; \
					if ( !getClientData<Communication::CLIENT_MEMORY_USAGE_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::CLIENT_MEMORY_USAGE_TAG>::Type*)_rhs); \
				} \
				case Communication::MEMORY_USAGE_TAG: { \
					Communication::ConnectionInfoT<Communication::MEMORY_USAGE_TAG>::Type val; \
					if ( !getClientData<Communication::MEMORY_USAGE_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::MEMORY_USAGE_TAG>::Type*)_rhs); \
				} \
				case Communication::SENT_MESSAGES_TAG: { \
					Communication::ConnectionInfoT<Communication::SENT_MESSAGES_TAG>::Type val; \
					if ( !getClientData<Communication::SENT_MESSAGES_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::SENT_MESSAGES_TAG>::Type*)_rhs); \
				} \
				case Communication::RECEIVED_MESSAGES_TAG: { \
					Communication::ConnectionInfoT<Communication::RECEIVED_MESSAGES_TAG>::Type val; \
					if ( !getClientData<Communication::RECEIVED_MESSAGES_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::RECEIVED_MESSAGES_TAG>::Type*)_rhs); \
				} \
				case Communication::MESSAGE_QUEUE_SIZE_TAG: { \
					Communication::ConnectionInfoT<Communication::MESSAGE_QUEUE_SIZE_TAG>::Type val; \
					if ( !getClientData<Communication::MESSAGE_QUEUE_SIZE_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::MESSAGE_QUEUE_SIZE_TAG>::Type*)_rhs); \
				} \
				case Communication::SUMMED_MESSAGE_QUEUE_SIZE_TAG: { \
					Communication::ConnectionInfoT<Communication::SUMMED_MESSAGE_QUEUE_SIZE_TAG>::Type val; \
					if ( !getClientData<Communication::SUMMED_MESSAGE_QUEUE_SIZE_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::SUMMED_MESSAGE_QUEUE_SIZE_TAG>::Type*)_rhs); \
				} \
				case Communication::AVERAGE_MESSAGE_QUEUE_SIZE_TAG: { \
					Communication::ConnectionInfoT<Communication::AVERAGE_MESSAGE_QUEUE_SIZE_TAG>::Type val; \
					if ( !getClientData<Communication::AVERAGE_MESSAGE_QUEUE_SIZE_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::AVERAGE_MESSAGE_QUEUE_SIZE_TAG>::Type*)_rhs); \
				} \
				case Communication::SUMMED_MESSAGE_SIZE_TAG: { \
					Communication::ConnectionInfoT<Communication::SUMMED_MESSAGE_SIZE_TAG>::Type val; \
					if ( !getClientData<Communication::SUMMED_MESSAGE_SIZE_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::SUMMED_MESSAGE_SIZE_TAG>::Type*)_rhs); \
				} \
				case Communication::AVERAGE_MESSAGE_SIZE_TAG: { \
					Communication::ConnectionInfoT<Communication::AVERAGE_MESSAGE_SIZE_TAG>::Type val; \
					if ( !getClientData<Communication::AVERAGE_MESSAGE_SIZE_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::AVERAGE_MESSAGE_SIZE_TAG>::Type*)_rhs); \
				} \
				case Communication::OBJECT_COUNT_TAG: { \
					Communication::ConnectionInfoT<Communication::OBJECT_COUNT_TAG>::Type val; \
					if ( !getClientData<Communication::OBJECT_COUNT_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::OBJECT_COUNT_TAG>::Type*)_rhs); \
				} \
				case Communication::UPTIME_TAG: { \
					Communication::ConnectionInfoT<Communication::UPTIME_TAG>::Type val; \
					if ( !getClientData<Communication::UPTIME_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::UPTIME_TAG>::Type*)_rhs); \
				} \
				case Communication::RESPONSE_TIME_TAG: { \
					Communication::ConnectionInfoT<Communication::RESPONSE_TIME_TAG>::Type val; \
					if ( !getClientData<Communication::RESPONSE_TIME_TAG>(clientData, val) ) \
						return false; \
					return val op *((Communication::ConnectionInfoT<Communication::RESPONSE_TIME_TAG>::Type*)_rhs); \
				} \
				default: \
					break; \
			} \
			return false; \
		} \
};


DEFINE_ROPERATOR(MEqOperator, ==)
DEFINE_ROPERATOR(MNeOperator, !=)
DEFINE_ROPERATOR(MLtOperator, <)
DEFINE_ROPERATOR(MGtOperator, >)
DEFINE_ROPERATOR(MLeOperator, <=)
DEFINE_ROPERATOR(MGeOperator, >=)



class MOperatorFactory {
	public:
		MFilterInterface* createOperator(const std::string& op) {
			if (op == "&&")
				return new MAndOperator;
			else if (op =="||")
				return new MOrOperator;
			else if (op == "!")
				return new MNotOperator;
			else if (op == "==")
				return new MEqOperator;
			else if (op == "!=")
				return new MNeOperator;
			else if (op == "<=")
				return new MLeOperator;
			else if (op == ">=")
				return new MGeOperator;
			else if (op == "<")
				return new MLtOperator;
			else if (op == ">")
				return new MGtOperator;
			else
				return NULL;
		}
};


class TagFactory {
	public:
		static void* CrateObjectFromTag(Communication::ConnectionInfoTag tag, const std::string& valStr) {
				switch ( tag ) {
					case Communication::PRIVATE_GROUP_TAG: {
						typedef Communication::ConnectionInfoT<Communication::PRIVATE_GROUP_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::HOSTNAME_TAG: {
						typedef Communication::ConnectionInfoT<Communication::HOSTNAME_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::CLIENTNAME_TAG:{
						typedef Communication::ConnectionInfoT<Communication::CLIENTNAME_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::IPS_TAG:{
						typedef Communication::ConnectionInfoT<Communication::IPS_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::PROGRAMNAME_TAG:{
						typedef Communication::ConnectionInfoT<Communication::PROGRAMNAME_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::PID_TAG: {
						typedef Communication::ConnectionInfoT<Communication::PID_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::CPU_USAGE_TAG: {
						typedef Communication::ConnectionInfoT<Communication::CPU_USAGE_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::TOTAL_MEMORY_TAG: {
						typedef Communication::ConnectionInfoT<Communication::TOTAL_MEMORY_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::CLIENT_MEMORY_USAGE_TAG: {
						typedef Communication::ConnectionInfoT<Communication::CLIENT_MEMORY_USAGE_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::MEMORY_USAGE_TAG: {
						typedef Communication::ConnectionInfoT<Communication::MEMORY_USAGE_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::SENT_MESSAGES_TAG: {
						typedef Communication::ConnectionInfoT<Communication::SENT_MESSAGES_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::RECEIVED_MESSAGES_TAG: {
						typedef Communication::ConnectionInfoT<Communication::RECEIVED_MESSAGES_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::MESSAGE_QUEUE_SIZE_TAG: {
						typedef Communication::ConnectionInfoT<Communication::MESSAGE_QUEUE_SIZE_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::SUMMED_MESSAGE_QUEUE_SIZE_TAG: {
						typedef Communication::ConnectionInfoT<Communication::SUMMED_MESSAGE_QUEUE_SIZE_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::AVERAGE_MESSAGE_QUEUE_SIZE_TAG: {
						typedef Communication::ConnectionInfoT<Communication::AVERAGE_MESSAGE_QUEUE_SIZE_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::SUMMED_MESSAGE_SIZE_TAG: {
						typedef Communication::ConnectionInfoT<Communication::SUMMED_MESSAGE_SIZE_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::AVERAGE_MESSAGE_SIZE_TAG: {
						typedef Communication::ConnectionInfoT<Communication::AVERAGE_MESSAGE_SIZE_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::OBJECT_COUNT_TAG: {
						typedef Communication::ConnectionInfoT<Communication::OBJECT_COUNT_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::UPTIME_TAG: {
						typedef Communication::ConnectionInfoT<Communication::UPTIME_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					case Communication::RESPONSE_TIME_TAG: {
						typedef Communication::ConnectionInfoT<Communication::RESPONSE_TIME_TAG>::Type Type;
						Type* object = new Type;
						if ( Core::fromString(*object, valStr) )
							return object;
					}
					default:
						break;
				}
				return NULL;
		}
};




typedef bs::tree_match<const char*>::tree_iterator iter_t;
MFilterInterface* evalParseTree(iter_t iter, MOperatorFactory& factory) {
	if ( iter->value.id() == MFilterParser::rexpID ) {
		SEISCOMP_DEBUG("= rexp ( %lu children ) =", (unsigned long)iter->children.size());

		std::string tagStr((iter->children.begin())->value.begin(), (iter->children.begin())->value.end());
		std::string opStr((iter->children.begin()+1)->value.begin(), (iter->children.begin()+1)->value.end());
		std::string valStr((iter->children.begin()+2)->value.begin(), (iter->children.begin()+2)->value.end());

		SEISCOMP_DEBUG("%s %s %s", tagStr.c_str(), opStr.c_str(), valStr.c_str());
		Communication::ConnectionInfoTag* tag = new Communication::ConnectionInfoTag;
		if ( !tag->fromString(tagStr) ) {
			delete tag;
			return NULL;
		}

		MFilterInterface* opPtr = factory.createOperator(opStr);
		if ( !opPtr ) {
			SEISCOMP_ERROR("Could not create operator %s", opStr.c_str());
			return NULL;
		}

		void* object = TagFactory::CrateObjectFromTag(*tag, valStr);
		if ( !object ) {
			SEISCOMP_ERROR("Could not allocate memory for %s = %s", tagStr.c_str(), valStr.c_str());
			delete tag;
			return NULL;
		}
		static_cast<MBinaryOperator<MFilterInterface>* >(opPtr)->setOperants(tag, object);
		return opPtr;

	}
	else if ( iter->value.id() == MFilterParser::notLexpID ) {
		SEISCOMP_DEBUG("= not_lexp ( %lu children ) =", (unsigned long)iter->children.size());
		std::string opStr(iter->value.begin(), iter->value.end());
		SEISCOMP_DEBUG("operator: %s", opStr.c_str());

		MFilterInterface* tmp = evalParseTree(iter->children.begin(), factory);
		if ( !tmp )
			return NULL;

		MFilterInterface* opPtr = factory.createOperator(opStr);
		if ( !opPtr ) {
			SEISCOMP_DEBUG("Could not create operator %s", opStr.c_str());
			return NULL;
		}
		static_cast<MUnaryOperator<MFilterInterface>* >(opPtr)->setOperant(tmp);
		return opPtr;
	}
	else if ( iter->value.id() == MFilterParser::groupID ) {
		SEISCOMP_DEBUG("= group ( %lu children ) =", (unsigned long)iter->children.size());
		return evalParseTree(iter->children.begin()+1, factory);
	}
	else if ( iter->value.id() == MFilterParser::expressionID ) {
		SEISCOMP_DEBUG("= expression ( %lu children ) =", (unsigned long)iter->children.size());
		if ( iter->children.size() != 2 ) {
			SEISCOMP_ERROR("Expression has more or less than 2 children (%lu)",
					(unsigned long)iter->children.size());
			return NULL;
		}
		std::string opStr(iter->value.begin(), iter->value.end());
		SEISCOMP_DEBUG("operator: %s", opStr.c_str());

		MFilterInterface* tmp = evalParseTree(iter->children.begin(), factory);;
		MFilterInterface* tmp1 = evalParseTree(iter->children.begin()+1, factory);;
		if (!tmp || !tmp1)
			return NULL;

		MFilterInterface* opPtr = factory.createOperator(opStr);
		if ( !opPtr ) {
			SEISCOMP_DEBUG("Could not create operator %s", opStr.c_str());
			return NULL;
		}
		static_cast<MBinaryOperator<MFilterInterface>* >(opPtr)->setOperants(tmp, tmp1);
		return opPtr;
	}
	else
		SEISCOMP_DEBUG("Expression not handled");

	SEISCOMP_DEBUG("Returning null");
	return NULL;
}


} // namespace Applications
} // namespace Seiscomp

#endif
