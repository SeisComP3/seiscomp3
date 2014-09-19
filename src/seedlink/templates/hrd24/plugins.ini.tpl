* Generated at $date - Do not edit!
* template: $template

[hrd24_$seedlink.station.id]

* Settings for Nanometrics HRD24 digitizer

* Station ID (network/station code is set in seedlink.ini)
station=$seedlink.station.id

* Use the command 'serial_plugin -m' to find out which protocols are
* supported.
protocol=hrd24

* Serial port 
port=$sources.hrd24.comport

* Baud rate
bps=$sources.hrd24.baudrate

* Number of "bundles" in one packet
bundles=$sources.hrd24.bundles

* Maximum number of consecutive zeros in datastream before data gap will be
* declared (-1 = disabled).
zero_sample_limit = -1

* Default timing quality in percents. This value will be used when no
* timing quality information is available. Can be -1 to omit the blockette
* 1001 altogether.
default_tq = 0

* Timing quality to use when GPS is out of lock
unlock_tq = 10

* Directory for state-of-health log files (optional)
* soh_log_dir=$pkgroot/var/log/hrd24-SOH-files

* Keyword 'channel' is used to map input channels to symbolic channel
* names. Channel names are arbitrary 1..10-letter identifiers which should
* match the input names of the stream processing scheme in streams.xml,
* which is referenced from seedlink.ini

channel Z source_id=0
channel N source_id=1
channel E source_id=2

