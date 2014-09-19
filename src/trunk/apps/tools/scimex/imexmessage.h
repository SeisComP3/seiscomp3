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



#ifndef __SEISCOMP_APPLICATIONS_IMEXMESSAGE_H__
#define __SEISCOMP_APPLICATIONS_IMEXMESSAGE_H__

#include <seiscomp3/core/baseobject.h>
#include <seiscomp3/core/message.h>
#include <seiscomp3/datamodel/notifier.h>


namespace Seiscomp {
namespace Applications {

DEFINE_SMARTPOINTER(IMEXMessage);

class IMEXMessage : public Core::Message {
	DECLARE_SC_CLASS(IMEXMessage);
	
	// ----------------------------------------------------------------------
	//  Xstruction
	// ----------------------------------------------------------------------
	public:
		//! Constructor
		IMEXMessage() {}


	// ----------------------------------------------------------------------
	//  Public Interface
	// ----------------------------------------------------------------------
	public:
		/**
		 * @return Returns the number of objects attached to a message
		 */
		virtual int size() const;
	
		/**
		 * Checks whether a message is empty or not.
		 * @retval true The message is empty
		 * @retval false The message is not empty
		 */
		virtual bool empty() const;
	
		/**
		 * Erases the content of the message.
		 */
		virtual void clear();
	
		DataModel::NotifierMessage& notifierMessage();
		
		virtual void serialize(Archive& ar);
		
		
	// ----------------------------------------------------------------------
	// Private data members 
	// ----------------------------------------------------------------------
	private:
		DataModel::NotifierMessage _notifierMessage;
};

}
}

#endif
