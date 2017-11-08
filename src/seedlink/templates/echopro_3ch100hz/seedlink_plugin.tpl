* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/echopro_plugin -s $sources.echopro.station -p $sources.echopro.comport"
	timeout = 0
	start_retry = 60
	shutdown_wait = 10

