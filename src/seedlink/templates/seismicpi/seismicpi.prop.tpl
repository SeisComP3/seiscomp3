#=========================================
# section name (do not remove)
[Logging]

#--- Device path and name of port for SeismicPI HAT. type=string default=/dev/ttyS0
#   If the specified port cannot be opened or is not a USB Seismometer Interface device, all available ports
#	will be scanned to find a USB Seismometer Interface device.
#
port_path_hint=$sources.seismicpi.port_path_hint

#--- Allow low-level setting of port interface attributes when available ports are scanned 
#   to find a USB Seismometer Interface device, 0=NO, 1=Yes. type=int default=1
#   Setting 1 (=Yes) may help successful detection and correct reading of the SeismicPI HAT Interface device.
#
allow_set_interface_attribs=$sources.seismicpi.allow_set_interface_attribs

#--- Sets a fixed sample rate to report in the miniseed file header. type=real; default=-1
#   The default (value < 32) sets an estimated sample rate based on recent packet start times.
#   See also: [Station] nominal_sample_rate
#
#   For SEISMOMETER INTERFACE (USB) (CODE: SEP 064) use:
#    nominal_sample_rate: mswrite_header_sample_rate
#	  32: 32 SPS
#	  64: 64 SPS
#        128: 128 SPS
#
mswrite_header_sample_rate=$sources.seismicpi.mswrite_header_sample_rate

#--- SEED data encoding type for writing miniseed files. type=string; default=STEIM2
#   Supported values are: DE_INT16, DE_INT32, DE_STEIM1, DE_STEIM2
mswrite_data_encoding_type=DE_$sources.seismicpi.mswrite_data_encoding_type

#=========================================
# section name (do not remove)
[Station]

#--- The code representing the network this station belongs to. type=string default=UK
#
station_network=$seedlink.station.network

#--- Descriptive name for station. Used for AmaSeis server. type=string default=TEST
#
station_name=$seedlink.station.code

#--- Descriptive name for the location code. Used for AmaSeis server. type=string default=00
#
location_code=$sources.seismicpi.locationcode_prefix

#--- The initial letters to set for the miniseed header 'channel', will be prepended to the component. type=string default=BH
#
channel_prefix=$sources.seismicpi.channel_prefix

#--- Component of seismogram, one of Z, N or E. type=string default=Z
#
component=$sources.seismicpi.component


#--- Set sample rate and gain on SEP 064 device, 0=NO, 1=Yes. type=int default=0
#--- For SEISMOMETER INTERFACE (USB) (CODE: SEP 064) can be one of 20, 40 or 80
#
#
do_settings_pihat=$sources.seismicpi.do_settings_pihat

#--- Nominal sample rate per second. type=int default=32
#    See also: [Logging] mswrite_header_sample_rate
#
#--- For SeismicPI HAT can be one of 32, 64 or 128
#
#
nominal_sample_rate=$sources.seismicpi.nominal_sample_rate


#--- Nominal gain, one of 1, 2 or 4. type=int default=1
#
#--- For SeismicPi HAT  can be 1, 2, 4 or 8:
#	   ‘1’: ×1 = 0.64μV/count 
#	   ‘2’: ×2 = 0.32μV/count 
#	   ‘4’: ×2 = 0.16μV/count 
#	   ‘8’: ×2 = 0.08μV/count 
#
nominal_gain=$sources.seismicpi.nominal_gain

#--- Single or Multi Channel Mode, 1 or 2. type=int default=2
#
#--- For SeismicPI HAT can be 1 or 2:
#	   ‘1’: Single Channel Mode
#	   ‘2’: Multi Channel Mode
#
single_multi=$sources.seismicpi.single_multi

#--- Internal or External Source, 1 or 2. type=int default=1
#
#--- For SeismicPI HAT can be 1 or 2:
#	   ‘1’: Use Internal Accelerometer
#	   ‘2’: Use external inputs
#
select_source=$sources.seismicpi.select_source
