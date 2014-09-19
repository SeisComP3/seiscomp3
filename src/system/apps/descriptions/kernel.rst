The kernel is the basic configuration for the :command:`seiscomp` tool.
It contains configuration parameters for all init scripts in :file:`etc/init`. Each init script can, but does not
need to, read the kernel parameters and configure itself accordingly. Kernel parameters are not mandatory but
should be taken as (serious) hints. If for example syslog is enabled in the kernel then all init scripts should
configure syslog as logging backend for the programs they start. But if a program does not support syslog it can
also be started without logging to syslog.
