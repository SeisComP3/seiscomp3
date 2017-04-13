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
 * A SpreadException is thrown whenever a problem occurs in a spread method.
 * One example is calling a SpreadConnection object's {@link SpreadConnection#receive()} method
 * before calling {@link SpreadConnection#connect(InetAddress, int, String, boolean, boolean)}
 * on the object.
 * <p>
 * More exceptions will probably be provided in the future, to give more detailed error information.
 * However, these exceptions will all be extended from SpreadException, so any exception-handling
 * code written to handle SpreadException's will catch the new exceptions equally well.
 */
public class SpreadException extends Exception
{
	// Constructor.
	///////////////
	/**
	 * Creates a SpreadException with no error message.
	 */
	public SpreadException()
	{
		super();
	}
	
	// Constructor.
	///////////////
	/**
	 * Creates a SpreadException with an error message.
	 * 
	 * @param  errorMessage  a description of the error
	 */
	public SpreadException(String errorMessage)
	{
		super(errorMessage);
	}
}
