* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/ewexport_pasv_plugin -v -s $sources.ewexport_pasv.address -p $sources.ewexport_pasv.port -At '$sources.ewexport_pasv.heartbeat.message' -Ar $sources.ewexport_pasv.heartbeat.rate"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

