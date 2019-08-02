# Jakarta

## Release YYYY.ddd

* seedlink

  * Fix duplicate mapping detection in scream_plugin_ring plugin. Whenever a duplicate stream
    id was given then it was ignored regardless of the attached sysid.

* scolv

  * Fix event type list preparation in combination with `olv.commonEventTypes`.

## Release 2018.327 patch15

* ql2sc

  * Enable sending of keep-alive messages by default. This prevents connection resets by firewall
    on long idle periods.
  * Fix bug that prevents forwarding updates if the routing must be resolved via the parent object
    which hasn't updated. A workaround is to explicitly specify routing rules on all object levels.

* python-apps

  * Add simple script to dump public objects

* scmag

  * Avoid setting a network magnitude to NaN (not supported with any database)
    and use 0 instead. In order to detect if a network magnitude is valid one could
    use the station count (0 == invalid). But that is in general a hack for the
    time being and the correct solution is to naje the Magnitude.value an
    optional quantity.

* GUI

  * Improve spectrogram rendering speed
  * Normalize spectrogram spectral amplitudes with respect to
    sampling rate

* scdispatch

  * Fix routing table lookup when dealing with merge operation

* ql2sc

  * Enable sending of keep-alive messages by default. This prevents connection resets by firewall
    on long idle periods.
  * Fix bug that prevents forwarding updates if the routing must be resolved via the parent object
    which hasn't updated. A workaround is to explicitly specify routing rules on all object levels.
  * Add event attribute synchronization per input host

## Release 2018.327 patch14

* sh2proc

  * Add parameters parsed from Seismic Handler to SeisComP3.·
    Thanks to Klaus Stammler for providing valuable information on Seismic Handler.

* trunk

  * sdsarchive support multiple archives to read its files from

* fdsnws

  * Add service specific version string to error messages

* scevtstreams

  * Add ```--input``` and ```--format``` options to read event parameters·
    from file
  * Add asymmetric time margins

* scmag

  * Add ```--warning``` flag to output a warning for standard deviations of
    network magnitudes exceeding the provided value.

* GUI

  * Fix spectrogram rendering with logarithmic scale

## Release 2018.327 patch13

* trunk

  * Add distance and depth range confguration to amplitude profiles
    in scconfig
  * Fix PostgreSQL database plugin to unescape bytea encoded data

* scmergexml

  * New module: Merge the content of multiple XML files in SC3ML format. Currently only
    event parameters are supported.

* scmag

  * Add ```--keep-weights``` flag to retain station magnitude weights
    when reprocessing with ```--static```

* scautoloc

  * Fix station lookup with inventories with overlapping network epochs
  * Do not use database if inventory should be read from a file

* fdsnws

  * Fix dataselect restricted handling with respect to station service

    * Require valid user if network or station is restricted
    * Stop iteration at network or station level if restricted flag but
      no user is present 

* fdsnws

  * Fix dataselect restricted handling with respect to station service

    * Require valid user if network or station is restricted
    * Stop iteration at network or station level if restricted flag but
      no user is present

  * Implement service specific version numbers 

## Release 2018.327 patch12

* scamp

  * Do not reprocess manual amplitudes with ```--reprocess``` by default
  * Add ```--force``` flag to reprocess also manual amplitudes

* scmag

  * Add ```--static``` flag which only updates existing magnitudes based
    on their associated amplitudes

* seedlink

  * caps\_plugin does not enforce in-order data by default and discards
    records with timestamps too far in future from being added to the
    state file (both parameters are configurable)

* scolv

  * Add amplitude, created and updated columns to station magnitude table
  * Color station magnitude symbols with respect to evaluation mode of its
    associated amplitude
  * Increase plot symbol sizes to 50% of the applications base font size

* scautoloc

  * Fixed a bug that occasionally caused scautoloc to merge events
    incorrectly only in case autoloc.useManualOrigins is active.

## Release 2018.327 patch11

* scmag

  * Update existing station magnitudes with ```--reprocess```

## Release 2018.327 patch10

* trunk

  * Fixed SDS archive bug (recordstream sdsarchive://) which caused incomplete data retrieval
    under some circumstances

* scautoloc

  * Fixed a bug that occasionally caused scautoloc to segfault.

* scautopick

  * Add playback option to module configuration

* scolv

  * Fixed a bug that caused scolv to crash if the weight of an arrival is not set
  * Added two new optional columns to arrival table: slowness residual and back azimuth residual
  * Added option ```olv.commit.eventCommentOptions``` to defined predefined event comment values
    which can be selected from a drop-down list
  * Allow to sort the "Used" column in arrival table
  * Update map residual colors and residual plot rect if the magnitude was recomputed
  * Show amplitude values in amplitude picker if only QC fails
  * Compute magnitudes
  * Preserve channel code in picker if only the polarity or uncertainy of an existing pick
    was changed

* scquery

  * Add more examples to query events and some statistics from the database 

* scmag

  * Load inventory which is necessary for magnitude implementation which require access to
    station meta data
  * Added command line option ```--reprocess``` to re-process magnitudes which have been
    created manually. This is in particular important for XML processing.
  * Fixed magnitude calculation if only zero-weight station magnitudes were associated
    e.g. due to failed QC

* nuttli plugin

  * Fixed magnitude computation which caused invalid values being treated as valid values

* scbulletin

  * Fix evaluation of command line option ```--weight```
  * Show only station magnitudes associated with network magnitudes
  * Remove amplitude type filter and output all amplitudes which are associated with printed
    station magnitudes

* scxmldump

  * Export all pick amplitudes if station magnitudes should be omitted

* msrtsimul

  * Add documentation

## Release 2018.327 patch9

* Seedlink Q330 plugin

  * Reverted upgrade to revision 99 which caused a segmentation fault. We
    will investigate into the issue and contact the authors of the library.
    For the time being we downgrade to the last working version.

## Release 2018.327 patch8

* scquery

  * Add example to documentation. Thanks to Tiziana on https://forum.seiscomp3.org

* scamp

  * Fix order of processor setup calls to correctly enable e.g. AMN

* scmv

  * Set correct legend if started with ```--displaymode```

* scolv

  * If arrival state (active/inactive) is taken from the weight
    then it now compares against 0 instead of < 0.5 (inactive)
    and > 0.5 (active). It otherwise shows sometimes inconsistent
    results in combination with locators that assign non binary
    weight such as NonLinLoc.

* Hypo71

  * Increase maximum phase number to 1001
  * Raise an error if no velocity model is given for a profile

## Release 2018.327 patch7

```SC_API_VERSION 12.1.0```

* trunk

  * Split ```amplitudes.ML.measureType``` for ML and MLv. MLv must be configured
    with ```amplitudes.MLv.measureType```.
  * Added ```amplitudes.ML.combiner``` to configure how the amplitudes of either
    horizontal component are combined to the final amplitude.
  * Add blockette 1001 when writing miniSEED with MSeedRecord
  * Fixed Hypo71 bug that caused a crash if no phase was part of a solution

## Release 2018.327 patch6

* seedlink

  * Fix caps\_plugin to not send incompatible miniSEED records (!= 512)

## Release 2018.327.patch5

* scolv

  * Fix crash when artificial origin is received sent from e.g. scrttv. This
    is a regression from 2018.327.patch4.

## Release 2018.327.patch4

* system

  * Updated libmseed to 2.19.6

* trunk

  * Fix bug in ims10 export if AgencyID contains white spaces
  * Fixed bug with hyp71sum2k and ims10 export that caused truncated strings.
    Many thanks to Luca Scarabello (ETHZ) for finding and fixing this bug.

* seedlink

  * Added caps\_plugin which allows to retrieve data from gempa's CAPS
    server, for example to import RaspberryShake data

* scolv

  * Added toggle to amplitude picker to either override the configured minimum
    SNR per station or not. The default is now to use what is configured.

* scevent

  * The evrc plugin does not change the status of events with manual preferred origin

* scautoloc

  * Fixed a bug that sometimes caused manual origins not to be used with the correct depth.
  * Minor cleanups.

## Release 2018.327.patch3

* trunk

  * Add MN magnitude plugin (still experimental)

* scolv

  * Keep showing waveform components that do not depend on other components
    for transformation. Vertical components which are the only components
    of a station will still be visible with any rotation selected if they
    are properly aligned upwards.

## Release 2018.327 patch2

* GUI

  * Allow to configure the N top populated places to be shown in the
    cities map layer

* trunk

  * Fix infinite loop when building a quadtree of some geo region sets

## Release 2018.327 patch1

* fdsnws

  * Fix potential security issue

* scheli

  * Improve anti-aliased trace rendering

* GUI

  * Add option to map context menu to save an image of the current view

## Release 2018.327

```SC_API_VERSION 12.0.0```
```Schema version 0.11```

----

**The ML amplitude calculation has changed because of a bug that existed in
previous versions.** This bug caused a scaling of two for the zero-to-peak
amplitudes. This is fixed now and it results in ML magnitudes that are
log10(2) ~= 0.3 lower. If you want to preserve the former behaviour, apply a
magnitude correction. Note that **it only affects ML, not MLv and not MLh**.

----

* scevtstreams

  * Remove duplicate lines in output
  * Add capstool format for output
  * Add options for using wildcards instead of network, stations, streams codes

* scautoloc

  * Check the depth of the origin before sending to discard solutions with
    too great depth
  * Add configuration parameter maxDepth
  * Remove depth check from previous origin filter where it was not effective
  * Changes affecting behaviour in playback mode:
    * A time stamp is now logged whenever an new object arrives.
    * The creationTime of new origins is set to the creation
      time of the most recently received object.

    Both changes help to more realistically simulate and assess the real-time behaviour.

  * Fix bug that caused autoloc.useManualOrigins to always be treated
    as true
  * Fix bug that caused autoloc.useManualPicks to not be used as specified in
    config but rather always treated as false (causing this option to only work
    when calling scautoloc from command line with --use-manual-picks)

* scdumpcfg

  * Add the documentation

* bindings2cfg

  * Add the documentation

* trunk

  * Add namespace support in configuration files. Example:
    ```
    recordstream {
      service = slink
      source = localhost:18000
    }
    ```
    Namespaces can be nested.
  * Remov LocSAT unused configuration option to use the location rms as
    time error
  * Add LocSAT options ```defaultTimeError``` and ```usePickUncertainties```
    which can be configured via the configuration files or during runtime in
    the scolv locator setup dialog. The latter defaults to false to preserve
    the old behaviour
  * Add ConfigSyncMessage which wraps a database configuration synchronization
  * All applications are now encouraged to use ```recordstream = service://params```
    in their configuration rather than the two parameters ```recordstream.service```
    and ```recordstream.source```. They are now declared deprecated and will be
    removed in future versions.
  * All applications are now encouraged to use ```database = type://params```
    in their configuration rather than the two parameters ```database.type```
    and ```database.parameters```. They are now declared deprecated and will be
    removed in future versions.
  * Fix amplitude computation for ML (on horizontals, not MLv) which applied
    a wrong correction factor of 2. Therefore ML magnitudes will be 0.3 magnitudes
    lower than before. To preserve the old (wrong) behaviour, apply a constant
    correction in the bindings:
    ```
    # Add log10(2) to ML magnitudes for SC3 compatibility with <= 2017.334
    mag.ML.offset = 0.301029995664
    ```
  * Add scardac module, see below

* fdsnws

  * Add FocalMechanism support in event query
  * Add support for cross-referenced preferred magnitude in event query
  * Add data availability support according to the IRIS webservice
    availability http://service.iris.edu/irisws/availability/1/ under
    path ```ext/availability```
  * Fix memory leak which was caused by active request tracking

* fdsnxml2inv

  * Fix wrong datalogger conversion if commandline option ```--log-stages```
    is not set

* GUI

  * Add event layer legend which will show if legends are visible in scolv
  * scmv uses now the map legend interface rather than rendering the legends
    as map symbols
  * Add support for BNA point rendering. New configuration options have
    been added: ```[{prefix}.]symbol.size``` and ```[{prefix}.]symbol.shape```.
  * Show in scconfig header of module configuration if bindings are required
  * Fix event list "Hide events outside region" edit dialog: OK and Cancel
    did not work
  * Prepend "amax" and "mean" to the statistic values printed in the lower
    left corner in the trace widgets
  * Improve rendering of traces significantly when anti-aliasing is enabled
    which is default now
  * Improved optimizing traces for rendering with respect to correctness and
    memory consumption

* scheli

  * Add documentation and descriptions
  * Add option to configure a post processing script that is called after an image has
    been written to disc

* scmv

  * Add filter of origin mode and status for showing events on map
  * Add configuration for map/event legend position
  * Improve documentation

* scwfparam

  * Fix wrong handling of data acquisition timeouts when no data is received
    within one second
  * Compare inventory gain.unit ignoring the case to detect velocity and
    acceleration channels correctly

* scbulletin

  * In enhanced mode all coordinates and distances have precisions
    of e-05 degree

* scart

  * Change default archive path from $SEISCOMP_ROOT/acquisition/archive to
    $SEISCOMP_ROOT/var/lib/archive. Thanks to Sergio Tardioli.

* scqc

  * Fix duplicate object bug which caused many error log messages in
    scmaster.log with respect to database insertions

* scolv

  * Allow to configure preferred event types which will be displayed first
    in the event type drop-down list
  * Fix rename phases command when only a subset of source phases is selected
  * Show vertical scale in picker zoom trace
  * Set default for hiding disabled stations to ```false```
  * Add configuration option to either ignore disabled stations or not
  * Allow to show all traces in the same unit, either acceleration, velocity
    or displacement
  * Add travel time table selection, as default libtau (iasp91 and ak135) and
    LocSAT (iasp91, tab) can selected
  * Fix synchronization of event type drop-down in event editor when the
    event type was changed through "With additional options" commit dialog
  * Add hotkey 't' to toggle showing all three components in the current
    trace widget or just the active component
  * Add checkbox to toggle spectrogram display between smooth and nearest neighbor

* scmm

  * Fix crash when connection cannot be established at startup

* scardac

  * Initial version of the module which collect availability information
    from an SDS archive by scanning its content repeatedly and populating
    the new availability database tables (read by fdsnws)

## Release 2017.334 patch11

* seedlink

  * Fix caps\_plugin to not send incompatible miniSEED records (!= 512)

## Release 2017.334 patch10

* trunk

  * Fix bug in ims10 export if AgencyID contains white spaces

* seedlink

  * Added caps\_plugin which allows to retrieve data from gempa's CAPS
    server, for example to import RaspberryShake data

## Release 2017.334 patch9

* trunk

  * Fix bug in stringify functions which discards the last character if
    the output length of a string should be exactly 64 charaters. This
    affects the hyp71sum2k and ims10 output.

## Release 2017.334 patch8

 * gui

   * Fix bug that caused a segfault when GUI application are run in TTY mode
     and the database connection is configured in the configuration file

## Release 2017.334 patch7

* fdsnws

  * Fix potential security issue

## Release 2017.334 patch6

* scesv

  * Add new script option ```exportMap``` that allows to export the current map to
    file. The script has to take ownership of the file. This option is disabled by
    default.

## Release 2017.334 patch5

* scart

  * Support loading of plugins via scart.cfg

* trunk

  * Fix sdsarchive handler with respect to corrupt files which caused
    and endless loop
  * Add parameter splitTime to sdsarchive which is an absolute time rather
    than a relative time
  * Add sh2proc script which converts Seismic Handler event files to
    EventParameters

* Seedlink

  * Fix scream_plugin scream2sl.map parser if a stream id is composed
    from sysid.streamid. The internal parser has rejected lines which
    duplicate streamid without taking the sysid into account.
  * Enable modbus support in serial plugin

* Arclink

  * Fix crash of Python components when inventory comments are used
  * Fix re-generation of empty or conflicting publicIDs

* scevent

  * Fix sending updates if automatic fake event declaration is activated
    and the event.type would actually not change

* slmon

  * Fix default paths and streams regular expression which is used to gather
    station information

* fdsnws

  * Set default data source to sdsarchive:// and do not use the global default
    which most likely is set to a streaming source such as Seedlink
  * Fix queryauth exception in combination with htpasswd
  * Fix incorrect epoch usage when request spans multiple epochs

* scolv

  * Fix arrival used flags evaluation. This resulted in activated arrivals in
    the various plots event if they were not used.
  * Fix rename phases command when only a subset of source phases is selected

* scautoloc

  * Fix bug with respect to latest data model changes and taking Arrival.timeUsed()
    into account.
  * Make playbacks work with station epochs that ended in the past. This is important
    for playbacks of historical events.

* scimport

  * Add support to enable/disable message filtering in configuration file

    ```
    useFilter = false
    ```

    Settings ```useFilter``` to false is equal to passing ```--no-filter``` via
    command-line.

## Release 2017.334 patch4

* dlsv2inv

  * Add continuation support for blockette 54

* scqcv

  * Fix crash that happened after some time receiving
    QC updates

* scolv

  * If ```olv.computeMagnitudesSilently``` is enabled and magnitudes are computed with
    all amplitudes from cache/database then the popup window appeared anyhow. This has
    been fixed.

* scmag

  * Fix crash if an amplitude value is not set

## Release 2017.334 patch3

* scvsmag

  * Catch exception when origin.quality.azimuthalGap is not set

* arclink

  * Avoid leaking file descriptors when connections close unexpectedly

* scm

  * Fix keyboard handling of ncurses plugin. Prior to that fix, keyboard input
    was ignored in combination with some Linux distributions.

* fdsnws

  * Allow combination of simple and window time in station requests

* trunk

  * Fix SDK implementation of Seiscomp::DataModel::DiffMerge
  * Use processed gainUnit (converted to uppercase) rather than Sensor.unit to
    prepare data in AmplitudeProcessor. This affect amplitude computation
    for inventories where the gainUnit is given in lowercase characters, e.g. "m/s"
    rather than "M/S".
  * Order Seedlink requests from wildcarded to concrete

* key2inv

  * Fix stream creation API call with respect to latest API version

* scdispatch

  * Add operation "merge-no-remove" which filters out remove operations

## Release 2017.334 patch2

```SC_API_VERSION 11.1.0```

* doc

  * Add amplitude units expected by each magnitude implementation to glossary
  * Add to data model API documentation for Origin::latitude and Origin::longitude
    that uncertainties are given in kilometers

* trunk

  * Fix memory leak and type mapping of QuakeML exporter used with sccnv
  * Ship xsd and xsl files in ```share/xml```

* scwfparam

  * Add Rupture::strike attribute to strong motion data model extension

* GUI

  * Add StandardLegend class to create map legends in an easy way
  * Add support for map legend descriptions in map layer configuration

## Release 2017.334 patch1

* trunk

  * Fix crash in FDSNWS recordstream if server cannot be reached
  * Updated sc3ml_0.10.xsd
  * Use correct name in connection info for Python scripts
  * Fix SQL character escape bug for strings starting with a single quote

* scamp

  * Update internal cache if picks and/or amplitudes were removed via
    messaging

* GUI

  * Event list shows only the full summary tooltip if the mouse hovers
    the ID column (last column)

* scmapcut

  * Add new module documentation
  * Allow float values for the region parameter

## Release 2017.334

```SC_API_VERSION 11.0.0```

The database schema has changed since the previous version. To upgrade your
database from version 0.9 to 0.10, please run ```seiscomp update-config scmaster```.
If a database plugin is configured, then it will check the current database
schema version and suggest migration scripts to be run. The output should
look as follows:

```
* starting kernel modules
spread is already running
starting scmaster
* configure scmaster
  * check database write access ... OK
  * database schema version is 0.9
  * last migration version is 0.10
  * migration to the current version is required. apply the following
    scripts in exactly the given order:
    * /home/sysop/seiscomp3/share/db/migrations/mysql/0_9_to_0_10.sql
error: updating configuration for scmaster failed
```

To apply the given script, log into your database server and execute the script.
In mysql it can be done with

```
mysql> source /home/sysop/seiscomp3/share/db/migrations/mysql/0_9_to_0_10.sql;
```

and in psql with

```
seiscomp3=> \i /home/sysop/seiscomp3/share/db/migrations/postgresql/0_9_to_0_10.sql;
```

**Rationale**

Most of the inventory objects are valid for certain epochs defined with start
and end time. The database schema did not support microsecond storage of those
times although the structures in the source code do. This schema revision closes
the gap. Furthermore a ResponsePAZ filter could be part of the decimation filter
chain. Therefore the decimation attributes decimationFactor, delay and correction
have been added. Furthermore the ResponseIIR type has been added to correctly
store SEED response coefficients (blockette 54) without the need to convert IIR
filters to poles and zeros.

Furthermore a description of a pdf (probability density function) has been added
to the RealQuantity and TimeQuantity type.

**Important API changes**

***MagnitudeProcessor***

The MagnitudeProcessor interface has changed to support regionalized
magnitude computations. The method ```computeMagnitude``` receives additionally
two parameters, the origin and the sensor location object.
All external magnitude plugins need to be adapted. Change from

```c++
Status computeMagnitude(double amplitude, double period,
                        double delta, double depth, double &value);
```

to

```c++
Status computeMagnitude(double amplitude, double period,
                        double delta, double depth,
#ifdef SC_API_VERSION >= SC_API_VERSION_CHECK(11,0,0)
                        const DataModel::Origin *hypocenter,
                        const DataModel::SensorLocation *receiver,
#endif
                        double &value);
```

Furthermore a new enumeration has been added to return the status of
the magnitude processing: ```EpicenterOutOfRegions```.

***RecordStream***

The RecordStream interface has changed considerably. All ```std::string```
parameters that were passed by value have changed to be passed by const
reference. Due to the rather complicated structure of the RecordStream
interface and its usage in RecordInput, the following methods were removed:

* std::istream& stream()
* Record *createRecord(Array::DataType, Record::Hint)
* void recordStored(Record*)
* bool filterRecord(Record*)

The new interface does deal directly with records and therefore only provides
the single method ```Record *next()```. Iteration stops when a NULL record will
be returned. The advantage is, that an implementation which would route requests
to several backends in parallel such as the balanced recordstream do not need
to deserialize and serialize a record additionally to the application
deserialization. This improves performance and makes it easier to develop more
complex implementations such as a router (which is available as extension from
gempa).

The Python API with respect to RecordInput did not change. You can still use
your old code. Anyone with custom recordstream implementations will have to
port their code.

***Python***

The API changed with respect to exceptions. Rather than throwing wrapped
SC3 exceptions either the Python *ValueError* exception which corresponds to
the C++ *Seiscomp::Core::ValueException* and replaces the old Python
*seiscomp3.Core.ValueException* or the Python *RuntimeError*, which corresponds
to all other C++ exceptions, is raised. See the following example:

```python
# Old code
try:
    print station.latitude()
except seiscomp3.Core.ValueException e:
    print >> sys.stderr, "Station latitude is not set"

# New code
try:
    print station.latitude()
except ValueError e:
    print >> sys.stderr, "Station latitude is not set"
```

Furthermore the *seiscomp3.Config.Exception* was replaced with the Python
*Exception*. See the code below:

```python
# Old code
try:
    param = self.configGetString("param")
except seiscomp3.Config.Exception e:
    print >> sys.stderr, "param is not set"

# New code
try:
    param = self.configGetString("param")
except Exception e:
    print >> sys.stderr, "param is not set"
```

* trunk

  * The API version (```SC_API_VERSION```) is now 11.0.0
  * Set seiscomp3 database bytea encoding to 'escape' for PostgreSQL database
    servers with version >= 9 in postgres.sql script.
  * Add InventorySyncMessage which is used to enclose an inventory synchronization
    process. An application can listen to that message and trigger processing of
    the updated inventory. The InventorySyncMessage is currently sent by
    scinv (seiscomp update inventory) to STATUS_GROUP.
  * Changed default publicID pattern from "@classname@#@time/%Y%m%d%H%M%S.%f@.@id@"
    to "@classname@/@time/%Y%m%d%H%M%S.%f@.@id@". The hash was removed due to
    possible conflicts with QuakeML publicID constraints.
  * FDSNWS recordstream sets default URL path to /fdsnws/dataselect/1/query which
    makes it more easy to use e.g. fdsnws://geofon.gfz-potsdam.de
  * Removed obsolete recordstream **isoarchive**.
  * Removed obsolete Greens function access via Arclink.

* GUI

  * The event list shows status REVIEWED as V, FINAL as F and REPORTED as R
  * Added option to allow map layer visibilities and order
  * Allow to add custom map layers via plugins to the map
  * Refactored Map API (Canvas, Layer, Legend)
  * All GUI applications support an author and/or user blacklist to prevent sending
    messages to scmaster. This is not a proper secure access control implementation
    but helps to setup read-only applications to avoid accidental commits.
    ```
    blacklist.users = sysop1, sysop2
    blacklist.authors = sysop1@host, sysop2@host
    ```
  * Map layer drawing properties may be additionally defined in a "map.cfg" file
    located in the data set folder and subfolder, e.g. ```~/.seiscomp3/fep/map.cfg```,
    ```~/.seiscomp3/bna/map.cfg```, ```~/.seiscomp3/bna/category/map.cfg```.
  * Added support for event summary to listen to alert comments and adapt size and color
    of time ago label accordingly.

* scmm

  * Added the module documentation.

* scmv

  * Added option ```expiredEventsInterval``` which controls the interval to check for expired events.
    The default value is 0 and does disable the interval check.
  * Added option to show the event table initially and to configure visible columns
    ```
    eventTable.visible = true
    eventTable.columns = Event, Depth
    ```
  * Add legend for event symbols

* scolv

  * ```locator.minimumDepth``` is now deprecated in favour of ```olv.locator.minimumDepth```
  * ```olv.locator``` is now deprecated in favour of ```olv.locator.interface```
  * Add option to configure the default checkstate of the event association button and
    fix origin button of the popup for committing with additional options: ```olv.commit.forceEventAssociation```
    and ```olv.commit.fixOrigin```. Either default value is true.
  * Add system tray icon which shows a notification if a new event
    has been detected. This can be disabled with
    ```
    olv.systemTray = false
    ```
  * Replace single arrival usage flag with three separate usage flags: time used,
    backazimuth used and horizontal slowness used which can also be toggled
    separately

* scrttv

  * Normalize visible amplitudes (S) now toggles between normalizing amplitudes
    of the currently visible time window (true) or the entire trace (false). The
    old behaviour caused traces to degenerate into a straight line if the data buffer
    runs out the time window which was used to normalize amplitudes.

* scqc

  * Added configuration option ```use3Components``` that allows to use all
    3 components of a configured station. This only applies if ```useConfiguredStreams```
    is active (default).

* scinv

  * Print file source of conflicting definitions

* seiscomp

  * An init script can not forward its configuration to another module. This is
    especially useful if e.g. ```seiscomp update-config scautopick``` is ran
    which did not do anything. Now it forwards its configuration to module
    *trunk* and will update the bindings database. The old behaviour has always
    confused users.

* dlsv2inv

  * Improve conversion to SC3. Many thanks to Arnaud Lemarchand from IPGP France
    for his exhaustive tests and valuable advises.
  * Add station and channel comment support

* fdsnxml2inv

  * Improve conversion to SC3. Many thanks to Arnaud Lemarchand from IPGP France
    for his exhaustive tests and valuable advises.
  * Declare NumeratorCoefficient.i as optional according to the official schema. Before
    that change, a lot of responses failed to convert.
  * Do not populate NumeratorCoefficient.i when converting to FDSNXML to avoid
    bloating the XML.
  * Add station and channel comment support

* scsohlog

  * Add description and documentation

* scbulletin

  * Add option -e for enhanced output at higher precision.

* seedlink

  * Removed option -C from nmxptool plugin template. This should go into the
    additional options parameter.

* arclink

  * Removed ```isodir``` option
  * Removed obsolete GREENSFUNC request type

* scwfparam

  * Add configuration option for the path to the processing log file
  * Add commandline option ```--force-shakemap``` to run the ShakeMap
    script even if no station has contributed any data

* VS

  * StrongMotion data model has changed. It introduces pdf descriptions for all
    RealQuantities and TimeQuantities and adds centroidReference to Rupture
    table. Either recreate the database from scratch with the new schema or
    diff the new sql with the old and apply the changes manually.

* Hypo71

  * Fix a bug when all arrivals uncertainties where not set
  * Fix a bug in Hypo71PC for earthquakes near longitude 0 and longitude 180
    Thanks to M. Sylvander from IRAP/OMP France for finding and fixing this.

## Release 2017.124

* seiscomp

  * Use symbolic links to module defaults and configurations instead of
    copying when creating module aliases

* doc

  * Added plugin section to module documentation with all documented
    plugins

* spread

  * Upgraded to version 4.4

* LocSAT

  * Started to clean up code and to remove f2c dependencies. Goal of this
    was to make compute_tt work in subsequent calls. Prior to the changes
    travel time computation with LocSAT gave unpredictable results which
    is now fixed.

* scconfig

  * Fixed issue with deleted structures when saving a configuration file.
    Prior to that fix the structure was not deleted.
  * Added documentation section which allows to browse changelogs and
    documentations of installed modules

* scautopick

  * Added the option ```killPendingSPickers``` to configure whether to
    terminate pending secondary processors if a new detection has been
    found or not. The downside of disabling that is that two picks will
    be possibly sent: a P and an S pick.

* GUI

  * Fixed bug in map tilestore that caused custom tilestore implementations
    to crash under certain circumstances
  * Add option to show times in localtime
    ```
    scheme.dateTime.useLocalTime = true
    ```
    This will show (hopefully) all times in the GUI with respect to the
    systems timezone instead of UTC.

* scesv

  * Fixed crash if no event was loaded and either "Show full tensor" or
    "Show waveform propagation" was toggled

* scrttv

  * If loading data from a file then all data is loaded and --buffer-size
    is being ignored. Furthermore XML event parameter files can be loaded
    (File->Open) and picks will be shown on top of loaded traces.

* scolv

  * Show symbols for unassociated station by default up to 360 degrees.
    This distance Can be changed with:
    ```
    # Show unassociated stations up to 20 degrees
    olv.map.stations.unassociatedMaxDist = 20
    ```
  * Apply show spectrogram values initially
  * Use separate trace color if spectrogram is shown: ```scheme.colors.records.spectrogram```
  * Read and apply scolv/global bindings to the repicker

* scmv

  * Add "Show Details" button to event details widget

* scevent

  * Added parameter to also compare picks that are associated with
    weight 0
    ```
    eventAssociation.allowLooseAssociatedArrivals = true
    ```
  * Added region check plugin that allows to configure a list of arbitrary
    regions and to set the event type to "outside of network interest" if
    its location is outside any region configured.

* scevtls

  * Added --modified-after option

* dlsv2inv, fdsnxml2inv

  * Make sample rate conversion from float to fraction more stable

* fdsnxml2inv

  * Fix epoch creation of sensor locations for some channel epoch
    combinations which caused the creation of two sensor locations
    with split epochs

* fdsnws

  * Make arclink-access bindings optional through configuration parameter
    useArclinkAccess
  * Add option ```recordBulkSize``` which defaults to 100kb and improves
    the dataselect performance significantly

* seedlink

  * Added ps2400_eth plugin configuration

* sh2proc

  * New Python tool to convert SeismicHandler event files to SC3XML


## Release 2016.333

The database schema has changed since the previous version. To upgrade your
database from version 0.8 to 0.9 to following SQL script can be used:


**MYSQL**

```sql
ALTER TABLE ConfigStation ADD creationInfo_agencyID VARCHAR(64);
ALTER TABLE ConfigStation ADD creationInfo_agencyURI VARCHAR(255);
ALTER TABLE ConfigStation ADD creationInfo_author VARCHAR(128);
ALTER TABLE ConfigStation ADD creationInfo_authorURI VARCHAR(255);
ALTER TABLE ConfigStation ADD creationInfo_creationTime DATETIME;
ALTER TABLE ConfigStation ADD creationInfo_creationTime_ms INTEGER;
ALTER TABLE ConfigStation ADD creationInfo_modificationTime DATETIME;
ALTER TABLE ConfigStation ADD creationInfo_modificationTime_ms INTEGER;
ALTER TABLE ConfigStation ADD creationInfo_version VARCHAR(64);
ALTER TABLE ConfigStation ADD creationInfo_used TINYINT(1) NOT NULL DEFAULT '0';

UPDATE Meta SET value='0.9' WHERE name='Schema-Version';
```

**PostgreSQL**

```sql
ALTER TABLE ConfigStation ADD m_creationInfo_agencyID VARCHAR(64);
ALTER TABLE ConfigStation ADD m_creationInfo_agencyURI VARCHAR(255);
ALTER TABLE ConfigStation ADD m_creationInfo_author VARCHAR(128);
ALTER TABLE ConfigStation ADD m_creationInfo_authorURI VARCHAR(255);
ALTER TABLE ConfigStation ADD m_creationInfo_creationTime TIMESTAMP;
ALTER TABLE ConfigStation ADD m_creationInfo_creationTime_ms INTEGER;
ALTER TABLE ConfigStation ADD m_creationInfo_modificationTime TIMESTAMP;
ALTER TABLE ConfigStation ADD m_creationInfo_modificationTime_ms INTEGER;
ALTER TABLE ConfigStation ADD m_creationInfo_version VARCHAR(64);
ALTER TABLE ConfigStation ADD m_creationInfo_used BOOLEAN NOT NULL DEFAULT '0';

UPDATE Meta SET value='0.9' WHERE name='Schema-Version';
```

**Rationale**

The ConfigStation object will be updated by three instances in SeisComP3:
```seiscomp update-config```, ```scrttv``` and ```scqcv```. To track which
module has disabled or enabled a particular station the CreationInfo
structure has been added to ConfigStation.

----

* slmon

  * Ported package from SeisComP 2.5

* trunk

  * Set default author to appname@hostname instead of user@hostname
  * Upgraded rapidjson library to 1.1.0
  * Apply processing stage0 gain correction if streams gain frequency
    does not match the sensors gain frequency

* scconfig

  * Fix parameter tooltip if description text contains special HTML characters
    such as < (less than) or > (greater than)

* fdsnws

  * Add URL builder page to each service

* ql2sc

  * Use socket timeout of 60s if keepAlive is activated

* scinv

  * Split Spread messages into smaller chunks if the payload size exceeds
    allowed limit

* scautopick

  * Fixed removal of expired secondary pickers that caused a segmentation
    fault

* fdsnxml2inv

  * Correct types of some attributes of FDSNXML::PolynomialResponse
  * Convert polynomial responses correctly
  * Made FDSNXML::Comment::id optional according to the standard
  * Output line numbers in case of errors

* inv2dlsv

  * Fix handling of blockette62

* dlsv2inv

  * Fix bug that caused wrong sensor calibration gain

* seedlink

  * Fix Q330 setup if multiple instances are configured per station
  * Fix scream_ring setup in combination with a configured map file

* scwfparam

  * Apply lost patch again to use the same path name as earthquake.id for input files
  * Add configuration of output XML encoding due to Shakemap issues with non ASCII
    characters
  * Add station bindings to configure saturation threshold

* scrttv

  * Add option `autoApplyFilter` to apply the configured filter initially

* scalert

  * Fix Python import issue

* fdsnws

  * Add support for logging Arclink-style request statistics
  * Add support for EIDA authentication scheme
  * Set CORS headers to allow cross-site Javascript use
  * Fix geo filter for POST queries
  * Allow access to non-restricted streams even if network or station is marked as restricted
  * Include fdsnws_fetch client

* scevent

  * Fix segfault

* GUIs

  * Connection setup dialog removes the fetch database parameters button
    and replaces it with "Switch to reported" action that connects to the
    database as reported by scmaster while handshaking

* NonLinLoc

  * Fix compilation and linking for gcc >= 5

## Release 2016.161

The database schema has changed since the previous version. To upgrade your
database from version 0.7 to 0.8 the following SQL script can be used:

**MYSQL**

```sql
CREATE TABLE ResponseFAP (
  _oid INTEGER(11) NOT NULL,
  _parent_oid INTEGER(11) NOT NULL,
  _last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  name VARCHAR(255),
  gain DOUBLE UNSIGNED,
  gainFrequency DOUBLE UNSIGNED,
  numberOfTuples SMALLINT UNSIGNED,
  tuples_content BLOB,
  tuples_used TINYINT(1) NOT NULL DEFAULT '0',
  remark_content BLOB,
  remark_used TINYINT(1) NOT NULL DEFAULT '0',
  PRIMARY KEY(_oid),
  INDEX(_parent_oid),
  FOREIGN KEY(_oid)
    REFERENCES Object(_oid)
    ON DELETE CASCADE,
  UNIQUE(_parent_oid,name)
) ENGINE=INNODB;

UPDATE Meta SET value='0.8' WHERE name='Schema-Version';
```

**PostgreSQL**

```sql
CREATE TABLE ResponseFAP (
  _oid BIGINT NOT NULL,
  _parent_oid BIGINT NOT NULL,
  _last_modified TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  m_name VARCHAR(255),
  m_gain DOUBLE PRECISION,
  m_gainFrequency DOUBLE PRECISION,
  m_numberOfTuples SMALLINT,
  m_tuples_content BYTEA,
  m_tuples_used BOOLEAN NOT NULL DEFAULT '0',
  m_remark_content BYTEA,
  m_remark_used BOOLEAN NOT NULL DEFAULT '0',
  PRIMARY KEY(_oid),
  FOREIGN KEY(_oid)
    REFERENCES Object(_oid)
    ON DELETE CASCADE,
  UNIQUE(_parent_oid,m_name)
);

UPDATE Meta SET value='0.8' WHERE name='Schema-Version';
```

----

* documentation

  * Enhance documentation of scqc and others

* scart

  * Fix bug that caused wrong or no data to be returned if a request spans
    multiple years

* NonLinLoc

  * Fixed bug that caused wrong confidence ellipsoid axis length measures.
    Values are expected in meters but the wrapper exported then as kilometers.

* trunk

  * Add support for HMB (http messaging bus) messaging protocol
  * Add recordstream implementation for HMB (http messaging bus) protocol
  * Remove parent_oid foreign key constraint in database tables
  * Add support for FAP responses (response list) to datamodel
  * Add support for FAP responses to processing
  * Make ML logA0 configurable in bindings and add documentation
  * Hide database passwords in logfiles

* dlsv2inv

  * Add support for response list (blockette 55) which are converted to
    ResponseFAP

* fdsnxml2inv

  * Add support for reponse list
  * Prefer SampleRateRatio over SampleRate when converting to SC3

* scinv

  * Synchronize ResponseFAP objects

* scimport

  * Allow to configure the sink protocol

* scimex

  * Allow to configure the sink protocol

* msrtsimul

  * Wait until the time of the last data point in record has been reached
  * Add option to reopen the pipe if closed

* GUI

  * Display also degrees when measuring distance in a map

* scolv

  * Fix timing quality rendering in picker
  * Fix magnitude table distance entries if distance is displayed in km
  * Fix spectrum display on 32bit systems
  * Fall back to detecStream for vertical component in picker if 3
    components are not available in inventory
  * Add spectra plotting in picker (hit space if picking is disabled) for
    the current trace
  * Fix sorting of take off column in Arrival table
  * Allow columns to be reorderd for the current session

* scmv

  * Add plotting of event focal mechanism if available

* fseed

  * Add conversion from ResponseFAP to blockette 55

* system

  * Upgrade libmseed to 2.17

* seiscomp

  * Fixed resolving the path to the seiscomp script when it is a symlink

## Release 2016.062

* FDSNWS

 * Fixed missing lines in text output of station and event service
 * Added sorted text output in station service
 * Add inventory filter option for station and dataselect service
 * Fixed missing responses if public ID does start with *Response*

* seiscomp

 * Fixed parsing of the release string with lsb_release and CentOS
 * Fixed resolving directory links during SEISCOMP_ROOT determination
 * Added shell completion support for bash and fish (optional)
 * Added support for remote database servers when setting up SeisComP
 * Added Debian8 support to install-deps
 * Added output of exceptions when stopping a module which can sometimes
   fail when garbarge is written to pid files (to be investigated)

* trunk

 * Removed KIWI Greens function archive plugin
 * Removed MultiComponentArray from source to be replaced by a more advanced version
 * Fixed (hopefully) a synchronization issue when an applications shuts down that caused a mutex lock assertation
 * Fixed exception with unset attributes in format autoloc1 and autoloc3 in scbulletin
 * Fixed crash in scqc when an invalid regexp was used
 * Fixed azimuth output in fdsnxml2inv to stay in range [0,360)
 * Fixed database configuration reader issue with SQLite3 databases
 * Added Instaseis Greens function archive
 * Added bindings2cfg app that reads an key dir and generates config XML output into a file or to stdout
 * Added E?? to default channel regexp in scart
 * Added offline XML processing to scevent
 * Added printing of analogue response for digitizer stages in scinv ls
 * Added libbson and rapidjson as 3rd-party library to support object (de-)serialization to JSON/BSON and back
 * Added offline XML processing to scautoloc and a command line option to use manual picks for association (--use-manual-picks)
 * Added active record filtering (time and code) to file record stream
 * Added support for amplitude calculation on displacement
 * Added possibility to return polarity information in PickProcessors
 * Added database index to Amplitude.timeWindow.reference
 * Added options to configure deconvolution frequency tapers
 * Added filtering of records to the file data source according to requested
   time windows and channels

* scconfig

 * Added guard to start scconfig only once per installation directory
 * Show popup window during save if an application configuration file
   has been changed outside of scconfig and allow to decide which version
   to take

* Seedlink

 * Fixed compilation of Q330 plugin with Debian8 and probably others

* Hypo71

 * Fixed crash when built as release
 * Fixed bug for stations with elevation <-999
 * Fixed documentation
 * Fixed plugin use of snrMin when changed in the waveform review window

* scautopick

 * Added option to bindings (sensitivityCorrection) to correct for sensitivity
   before applying the filter
 * Added --playback option to not issue a time window request to the underlying
   data source which is important in connection with playbacks and offline runs
   with file sources, see documentation
 * Fixed adding S pick comments also in --ep mode

* scevent

 * Fixed association bug with automatic origins which prevented origins from
   being set preferred if the phase count is larget but the creation time is
   lower

* fdsnxml2inv

 * Only allow positive azimuth values

* scmv

 * Added option to display station names on startup and to configure if channel names are shown or not

   ```
   annotations = true
   annotionsWithChannels = false
   ```

* scrttv

 * Fixed crash when a station configuration was received that was not configured

* scolv

 * Fixed bug that disabled the locator profile selection box
 * Added spectrogram rendering for active trace
 * Added button to sort traces by azimuth

* GUI

 * Added options to configure trace line width, antialiasing and optimization

   ```
   scheme.records.lineWidth = 2
   scheme.records.antiAliasing = true
   scheme.records.optimize = false
   ```

* doc

 * Added Python example on how filter records

## Release 2015.149

* doc

 * Added mB time window computation
 * Corrected M(JMA) depth range
 * Made generator to follow symlinks

* trunk

 * Application class issues a warning if a loaded plugin is built against a different API version
 * Fixed compilation with boost 1.54
 * Bumped API version to 1.15
 * Improved finding 3 oriented components of a stream which should work also for UVW streams
 * !RecordStream "combined" introduces new syntax to enable combined of combined streams, e.g. combined://combined/(arclink/host1:18001;arclink/host2:18001??rtMax=86400);slink/host2:18000??rtMax=3600
 * Fixed fft computation bug
 * Set default taper length for restitution to 5s
 * Added Python wrappers for geo lib
 * Added scgitinit.sh to initialize a Git repository for SeisComP3 configuration files

* GUI

 * Corrected default color description for gaps
 * Added description of overlap and alignment bar color
 * Shutdown GUI applications gracefully on ctrl+c
 * Do not try to fetch database parameters from master if database is not enabled
 * Increased default size of plot symbols
 * Increased defaultstation symbol size

* scrttv

 * Added option to define multiple filters to cycle through

* scolv

 * Added more functional row selection and activation in magnitude view
 * Added warning to trace if not metadata are available
 * Added option to cycle through filters with keyboard
 * Do not show acquisition error box if acquisition has been cancelled by user

* tabinvmodifier

 * Fixed Python return codes for event handlers

* screloc

 * Added option --use-weight to forward arrival weights to locator
 * Added option --evaluation-mode to override default evaluation mode of relocated origin
 * Added option --ep for XML processing

* scwfparam

 * Added configuration parameter "SC3EventID" to set the event publicID as earthquake.id
 * Added configuration parameter "regionName" to set the events region name in locstring
 * Fixed processing bug

* fdsnws

 * Hide author from creationInfo in format=text, too
 * Fixed order by time if magnitude filter is enabled

* scamp

 * Added option --ep for XML processing

* scautopick

 * Added option --ep for XML processing

* scmag

 * Added option --ep for XML processing

## Release 2015.040

**WARNING:** Due to a fixed bug in the amplitude calculation if deconvolution is enabled the processing of amplitudes introduces now a delay of 60s unless
configured otherwise. Background: the default cosine taper time window length has been left unchanged for now and is still 60s at either side. To not affect
the signal time window the taper must start at signal end and thus 60s more data are required. That is usually not necessary and too much. Upcoming releases
will set the default to 5s but for now it is 60s. The configuration of "amplitudes.[type].resp.taper" in global bindings can fix it. As an override the following
changes can be applied to global.cfg:

``` c++
module.trunk.global.amplitudes.ML.resp.taper = 5.0
module.trunk.global.amplitudes.MLv.resp.taper = 5.0
module.trunk.global.amplitudes.MLh.resp.taper = 5.0
module.trunk.global.amplitudes.mb.resp.taper = 5.0
module.trunk.global.amplitudes.mB.resp.taper = 5.0
module.trunk.global.amplitudes.Mwp.resp.taper = 5.0
```

* seiscomp

 * Refuse to run as root unless "--asroot" is passed

   ```
   $ sudo seiscomp help
   [sudo] password for sysop:
   Running seiscomp as root is dangerous. Use --asroot if you know what you are doing
   $ sudo seiscomp --asroot help
   Available commands:
     install-deps
     setup
     ...
   ```

* VS

 * Fixed installation of !VsMag Python libraries if wrappers are disabled

* scvsmaglog

 * ActiveMQ messages can now be sent using either the QuakeML, SeisComP3ML, or ShakeAlertML format. This allows to use both the !ShakeAlert !UserDisplay (developed at Caltech) and the EEWD to receive real-time messages from VS in SeisComP3
 * Event ID added to the email subject
 * Additional configuration options enable operating several instances of scvsmaglog

* scenvelope

 * The default message group for envelope messages is now AMPLITUDE. Adding the message group VS is therefore no longer needed.

* scvsmag

 * Fixing a bug in reading Vs30 files for amplitude correction (thanks Palmi Erlendsson for spotting it!)
 * Fixing a bug in magnitude computation: the proxy for the frequency content of the earthquake signal is now taken into account in MVS.
 * Fixing a bug in the computation of the pick rate
 * The computation of the likelihood quantifying the quality of the VS alert has been slightly modified (see documentation of scvsmag for details)
 * The azimuthal gap is used as an additional quality criterion
 * VS envelope messages are now received by default from the message group AMPLITUDE

* ud_interface

 * Use xslt files to format AMQ messages

* Trunk

 * Added default port 80 to FDSN Webservice RecordStream
 * Made Helmberger format with 10 component files work with Green's functions
 * Removed gain correction (if gain was defined outside the target frequency of interest) and rely on operator to define reasonable Gains in inventory files
 * Added BOOST link and include directories if installed in a non standard location to projects missing that entries
 * Fixed compilation of GUI tree if boost is installed in non-standard location
 * Added "no-batch" option to Seedlink recordstream to connect to Seedlink servers that do not support BATCH requests
 * Fixed Seedlink connection with respect to reconnect that caused restart of the entire request
 * Fixed mapping to event type 'other' in QuakeML converter
 * Removed StationXML support which is superseded by FDSNXML
 * Create more database indexes as suggested by Fabian Engels
 * LocSAT: take weight into account when locate with a picklist
 * Allow to configure usage of responses per amplitude type
 * Clip MLv and Ms(BB) time window to given signalEnd
 * Added amplitude check for valid range if computing magnitudes
 * Return correct status if distance is out of range for ML
 * AmplitudeProcessor moved deconvolution cosine taper start to signal end to not damp the amplitudes

* GUI

 * Added scheme.precision.uncertainties description

* scolv

 * Show acquisition errors in a message box
 * Added smart layout mode to focal mechanism map
 * Group by agency for focal mechanisms
 * Added option picker.ignoreUnconfiguredStations to add only stations that are configured with "detecStream" when adding stations in a certain range
 * Added option to define picker alignment position: picker.alignmentPosition
 * Set computeMissingTakeOffAngles to true by default
 * Fixed crash when computing magnitudes
 * Populate Pick.filterID with currently active filter
 * Crash if raw data is shown in amplitude view and the time window is changed

* hypo71

 * Fixed bug when a pick has been deactivated in scolv
 * Fixed "fixed depth" issue
 * Removed OPTIONAL flag for Fortran compiler check if HYPO build is enabled

* fdsnws

 * Added hideAuthor config parameter
 * Added evaluationMode config parameter
 * Added event type white and blacklist
 * Make stream codes case sensitive
 * Fixed service outage on HTTP error 204
 * Fixed combined analogue and digital filter chain, https://github.com/SeisComP3/seiscomp3/issues/10
 * Avoid unnecessary array lookup

* fdsnxml2inv

 * Fixed order of response stages if pre-amp stages are used

* scevtlog

 * Fixed issue with duplicate magnitudes in XML files

* scbulletin

 * Added event type to format "-3"
 * Added Ms(BB) amplitudes and periods to station magnitude list (contributed by Aleksey Emanov)
 * Fixed output for XML if only an origin(ID) is given

* dlsv2inv

 * Added support for TEMPERATURE and PRESSURE channels when converting PAZ responses

* screloc

 * Added commandline option "dump" and "profile" to dump results in XML to stdout and to set a locator profile

* scautoloc

 * Added parameter autoloc.stationLocations to description

* scdbstrip

 * Disable daemon mode

* scimport

 * Reconnect to sink until connection can be established during init

* scautopick

 * Populate Pick.filterID with currently active filter

* scevent

 * Config parameters are float rather than int (Marc Grunberg)

* scwfparam

 * Use significant duration as processing time window scaled by optional durationScale parameter, replaced sensitivity correction filter by post-deconvolution filter
 * Replaced "2nd filter" with "filter" and use t05+duration*x
 * Implemented requested XML changes
 * Fixed invalid description file

* scconfig

 * Log XML file which produced an error

* Seedlink

 * lib330 update
 * Fixed Antelope ORB plugin setup
 * Added global access parameter configuration

* NLL

 * Applied patches from Anthony Lomax and Romane Racine (Swiss Seismological Service)

* Known bugs

 * scwfparam produces wrong amplitudes if deconvolution or acausal filtering is enabled. There are [https://github.com/SeisComP3/seiscomp3/commit/1b9ea9a0f9417d72317343ae87fbc9e2ea42df43 two] [https://github.com/SeisComP3/seiscomp3/commit/e5bb17fe3492b62004526524b6bde23e5f3e1d99 patches] available.
