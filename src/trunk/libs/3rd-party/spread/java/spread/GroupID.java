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
 * A GroupID represents a particular group membership view at a particular time in the history of the group.
 * A GroupID can be retrieved from a membership message using {@link MembershipInfo#getGroupID()}.
 * To check if two GroupID's represent the same group, use {@link GroupID#equals(Object)}:
 * <p><blockquote><pre>
 * if(thisID.equals(thatID) == true)
 *     System.out.println("Equal group ID's.");
 * </pre></blockquote><p>
 */
public class GroupID
{
	// The group ID.
	////////////////
	private int ID[] = new int[3];
	
	// Constructor.
	///////////////
	protected GroupID(int id0, int id1, int id2)
	{
		// Setup the ID.
		////////////////
		ID[0] = id0;
		ID[1] = id1;
		ID[2] = id2;
	}
	
	// Returns true if the object represents the same group ID.
	///////////////////////////////////////////////////////////
	/**
	 * Returns true if the two GroupID's represent the same group membership view at the same point in time
	 * in the history of the group.
	 * 
	 * @param  object  a GroupID to compare against
	 * @return  true if object is a GroupID object and the two GroupID's are the same
	 */
	public boolean equals(Object object)
	{
		// Check if it's the correct class.
		///////////////////////////////////
		if(object.getClass() != this.getClass())
		{
			return false;
		}
		
		// Check if the ID's are the same.
		//////////////////////////////////
		GroupID other = (GroupID)object;
		int otherID[] = other.getID();
		for(int i = 0 ; i < 3 ; i++)
		{
			if(ID[i] != otherID[i])
			{
				// The ID's are different.
				//////////////////////////
				return false;
			}
		}
		
		// No differences, the ID's are the same.
		/////////////////////////////////////////
		return true;
	}
	
	// Get the ID.
	//////////////
	protected int[] getID()
	{
		return ID;
	}

	// Returns the hash code of the group ID.
	///////////////////////////////////////////////////////////////////
	/**
	 * Returns the hash code of the group ID.
	 *
	 * @return int the hash code
	 */
	public int hashCode()
	{
		return ID.hashCode();
	}

	// Converts the group ID to a string.
	/////////////////////////////////////
	/**
	 * Converts the GroupID to a string.
	 * 
	 * @return  the string form of this GroupID
	 */
	public String toString()
	{
		return new String(ID[0] + " " + ID[1] + " " + ID[2]);
	}
}
