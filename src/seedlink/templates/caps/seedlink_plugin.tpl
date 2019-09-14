* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/caps_plugin -I $sources.caps.address -f $sources.caps.streamsFile -j $sources.caps.log --max-time-diff $sources.caps.maxTimeDiff$sources.caps.inOrder"
             timeout = 0
             start_retry = 60
             shutdown_wait = 10
             proc = "$sources.caps.proc"

