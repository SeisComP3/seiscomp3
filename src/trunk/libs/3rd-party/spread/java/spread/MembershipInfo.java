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

import java.util.*;

/**
 * A MembershipInfo object represents information stored in a membership message.
 * To check if a message is a membership message, use {@link SpreadMessage#isMembership()}:
 * <p><blockquote><pre>
 * if(message.isMembership() == true)
 *     System.out.println("Membership message.");
 * </pre></blockquote><p>
 * To get the MembershipInfo object from a membership message, use {@link SpreadMessage#getMembershipInfo()}:
 * <p><blockquote><pre>
 * MembershipInfo membershipInfo = message.getMembershipInfo();
 * </pre></blockquote><p>
 * A MembershipInfo object can be used to get information on what caused the membership change
 * (the is*() methods) and extra information about the change (the get*() methods).
 * Which get*() methods are valid for a particular membership message depends on what caused the message.
 */
public class MembershipInfo
{
	// The message's serviceType.
	/////////////////////////////
	private int serviceType;
	
	// The group.
	/////////////
	private SpreadGroup group;
	
	// The group's id.
	//////////////////
	private GroupID groupID;
	
	// The group's members.
	///////////////////////
	private Vector members;
	
	// The private groups of members that joined/left/disconected/stayed.
	// For caused-by join/leave/disconnect, this has 1 element, with 1 member.
	// For caused-by network, this has 1 element for every virtual synchrony set,
  //  and each set has a variable number of members.
	/////////////////////////////////////////////////////////////////////////////
	private VirtualSynchronySet virtualSynchronySets[];
  
	// Null for caused-by join/leave/disconnect,
	// For caused-by network, points to vs-set for the SpreadConnection
	// that this was sent on.
	private VirtualSynchronySet myVirtualSynchronySet;

	// Constructor.
	///////////////
	protected MembershipInfo(SpreadConnection connection, int serviceType,
													 Vector groups, SpreadGroup sender,
													 byte data[], boolean daemonEndianMismatch)
	{
		// Set local variables.
		///////////////////////
		this.serviceType = serviceType;
		this.group       = sender;
		this.members     = groups;

		// Is this a regular membership message.
		////////////////////////////////////////
		if(isRegularMembership())
		{
			// Extract the groupID.
			///////////////////////
			int dataIndex = 0;
			int id[] = new int[3];
			id[0] = SpreadConnection.toInt(data, dataIndex);
			dataIndex += 4;
			id[1] = SpreadConnection.toInt(data, dataIndex);
			dataIndex += 4;
			id[2] = SpreadConnection.toInt(data, dataIndex);
			dataIndex += 4;
			// Endian-flip?
			///////////////
			if(daemonEndianMismatch)
			{
				id[0] = SpreadConnection.flip(id[0]);
				id[1] = SpreadConnection.flip(id[1]);
				id[2] = SpreadConnection.flip(id[2]);
			}

			// Create the group ID.
			///////////////////////
			this.groupID = new GroupID(id[0], id[1], id[2]);

			// Get the number of groups.
			////////////////////////////
			int numSets = SpreadConnection.toInt(data, dataIndex);
			if(daemonEndianMismatch) {
					numSets = SpreadConnection.flip(numSets);
			}
			dataIndex += 4;

			// Maybe this should throw a SpreadException if the number of sets
			// isn't positive, or if the offset isn't found, or if a set
			// doesn't have a positive number of members.  Older versions
			// did nothing of the kind, and trusted Spread, however.

			int byteOffsetToLocalSet = SpreadConnection.toInt(data, dataIndex);
			if(daemonEndianMismatch) {
					byteOffsetToLocalSet = SpreadConnection.flip(byteOffsetToLocalSet);
			}
			dataIndex += 4;

			int byteIndexOfFirstSet = dataIndex;
			virtualSynchronySets    = new VirtualSynchronySet[numSets];

			// Get the virtual synchrony sets
			/////////////////////////////////
			for( int i = 0; i < numSets; ++i ) {
					int numMembers = SpreadConnection.toInt(data, dataIndex);
					if(daemonEndianMismatch) {
							numMembers = SpreadConnection.flip(numMembers);
					}
					virtualSynchronySets[i] = new VirtualSynchronySet(numMembers);
					if( byteOffsetToLocalSet == dataIndex - byteIndexOfFirstSet ) {
							myVirtualSynchronySet = virtualSynchronySets[i];
					}
					dataIndex += 4;
					for( int j = 0 ; j < numMembers ; ++j ) {
							virtualSynchronySets[i].addMember( connection.toGroup(data, dataIndex) );
							dataIndex += connection.MAX_GROUP_NAME;
					}
			}
		}
	}

	// Checking the service-type.
	/////////////////////////////
	/**
	 * Check if this is a regular membership message.
	 * If true, {@link MembershipInfo#getGroup()}, {@link MembershipInfo#getGroupID()},
	 * and {@link MembershipInfo#getMembers()} are valid.
	 * One of {@link MembershipInfo#getJoined()}, {@link MembershipInfo#getLeft()}, 
	 * {@link MembershipInfo#getDisconnected()}, or {@link MembershipInfo#getStayed()} are valid
	 * depending on the type of message.
	 * 
	 * @return  true if this is a regular membership message
	 * @see  MembershipInfo#getGroup()
	 * @see  MembershipInfo#getGroupID()
	 * @see  MembershipInfo#getMembers()
	 * @see  MembershipInfo#getJoined()
	 * @see  MembershipInfo#getLeft()
	 * @see  MembershipInfo#getDisconnected()
	 * @see  MembershipInfo#getStayed()
	 */
	public boolean isRegularMembership()
	{
		return ((serviceType & SpreadMessage.REG_MEMB_MESS) != 0);
	}
	
	/**
	 * Check if this is a transition message.
	 * If true, {@link MembershipInfo#getGroup()} is the only valid get*() function.
	 * 
	 * @return  true if this is a transition message
	 * @see  MembershipInfo#getGroup()
	 */
	public boolean isTransition()
	{
		return ((serviceType & SpreadMessage.TRANSITION_MESS) != 0);
	}
	
	/**
	 * Check if this message was caused by a join.
	 * If true, {@link MembershipInfo#getJoined()} can be used to get the group member who joined.
	 * 
	 * @return  true if this message was caused by a join
	 * @see  MembershipInfo#getJoined()
	 */
	public boolean isCausedByJoin()
	{
		return ((serviceType & SpreadMessage.CAUSED_BY_JOIN) != 0);
	}
	
	/**
	 * Check if this message was caused by a leave.
	 * If true, {@link MembershipInfo#getLeft()} can be used to get the group member who left.
	 * 
	 * @return  true if this message was caused by a leave
	 * @see  MembershipInfo#getLeft()
	 */
	public boolean isCausedByLeave()
	{
		return ((serviceType & SpreadMessage.CAUSED_BY_LEAVE) != 0);
	}
	
	/**
	 * Check if this message was caused by a disconnect.
	 * If true, {@link MembershipInfo#getDisconnected()} can be used to get the group member who left.
	 * 
	 * @return  true if this message was caused by a disconnect
	 * @see  MembershipInfo#getDisconnected()
	 */
	public boolean isCausedByDisconnect()
	{
		return ((serviceType & SpreadMessage.CAUSED_BY_DISCONNECT) != 0);
	}
	
	/**
	 * Check if this message was caused by a network partition.
	 * If true, {@link MembershipInfo#getStayed()} can be used to get the members who are still in the group.
	 * 
	 * @return  true if this message was caused by a network partition
	 * @see  MembershipInfo#getStayed()
	 */
	public boolean isCausedByNetwork()
	{
		return ((serviceType & SpreadMessage.CAUSED_BY_NETWORK) != 0);
	}
	
	/**
	 * Check if this is a self-leave message.
	 * If true, {@link MembershipInfo#getGroup()} is the only valid get*() function.
	 * 
	 * @return  true if this is a self-leave message
	 * @see  MembershipInfo#getGroup()
	 */
	public boolean isSelfLeave()
	{
		return ((isCausedByLeave() == true) && (isRegularMembership() == false));
	}
	
	// Get the group this message came from.
	////////////////////////////////////////
	/**
	 * Gets a SpreadGroup object representing the group that caused this message.
	 * 
	 * @return  the group that caused this message
	 */
	public SpreadGroup getGroup()
	{
		return group;
	}
	
	// Get the group's ID.
	//////////////////////
	/**
	 * Gets the GroupID for this group membership at this point in time.
	 * This is only valid if {@link MembershipInfo#isRegularMembership()} is true.
	 * If it is not true, null is returned.
	 * 
	 * @return  the GroupID for this group memebership at this point in time
	 */
	public GroupID getGroupID()
	{
		return groupID;
	}
	
	// Get the group's members.
	///////////////////////////
	/**
	 * Gets the private groups for all the members in the new group membership.
	 * This list will be in the same order everywhere it is received.
	 * This is only valid if {@link MembershipInfo#isRegularMembership()} is true.
	 * If it is not true, null is returned.
	 * 
	 * @return  the private groups for everyone in the new group membership
	 */
	public SpreadGroup[] getMembers()
	{	
		// Check for a non-regular memberhsip message.
		//////////////////////////////////////////////
		if(isRegularMembership() == false)
		{
			return null;
		}
		
		// Get an array of groups.
		//////////////////////////
		SpreadGroup[] groupArray = new SpreadGroup[members.size()];
		for(int i = 0 ; i < groupArray.length ; i++)
		{
			// Set the array element.
			/////////////////////////
			groupArray[i] = (SpreadGroup)members.elementAt(i);
		}
		
		// Return the array.
		////////////////////
		return groupArray;
	}
	
	// Get the private group of the member who joined.
	//////////////////////////////////////////////////
	/**
	 * Gets the private group of the member who joined.
	 * This is only valid if both {@link MembershipInfo#isRegularMembership()} and
	 * {@link MembershipInfo#isCausedByJoin()} are true.
	 * If either or both are false, null is returned.
	 * 
	 * @return  the private group of the member who joined
	 * @see  MembershipInfo#isCausedByJoin()
	 */
	public SpreadGroup getJoined()
	{
		if((isRegularMembership()) && (isCausedByJoin()))
			return virtualSynchronySets[0].getMembers()[0];
		return null;
	}
	
	// Get the private group of the member who left.
	////////////////////////////////////////////////
	/**
	 * Gets the private group of the member who left.
	 * This is only valid if both {@link MembershipInfo#isRegularMembership()} and
	 * ( {@link MembershipInfo#isCausedByLeave()} or {@link MembershipInfo#isCausedByDisconnect()} ) are true.
	 * If either or both are false, null is returned.
	 * 
	 * @return  the private group of the member who left
	 * @see  MembershipInfo#isCausedByLeave() MembershipInfo#isCausedByDisconnect()
	 */
	public SpreadGroup getLeft()
	{
		if((isRegularMembership()) && 
		   ( isCausedByLeave() || isCausedByDisconnect() ) )
			return virtualSynchronySets[0].getMembers()[0];
		return null;
	}
	
	// Get the private group of the member who disconnected.
	////////////////////////////////////////////////////////
	/**
	 * Gets the private group of the member who disconnected.
	 * This is only valid if both {@link MembershipInfo#isRegularMembership()} and
	 * {@link MembershipInfo#isCausedByDisconnect()} are true.
	 * If either or both are false, null is returned.
	 * 
	 * @return  the private group of the member who disconnected
	 * @see  MembershipInfo#isCausedByDisconnect()
	 */
	public SpreadGroup getDisconnected()
	{
		if((isRegularMembership()) && (isCausedByDisconnect()))
			return virtualSynchronySets[0].getMembers()[0];
		return null;
	}
	
	// Get the private groups of the member who were not partitioned.
	/////////////////////////////////////////////////////////////////
	/**
	 * Gets the private groups of the members who were not partitioned.
	 * This is only valid if both {@link MembershipInfo#isRegularMembership()} and
	 * {@link MembershipInfo#isCausedByNetwork()} are true.
	 * If either or both are false, null is returned.
	 * 
	 * @return  the private groups of the members who were not partitioned
	 * @see  MembershipInfo#isCausedByNetwork()
	 * @deprecated -- new code should use the methods below.  This implementation
	 *                is provided for backwards-compatibility only.
	 */
	public SpreadGroup[] getStayed()
	{
			VirtualSynchronySet set = getMyVirtualSynchronySet();
			if( set == null ) {
					return null;
			}
			return set.getMembers();
	}

	// Get all the virtual synchrony sets from the associated membership
	// message.
	public VirtualSynchronySet[] getVirtualSynchronySets() {
		if( isRegularMembership() && isCausedByNetwork() ) {
			return virtualSynchronySets;
		}
		return null;
	}

	// Get my own virtual synchrony set.
	public VirtualSynchronySet getMyVirtualSynchronySet() {
		if( isRegularMembership() && isCausedByNetwork() ) {
			return myVirtualSynchronySet;
		}
		return null;
	}


	// This inner class is basically just a wrapper around an array of
	// SpreadGroups.  It is intended for use to represent the private group
	// names of a set of members known to have been virtually synchronous
	// in their last membership (before the membership represented by
	// this MembershipInfo).
	public class VirtualSynchronySet {

		private int size;
		private SpreadGroup privateGroups[];

		protected VirtualSynchronySet(int capacity) {
			size          = 0;
			privateGroups = new SpreadGroup[capacity];
		}

		public int getCapacity() {
			return privateGroups.length;
		}

		public int getSize() {
			return size;
		}

		public SpreadGroup[] getMembers() {
			return privateGroups;
		}

		public void addMember(SpreadGroup member) {
			privateGroups[size++] = member;
		}
	}

} // end class
