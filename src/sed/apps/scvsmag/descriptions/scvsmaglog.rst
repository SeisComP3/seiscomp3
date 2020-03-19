Part of the :ref:`VS` package.

*scvsmaglog* is part of a new SeisComp3 implementation of the
`Virtual Seismologist <http://www.seismo.ethz.ch/en/research-and-teaching/products-software/EEW/Virtual-Seismologist>`_
(VS) Earthquake Early Warning algorithm (Cua, 2005; Cua and Heaton, 2007) released
under the `'SED Public License for SeisComP3 Contributions'
<http://www.seismo.ethz.ch/static/seiscomp_contrib/license.txt>`_. It requires 
the Python package `dateutil <https://pypi.python.org/pypi/python-dateutil>`_ to
be installed.

It logs the VS magnitude messages received from :ref:`scvsmag` and, once an event
has timed out, generates report files. These report files are saved to disk and
can also be sent via email.

It also implements an ActiveMQ interface (http://activemq.apache.org/) which 
provides the possibility to send alert messages in real-time. Currently, 
messages can be sent in three different formats (SeisComP3ML, QuakeML, ShakeAlertML).
The recommended client to display these alert messages is the `Earthquake 
Early Warning Display (EEWD) <http://www.seismo.ethz.ch/en/research-and-teaching/products-software/EEW/earthquake-early-warning-display-eewd/>`_
an open source user interface developed within the 
European `REAKT <http://www.seismo.ethz.ch/en/research-and-teaching/past-projects/>`_ project and based on the 
the `UserDisplay <http://www.eew.caltech.edu/research/userdisplay.html>`_.
The UserDisplay is not openly available, however, people with permission to run
the UserDisplay can use it to receive alert messages from *scvsmaglog*.

To receive alerts with the EEWD set the format to *qml1.2-rt*, to receive alerts
with the UserDisplay set the format to *shakealert*. There are currently no clients 
which can digest SeisComP3ML. Using pipelines alerts can be sent out in more 
than one format.

The real-time ActiveMQ interface requires the Python packages 
`stompy <https://pypi.python.org/pypi/stompy>`_ and `lxml <http://lxml.de/>`_ to 
be installed. 

It is beyond the scope of this documentation to explain the complete setup of an
ActiveMQ broker. However, since scvsmaglog uses the STOMP protocol to send
messages to the broker it is essential to add the following line
to configuration of the ActiveMQ broker.

.. code-block:: sh

   <connector>
   <serverTransport uri="stomp://your-server-name:your-port"/>
   </connector>

Please refer to `ActiveMQ <http://activemq.apache.org/>`_ for setting up an 
ActiveMQ broker.


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
likelihood that this event is a real event (see documentation of :ref:`scvsmag`), # *st.(org)*
is the number of stations that contributed to the origin and # *st.(mag)* the number of envelope streams
that contributed to the magnitude.


