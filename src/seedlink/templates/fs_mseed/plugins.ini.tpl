* Generated at $date - Do not edit!
* template: $template

[$seedlink.source.id]

input_type = $sources.fs_mseed.input_type
data_format = $sources.fs_mseed.data_format
location = $sources.fs_mseed.location

* Stations to process, separated by a comma. Default is all stations.
$sources.fs_mseed.station_list_def

* "pattern" is a POSIX extended regular expression that must match
* input file names (useful for filtering out non-data files).  For
* example "BH[NEZ]" would match any files that contained "BHE",
* "BHN" or "BHZ".  If no pattern is specified all files will be
* processed.
$sources.fs_mseed.pattern_def

* Look for data files at the 1st or 2nd directory level
scan_level = $sources.fs_mseed.scan_level

* Move file to subdirectory "processed" before starting to read it
move_files = $sources.fs_mseed.move_files_yesno

* Delete processed files
delete_files = $sources.fs_mseed.delete_files_yesno

* Look only for files that are newer than the last file processed
use_timestamp = $sources.fs_mseed.use_timestamp_yesno

* Timestamp file is used to save the modification time of the last file
* processed
timestamp_file = "$sources.fs_mseed.timestamp_file"

* New files are searched for every "polltime" seconds
polltime = $sources.fs_mseed.polltime

* Wait until the file is at least 30 seconds old, before trying to read it
delay = $sources.fs_mseed.delay

* "verbosity" tells how many debugging messages are printed
verbosity = $sources.fs_mseed.verbosity

* Maximum number of consecutive zeros in datastream before data gap will be
* declared (-1 = disabled)
zero_sample_limit = $sources.fs_mseed.zero_sample_limit

* If timing quality is not available, use this value as default
* (-1 = disabled)
default_timing_quality = $sources.fs_mseed.default_timing_quality

* Channel definitions (Mini-SEED streams are defined in streams.xml,
* look for <proc name="generic_3x50">)

$sources.fs_mseed.channel_map
