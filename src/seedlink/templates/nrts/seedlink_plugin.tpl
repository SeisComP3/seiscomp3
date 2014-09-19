* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/nrts_plugin$seedlink._daemon_opt -m $seedlink.source.streams -s $sources.nrts.address -p $sources.nrts.port"
             timeout = 0
             start_retry = 60
             shutdown_wait = 10

