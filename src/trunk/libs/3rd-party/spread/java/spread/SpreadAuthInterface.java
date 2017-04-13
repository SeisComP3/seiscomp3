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
 * Objects of a class that implements the SpreadAuthInterface interface can
 * act as the client side of an authentication protocol to complete a Spread connection.
 * The connect method {@link SpreadConnection#connect(InetAddress, int, String, boolean, boolean)} 
 * will call the {@link SpreadAuthInterface#authenticate()} method during a connection.
 * If the module wants to deny the connection it should raise a SpreadException.
 * If it wants to allow the connection it should return normally.
 */
public interface SpreadAuthInterface
{
	// Authenticate a spread connection
	////////////////////////////////////////////
	/**
	 * This method will be called during the establishment of a Spread connection.
	 * It should run whatever protocol is necessary to authenticate the connection
	 * and return normally to allow the connection, or raise a SpreadException to deny
	 * the connection.
	 * 
	 */
    public void authenticate() throws SpreadException;
}
