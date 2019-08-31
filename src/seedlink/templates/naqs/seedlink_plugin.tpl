* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/naqs_plugin -v -t 300 -n $sources.naqs.address -N $seedlink.station.network -p $sources.naqs.port -c $sources.naqs.channels"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

