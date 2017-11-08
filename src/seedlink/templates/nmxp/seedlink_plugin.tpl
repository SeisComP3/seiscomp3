* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/nmxptool -H $sources.nmxp.address -P $sources.nmxp.port -S $sources.nmxp.short_term_completion -M $sources.nmxp.max_latency$sources.nmxp.additional_options -k"
             timeout = 0
             start_retry = 60
             shutdown_wait = 10

