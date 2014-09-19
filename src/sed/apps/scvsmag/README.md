\mainpage Virtual Seismologist

The Virtual Seismologist in SeisComP3 ("VS-in-SC3") provides near instantaneous 
estimates of earthquake magnitude as soon as SeisComp3 origins are available. With a 
well-configured SeisComP3 system running on a dense network, magnitudes for 
local events can be available within 10-20 s of origin time. VS-in-SC3 can be a key 
component of an earthquake early warning system, and can be used to provide 
rapid earthquake notifications. With the capability to estimate magnitude 
(given a location estimate) with 3-seconds of P-wave information at a single 
station, VS-in-SC3 magnitude estimates are tens of seconds faster than 
conventional Ml calculations, which require waiting for the peak S-wave 
amplitudes. The VS magnitude estimation relationships consist of 1) a 
relationship between observed ground motion ratios (between vertical 
acceleration and vertical displacement) and magnitude, and 2) envelope 
attenuation relationships describing how various channels of envelope 
amplitudes vary as a function of magnitude and distance. These relationships 
were derived from a Southern California dataset with events in the magnitude 
range 2.5 <= M <= 7.6 and the Next Generation Attenuation (NGA) strong motion 
dataset. Once a SeisComp3 location estimate is available, VS magnitude estimates 
can be computed with as little as 3 seconds of envelope data at a single 
station (i.e., 3 seconds after trigger time at a single station). Typically, 
usable envelope data is available at numerous stations by the time the first 
SeisComp3 origin is available. The VS magnitude estimates are then updated every 
second for 45 seconds (configurable). The SeisComp3 implementation allows for use of 
broadband high-gain seismometers (with clipping value selected) as well as 
strong motion data. For co-located stations, VS magnitudes are calculated using 
the strong motion channels if the broadband channels saturate. 
VS magnitudes in SeisComp3 are called MVS.

## Development of VS
The Virtual Seismologist method is a Bayesian approach to earthquake early 
warning (EEW) that estimates earthquake magnitude, location, and the 
distribution of peak ground shaking using observed picks and ground motion 
amplitudes, predefined prior information, and envelope attenuation 
relationships (Cua, 2005; Cua and Heaton, 2007; Cua et al., 2009). The 
application of Bayes' theorem in EEW (Cua, 2005) states that the most probable 
source estimate at any given time is a combination of contributions from prior 
information (candidate priors include network topology or station health status,
regional hazard maps, earthquake forecasts, and the Gutenberg-Richter 
magnitude-frequency relationship) and constraints from the available 
real-time ground motion and arrival observations. VS is envisioned as an 
intelligent, automated system capable of mimicking how human seismologists can 
make quick, relatively accurate “back-of-the-envelope” interpretations of 
real-time (and at times, incomplete) earthquake information, using a mix of 
experience, background information, and real-time data. The formulation of the 
VS Bayesian methodology, including the development of the underlying 
relationships describing the dependence of various channels of ground motion 
envelopes on magnitude and distance, and how these pieces come together in EEW 
source estimation, was the result of the PhD research of Dr. Georgia Cua with 
Prof. Thomas Heaton at Caltech, from 1998 through 2004.

The first real-time VS prototype system was developed by Georgia Cua and Michael Fischer at ETH 
Zurich from 2006-2012. This first prototype used location estimates generated 
by the Earthworm Binder modules (Dietz, 2002) as inputs to the VS magnitude 
estimation. This architecture has been undergoing continuous real-time testing 
in California (since 2008) and Switzerland (since 2010). In California, VS is 
one of the 3 EEW algorithms that make up the CISN ShakeAlert EEW system 
(http://www.cisn.org/eew/). The other algorithms are the ElarmS algorithm from 
UC Berkeley and the TauC/Pd OnSite algorithm from Caltech.
In 2012/13, with funding from the EU projects NERA ("Network of European 
Research Infrastructures for Earthquake Risk Assessment and Mitigation") and 
REAKT ("Strategies and Tools for Real-Time EArthquake RisK ReducTion"), VS was 
integrated into SeisComP3 by the Seismic Network group at the SED in ETH 
Zurich and gempa GmbH. Both real-time VS implementations (Binder- and SeisComp3-based)
focus on real-time processing of available pick and envelope data. Prior 
information is not included.

## VS and SeisComP3
Although the codes were effectively re-written, the basic architecture used in 
the original Earthworm-based implementation is used in SeisComp3. The SeisComp3 modules 
scautopick, scautoloc, and scevent replace the Earthworm Binder modules for 
providing location estimates. Two new VS-specific modules were developed to 
continuously calculate envelope amplitudes and to calculate and update VS 
magnitudes (MVS) once an SeisComp3 origin is available. 

- \subpage scenv -  reads SeedLink data streams and continuously calculates peak
                    parameters for each of acceleration, velocity and 
                    displacement for each component in 1 s envelopes.
- \subpage scvsm -  listens to new origins, and when they occur, uses the peak 
                    envelope values to rapidly estimate the overall VS magnitude
                    for the origin, including likelihood information on whether 
                    the location and magnitude are correct. Can take site 
                    amplifications into account. 

MVS is calculated and updated (with updates attached to the preferred origin) 
each second for 45 seconds (unless configured differently) after it is first 
invoked by the availability of a new SeisComp3 event. If configured, Ml can also be 
calculated for these events.

An additional module, \subpage scvsml, creates log output and mails solutions 
once a new event is fully processed.

## Configuring and optimizing VS in SeisComp3 EEW
The performance of VS-in-SC3 is strongly dependent on: 1) the quality and 
density of the seismic network; 2) the configuration of the general SeisComp3 system. 
scautoloc requires at least 6 triggers to create an origin. Given the network 
geometry, maps of when VS estimates would be first available 
(indicative of the size of the blind zone as a function of earthquake location 
relative to stations) can be generated for regions where EEW is of interest. SeisComp3 
VS uses scautoloc, which was not built for EEW, so an 
additional delay of some seconds is required for origin processing. VS 
magnitudes (MVS) can be expected within 1-2 seconds after a SeisComp3 origin is 
available. In the densest part of the Swiss network, SeisComp3 origins are available 
within 10-15 seconds after origin time; MVS is typically available 1-2 seconds 
later.

The VS magnitude estimation relationships in Cua (2005) were derived from a
dataset consisting of Southern California waveforms and the NGA strong motion 
dataset. In theory, customizing VS to a specific region requires deriving a set 
of envelope attenuation relationships (168 coefficients) and relationships 
between ground motion ratios and magnitude (6 coefficients) from a regional 
dataset. In practice, the VS magnitude estimation relationships derived from 
Southern California have been shown to work reasonably well in Northern 
California and Switzerland (Behr et al, 2012). The envelope and ground motion 
ratio coefficents from Cua (2005) are hard-coded in scvsmag, and should not be 
modified without full understanding of the VS methodology and potential 
consequences of the modifications. 

Although scautoloc can produce origins at any depth, the VS magnitude estimation
relationships assume a depth of 3 km. For this reason, it is expected that MVS 
will systematically underestimate magnitudes for deep earthquakes. It may be
most practical to simply add empirically derived offsets to MVS for deeper 
events, or for particular regions. 

## Understanding VS output
The VS system currently being offered is a test version. A tool for 
dissemination of results is not part of the core modules. 

## False alarms, missed events, solution quality
The rate of false alarms and missed events is determined by the output of the 
normal SeisComp3 origin chain (scautopick, scautoloc), and will be similar to the 
performance of the automatic setup for typical network operations (i.e. if you 
do not trust your automatic origins for the network, you will not trust them for
VS either). A solution quality is independently estimated by VS, combining 
information on location quality and station quality . A detailed graph on how 
the solution quality is determined can be found here: \subpage lh 

## VS License 
The SeisComp3 VS modules are free and open source, and are part of the SeisComp3 
distribution from Seattle v2013.xxx. They are distributed under the 'SED Public 
License for SeisComP3 Contributions' 
(see http://www.seismo.ethz.ch/static/seiscomp_contrib/license.txt ).

## References
Dietz, L., 2002: Notes on configuring BINDER_EW: Earthworm's phase associator, http://folkworm.ceri.memphis.edu/ew-doc/ovr/binder_setup.html (last accessed June 2013)

Cua, G., 2005: Creating the Virtual Seismologist: developments in ground motion characterization and seismic early warning. PhD thesis, California Institute of Technology, Pasadena, California.

Cua, G., and T. Heaton, 2007: The Virtual Seismologist (VS) method: a Bayesian approach to earthquake early warning, in Seismic early warning, editors: P. Gasparini, G. Manfredi, J. Zschau, Springer Heidelberg, 85-132.

Cua, G., M. Fischer, T. Heaton, S. Wiemer, 2009: Real-time performance of the Virtual Seismologist earthquake early warning algorithm in southern California, Seismological Research Letters, September/October 2009; 80: 740 - 747.

Behr, Y., Cua, G., Clinton, J., Heaton, T., 2012: Evaluation of Real-Time Performance of the Virtual Seismologist Earthquake 
Early Warning Algorithm in Switzerland and California. Abstract 1481084 presented at 2012 Fall Meeting, AGU, San Francisco, Calif., 3-7 Dec.


\page scenv scenvelope
*scenvelope* is part of a new SeisComp3 implementation of the [Virtual Seismologist]( http://www.seismo.ethz.ch/research/vs) 
(VS) Earthquake Early Warning algorithm (Cua, 2005; Cua and Heaton, 2007) released 
under the 'SED Public License for SeisComP3 Contributions' 
(http://www.seismo.ethz.ch/static/seiscomp_contrib/license.txt ). It generates 
real-time envelope values for horizontal and vertical 
acceleration, velocity and displacement from raw acceleration and velocity 
waveforms. It was implemented to handle the waveform pre-processing necessary for the 
*scvsmag* module. It provides in effect continuous real-time streams of PGA, PGV and PGD values which 
could also be used independently of *scvsmag*.
The processing procedure is as follows:
1. gain correction
2. baseline correction
3. high-pass filter with a corner frequency of 3 s period
4. integration or differentiation to each of velocity, acceleration and displacement
5. computing the absolute value within 1 s intervals

The resulting envelope values are sent as messages to scmaster. Depending on the 
number of streams that are processed this can result in a significant number of 
messages (#streams/s). 

## Options
*scenvelope* supports commandline options as well as configuration files (scenvelope.cfg)

## Configuration
\par envelope.streams.whitelist (list) [""]
\par envelope.streams.blacklist (list) [""]
This is the same concept of including/excluding streams in the processing 
that is also used in *scwfparam*. The rules to decide if a stream is used or 
not are the following:
1. if whitelist is not empty and the stream is not on the whitelist, don't use it, ok otherwise
2. if blacklist is not empty and the stream is on the blacklist, don't use it, ok otherwise

Both checks are made and combined with AND. Either whitelist or blacklist 
contains a list of patterns. Wildcards `*` and `?` are allowed, e.g. `GE.*.*.*`, `*`, `GE.MORC.*.BH?` . 
Each stream id (NET.STA.LOC.CHA) will be checked against the defined patterns.
Examples:

    #Disable all SH streams
    envelope.streams.blacklist = *.*.*.SH?

    # Disable all SH streams and BH on station STA01
    envelope.streams.blacklist = *.*.*.SH?, *.STA01.*.BH?

    # Disable network AB
    envelope.streams.blacklist = AB.*.*.*

\par envelope.saturationThreshold (integer) [80]
Identical to the configuration parameter of *scwfparam*, this parameter defines the relative saturation 
threshold in percent. If the absolute raw amplitude exceeds X% of 2^23 counts, 
the station will be flagged as 'clipped' by *scenvelope*. This assumes a 24-bit datalogger.

\par envelope.baselineCorrectionBuffer (integer) [60]
This parameter defines the length of the buffer (in seconds) that is used to 
compute a real-time average for baseline correction. 

\par envelope.useSC3Filter (bool) [false]
This is for internal testing purposes only. 'true' will cause the 
SeisComp3 filter routines to be used in step 3 of the processing (s.o.). If 
'false' the filter routines from the Earthworm based CISN/ETH implementation of 
VS will be employed.

## Commandline

### Generic

\par -h [\-\-help]
Produce help message.

\par -V [\-\-version]
Show version information.

\par \-\-config-file arg
Use alternative configuration file. When this option is used
the loading of all stages is disabled. Only the given configuration
file is parsed and used. To use another name for the configuration
create a symbolic link of the application or copy it, eg scautopick \-> scautopick2.

\par \-\-plugins arg
Load given plugins.

\par -D [\-\-daemon]
Run as daemon. This means the application will fork itself and
doesn't need to be started with \&.

\par \-\-auto-shutdown arg
Enable/disable self\-shutdown because a master module shutdown. This only
works when messaging is enabled and the master module sends a shutdown
message \(enabled with \-\-start\-stop\-msg for the master module\).

\par \-\-shutdown-master-module arg
Sets the name of the master\-module used for auto\-shutdown. This
is the application name of the module actually started. If symlinks
are used then it is the name of the symlinked application.

\par \-\-shutdown-master-username arg
Sets the name of the master-username used for auto-shutdown.

### Verbose:
\par \-\-verbosity arg
Verbosity level [0..4]

\par -v [ \-\-v ]
Increase verbosity level (may be repeated, eg. -vv).

\par -q [ \-\-quiet ]
Quiet mode: no logging output.

\par \-\-component arg
Limits the logging to a certain component. This option 
can be given more than once.

\par s [ \-\-syslog ]
Use syslog.

\par -l [ \-\-lockfile ] arg
Path to lock file.

\par \-\-console arg
Send log output to stdout.

\par \-\-debug
Debug mode: \-\-verbosity=4 \-\-console.

\par \-\-log-file arg
Use alternative log file.

### Messaging:
\par -u [ \-\-user ] arg
Client name used when connecting to the messaging.

\par -H [ \-\-host ] arg
Messaging host (host[:port]).

\par -t [ \-\-timeout ] arg
Connection timeout in seconds.

\par -g [ \-\-primary-group ] arg
The primary message group of the client.

\par -S [ \-\-subscribe-group ] arg
A group to subscribe to. This option can be 
given more than once.

\par \-\-encoding arg
Sets the message encoding (binary or xml).

\par \-\-start-stop-msg arg
Sets sending of a start- and a stop message.

### Database:
\par \-\-db-driver-list
List all supported database drivers.

\par -d [ \-\-database ] arg
The database connection string, format: service://user:pwd@host/database.

\par \-\-config-module arg
The configmodule to use.

\par \-\-inventory-db arg
Load the inventory database from a given XML file.

\par \-\-config-db arg
Load the config database from a given XML file.

### Records:
\par \-\-record-driver-list
List all supported record stream drivers.

\par -I [ \-\-record-url ] arg
The recordstream source URL, format: [service://]location[#type].

\par \-\-record-file arg
Specify a file as recordsource.

\par \-\-record-type arg
Specify a type for the records being read.

### Offline:
\par \-\-ts arg
Start time of data acquisition time window, requires also \-\-te.

\par \-\-te arg
End time of data acquisition time window, requires also \-\-ts.

\page scvsm scvsmag
*scvsmag* is part of a new SeisComp3 implementation of the [Virtual Seismologist]( http://www.seismo.ethz.ch/research/vs) 
(VS) Earthquake Early Warning algorithm (Cua, 2005; Cua and Heaton, 2007) released 
under the 'SED Public License for SeisComP3 Contributions' 
(http://www.seismo.ethz.ch/static/seiscomp_contrib/license.txt ). For a given 
origin it estimates single station magnitudes and a network magnitude based on 
the envelope attenuation relationship and ground motion amplitude ratio derived 
by Cua (2005). The original VS algorithm applies the Bayesian theorem by defining magnitude as
the value that maximizes the product of a likelihood function and a prior
probability density function. In the current version of scvsmag only
the likelihood function is implemented and no prior information is used
at this stage.

## Options

*scvsmag* supports commandline options as well as configuration files (scvsmag.cfg)

## Configuration
\par vsmag.siteEffect  (bool) [false]
Choose whether to use Vs30 base site effect corrections (see also the option 
'vsmag.vs30filename')

\par vsmag.vs30filename  (string) [CH_derivedVs30_910_07_sort.txt]  
An ascii grid file of Vs30 values conforming to the standard ShakeMap format.
Each line contains a comma separated list of longitude, latitude and the
VS30 value for one grid point. Longitude and latitude have to be ordered from lower
to higher values and longitudes increase faster than latitudes. For a given station the Vs30 value of the closest grid point will be used. If 
the distance to the closest grid point is further than 0.1 degrees, the default 
Vs30 value will be used. Vs30 is only used to distinguish between rock and 
soil sites (Vs30 > 464 m/s: rock; Vs30 <= 464 m/s soil). Site amplification is 
then computed for the given soil type using the equations of Borcherdt (1994).
Below, a few lines of a correctly formatted Vs30 grid file are shown.

    6.5800,46.1345,910
    6.5935,46.1345,1428
    6.6070,46.1345,1428
    6.6205,46.1345,910
    6.6340,46.1345,1428

\par vsmag.vs30default (float) [910]
Define a default Vs30 value for points not covered by the grid file given with
'vsmag.vs30filename'.

\par vsmag.eventExpirationTime (integer) [45]
This defines the time-span after an event's origin or first creation time during 
which the VS magnitude is re-evaluated every second. After origin-time (or creation time)
plus eventExpirationTime the evaluation will stop.

\par vsmag.ExpirationTimeReference (string) [ct]
Choose whether to time the event expiration time with respect to the origin 
time ('ot') or the time of the first VS estimates creation time ('ct').

\par vsmag.clipTimeout (integer) [30]
Define the number of seconds following a clipped record that a stream is not 
used for magnitude estimation. 

\par vsmag.twstarttime (integer) [4]
\par vsmag.twendtime (integer) [35]
These two parameters define the timewindow around picks in which scvsmag
looks for maximum amplitudes. twstarttime defines the time before the pick
and twendtime the time after the pick

\par vsmag.mode (string) [realtime]
You can choose between 'realtime' and 'playback' mode. In 'realtime' mode VS 
magnitudes are evaluated based on a realtime timer. In 'playback' mode the 
timing is determined by incoming envelope values (i.e. the internal timing is 
always set to the latest envelope arrival. 

\par vsmag.backslots (integer)  [6000]
Time in seconds with respect to the current Time that envelope values are kept
in memory. Envelope values with a timestamp that is older
than current Time - vsmag.backslots will be deleted/rejected.

\par vsmag.headslots (integer) [65]
Time in seconds in the future with respect to the current Time that envelope
values are kept in memory. This feature can be of interest if scenvelope and
scvsmag do not run on the same machine. A difference between the internal
clocks can generate envelope messages with a timestamp in the future
relative to the receiving machine.

\par vsmag.maxepicdist (double) [200.]
This defines a cutoff epicentral distance in kilometers; stations further than that
won't be used for magnitude computation; a negative value means no cutoff is 
applied. The VS equations are not verified beyond an epicentral distance of 200 km.

\par vsmag.logenvelopes (bool) [false]
This toggles envelope logging. Note that this will produce very large files and
may fill up your disk if left on for too long.

## Commandline

### Generic

\par -h [\-\-help]
Produce help message.

\par -V [\-\-version]
Show version information.

\par \-\-config-file arg
Use alternative configuration file. When this option is used
the loading of all stages is disabled. Only the given configuration
file is parsed and used. To use another name for the configuration
create a symbolic link of the application or copy it, eg scautopick \-> scautopick2.

\par \-\-plugins arg
Load given plugins.

\par -D [\-\-daemon]
Run as daemon. This means the application will fork itself and
doesn't need to be started with \&.

\par \-\-auto-shutdown arg 
Enable/disable self\-shutdown because a master module shutdown. This only
works when messaging is enabled and the master module sends a shutdown
message \(enabled with \-\-start\-stop\-msg for the master module\).

\par \-\-shutdown-master-module arg
Sets the name of the master\-module used for auto\-shutdown. This
is the application name of the module actually started. If symlinks
are used then it is the name of the symlinked application.

\par \-\-shutdown-master-username arg
Sets the name of the master-username used for auto-shutdown.

### Verbose:
\par \-\-verbosity arg
Verbosity level [0..4]

\par -v [ \-\-v ]
Increase verbosity level (may be repeated, eg. -vv).

\par -q [ \-\-quiet ]
Quiet mode: no logging output.

\par \-\-component arg
Limits the logging to a certain component. This option 
can be given more than once.

\par s [ \-\-syslog ]
Use syslog.

\par -l [ \-\-lockfile ] arg
Path to lock file.

\par \-\-console arg 
Send log output to stdout.

\par \-\-debug
Debug mode: \-\-verbosity=4 \-\-console.

\par \-\-log-file arg
Use alternative log file.

### Messaging:
\par -u [ \-\-user ] arg
Client name used when connecting to the messaging.

\par -H [ \-\-host ] arg
Messaging host (host[:port]).

\par -t [ \-\-timeout ] arg
Connection timeout in seconds.

\par -g [ \-\-primary-group ] arg
The primary message group of the client.

\par -S [ \-\-subscribe-group ] arg
A group to subscribe to. This option can be 
given more than once.

\par \-\-encoding arg
Sets the message encoding (binary or xml).

\par \-\-start-stop-msg arg 
Sets sending of a start- and a stop message.

### Database:
\par \-\-db-driver-list
List all supported database drivers.

\par -d [ \-\-database ] arg
The database connection string, format: service://user:pwd@host/database.

\par \-\-config-module arg
The configmodule to use.

\par \-\-inventory-db arg
Load the inventory database from a given XML file.

\par \-\-config-db arg
Load the config database from a given XML file.

### Mode:
\par \-\-playback
Sets the current time to the latest received envelope timestamp.

\par \-\-timeref arg
Set whether the expiration time is measured with respect to origin time ('ot') or creation time ('ct').

### Log:
\par \-\-processing-log arg
Set an alternative filename for the processing log-file.

\par \-\-envelope-log
Turn on envelope logging.

## Logging
Apart from the standard log messages in *scvsmag.log*, processing log messages are 
also written to *scvsmag-processing-info.log* every time the VS Magnitude of an event 
is re-evaluated. A typical entry is shown below.

    1  2013/06/28 10:51:01 [processing/info/VsMagnitude] Start logging for event: sed2012cyqr
    2  2013/06/28 10:51:01 [processing/info/VsMagnitude] update number: 0
    3  2013/06/28 10:51:01 [processing/info/VsMagnitude] Sensor: CH..BNALP.HH; Wavetype: P-wave; Soil class: rock; Magnitude: 3.47
    4  2013/06/28 10:51:01 [processing/info/VsMagnitude] station lat: 46.87; station lon: 8.43; epicentral distance: 32.26;
    5  2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(Z): 3.57e-03; PGV(Z): 6.91e-05; PGD(Z): 1.62e-06
    6  2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(H): 2.67e-03; PGV(H): 3.44e-05; PGD(H): 1.02e-06
    7  2013/06/28 10:51:01 [processing/info/VsMagnitude] Sensor: CH..MUO.HH; Wavetype: S-wave; Soil class: rock; Magnitude: 3.83
    8  2013/06/28 10:51:01 [processing/info/VsMagnitude] station lat: 46.97; station lon: 8.64; epicentral distance: 22.45;
    9  2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(Z): 8.19e-03; PGV(Z): 2.12e-04; PGD(Z): 6.91e-06
    10 2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(H): 2.18e-02; PGV(H): 5.00e-04; PGD(H): 1.72e-05
    11 2013/06/28 10:51:01 [processing/info/VsMagnitude] Sensor: CH..WILA.HH; Wavetype: P-wave; Soil class: rock; Magnitude: 3.50
    12 2013/06/28 10:51:01 [processing/info/VsMagnitude] station lat: 47.41; station lon: 8.91; epicentral distance: 41.16;
    13 2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(Z): 4.38e-03; PGV(Z): 6.42e-05; PGD(Z): 1.85e-06
    14 2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(H): 3.35e-03; PGV(H): 6.40e-05; PGD(H): 1.88e-06
    15 2013/06/28 10:51:01 [processing/info/VsMagnitude] Sensor: CH..ZUR.HH; Wavetype: S-wave; Soil class: rock; Magnitude: 3.79
    16 2013/06/28 10:51:01 [processing/info/VsMagnitude] station lat: 47.37; station lon: 8.51; epicentral distance: 23.99;
    17 2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(Z): 9.17e-02; PGV(Z): 1.03e-03; PGD(Z): 1.64e-05
    18 2013/06/28 10:51:01 [processing/info/VsMagnitude] PGA(H): 9.63e-02; PGV(H): 2.12e-03; PGD(H): 5.31e-05
    19 2013/06/28 10:51:01 [processing/info/VsMagnitude] VS-mag: 3.69; median single-station-mag: 3.79; lat: 47.15; lon: 8.52; depth : 25.32 km
    20 2013/06/28 10:51:01 [processing/info/VsMagnitude] creation time: 2012-02-11T22:45:40.00Z; origin time: 2012-02-11T22:45:26.27Z; t-diff: 13.73; time since origin arrival: 0.864; time since origin creation: 0.873
    21 2013/06/28 10:51:01 [processing/info/VsMagnitude] # picked stations: 6; # envelope streams: 79
    22 2013/06/28 10:51:01 [processing/info/VsMagnitude] Distance threshold (dt): 44.68 km; # picked stations < dt: 4; # envelope streams < dt: 4
    23 2013/06/28 10:51:01 [processing/info/VsMagnitude] Stations not used for VS-mag: CH.HASLI CH.LLS 
    24 2013/06/28 10:51:01 [processing/info/VsMagnitude] Magnitude check: 0.027; Arrivals check: 0.000
    25 2013/06/28 10:51:01 [processing/info/VsMagnitude] likelihood: 0.99
    26 2013/06/28 10:51:01 [processing/info/VsMagnitude] End logging for event: sed2012cyqr


### Explanation:

- 1     : Start of the log message for the event with the given event ID
- 2     : Update counter for this event.
- 3 - 18: Information about the stations that contribute to a VS magnitude estimate.
          Each station has four lines with the first line giving the the stream name,
          the wavetype of the contributing amplitude, the soil type at the site
          and the single station magnitude. The next line shows the location and
          epicentral distance of the sensor. On the two following lines
          peak-ground-acceleration (PGA) -velocity (PGV) and -displacement (PGD)
          are given in SI units for vertical and the root-mean-square horizontal component.
- 19    : The VS magnitude, the median of the single station magnitudes, the cordinates of the hypocenter
- 20    : The creation time of the magnitude, the origin time and the difference between the two ('tdiff'); also given
          are the time since origin arrival and time since origin creation which is a measure of how long it took
          to evaluate the first magnitude estimate.
- 21    : The number of stations contributing to an origin ('# picked stations') and the number of
          envelope streams available ('# envelope streams').
- 22    : Distance threshold from epicenter within which the relative difference between picked stations and envelope streams
          is evaluated (see line 24). Also shown is the number of picked stations and envelope streams within this distance threshold.
- 23    : Stations that were used for picking but not for the magnitude estimation.
- 24    : 'Magnitude check' is the relative difference between the VS magnitude and the median of the single station magnitudes.
          If it exceeds a certain threshold the magnitude quality value is set to 0.4 otherwise to 1.0. 'Arrivals check' is the relative difference betweeen the number
          of picked stations and the number of envelope streams contributing to the VS magnitude. If it exceeds a certain threshold the arrivals
          quality criteria is set to 0.3 otherwise to 1.0. The full decision tree for computing the likelihood and the related thresholds is shown [here](@ref lh).
- 25    : The 'likelihood' is the product of the magnitude and the arrivals quality criteria. If both are 1.0 than the likelihood
          is set to 0.99.
- 26    : End of the log message for the event with the given event ID

## References
Borcherdt, R. D., 1994: Estimates of Site-Dependent Response Spectra for Design (Methodology and Justification), Earthquake Spectra

\page scvsml scvsmaglog

*scvsmaglog* is part of a new SeisComp3 implementation of the [Virtual Seismologist]( http://www.seismo.ethz.ch/research/vs) 
(VS) Earthquake Early Warning algorithm (Cua, 2005; Cua and Heaton, 2007) released 
under the 'SED Public License for SeisComP3 Contributions' 
(http://www.seismo.ethz.ch/static/seiscomp_contrib/license.txt ). It logs the VS 
magnitude messages received from *scvsmag* and, once 
an event has timed out, generates report files. These report files are saved 
to disk and can also be sent via email. 

It also provides the possibility to send messages to the UserDisplay, a real-time EEW user 
interface developed by Caltech (http://www.eew.caltech.edu/research/userdisplay.html).
While the UserDisplay is currently not openly available, the message stream from 
this interface can be used. The xml scheme and set-up of an ActiveMQ broker 
necessary to receive the messages is briefly discribed here: \subpage udi

## Options

*scvsmaglog* supports commandline options as well as configuration files (scvsmaglog.cfg)

## Configuration
\par email.smtpserver  (string) [None]  
URL of the smtp server to send the report files to.

\par email.port (integer) [25]
Port where the SMTP server accepts connections.

\par email.usetls (bool) [false]
Whether to use TLS when connecting to the smtp server.

\par email.usessl (bool) [false]
Whether to use SSL when connecting to the smtp server. Note, only 'email.usetls'
or 'user.ssl' can be true. 

\par email.authenticate (bool) [false]
Whether the smtp server requires authentication (username + password) 

\par email.credentials (string) [None]
If the smtp server requires authentication you have to specify a file that contains
username and password in the format:
    
    username=your-username
    password=your-password

Make sure that you set the file permission as restrictive as possible. 

\par email.senderaddress (string) [None]
Email address that will appear as sender in the report email.
 
\par email.subject (string) [None]
Any string that should be prepended to the email's subject string.

\par email.host (string) [None]
Host as it is supposed to appear in the email's subject string.

\par email.recipients (list) [None]
A list of email addresses that receive the report emails.

\par userdisplay.configfile (string) [None]
Path to the configuration file that defines the UserDisplay interface (see 
\subpage udi for details).

\par report.eventbuffer (int) [3600]
Time in seconds that events and the related objects are buffered.

\par report.directory (string) [~/.seiscomp3/log/VS_reports]
Directory to save reports to. 


## Commandline

### Generic

\par -h [\-\-help]
Produce help message.

\par -V [\-\-version]
Show version information.

\par \-\-config-file arg
Use alternative configuration file. When this option is used
the loading of all stages is disabled. Only the given configuration
file is parsed and used. To use another name for the configuration
create a symbolic link of the application or copy it, eg scautopick \-> scautopick2.

\par \-\-plugins arg
Load given plugins.

\par -D [\-\-daemon]
Run as daemon. This means the application will fork itself and
doesn't need to be started with \&.

\par \-\-auto-shutdown arg
Enable/disable self\-shutdown because a master module shutdown. This only
works when messaging is enabled and the master module sends a shutdown
message \(enabled with \-\-start\-stop\-msg for the master module\).

\par \-\-shutdown-master-module arg
Sets the name of the master\-module used for auto\-shutdown. This
is the application name of the module actually started. If symlinks
are used then it is the name of the symlinked application.

\par \-\-shutdown-master-username arg
Sets the name of the master-username used for auto-shutdown.

### Verbose:
\par \-\-verbosity arg
Verbosity level [0..4]

\par -v [ \-\-v ]
Increase verbosity level (may be repeated, eg. -vv).

\par -q [ \-\-quiet ]
Quiet mode: no logging output.

\par \-\-component arg
Limits the logging to a certain component. This option 
can be given more than once.

\par s [ \-\-syslog ]
Use syslog.

\par -l [ \-\-lockfile ] arg
Path to lock file.

\par \-\-console arg
Send log output to stdout.

\par \-\-debug
Debug mode: \-\-verbosity=4 \-\-console.

\par \-\-log-file arg
Use alternative log file.

### Messaging:
\par -u [ \-\-user ] arg
Client name used when connecting to the messaging.

\par -H [ \-\-host ] arg
Messaging host (host[:port]).

\par -t [ \-\-timeout ] arg
Connection timeout in seconds.

\par -g [ \-\-primary-group ] arg
The primary message group of the client.

\par -S [ \-\-subscribe-group ] arg
A group to subscribe to. This option can be 
given more than once.

\par \-\-encoding arg
Sets the message encoding (binary or xml).

\par \-\-start-stop-msg arg
Sets sending of a start- and a stop message.

### Reports
\par \-\-savedir arg
Directory to save reports to. The default is ~/.seiscomp3/log/VS_reports.

## Reports
Below is an example of the first few lines of a report file:

    Mag.|Lat.  |Lon.  |tdiff |Depth |creation time            |origin time              |likeh.|#st.(org.) |#st.(mag.)
    ------------------------------------------------------------------------------------------------------------------
    3.42| 47.15|  8.52| 12.73| 25.32|2012-02-11T22:45:39.0000Z|2012-02-11T22:45:26.2729Z|  0.99|          6|         6
    3.43| 47.15|  8.52| 13.73| 25.32|2012-02-11T22:45:40.0000Z|2012-02-11T22:45:26.2729Z|  0.99|          6|         6
    3.56| 47.15|  8.54| 14.70| 25.73|2012-02-11T22:45:41.0000Z|2012-02-11T22:45:26.3032Z|  0.99|         10|        10
    3.64| 47.16|  8.54| 15.58| 24.32|2012-02-11T22:45:42.0000Z|2012-02-11T22:45:26.4178Z|  0.99|         12|        12
    3.54| 47.16|  8.53| 16.45| 22.40|2012-02-11T22:45:43.0000Z|2012-02-11T22:45:26.5547Z|  0.99|         14|        14
    3.67| 47.15|  8.54| 17.29| 20.40|2012-02-11T22:45:44.0000Z|2012-02-11T22:45:26.7142Z|  0.99|         16|        16
    3.66| 47.16|  8.54| 18.34| 21.31|2012-02-11T22:45:45.0000Z|2012-02-11T22:45:26.6562Z|  0.99|         18|        18
    3.75| 47.16|  8.54| 19.27| 19.91|2012-02-11T22:45:46.0000Z|2012-02-11T22:45:26.7326Z|  0.99|         19|        19
   
*Creation time* is the time the VS magnitude message was generated, *tdiff* is the 
time difference between *creation time* and *origin time* in seconds, *likeh* is the 
likelihood that this event is a real event (see documentation of the *scvsmag* module), # *st.(org)* 
is the number of stations that contributed to the origin and # *st.(mag)* the number of envelope streams
that contributed to the magnitude.

\page lh Solution quality

@image html VS_likelihood_scheme_v1.2.png 

\page udi UserDisplay interface

## Configuration:
\par host (string) [None]
This is the address of the server running the ActiveMQ broker

\par port (integer) [None]
Port at which the ActiveMQ broker listens for messages.

\par username (string) [None]
\par password (string) [None]
Username password necessary to connect to the ActiveMQ broker.

\par topic (string) [None]
Broker topic to send event messages to.

\par topichb (string) [None]
Broker topic to send hearbeat messages to.

## Event messages:
Event messages are sent once a first magnitude estimate is available. The 
'message_type' of the first message is 'new', and for any successive message it's
either 'update' or 'delete'. Currently all values except the uncertainty 
estimates will be set by *scvsmaglog*.

    <?xml version='1.0' encoding='UTF-8'?>
    <event_message message_type="new" orig_sys="dm" version="0">
        <core_info id="-9">
            <mag units="Mw">-9.9</mag>
            <mag_uncer units="Mw">-9.9</mag_uncer>
            <lat units="deg">-999.9</lat>
            <lat_uncer units="deg">-999.9</lat_uncer>
            <lon units="deg">-999.9</lon>
            <lon_uncer units="deg">-999.9</lon_uncer>
            <depth units="km">-9.9</depth>
            <depth_uncer units="km">-9.9</depth_uncer>
            <orig_time units="UTC">2013-06-10T13:35:12Z</orig_time>
            <orig_time_uncer units="sec">-9.9</orig_time_uncer>
            <likelihood>-9.9</likelihood>
        </core_info>
    </event_message>


## Heartbeat messages
Heartbeat messages are sent in 5 s intervals.

    <?xml version='1.0' encoding='UTF-8'?>
    <hb originator="vs.9" sender="vs.9" timestamp="Mon June 10 13:41:35 2013" />

## ActiveMQ broker configuration
It is beyond the scope of this documentation to explain the complete setup of an
ActiveMQ broker. However, since scvsmaglog uses the STOMP protocol to send 
messages to the broker it is essential to install the stompy package, which 
provides Python bindings for the STOMP protocol, and to add the following line 
to configuration of the ActiveMQ broker.

    <connector>
    <serverTransport uri="stomp://your-server-name:your-port"/>
    </connector>

