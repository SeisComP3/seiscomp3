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

import java.net.*;
import java.io.*;
import java.util.*;

/**
 * A SpreadConnection object is used to establish a connection to a spread daemon.
 * To connect to a spread daemon, first create a new SpreadConnection object, and then
 * call {@link SpreadConnection#connect(InetAddress, int, String, boolean, boolean)}:
 * <p><blockquote><pre>
 * SpreadConnection connection = new SpreadConnection();
 * connection.connect(null, 0, "name", false, false);
 * </pre></blockquote><p>
 * The only methods that can be called before
 * {@link SpreadConnection#connect(InetAddress, int, String, boolean, boolean)} are the add
 * ({@link SpreadConnection#add(BasicMessageListener)}, {@link SpreadConnection#add(AdvancedMessageListener)})
 * and remove ({@link SpreadConnection#remove(BasicMessageListener)}, {@link SpreadConnection#remove(AdvancedMessageListener)})
 * methods.  If any other methods are called, a SpreadException is thrown, except for
 * {@link SpreadConnection#getPrivateGroup()}, which returns null.
 * <p>
 * To disconnect from the daemon, call {@link SpreadConnection#disconnect()}:
 * <p><blockquote><pre>
 * connection.disconnect();
 * </pre></blockquote><p>
 * To send a message on this connection, call {@link SpreadConnection#multicast(SpreadMessage)}:
 * <p><blockquote><pre>
 * connection.multicast(message);
 * </pre></blockquote><p>
 * To receive a message sent to this connection, call {@link SpreadConnection#receive()}:
 * <p><blockquote><pre>
 * SpreadMessage message = connection.receive();
 * </pre></blockquote><p>
 */
public class SpreadConnection
{
	// The default Spread port.
	///////////////////////////
	private static final int DEFAULT_SPREAD_PORT = 4803;
	
	// The maximum length of the private name.
	//////////////////////////////////////////
	private static final int MAX_PRIVATE_NAME = 10;
	
	// The maximum length of a message + group names.
	//////////////////////////////////////////
	private static final int MAX_MESSAGE_LENGTH = 140000;

	// The maximum length of the group name.
	////////////////////////////////////////
	protected static final int MAX_GROUP_NAME = 32;
	
	// The Spread version.
	//////////////////////
        private static final int SP_MAJOR_VERSION = 4;
	private static final int SP_MINOR_VERSION = 4;
	private static final int SP_PATCH_VERSION = 0;

        // The default authentication method
        ////////////////////////////////////
	private static final String DEFAULT_AUTH_NAME = "NULL";

        // The class name of the default authentication method
        //////////////////////////////////////////////////////
	private static final String DEFAULT_AUTHCLASS_NAME = "spread.NULLAuth";

	// The maximum length of a authentication method name
	/////////////////////////////////////////////////////
        private static final int MAX_AUTH_NAME = 30;

	// The maximum number of authentication methods
	///////////////////////////////////////////////
        private static final int MAX_AUTH_METHODS = 3;

	// Received if a connection attempt was successful.
	///////////////////////////////////////////////////
	private static final int ACCEPT_SESSION = 1;
	
	// Used to determine endianness.
	////////////////////////////////
	private static final int ENDIAN_TYPE = 0x80000080;
	
	// Only true if this connection is connected.
	/////////////////////////////////////////////
	private boolean connected;
	
	// Reading synchro.
	///////////////////
	private Boolean rsynchro;
	// Writing synchro.
	///////////////////
	private Boolean wsynchro;
	// Listener list synchro.
	///////////////////
	private Boolean listenersynchro;

    
	// True if calling listeners.
	/////////////////////////////
	private boolean callingListeners;
	
	// The thread feeding the listeners.
	////////////////////////////////////
	private Listener listener;
	
	// Basic listeners.
	///////////////////
	protected Vector basicListeners;
	
	// Advanced listeners.
	//////////////////////
	protected Vector advancedListeners;
	
	// The daemon's address.
	////////////////////////
	private InetAddress address;
	
	// The daemon's port.
	/////////////////////
	private int port;
	
	// Is this a priority connection?
	/////////////////////////////////
	private boolean priority;
	
	// Getting group membership messages?
	/////////////////////////////////////
	private boolean groupMembership;
	
	// Name of active choosen Authentication method
	///////////////////////////////////////////////
	private String authName;

	// Name of class for active choosen Authentication method
	/////////////////////////////////////////////////////////
	private String authClassName;

        // Object reference to current authentication class
        ///////////////////////////////////////////////////
        private Object authObj;

	// Method reference to authenticate method
	//////////////////////////////////////////
	private java.lang.reflect.Method authMethodAuthenticate;

	// The socket this connection is using.
	///////////////////////////////////////
	private Socket socket;
	
	// The socket's input stream.
	/////////////////////////////
	private InputStream socketInput;
	
	// The socket's output stream.
	//////////////////////////////
	private OutputStream socketOutput;
	
	// The private group.
	/////////////////////
	private SpreadGroup group;
	
	// Commands buffered during listener callbacks.
	// The buffer is a list of BUFFER_* constants.
	// For commands with an argument (all except
	// for BUFFER_DISCONNECT), the argument follows in the Vector.
	//////////////////////////////////////////////////////////////
	private Vector listenerBuffer;
	
	// Listener buffer commands.
	// These are Object's because they need to be added to a Vector.
	////////////////////////////////////////////////////////////////
	private static final Object BUFFER_DISCONNECT = new Object();
	private static final Object BUFFER_ADD_BASIC = new Object();
	private static final Object BUFFER_ADD_ADVANCED = new Object();
	private static final Object BUFFER_REMOVE_BASIC = new Object();
	private static final Object BUFFER_REMOVE_ADVANCED = new Object();
	
	// Checks if the int is the same-endian type as the local machine.
	//////////////////////////////////////////////////////////////////
	private static boolean sameEndian(int i)
	{
		return ((i & ENDIAN_TYPE) == 0);
	}
	
	// Clears the int's endian type.
	////////////////////////////////
	private static int clearEndian(int i)
	{
		return (i & ~ENDIAN_TYPE);
	}
	
	// Endian-flips the int.
	////////////////////////
	protected static int flip(int i)
	{
		return (((i >> 24) & 0x000000ff) | ((i >>  8) & 0x0000ff00) | ((i <<  8) & 0x00ff0000) | ((i << 24) & 0xff000000));
	}

	// Endian-flips the short.
	//////////////////////////
	private static short flip(short s)
	{
		return ((short)(((s >> 8) & 0x00ff) | ((s << 8 ) & 0xff00)));
	}
	
	// Puts a group name into an array of bytes.
	////////////////////////////////////////////
	private static void toBytes(SpreadGroup group, byte buffer[], int bufferIndex)
	{
		// Get the group's name.
		////////////////////////
		byte name[];
		try
		{
			name = group.toString().getBytes("ISO8859_1");
		}
		catch(UnsupportedEncodingException e)
		{
			// Already checked for this exception in connect.
			/////////////////////////////////////////////////
			name = new byte[0];
		}
		
		// Put a cap on the length.
		///////////////////////////
		int len = name.length;
		if(len > MAX_GROUP_NAME)
			len = MAX_GROUP_NAME;

		// Copy the name into the buffer.
		/////////////////////////////////
		System.arraycopy(name, 0, buffer, bufferIndex, len);
		for( ; len < MAX_GROUP_NAME ; len++ )
			buffer[bufferIndex + len] = 0;
	}
	
	// Puts an int into an array of bytes.
	//////////////////////////////////////
	private static void toBytes(int i, byte buffer[], int bufferIndex)
	{
		buffer[bufferIndex++] = (byte)((i >> 24) & 0xFF);
		buffer[bufferIndex++] = (byte)((i >> 16) & 0xFF);
		buffer[bufferIndex++] = (byte)((i >> 8 ) & 0xFF);
		buffer[bufferIndex++] = (byte)((i      ) & 0xFF);
	}
	
	// Gets an int from an array of bytes.
	//////////////////////////////////////
	protected static int toInt(byte buffer[], int bufferIndex)
	{
		int i0 = (buffer[bufferIndex++] & 0xFF);
		int i1 = (buffer[bufferIndex++] & 0xFF);
		int i2 = (buffer[bufferIndex++] & 0xFF);
		int i3 = (buffer[bufferIndex++] & 0xFF);
		
		return ((i0 << 24) | (i1 << 16) | (i2 << 8) | (i3));
	}	

    // Reads from inputsocket until all bytes read or exception raised
    //////////////////////////////////////////////////////////////////
    private void readBytesFromSocket(byte buffer[], String bufferTypeString) throws SpreadException
    {
	int byteIndex;
	int rcode;
	try
	{
	    for(byteIndex = 0 ; byteIndex < buffer.length ; byteIndex += rcode)
	    {
		rcode = socketInput.read(buffer, byteIndex, buffer.length - byteIndex);
		if(rcode == -1)
		{
		    throw new SpreadException("Connection closed while reading " + bufferTypeString);
		}
	    }
	}
	catch(InterruptedIOException e) {
	    throw new SpreadException("readBytesFromSocket(): InterruptedIOException " + e);
	}
	catch(IOException e)
	{
	    throw new SpreadException("readBytesFromSocket(): read() " + e);
	}

    }

    // Reads from inputsocket until all bytes read or an exception raised which indicates the read is
    // failing and the socket will be closed
    //////////////////////////////////////////////////////////////////
    private void readBytesFromSocketListener(byte buffer[], String bufferTypeString) throws SpreadException, InterruptedIOException
    {
	int byteIndex;
	int rcode;
	boolean keep_going = true;

	byteIndex = 0;
	while( keep_going == true )
	{
	    keep_going = false;
	    try {
		while( byteIndex < buffer.length) 
		{
		    rcode = socketInput.read(buffer, byteIndex, buffer.length - byteIndex);
		    if(rcode == -1)
		    {
			throw new SpreadException("Connection closed while reading " + bufferTypeString);
		    }
		    byteIndex += rcode;
		}

	    } 
	    catch( InterruptedIOException e) {
		if ( listener != null && listener.signal == true )
		{
		    throw( e );
		}
		if ( e.bytesTransferred >= 0 ) {
		    byteIndex += e.bytesTransferred;
		}
		keep_going = true;
	    }
	    catch(IOException e)
	    {
		throw new SpreadException("readBytesFromSocketListener(): read() " + e);
	    }
	}

    }

	
	// Gets a string from an array of bytes.
	////////////////////////////////////////
	protected SpreadGroup toGroup(byte buffer[], int bufferIndex)
	{
		try
		{
			for(int end = bufferIndex ; end < buffer.length ; end++)
			{
				if(buffer[end] == 0)
				{
					// Get the group name.
					//////////////////////
					String name = new String(buffer, bufferIndex, end - bufferIndex, "ISO8859_1");
					
					// Return the group.
					////////////////////
					return new SpreadGroup(this, name);
				}
			}
		}
		catch(UnsupportedEncodingException e)
		{
			// Already checked for this exception in connect.
			/////////////////////////////////////////////////
		}
	
		return null;
	}
	
	// Set the send and receive buffer sizes.
	/////////////////////////////////////////
	private void setBufferSizes() throws SpreadException
	{
/* NOT SUPPORTED IN 1.1			
		try
		{
			for(int i = 10 ; i <= 200 ; i += 5)
			{
				// The size to try.
				///////////////////
				int size = (1024 * i);
				
				// Set the buffer sizes.
				////////////////////////
				socket.setSendBufferSize(size);
				socket.setReceiveBufferSize(size);
				
				// Check the actual sizes.  If smaller, then the max was hit.
				/////////////////////////////////////////////////////////////
				if((socket.getSendBufferSize() < size) || (socket.getReceiveBufferSize() < size))
				{
					break;
				}
			}
		}
		catch(SocketException e)
		{
			throw new SpreadException("set/getSend/ReceiveBufferSize(): " + e);
		}
NOT SUPPORTED IN 1.1	*/
	}
	
	// Sends the initial connect message.
	/////////////////////////////////////
	private void sendConnect(String privateName) throws SpreadException
	{
		// Check the private name for validity.
		///////////////////////////////////////
		int len = (privateName == null ? 0 : privateName.length());
		if(len > MAX_PRIVATE_NAME)
		{
			privateName = privateName.substring(0, MAX_PRIVATE_NAME);
			len = MAX_PRIVATE_NAME;
		}
		
		// Allocate the buffer.
		///////////////////////
		byte buffer[] = new byte[len + 5];
		
		// Set the version.
		///////////////////
		buffer[0] = (byte)SP_MAJOR_VERSION;
		buffer[1] = (byte)SP_MINOR_VERSION;
		buffer[2] = (byte)SP_PATCH_VERSION;

		// Byte used for group membership and priority.
		///////////////////////////////////////////////
		buffer[3] = 0;
		
		// Group membership.
		////////////////////
		if(groupMembership)
		{
			buffer[3] |= 0x01;
		}
		
		// Priority.
		////////////
		if(priority)
		{
			buffer[3] |= 0x10;
		}
		
		// Write the length.
		////////////////////
		buffer[4] = (byte)len;
		
		if(len > 0)
		{
			// Write the private name.
			//////////////////////////
			byte nameBytes[] = privateName.getBytes();
			for(int src = 0, dest = 5 ; src < len ; src++, dest++)
			{
				buffer[dest] = nameBytes[src];
			}
		}
		
		// Send the connection message.
		///////////////////////////////
		try
		{
			socketOutput.write(buffer);
		}
		catch(IOException e)
		{
			throw new SpreadException("write(): " + e);
		}		
	}

        // read the Auth List
        /////////////////////
    	private void readAuthMethods() throws SpreadException
	{
		// Read the length.
		///////////////////
		int len;
		try
		{
			len = socketInput.read();
		}
		catch(IOException e)
		{
			throw new SpreadException("read(): " + e);
		}
		
		// System.out.println("readAuthMethods: len is " + len);
		// Check for no more data.
		//////////////////////////
		if(len == -1)
		{
			throw new SpreadException("Connection closed during connect attempt to read authlen");
		}
		// Check if it was a response code
		//////////////////////////////////
		if( len >= 128 )
		{
			throw new SpreadException("Connection attempt rejected=" + (0xffffff00 | len));
		}

		// Read the name.
		/////////////////
		byte buffer[] = new byte[len];
		readBytesFromSocket(buffer, "authname");
		// System.out.println("readAuthMethods: string is " + new String(buffer));

		//		if(numRead != len)
		//{
		//	throw new SpreadException("Connection closed during connect attempt to read authnames");
		//}
		
		// for now we ignore the list.
		//////////////////////////////
	}

        // Sends the choice of auth methods  message.
	/////////////////////////////////////
	private void sendAuthMethod() throws SpreadException
	{
		int len = authName.length();
		// Allocate the buffer.
		///////////////////////
		byte buffer[] = new byte[MAX_AUTH_NAME*MAX_AUTH_METHODS];
		
		try
		{
			System.arraycopy(authName.getBytes("ISO8859_1"), 0, buffer, 0, len);
		}
		catch(UnsupportedEncodingException e)
		{
			// Already checked for this exception in connect.
			/////////////////////////////////////////////////
		}
		for( ; len < (MAX_AUTH_NAME*MAX_AUTH_METHODS) ; len++ )
			buffer[len] = 0;

		// Send the connection message.
		///////////////////////////////
		try
		{
			socketOutput.write(buffer);
		}
		catch(IOException e)
		{
			throw new SpreadException("write(): " + e);
		}		
	}

        // 
	/////////////////////////////////////
	private void instantiateAuthMethod() throws SpreadException
	{
		Class authclass;

		//		System.out.println("Authname is " + authName);
		//		System.out.println("class name is " + authClassName);
		try {
			authclass = Class.forName(authClassName);
		} catch (ClassNotFoundException e) {
			throw new SpreadException("class " + authClassName + " not found.\n");
		}

		try {
			authObj = authclass.newInstance();
		} catch (Exception e) {
			throw new SpreadException("class " + authClassName + " error getting instance.\n" + e);
		}
		try {
		    authMethodAuthenticate = authclass.getMethod("authenticate", new Class[] { });
		} catch (NoSuchMethodException e) {
		    System.out.println("Failed to find auth method authenticate()");
		    System.exit(1);
		} catch (SecurityException e) {
		    System.out.println("security exception for method authenticate()");
		    System.exit(1);
		}


	}
	// Checks for an accept message.
	////////////////////////////////
	private void checkAccept() throws SpreadException
	{
		// Read the connection response.
		////////////////////////////////
		int accepted;
		try
		{
			accepted = socketInput.read();
		}
		catch(IOException e)
		{
			throw new SpreadException("read(): " + e);
		}
		
		// Check for no more data.
		//////////////////////////
		if(accepted == -1)
		{
			throw new SpreadException("Connection closed during connect attempt");
		}
		
		// Was it accepted?
		///////////////////
		if(accepted != ACCEPT_SESSION)
		{
			throw new SpreadException("Connection attempt rejected=" + (0xffffff00 | accepted));
		}
	}
	
	// Checks the daemon version.
	/////////////////////////////
	private void checkVersion() throws SpreadException
	{		
		// Read the version.
		////////////////////
		int majorVersion;
		try
		{
			majorVersion = socketInput.read();
		}
		catch(IOException e)
		{
			throw new SpreadException("read(): " + e);
		}
		
		// Read the sub-version.
		////////////////////////
		int minorVersion;
		try
		{
			minorVersion = socketInput.read();
		}
		catch(IOException e)
		{
			throw new SpreadException("read(): " + e);
		}
		
		// Read the patch-version.
		////////////////////////
		int patchVersion;
		try
		{
			patchVersion = socketInput.read();
		}
		catch(IOException e)
		{
			throw new SpreadException("read(): " + e);
		}
		
		// Check for no more data.
		//////////////////////////
		if((majorVersion == -1) || (minorVersion == -1) || (patchVersion == -1) )
		{
			throw new SpreadException("Connection closed during connect attempt");
		}
		
		// Check the version.
		/////////////////////
		int version = ( (majorVersion*10000) + (minorVersion*100) + patchVersion );
		if(version < 30100)
		{
			throw new SpreadException("Old version " + majorVersion + "." + minorVersion + "." + patchVersion + " not supported");
		}
		if((version < 30800) && (priority))
		{
			throw new SpreadException("Old version " + majorVersion + "." + minorVersion + "." + patchVersion + " does not support priority");
		}
	}
	
	// Get the private group name.
	//////////////////////////////
	private void readGroup() throws SpreadException
	{
		// Read the length.
		///////////////////
		int len;
		try
		{
			len = socketInput.read();
		}
		catch(IOException e)
		{
			throw new SpreadException("read(): " + e);
		}
		
		// Check for no more data.
		//////////////////////////
		if(len == -1)
		{
			throw new SpreadException("Connection closed during connect attempt");
		}
		
		// Read the name.
		/////////////////
		byte buffer[] = new byte[len];
		readBytesFromSocket(buffer, "group name");

		// Store the group.
		///////////////////
		group = new SpreadGroup(this, new String(buffer));
	}
	
	// Constructor.
	///////////////
	/**
	 * Initializes a new SpreadConnection object.  To connect to a daemon with this
	 * object, use {@link SpreadConnection#connect(InetAddress, int, String, boolean, boolean)}.
	 * 
	 * @see  SpreadConnection#connect(InetAddress, int, String, boolean, boolean)
	 */
	public SpreadConnection()
	{
		// We're not connected.
		///////////////////////
		connected = false;
		
		// Init synchros.
		/////////////////
		rsynchro = new Boolean(false);
		wsynchro = new Boolean(false);
		listenersynchro = new Boolean(false);
		// Init listeners.
		//////////////////
		basicListeners = new Vector();
		advancedListeners = new Vector();
		
		// Init listener command buffer.
		////////////////////////////////
		listenerBuffer = new Vector();

		// Init default authentication
		//////////////////////////////
		authName = DEFAULT_AUTH_NAME;
		authClassName = DEFAULT_AUTHCLASS_NAME;
	}

        /**
         * Sets the authentication name and class string for the client side authentication method.
         * An authentication method can only be registered before connect is called. 
         * The authentication method registered will then be used whenever
         * {@link SpreadConnection#connect(InetAddress, int, String, boolean, boolean)} is called.
         *
         * @param  authName  the short official "name" of the method begin registered.
         * @param  authClassName  the complete class name for the method (including package)
         * @throws SpreadException if the connection is already established
         */
        synchronized public void registerAuthentication( String authName, String authClassName ) throws SpreadException
        {
		// Check if we're connected.
		////////////////////////////
		if(connected == true)
		{
			throw new SpreadException("Already connected.");
		}

                this.authClassName = authClassName;

		try
		{
			this.authName = authName.substring(0, MAX_AUTH_NAME);
		}
		catch(IndexOutOfBoundsException e)
		{
			// Nothing to shorten.
			//////////////////////
			this.authName = authName;
		}
        }

	// Establishes a connection with the spread daemon.
	///////////////////////////////////////////////////
	/**
	 * Establishes a connection to a spread daemon.  Groups can be joined, and messages can be
	 * sent or received once the connection has been established.
	 * 
	 * @param  address  the daemon's address, or null to connect to the localhost
	 * @param  port  the daemon's port, or 0 for the default port (4803)
	 * @param  privateName  the private name to use for this connection
	 * @param  priority  if true, this is a priority connection
	 * @param  groupMembership  if true, membership messages will be received on this connection
	 * @throws SpreadException  if the connection cannot be established
	 * @see  SpreadConnection#disconnect()
	 */
	synchronized public void connect(InetAddress address, int port, String privateName, boolean priority, boolean groupMembership) throws SpreadException
	{
		// Check if we're connected.
		////////////////////////////
		if(connected == true)
		{
			throw new SpreadException("Already connected.");
		}
		
		// Is ISO8859_1 encoding supported?
		///////////////////////////////
		try
		{
			new String("ASCII/ISO8859_1 encoding test").getBytes("ISO8859_1");
		}
		catch(UnsupportedEncodingException e)
		{
			throw new SpreadException("ISO8859_1 encoding is not supported.");
		}
		
		// Store member variables.
		//////////////////////////
		this.address = address;
		this.port = port;
		this.priority = priority;
		this.groupMembership = groupMembership;
		
		// Check if no address was specified.
		/////////////////////////////////////
		if(address == null)
		{
			// Use the local host.
			//////////////////////
			try
			{
				address = InetAddress.getLocalHost();
			}
			catch(UnknownHostException e)
			{
				throw new SpreadException("Error getting local host");
			}
		}
		
		// Check if no port was specified.
		//////////////////////////////////
		if(port == 0)
		{
			// Use the default port.
			////////////////////////
			port = DEFAULT_SPREAD_PORT;
		}
		
		// Check if the port is out of range.
		/////////////////////////////////////
		if((port < 0) || (port > (32 * 1024)))
		{
			throw new SpreadException("Bad port (" + port + ").");
		}
		
		// Create the socket.
		/////////////////////
		try
		{
			socket = new Socket(address, port);
		}
		catch(IOException e)
		{
			throw new SpreadException("Socket(): " + e);
		}
		
		try 
		{
		    socket.setTcpNoDelay(true);
		}
		catch(SocketException e) 
                {
		    SpreadException se = new SpreadException("setTcpNoDelay(): " + e.getMessage());
		    se.initCause(e);
		    throw se;
		}

		// Set the socket's buffer sizes.
		/////////////////////////////////
		setBufferSizes();
				
		// Get the socket's streams.
		////////////////////////////
		try
		{
			socketInput = socket.getInputStream();
			socketOutput = socket.getOutputStream();
		}
		catch(IOException e)
		{
			throw new SpreadException("getInput/OutputStream(): " + e);
		}
		
		// Send the connect message.
		////////////////////////////
		sendConnect(privateName);
		
		// Recv the authentication method list
		//////////////////////////////////////
		readAuthMethods();

		// Send auth method choice
		//////////////////////////
		sendAuthMethod();

		// turn string name of auth method into class and instance
		//////////////////////////////////////////////////////////
		try {
		    instantiateAuthMethod();
		} catch (SpreadException e) {
		    System.out.println("Failed to create authMethod instance" + e.toString() );
		    System.exit(1);
		}
		// Call authenticate module. This will only return when connection is authenticated
		////////////////////////////
		try {
			    authMethodAuthenticate.invoke( authObj, new Object[] { });
		    } catch (IllegalAccessException e) {
			    System.out.println("error calling authenticate" + e.toString() );
			    System.exit(1);
		    } catch (IllegalArgumentException e) {
			    System.out.println("error calling authenticate" + e.toString() );
			    System.exit(1);
		    } catch (java.lang.reflect.InvocationTargetException e) {
			    Throwable thr = e.getTargetException();
			    if ( thr.getClass().equals(SpreadException.class) )
			    {
				 throw new SpreadException("Connection Rejected: Authentication failed");   
			    }
		}
		// Check for acceptance.
		////////////////////////
		checkAccept();
		
		// Check the version.
		/////////////////////
		checkVersion();
		
		// Get the private group name.
		//////////////////////////////
		readGroup();
		
		// Connection complete.
		///////////////////////
		connected = true;
		
		// Are there any listeners.
		///////////////////////////
		if((basicListeners.size() != 0) || (advancedListeners.size() != 0))
		{
			// Start the listener thread.
			/////////////////////////////
			startListener();
		}
	}
	
	// Disconnects from the spread daemon.
	//////////////////////////////////////
	/**
	 * Disconnects the connection to the daemon.  Nothing else should be done with this connection
	 * after disconnecting it.
	 * 
	 * @throws  SpreadException  if there is no connection or there is an error disconnecting
	 * @see  SpreadConnection#connect(InetAddress, int, String, boolean, boolean)
	 */
	synchronized public void disconnect() throws SpreadException
	{
		// Check if we're connected.
		////////////////////////////
		if(connected == false)
		{
			throw new SpreadException("Not connected.");
		}
		
		// Are we in a listener callback?
		/////////////////////////////////
		if(callingListeners)
		{
			// Add it to the command buffer.
			////////////////////////////////
			listenerBuffer.addElement(BUFFER_DISCONNECT);
			
			// Don't need to do anything else.
			//////////////////////////////////
			return;
		}
		try {
		    // Get a new message.
		    /////////////////////
		    SpreadMessage killMessage = new SpreadMessage();
		
		    // Send it to our private group.
		    ////////////////////////////////
		    killMessage.addGroup(group);
		
		    // Set the service type.
		    ////////////////////////
		    killMessage.setServiceType(SpreadMessage.KILL_MESS);
		
		    // Send the message.
		    ////////////////////
		    multicast(killMessage);
		
		    // Check for a listener thread.
		    ///////////////////////////////
		    if(listener != null)
		    {
			// Stop it.
			///////////
			stopListener();
		    }
		
		    // Close the socket.
		    ////////////////////
		    try
		    {
			socket.close();
		    }
		    catch(IOException e)
		    {
			throw new SpreadException("close(): " + e);
		    }
		} 
		finally {
		    connected = false;
		}
	}
	
	// Gets the user's private group.
	/////////////////////////////////
	/**
	 * Gets the private group for this connection.
	 * 
	 * @return  the SpreadGroup representing this connection's private group, or null if there is no connection
	 */
	public SpreadGroup getPrivateGroup()
	{
		// Check if we're connected.
		////////////////////////////
		if(connected == false)
		{
			return null;
		}
		
		return group;
	}
	
	// Receives a new message.
	//////////////////////////
	/**
	 * Receives the next message waiting on this connection.  If there are no messages
	 * waiting, the call will block until a message is ready to be received.
	 * 
	 * @return  the message that has just been received
	 * @throws  SpreadException  if there is no connection or there is any error reading a new message
	 */
        public SpreadMessage receive() throws SpreadException, InterruptedIOException
	{

	    synchronized(rsynchro) {
	        // Check if there are any listeners.
		////////////////////////////////////
	        if((basicListeners.isEmpty() == false) || (advancedListeners.isEmpty() == false))
		{
			// Get out of here.
			///////////////////
		 	throw new SpreadException("Tried to receive while there are listeners");
		}
		
		return internal_receive();
	    }
	}

        // Actually receives a new message
        /////////////////////////////////// 
	private SpreadMessage internal_receive() throws SpreadException, InterruptedIOException
	{
		// Check if we're connected.
		////////////////////////////
		if(connected == false)
		{
			throw new SpreadException("Not connected.");
		}
		
		// Read the header.
		///////////////////
		byte header[] = new byte[MAX_GROUP_NAME+16];
		int headerIndex;

		try 
	        {
		    readBytesFromSocketListener( header, "message header" );
		}
		catch(InterruptedIOException e) {
		    throw e;
		} 
		catch(IOException e) {
			throw new SpreadException("read(): " + e);
		}
		
		// Reset header index.
		//////////////////////
		headerIndex = 0;
		
		// Get service type.
		////////////////////
		int serviceType = toInt(header, headerIndex);
		headerIndex += 4;

		// Get the sender.
		//////////////////
		SpreadGroup sender = toGroup(header, headerIndex);
		headerIndex += MAX_GROUP_NAME;

		// Get the number of groups.
		////////////////////////////
		int numGroups = toInt(header, headerIndex);
		headerIndex += 4;

		// Get the hint/type.
		/////////////////////
		int hint = toInt(header, headerIndex);
		headerIndex += 4;

		// Get the data length.
		///////////////////////
		int dataLen = toInt(header, headerIndex);
		headerIndex += 4;

		// Does the header need to be flipped?
		// (Checking for a daemon server endian-mismatch)
		/////////////////////////////////////////////////
		boolean daemonEndianMismatch;
		if(sameEndian(serviceType) == false)
		{
			// Flip them.
			/////////////
			serviceType = flip(serviceType);
			numGroups = flip(numGroups);
			dataLen = flip(dataLen);
		
			// The daemon endian-mismatch.
			//////////////////////////////
			daemonEndianMismatch = true;
		}
		else
		{
			// The daemon endian-mismatch.
			//////////////////////////////
			daemonEndianMismatch = false;
		}
			
		// Validate numGroups and dataLen

		if ( (numGroups < 0) || (dataLen < 0) ) 
		{
			// drop message
			throw new SpreadException("Illegal Message: Message Dropped");
		}

		// An endian mismatch.
		//////////////////////
		boolean endianMismatch;

		// The type.
		////////////
		short type;
		
		// Is this a regular message?
		/////////////////////////////
		if( SpreadMessage.isRegular(serviceType) || SpreadMessage.isReject(serviceType) )
		{
			// Does the hint need to be flipped?
			// (Checking for a sender endian-mismatch)
			//////////////////////////////////////////
			if(sameEndian(hint) == false)
			{
				hint = flip(hint);
				endianMismatch = true;
			}
			else
			{
				endianMismatch = false;
			}
			
			// Get the type from the hint.
			//////////////////////////////
			hint = clearEndian(hint);
			hint >>= 8;
			hint &= 0x0000FFFF;
			type = (short)hint;
		}
		else
		{
			// This is not a regular message.
			/////////////////////////////////
			type = -1;
			endianMismatch = false;
		}

                if( SpreadMessage.isReject(serviceType) )
                {
                        // Read in the old type and or with reject type field.
                        byte oldtypeBuffer[] = new byte[4];
			try 
		        {
			    readBytesFromSocketListener( oldtypeBuffer, "message reject type" );
			}
			catch(InterruptedIOException e) {
			    throw e;
			} 
			catch(IOException e) {
			    throw new SpreadException("read(): " + e);
			}

                        int oldType = toInt(oldtypeBuffer, 0);
                        if ( sameEndian(serviceType) == false )
                            oldType = flip(oldType);

                        serviceType = (SpreadMessage.REJECT_MESS | oldType);
                }

		// Read in the group names.
		///////////////////////////
		byte buffer[] = new byte[numGroups * MAX_GROUP_NAME];
		try 
		{
		    readBytesFromSocketListener( buffer, "message group name" );
		}
		catch(InterruptedIOException e) {
		    throw e;
		} 
		catch(IOException e) {
		    throw new SpreadException("read(): " + e);
		}

		// Clear the endian type.
		/////////////////////////
		serviceType = clearEndian(serviceType);

		// Get the groups from the buffer.
		//////////////////////////////////
		Vector groups = new Vector(numGroups);
		for(int bufferIndex = 0 ; bufferIndex < buffer.length ; bufferIndex += MAX_GROUP_NAME)
		{
			// Translate the name into a group and add it to the vector.
			////////////////////////////////////////////////////////////
			groups.addElement(toGroup(buffer, bufferIndex));
		}

		// Read in the data.
		////////////////////
		byte data[] = new byte[dataLen];
		try 
		{
		    readBytesFromSocketListener( data, "message body" );
		}
		catch(InterruptedIOException e) {
		    throw e;
		} 
		catch(IOException e) {
		    throw new SpreadException("read(): " + e);
		}

		// Is it a membership message?
		//////////////////////////////
		MembershipInfo membershipInfo;
		if(SpreadMessage.isMembership(serviceType))
		{
			// Create a membership info object.
			///////////////////////////////////
			membershipInfo = new MembershipInfo(this, serviceType, groups, sender, data, daemonEndianMismatch);
			
			// Is it a regular membership message?
			//////////////////////////////////////
			if(membershipInfo.isRegularMembership())
			{
				// Find which of these groups is the local connection.
				//////////////////////////////////////////////////////
				type = (short)groups.indexOf(group);
			}
		}
		else
		{
			// There's no membership info.
			//////////////////////////////
			membershipInfo = null;
		}

		// Create the message.
		//////////////////////
		return new SpreadMessage(serviceType, groups, sender, data, type, endianMismatch, membershipInfo);
	}
	
	// Receives numMessages new messages.
	/////////////////////////////////////
	/**
	 * Receives <code>numMessages</code> messages on the connection and returns them in an array.
	 * If there are not <code>numMessages</code> messages waiting, the call will block until there are
	 * enough messages available.
	 * 
	 * @param  numMessages  the number of messages to receive
	 * @return an array of messages
	 * @throws  SpreadException  if there is no connection or if there is any error reading the messages
	 */
        public SpreadMessage[] receive(int numMessages) throws SpreadException, InterruptedIOException
	{
		// Allocate the messages array.
		///////////////////////////////
		SpreadMessage[] messages = new SpreadMessage[numMessages];
		synchronized(rsynchro) {
		        // Check if there are any listeners.
		        ////////////////////////////////////
		        if ((basicListeners.isEmpty() == false) || (advancedListeners.isEmpty() == false))
			{
			    // Get out of here.
			    ///////////////////
			    throw new SpreadException("Tried to receive while there are listeners");
			}

			// Receive the messages.
			////////////////////////
			for(int i = 0 ; i < numMessages ; i++)
			{
				messages[i] = internal_receive();
			}
		
		}
		// Return the array.
		////////////////////
		return messages;
	}
	
	// Returns true if there are messages waiting.
	//////////////////////////////////////////////
	/**
	 * Returns true if there are any messages waiting on this connection.
	 * 
	 * @return true if there is at least one message that can be received
	 * @throws  SpreadException  if there is no connection or if there is an error checking for messages
	 */
	public boolean poll() throws SpreadException
	{
		// Check if we're connected.
		////////////////////////////
		if(connected == false)
		{
			throw new SpreadException("Not connected.");
		}
		
		// Check if there is anything waiting.
		//////////////////////////////////////
		try
		{
			if(socketInput.available() == 0)
			{
				// There's nothing to read.
				///////////////////////////
				return false;
			}
		}
		catch(IOException e)
		{
			throw new SpreadException("available(): " + e);
		}
		
		// There's something to read.
		/////////////////////////////
		return true;
	}
	
	// Private function for starting a listener thread.
	///////////////////////////////////////////////////
	private void startListener()
	{
		// Get a new thread.
		////////////////////
		listener = new Listener(this);
		
		// Start it.
		////////////
		listener.start();
	}
	
	// Adds a new basic message listener.
	/////////////////////////////////////
	/**
	 * Adds the BasicMessageListener to this connection.  If there are no other listeners, this call will
	 * start a thread to listen for new messages.  From the time this function is called until
	 * this listener is removed, {@link BasicMessageListener#messageReceived(SpreadMessage)} will
	 * be called every time a message is received.
	 * 
	 * @param  listener  a BasicMessageListener to add to this connection
	 * @see  SpreadConnection#remove(BasicMessageListener)
	 */
	public void add(BasicMessageListener listener)
	{
	synchronized(listenersynchro) {
		// Are we in a listener callback?
		/////////////////////////////////
		if(callingListeners)
		{
			// Add it to the command buffer.
			////////////////////////////////
			listenerBuffer.addElement(BUFFER_ADD_BASIC);
			listenerBuffer.addElement(listener);
			
			// Don't need to do anything else.
			//////////////////////////////////
			return;
		}
		// Add the listener.
		////////////////////
		basicListeners.addElement(listener);
		
		// Check if we're connected.
		////////////////////////////
		if(connected == true)
		{
			// Check if the thread is running.
			//////////////////////////////////
			if(this.listener == null)
			{
				// Start the thread.
				////////////////////
				startListener();
			}
		}
	}
	}
	
	// Adds a new advanced message listener.
	////////////////////////////////////////
	/**
	 * Adds the AdvancedMessageListener to this connection.  If there are no other listeners, this call will
	 * start a thread to listen for new messages.  From the time this function is called until
	 * this listener is removed, {@link AdvancedMessageListener#regularMessageReceived(SpreadMessage)} will
	 * be called every time a regular message is received, and 
	 * {@link AdvancedMessageListener#membershipMessageReceived(SpreadMessage)} will be called every time
	 * a membership message is received.
	 * 
	 * @param  listener an AdvancedMessageListener to add to this connection
	 * @see  SpreadConnection#remove(AdvancedMessageListener)
	 */
	public void add(AdvancedMessageListener listener)
	{
	synchronized (listenersynchro) {
		// Are we in a listener callback?
		/////////////////////////////////
		if(callingListeners)
		{
			// Add it to the command buffer.
			////////////////////////////////
			listenerBuffer.addElement(BUFFER_ADD_ADVANCED);
			listenerBuffer.addElement(listener);
			
			// Don't need to do anything else.
			//////////////////////////////////
			return;
		}

		// Add the listener.
		////////////////////
		advancedListeners.addElement(listener);
		
		// Check if we're connected.
		////////////////////////////
		if(connected == true)
		{
			// Check if the thread is running.
			//////////////////////////////////
			if(this.listener == null)
			{
				// Start the thread.
				////////////////////
				startListener();
			}
		}
	}
	}
	
	// Stops the listener thread.
	/////////////////////////////
	private void stopListener()
	{
		// Set the signal.
		//////////////////
		listener.signal = true;
		
		// Wait for the thread to die, to avoid inconsistencies.
		////////////////////////////////////////////////////////
		if ( listener != null && ! listener.equals(Thread.currentThread()) ) 
		{
		    try {
			listener.join();
		    } 
		    catch ( InterruptedException e ) {
			// Ignore
		    }
		}
		// Clear the variable.
		//////////////////////
		listener = null;
	}
	
	// Removes a basic message listener.
	////////////////////////////////////
	/**
	 * Removes the BasicMessageListener from this connection.  If this is the only listener on this
	 * connection, the listener thread will be stopped.
	 * 
	 * @param  listener  the listener to remove
	 * @see  SpreadConnection#add(BasicMessageListener)
	 */
        public void remove(BasicMessageListener listener)
	{
	synchronized (listenersynchro) {
		// Are we in a listener callback?
		/////////////////////////////////
		if(callingListeners)
		{
			// Add it to the command buffer.
			////////////////////////////////
			listenerBuffer.addElement(BUFFER_REMOVE_BASIC);
			listenerBuffer.addElement(listener);
			
			// Don't need to do anything else.
			//////////////////////////////////
			return;
		}
		
		// Remove the listener.
		///////////////////////
		basicListeners.removeElement(listener);
		
		// Check if we're connected.
		////////////////////////////
		if (connected == true)
		{
			// Check if there are any more listeners.
			/////////////////////////////////////////
			if((basicListeners.size() == 0) && (advancedListeners.size() == 0))
			{
				// Stop the listener thread.
				////////////////////////////
				stopListener();
			}
		}
	}
	}

	// Removes an advanced message listener.
	////////////////////////////////////////
	/**
	 * Removes the AdvancedMessageListener from this connection.  If this is the only listener on this
	 * connection, the listener thread will be stopped.
	 * 
	 * @param  listener  the listener to remove
	 * @see SpreadConnection#add(AdvancedMessageListener)
	 */
        public void remove(AdvancedMessageListener listener)
	{
	synchronized (listenersynchro) {
		// Are we in a listener callback?
		/////////////////////////////////
		if(callingListeners)
		{
			// Add it to the command buffer.
			////////////////////////////////
			listenerBuffer.addElement(BUFFER_REMOVE_ADVANCED);
			listenerBuffer.addElement(listener);
			
			// Don't need to do anything else.
			//////////////////////////////////
			return;
		}
		
		// Remove the listener.
		///////////////////////
		advancedListeners.removeElement(listener);
		
		// Check if we're connected.
		////////////////////////////
		if (connected == true)
		{
			// Check if there are any more listeners.
			/////////////////////////////////////////
			if((basicListeners.size() == 0) && (advancedListeners.size() == 0))
			{
				// Stop the listener thread.
				////////////////////////////
				stopListener();
			}
		}
	}
	}
	
	// This is the thread used to handle listener interfaces.
	/////////////////////////////////////////////////////////
	private class Listener extends Thread
	{
		// The connection this thread is listening to.
		//////////////////////////////////////////////
		private SpreadConnection connection;
		
		// If true, the connection wants the thread to stop.
		////////////////////////////////////////////////////
		protected volatile boolean signal;
		
		// The constructor.
		///////////////////
		public Listener(SpreadConnection connection)
		{
			// Store local variables.
			/////////////////////////
			this.connection = connection;
			this.signal = false;
			
			// Be a daemon.
			///////////////
			this.setDaemon(true);

		}
		
		// The thread's entry point.
		////////////////////////////
		public void run()
		{
			// An incoming message.
			///////////////////////
			SpreadMessage message;
			
			// A basic listener.
			////////////////////
			BasicMessageListener basicListener;
			
			// An advanced listener.
			////////////////////////
			AdvancedMessageListener advancedListener;
			
			// A buffered command.
			//////////////////////
			Object command;
			
			int previous_socket_timeout = 100;

			try
			{
			        try {
				    previous_socket_timeout = connection.socket.getSoTimeout();
				    connection.socket.setSoTimeout(100);
				}
				catch( SocketException e ) {
				    // just ignore for now 
				    System.out.println("socket error setting timeout" + e.toString() );
				}
				while(true)
				{
				    // Do they want us to stop?
				    ///////////////////////////
				    if(signal == true)
				    {
					// We're done.
					//////////////
					System.out.println("LISTENER: told to exit so returning");
					try {
					    connection.socket.setSoTimeout(previous_socket_timeout);
					}
					catch( SocketException e ) {
					    // just ignore for now 
					    System.out.println("socket error setting timeout" + e.toString() );
					}
					
					return;
				    }
				    // Get a lock on the connection.
				    ////////////////////////////////
				    synchronized(connection)
				    {
							
						// Get a message.
						// WE WILL BLOCK HERE UNTIL DATA IS AVAILABLE
						// or 100 MS expires
						/////////////////
    					        message = null;
						try {
					
						    synchronized(rsynchro) {
							boolean keep_going = true;
							while( keep_going == true )
							{
							    keep_going = false;
							    try {
								message = connection.internal_receive();
							    } 
							    catch( InterruptedIOException e) {
								if ( signal  == true )
								{
								    throw( e );
								}
								keep_going = true;
							    }
							}
						    }
						    // Calling listeners.
						    /////////////////////
						    callingListeners = true;
						
						    // Tell all the basic listeners.
						    ////////////////////////////////
						    for(int i = 0 ; i < basicListeners.size() ; i++)
						    {
							    // Get the listener.
							    ////////////////////
							    basicListener = (BasicMessageListener)basicListeners.elementAt(i);
									
							    // Tell it.
							    ///////////
							    basicListener.messageReceived(message);
						    }

						    // Tell all the advanced listeners.
						    ///////////////////////////////////
						    for(int i = 0 ; i < advancedListeners.size() ; i++)
						    {
							    // Get the listener.
							    ////////////////////
							    advancedListener = (AdvancedMessageListener)advancedListeners.elementAt(i);
									
							    // What type of message is it?
							    //////////////////////////////
							    if(message.isRegular())
							    {
								    // Tell it.
								    ///////////
								    advancedListener.regularMessageReceived(message);
							    }
							    else
							    {
								    // Tell it.
								    ///////////
								    advancedListener.membershipMessageReceived(message);
							    }
						    }
							
						    // Done calling listeners.
						    //////////////////////////
						    callingListeners = false;
							

						} catch( InterruptedIOException e) {
						    /// Ignore
						}
						// Execute buffered commands.
						/////////////////////////////
						while(listenerBuffer.isEmpty() == false)
						{
							// Get the first command.
							/////////////////////////
							command = listenerBuffer.firstElement();
								
							// Remove it from the list.
							///////////////////////////
							listenerBuffer.removeElementAt(0);
								
								// Check what type of command it is.
								////////////////////////////////////
							if(command == BUFFER_DISCONNECT)
							{
								// Disconnect.
								//////////////
								connection.disconnect();
									
								// Don't execute any more commands.
								///////////////////////////////////
								listenerBuffer.removeAllElements();
							}
							else if(command == BUFFER_ADD_BASIC)
							{
								// Get the listener.
								////////////////////
								basicListener = (BasicMessageListener)listenerBuffer.firstElement();
									
								// Add it.
								//////////
								connection.add(basicListener);
								// Remove the listener from the Vector.
								///////////////////////////////////////
								listenerBuffer.removeElementAt(0);

							}
							else if(command == BUFFER_ADD_ADVANCED)
							{
								// Get the listener.
								////////////////////
								advancedListener = (AdvancedMessageListener)listenerBuffer.firstElement();
									
								// Add it.
								//////////
								connection.add(advancedListener);
								// Remove the listener from the Vector.
								///////////////////////////////////////
								listenerBuffer.removeElementAt(0);

							}
							else if(command == BUFFER_REMOVE_BASIC)
							{
								// Get the listener.
								////////////////////
								basicListener = (BasicMessageListener)listenerBuffer.firstElement();
									
								// Remove it.
								/////////////
								basicListeners.removeElement(basicListener);

								// Remove the listener from the Vector.
								///////////////////////////////////////
								listenerBuffer.removeElementAt(0);
							}
							else if(command == BUFFER_REMOVE_ADVANCED)
							{
								// Get the listener.
								////////////////////
								advancedListener = (AdvancedMessageListener)listenerBuffer.firstElement();
									
								// Remove it.
								/////////////
								advancedListeners.removeElement(advancedListener);
								// Remove the listener from the Vector.
								///////////////////////////////////////
								listenerBuffer.removeElementAt(0);
							}
						}
						// Check if there are any more listeners, if not, then quit listener thread. 
						/////////////////////////////////////////
						if((basicListeners.size() == 0) && (advancedListeners.size() == 0))
						{
						    // Terminate and return from this listener thread.
						    ////////////////////////////
						    System.out.println("LISTENER: no current listeners registered so terminate Listener thread.");
						    try {
							connection.socket.setSoTimeout(previous_socket_timeout);
						    }
						    catch( SocketException e ) {
							// just ignore for now 
							System.out.println("socket error setting timeout" + e.toString() );
						    }
						    
						    listener = null;
						    // Exit from running Listener Thread
						    return;
						}

					}

					// There are no messages waiting, take a break.
					///////////////////////////////////////////////
					yield();
				}
			}
			catch(SpreadException e)
			{
				// Nothing to do but ignore it.
				///////////////////////////////
			    System.out.println("SpreadException: " + e.toString() );
			}
		}
	}

	// Sends the message.
	/////////////////////
	/**
	 * Multicasts a message.  The message will be sent to all the groups specified in
	 * the message.
	 * 
	 * @param  message  the message to multicast
	 * @throws  SpreadException  if there is no connection or if there is any error sending the message
	 */
	public void multicast(SpreadMessage message) throws SpreadException
	{
		// Check if we're connected.
		////////////////////////////
		if(connected == false)
		{
			throw new SpreadException("Not connected.");
		}
	
		// The groups this message is going to.
		///////////////////////////////////////
		SpreadGroup groups[] = message.getGroups();
		
		// The message data.
		////////////////////
		byte data[] = message.getData();
		
		// Calculate the total number of bytes.
		///////////////////////////////////////
		int numBytes = 16;  // serviceType, numGroups, type/hint, dataLen
		numBytes += MAX_GROUP_NAME;  // private group
		numBytes += (MAX_GROUP_NAME * groups.length);  // groups
		
		if (numBytes + data.length > MAX_MESSAGE_LENGTH )
		{
		    throw new SpreadException("Message is too long for a Spread Message");
		}
		// Allocate the send buffer.
		////////////////////////////
		byte buffer[] = new byte[numBytes];
		int bufferIndex = 0;
		
		// The service type.
		////////////////////
		toBytes(message.getServiceType(), buffer, bufferIndex);
		bufferIndex += 4;

		// The private group.
		/////////////////////
		toBytes(group, buffer, bufferIndex);
		bufferIndex += MAX_GROUP_NAME;
		
		// The number of groups.
		////////////////////////
		toBytes(groups.length, buffer, bufferIndex);
		bufferIndex += 4;
		
		// The service type and hint.
		/////////////////////////////
		toBytes(((int)message.getType() << 8) & 0x00FFFF00, buffer, bufferIndex);
		bufferIndex += 4;
		
		// The data length.
		///////////////////
		toBytes(data.length, buffer, bufferIndex);
		bufferIndex += 4;
		
		// The group names.
		///////////////////
		for(int i = 0 ; i < groups.length ; i++)
		{
			toBytes(groups[i], buffer, bufferIndex);
			bufferIndex += MAX_GROUP_NAME;
		}
		
		// Send it.
		///////////
	synchronized (wsynchro) {
		try
		{
			socketOutput.write(buffer);
			socketOutput.write(data);
		}
		catch(IOException e)
		{
			throw new SpreadException("write(): " + e.toString());
		}
	}
	}
	
	// Sends the array of messages.
	///////////////////////////////
	/**
	 * Multicasts an array of messages.  Each message will be sent to all the groups specified in
	 * the message.
	 * 
	 * @param  messages  the messages to multicast
	 * @throws  SpreadException  if there is no connection or if there is any error sending the messages
	 */
	public void multicast(SpreadMessage messages[]) throws SpreadException
	{
		// Go through the array.
		////////////////////////
		for(int i = 0 ; i < messages.length ; i++)
		{
			// Send this message.
			/////////////////////
			multicast(messages[i]);
		}
	}

	// Returns the connected state of the connection.
	/////////////////////////////////////////////////
	/**
	  * Returns the current connection state.
	  */

	public boolean isConnected()
	{
		return connected;
	}
}
