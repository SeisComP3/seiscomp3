# Jakarta

## Release YYYY.DDD

The database schema has changed since previous version. To upgrade your
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

* trunk

  * Add support for HMB (http messaging bus) messaging protocol
  * Add recordstream implementation for HMB (http messaging bus) protocol
  * Remove parent_oid foreign key constraint in database tables
  * Add support for FAP responses (response list) to datamodel
  * Add support for FAP responses to processing
  * Make ML logA0 configurable in bindings and add documentation

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

* scolv

  * Fix timing quality rendering in picker
  * Fix magnitude table distance entries if distance is displayed in km
  * Fix spectrum display on 32bit systems
  * Fall back to detecStream for vertical component in picker if 3 components are not available in inventory

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
