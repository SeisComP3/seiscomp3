* template: $template
plugin $seedlink.source.id cmd = "$seedlink.plugin_dir/seismicpi_plugin -p $seedlink.config_dir/${seedlink.station.network}.${seedlink.station.code}.ini"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

