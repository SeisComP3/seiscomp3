* template: $template
plugin mseedfifo cmd="$seedlink.plugin_dir/mseedfifo_plugin$seedlink._daemon_opt -v -d $seedlink.run_dir/mseedfifo"
             timeout = 0
             start_retry = 1
             shutdown_wait = 10

