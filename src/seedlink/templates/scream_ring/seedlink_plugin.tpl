* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/scream_plugin_ring -h $sources.scream_ring.address -p $sources.scream_ring.udpport -r $sources.scream_ring.tcpport -m '$sources.scream_ring.mapFlag'"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

