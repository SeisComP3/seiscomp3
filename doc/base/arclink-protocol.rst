.. _arclink_protocol:

****************
ArcLink Protocol
****************

The ArcLink protocol is a plain ASCII protocol. On connection, the server waits for a sequence of commands
from the client. Each command is processed sequentially and the client should waits for the previous command
to end before sending a new command.

ArcLink commands consist of an ASCII string followed by zero or more arguments separated by spaces and
terminated with carriage return (<cr>, ASCII code 13) followed by an optional line feed (<lf>, ASCII code 10).
Except for STATUS, the response on the command consists of one or several lines terminated with <cr><lf>.
Unless noted otherwise, the response is OK<cr><lf>or ERROR<cr><lf>, depending if the command has been
successful or not. After getting the ERROR response, it is possible to retrieve the error message with
SHOWERR command.

Client protocol
===============

The description of every know command to the ArcLink client protocol is given here.

HELLO
    returns two <cr><lf>-terminated lines with the software version and the data centre name.

BYE
    close the connection (useful for testing the server with telnet, otherwise it is enough to close the
    client-side socket).

USER <username> <password>
    authenticates the user; required before any of the following commands.

INSTITUTION <any string>
    optionally specifies institution name.

LABEL <label>
    optional label of request.

SHOWERR
    returns one <cr><lf>-terminated line, containing the error message (to be used after getting the ERROR
    response).

REQUEST <request_type> <optional_attributes>
    start of a request

END
    end of a request; if successful, returns request ID, otherwise ERROR<cr><lf>

STATUS req id
    fetch the status of request with the ID "req id". if "req id"==ALL, the status of all requests of the user
    are send. The response is either ERROR<cr><lf>or an XML document, followed by END<cr><lf>.

DOWNLOAD <req_id[.vol_id] [pos]>
    download the result of the request. Response is ERROR<cr><lf>or size, followed by the data and
    END<cr><lf>. Optional argument "pos" enables to resume a broken download.

BDOWNLOAD <req_id[.vol_id] [pos]>
    like DOWNLOAD, but will block the download until the data are complete.

PURGE <req_id>
    delete the result of a request from the server.

Making a request
================

To make a request to an ArcLink server the user should initially identify itself using the USER command. After
this operation is performed and you received an OK string from the server you can start a request by using the
REQUEST command. The REQUEST command is the only multiple lines command given to the ArcLink server, to end a
REQUEST command you should use the END command.

The general representation of a request is::

  REQUEST <request_type> [optional_attributes]
  <start_time> <end_time> <net> <station> <stream> <loc_id> [optional_constraints]
  [more request lines...]
  END

Allowed request types are currently WAVEFORM, RESPONSE, INVENTORY, ROUTING and QC. Resulting data format of
WAVEFORM and RESPONSE requests is SEED (Mini-SEED or full SEED for WAVEFORM and dataless SEED for RESPONSE).
Data format of INVENTORY, ROUTING and QC requests is XML. Data can be optionally compressed by bzip2 when the
optional_options contains the compression option set (e.g. compression=bzip2).

The start_time and end_time are strings containing: YYYY,MM,DD,HH,MM,SS e.g. "1980,1,1,0,1,1" and net is
normally a FDSN network code, station a station code, stream a stream code and loc_id a location code.

The server does a check on the request type and after the REQUEST command it returns OK or ERROR. If ERROR is
returned the request type was rejected by the server. If OK is received you should continue and supply to the
server the request lines and finish the REQUEST block with an END command. 

When no errors are found the server will attribute a request ID to your request, and as soon as a request
handler of the correct type is free your request will be sent for processing. In the meatime your request
status will be marked as UNSET status. At this point, the server also returns to the user the request ID
attributed to this request that should be used for checking status, download and purging the request afterwards.

Request types
=============

WAVEFORM request
----------------

  If request_type is WAVEFORM, attributes "format" and "compression" are defined. The value of "format"
  can be "MSEED" for Mini-SEED or "FSEED" (default) for full SEED; "compression" can be "bzip2"
  or "none" (default). Wildcards ("*") are allowed only in stream and loc_id. Constraints are not allowed. loc_id
  is optional. If loc_id is missing or ".", only streams with empty location ID are requested. Sample
  waveform request::

      REQUEST WAVEFORM format=MSEED
      2005,09,01,00,05,00 2005,09,01,00,10,00 IA PPI BHZ .
      END

RESPONSE request
----------------

  If request_type is RESPONSE, attribute "compression" is defined, which can be "bzip2" or "none"
  (default). Constraints are not allowed. Wildcard ("*") is allowed in station stream and loc_id, so it is
  possible to request a dataless volume of a whole network. If loc_id is missing or ".", only streams with
  empty location ID are included in the dataless volume. The dataless volume generated is compatible with the
  FDSN dataless seed specifications.

INVENTORY request
-----------------

  If request_type is INVENTORY, attributes "instruments", "compression" and "modified_after" are defined. The
  value of "instruments" can be "true" or "false" (default), "compression" can be "bzip2" or "none" (default),
  and "modified_after", if present, must contain an ISO time string. When instruments is set to true whether
  instrument data is added to XML, compression set bzip2 compress the XML data and modified_after if set, only
  entries modified after given time will be returned. Wildcard ("*") is allowed in all fields, except start time
  and end time. Station, stream and loc_id are optional. If station or stream is not specified, the respective
  elements are not added to the XML tree; if loc_id is missing or ".", only streams with empty location ID are
  included. For example, to request a list of GEOFON stations (but not location and streams information), one
  would use::

    REQUEST INVENTORY
    1990,1,1,0,0,0 2030,12,31,0,0,0 GE *
    END

  For inventory requet type the following constraints are defined:

    sensortype
        limit streams to those using specific sensor types: "VBB", "BB", "SM", "OBS", etc. Can be also a
        combination like "VBB+BB+SM".

    latmin
        minimum latitude

    latmax
        maximum latitude

    lonmin
        minimum longitude

    lonmax
        maximum longitude

    permanent
        true or false, requesting only permanent or temporary networks respectively

    restricted
        true or false, requesting only networks/stations/streams that have restricted or open data respectively.

  If any of station, stream or loc id is missing, one or more dots should be used before constraints. For
  example, to request the list of networks with open data, one would use::

    REQUEST INVENTORY
    1990,1,1,0,0,0 2030,12,31,0,0,0 * . restricted=false
    END

  More than one constrain can be added and should be separated by spaces.

ROUTING request
---------------

  If request_type is ROUTING, attributes "compression" and "modified_after" are defined. The value of
  "compression" can be "bzip2" or "none" (default); "modified after", if present, must contain an ISO time
  string. compression set to bzip2 will compress XML data and modified after if set, only entries modified after
  given time will be returned. Wildcard ("*") is allowed in all fields, except start time and end time.
  Constraints are not allowed. All fields except start time, end time and net are optional; missing station
  stands for "default route" of a given network. stream and loc id are ignored.

QC request
----------

  If request_type is QC, attributes "compression", "outages", "logs" and "parameters" are defined. The value
  of "compression" can be "bzip2" or "none" (default). compression set to bzip2 will compress XML data,
  outages include list of outages ("true" or "false"). logs include log messages ("true" or "false").
  parameters can contain a comma-separated list of QC parameters. Wildcard "*" is allowed in all fields,
  except start time and end time. All fields must be present. Constraints are not allowed. The following QC
  parameters are implemented in the present version: availability, delay, gaps count, gaps interval, gaps
  length, latency, offset, overlaps count, overlaps interval, overlaps length, rms, spikes amplitude, spikes
  count, spikes interval, timing quality. These parameters are documented at [wiki:doc/applications/scqc].

.. toctree::
  /base/request-handler-protocol

