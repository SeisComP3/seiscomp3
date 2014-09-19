* template: $template
plugin IO_$seedlink.station.id cmd = "$pkgroot/share/plugins/seedlink/serial_plugin$seedlink._daemon_opt -v -f $pkgroot/var/lib/seedlink/config/plugins.ini"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10

