arclinktool is a simple client for requesting information to an ArcLink servers. 
It is possible to send all types of requests directly to a specified server, check
status, download or purge requests. You can request routing information but the 
tool itself does not resolve the routing automatically before sending the requests.

An ArcLink request has the following syntax:

.. code-block:: sh

   REQUEST <TYPE> <ATTRIBUTES>
   <TimeSpam line>
   <TimeSpam line>
   <TimeSpam line>
   END

Where TYPE is one of WAVEFORM, INVENTORY, RESPONSE, ROUTING, QC. 

And a Timespam line contains: 

.. code-block:: sh

   YYYY,MM,DD,HH,MM,SS YYYY,MM,DD,HH,MM,SS <Network Code> [<Station Code> [<Channel Code> [Location Code]]]

where the network/station/channel/location codes can contains wildcards or can 
be empty depending on the request type.

A complete description of the request format for the ArcLink protocol can be found at the 
:ref:`ArcLink protocol <arclink_protocol>` page.

Examples
========

Example of a request file named "req.txt":

.. code-block:: sh

   REQUEST WAVEFORM format=MSEED
   2008,06,04,06,00,00 2008,06,04,06,10,00 GE CART BHZ .
   2008,06,04,06,00,00 2008,06,04,06,10,00 GE MAHO BHZ .
   END


Submit the request to the ArcLink server.

.. code-block:: sh

   > arclinktool -u andres -r req.txt localhost:18001
   Connected to ArcLink v0.4 (2006.276) at GITEWS
   Request successfully submitted
   Request ID: 91

Check the status.

.. code-block:: sh

   > arclinktool -u andres -s 91 localhost:18001
   Connected to ArcLink v0.4 (2006.276) at GITEWS
   Request ID: 91, Type: WAVEFORM, Args: format=MSEED
   Status: READY, Size: 37376, Info:
   Volume ID: local, Status: OK, Size: 37376, Info:
   Request: 2008,06,04,06,00,00 2008,06,04,06,10,00 GE CART BHZ .
   Status: OK, Size: 18432, Info:
   Request: 2008,06,04,06,00,00 2008,06,04,06,10,00 GE MAHO BHZ .
   Status: OK, Size: 18944, Info:

Download data.

.. code-block:: sh

   > arclinktool -u andres -d 91 -o data.mseed localhost:18001
   Connected to ArcLink v0.4 (2006.276) at GITEWS
   Download successful

Delete request from server.

.. code-block:: sh

   > arclinktool -u andres -p 91 localhost:18001
   Connected to ArcLink v0.4 (2006.276) at GITEWS
   Product successfully deleted
