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
 * A MessageFactory object is used to create any number of messages based on a default message.
 * The default message is first specified in the constructor:
 * <p><blockquote><pre>
 * messageFactory = new MessageFactory(message);
 * </pre></blockquote><p>
 * The default message can later be changed using {@link MessageFactory#setDefault(SpreadMessage)}:
 * <p><blockquote><pre>
 * messageFactory.setDefault(message);
 * </pre></blockquote><p>
 * To get a message from the factory, use {@link MessageFactory#createMessage()}:
 * <p><blockquote><pre>
 * SpreadMessage message = messageFactory.createMessage();
 * </pre></blockquote><p>
 * Classes that extend MessageFactory can override {@link MessageFactory#createMessage()}
 * to provide behaviors other than simply cloning the default message.  One example is a factory that
 * sets a time-stamp in the message:
 * <p><blockquote><pre>
 * public class TimeStampMessageFactory extends MessageFactory
 * {
 *     public SpreadMessage createMessage()
 *     {
 *         SpreadMessage message = super.createMessage();
 *         message.setObject(new Long(System.currentTimeMillis()));
 *         return message;
 *     }
 * }
 * </pre></blockquote><p>
 */
public class MessageFactory
{
	// The default message.
	///////////////////////
	protected SpreadMessage defaultMessage;
	
	// Creates a new message factory with the default message.
	////////////////////////////////////////////////////////////////
	/**
	 * Constructs a new MessageFactory and sets the default message.
	 * 
	 * @param  message  the default message
	 */
	public MessageFactory(SpreadMessage message)
	{
		setDefault(message);
	}
	
	// Sets the default message to message.
	///////////////////////////////////////
	/**
	 * Sets a new default message for this factory.
	 * 
	 * @param  message  the new default message
	 */
	public void setDefault(SpreadMessage message)
	{
		// Check for null.
		//////////////////
		if(message == null)
		{
			defaultMessage = null;
		}
		else
		{		
			defaultMessage = (SpreadMessage)message.clone();
		}
	}
	
	// Returns a new Message provided by the object implementing the class.
	///////////////////////////////////////////////////////////////////////
	/**
	 * Returns a new message.  The message is a clone of the default message.
	 * If the default message has been specified as null, null is returned.
	 * 
	 * @return  a new message
	 */
	public SpreadMessage createMessage()
	{
		// Check for null.
		//////////////////
		if(defaultMessage == null)
		{
			return null;
		}
		
		return (SpreadMessage)defaultMessage.clone();
	}
}
