* template: $template
plugin $seedlink.source.id cmd = "$seedlink.plugin_dir/q330_plugin$seedlink._daemon_opt -v -f $seedlink.config_dir/plugins.ini"
             timeout = 3600
             start_retry = 60
             shutdown_wait = 10
             proc = "$sources.q330.proc"

