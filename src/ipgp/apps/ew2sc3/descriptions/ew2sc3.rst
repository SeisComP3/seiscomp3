Earthworm to SeisComP3 application.

This application connects to an Earthworm export module using IP protocol and
listens for messages through a specified socket.



Communication
=============

The communication between Earthworm and SeisComP3 is done by using Earthworm
export_generic protocol. It's a TCP/IP protocol over which text messages are
sent by Earthworm. An hearbeat is sent by both ends of the socket (Earthworm
and SeisComP3), at a regular rate. If for some reason the hearbeat is not
received during a configurable amount of time (which should be higher than
the expected heartbeat rate), then the connection is re-initiated.



Message processing
==================

The messages sent trough the socket are identified by their institute ID and
their module ID. These IDs are configured within the export_generic Earthworm
module and Earthworm system.
Ew2sc3 only supports hypo2000_arc messages type from Earthworm identified by
the message ID 14. Therefore, any other message which ID is not 14 will be ignored.

When a message is received, ew2sc3 will first assess whereas it is correct or
not according to the institute and module IDs. Then, the hypo2000_arc message
is parsed and picks, arrivals, magnitudes and origin are created.
A new origin message with associated magnitudes, arrivals and picks is then
sent to SeisComP3 messaging system so it can be properly treated by scevent.

In case latitude and longitude in the hypo2000_arc message are null (space filled),
a default location is used.



Pick uncertainties
==================

Hypo2000_arc message uses weights for pick quality, 0 being the best picks and
4 the worst. Those weights are translated into uncertainties for SeisComP3.
A list of uncertainties is configured into ew2sc3 configuration file (pickerUncertainties).
Then weights from 0 to maxUncertainty are matched with the minimum and maximum
uncertainties. Finally, a linear formula allows to find the uncertainty
corresponding to the assigned weight.



More
====

A great deal of information is available at `Earthworm User Community Wiki`
`<http://love.isti.com/trac/ew/wiki>`_

More informations on the content of Hypo2000_arc message can be found on Earthworm website.
`<http://love.isti.com/trac/ew/wiki/Year_2000_Compliant_Earthworm_Message_Formats>`_

More informations on export_generic configuration can be found on Earthworm website.
`<http://love.isti.com/trac/ew/wiki/export_generic>`_

