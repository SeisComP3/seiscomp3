* template: $template
plugin $seedlink.source.id cmd = "$seedlink.plugin_dir/wave24_plugin -p $sources.wave24.comport -s $sources.wave24.baudrate -N $seedlink.station.network -S $seedlink.station.code -1 HHZ -2 HHN -3 HHE -f 100"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10
             proc = "$sources.wave24.proc"

