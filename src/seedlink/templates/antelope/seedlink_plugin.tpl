* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/orb_plugin -v -S $pkgroot/var/lib/seedlink/${seedlink.station.id}.pkt:100 -m '.*${seedlink.station.code}.*' -o $sources.orb.address:$sources.orb.port"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10
             proc = "$sources.antelope.proc"

