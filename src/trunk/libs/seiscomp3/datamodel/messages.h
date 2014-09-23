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



#ifndef __SEISCOMP_DATAMODEL_MESSAGES_H__
#define __SEISCOMP_DATAMODEL_MESSAGES_H__


#include <seiscomp3/core/genericmessage.h>
#include <seiscomp3/datamodel/origin.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ArtificialOriginMessage);

class SC_SYSTEM_CORE_API ArtificialOriginMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(ArtificialOriginMessage);
	DECLARE_SERIALIZATION;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		ArtificialOriginMessage();

	public:
		ArtificialOriginMessage(DataModel::Origin* origin);

		
	// ------------------------------------------------------------------
	//  Message interface
	// ------------------------------------------------------------------
	public:
		DataModel::Origin* origin() const;
		void setOrigin(DataModel::Origin* origin);
		
		virtual bool empty() const;
		
	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		DataModel::OriginPtr _origin;
		
		DECLARE_SC_CLASSFACTORY_FRIEND(ArtificialOriginMessage);
};


}
}


#endif
