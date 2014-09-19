* template: $template
plugin ${seedlink.source.id}_$seedlink.station.id cmd = "$seedlink.plugin_dir/q330_plugin$seedlink._daemon_opt -v -f $seedlink.config_dir/plugins.ini"
             timeout = 3600
             start_retry = 60
             shutdown_wait = 10

