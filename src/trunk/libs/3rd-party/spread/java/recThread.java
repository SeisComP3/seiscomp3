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

import spread.*;

public class recThread extends Thread implements Runnable {
	private SpreadConnection connection;
        public  boolean threadSuspended;

	public recThread(SpreadConnection aConn) {
		connection=aConn;
	}

	// Print this membership data.  Does so in a generic way so identical
	// function is used in recThread and User. 
	private void printMembershipInfo(MembershipInfo info) 
	{
	        SpreadGroup group = info.getGroup();
		if(info.isRegularMembership()) {
			SpreadGroup members[] = info.getMembers();
			MembershipInfo.VirtualSynchronySet virtual_synchrony_sets[] = info.getVirtualSynchronySets();
			MembershipInfo.VirtualSynchronySet my_virtual_synchrony_set = info.getMyVirtualSynchronySet();

			System.out.println("REGULAR membership for group " + group +
					   " with " + members.length + " members:");
			for( int i = 0; i < members.length; ++i ) {
				System.out.println("\t\t" + members[i]);
			}
			System.out.println("Group ID is " + info.getGroupID());

			System.out.print("\tDue to ");
			if(info.isCausedByJoin()) {
				System.out.println("the JOIN of " + info.getJoined());
			}	else if(info.isCausedByLeave()) {
				System.out.println("the LEAVE of " + info.getLeft());
			}	else if(info.isCausedByDisconnect()) {
				System.out.println("the DISCONNECT of " + info.getDisconnected());
			} else if(info.isCausedByNetwork()) {
				System.out.println("NETWORK change");
				for( int i = 0 ; i < virtual_synchrony_sets.length ; ++i ) {
					MembershipInfo.VirtualSynchronySet set = virtual_synchrony_sets[i];
					SpreadGroup         setMembers[] = set.getMembers();
					System.out.print("\t\t");
					if( set == my_virtual_synchrony_set ) {
						System.out.print( "(LOCAL) " );
					} else {
						System.out.print( "(OTHER) " );
					}
					System.out.println( "Virtual Synchrony Set " + i + " has " +
							    set.getSize() + " members:");
					for( int j = 0; j < set.getSize(); ++j ) {
						System.out.println("\t\t\t" + setMembers[j]);
					}
				}
			}
		} else if(info.isTransition()) {
			System.out.println("TRANSITIONAL membership for group " + group);
		} else if(info.isSelfLeave()) {
			System.out.println("SELF-LEAVE message for group " + group);
		}
	}

	private void DisplayMessage(SpreadMessage msg)
	{
		try
		{
   		        System.out.println("*****************RECTHREAD Received Message************");
			if(msg.isRegular())
			{
				System.out.print("Received a ");
				if(msg.isUnreliable())
					System.out.print("UNRELIABLE");
				else if(msg.isReliable())
					System.out.print("RELIABLE");
				else if(msg.isFifo())
					System.out.print("FIFO");
				else if(msg.isCausal())
					System.out.print("CAUSAL");
				else if(msg.isAgreed())
					System.out.print("AGREED");
				else if(msg.isSafe())
					System.out.print("SAFE");
				System.out.println(" message.");
				
				System.out.println("Sent by  " + msg.getSender() + ".");
				
				System.out.println("Type is " + msg.getType() + ".");
				
				if(msg.getEndianMismatch() == true)
					System.out.println("There is an endian mismatch.");
				else
					System.out.println("There is no endian mismatch.");
				
				SpreadGroup groups[] = msg.getGroups();
				System.out.println("To " + groups.length + " groups.");
				
				byte data[] = msg.getData();
				System.out.println("The data is " + data.length + " bytes.");
				
				System.out.println("The message is: " + new String(data));
			}
			else if ( msg.isMembership() )
			{
				MembershipInfo info = msg.getMembershipInfo();
				printMembershipInfo(info);
			} else if ( msg.isReject() )
			{
			        // Received a Reject message 
				System.out.print("Received a ");
				if(msg.isUnreliable())
					System.out.print("UNRELIABLE");
				else if(msg.isReliable())
					System.out.print("RELIABLE");
				else if(msg.isFifo())
					System.out.print("FIFO");
				else if(msg.isCausal())
					System.out.print("CAUSAL");
				else if(msg.isAgreed())
					System.out.print("AGREED");
				else if(msg.isSafe())
					System.out.print("SAFE");
				System.out.println(" REJECTED message.");
				
				System.out.println("Sent by  " + msg.getSender() + ".");
				
				System.out.println("Type is " + msg.getType() + ".");
				
				if(msg.getEndianMismatch() == true)
					System.out.println("There is an endian mismatch.");
				else
					System.out.println("There is no endian mismatch.");
				
				SpreadGroup groups[] = msg.getGroups();
				System.out.println("To " + groups.length + " groups.");
				
				byte data[] = msg.getData();
				System.out.println("The data is " + data.length + " bytes.");
				
				System.out.println("The message is: " + new String(data));
			} else {
			    System.out.println("Message is of unknown type: " + msg.getServiceType() );
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
			System.exit(1);
		}
	}
	
	public void run() {
	  while(true) {
            try {
	      DisplayMessage(connection.receive());

	      if (threadSuspended) {
                synchronized(this) {
                    while (threadSuspended) {
                        wait();
		    }
                }
	      }
	    } catch(Exception e) {

	    }
	  }
        }
}
