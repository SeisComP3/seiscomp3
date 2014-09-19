* Generated at $date - Do not edit!
* template: $template

[WS_$seedlink.station.id]

* Settings for Reinhardt weather station (GITEWS)

* Station ID (network/station code is set in seedlink.ini)
station=$seedlink.station.id

* Use the command 'serial_plugin -m' to find out which protocols are
* supported.
protocol=mws

* Serial port 
port=$sources.mws.comport

* Baud rate
bps=$sources.mws.baudrate

* Time interval in minutes when weather information is logged, 0 (default)
* means "disabled". Weather channels can be used independently of this
* option.
statusinterval=60

* Maximum number of consecutive zeros in datastream before data gap will be
* declared (-1 = disabled).
zero_sample_limit = -1

* Default timing quality in percents. This value will be used when no
* timing quality information is available. Can be -1 to omit the blockette
* 1001 altogether.
default_tq = -1

* Keyword 'channel' is used to map input channels to symbolic channel
* names. Channel names are arbitrary 1..10-letter identifiers which should
* match the input names of the stream processing scheme in streams.xml,
* which is referenced from seedlink.ini

* Indoor Temperature (C * 100)
channel KI source_id=TE scale=100

* Indoor Humidity (%)
channel II source_id=FE scale=1

* Air Pressure (hPa * 10)
channel DI source_id=DR scale=10

