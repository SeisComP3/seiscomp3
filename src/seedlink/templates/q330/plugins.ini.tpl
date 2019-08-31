* Generated at $date - Do not edit!
* template: $template

[$seedlink.source.id]

station=$seedlink.station.id
udpaddr=$sources.q330.address
baseport=$sources.q330.port
dataport=$sources.q330.slot
hostport=$sources.q330.udpport
serialnumber=$sources.q330.serial
authcode=$sources.q330.auth
statefile=$seedlink.run_dir/${seedlink.station.id}.${seedlink.source.id}.cn
messages=SDUMP,LOGEXTRA,AUXMSG
statusinterval=60

