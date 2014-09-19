* Generated at $date - Do not edit!
* template: $template

[ED_$seedlink.station.id]

* Settings for EarthData PS6-24 digitizer

* Station ID (network/station code is set in seedlink.ini)
station=$seedlink.station.id

* Use the command 'serial_plugin -m' to find out which protocols are
* supported.
protocol=edata_r

* Serial port 
port=$sources.edata.comport

* Baud rate
bps=$sources.edata.baudrate

* lsb (defaults to 8): least significant bit (relative to 32-bit samples),
*   normally 8 for 24-bit samples, but can be set for example to 7 to get
*   25-bit samples;
* statusinterval (defaults to 0): time interval in minutes when "state of
*   health" information is logged, 0 means "disabled". State of health
*   channels can be used independently of this option.
*
* If you set 'checksum' to a wrong value then the driver will not work and
* you will get error messages like "bad SUM segment" or "bad MOD segment".
lsb=8
statusinterval=60

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

* "State of health" channels (1 sps) have source IDs 6...19.
* Which Mini-SEED streams (if any) are created from these channels
* depends on the stream processing scheme.

channel S1 source_id=6
channel S2 source_id=7
channel S3 source_id=8
channel S4 source_id=9
channel S5 source_id=10
channel S6 source_id=11
channel S7 source_id=12
channel S8 source_id=13

* Channel 20 records the phase difference between the incoming GPS 1pps
* signal and the internal one second mark. One unit is 333 us.

channel PLL source_id=20

