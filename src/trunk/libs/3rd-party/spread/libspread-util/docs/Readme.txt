SPREAD: A Reliable Multicast and Group Communication Toolkit
-----------------------------------------------------------

/===========================================================================\
| The Spread Group Communication Toolkit Utility Library                    |
| Copyright (c) 1993-2012 Spread Concepts LLC                               |
| All rights reserved.                                                      |
|                                                                           |
| The Spread package is licensed under the Spread Open-Source License.      |
| You may only use this software in compliance with the License.            |
| A copy of the license can be found at http://www.spread.org/license       |
|                                                                           |
| This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF     |
| ANY KIND, either express or implied.                                      |
|                                                                           |
| Spread is developed at Spread Concepts LLC with the support of:	    |
|    The Distributed Systems and Networks Lab, Johns Hopkins University     |
|    The Experimental Networked Systems Lab, George Washington University   |
|                                                                           |
| Creators:                                                                 |
|    Yair Amir             yairamir@cs.jhu.edu                              |
|    Michal Miskin-Amir    michal@spreadconcepts.com                        |
|    Jonathan Stanton      jstanton@gwu.edu                                 |
|    John Schultz	   jschultz@spreadconcepts.com			    |
|                                                                           |
| Major Contributors:                                                       |
|    Ryan Caudy           rcaudy@gmail.com - contribution to process groups.|
|    Claudiu Danilov	  claudiu@acm.org - scalable, wide-area support.    |
|    Cristina Nita-Rotaru crisn@cs.purdue.edu - GC security.                |
|    Theo Schlossnagle    jesus@omniti.com - Perl, autoconf, old skiplist.  |
|    Dan Schoenblum       dansch@dsn.jhu.edu - Java interface.              |
|                                                                           |
| Contributors:                                                             |
|    Ben Laurie	       ben@algroup.co.uk - FreeBSD port and warning fixes   |
|    Daniel Rall       dlr@finemaltcoding.com - Java & networking fixes,    |
|                                               configuration improvements  |
|    Marc Zyngier                        - Windows fixes                    |
|    Jacob Green       jgreen@spreadconcepts.com - Windows support          |
|                                                                           |
| Special thanks to the following for discussions and ideas:                |
|    Ken Birman, Danny Dolev, Jacob Green, Mike Goodrich, Ben Laurie,       |
|    David Shaw, Gene Tsudik, Robbert VanRenesse.                           |
|                                                                           |
| Partial funding provided by the Defense Advanced Research Projects Agency |
| (DARPA) and The National Security Agency (NSA) 2000-2004. The Spread      |
| toolkit is not necessarily endorsed by DARPA or the NSA.                  |
|                                                                           |
| WWW    : http://www.spread.org  and  http://www.spreadconcepts.com        |
| Contact: info@spreadconcepts.com                                          |
|                                                                           |
| Version 4.2.0  built 05/March/2012                                        |
\===========================================================================/

March 05, 2012 Ver 4.2.0 
-----------------------------
This is the first release of the Spread Utility Library which consists of 
software contained in the Spread Group Communication Toolkit in previous 
releases and is now packaged separately to make it easier to use in other
projects. 

It contains the following code subsystems from the Spread codebase:

Alarm
Data_Link
Memory
Events

This standalone package has a standard autoconf build system for Unix like 
systems and also supports Windows builds although it does not currently
provide Windos Make files or VC project files. 

The detailed changes in this first release (from the internal code in Spread)
can be found in the Changelog file. 

STEPS TO CONVERT APPLICATION USING PREVIOUS CODE TO NEW LIBRARY:
----------------------------------------------------------------

For an application that previously used alarm.c, data_link.c, events.c or memory.c, 
these are the suggested changes to make the application compatible with using the
same APIs via this new library. 

1) Fix build:
   - Change configure.in in application to call subconfigure for libspread-util package
   - Change top level Makefile.in to add libspread-util as a subdirectory to build (and install if using shared libraries)

2) Integrate with svn to allow customization
   - Import libspread-util release into vendor branch. Then merge into subdirectory in main package. 
   This allows local changes to be made to application's version of the library. 

3) Add application specific memory types to libspread-util/include/spu_objects_local.h

4) Fix any usage of the previous APIs that have changed in the new release. These include:
   - Fix any callers of Alarm_enable_precise_timestamps() to use new function for high_res alarm formats. 
   - Fix all callers of Mem_init_object() to add the 'obj_name' parameter to the calls. 
   - Change all #include of the headers to use the new header names of spu_*
   - Compare local copies of memory.h objects.h alarm.h and make sure the defined types match those in the standard library, or modify the objects_local.h and alarm_types.h files to match the app. 
   - Change all use of SPLOG_PRINT_NODATE to new feature flag SPLOG_NODATE along with correct main type like SPLOG_PRINT.
   - Consider changing all application use of "spu_objects.h" to "spu_objects_local.h" as most applications do not need anything from objects.h.



SOURCE INSTALL:
---------------

The source install uses autoconf and make to support many Unix and unix-like 
platforms as well as Windows.

From the directory where you unpacked the Spread source distribution do 
the following:

1) Run "./configure"

2) Run "make"

3) Run "make install" as a user with rights to write to the selected
installation location. (/usr/local/{bin,man,sbin,include,lib} by default)

OVERVIEW:
---------

The libspread-util library provides APIs to suppport logging events, 
sending and receiving network packets in UDP or TCP, monitoring and
responding to both time based and file-descriptor based events, and
managing memory when you need memory pools of fixed size structures. 

CONTACT:
--------

If you have any questions please feel free to contact:

   spread@spread.org
