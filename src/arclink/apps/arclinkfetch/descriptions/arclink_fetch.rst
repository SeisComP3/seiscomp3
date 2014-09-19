arclink_fetch is a more sophisticated client that can be used to get data with
a single command. It performs routing automatically. When you install SeisComP3
it is automatically installed on your computer, but it may also be installed as
a standalone package for users wishing only to request data from an ArcLink
server.

Input File Format
=================

ArcLink Fetch  program supports  two different input  formats for  the request
file.  It  supports  the traditional  BREQ  FAST  format,  and its own  native
format. Both formats  contains the same information and  they differ slightly.

The program has the ability to read the request lines from files, or from the 
standard input (when no request file is supplied).

Native Format:
--------------

The native format has the following format: 

.. code-block:: sh

   YYYY,MM,DD,HH,MM,SS YYYY,MM,DD,HH,MM,SS Network Station Channel [Location]

the Channel, Station and Location, can contains wildcards (*) and the Location
field is optional. For matching all locations please use the '*' symbol, if 
empty it assumes that only empty locations are being requested.

Example:

.. code-block:: sh

   2010,02,18,12,00,00 2010,02,18,12,10,00 GE WLF BH*
   2010,02,18,12,00,00 2010,02,18,12,10,00 GE VSU BH* 00

BREQ FAST Format:
-----------------

The  BREQ FAST  format is  a  standard format  used on  seismology to  request
data. Each header line start with '.' and the request lines have the following
format:

.. code-block:: sh

   Station Network {Time Start} {Time End} {Number of Channels} N x Channels Location

Time Specification should have the following format: YYYY MM DD HH MM SS.TTTT

Please read more about the BREQ FAST format at:

http://www.iris.edu/manuals/breq_fast.htm

ArcLink Password File (for decryption)
======================================

In this file (default: dcidpasswords.txt) you can store your private passwords
given by different  data centers. Each data center  that you request encrypted
data will send you a different password.

The format of the  file is really simple: just the data  center ID followed by
the password that you received. One  data center ID and password per line. Any
empty lines or lines starting with # are ignored.

Example:

.. code-block:: sh

   gfz password1
   odc password2
   ipgp password3

The data  center ID and password  can be found on the automatically generated
e-mail that  you received from each  data center. (You will  only receive this
email if  you have been  authorized to download  encrypted data, and  you have
tried to download it.)

MiniSeed reblocking
===================

The mseed4k data format (-k mseed4k) enables a client side re-blocking of the
received MiniSeed records from the server to a default block size of 4096. This
is especially important when you are downloading data from two different
networks that were archived with different block sizes and would like to
receive a uniform (in terms of block size) MiniSeed file.

Some programs have problems to proper display miniseed packages with different
block sizes on the same file. This issue can be avoided by instead of requesting
MiniSeed data directly (-k mseed), just request re-blocked MiniSeed (-k mseed4k).

Rebuilding SEED volume
======================

If -g option is used in combination with the fseed data format (-k fseed), then
instead of full SEED, inventory and Mini-SEED is requested; full SEED volume is
created from inventory and Mini-SEED data locally. In this case, waveform data
from multiple data centers can be merged into one SEED volume.

Examples
========

Example of a request file named "req.txt":

.. code-block:: sh

   2010,02,18,12,00,00 2010,02,18,12,10,00 GE WLF BH*
   2010,02,18,12,00,00 2010,02,18,12,10,00 GE VSU BH*

Submit the request in req.txt to the ArcLink server on st55, and download full
SEED data to req.mseed.

.. code-block:: sh

   > arclink_fetch -a st55:18002 -k fseed -g -u andres@gfz-potsdam.de \
                   -o req.mseed -v req.txt
   requesting inventory from st55:18002
   requesting routing from st55:18002
   launching request thread (st55:18002)
   st55:18002: request 41 ready
   launching request thread (st14:18002)
   st14:18002: request 39 ready
   the following data requests were sent:
   GFTEST55
   Request ID: 41, Label: , Type: WAVEFORM, Args: compression=bzip2 format=MSEED
   Status: READY, Size: 37137, Info:
     Volume ID: GFTEST, Status: OK, Size: 37137, Info:
       Request: 2010,2,18,12,0,0 2010,2,18,12,10,0 GE WLF BHN .
       Status: OK, Size: 15360, Info:
       Request: 2010,2,18,12,0,0 2010,2,18,12,10,0 GE WLF BHE .
       Status: OK, Size: 15360, Info:
       Request: 2010,2,18,12,0,0 2010,2,18,12,10,0 GE WLF BHZ .
       Status: OK, Size: 15872, Info:
       Request: 2010,2,18,12,0,0 2010,2,18,12,10,0 GE VSU BHN .
       Status: NODATA, Size: 0, Info:
       Request: 2010,2,18,12,0,0 2010,2,18,12,10,0 GE VSU BHZ .
       Status: NODATA, Size: 0, Info:
       Request: 2010,2,18,12,0,0 2010,2,18,12,10,0 GE VSU BHE .
       Status: NODATA, Size: 0, Info:
   GFTEST
   Request ID: 39, Label: , Type: WAVEFORM, Args: compression=bzip2 format=MSEED
   Status: READY, Size: 46269, Info:
     Volume ID: GFTEST, Status: OK, Size: 46269, Info:
       Request: 2010,2,18,12,0,0 2010,2,18,12,10,0 GE VSU BHN .
       Status: OK, Size: 17408, Info:
       Request: 2010,2,18,12,0,0 2010,2,18,12,10,0 GE VSU BHZ .
       Status: OK, Size: 16896, Info:
       Request: 2010,2,18,12,0,0 2010,2,18,12,10,0 GE VSU BHE .
       Status: OK, Size: 17408, Info:
   rebuilding SEED volume

.. note:: Part of the request was routed to secondary server after the primary
   server returned NODATA.

This client is intended to connect to an ArcLink server as implemented by the :ref:`Seiscomp3 Arclink Server
<arclink>` or to any other server implementing the :ref:`ArcLink protocol <arclink_protocol>`.
