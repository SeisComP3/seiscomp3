scautoloc is the SeisComP3 program responsible for automatically locating
seismic events in near-real time. It normally runs as a daemon continuously
reading picks and amplitudes and processing them in real time. An offline
mode is available as well. scautoloc reads automatic picks and several
associated amplitudes. On that basis it tries to identify combinations of
picks that correspond to a common seismic event. If the produced location
meets certain consistency criteria, it is reported, i.e. passed on to other
programs that take the origins as input.

Location procedure
==================

The procedure of scautoloc to identify and locate seismic events basically
consists of the following steps:
 
Pick preparation 
----------------

In scautoloc each incoming pick needs to be accompanied by a specific set
of amplitudes. Since in the SeisComP3 data model amplitudes and picks are
independent objects, the amplitudes are added as attributes to their
corresponding picks upon reception by scautoloc.

Pick filtering 
--------------

Each incoming pick is filtered, i.e. it is checked if a pick is outdated
and if the complete set of associated amplitudes is present already. If
a station produces picks extremely often, these are considered to be more
likely glitches and result in an increased SNR threshold.

Association 
-----------

It is first attempted to associate an incoming pick with the known origins.
Especially for large events with stable locations based on many picks already
associated, this is the preferred way to handle the pick. If the association
succeeds, the nucleation process can be bypassed. Under certain circumstances
picks are both associated and fed into the nucleator.

Nucleation 
----------

If direct association fails, scautoloc tries to make a new origin out of this
and other unassociated, previously received picks. This process is called
"nucleation". scautoloc performs a grid search over space and time, which is
a rather expensive procedure as it requires lots of resources both in terms
of CPU and RAM. Additional nucleation algorithms will become available in
future. The grid is a discrete set of -in principle- arbitrary points that
sample the area of interest sufficiently densely. In the grid search, each
of the grid points is taken as a hypothetical hypocenter for all incoming
picks. Each incoming pick is back projected in time for each of the grid
points, on the assumption that it is a first-arrival "P" onset. If the pick
indeed corresponds to a "P" arrival of a seismic event, and if this event was
recorded at a sufficient number of stations, the back projected new pick will
cluster with previous picks from the same event. The cluster will be densest
around the origin time at the grid point closest to the hypocenter. In
principle, the grid could be so dense that the location obtained from the
grid search can be used directly. However, as RAM memory as well as CPU speed
is limited, this is not possible. Therefore, if a cluster is identified as a
potential origin, it does not necessarily mean that all contributing picks
actually correspond to "P" arrivals. It may as well be a coincidental match
caused by the coarseness of the grid or possible contamination by picked noise.
Therefore, a location program (LocSAT) is run in order to try a location and
test if the set of picks indeed forms a consistent hypocenter. If the pick
residual RMS is too large, an improvement is attempted by excluding each of
the contributing picks once to test if a reduction in RMS can be achieved.
If the new origin meets all requirements, it is accepted as new seismic event
location.[[br]]The grid points are specified in a text file "grid.txt".
The default file shipped with scautoloc defines a grid with globally even
distributed points at the surface, and depth points confined to regions of
known deep seismicity. It may be modified, but should not comprise too many
grid points (>3000, depending on CPU speed and RAM). See below for more
details about the grid file.

Origin refinement 
-----------------

An origin produced or updated through association and/or nucleation may still
be contaminated by phases wrongly interpreted as "P" arrivals. scautoloc
tries to improve these origins based on e.g. pick SNR and amplitude. In this
processing step, it is also attempted to associate phases which slipped through
during the first association attempt, e.g. because the initial location was
incorrect. If the origin contains a sufficient number of arrivals to assume
a reasonably well location result, scautoloc additionally tries to associate
picks as secondary phases such as "pP". Such secondary phases are only "weakly
associated", i.e. these phases are not used for the location. For the analyst,
however, it is useful to have possible “pP” phases predefined.

Origin filtering
----------------

This process involves final consistency checks of new/updated origins etc.
During this procedure, the origins are not modified any more.

In the course of nucleation and association, as well as in the origin
refinement and filtering, certain heuristic criteria are applied to compare
the "qualities" of concurring origins. These criteria are combined in an
internal origin score, which is based on properties of the picks themselves
in the context of the respective origin (residuals, RMS, azimuthal gaps).
In addition, the amplitudes provide valuable means of comparing origin
qualities. Obviously, a pick with a high SNR will less likely be a transient
burst of noise than a pick merely exceeding the SNR threshold. A high-SNR
pick thus increases the origin score. Similarly, a pick associated to a large
absolute amplitude is more likely to correspond to a real seismic onset,
especially in case of simultaneous, large-amplitude observations at neighboring
stations. A special case arises, when several nearby stations report amplitudes
above a certain “XXL threshold”. For details see the section
"Preliminary origins".
The amplitudes used by scautoloc are of type "snr" and "mb", corresponding
to the (relative, unit-less) SNR amplitude and the (absolute) "mb" amplitude,
respectively. These two amplitudes are provided by [#scautopick scautopick].
In case of a setup in which scautopick is replaced by a different automatic
picker, these two amplitudes must nevertheless be provided to scautoloc.
Otherwise, the picks are not used. At the moment this is a strict requirement,
in the future it may be changed.


Grid file
=========

The grid configuration file consists of one line per grid point, each grid
point specified by 6 columns::

    -10.00 105.00 20.0 5.0 180.0 8

The columns are grid point coordinates (latitude, longitude, depth), diameter,
maximum station distance and minimum pick count, respectively. The above line
sets a grid point centered at 10° S / 105° E at the depth of 20 km. It is
sensitive to events within 5° of the center. Stations in a distance of up
to 180° may be used to nucleate an event. At least 8 picks have to contribute
to an origin at this location. The diameter should be chosen large enough to
allow grid cells to overlap, but not too large. The size also determines the
time windows for grouping the picks in the grid search. If the time windows
are too long the risk of contamination with wrong picks increases. The maximum
station distance allows to restrict to certain stations for the according grid
points. E.g. stations from Australia are normally not required to create an
event in Europe. If there is doubt, set the value to 180. The minimum pick
count specifies how many picks are required for a given grid point to allow
the creation of a new origin. The default grid file contains a global grid
with even spacing of ~5° with additional points at greater depths where
deep-focus events are known to occur.

Station configuration file
==========================

The station configuration file contains lines consisting of network code,
station code, usage flag (0 or 1) and maximum nucleation distance. A usage
flag of 1 indicates the station shall be used by scautoloc. If it shall not
be used, 0 must be specified here. The maximum nucleation distance is the
distance (in degrees) from the station up to which this station may contribute
to a new origin. If this distance is 180°, this station may contribute to new
origins world-wide. However, if the distance is only 10°, the range of this
station is limited. This is a helpful setting in case of mediocre stations
in a region where there are numerous good and reliable stations nearby. The
station will then not pose a risk for locations generated outside the maximum
nucleation distance. Network and station code may be wildcards (\*) for
convenience ::

    * * 1 90 
    GE * 1 180
    GE HLG 1 10 
    TE RGN 0 10

The example above means that all stations from all networks by default can
create new events within 90°. The GE stations can create events at any distance,
except for the rather noisy station HLG in the network GE, which is restricted
to 10°. By setting the 3rd column to 0, TE RGN is ignored by scautoloc.

Preliminary origins
===================

Usually, scautoloc will not report origins with less than a certain
number of defining phases (specified by autoloc.minPhaseCount),
typically 6-8 phases, with 6 being the absolute minimum.  However,
in case of potentially dangerous events, it may be desirable to
receive "heads up" alert prior to reaching the minimum phase count,
especially in a tsunami warning context. If very large amplitudes
are registered at a sufficient number of stations, it is possible to
produce preliminary origins (hereafter called ~@~\XXL events~@~])
based on less than 6 picks.[[br]]Prerequisite is that all these
picks have extraordinary large amplitudes and SNR and lie within a
relatively small region. Such picks are hereafter called “XXL
picks”. A pick is internally tagged as “XXL pick” if its
amplitude exceeds a certain threshold (specified by
autoloc.thresholdXXL) and has a SNR > 8. For larger SNR picks with
smaller amplitude can reach the XXL tag, because it is justified to
treat a large-SNR pick as XXL pick even if its amplitude is somewhat
below the XXL amplitude threshold. The XXL criterion should be
judged as workaround to identify picks which justify the nucleation
of preliminary origins.

Logging
=======

scautoloc produces two kinds of log files: a normal application log file
containing the processing and location history and an optional pick log.
The pick log contains all received picks with associated amplitudes in a
simple text file, one entry per line. This pick log should always be active
as it allows pick playback for trouble shooting and optimization of scautoloc.
If something did not work as expected, playing back the pick log will provide
a useful way to find the source of the problem without the need of processing
the raw waveforms again. The application log file contains miscellaneous
information in variable format. The format of the entries may change anytime,
so no downstream application should ever depend on it. There are some special
lines, however. These contain certain keywords that allow convenient filtering
of the most important information using grep. These keywords are NEW, UPD and
OUT, for a new, updated and output origin, respectively. They can be used like::

    grep '\(NEW\---UPD\---OUT\)' ~/.seiscomp3/log/scautoloc.log

This will extract all lines containing the above keywords, providing a very
simple (and primitive) origin history.

Publication interval
====================

In principle, scautoloc produces a new solution (origin) after each processed
pick. This is desirable at an early stage of an event, when every additional
information may lead to significant improvements. A consolidated solution,
consisting of many (dozens) of picks, on the other hand may not always benefit
greatly from additional picks that usually originate from large distances.
Updates after each pick are therefore unnecessary. It is possible to control
the time interval between subsequent origins reported by scautoloc. The time
interval is a linear function of the number of picks::

    Δt = aN + b

Setting a = b = 0, then Δt is always zero, meaning there is never a delay in
sending new solutions. This is not desirable. Setting a = 0.5, each pick will
increase the time interval until the next solution will be sent by 0.5s. This
means that scautoloc will wait 10 seconds after an origin with 20 picks is sent.

Housekeeping
============

scautoloc keeps objects in memory only for a certain amount of time. This time
span is specified in seconds in autoloc.maxAge. The default value is 21600
seconds (6 hours). After this time, unassociated picks expire. Newly arriving
picks older than that (e.g. in the case of high data latencies) are ignored.
Origins will live slightly longer, including the picks associated to them.
In a setup where many stations have considerable latencies, e.g. dialup
stations, the expiration time should be chosen long enough to accommodate
late picks. On the other hand, the memory usage for large networks may be a
concern as well. scautoloc periodically cleans up its memory from expired
objects. The time interval between subsequent housekeepings is specified in
autoloc.cleanupInterval in seconds.

Test mode
=========

In the test mode, scautoloc connects to a messaging server as usual and
receives picks and amplitudes from there, but no results are sent back to
the server. Log files are written as usual. This mode can be used to test
new parameter settings before implementation in the real-time system. It also
provides a simple way to log picks from a real-time system to the pick log.

Offline mode
============

scautoloc normally runs as a daemon in the background, continuously reading
picks and amplitudes and processing them in real time. However, scautoloc
may also be operated in offline mode. This is useful for debugging. Offline
mode is activated by setting autoloc.offline to true or by adding the
parameter --offline to the command line. When operated in offline mode,
scautoloc will connect neither to the messaging nor to the database. Instead,
it reads picks in the pick file format from standard input. Example for
entries in a pick file::
  
    2008-09-25 00:20:16.6 SK LIKS EH __ 4.6 196.953 1.1 A [id]
    2008-09-25 00:20:33.5 SJ BEO BH __ 3.0 479.042 0.9 A [id]
    2008-09-25 00:21:00.1 CX MNMCX BH __ 21.0 407.358 0.7 A [id]
    2008-09-25 00:21:02.7 CX HMBCX BH __ 14.7 495.533 0.5 A [id]
    2008-09-24 20:53:59.9 IA KLI BH __ 3.2 143.752 0.6 A [id]
    2008-09-25 00:21:04.5 CX PSGCX BH __ 7.1 258.407 0.6 A [id]
    2008-09-25 00:21:09.5 CX PB01 BH __ 10.1 139.058 0.6 A [id]
    2008-09-25 00:21:24.0 NU ACON SH __ 4.9 152.910 0.6 A [id]
    2008-09-25 00:22:09.0 CX PB04 BH __ 9.0 305.960 0.6 A [id]
    2008-09-25 00:19:13.1 GE BKNI BH __ 3.3 100.523 0.5 A [id]
    2008-09-25 00:23:47.6 RO IAS BH __ 3.1 206.656 0.3 A [id]
    2008-09-25 00:09:12.8 GE JAGI BH __ 31.9 1015.304 0.8 A [id]
    2008-09-25 00:25:10.7 SJ BEO BH __ 3.4 546.364 1.1 A [id]

where [id] is a placeholder for the real pick id which has been omitted in this
example.

.. note:: In the above example some of the picks are not in right order of
   time because of data latencies. In offline mode scautoloc will not connect to
   the database, in consequence the station coordinates cannot be read from the
   database and thus have to be supplied via a file. The station coordinates file
   has a simple format with one line per entry, consisting of 5 columns: network
   code, station code, latitude, longitude, elevation (in meters), e.g., ::
  
       GE APE 37.0689 25.5306 620.0
       GE BANI -4.5330 129.9000 0.0
       GE BKB -1.2558 116.9155 0.0
       GE BKNI 0.3500 101.0333 0.0
       GE BOAB 12.4493 -85.6659 381.0
       GE CART 37.5868 -1.0012 65.0
       GE CEU 35.8987 -5.3731 320.0
       GE CISI -7.5557 107.8153 0.0
  
The location of this file is specified in autoloc.stationLocations or on the
command line using --station-locations


How to make Autopick and Autoloc work together
==============================================

The two main programs in the automatic event detection and location processing
chain, scautopick and scautoloc, only work together if the information needed
by scautoloc can be supplied by scautopick. This document explains current
implicit dependencies between these two utilities and is meant as a guide
especially for those who plan to modify or replace one or both of these
utilities by own developments.

Both scautopick and scautoloc are subject to ongoing developments.
The explanation given below can therefore only be considered a hint, but not
a standard.

Picks
-----

The data scautoloc works with are primarily seismic phase picks. In addition,
certain amplitudes are used as a kind of quality criterion for the pick, allowing
picks with a higher absolute amplitude or signal-to-noise ratio to be given
priority in the processing over weak low-quality picks.

Currently scautoloc only processes automatic, 1st-arrival P picks. Furthermore,
in the current version of scautopick only P picks are produced anyway. It can
therefore be safely assumed by scautoloc that any automatic pick is a P pick
that either has a phaseHint attribute explicitly stating "P" ot the phaseHint
attribute left empty. Automatic picks with a phaseHint other than "P" as well
as any picks not tagged as automatic are currently ignored. It is thus highly
recommended to always set the phaseHint attribute with the appropriate phase
name. There is no restriction regarding the choice of the publicID of the pick.

Optionally scautoloc performance may be improved by processing certain
amplitudes accompanying the picks. Two kinds of amplitudes may be used together

* an absolute amplitude like the one used for calculation of the magnitude "mb"
* relative amplitude like the dimension-less signal-to-noise ratio amplitude "snr"

Neither amplitude is used for magnitude computation by scautoloc. The default
amplitude types used by scautoloc are of type "mb" and "snr". These defaults
can be overridden in scautoloc.cfg:

.. code-block:: sh

   autoloc.amplTypeSNR = snr
   autoloc.amplTypeAbs = mb

If for instance an alternate picker implementation doesn't produce "mb"-type
absolute amplitude but e.g. "xy", then autoloc.amplTypeAbs needs to be set to
"xy" to have them recognized by scautoloc.

Currently there **must** be an absolute and a relative amplitude for every pick.
However, this requirement will be relaxed in a future version. But currently
scautoloc will always wait until both amplitude have arrived, which results
in an overall processing delay, corresponding to the usually delayed availability
of amplitudes with respect to the corresponding pick. The default absolute
amplitude "mb", for instance, takes a hard-coded 30-seconds time interval to
be computed. This length of data thus has to be waited for, plus a little
extra because of the size of the MiniSEED records. An alternate picker
implementation could produce a different absolute-amplitude type than "mb".
That amplitude might be based on a different filter pass band and much shorter
time window than the default "mb" amplitude, thus allowing a significantly
improved processing speed. The choice of amplitude type and time window greatly
depends on the network. For a regional or even global network the 30-seconds
processing delay won't play a role, and we need the mb amplitude anyway. Here
the delay of solutions produced by scautoloc is mostly controlled by the seismic
traveltimes. Not so in case of a local or small-regional network, where the
mb-type amplitude is of limited value and where a meaningful absolute amplitude
might well be produced with just a second of data and at higher frequencies.
Currently this isn't possible with scautopick but this issue will be addressed
in a future version.
