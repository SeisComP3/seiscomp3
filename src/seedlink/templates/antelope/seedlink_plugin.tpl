* template: $template
plugin ORB_$seedlink.station.id cmd="$seedlink.plugin_dir/orb_plugin -v -S $pkgroot/var/lib/seedlink/${seedlink.station.id}.pkt:100 -m '.*${seedlink.station.code}.*' -o $sources.antelope.address:$sources.antelope.port"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

