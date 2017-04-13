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

import java.io.*;
import java.util.*;

/**
 * A SpreadMessage object represents either an incoming or outgoing spread message.
 * An outgoing message is one being sent with {@link SpreadConnection#multicast(SpreadMessage)}.
 * An incoming message is one received with {@link SpreadConnection#receive()}.
 * To send a message on a spread connection, first create a message object:
 * <p><blockquote><pre>
 * SpreadMessage message = new SpreadMessage();
 * </pre></blockquote><p>
 * Then set the data with either {@link SpreadMessage#setData(byte[])}, {@link SpreadMessage#setObject(Serializable)},
 * or {@link SpreadMessage#digest(Serializable)}:
 * <p><blockquote><pre>
 * message.setData(data);
 * </pre></blockquote><p>
 * Select which group(s) to send the message to with {@link SpreadMessage#addGroup(SpreadGroup)}:
 * <p><blockquote><pre>
 * message.setGroup(group);
 * </pre></blockquote><p>
 * When the message is read to be sent, send it with {@link SpreadConnection#multicast(SpreadMessage)}:
 * <p><blockquote><pre>
 * connection.multicast(message);
 * </pre></blockquote><p>
 */
public class SpreadMessage
{
	// Service-type constants.
	//////////////////////////
	protected static final int UNRELIABLE_MESS        = 0x00000001;
	protected static final int RELIABLE_MESS          = 0x00000002;
	protected static final int FIFO_MESS              = 0x00000004;
	protected static final int CAUSAL_MESS            = 0x00000008;
	protected static final int AGREED_MESS            = 0x00000010;
	protected static final int SAFE_MESS              = 0x00000020;
	protected static final int REGULAR_MESS           = 0x0000003f;
	protected static final int SELF_DISCARD           = 0x00000040;
	protected static final int REG_MEMB_MESS          = 0x00001000;
	protected static final int TRANSITION_MESS        = 0x00002000;
	protected static final int CAUSED_BY_JOIN         = 0x00000100;
	protected static final int CAUSED_BY_LEAVE        = 0x00000200;
	protected static final int CAUSED_BY_DISCONNECT   = 0x00000400;
	protected static final int CAUSED_BY_NETWORK      = 0x00000800;
	protected static final int MEMBERSHIP_MESS        = 0x00003f00;
	
        protected static final int REJECT_MESS		  = 0x00400000;

	// Service-types used only within the package.
	//////////////////////////////////////////////
	protected static final int JOIN_MESS            = 0x00010000;
	protected static final int LEAVE_MESS           = 0x00020000;
	protected static final int KILL_MESS            = 0x00040000;
	protected static final int GROUPS_MESS          = 0x00080000;
	

	// Message content constants.
	/////////////////////////////
	private static final int CONTENT_DATA           = 1;
	private static final int CONTENT_OBJECT         = 2;
	private static final int CONTENT_DIGEST         = 3;

	// Is this an outgoing message?
	///////////////////////////////
	private boolean outgoing;
	
	// The message content.
	///////////////////////
	protected int content;
	
	// The service type.
	////////////////////
	private int serviceType;
	
	// The groups this message is from/to.
	//////////////////////////////////////
	protected Vector groups;
	
	// The sender's group.
	//////////////////////
	private SpreadGroup sender;
	
	// The data.
	////////////
	private byte[] data;
	
	// The type.
	////////////
	private short type;
	
	// The endian mismatch.
	///////////////////////
	private boolean endianMismatch;
	
	// The membership info (for membership messages).
	/////////////////////////////////////////////////
	private MembershipInfo membershipInfo;
	
	// Byte stream used for digesting.
	//////////////////////////////////
	protected ByteArrayOutputStream digestBytes;
	
	// Object stream used for digesting.
	////////////////////////////////////
	protected ObjectOutputStream digestOutput;
	
	// Creates a new incoming message.
	//////////////////////////////////
	protected SpreadMessage(int serviceType, Vector groups, SpreadGroup sender, byte[] data, short type, boolean endianMismatch, MembershipInfo membershipInfo)
	{
		// Set member variables.
		////////////////////////
		outgoing = false;
		this.serviceType = serviceType;
		this.groups = groups;
		this.sender = sender;
		this.data = data;
		this.type = type;
		this.endianMismatch = endianMismatch;
		this.membershipInfo = membershipInfo;
	}
	
	// Creates a new outgoing message.
	//////////////////////////////////
	/**
	 * Initializes a new outgoing SpreadMessage object.  By default the message is reliable.
	 */
	public SpreadMessage()
	{
		// Set member variables to defaults.
		////////////////////////////////////
		outgoing = true;
		content = CONTENT_DATA;
		serviceType = RELIABLE_MESS;
		groups = new Vector();
		data = new byte[0];
	}
	
	// Returns true if this is an incoming message.
	///////////////////////////////////////////////
	/**
	 * Check if this is an incoming message.  This is true if it has been received with 
	 * {@link SpreadConnection#receive()}.
	 * 
	 * @return  true if this in an incoming message
	 */
	public boolean isIncoming()
	{
		return !outgoing;
	}
	
	// Returns true if this is an outgoing message.
	///////////////////////////////////////////////
	/**
	 * Check if this is an outgoing message.  This is true if this is a message being sent with
	 * {@link SpreadConnection#multicast(SpreadMessage)}.
	 * 
	 * @return  true if this is an outgoing message
	 */
	public boolean isOutgoing()
	{
		return outgoing;
	}
	
	// Checking the service type.
	/////////////////////////////
	/**
	 * Get the message's service type.  The service type is a bitfield representing the type of message.
	 * 
	 * @return  the service-type
	 */
	public int getServiceType()
	{
		return serviceType;
	}
	
	/**
	 * Checks if this is a regular message.  If true, the get*() functions can be
	 * used to obtain more information about the message.
	 * 
	 * @return  true if this is a regular message
	 * @see SpreadMessage#getGroups()
	 * @see SpreadMessage#getSender()
	 * @see SpreadMessage#getData()
	 * @see SpreadMessage#getObject()
	 * @see SpreadMessage#getDigest()
	 * @see SpreadMessage#getType()
	 * @see SpreadMessage#getEndianMismatch()
	 */
	public boolean isRegular()
	{
		return ( ((serviceType & REGULAR_MESS) != 0) && ( (serviceType & REJECT_MESS) == 0) );
	}
	
	protected static boolean isRegular(int serviceType)
	{
		return ( ((serviceType & REGULAR_MESS) != 0) && ( (serviceType & REJECT_MESS) == 0) );
	}

	/**
	 * Checks if this is a rejected message.  If true, the get*() methods can
	 * be used to get more information on which message or join/leave was rejected.
	 * 
	 * @return  true if this is a rejected message
	 * @see SpreadMessage#getGroups()
	 * @see SpreadMessage#getSender()
	 * @see SpreadMessage#getData()
	 * @see SpreadMessage#getObject()
	 * @see SpreadMessage#getDigest()
	 * @see SpreadMessage#getType()
	 * @see SpreadMessage#getEndianMismatch()
	 */
	public boolean isReject()
	{
		return ((serviceType & REJECT_MESS) != 0);
	}
	
	protected static boolean isReject(int serviceType)
	{
		return ((serviceType & REJECT_MESS) != 0);
	}
	
	/**
	 * Checks if this is a membership message.  If true, {@link SpreadMessage#getMembershipInfo()} can
	 * be used to get more information on the membership change.
	 * 
	 * @return  true if this is a membership message
	 * @see  SpreadMessage#getMembershipInfo()
	 */
	public boolean isMembership()
	{
		return ( ((serviceType & MEMBERSHIP_MESS) != 0) && ((serviceType & REJECT_MESS) == 0) );
	}
	
	protected static boolean isMembership(int serviceType)
	{
		return ( ((serviceType & MEMBERSHIP_MESS) != 0) && ((serviceType & REJECT_MESS) == 0) );
	}
	
	/**
	 * Checks if this is an unreliable message.
	 * 
	 * @return  true if this is an unreliable message
	 */
	public boolean isUnreliable()
	{
		return ((serviceType & UNRELIABLE_MESS) != 0);
	}
	
	/**
	 * Checks if this is a reliable message.
	 * 
	 * @return  true if this is a reliable message
	 */
	public boolean isReliable()
	{
		return ((serviceType & RELIABLE_MESS) != 0);
	}
	
	/**
	 * Checks if this is a fifo message.
	 * 
	 * @return  true if this is a fifo message
	 */
	public boolean isFifo()
	{
		return ((serviceType & FIFO_MESS) != 0);
	}
	
	/**
	 * Checks if this is a causal message.
	 * 
	 * @return  true if this is a causal message
	 */
	public boolean isCausal()
	{
		return ((serviceType & CAUSAL_MESS) != 0);
	}
	
	/**
	 * Checks if this is an agreed message.
	 * 
	 * @return  true if this is an agreed message
	 */
	public boolean isAgreed()
	{
		return ((serviceType & AGREED_MESS) != 0);
	}
	
	/**
	 * Checks if this is a safe message.
	 * 
	 * @return  true if this is a safe message
	 */
	public boolean isSafe()
	{
		return ((serviceType & SAFE_MESS) != 0);
	}
	
	/**
	 * Checks if this is a self-discard message.
	 * 
	 * @return  true if this is a self-discard message
	 */
	public boolean isSelfDiscard()
	{
		return ((serviceType & SELF_DISCARD) != 0);
	}
	
	// Setting the service type.
	////////////////////////////
	/**
	 * Sets the service type.  The service type is a bitfield representing the type of message.
	 * 
	 * @param  serviceType  the new service type
	 */
	public void setServiceType(int serviceType)
	{
		this.serviceType = serviceType;
	}
	
	/**
	 * Sets the message to be unreliable.
	 */
	public void setUnreliable()
	{
		// Clear any conflicting flags.
		///////////////////////////////
		serviceType &= ~REGULAR_MESS;
		
		// Set the flag.
		////////////////
		serviceType |= UNRELIABLE_MESS;
	}
	
	/**
	 * Sets the message to be reliable.  This is the default type for a new outgoing message.
	 */
	public void setReliable()
	{
		// Clear any conflicting flags.
		///////////////////////////////
		serviceType &= ~REGULAR_MESS;
		
		// Set the flag.
		////////////////
		serviceType |= RELIABLE_MESS;
	}
	
	/**
	 * Sets the message to be fifo.
	 */
	public void setFifo()
	{
		// Clear any conflicting flags.
		///////////////////////////////
		serviceType &= ~REGULAR_MESS;
		
		// Set the flag.
		////////////////
		serviceType |= FIFO_MESS;
	}
	
	/**
	 * Sets the message to be causal.
	 */
	public void setCausal()
	{
		// Clear any conflicting flags.
		///////////////////////////////
		serviceType &= ~REGULAR_MESS;
		
		// Set the flag.
		////////////////
		serviceType |= CAUSAL_MESS;
	}
	
	/**
	 * Sets the message to be agreed.
	 */
	public void setAgreed()
	{
		// Clear any conflicting flags.
		///////////////////////////////
		serviceType &= ~REGULAR_MESS;
		
		// Set the flag.
		////////////////
		serviceType |= AGREED_MESS;
	}
	
	/**
	 * Sets the message to be safe.
	 */
	public void setSafe()
	{
		// Clear any conflicting flags.
		///////////////////////////////
		serviceType &= ~REGULAR_MESS;
		
		// Set the flag.
		////////////////
		serviceType |= SAFE_MESS;
	}
	
	/**
	 * If <code>selfDiscard</code> is true, sets the self discard flag for the message, otherwise
	 * clears the flag.  If the self discard flag is set, the message will not be received at the 
	 * connection it is multicast on.
	 * 
	 * @param  selfDiscard  if true, set the self discard flag, if false, clear the self discard flag
	 */
	public void setSelfDiscard(boolean selfDiscard)
	{
		if(selfDiscard)
		{
			// Set the flag.
			////////////////
			serviceType |= SELF_DISCARD;
		}
		else
		{
			// Clear the flag.
			//////////////////
			serviceType &= ~SELF_DISCARD;
		}
	}
	
	// Get the groups this message was sent to.
	///////////////////////////////////////////
	/**
	 * Gets an array containing the SpreadGroup's to which this message was sent.
	 * 
	 * @return  the groups to which this message was sent
	 */
	public SpreadGroup[] getGroups()
	{
		// Get an array of groups.
		//////////////////////////
		SpreadGroup[] groupArray = new SpreadGroup[groups.size()];
		for(int i = 0 ; i < groupArray.length ; i++)
		{
			// Set the array element.
			/////////////////////////
			groupArray[i] = (SpreadGroup)groups.elementAt(i);
		}
		
		// Return the array.
		////////////////////
		return groupArray;
	}
	
	// Get the private group of the message's sender.
	/////////////////////////////////////////////////
	/**
	 * Gets the message sender's private group.  This can be used to uniquely identify
	 * the sender on the connection this message was received from or to send
	 * a reply to the sender.
	 */
	public SpreadGroup getSender()
	{
		return sender;
	}
	
	// Get the message data as a byte array (for CONTENT_DATA).
	///////////////////////////////////////////////////////////
	/**
	 * Gets the message data as an array of bytes.  This can be used no matter how
	 * the message was sent, but is usually used when the message was sent using
	 * {@link SpreadMessage#setData(byte[])} or from an application using the C library.
	 * 
	 * @return  the message data
	 * @see  SpreadMessage#setData(byte[])
	 */
	public byte[] getData()
	{
		// Is this a digest?
		////////////////////
		if(content == CONTENT_DIGEST)
		{
			// Get the data.
			////////////////
			data = digestBytes.toByteArray();
		}
		
		// Return the data.
		///////////////////
		return data;
	}
	
	// Get the message as a java object (for CONTENT_OBJECT).
	/////////////////////////////////////////////////////////
	/**
	 * Gets the message data as a java object.  The message data should have been set
	 * using {@link SpreadMessage#setObject(Serializable)}.  Regardless of the type of
	 * object passed to {@link SpreadMessage#setObject(Serializable)}, this method returns
	 * an object of type Object, so it must be cast to the correct type.
	 * 
	 * @return  the message data as an object
	 * @throws  SpreadException  if there is an error reading the object
	 * @see SpreadMessage#setObject(Serializable)
	 */
	public Object getObject() throws SpreadException
	{
		// The serialized object bytes.
		///////////////////////////////
		ByteArrayInputStream objectBytes = new ByteArrayInputStream(data);
			
		// Setup the object input stream.
		/////////////////////////////////
		ObjectInputStream objectInput;
		try
		{
			objectInput = new ObjectInputStream(objectBytes);
		}
		catch(IOException e)
		{
			throw new SpreadException("ObjectInputStream(): " + e);
		}
			
		// De-serialize the object.
		///////////////////////////
		Object object;
		try
		{
			object = objectInput.readObject();
		}
		catch(ClassNotFoundException e)
		{
			throw new SpreadException("readObject(): " + e);
		}
		catch(IOException e)
		{
			throw new SpreadException("readObject(): " + e);
		}
			
		// Close the streams.
		/////////////////////
		try
		{
			objectInput.close();
			objectBytes.close();
		}
		catch(IOException e)
		{
			throw new SpreadException("close/close(): " + e);
		}
			
		// Return the object.
		/////////////////////
		return object;
	}

	// Get the message as a vector (for CONTENT_DIGEST).
	////////////////////////////////////////////////////
	/**
	 * Gets the message data as a digest.  The message data should have been set using
	 * {@link SpreadMessage#digest(Serializable)}.  This method returns a Vector containing
	 * all of the objects passed to {@link SpreadMessage#digest(Serializable)}, in the order
	 * they were passed.
	 * 
	 * @return  the message data as a list of objects
	 * @throws  SpreadException  if there is an error reading the objects
	 * @see  SpreadMessage#digest(Serializable)
	 */
	public Vector getDigest() throws SpreadException
	{
		// Make a vector to hold the objects.
		/////////////////////////////////////
		Vector objects = new Vector();
			
		// Setup the byte array stream.
		///////////////////////////////
		ByteArrayInputStream objectBytes = new ByteArrayInputStream(data);
			
		// Setup the object input stream.
		/////////////////////////////////
		ObjectInputStream objectInput;
		try
		{
			objectInput = new ObjectInputStream(objectBytes);
		}
		catch(IOException e)
		{
			throw new SpreadException("ObjectInputStream(): " + e);
		}
			
		// While there are objects to read.
		///////////////////////////////////
		try
		{
			// ObjectInputStream.available() DOES NOT seem to work.
			// So, it keeps reading until EOF is detected.
			////////////////////////////////////////////////////////
			//while(objectInput.available() > 0)
			while(true)
			{
				// Read the next object, and add it to the vector.
				//////////////////////////////////////////////////
				objects.addElement(objectInput.readObject());
			}
		}
		catch(EOFException e)
		{
			// ObjectInputStream.available() DOES NOT seem to work.
			// So, just ignore this.
			///////////////////////////////////////////////////////
		}
		catch(ClassNotFoundException e)
		{
			throw new SpreadException("readObject(): " + e);
		}
		catch(IOException e)
		{
			throw new SpreadException("readObject(): " + e);
		}
			
		// Close the streams.
		/////////////////////
		try
		{
			objectInput.close();
			objectBytes.close();
		}
		catch(IOException e)
		{
			throw new SpreadException("close/close(): " + e);
		}
			
		// Return the digest.
		/////////////////////
		return objects;
	}
	
	// Get the message type.
	////////////////////////
	/**
	 * Gets the message type.  The message type is set with {@link SpreadMessage#setType(short)}.
	 * 
	 * @return  the message type
	 * @see  SpreadMessage#setType(short)
	 */
	public short getType()
	{
		return type;
	}
	
	// Returns true if there was an endian mismatch.
	////////////////////////////////////////////////
	/**
	 * Checks for an endian mismatch.  If there is an endian mismatch between the machine that sent
	 * the message and the local machine, this is true.  This is a signal to the application so that
	 * it can handle endian flips in its message data.  Aside from the message data, spread handles
	 * all other endian mismatches itself (for example, the message type).
	 * 
	 * @return  true if there is an endian mismatch
	 */
	public boolean getEndianMismatch()
	{
		return endianMismatch;
	}
	
	// Adds a group to the message's destination group list.
	////////////////////////////////////////////////////////
	/**
	 * Adds this group to the list this message will be sent to.  When the message is multicast, 
	 * all members of this group will receive it.
	 * 
	 * @param  group  a group to send this message to
	 */
	public void addGroup(SpreadGroup group)
	{
		groups.addElement(group);
	}
	
	// Adds an array of groups to the message's destination group list.
	///////////////////////////////////////////////////////////////////
	/**
	 * Adds these groups to the list this message will be sent to.  When the message is multicast, 
	 * all members of these groups will receive it.
	 * 
	 * @param  groups  a list of groups to send this message to
	 */
	public void addGroups(SpreadGroup groups[])
	{
		int len = groups.length;
		for(int i = 0 ; i < len ; i++)
			this.groups.addElement(groups[i]);
	}
	
	// Adds a group to the message's destination group list.
	////////////////////////////////////////////////////////
	/**
	 * Adds this group to the list this message will be sent to.  When the message is multicast, 
	 * all members of this group will receive it.
	 * 
	 * @param  group  a group to send this message to
	 */
	public void addGroup(String group)
	{
		// Create a new group object for this name.
		///////////////////////////////////////////
		SpreadGroup spreadGroup = new SpreadGroup(null, group);
		
		// Add the group.
		/////////////////
		addGroup(spreadGroup);
	}
	
	// Adds an array of groups to the message's destination group list.
	///////////////////////////////////////////////////////////////////
	/**
	 * Adds these groups to the list this message will be sent to.  When the message is multicast, 
	 * all members of these groups will receive it.
	 * 
	 * @param  groups  a list of groups to send this message to
	 */
	public void addGroups(String groups[])
	{
		int len = groups.length;
		for(int i = 0 ; i < len ; i++)
		{
			// Create a new group object for this name.
			///////////////////////////////////////////
			SpreadGroup spreadGroup = new SpreadGroup(null, groups[i]);
		
			// Add the group.
			/////////////////
			addGroup(spreadGroup);
		}
	}
	
	// Sets the message to send the data.
	/////////////////////////////////////
	/**
	 * Sets the message's data to this array of bytes.  This cancels any previous calls to
	 * {@link SpreadMessage#setData(byte[])}, {@link SpreadMessage#setObject(Serializable)},
	 * {@link SpreadMessage#digest(Serializable)}.
	 * 
	 * @param  data  the new message data
	 * @see  SpreadMessage#getData()
	 */
	public void setData(byte[] data)
	{
		// If there's a digest, clear it.
		/////////////////////////////////
		digestBytes = null;
		digestOutput = null;
		
		// Set the content type.
		////////////////////////
		content = CONTENT_DATA;
		
		// Copy the data.
		/////////////////
		this.data = (byte[])data.clone();
	}
	
	// Sets the message to send the object.
	///////////////////////////////////////
	/**
	 * Sets the message's data to this object, in serialized form.  The object must support
	 * the Serializable interface to use this method.  This cancels any previous calls to
	 * {@link SpreadMessage#setData(byte[])}, {@link SpreadMessage#setObject(Serializable)},
	 * {@link SpreadMessage#digest(Serializable)}.  This should not be used if an application using
	 * the C library needs to read this message.
	 * 
	 * @param  object  the object to set the data to
	 * @see  SpreadMessage#getObject()
	 */
	public void setObject(Serializable object) throws SpreadException
	{
		// If there's a digest, clear it.
		/////////////////////////////////
		digestBytes = null;
		digestOutput = null;
		
		// Set the content type.
		////////////////////////
		content = CONTENT_OBJECT;
		
		// Make a byte stream to get the serialized data.
		/////////////////////////////////////////////////
		ByteArrayOutputStream objectBytes = new ByteArrayOutputStream();
		
		// Make an output stream for the object.
		////////////////////////////////////////
		ObjectOutputStream objectOutput;
		try
		{
			objectOutput = new ObjectOutputStream(objectBytes);
		}
		catch(IOException e)
		{
			throw new SpreadException("ObjectOutputStream(): " + e);
		}
		
		// Serialize the object.
		////////////////////////
		try
		{
			objectOutput.writeObject(object);
			objectOutput.flush();
		}
		catch(IOException e)
		{
			throw new SpreadException("writeObject/flush(): " + e);
		}
		
		// Store the bytes.
		///////////////////
		data = objectBytes.toByteArray();
		
		// Close the streams.
		/////////////////////
		try
		{
			objectOutput.close();
			objectBytes.close();
		}
		catch(IOException e)
		{
			throw new SpreadException("close/close(): " + e);
		}
	}
	
	// Sets the message to send a digest and adds this object to the digest.
	////////////////////////////////////////////////////////////////////////
	/**
	 * Adds this message to the digest.  The object must support
	 * the Serializable interface to use this method.  This cancels any previous calls to
	 * {@link SpreadMessage#setData(byte[])} or {@link SpreadMessage#setObject(Serializable)}.
	 * This should not be used if an application using the C library needs to read this message.
	 * When the message is sent, all of the objects that have been passed to this method get
	 * sent as the message data.
	 * 
	 * @param  object  the object to add to the digets
	 * @see  SpreadMessage#getDigest()
	 */
	public void digest(Serializable object) throws SpreadException
	{
		// Check the content type.
		//////////////////////////
		if(content != CONTENT_DIGEST)
		{
			// Set the content type.
			////////////////////////
			content = CONTENT_DIGEST;
			
			// Make a byte stream to get the serialized data.
			/////////////////////////////////////////////////
			digestBytes = new ByteArrayOutputStream();
			
			// Make an output stream for the objects.
			/////////////////////////////////////////
			try
			{
				digestOutput = new ObjectOutputStream(digestBytes);
			}
			catch(IOException e)
			{
				throw new SpreadException("ObjectOutputStream(): " + e);
			}
		}
		
		// Serialize the object.
		////////////////////////
		try
		{
			digestOutput.writeObject(object);
			digestOutput.flush();
		}
		catch(IOException e)
		{
			throw new SpreadException("writeObject/flush(): " + e);
		}
	}
	
	// Sets the message's type.
	///////////////////////////
	/**
	 * Set's the message type.  This is a 16-bit integer that can be used by the application
	 * to identify the message.
	 * 
	 * @param  type  the message type
	 * @see  SpreadMessage#getType()
	 */
	public void setType(short type)
	{
		this.type = type;
	}
	
	// Gets the membership change info, if this is a membership message.
	////////////////////////////////////////////////////////////////////
	/**
	 * Get the membership info for this message.  This should only be called if this is a
	 * membership message ({@link SpreadMessage#isMembership()} is true).
	 * 
	 * @return  the membership info for this message
	 */
	public MembershipInfo getMembershipInfo()
	{
		return membershipInfo;
	}
	
	// Creates a copy of this message.
	//////////////////////////////////
	/**
	 * Creates a copy of this message.
	 * 
	 * @return  a copy of this message
	 */
	public Object clone()
	{
		SpreadMessage message = new SpreadMessage();
		message.setServiceType(serviceType);
		message.groups = (Vector)groups.clone();
		message.setType(type);
		message.setData(data);
		message.content = content;
		try
		{
			if(content == CONTENT_DIGEST)
			{
				message.digestBytes = new ByteArrayOutputStream();
				message.digestBytes.write(digestBytes.toByteArray());
				message.digestOutput = new ObjectOutputStream(message.digestBytes);
			}
		}
		catch(IOException e)
		{
			// Will this ever be thrown?
			////////////////////////////
		}
		
		return message;
	}
}
