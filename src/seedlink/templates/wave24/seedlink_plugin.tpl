* template: $template
plugin wave24_$seedlink.station.id cmd = "$seedlink.plugin_dir/wave24_plugin -p $sources.wave24.comport -s $sources.wave24.baudrate -N $seedlink.network.code -S $seedlink.station.code -1 HHZ -2 HHN -3 HHE -f 100"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

