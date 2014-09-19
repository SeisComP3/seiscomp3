* Generated at $date - Do not edit!
* template: $template

[M24]

* Settings for the Lennartz M24 digitizer

* Station ID (network/station code is set in seedlink.ini)
station       = $seedlink.station.id

* Device for serial port
device        = $sources.m24.comport

* Speed for serial port
speed         = $sources.m24.baudrate

* Frame type on serial line
frame-type    = 0
 
* Sample interval in usecs
sample-iv     = 10000

* Time offset in usecs
time-off      = $sources.m24.time_offset

* 
resync-delay  = 900

* Leapseconds file to use
* leapseconds = /usr/src/share/zoneinfo/leapseconds

