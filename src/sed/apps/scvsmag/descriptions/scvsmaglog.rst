Part of the :ref:`VS` package.

scvsmaglog is part of a new SeisComp3 implementation of the
`Virtual Seismologist <http://www.seismo.ethz.ch/research/vs>`_
(VS) Earthquake Early Warning algorithm (Cua, 2005; Cua and Heaton, 2007) released
under the `'SED Public License for SeisComP3 Contributions'
<http://www.seismo.ethz.ch/static/seiscomp_contrib/license.txt>`_. It requires 
the Python package `dateutil <https://pypi.python.org/pypi/python-dateutil>`_ to
be installed.

It logs the VS magnitude messages received from *scvsmag* and, once an event
has timed out, generates report files. These report files are saved to disk and
can also be sent via email.

It also implements an ActiveMQ interface which provides the possibility to send
messages to the UserDisplay, a real-time EEW user interface developed by `Caltech
<http://www.eew.caltech.edu/research/userdisplay.html>`_.
While the UserDisplay is currently not openly available, the message stream from
this interface can be used. The xml scheme and set-up of an ActiveMQ broker
necessary to receive the messages is briefly described in section :ref:`ref_VS_UDI`.
Note that the ActiveMQ interface requires the installation of the Python 
package `stompy <https://pypi.python.org/pypi/stompy>`_. 


Reports
=======

Below is an example of the first few lines of a report file:

.. code-block:: sh

   Mag.|Lat.  |Lon.  |tdiff |Depth |creation time (UTC)      |origin time (UTC)        |likeh.|#st.(org.) |#st.(mag.)
   ------------------------------------------------------------------------------------------------------------------
   3.42| 47.15|  8.52| 12.73| 25.32|2012-02-11T22:45:39.0000Z|2012-02-11T22:45:26.2729Z|  0.99|          6|         6
   3.43| 47.15|  8.52| 13.73| 25.32|2012-02-11T22:45:40.0000Z|2012-02-11T22:45:26.2729Z|  0.99|          6|         6
   3.56| 47.15|  8.54| 14.70| 25.73|2012-02-11T22:45:41.0000Z|2012-02-11T22:45:26.3032Z|  0.99|         10|        10
   3.64| 47.16|  8.54| 15.58| 24.32|2012-02-11T22:45:42.0000Z|2012-02-11T22:45:26.4178Z|  0.99|         12|        12
   3.54| 47.16|  8.53| 16.45| 22.40|2012-02-11T22:45:43.0000Z|2012-02-11T22:45:26.5547Z|  0.99|         14|        14
   3.67| 47.15|  8.54| 17.29| 20.40|2012-02-11T22:45:44.0000Z|2012-02-11T22:45:26.7142Z|  0.99|         16|        16
   3.66| 47.16|  8.54| 18.34| 21.31|2012-02-11T22:45:45.0000Z|2012-02-11T22:45:26.6562Z|  0.99|         18|        18
   3.75| 47.16|  8.54| 19.27| 19.91|2012-02-11T22:45:46.0000Z|2012-02-11T22:45:26.7326Z|  0.99|         19|        19

*Creation time* is the time the VS magnitude message was generated, *tdiff* is the
time difference between *creation time* and *origin time* in seconds, *likeh* is the
likelihood that this event is a real event (see documentation of the *scvsmag* module), # *st.(org)*
is the number of stations that contributed to the origin and # *st.(mag)* the number of envelope streams
that contributed to the magnitude.


.. _ref_VS_UDI:

ActiveMQ interface
=====================

Event messages
--------------

Event messages are sent once a first magnitude estimate is available. The
'message_type' of the first message is 'new', and for any successive message it's
either 'update' or 'delete'. Currently all values except the uncertainty
estimates will be set by scvsmaglog.

.. code-block:: xml

   <?xml version='1.0' encoding='UTF-8'?>
   <event_message message_type="new" orig_sys="dm" version="0">
       <core_info id="-9">
           <mag units="Mw">-9.9</mag>
           <mag_uncer units="Mw">-9.9</mag_uncer>
           <lat units="deg">-999.9</lat>
           <lat_uncer units="deg">-999.9</lat_uncer>
           <lon units="deg">-999.9</lon>
           <lon_uncer units="deg">-999.9</lon_uncer>
           <depth units="km">-9.9</depth>
           <depth_uncer units="km">-9.9</depth_uncer>
           <orig_time units="UTC">2013-06-10T13:35:12Z</orig_time>
           <orig_time_uncer units="sec">-9.9</orig_time_uncer>
           <likelihood>-9.9</likelihood>
       </core_info>
   </event_message>


Heartbeat messages
------------------

Heartbeat messages are sent in 5 s intervals.

.. code-block:: sh

   <?xml version='1.0' encoding='UTF-8'?>
   <hb originator="vs.9" sender="vs.9" timestamp="Mon June 10 13:41:35 2013" />

ActiveMQ broker configuration
-----------------------------

It is beyond the scope of this documentation to explain the complete setup of an
ActiveMQ broker. However, since scvsmaglog uses the STOMP protocol to send
messages to the broker it is essential to install the stompy package, which
provides Python bindings for the STOMP protocol, and to add the following line
to configuration of the ActiveMQ broker.

.. code-block:: sh

   <connector>
   <serverTransport uri="stomp://your-server-name:your-port"/>
   </connector>

Please refer to `ActiveMQ <http://activemq.apache.org/>`_ for setting up an 
ActiveMQ broker.

Consumer example
----------------
The following listing shows a consumer that listens for heartbeats and alerts 
and writes them to stdout.

.. code-block:: python

   #!/usr/bin/env python                                                                                 
   from stompy.simple import Client
   stomp = Client(host='your-server-name', port=your-port)
   stomp.connect(username='username', password='password')

   stomp.subscribe("/topic/heartbeat")
   stomp.subscribe("/topic/alert")
   while True:                                                                                           
       message = stomp.get()                                                                             
       print message.body                                                                                
   stomp.unsubscribe("/topic/heartbeat")
   stomp.unsubscribe("/topic/alerts")
   stomp.disconnect()

