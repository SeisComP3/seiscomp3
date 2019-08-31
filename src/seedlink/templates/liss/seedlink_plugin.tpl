* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/sock_plugin -v -s $sources.liss.address -p $sources.liss.port -n $seedlink.station.network"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

