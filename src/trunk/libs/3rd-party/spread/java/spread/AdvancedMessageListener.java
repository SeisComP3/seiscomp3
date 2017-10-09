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
 *  Copyright (C) 1993-2014 Spread Concepts LLC <info@spreadconcepts.com>
 *
 *  All Rights Reserved.
 *
 * Major Contributor(s):
 * ---------------
 *    Amy Babay            babay@cs.jhu.edu - accelerated ring protocol.
 *    Ryan Caudy           rcaudy@gmail.com - contributions to process groups.
 *    Claudiu Danilov      claudiu@acm.org - scalable wide area support.
 *    Cristina Nita-Rotaru crisn@cs.purdue.edu - group communication security.
 *    Theo Schlossnagle    jesus@omniti.com - Perl, autoconf, old skiplist.
 *    Dan Schoenblum       dansch@cnds.jhu.edu - Java interface.
 *
 */


package spread;

/**
 * Objects of a class that implements the AdvancedMessageListener interface can
 * add themselves to a SpreadConnection with {@link SpreadConnection#add(AdvancedMessageListener)}.
 * The object is an active listener until it is removed by a call to 
 * {@link SpreadConnection#remove(AdvancedMessageListener)}.  While the listener is active, it will be alerted
 * to all messages received on the connection.  When a regular message is received, 
 * {@link AdvancedMessageListener#regularMessageReceived(SpreadMessage)} will be called.
 * When a membership message is received, {@link AdvancedMessageListener#membershipMessageReceived(SpreadMessage)}
 * will be called.
 */
public interface AdvancedMessageListener
{
	// A new regular message has been receieved.
	////////////////////////////////////////////
	/**
	 * If the object has been added to a connection with {@link SpreadConnection#add(AdvancedMessageListener)},
	 * this gets called whenever a regular message is received.  The call happens in a seperate thread.
	 * 
	 * @param  message  the message that has been received
	 */
	public void regularMessageReceived(SpreadMessage message);
	
	// A new membership message has been received.
	//////////////////////////////////////////////
	/**
	 * If the object has been added to a connection with {@link SpreadConnection#add(AdvancedMessageListener)},
	 * this gets called whenever a membership message is received.  The call happens in a seperate thread.
	 * 
	 * @param  message  the message that has been received
	 */
	public void membershipMessageReceived(SpreadMessage message);
}
