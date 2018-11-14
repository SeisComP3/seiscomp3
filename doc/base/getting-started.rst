.. _getting-started:

***************
Getting started
***************

Once the system is installed it needs to be configured. The central tool to
configure and control the system is :program:`seiscomp` which is explained
more deeply in the :ref:`next chapter<system-management>`.

Initial configuration
=====================

To configure SeisComP3 initially, run :program:`seiscomp setup`. This is the
successor of the former :program:`./setup` script.
The initial configuration also allows to setup the MySQL database for SeisComP3.

As a wrapper to :program:`seiscomp setup`, a wizard can be started from
:ref:`scconfig<scconfig>` (Ctrl-N).

.. note::

    With **Ubuntu 16.04** MariaDB has become the standard flavour of MySQL in Ubuntu
    and either MariaDB or MySQL can be installed. The implementation of MariaDB
    in Ubuntu requires additional steps. They must be taken in order to allow
    SeisComP3 to make use of MariaDB.

    The full procedure including database optimization is:

    .. code-block:: sh

        user@hosst:~$ sudo systemctl enable mysql
        user@host:~$ sudo mysql_secure_installation
            provide new root password
            answer all questions with yes [Enter]

        user@host:~$ sudo mysql -u root -p
            CREATE DATABASE seiscomp3 CHARACTER SET utf8 COLLATE utf8_bin;
            grant usage on seiscomp3.* to sysop@localhost identified by 'sysop';
            grant all privileges on seiscomp3.* to sysop@localhost;
            grant usage on seiscomp3.* to sysop@'%' identified by 'sysop';
            grant all privileges on seiscomp3.* to sysop@'%';
            flush privileges;

        user@host:~$ sudo vim /etc/mysql/mariadb.conf.d/50-server.cnf
            [mysqld]
            innodb_buffer_pool_size = <your value>
            innodb_flush_log_at_trx_commit = 2

        user@host:~$ sudo systemctl restart mysql
        user@host:~$ mysql -u sysop -p seiscomp3 < ~/seiscomp3/share/db/mysql.sql

    Currently, the :ref:`scconfig` wizard and :program:`seiscomp setup` cannot
    be used to set up the MariaDB database. The option "Create database" must
    therefore be unchecked or answered with "no".

In :program:`seiscomp setup` default values are given in brackets [].

.. code-block:: none

   user@host:~$ seiscomp3/bin/seiscomp setup

   ====================================================================
   SeisComP setup
   ====================================================================

   This initializes the configuration of your installation.
   If you already made adjustments to the configuration files
   be warned that this setup will overwrite existing parameters
   with default values. This is not a configurator for all
   options of your setup but helps to setup initial standard values.

   --------------------------------------------------------------------
   Hint: Entered values starting with a dot (.) are handled
         as commands. Available commands are:

         quit: Quit setup without modification to your configuration.
         back: Go back to the previous parameter.
         help: Show help about the current parameter (if available).

         If you need to enter a value with a leading dot, escape it
         with backslash, e.g. "\.value".
   --------------------------------------------------------------------

This will ask for initial settings as database (if package trunk is installed)
parameters and the logging backend.

----

.. code-block:: none

   Organization name []:

Sets the organisation name printed e.g. when you say *hello* to Seedlink
or Arclink.

----

.. code-block:: none

   Enable database storage [yes]:

Enables or disables the database for the system. This option should be left
enabled unless all modules should connect to remote processing machine which
is already available. The database is required to store inventory information
as well as processing results. The database is the central storage for all
trunk modules and the default request handler of Arclink.

----

.. code-block:: none

    0) mysql
         MySQL server.
    1) postgresql
         PostgreSQL server. There is currently no support in setup to create the
         database for you. You have to setup the database and user accounts on
         your own. The database schema is installed under share/db/postgresql.sql.
   Database backend [0]:

If the database is enable the database backend can be selected. SeisComP3
supports two main backends: MySQL and PostgreSQL. Select the backend to be used
here but be prepared that only for the MySQL backend the setup can help to
create the database and tables for you. If you are using PostgreSQL you have
to provide a working database with the correct schema. The schema files are
part of the distribution and can be found in :file:`share/db/postgresql.sql`.

.. note::

   As of PostgreSQL version 9 the default output encoding has changed to hex.
   In order to fix issues with seiscomp3 log in to your database and run the
   following command.

   .. code-block:: sql

      ALTER DATABASE seiscomp3 SET bytea_output TO 'escape';


----

.. code-block:: none

   Create database [yes]:

If MySQL is selected it is possible to let :program:`seiscomp setup` to create
the database and all tables for you. If the database has been created already,
say 'no' here.

----

.. code-block:: none

   MYSQL root password (input not echoed) []:

Give the MySQL root password for your database server to create the database
tables. This is only required if the last question has been answered with 'yes'.

----

.. code-block:: none

   Drop existing database [no]:

If a database with the same name (to be selected later) exists already and the
database should be created for you, an error is raised. To delete an existing
database with the same name, say 'yes' here.

----

.. code-block:: none

   Database name [seiscomp3]:
   Database hostname [localhost]:
   Database read-write user [sysop]:
   Database read-write password [sysop]:
   Database public hostname [localhost]:
   Database read-only user [sysop]:
   Database read-only password [sysop]:

Setup the various database options valid for all database backends. Give
:command:`.help` for more information.

----

If all question have been answered the final choice needs to be made to either
create the initial configuration, go back to the last question or to quit
without doing anything.

.. code-block:: none

   Finished setup
   --------------

   P) Proceed to apply configuration
   B) Back to last parameter
   Q) Quit without changes
   Command? [P]:


Activate modules
================

After the installation all module are disabled for auto start. If :program:`seiscomp start`
is called, nothing will happen. To enable a set of modules,
:program:`seiscomp enable` needs to be called with a list of modules.
For example, for a processing system with Seedlink for data acquisition,
you may use:

.. code-block:: sh

   user@host:~$ seiscomp3/bin/seiscomp enable seedlink scautopick scautoloc scamp scmag scevent
   enabled seedlink
   enabled scautopick
   enabled scautoloc
   enabled scamp
   enabled scmag
   enabled scevent

A successive call of :program:`seiscomp start` will then start all enabled
modules. This is also required to restart enabled modules with :program:`seiscomp check`.

Alternatively, :ref:`scconfig<scconfig>` can be used to enable/disable
and to start/stop/restart modules.

However, before starting seiscomp, station information (metadata) need to
be provided and the configuration needs to be updated.


Supply metadata for networks and stations
=========================================

SeisComP3 requires the metadata from seismic stations for data acquisition
and processing. The metadata can be obtained from network operators or
various other sources in different formats. The metadata include, e.g.:

- network association
- operation times
- location
- sensor and data logger specifications
- data stream specificiations

SeisComP3 comes with various importers to add metadata
for networks and stations including full response information.

:ref:`import_inv` is the tool to import inventory data into SeisComP3.
Alternatively can be used.

.. code-block:: sh

   user@host:~$ seiscomp3/bin/seiscomp exec import_inv dlsv inventory.dataless

This will import a dataless SEED volume into `etc/inventory/inventory.dataless.xml`.

Repeat this step for all inventory data you want to import.


Configure station bindings
==========================

The configuration of modules and bindings is explained in :ref:`global`. To
add bindings in a more convenient way, start :ref:`scconfig`.

.. code-block:: sh

   user@host:~$ seiscomp3/bin/seiscomp exec scconfig


Update configuration and start everything
=========================================

To update the configuration when new stations have been added or modified,
:program:`seiscomp update-config` needs to be run. This creates configuration
files of modules that do not use the configuration directly, writes the trunk
bindings to the database and synchronizes the inventory with the database.

.. code-block:: sh

   user@host:~$ seiscomp3/bin/seiscomp update-config
   [output]

After the configuration has been updated and the inventory has been synchronized,
call :program:`seiscomp start` to start all enabled modules:

.. code-block:: sh

   user@host:~$ seiscomp3/bin/seiscomp start
   starting seedlink
   starting scautopick
   starting scautoloc
   starting scamp
   starting scmag
   starting scevent

Now the system should run. To check everything again, :program:`seiscomp check`
can be run which should print *is running* for all started modules.
If everything is working, the analysis tools can be started, e.g. MapView.

.. code-block:: sh

   user@host:~$ seiscomp3/bin/seiscomp exec scmv
