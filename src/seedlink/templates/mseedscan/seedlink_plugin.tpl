* template: $template
plugin $seedlink.source.id cmd="$seedlink.plugin_dir/mseedscan_plugin -v -d $sources.mseedscan.dir -x $pkgroot/var/lib/seedlink/${seedlink.source.id}.seq"
             timeout = 1200
             start_retry = 60
             shutdown_wait = 10

