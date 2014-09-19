* Generated at $date - Do not edit!
* template: $template

[arclink]

organization = "$organization"
request_dir = "$request_dir"
dcid = "$datacenterID"
contact_email = "$contact_email"
connections = $connections
connections_per_ip = $connections_per_ip
request_queue = $request_queue
request_queue_per_user = $request_queue_per_user
request_size = $request_size
handlers_soft = $handlers_soft
handlers_hard = $handlers_hard
handler_cmd = "$handler_cmd"
handler_timeout = $handler_timeout
handler_start_retry = $handler_start_retry
handler_shutdown_wait = $handler_shutdown_wait
port = $port
lockfile = "$lockfile"
statefile = "$statefile"
admin_password = "$admin_password"

* Maximum number of simultaneous request handler instances per request type
handlers_waveform = $handlers_waveform
handlers_response = $handlers_response
handlers_inventory = $handlers_inventory
handlers_routing = $handlers_routing
handlers_qc = $handlers_qc
handlers_greensfunc = $handlers_greensfunc

* Delete requests from RAM after 600 seconds of inactivity
swapout_time = $swapout_time

* Delete requests (and data products) completely after 10 days of inactivity
purge_time = $purge_time

* Enable the use of encryption to deliver restricted network data volumes
encryption = $encryption

* File containing a list of users(emails) and passwords separated by ":"
password_file = "$password_file"
