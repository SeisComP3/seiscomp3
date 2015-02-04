.. _request_handler_protocol:

****************************************
ArcLink Server, Request Handler protocol
****************************************

As explained in the ArcLink server documentation, the server distributed with seiscomp3 does not know anything
about how to prepare the data products it serves. Instead it uses a request handler program to generate the products
and just delivers the results prepared to the user. When the server starts it will start a couple of
request handlers (processes) that will communicate to the server using file descriptors. The server
uses the file descriptor 62 to send requests to a request handler and reads status responses from file
descriptor 63.

The communication between the server and the request handler is governed by an asynchronous protocol that is
described in this document.

Request handler protocol
------------------------

The ArcLink server sends a request to a request handler in the following format::

    USER <username> <password>
    [INSTITUTION <any string>]
    [LABEL <label>]
    REQUEST <request_type> <req_id> <optional_attributes>
    [one or more request lines...]
    END

After receiving the request, the request handler can send responses to the server. The following responses are
defined:

STATUS LINE <n> PROCESSING <vol_id>
    add request line number n (0-based) to volume vol id. The volume is created if it does not already exist.

STATUS <ref> <status>
    set line or volume status, where ref is "LINE n" or "VOLUME vol id" and status is one of the
    following:[[br]][[br]]

    ======== =====================================================
    Status   Description
    ======== =====================================================
    OK       request successfully processed, data available
    NODATA   no processing errors, but data not available
    WARN     processing errors, some downloadable data available
    ERROR    processing errors, no downloadable data available
    RETRY    temporarily no data available
    DENIED   access to data denied for the user
    CANCEL   processing cancelled (e.g., by operator)
    ======== =====================================================

MESSAGE
    any string error message in case of WARN or ERROR, but can be used regardless of the status (the last
    message is shown in the STATUS response)

SIZE <n>
    data size; in case of a volume, it must be the exact size of downloadable product.

MESSAGE <any_string>
    send a general processing (error) message. The last message is shown in the STATUS response.

ERROR
    the request handler could not process the request due to an error (e.g., got an unhandled Python
    exception). This terminates the request and normally the request handler quits. If the request handler
    does not quit, it should be ready to handle the next request. Note that if the request handler quits
    (crashes) without sending ERROR, then the request will be repeated (sent to another request handler
    instance) by the server. This behavior might be changed in future server versions to avoid loops, e.g., by
    implying ERROR if the request handler quits.

END
    request processing finished normally. The request handler is ready for the next request.

Session example
---------------

Since the communication from the request handler to the ArcLink server is performed by file descriptors it is
possible to test a request handler (provided on the seiscomp3 distribution) interactively by running a command
similar to following::

    python reqhandler -vvv 62>&0 63<&1 >reqhandler.log 2>&1

Now the program "reqhandler" waits for input from terminal and writes output to terminal as well. Additionally
a log file is written.

Let's type the following request::

    USER somebody@gfz-potsdam.de
    REQUEST WAVEFORM 123 format=MSEED
    2008,2,21,2,50,0 2008,2,21,3,10,0 EE MTSE BHZ .
    2008,2,21,2,50,0 2008,2,21,3,10,0 GE WLF BHZ .
    END

The log might look similar to following::

    01 [None] > USER somebody@gfz-potsdam.de
    02 [None] > REQUEST WAVEFORM 123 format=MSEED
    03 [123] new WAVEFORM request from somebody@gfz-potsdam.de, None
    04 [123] > 2008,2,21,2,50,0 2008,2,21,3,10,0 EE MTSE BHZ .
    05 [123] < STATUS LINE 0 PROCESSING GFZ
    06 [123] > 2008,2,21,2,50,0 2008,2,21,3,10,0 GE WLF BHZ .
    07 [123] < STATUS LINE 0 SIZE 43008
    08 [123] > END
    09 [123] < STATUS LINE 1 PROCESSING GFZ
    10 [123] < STATUS LINE 0 OK
    11 [123] < STATUS LINE 1 MESSAGE size not known
    12 [123] < STATUS LINE 1 OK
    13 [123] < STATUS VOLUME GFZ SIZE 73728
    14 [123] < STATUS VOLUME GFZ OK
    15 [123] < END


Note that the status responses are asynchronous.

02
    request ID "123" is chosen by the server (not user)

05
    the first request line is associated with volume ID "GFZ" (chosen by the request handler)

07
    request handler tells the size of data that is related to the first request line (optional)

09
    the second request line is associated with volume ID "GFZ"

10
    processing of the first line completed without error

11
    send optional message regarding the second line

12
    processing of the second line completed without error

13
    request handler tells the size of volume (mandatory)

14
    volume completed without error

15
    request processing finished

Minimal status response when data is available::

    STATUS LINE 0 PROCESSING GFZ
    STATUS LINE 1 PROCESSING GFZ
    STATUS LINE 0 OK
    STATUS LINE 1 OK
    STATUS VOLUME GFZ SIZE 73728
    STATUS VOLUME GFZ OK
    END

Minimal status response (and an optional error message) when data is not available::

    STATUS LINE 0 PROCESSING GFZ
    STATUS LINE 1 PROCESSING GFZ
    STATUS LINE 0 NODATA
    STATUS LINE 1 NODATA
    STATUS VOLUME GFZ NODATA
    MESSAGE optional error message
    END
