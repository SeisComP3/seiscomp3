#=========================================
# section name (do not remove)
[Logging]


#--- Device path and name of port for USB Seismometer Interface. type=string default=/dev/usbdev1.1
#   If the specified port cannot be opened or is not a USB Seismometer Interface device, all available ports
#	will be scanned to find a USB Seismometer Interface device.
#
port_path_hint=$sources.minilogger.port_path_hint


#--- Allow low-level setting of port interface attributes when available ports are scanned 
#   to find a USB Seismometer Interface device, 0=NO, 1=Yes. type=int default=0
#   Setting 1 (=Yes) may help successful detection and correct reading of the USB Seismometer Interface device,
#   particularly for the RasberryPi (to set correct baud rate?), 
#   but can have adverse effects on other devices, terminals, etc. open on the system.
#
allow_set_interface_attribs=$sources.minilogger.allow_set_interface_attribs


#--- Sets a fixed sample rate to report in the miniseed file header. type=real; default=-1
#   The default (value < 0.0) sets an estimated sample rate based on recent packet start times.
#   This estimated sample rate will vary slightly over time, potentially producing errors in some
#   software when reading the miniseed files.
#   See also: [Station] nominal_sample_rate
#
#   For SEISMOMETER INTERFACE (USB) (CODE: SEP 064) use:
#    nominal_sample_rate: mswrite_header_sample_rate
#	  20: 20.032 SPS
#	  40: 39.860 SPS
#	  80: 79.719 SPS
#
mswrite_header_sample_rate=$sources.minilogger.mswrite_header_sample_rate


#--- SEED data encoding type for writing miniseed files. type=string; default=DE_INT32
#   Supported values are: DE_INT16, DE_INT32
mswrite_data_encoding_type=DE_$sources.minilogger.mswrite_data_encoding_type



#=========================================
# section name (do not remove)
[Station]


#--- The code representing the network this station belongs to. type=string default=UK
#
station_network=$seedlink.station.network


#--- Descriptive name for station. Used for AmaSeis server. type=string default=TEST
#
station_name=$seedlink.station.code


#--- The initial letters to set for the miniseed header 'channel', will be prepended to the component. type=string default=BH
#
channel_prefix=$sources.minilogger.channel_prefix


#--- Component of seismogram, one of Z, N or E. type=string default=Z
#
component=$sources.minilogger.component


#--- Set sample rate and gain on SEP 064 device, 0=NO, 1=Yes. type=int default=0
#--- For SEISMOMETER INTERFACE (USB) (CODE: SEP 064) can be one of 20, 40 or 80
#
#
do_settings_sep064=$sources.minilogger.do_settings_sep064


#--- Nominal sample rate per second. type=int default=20
#    See also: [Logging] mswrite_header_sample_rate
#
#--- For SEISMOMETER INTERFACE (USB) (CODE: SEP 064) can be one of 20, 40 or 80
#
#
nominal_sample_rate=$sources.minilogger.nominal_sample_rate


#--- Nominal gain, one of 1, 2 or 4. type=int default=1
#
#--- For SEISMOMETER INTERFACE (USB) (CODE: SEP 064) can be 1, 2 or 4:
#	   '1': x1 = 0.64uV/count
#	   '2': x2 = 0.32uV/count
#	   '4': x2 = 0.32uV/count
#
nominal_gain=$sources.minilogger.nominal_gain


