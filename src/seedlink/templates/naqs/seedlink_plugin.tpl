* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/naqs_plugin -v -t 300 -n $sources.naqs.address -p $sources.naqs.port -c ."
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

