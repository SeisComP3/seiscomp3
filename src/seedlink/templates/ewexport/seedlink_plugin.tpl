* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/ewexport_plugin -v -s $sources.ewexport.address -p $sources.ewexport.port -At '$sources.ewexport.heartbeat.message' -Ar $sources.ewexport.heartbeat.rate"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10
             proc = "$sources.ewexport.proc"

