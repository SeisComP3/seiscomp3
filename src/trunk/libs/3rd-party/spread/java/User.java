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
import java.net.*;
import java.io.*;


public class User implements BasicMessageListener
{
	// The Spread Connection.
	/////////////////////////
	private SpreadConnection connection;
	
	// The keyboard input.
	//////////////////////
	private InputStreamReader keyboard;
	
	// A group.
	///////////
	private SpreadGroup group;
	
	// The number of messages sent.
	///////////////////////////////
	private int numSent;
	
	// True if there is a listening thread.
	///////////////////////////////////////
	private boolean listening;
	private recThread rt;

	private void PrintMenu()
	{
		// Print the menu.
		//////////////////
		System.out.print("\n" +
						 "==========\n" +
						 "User Menu:\n" +
						 "==========\n" +
						 "\n" +
						 "\tj <group> -- join a group\n" + 
						 "\tl -- leave a group\n" +
						 "\n" +
						 "\ts <group> -- send a message\n" +
						 "\tb <group> -- send a burst of messages\n" + 
						 "\n");
		if(listening == false)
		{
			System.out.print("\tr -- receive a message\n" +
						 "\tp -- poll for a message\n" +
						 "\tt -- add a listener\n" + 
						 "\n");
		}
		else
		{
			System.out.print("\tt -- stop the listener\n" +
						 "\n");
		}
		System.out.print("\tq -- quit\n");
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
			else if (msg.isMembership())
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
	
	private void UserCommand()
	{
		// Show the prompt.
		///////////////////
		System.out.print("\n" + 
						 "User> ");
		
		// Get the input.
		/////////////////
		char command[] = new char[1024];
		int num = 0;
		try
		{
			num = keyboard.read(command);
		}
		catch(IOException e)
		{
			e.printStackTrace();
			System.exit(1);
		}
		
		// Setup a tokenizer for the input.
		///////////////////////////////////
		StreamTokenizer tokenizer = new StreamTokenizer(new StringReader(new String(command, 1, num - 1)));
		
		// Check what it is.
		////////////////////
		SpreadMessage msg;
		char buffer[];
		try
		{
			switch(command[0])
			{
			//JOIN
			case 'j':
				// Join the group.
				//////////////////
				if(tokenizer.nextToken() != tokenizer.TT_EOF)
				{
					group = new SpreadGroup();
					group.join(connection, tokenizer.sval);
					System.out.println("Joined " + group + ".");
				}
				else
				{
					System.err.println("No group name.");
				}
				
				break;
				
			//LEAVE
			case 'l':
				// Leave the group.
				///////////////////
				if(group != null)
				{
					group.leave();
					System.out.println("Left " + group + ".");
				}
				else
				{
					System.out.println("No group to leave.");
				}
				
				break;
				
			//SEND
			case 's':
				// Get a new outgoing message.
				//////////////////////////////
				msg = new SpreadMessage();
				msg.setSafe();
				
				// Add the groups.
				//////////////////
				while(tokenizer.nextToken() != tokenizer.TT_EOF)
				{
					msg.addGroup(tokenizer.sval);
				}
				
				// Get the message.
				///////////////////
				System.out.print("Enter message: ");
				buffer = new char[100];
				num = keyboard.read(buffer);
				msg.setData(new String(buffer, 0, num - 1).getBytes());
				
				// Send it.
				///////////
				connection.multicast(msg);
				
				// Increment the sent message count.
				////////////////////////////////////
				numSent++;
				
				// Show how many were sent.
				///////////////////////////
				System.out.println("Sent message " + numSent + ".");
				
				break;
				
			//BURST
			case 'b':				
				// Get a new outgoing message.
				//////////////////////////////
				msg = new SpreadMessage();
				msg.setSafe();
				
				// Get the group.
				/////////////////
				if(tokenizer.nextToken() != tokenizer.TT_EOF)
				{
					msg.addGroup(tokenizer.sval);
				}
				else
				{
					System.err.println("No group name.");
					break;
				}
				
				// Get the message size.
				////////////////////////
				System.out.print("Enter the size of each message: ");
				buffer = new char[100];
				num = keyboard.read(buffer);
				int size = Integer.parseInt(new String(buffer, 0, num - 1));
				if(size < 0)
					size = 20;
				
				// Send the messages.
				/////////////////////
				System.out.println("Sending 10 messages of " + size + " bytes.");
				byte data[] = new byte[size];
				for(int i = 0 ; i < size ; i++)
				{
					data[i] = 0;
				}
				for(int i = 0 ; i < 10 ; i++)
				{
					// Increment the sent message count.
					////////////////////////////////////
					numSent++;
					
					// Set the message data.
					////////////////////////
					byte mess[] = new String("mess num " + i).getBytes("ISO8859_1");
					System.arraycopy(mess, 0, data, 0, mess.length);
					msg.setData(data);
					
					// Send the message.
					////////////////////
					connection.multicast(msg);
					System.out.println("Sent message " + (i + 1) + " (total " + numSent + ").");
				}
				
				break;
				
			//RECEIVE
			case 'r':
				if(listening == false)
				{
					// Receive a message.
					/////////////////////
					DisplayMessage(connection.receive());
				}
				
				break;
				
			//POLL
			case 'p':
				if(listening == false)
				{
					// Poll.
					////////
					if(connection.poll() == true)
					{
						System.out.println("There is a message waiting.");
					}
					else
					{
						System.out.println("There is no message waiting.");
					}
				}
				
				break;
				
			//THREAD
			case 't':
				if(listening)
				{
					connection.remove(this);
					if (rt.threadSuspended)
					    synchronized(rt) {
						rt.notify();
						rt.threadSuspended = false;
					    }
				}
				else
				{
					connection.add(this);
					synchronized(rt) {
					    rt.threadSuspended = true;
					}
				}
				
				listening = !listening;
				
				break;
				
			//QUIT
			case 'q':
				// Disconnect.
				//////////////
				connection.disconnect();
				
				// Quit.
				////////
				System.exit(0);
				
				break;
				
			default:
				// Unknown command.
				///////////////////
				System.out.println("Unknown command");
				
				// Show the menu again.
				///////////////////////
				PrintMenu();
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
			System.exit(1);
		}
	}
	
	public User(String user, String address, int port)
	{
		// Setup the keyboard input.
		////////////////////////////
		keyboard = new InputStreamReader(System.in);
		
		// Establish the spread connection.
		///////////////////////////////////
		try
		{
			connection = new SpreadConnection();
			connection.connect(InetAddress.getByName(address), port, user, false, true);
		}
		catch(SpreadException e)
		{
			System.err.println("There was an error connecting to the daemon.");
			e.printStackTrace();
			System.exit(1);
		}
		catch(UnknownHostException e)
		{
			System.err.println("Can't find the daemon " + address);
			System.exit(1);
		}
		
		rt = new recThread(connection);
		rt.start();
		// Show the menu.
		/////////////////
		PrintMenu();
		
		// Get a user command.
		//////////////////////
		while(true)
		{
			UserCommand();
		}
	}

	public void messageReceived(SpreadMessage message)
	{
		DisplayMessage(message);
	}
	
	public final static void main(String[] args)
	{			
		// Default values.
		//////////////////
		String user = new String("User");
		String address = null;
		int port = 0;
		
		// Check the args.
		//////////////////
		for(int i = 0 ; i < args.length ; i++)
		{
			// Check for user.
			//////////////////
			if((args[i].compareTo("-u") == 0) && (args.length > (i + 1)))
			{
				// Set user.
				////////////
				i++;
				user = args[i];
			}
			// Check for server.
			////////////////////
			else if((args[i].compareTo("-s") == 0) && (args.length > (i + 1)))
			{
				// Set the server.
				//////////////////
				i++;
				address = args[i];
			}
			// Check for port.
			//////////////////
			else if((args[i].compareTo("-p") == 0) && (args.length > (i + 1)))
			{
				// Set the port.
				////////////////
				i++;
				port = Integer.parseInt(args[i]);
			}
			else
			{
				System.out.print("Usage: user\n" + 
								 "\t[-u <user name>]   : unique user name\n" + 
								 "\t[-s <address>]     : the name or IP for the daemon\n" + 
								 "\t[-p <port>]        : the port for the daemon\n");
				System.exit(0);
			}
		}
		
		User u = new User(user, address, port);
	}
}
