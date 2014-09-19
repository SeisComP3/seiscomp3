* Generated at $date - Do not edit!
* template: $template

[$seedlink.source.id]

* Settings for the RefTek plugin

* Hostname and TCP port of RTPD server
host = $sources.reftek.address
port = $sources.reftek.port

* Connection retry control:
* never     => no retry
* transient => retry on transient errors
* nonfatal  => retry on non-fatal errors
*
* Note that when reftek_plugin exits, it will be automatically restarted
* by seedlink, depending on the "start_retry" parameter in seedlink.ini
retry = nonfatal

* Timeout length in seconds. If no data is received from a Reftek unit
* during this period, the plugin assumes that the unit is disconnected.
timeout = $sources.reftek.timeout

* Default timing quality in percents. This value will be used when no
* timing quality information is available. Can be -1 to omit the blockette
* 1001 altogether.
default_tq = $sources.reftek.default_tq

* Timing quality to use when GPS is out of lock
unlock_tq = $sources.reftek.unlock_tq

* Send Reftek state-of-health data as Mini-SEED LOG stream
log_soh = $sources.reftek.log_soh
