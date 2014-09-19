* template: $template
plugin chain$seedlink.chain.id cmd="$seedlink.plugin_dir/chain_plugin$seedlink._daemon_opt -v -f $seedlink.config_dir/chain${seedlink.chain.id}.xml"
             timeout = 600
             start_retry = 60
             shutdown_wait = 15

