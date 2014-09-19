* template: $template
plugin $seedlink.source.id cmd = "$seedlink.plugin_dir/minilogger_plugin -p $seedlink.config_dir/${seedlink.station.network}.${seedlink.station.code}.${sources.minilogger.channel_prefix}${sources.minilogger.component}.prop"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

