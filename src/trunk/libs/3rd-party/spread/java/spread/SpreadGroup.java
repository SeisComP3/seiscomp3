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

import java.io.*;

/**
 * A SpreadGroup object represents a group on a spread daemon.  The group is either a group that
 * has been joined or a private group.
 * To join a group, first create a SpreadGroup object, then join it with
 * {@link SpreadGroup#join(SpreadConnection, String)}:
 * <p><blockquote><pre>
 * SpreadGroup group = new SpreadGroup();
 * group.join(connection, "users");
 * </pre></blockquote><p>
 * To leave the group, use {@link SpreadGroup#leave()}:
 * <p><blockquote><pre>
 * group.leave();
 * </pre></blockquote><p>
 */
public class SpreadGroup
{	
	// The group's name.
	////////////////////
	private String name;
	
	// The connection this group exists on.
	///////////////////////////////////////
	private SpreadConnection connection;
	
	// Package constructor.
	///////////////////////
	protected SpreadGroup(SpreadConnection connection, String name)
	{
		// Store member variables.
		//////////////////////////
		this.connection = connection;
		this.name = name;
	}
	
	// Public constructor.
	//////////////////////
	/**
	 * Initializes a new SpreadGroup object.  To join a group with this object,
	 * use {@link SpreadGroup#join(SpreadConnection, String)}.
	 * 
	 * @see  SpreadGroup#join(SpreadConnection, String)
	 */
	public SpreadGroup()
	{
		// There is no connection yet.
		//////////////////////////////
		connection = null;
		
		// There is no name.
		////////////////////
		name = null;
	}
	
	// Joins the group "name" on the connection.
	////////////////////////////////////////////
	/**
	 * Joins the group <code>name</code> on the connection <code>connection</code>.
	 * Between when the group has been joined with this method and when it is left with
	 * {@link SpreadGroup#leave()}, all messages sent to the group will be received by
	 * the connection.
	 * 
	 * @param  connection  the connection to join the group with
	 * @param  name  the name of the group to join
	 * @throws  SpreadException  if attempting to join again with the same group object, if an illegal character is in the group name, or if there is an error trying to join the group
	 * @see  SpreadGroup#leave()
	 */
	public void join(SpreadConnection connection, String name) throws SpreadException
	{
		// Check if this group has already been joined.
		///////////////////////////////////////////////		
		if(this.connection != null)
		{
			throw new SpreadException("Already joined.");
		}

		// Set member variables.
		////////////////////////
		this.connection = connection;
		this.name = name;
		
		// Error check the name.
		////////////////////////
		byte bytes[];
		try
		{
			bytes = name.getBytes("ISO8859_1");
		}
		catch(UnsupportedEncodingException e)
		{
			throw new SpreadException("ISO8859_1 encoding not supported.");
		}
		for(int i = 0 ; i < bytes.length ; i++)
		{
			// Make sure the byte (character) is within the valid range.
			////////////////////////////////////////////////////////////
			if((bytes[i] < 36) || (bytes[i] > 126))
			{
				throw new SpreadException("Illegal character in group name.");
			}
		}
		
		// Get a new message.
		/////////////////////
		SpreadMessage joinMessage = new SpreadMessage();
		
		// Set the group we're sending to.
		//////////////////////////////////
		joinMessage.addGroup(name);
		
		// Set the service type.
		////////////////////////
		joinMessage.setServiceType(SpreadMessage.JOIN_MESS);
		
		// Send the message.
		////////////////////
		connection.multicast(joinMessage);
	}
	
	// Leaves the group.
	////////////////////
	/**
	 * Leaves the group.  No more messages will be received from this group after
	 * this method is called.
	 * 
	 * @throws  SpreadException  if the group hasn't been joined, or if there is an error leaving the group
	 * @see  SpreadGroup#join(SpreadConnection, String)
	 */
	public void leave() throws SpreadException
	{
		// Check if we can leave.
		/////////////////////////
		if(connection == null)
		{
			throw new SpreadException("No group to leave.");
		}
		
		// Get a new message.
		/////////////////////
		SpreadMessage leaveMessage = new SpreadMessage();
		
		// Set the group we're sending to.
		//////////////////////////////////
		leaveMessage.addGroup(name);
		
		// Set the service type.
		////////////////////////
		leaveMessage.setServiceType(SpreadMessage.LEAVE_MESS);
		
		// Send the message.
		////////////////////
		connection.multicast(leaveMessage);
		
		// No longer connected.
		///////////////////////
		connection = null;
	}
	
	// Gets the group's name as a String.
	/////////////////////////////////////
	/**
	 * Returns the name of the group as a string.
	 * 
	 * @return  the name of the group
	 */
	public String toString()
	{
		// Return the name.
		///////////////////
		return name;
	}

	// Returns true if object represents the same group as this object.
	///////////////////////////////////////////////////////////////////
	/**
	 * Checks if two groups are the same.  Two groups are the same if they have
	 * the same name.
	 * 
	 * @param  object  the object to compare against
	 * @return  true if object is a SpreadGroup and it has the same name
	 */
	public boolean equals(Object object)
	{
		// Check if it's the correct class.
		///////////////////////////////////
		if(object.getClass() != this.getClass())
		{
			return false;
		}
		
		// Check if the names are the same.
		///////////////////////////////////
		SpreadGroup other = (SpreadGroup)object;
		return (other.toString().equals(this.toString()));
	}

	// Returns the hash code of the group, which is defined as the
	// hash code of its name.
	///////////////////////////////////////////////////////////////////
	/**
	 * Returns the hash code of the group, which is defined as the
	 * hash code of its name.
	 *
	 * @return int the hash code
	 */
	public int hashCode()
	{
		return toString().hashCode();
	}
}
