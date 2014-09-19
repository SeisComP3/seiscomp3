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



#include "imexmessage.h"

namespace Seiscomp {
namespace Applications {

IMPLEMENT_SC_CLASS_DERIVED(IMEXMessage, Core::Message, "IMEXMessage");

int IMEXMessage::size() const
{
	return _notifierMessage.size();
}




bool IMEXMessage::empty() const
{
	return _notifierMessage.empty();
}




void IMEXMessage::clear()
{
	return _notifierMessage.clear();
}




DataModel::NotifierMessage& IMEXMessage::notifierMessage()
{
	return _notifierMessage;
}




void IMEXMessage::serialize(Archive& ar)
{
	ar & TAGGED_MEMBER(notifierMessage);
}


} // namespace Applications
} // namespace Seiscomp
