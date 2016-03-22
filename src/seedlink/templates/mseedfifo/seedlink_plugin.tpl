* template: $template
plugin mseedfifo cmd="$seedlink.plugin_dir/mseedfifo_plugin$seedlink._daemon_opt -v $plugins.mseedfifo.noexit_param -d $plugins.mseedfifo.fifo_param"
             timeout = 0
             start_retry = 1
             shutdown_wait = 10

