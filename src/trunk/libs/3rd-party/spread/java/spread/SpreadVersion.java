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
 * A SpreadVersion object is used to get the version of spread that is being used.
 */
public class SpreadVersion
{
	// The major version.
	/////////////////////
	private int majorVersion = 3;
	
	// The minor version.
	/////////////////////
	private int minorVersion = 16;

	// The patch version.
	/////////////////////
	private int patchVersion = 1;
	
	// Get the spread version.
	//////////////////////////
	/**
	 * Returns the spread version as a float.  The float is of
	 * the form A.B where A is the major version and B is the minor version.
	 * The patch version is not returned.
	 * 
	 * @return  the spread version
	*/
	public float getVersion()
	{
		return (float)((float)majorVersion + ((float)minorVersion / 100.0));
	}
	
	// Get the major version.
	/////////////////////////
	/**
	 * Returns the spread major version as an int.
	 * 
	 * @return  the spread major version
	 */
	public int getMajorVersion()
	{
		return majorVersion;
	}
	
	// Get the minor version.
	/////////////////////////
	/** Returns the spread minor version as an int.
	 * 
	 * @return  the spread minor version
	 */
	public int getMinorVersion()
	{
		return minorVersion;
	}

	// Get the patch version.
	/////////////////////////
	/** Returns the spread patch version as an int.
	 * 
	 * @return  the spread patch version
	 */
	public int getPatchVersion()
	{
		return patchVersion;
	}

	// Convert to a string.
	///////////////////////
	/**
	 * Returns the spread version in string form.  The string is of
	 * the form A.BB.CC where A is the major version and BB is the minor version
	 * and CC is the patch version.
	 */
	public String toString()
	{
		return new String(majorVersion + "." + (minorVersion / 10) + "" + (minorVersion % 10) + "." + 
				  (patchVersion / 10) + (patchVersion % 10));
	}
}
