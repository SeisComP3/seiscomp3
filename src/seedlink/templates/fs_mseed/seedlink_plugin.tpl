* template: $template
plugin ${seedlink.source.id} cmd = "$seedlink.plugin_dir/fs_plugin$seedlink._daemon_opt -v -f $seedlink.config_dir/plugins.ini"
             timeout = 1200
             start_retry = 60
             shutdown_wait = 10

