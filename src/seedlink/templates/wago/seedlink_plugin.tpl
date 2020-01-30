* template: $template
plugin $seedlink.source.id cmd = "$pkgroot/share/plugins/seedlink/serial_plugin$seedlink._daemon_opt -v -f $pkgroot/var/lib/seedlink/plugins.ini"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10
             proc = "$sources.wago.proc"

