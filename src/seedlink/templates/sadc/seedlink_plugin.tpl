* template: $template
plugin $seedlink.source.id cmd = "$seedlink.plugin_dir/serial_plugin$seedlink._daemon_opt -v -f $seedlink.config_dir/plugins.ini"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10
             proc = "$sources.sadc.proc"

