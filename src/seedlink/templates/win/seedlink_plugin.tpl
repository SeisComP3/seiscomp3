* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/win_plugin$seedlink._daemon_opt -v -F '$sources.win.mapFlag' $sources.win.udpport -"
             timeout = 3600
             start_retry = 60
             shutdown_wait = 10

