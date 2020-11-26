* template: $template
plugin M24 cmd = "$seedlink.plugin_dir/m24-plug -p $seedlink.config_dir"
             timeout = 600
             start_retry = 60
             shutdown_wait = 10
             proc = "$sources.m24.proc"

