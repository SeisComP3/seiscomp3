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

public class Flooder
{
	public Flooder(String user, int numMessages, int numBytes, String address, int port, boolean readOnly, boolean writeOnly)
	{
		try
		{
			// Start timer.
			///////////////
			long startTime = System.currentTimeMillis();
			
			// Connect.
			///////////
			SpreadConnection connection = new SpreadConnection();
			connection.connect(InetAddress.getByName(address), port, user, false, false);
			String privateName = connection.getPrivateGroup().toString();
			
			// Join.
			////////
			SpreadGroup group = new SpreadGroup();
			if(readOnly)
			{
				System.out.println("Only receiving messages");
				group.join(connection, "flooder");
			}
			else if(writeOnly)
			{
				System.out.println("Starting multicast of " + numMessages + " messages, " + numBytes + " bytes each (self discarding).");
			}
			else
			{
				group.join(connection, "flooder");
				System.out.println("Starting multicast of " + numMessages + " messages, " + numBytes + " bytes each.");
			}
			
			// The outgoing message.
			////////////////////////
			SpreadMessage out = null;
			if(readOnly == false)
			{
				out = new SpreadMessage();
				out.setData(new byte[numBytes]);
				out.addGroup("flooder");
			}
			
			// Send/Receive.
			////////////////
			for(int i = 1 ; i <= numMessages ; i++)
			{
				// Send.
				////////
				if(readOnly == false)
				{
					connection.multicast(out);
				}
				
				// Receive.
				///////////
				if((readOnly) || ((i > 50) && (writeOnly == false)))
				{
					SpreadMessage in;
					do
					{
						in = connection.receive();
					}
					while((readOnly == false) && (privateName.equals(in.getSender().toString()) == false));
				}
				
				// Report.
				//////////
				if((i % 1000) == 0)
				{
					System.out.println("Completed " + i + " messages");
				}
			}
			
			// Stop timer.
			//////////////
			long stopTime = System.currentTimeMillis();
			long time = (stopTime - startTime);
			float Mbps = numBytes;
			Mbps *= numMessages;
			Mbps *= 8;
			if((readOnly == false) && (writeOnly == false))
				Mbps *= 2;
			Mbps *= 1000;
			Mbps /= time;
			Mbps /= (1024 * 1024);
			System.out.println("Time: " + time + "ms (" + (int)Mbps + "." + (((int)(Mbps * 100)) % 100) + " Mbps)");
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	public static void main(String[] args)
	{
		String user = new String("Flooder");
		int numMessages = 10000;
		int numBytes = 1000;
		String address = null;
		int port = 0;
		boolean readOnly = false;
		boolean writeOnly = false;
		
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
			// Check for numMessages.
			/////////////////////////
			else if((args[i].compareTo("-m") == 0) && (args.length > (i + 1)))
			{
				// Set numMessages.
				///////////////////
				i++;
				numMessages = Integer.parseInt(args[i]);
			}
			// Check for numBytes.
			//////////////////////
			else if((args[i].compareTo("-b") == 0) && (args.length > (i + 1)))
			{
				// Set numBytes.
				////////////////
				i++;
				numBytes = Integer.parseInt(args[i]);
			}
			// Check for address.
			/////////////////////
			else if((args[i].compareTo("-s") == 0) && (args.length > (i + 1)))
			{
				// Set address.
				///////////////
				i++;
				address = args[i];
			}
			// Check for port.
			//////////////////
			else if((args[i].compareTo("-p") == 0) && (args.length > (i + 1)))
			{
				// Set port.
				////////////
				i++;
				port = Integer.parseInt(args[i]);
			}
			// Check for readOnly.
			//////////////////////
			else if(args[i].compareTo("-ro") == 0)
			{
				// Set readOnly.
				////////////////
				readOnly = true;
				writeOnly = false;
			}
			// Check for writeOnly.
			///////////////////////
			else if(args[i].compareTo("-wo") == 0)
			{
				// Set writeOnly.
				/////////////////
				writeOnly = true;
				readOnly = false;
			}
			else
			{
				System.out.print("Usage: flooder\n" + 
								 "\t[-u <user name>]     : unique user name\n" + 
								 "\t[-m <num messages>]  : number of messages\n" + 
								 "\t[-b <num bytes>]     : number of bytes per message\n" + 
								 "\t[-s <address>]       : the name or IP for the daemon\n" + 
								 "\t[-p <port>]          : the port for the daemon\n" + 
								 "\t[-ro]                : read  only (no multicast)\n" + 
								 "\t[-wo]                : write only (no receive)\n");
				System.exit(0);
			}
		}
		
		Flooder f = new Flooder(user, numMessages, numBytes, address, port, readOnly, writeOnly);
	}
}
