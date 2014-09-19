/*
 * The Spread Toolkit.
 *     
 * The contents of this file are subject to the Spread Open-Source
 * License, Version 1.0 (the ``License''); you may not use
 * this file except in compliance with the License.  You may obtain a
 * copy of the License at:
 *
 * http://www.spread.org/license/
 *
 * or in the file ``license.txt'' found in this distribution.
 *
 * Software distributed under the License is distributed on an AS IS basis, 
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License 
 * for the specific language governing rights and limitations under the 
 * License.
 *
 * The Creators of Spread are:
 *  Yair Amir, Michal Miskin-Amir, Jonathan Stanton, John Schultz.
 *
 *  Copyright (C) 1993-2013 Spread Concepts LLC <info@spreadconcepts.com>
 *
 *  All Rights Reserved.
 *
 * Major Contributor(s):
 * ---------------
 *    Ryan Caudy           rcaudy@gmail.com - contributions to process groups.
 *    Claudiu Danilov      claudiu@acm.org - scalable wide area support.
 *    Cristina Nita-Rotaru crisn@cs.purdue.edu - group communication security.
 *    Theo Schlossnagle    jesus@omniti.com - Perl, autoconf, old skiplist.
 *    Dan Schoenblum       dansch@cnds.jhu.edu - Java interface.
 *
 */


package spread;

/**
 * Objects of a class that implements the BasicMessageListener interface can
 * add themselves to a SpreadConnection with {@link SpreadConnection#add(BasicMessageListener)}.
 * The object is an active listener until it is removed by a call to 
 * {@link SpreadConnection#remove(BasicMessageListener)}.  While the listener is active, it will be alerted
 * to all messages received on the connection.  When a message is received, 
 * {@link BasicMessageListener#messageReceived(SpreadMessage)} will be called.
 */
public interface BasicMessageListener
{
	// A new message has been received.
	///////////////////////////////////
	/**
	 * If the object has been added to a connection with {@link SpreadConnection#add(BasicMessageListener)},
	 * this gets called whenever a message is received.  The call happens in a seperate thread.
	 * 
	 * @param  message  the message that has been received
	 */
	public void messageReceived(SpreadMessage message);
}
