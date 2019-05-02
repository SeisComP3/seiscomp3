* template: $template
plugin caps$seedlink.caps.id cmd="$seedlink.plugin_dir/caps_plugin -I $sources.caps.address -f $sources.caps.streamsFile -j $sources.caps.log"
             timeout = 0
             start_retry = 60
             shutdown_wait = 10

