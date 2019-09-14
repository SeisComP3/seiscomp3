* Generated at $date - Do not edit!
* template: $template

[$name]
* Organization and default network code
organization = "$organization"
network = "$network"

* Lockfile path
lockfile = "$lockfile"

* Directory where Seedlink will store its disk buffers
filebase = "$filebase"

* Use Steim2 encoding by default
encoding = "$encoding"

* List of trusted addresses
trusted = "$trusted"

* List of IP addresses or IP/mask pairs (in ipchains/iptables syntax)
* that can access stations. Per station access definitions
* supersede this parameter.
access = "$access"

* Check start and end times of streams
stream_check = "$stream_check"

* If stream_check = enabled, also check for gaps in all channels that
* match given pattern. Register all gaps that are larger than +-0.5 seconds.
* gap_check_pattern = [EBLV][HLNG][ZNE]|S[NG][ZNE]
* Disabled to save memory.
gap_check_pattern = "$gap_check_pattern"
gap_treshold = "$gap_treshold"

* Allow time window requests from arbitrary Internet hosts
window_extraction = "$window_extraction"

* Allow time window requests from trusted hosts
window_extraction_trusted = "$window_extraction_trusted"

* Allow websocket connections from arbitrary Internet hosts
websocket = "$websocket"

* Allow websocket connections from trusted hosts
websocket_trusted = "$websocket_trusted"

* INFO provided to arbitrary Internet hosts: ID, CAPABILITIES, STATIONS,
* STREAMS
info = "$info"

* INFO provided to trusted hosts: ID, CAPABILITIES, STATIONS, STREAMS,
* GAPS, CONNECTIONS, ALL
info_trusted = "$info_trusted"

* Show requests in log file
request_log = "$request_log"

* Give warning if an input channel has time gap larger than 10 us
proc_gap_warn = "$proc_gap_warn"

* Flush streams if an input channel has time gap larger than 0.1 s
proc_gap_flush = "$proc_gap_flush"

* Reset FIR filters if an input channel has time gap larger than 1 s
proc_gap_reset = "$proc_gap_reset"

* Enable backfilling buffer for out-of-order records.
* This values defines its capacity in seconds.
backfill_buffer = "$backfill_buffer"

* Maximum allowed deviation from the sequence number of oldest packet if
* packet with requested sequence number is not found. If seq_gap_limit is
* exceeded, data flow starts from the next packet coming in, otherwise
* from the oldest packet in buffer.
* Use the following to always start with the oldest packet:
* seq_gap_limit = 16777216
seq_gap_limit = "$seq_gap_limit"

* Server's TCP port
port = "$port"

* Number of recent Mini-SEED packets to keep in memory
buffers = "$buffers"

* Number of temporary files to keep on disk
segments = "$segments"

* Size of one segment in 512-byte blocks
segsize = "$segsize"

* Total number of TCP/IP connections allowed
connections = "$connections"

* Maximum number of TCP/IP connections per IP
connections_per_ip = "$connections_per_ip"

* Maximum speed per connection (0: throttle disabled)
bytespersec = "$bytespersec"

* Defaults for all plugins. All of these parameters take value in seconds;
* zero disables the corresponding feature.
* 
* timeout -- shut down the plugin if no data arrives in this time period
*   [0 = wait for data forever];
* start_retry -- restart terminated plugins after this time period (plugins
*   can terminate themselves because of some internal error, or they can be
*   shut down by the Seedlink if timeout occurs or invalid data received)
*   [0 = don' restart terminated plugins];
* shutdown_wait -- wait this time period for a plugin to terminate after
*   sending the TERM signal. If a plugin will not terminate, it will be
*   terminated the hard way by sending the KILL signal [0 = wait forever].
plugin_timeout = 0
plugin_start_retry = 60
plugin_shutdown_wait = 10

