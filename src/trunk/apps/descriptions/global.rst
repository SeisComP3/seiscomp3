*************
Configuration
*************

The SeisComP3 configuration uses a unified schema to configure modules.
Modules which use  the SeisComP3 libraries can read this configuration
directly and share global configuration options like messaging connections,
database configurations, logging and much more.
There are still some modules that do not use
the libraries and are called **standalone** modules as :ref:`seedLink`, :ref:`arclink`
or :ref:`slarchive`. They need wrappers to generate their native configuration when
:command:`seiscomp update-config` is run.

Though it is easy to create the configuration by directly editing the configuration
files, it is even more convenient to use a configurator.
SeisComP3 ships with a graphical
configurator and management tool (:ref:`scconfig`) which makes it easy to maintain
module configurations and station bindings even for large networks. It has built-in
functionality to check the state of all registered modules and to start and stop them.

The configuration is divided into three parts: stations, bindings and modules.

Configuration files
===================

The :term:`trunk` configuration files are simple text files where each line
is a name-value pair.

.. warning::

   In contrast to previous versions of SeisComP3 the parameter names are now
   case-sensitive. To check configurations from previous versions regarding
   case-sensitivity, :program:`scchkcfg` can be used.

A simple example to assign a parameter "skyColor" the value "blue":

.. code-block:: sh

   skyColor = blue

Everything following an un-escaped '#' (hash) is a comment and ignored.
Blank lines and white spaces are ignored by the parser as well unless
quoted or escaped.

.. code-block:: sh

   skyColor = yellow  # This is a comment

   # The preceding empty line is ignored and previous setting "yellow"
   # is replaced by "blue":
   skyColor = blue

Later assignments overwrite earlier ones so the order of lines in the
configuration file is important. The file is parsed top-down.

Values can be either scalar values or lists. List items are separated by commas.

.. code-block:: sh

   # This is a list definition
   rainbowColors = red, orange, yellow, green, blue, indigo, violet

If a value needs to include a comma, white space or any other interpretable
character it can either be escaped with backslash (\\) or quoted using double
quotes ("). White space is removed in unquoted and un-escaped values.

.. code-block:: sh

   # This is a comment

   # The following list definitions have 2 items: 1,2 and 3,4
   # quoted values
   tuples = "1,2", "3,4"
   # escaped values
   tuples = 1\,2, 3\,4

Values can extend over multiple lines if a backslash is appended to each line

.. code-block:: sh

   # Multi-line string
   text = "Hello world. "\
          "This text spawns 3 lines in the configuration file "\
          "but only one line in the value."

   # Multi-line list definition
   rainbowColors = red,\
                   orange,\
                   yellow,\
                   green, blue,\
                   indigo, violet

Environment or preceding configuration variables can be used with ``${var}``.

.. code-block:: sh

   homeDir = ${HOME}
   myPath = ${homeDir}/test

.. note::

   Values are not type-checked. Type checking is part of the application logic
   and will be handled there. The configuration file parser will not raise an
   error if a string is assigned to a parameter that is expected to be an integer.


.. _global-stations:

Stations
========

Station meta-data is a fundamental requirement for a seismic processing system and
for SeisComP3. Older version used key files to configure available networks and stations.
Because the support of response meta-data was very limited, tools were build to add
this functionality. In this version the concept of key files for station meta-data has
been completely removed from the system. SeisComP3 only handles station meta-data in its
own XML format called **inventory ML**.
The task of supporting old key files, dataless SEED and other formats has been out-sourced to external applications (see :ref:`config-fig-inventory-sync`).

.. _config-fig-inventory-sync:

.. figure:: media/config/inventory-sync.*
   :align: center

   Inventory synchronization is a two-stage process:

   \(1\) convert external formats into inventory ML, then
   \(2\) synchronize inventory pool with the database

External formats are first converted into inventory ML,
and then merged and synchronized with the database using
:program:`seiscomp update-config`.
All station meta-data are stored in :file:`etc/inventory`
and can be organized as needed. Either one file per network, a file containing the complete inventory
or one file for all instruments and one file per station. The update script loads the existing inventory
from the database and merges each file in :file:`etc/inventory`. Finally it removes all unreferenced
objects and sends all updates to the database. 

The SeisComP3 configuration does not deal with station meta-data anymore. It only configures parameters
for modules and module-station associations. The management of the inventory can and should be handled
by external tools.


Bindings
========

A :term:`binding` is always connected to a :term:`module`. The binding configuration directory
for each module is :file:`etc/key/modulename`. It contains either station bindings or :term:`profiles<profile>`.

Bindings are configured and stored in :file:`etc/key`.

.. _config-fig-binding:

.. figure:: media/config/binding.*
   :align: center

   Binding

   A binding holds the configuration how a station is used in a module.

To bind a station (identified by net_sta) to a module with a set of parameters the first step is to
register a module for that station. For that a station key file needs to be created or modified.

.. note::

   To reflect the old framework, a station binding is prefixed with *station_* and a profile with *profile_*.

Let's suppose we have two stations, GE.MORC and GE.UGM and both stations should be configured for
SeedLink. Two station key files need to be created (or modified later): :file:`etc/key/station_GE_MORC` and
:file:`etc/key/station_GE_UGM`.

Both files must contain a line with the module the station is configured for, e.g.:

.. code-block:: sh

   seedlink

which uses the binding at :file:`etc/key/seedlink/station_GE_UGM`. When a profile should
be used, append it to the module with a colon.

.. code-block:: sh

   seedlink:geofon

Then the binding at :file:`etc/key/seedlink/profile_geofon` is read for station GE.UGM.
To list all modules a particular station is configured for is very simple by printing the content
of the station key file:

.. code-block:: sh

   $ cat etc/key/station_GE_MORC
   seedlink:geofon
   global:BH
   scautopick

The other way round is a bit more complicated but at least all information is
there.
To show all
stations configured for SeedLink could be done this way:

.. code-block:: sh

   $ for i in `find etc/key -type f -maxdepth 1 -name "station_*_*"`; do
   > egrep -q '^seedlink(:.*){0,1}$' $i && echo $i;
   > done
   etc/key/station_GE_MORC
   etc/key/station_GE_UGM


Storage
-------

Where are bindings stored? For standalone modules: nobody knows.
It is the task of a standalone module's
initialization script to convert the bindings to the module's native configuration.

For all :term:`trunk` (non-standalone) modules the bindings are written to the
SeisComP3 database following the configuration schema. This is done when
:program:`seiscomp update-config` is called.
Each module reads the configuration database and fetches all station bindings registered
for that module. The database schema used consists of five tables:
ConfigModule, ConfigStation, Setup, ParameterSet and Parameter.

.. _config-fig-configdb-schema:

.. figure:: media/config/configdb-schema.*
   :align: center

   Configuration database schema

Now an example is shown how the tables are actually linked and how the station bindings are
finally stored in the database. To illustrate the contents of the objects, the XML representation
is used.

.. code-block:: xml

   <Config>
     <module publicID="Config/trunk" name="trunk" enabled="true">
       ...
     </module>
   </Config>

A ConfigModule with publicID *Config/trunk* is created with name *trunk*. This
ConfigModule is managed by the global initialization script (:file:`etc/init/trunk.py`)
and will be synchronized with configured bindings of all trunk modules. The
ConfigModule trunk is the one that is actually used by all configurations unless
configured otherwise with:

.. code-block:: sh

   scapp --config-module test

Here :program:`scapp` would read ConfigModule *test*. Because a ConfigModule *test*
is not managed by :program:`seiscomp update-config` it is up to the user to create
it.


For each station that has at least one binding, a ConfigStation object is
attached to the ConfigModule:

.. code-block:: xml

   <Config>
     <module publicID="Config/trunk" name="trunk" enabled="true">
       <station publicID="Config/trunk/GE/UGM"
                networkCode="GE" stationCode="UGM" enabled="true">
         ...
       </station>
     </module>
   </Config>

and finally one Setup per module:

.. code-block:: xml

   <Config>
     <module publicID="Config/trunk" name="trunk" enabled="true">
       <station publicID="Config/trunk/GE/UGM"
                networkCode="GE" stationCode="UGM" enabled="true">
         <setup name="default" enabled="true">
           <parameterSetID>
             ParameterSet/trunk/Station/GE/UGM/default
           </parameterSetID>
         </setup>
         <setup name="scautopick" enabled="true">
           <parameterSetID>
             ParameterSet/trunk/Station/GE/UGM/scautopick
           </parameterSetID>
         </setup>
       </station>
     </module>
   </Config>


Here two setups have been created: *default* (which is a special case for
module *global* to be backwards compatible) and *scautopick* where each
refers to a ParameterSet by its publicID. The next XML fragment shows
the ParameterSet referred by the scautopick setup of station GE.UGM:

.. code-block:: xml

   <Config>
     <parameterSet publicID="ParameterSet/trunk/Station/GE/UGM/scautopick"
                   created="...">
       <baseID>ParameterSet/trunk/Station/GE/UGM/default</baseID>
       <moduleID>Config/trunk</moduleID>
       <parameter publicID="...">
         <name>timeCorr</name>
         <value>-0.8</value>
       </parameter>
       <parameter publicID="...">
         <name>detecFilter</name>
         <value>
           RMHP(10)&gt;&gt;ITAPER(30)&gt;&gt;BW(4,0.7,2)&gt;&gt;STALTA(2,80)
         </value>
       </parameter>
       <parameter publicID="...">
         <name>trigOff</name>
         <value>1.5</value>
       </parameter>
       <parameter publicID="...">
         <name>trigOn</name>
         <value>3</value>
       </parameter>
     </parameterSet>
   </Config>

The mapping to the binding configuration files is 1:1. Each parameter in
the configuration file is exactly one parameter in the database and their
names are matching exactly.

The concept of global bindings which are specialized for each application is
reflected by the *baseID* of the ParameterSet which points to setup *default*
of station GE.UGM:

.. code-block:: xml

   <Config>
     <parameterSet publicID="ParameterSet/trunk/Station/GE/UGM/default"
                   created="...">
       <moduleID>Config/trunk</moduleID>
       <parameter publicID="...">
         <name>detecStream</name>
         <value>BH</value>
       </parameter>
     </parameterSet>
   </Config>

This ends up with a final configuration for scautopick and station GE.UGM:

===========   ==================
Name          Value
===========   ==================
detecStream   BH
timeCorr      -0.8
detecFilter   RMHP(10)>>ITAPER(30)>>BW(4,0.7,2)>>STALTA(2,80)
trigOff       1.5
trigOn        3
===========   ==================

which is the concatenation of the two files :file:`etc/key/global/station_GE_UGM` and :file:`etc/key/scautopick/station_GE_UGM`.


.. _global_modules:

Modules
=======

A :term:`module` is configured by its configuration files either to be used directly or to
generate its native configuration. Modules that need to convert the configuration or do not
use the default configuration options (see below) are called **standalone** modules.

Each standalone module tries to read from three configuration files whereas trunk modules
try to read six files. Note that configuration parameters defined earlier are overwritten
if defined in files read in later:

+---------------------------------+------------+----------------+
| File                            | Standalone | Trunk          |
+=================================+============+================+
|        etc/defaults/global.cfg  |            |    X           |
+---------------------------------+------------+----------------+
|        etc/defaults/module.cfg  |  X         |    X           |
+---------------------------------+------------+----------------+
|        etc/global.cfg           |            |    X           |
+---------------------------------+------------+----------------+
|        etc/module.cfg           |  X         |    X           |
+---------------------------------+------------+----------------+
|        ~/.seiscomp3/global.cfg  |            |    X           |
+---------------------------------+------------+----------------+
|        ~/.seiscomp3/module.cfg  |  X         |    X           |
+---------------------------------+------------+----------------+

The :ref:`configuration section<global-configuration>` describes all available configuration parameters for a trunk module.
Not all modules make use of all available parameters because they may be disabled, e.g. the
messaging component. So the configuration of the messaging server is disabled too.


Extensions
----------

Extensions add new configuration options to :term:`modules<module>`. It does
not matter how those extensions are used. Commonly a module loads a plugin,
which requires additional configuration parameters - these are provided by an extension.

There are currently extensions for the following modules, corresponding to the
plugins shown:

.. include:: /base/extensions.doc

See the documentation for each module for further information about its
extensions.



.. _global-configuration:
