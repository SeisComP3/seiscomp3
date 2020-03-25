* Generated at $date - Do not edit!
* template: $template

[$seedlink.source.id]

* Settings for WAGO I/O Controller

* Station ID (network/station code is set in seedlink.ini)
station=$seedlink.station.id

* Use the command 'serial_plugin -m' to find out which protocols are
* supported.
protocol=modbus

* Serial port 
port=tcp://$sources.wago.address:$sources.wago.port

* Baud rate
bps=0

* Time interval in minutes when status information is logged, 0 (default)
* means "disabled". Status channels can be used independently of this
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

* Strom 5A 12V System
channel SA source_id=$sources.wago.channels.a.sid realscale=0.183 realunit=mA precision=2

* Strom 5A 24V System
channel SB source_id=$sources.wago.channels.b.sid realscale=0.183 realunit=mA precision=2

* Strom 5A Reserve
channel SC source_id=$sources.wago.channels.c.sid realscale=0.183 realunit=mA precision=2

* Strom 5A Reserve
channel SD source_id=$sources.wago.channels.d.sid realscale=0.183 realunit=mA precision=2

* Spannung 30V DC Solarcontroller 1
channel SE source_id=$sources.wago.channels.e.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V DC Solarcontroller 2
channel SF source_id=$sources.wago.channels.f.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V DC Solarcontroller 3
channel SG source_id=$sources.wago.channels.g.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V DC Solarcontroller 4
channel SH source_id=$sources.wago.channels.h.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V DC Solarcontroller 5
channel SI source_id=$sources.wago.channels.i.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V DC Solarcontroller 6
channel SJ source_id=$sources.wago.channels.j.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V DC Solarcontroller 7
channel SK source_id=$sources.wago.channels.k.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V DC Solarcontroller 8
channel SL source_id=$sources.wago.channels.l.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V DC Diodenverknuepfung SC1+SC2
channel SM source_id=$sources.wago.channels.m.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V DC Diodenverknuepfung SC3+SC8
channel SN source_id=$sources.wago.channels.n.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V DC 12V VSAT (geschaltet) [Sri Lanka, Madagaskar]
channel SO source_id=$sources.wago.channels.o.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V DC 24V Converter
channel SP source_id=$sources.wago.channels.p.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V DC VSAT Router geschaltet 24V
channel SQ source_id=$sources.wago.channels.q.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V DC Diodenverknuepfung SC1-SC4 [Sri Lanka]
channel SR source_id=$sources.wago.channels.r.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V Reserve
channel SS source_id=$sources.wago.channels.s.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V Reserve
channel ST source_id=$sources.wago.channels.t.sid realscale=0.000916 realunit=V precision=3

* Spannung 30V Reserve
channel SU source_id=$sources.wago.channels.u.sid realscale=0.000916 realunit=V precision=3

* Wiederstandsmessung Schwimmer
channel SV source_id=$sources.wago.channels.v.sid realscale=0.5 realunit=Ohm precision=3

* Wiederstandsmessung Tuerk
channel SW source_id=$sources.wago.channels.w.sid realscale=0.5 realunit=Ohm precision=3

* Wiederstandsmessung Reserve
channel SX source_id=$sources.wago.channels.x.sid realscale=0.5 realunit=Ohm precision=3

* Wiederstandsmessung Reserve
channel SY source_id=$sources.wago.channels.y.sid realscale=0.5 realunit=Ohm precision=3

* 230V Messung
channel SZ source_id=$sources.wago.channels.z.sid realscale=0.1 realunit=V precision=3

