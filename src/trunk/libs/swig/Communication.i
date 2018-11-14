/***************************************************************************
 *   Copyright (C) by GFZ Potsdam, gempa GmbH                              *
 *                                                                         *
 *   You can redistribute and/or modify this program under the             *
 *   terms of the SeisComP Public License.                                 *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   SeisComP Public License for more details.                             *
 ***************************************************************************/

%module(package="seiscomp3") Communication
%{
#include "seiscomp3/core/interruptible.h"
#include "seiscomp3/core/record.h"
#include "seiscomp3/core/genericrecord.h"
#include "seiscomp3/core/greensfunction.h"
#include "seiscomp3/core/typedarray.h"
#include "seiscomp3/core/datamessage.h"
#include "seiscomp3/communication/servicemessage.h"
#include "seiscomp3/communication/connection.h"
#include "seiscomp3/communication/networkinterface.h"
%}


typedef long time_t;

%newobject Seiscomp::Communication::Connection::Create;
%ignore Seiscomp::Communication::SystemConnection::messageStat;
%ignore Seiscomp::Communication::NetworkMessage::data();

%include "std_string.i"
%include "exception.i"
%import "Core.i"
%include "seiscomp3/client.h"
%include "seiscomp3/communication/protocol.h"
%include "seiscomp3/communication/servicemessage.h"
%include "seiscomp3/communication/systemmessages.h"
%include "seiscomp3/communication/networkinterface.h"
%include "seiscomp3/communication/systemconnection.h"
%include "seiscomp3/communication/connection.h"
