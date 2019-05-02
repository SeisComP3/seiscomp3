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
#include <seiscomp3/datamodel/eventparameters.h>
#include <seiscomp3/datamodel/origin.h>
#include <seiscomp3/datamodel/creationinfo.h>


namespace Seiscomp {
namespace DataModel {


DEFINE_SMARTPOINTER(ConfigSyncMessage);

class SC_SYSTEM_CORE_API ConfigSyncMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(ConfigSyncMessage);
	DECLARE_SERIALIZATION;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		ConfigSyncMessage();
		ConfigSyncMessage(bool finished);


	// ------------------------------------------------------------------
	//  Message interface
	// ------------------------------------------------------------------
	public:
		bool empty() const;


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		void setCreationInfo(const OPT(CreationInfo)& creationInfo);
		CreationInfo &creationInfo();
		const CreationInfo &creationInfo() const;


	// ------------------------------------------------------------------
	//  Public members
	// ------------------------------------------------------------------
	public:
		bool isFinished;

	private:
		OPT(CreationInfo) _creationInfo;
};


DEFINE_SMARTPOINTER(InventorySyncMessage);

class SC_SYSTEM_CORE_API InventorySyncMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(InventorySyncMessage);
	DECLARE_SERIALIZATION;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		InventorySyncMessage();
		InventorySyncMessage(bool finished);


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		void setCreationInfo(const OPT(CreationInfo)& creationInfo);
		CreationInfo &creationInfo();
		const CreationInfo &creationInfo() const;


	// ------------------------------------------------------------------
	//  Message interface
	// ------------------------------------------------------------------
	public:
		bool empty() const;


	// ------------------------------------------------------------------
	//  Public members
	// ------------------------------------------------------------------
	public:
		bool isFinished;

	private:
		OPT(CreationInfo) _creationInfo;
};


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
		ArtificialOriginMessage(DataModel::Origin *origin);

		
	// ------------------------------------------------------------------
	//  Message interface
	// ------------------------------------------------------------------
	public:
		DataModel::Origin *origin() const;
		void setOrigin(DataModel::Origin *origin);
		
		virtual bool empty() const;

	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		DataModel::OriginPtr _origin;
		
		DECLARE_SC_CLASSFACTORY_FRIEND(ArtificialOriginMessage);
};


DEFINE_SMARTPOINTER(ArtificialEventParametersMessage);

class SC_SYSTEM_CORE_API ArtificialEventParametersMessage : public Seiscomp::Core::Message {
	DECLARE_SC_CLASS(ArtificialEventParametersMessage);
	DECLARE_SERIALIZATION;

	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	protected:
		ArtificialEventParametersMessage();

	public:
		ArtificialEventParametersMessage(DataModel::EventParameters *eventParameters);


	// ------------------------------------------------------------------
	//  Message interface
	// ------------------------------------------------------------------
	public:
		DataModel::EventParameters *eventParameters() const;
		void setEventParameters(DataModel::EventParameters *eventParameters);

		virtual bool empty() const;

	// ------------------------------------------------------------------
	//  Implementation
	// ------------------------------------------------------------------
	private:
		DataModel::EventParametersPtr _eventParameters;

		DECLARE_SC_CLASSFACTORY_FRIEND(ArtificialEventParametersMessage);
};


}
}


#endif
