/***************************************************************************
 * Copyright (C) 2011 by gempa GmbH
 *
 * Author: Jan Becker
 * Email: jabe@gempa.de
 ***************************************************************************/


#ifndef __GEMPA_INVMGR_TASK_H__
#define __GEMPA_INVMGR_TASK_H__


//! Simple Task class that allows interruption from
//! outside.
class Task {
	// ------------------------------------------------------------------
	//  Xstruction
	// ------------------------------------------------------------------
	public:
		Task() : _interrupted(false) {}
		virtual ~Task() {}


	// ------------------------------------------------------------------
	//  Public interface
	// ------------------------------------------------------------------
	public:
		virtual void interrupt() { _interrupted = true; }


	// ------------------------------------------------------------------
	//  Protected members
	// ------------------------------------------------------------------
	protected:
		bool _interrupted;
};


#endif
