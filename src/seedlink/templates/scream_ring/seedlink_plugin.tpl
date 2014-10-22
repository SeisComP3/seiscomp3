* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/scream_plugin_ring$sources.scream_ring.tcpFlag -h $sources.scream_ring.address -p $sources.scream_ring.port -r $sources.scream_ring.tcpport -rsize $sources.scream_ring.rsize -m '$sources.scream_ring.mapFlag'"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

