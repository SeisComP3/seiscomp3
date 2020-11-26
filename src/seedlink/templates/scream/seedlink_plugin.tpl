* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/scream_plugin$sources.scream.tcpFlag -h $sources.scream.address -p $sources.scream.port -m '$sources.scream.mapFlag'"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10
             proc = "$sources.scream.proc"

