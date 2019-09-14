* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/reftek_plugin$seedlink._daemon_opt -v -f '$seedlink.config_dir/plugins.ini' -m '$sources.reftek.mapFlag'"
             timeout = 0
             start_retry = 60
             shutdown_wait = 60
             proc = "$sources.reftek.proc"

