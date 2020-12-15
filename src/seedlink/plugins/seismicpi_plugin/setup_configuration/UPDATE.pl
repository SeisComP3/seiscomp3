#!/usr/bin/perl
#
# Shutting down SeisComP
print "Stopping SeisComP...\n";
`seiscomp stop`;
print "Updating configuration...\n";
# Parse input settings
$config ="/home/sysop/SETTINGS.txt";
open(CONFIG,"<$config") or die "Cannot open $config, $!\n";
@conf = <CONFIG>;
close(CONFIG);
chomp @conf;
@sta_line = grep(/^STATION/i, @conf);
$station = (split(/=/,$sta_line[0]))[1];
@net_line = grep(/^NET/i, @conf);
$net = (split(/=/,$net_line[0]))[1];
@comp_line = grep(/^COMPONENT/i, @conf);
$comp = (split(/=/,$comp_line[0]))[1];
@channel_line = grep(/^CHANNEL/i, @conf);
$channel = (split(/=/,$channel_line[0]))[1];
@loc_line = grep(/^LOCATION/i, @conf);
$loc = (split(/=/,$loc_line[0]))[1];
@gain_line = grep(/^GAIN/i, @conf);
$gain = (split(/=/,$gain_line[0]))[1];
@mode_line = grep(/^MODE/i, @conf);
$mode = (split(/=/,$mode_line[0]))[1];
@src_line = grep(/^SOURCE/i, @conf);
$src = (split(/=/,$src_line[0]))[1];

# Validate input values
# NET
$lnet = length $net;
if ($lnet > 2) { print "\n*** Error: Network length maximum length must be 2 characters, e.g UK, exiting. ***\n"; exit; }
# STATION
$lstat = length $station;
if ($lstat > 5) { print "\n*** Error: Station name maximum length must be 4 characters, e.g TEST, exiting. ***\n"; exit; }
# COMP
$lcomp = length $comp;
if ($lcomp > 1) { print "\n*** Error: Component maximum length must be 1 character, e.g Z, exiting. ***\n"; exit; }
# CHANNEL 
$lchan = length $channel;
if ($lchan > 2) { print "\n*** Error: Channel maximum length must be 2 character2, e.g BH, exiting. ***\n"; exit; }
# LOCATION 
$lloc = length $loc;
if ($lloc > 2) { print "\n*** Error: Location maximum length must be 1 character, e.g 00, exiting. ***\n"; exit; }
# GAIN
if ($gain eq "1") { $gain_msg = "0.064 μV/count";}
elsif ($gain eq "2") { $gain_msg = "0.32 μV/count";}
elsif ($gain eq "4") { $gain_msg = "0.16 μV/count";}
elsif ($gain eq "8") { $gain_msg = "0.08 μV/count";}
else { print "\n*** Wrong gain value defined.\n
--- For SeismicPi HAT can be 1, 2, 4 or 8:
	   ‘1’: ×1 = 0.64μV/count 
           ‘2’: ×2 = 0.32μV/count 
	   ‘4’: ×2 = 0.16μV/count 
	   ‘8’: ×2 = 0.08μV/count, exiting. ***\n "; exit; }
# MODE
if ($mode eq "1") {$mode_msg = "Single Channel Mode"; }
elsif ($mode eq "2") {$mode_msg = "Multi Channel Mode"; }
else { print "\n*** Wrong Channel Mode defined.\n
--- For SeismicPI HAT can be 1 or 2:
	   ‘1’: Single Channel Mode
	   ‘2’: Multi Channel Mode, exiting. ***\n"; exit; }
# SOURCE
if ($src eq "1") {$src_msg = "Internal Accelerometer";}
elsif ($src eq "2") {$src_msg = "External Inputs"; }
else { print "\n*** Wrong Source defined,\n	
--- For SeismicPI HAT can be 1 or 2:
	   ‘1’: Use Internal Accelerometer
	   ‘2’: Use external inputs, exiting. ***\n"; exit; }

#print "$station, $net, $comp, $channel, $loc, $gain, $mode, $src\n";
# Configure SeisComP files
# Uncomment the following to clean old key and config file
#print "Cleaning old files...\n";
#`rm /home/sysop/seiscomp3/etc/key/station*`;
# Make new key file
$sc3_key = "/home/sysop/seiscomp3/etc/key/station\_$net\_$station";
open(SC3,"+>$sc3_key") or die "Cannot open $sc3_key\n";
printf SC3 "seedlink:seismicpi";
close(SC3);
print "$sc3_key\n";
`seiscomp update-config`;
# Make plugin configuration file
$plugin_cfg = "/home/sysop/seiscomp3/var/lib/seedlink/$net.$station.ini";
open(CFG,"+>$plugin_cfg") or die "Cannot open $plugin_cfg\n";
printf CFG "
#=========================================
# section name (do not remove)
[Logging]

#--- Device path and name of port for SeismicPI HAT. type=string default=/dev/ttyS0
#   If the specified port cannot be opened or is not a USB Seismometer Interface device, all available ports
#	will be scanned to find a USB Seismometer Interface device.
#
port_path_hint=/dev/ttyS0

#--- Allow low-level setting of port interface attributes when available ports are scanned 
#   to find a USB Seismometer Interface device, 0=NO, 1=Yes. type=int default=1
#   Setting 1 (=Yes) may help successful detection and correct reading of the SeismicPI HAT Interface device.
#
allow_set_interface_attribs=1

#--- Sets a fixed sample rate to report in the miniseed file header. type=real; default=-1
#   The default (value < 32) sets an estimated sample rate based on recent packet start times.
#   See also: [Station] nominal_sample_rate
#
mswrite_header_sample_rate=-1

#--- SEED data encoding type for writing miniseed files. type=string; default=STEIM2
#   Supported values are: DE_INT16, DE_INT32, DE_STEIM1, DE_STEIM2
mswrite_data_encoding_type=DE_STEIM2

#=========================================
# section name (do not remove)
[Station]

#--- The code representing the network this station belongs to. type=string default=UK
#
station_network=$net

#--- Descriptive name for station. Used for AmaSeis server. type=string default=TEST
#
station_name=$station

#--- Descriptive name for the location code. Used for AmaSeis server. type=string default=00
#
location_code=$loc

#--- The initial letters to set for the miniseed header 'channel', will be prepended to the component. type=string default=BH
#
channel_prefix=$channel

#--- Component of seismogram, one of Z, N or E. type=string default=Z
#
component=$comp

#--- Set sample rate and gain on SEP 064 device, 0=NO, 1=Yes. type=int default=0
#--- For SEISMOMETER INTERFACE (USB) (CODE: SEP 064) can be one of 20, 40 or 80
#
#
do_settings_pihat=1

#--- Nominal sample rate per second. type=int default=32
#    See also: [Logging] mswrite_header_sample_rate
#
#--- For SeismicPI HAT can be one of 32, 64 or 128
#
#
nominal_sample_rate=32

#--- Nominal gain, one of 1, 2 or 4. type=int default=1
#
#--- For SeismicPi HAT  can be 1, 2, 4 or 8:
#	   ‘1’: ×1 = 0.64μV/count 
#	   ‘2’: ×2 = 0.32μV/count 
#	   ‘4’: ×2 = 0.16μV/count 
#	   ‘8’: ×2 = 0.08μV/count 
#
nominal_gain=$gain

#--- Single or Multi Channel Mode, 1 or 2. type=int default=2
#
#--- For SeismicPI HAT can be 1 or 2:
#	   ‘1’: Single Channel Mode
#	   ‘2’: Multi Channel Mode
#
single_multi=$mode

#--- Internal or External Source, 1 or 2. type=int default=1
#
#--- For SeismicPI HAT can be 1 or 2:
#	   ‘1’: Use Internal Accelerometer
#	   ‘2’: Use external inputs
#
source_select=$src
";
`seiscomp start`;
print "\n
******************************************************
Configuration update completed and SeisComP is running.
Station: $station
Network: $net
Location: $loc
Channel: $channel
Component: $comp
Gain: $gain_msg
Mode: $mode_msg
Source: $src_msg
******************************************************
";
sleep(5);
