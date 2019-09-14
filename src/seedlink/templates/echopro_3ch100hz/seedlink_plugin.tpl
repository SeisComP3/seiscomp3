* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/echopro_plugin -s $seedlink.station.id -p $sources.echopro.comport"
             timeout = 0
             start_retry = 60
             shutdown_wait = 10
             proc = "$sources.echopro_3ch100hz.proc"

