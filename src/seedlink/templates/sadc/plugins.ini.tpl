* Generated at $date - Do not edit!
* template: $template

[$seedlink.source.id]

* Settings for SADC10/18/20/30 digitizer

* Station ID (network/station code is set in seedlink.ini)
station=$seedlink.station.id

* Use the command 'serial_plugin -m' to find out which protocols are
* supported.
protocol=sadc

* Serial port 
port=$sources.sadc.comport

* Baud rate
bps=$sources.sadc.baudrate

* Use PC clock for initial time setting.
pctime=$sources.sadc.pctime

* Parameter 'time_offset' contains the amount of microseconds to be added
* to the time reported by the digitizer.

* 1.389 sec is possibly the correct offset if you have a version of the
* Earth Data digitizer with external GPS unit.
* time_offset=1389044

* Maximum number of consecutive zeros in datastream before data gap will be
* declared (-1 = disabled).
zero_sample_limit = -1

* Default timing quality in percents. This value will be used when no
* timing quality information is available. Can be -1 to omit the blockette
* 1001 altogether.
default_tq = 0

* Timing quality to use when GPS is out of lock
unlock_tq = 10

* Keyword 'channel' is used to map input channels to symbolic channel
* names. Channel names are arbitrary 1..10-letter identifiers which should
* match the input names of the stream processing scheme in streams.xml,
* which is referenced from seedlink.ini

channel Z source_id=0
channel N source_id=1
channel E source_id=2

channel Z1 source_id=3
channel N1 source_id=4
channel E1 source_id=5

* Remaining channels of the 16-channel digitizer are defined as auxiliary
* channels (100 sps, downsampled to 1 sps)

channel S0 source_id=6
channel S1 source_id=7
channel S2 source_id=8
channel S3 source_id=9
channel S4 source_id=10
channel S5 source_id=11
channel S6 source_id=12
channel S7 source_id=13
channel S8 source_id=14
channel S9 source_id=15

