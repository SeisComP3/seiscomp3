************
Installation
************

SeisComP3 is distributed in the form of tar files for different releases,
Linux systems and architectures:

* Acquisition, processing and GUIs (for each supported platform)
* Maps (maps from the SeisComP3 releases Seattle and Zurich also work in Jakarta)
* Documentation
* Station configuration files (optional)

Download these from http://www.seiscomp3.org/ .
This section describes the installation of the binary packages of SeisComP3 on
an 

* :program:`Ubuntu 14`, 64 bit system
* :program:`openSUSE 11`, 64 bit system
* :program:`CentOS 6`, 64 bit system


Requirements
============

The hardware requirements for a seismic system depend on the size of the
station network to be operated.

Minimum requirements are:

+-----+----------------------------------------------------------------------------------------+
| CPU | 1                                                                                      |
+-----+----------------------------------------------------------------------------------------+
| RAM | 2 GB                                                                                   |
+-----+----------------------------------------------------------------------------------------+
| HDD | 20 GB                                                                                  |
+-----+----------------------------------------------------------------------------------------+
| OS  | SUSE 10.2/3 32/64bit, SUSE 11 32/64bit, (K)Ubuntu 7/8 32/64bit, Debian 5.0, CentOS 5.3 |
+-----+----------------------------------------------------------------------------------------+

In case large networks (>100 stations) are operated, a distributed system is
recommended. Normally a SeisComP3 system is separated in several subsystems.
A separation of data acquisition, processing and graphical user interfaces is
useful to permit stable performance.

The minimum specifications of the system should be:

Data acquisition system:

+-----+----------------------------------------------------------------+
| CPU | 1                                                              |
+-----+----------------------------------------------------------------+
| RAM | 2 GB                                                           |
+-----+----------------------------------------------------------------+
| HDD | Raid1/5/0+1 with >= 200GB                                      |
+-----+----------------------------------------------------------------+


Processing system:

+-----+----------------------------------------------------------------+
| CPU | 2                                                              |
+-----+----------------------------------------------------------------+
| RAM | 4 GB                                                           |
+-----+----------------------------------------------------------------+
| HDD | Raid1/5/0+1 with >= 100GB                                      |
+-----+----------------------------------------------------------------+

GUI system:

+-----+----------------------------------------------------------------+
| CPU | 2                                                              |
+-----+----------------------------------------------------------------+
| RAM | 4 GB                                                           |
+-----+----------------------------------------------------------------+
| HDD | > 50 GB                                                        |
+-----+----------------------------------------------------------------+



Installation procedure
======================

The next steps describe the installation of SeisComP3 with the prepared
tar.gz files. 

* Log in as user (e.g. sysop)
* Copy one of the :file:`seiscomp3-jakarta-[version]-[OS]-[arch].tar.gz` files to
  your home directory. Take care which is the right package (32 or 64-bit) for
  your operating system.

* Go to home directory
     
  .. code-block:: sh
  
     user@host:/tmp$ cd

* Un-tar the SeisComP3 binary packagemake 
   
  .. code-block:: sh

     user@host:~$ tar xzf seiscomp3-jakarta-[version]-[OS]-[arch].tar.gz

* Un-tar the SeisComP3 map package into seiscomp3/share/maps

  .. code-block:: sh

     user@host:~$ tar xzf seiscomp3-[release]-maps.tar.gz

* If desired, un-tar the documentation into seiscomp3/share/doc

  .. code-block:: sh

     user@host:~$ tar xzf seiscomp3-jakarta-[version]-doc.tar.gz

Unpacking these file creates the :ref:`SeisComP3 directory structure<directory_structure>`.

Install dependencies
--------------------

SeisComP3 depends on a number of additional packages shipped with each Linux
distribution. The following table gives an overview (the names of packages, 
files or commands may differ slightly for other Linux systems):

:program:`Packages`

+--------------------+--------------------+----------------------+----------------------------------------+
|:program:`Ubuntu 14`|:program:`OpenSUSE` |:program:`CentOS 6`   | SeisComP3 component                    |
+====================+====================+======================+========================================+
| flex               | flex               | flex                 | Seedlink (compilation only)            |
+--------------------+--------------------+----------------------+----------------------------------------+
| libxml2            | libxml2            | libxml2              | Seedlink, Arclink, trunk               |
+--------------------+--------------------+----------------------+----------------------------------------+
| libboost           | libboost           | boost                | trunk                                  |
+--------------------+--------------------+----------------------+----------------------------------------+
| libboost-dev       | libboost-dev       | boost-devel          | trunk (compilation only)               |
+--------------------+--------------------+----------------------+----------------------------------------+
| libncurses5        | libncurses         | ncurses              | trunk:scm (optional)                   |
+--------------------+--------------------+----------------------+----------------------------------------+
| libncurses5-dev    | libncurses-dev     | ncurses-devel        | trunk:scm (compilation only, optional) |
+--------------------+--------------------+----------------------+----------------------------------------+
| mysql-client       | libmysqlclient     | mysql                | trunk (only if MySQL is used)          |
+--------------------+--------------------+----------------------+----------------------------------------+
| libmysqlclient-dev | libmysqlclient-dev | mysql-devel          | trunk (compilation only if enabled)    |
+--------------------+--------------------+----------------------+----------------------------------------+
| mysql-server       | mysql-server       | mysql-server         | trunk (only if MySQL is used locally)  |
+--------------------+--------------------+----------------------+----------------------------------------+
| libpq5             | libpq5             | postgresql           | trunk (only if PostgreSQL is used)     |
+--------------------+--------------------+----------------------+----------------------------------------+
| libpq-dev          | libpq-dev          | postgresql-devel     | trunk (compilation only if enabled)    |
+--------------------+--------------------+----------------------+----------------------------------------+
| libqt4-dev         | libqt4             | qt4                  | trunk (only GUI should be used)        |
+--------------------+--------------------+----------------------+----------------------------------------+
| python-dev         | python-dev         | python-devel         | trunk (compilation only)               |
+--------------------+--------------------+----------------------+----------------------------------------+
| festival           | festival           | festival             | trunk (optional voice alert)           |
+--------------------+--------------------+----------------------+----------------------------------------+


First the environment has to be set up. The :program:`seiscomp` tool comes with
the command :command:`install-deps` which installs required packages.
Read the section :ref:`System management<system-management>` for more detailed instructions.
For example, to install the dependencies for using the MySQL database,
give 'mysql-server' as parameter. 

.. code-block:: sh

   user@host:~$ seiscomp3/bin/seiscomp install-deps base mysql-server
   Distribution: Ubuntu 10.04
   [sudo] password for sysop:
   Reading package lists... Done
   Building dependency tree
   Reading state information... Done
   ...

   
If your distribution is not supported by :command:`install-deps`
, install the above packages manually:

:program:`Ubuntu` `version`

.. code-block:: sh

   user@host:~$ cd seiscomp3/share/deps/ubuntu/[version]
   ...
   
   
:program:`OpenSUSE` `version`

.. code-block:: sh

   user@host:~$ cd seiscomp3/share/deps/sles/[version]
   ...
   
   
:program:`CentOS` `version`

.. code-block:: sh

   user@host:~$ cd seiscomp3/share/deps/centos/[version]
   ...

   
   
.. code-block:: sh

   su root
   bash install-mysql-server.sh
   bash install-postgresql-server.sh
   bash install-base.sh
   bash install-gui.sh
   ...
   
or contact the SeisComP3 developpers to add support for your distribution.
   
SQL configuration
-----------------

* For better performance with a MySQL database, adjust the following parameters:

  * "innodb_buffer_pool_size = 64M"
  * "innodb_flush_log_at_trx_commit = 2"

  The location of the configuration can differ between distributions. 
  
  :program:`OpenSUSE`
  
  :file:`/etc/my.cnf` 
  
  :program:`Ubuntu 14`
  
  :file:`/etc/mysql/my.cnf`  or :file:`/etc/mysql/conf.d/*`
  
  :program:`CentOS`
  
  :file:`/etc/my.cnf`
  
  Please read the documentation of your distribution. root privileges may 
  be required to make the changes.

*  After adjusting the parameters, MySQL needs to be restarted. One can run

  :program:`OpenSUSE`
  
  .. code-block:: sh

     user@host:~$ sudo rcmysql restart

  :program:`Ubuntu 14`

  .. code-block:: sh

     user@host:~$ sudo restart mysql

  :program:`CentOS`

  .. code-block:: sh

     user@host:~$ su root
     user@host:~$ /sbin/service mysqld restart


* To start MySQL automatically during boot set

  :program:`OpenSUSE`

  .. code-block:: sh

     user@host:~$ insserv mysql

  :program:`Ubuntu 14`

  .. code-block:: sh

     user@host:~$ sudo update-rc.d mysql defaults
     
  :program:`CentOS`

  .. code-block:: sh

     user@host:~$ su root
     user@host:~$ /sbin/chkconfig mysqld on

Now everything is installed and the system can be configured. The :ref:`next chapter<getting-started>`
chapter explains the first steps.

.. _directory_structure:

Directory structure
===================

The directory structure of the installed system is described with the
following table.

+---------------------+--------------------------------------------------------------------+
| Directory           | Description                                                        |
+=====================+====================================================================+
| *bin*               | The user module binaries.                                          |
+---------------------+--------------------------------------------------------------------+
| *lib*               | The base library directory used by all modules.                    |
+---------------------+--------------------------------------------------------------------+
| *lib/python*        | The Python library directory.                                      |
+---------------------+--------------------------------------------------------------------+
| *man*               | The manual pages.                                                  |
+---------------------+--------------------------------------------------------------------+
| *sbin*              | The system/service/server binaries such as seedlink.               |
+---------------------+--------------------------------------------------------------------+
| *var*               | Variable files whose content is expected to continually change.    |
+---------------------+--------------------------------------------------------------------+
| *var/log*           | Log files of started modules. Usually modules log either to syslog |
|                     | or ~/.seiscomp3/log. This directory contains the logs of the start |
|                     | of each module.                                                    |
+---------------------+--------------------------------------------------------------------+
| *var/lib*           | Default directory for files created by modules such as the         |
|                     | waveform ringbuffer of SeedLink or the waveform archive created    |
|                     | by slarchive.                                                      |
+---------------------+--------------------------------------------------------------------+
| *var/run*           | Contains the .run and .pid files of modules started by             |
|                     | :program:`seiscomp`.                                               |
+---------------------+--------------------------------------------------------------------+
| *include*           | SDK header files for all libraries.                                |
+---------------------+--------------------------------------------------------------------+
| *share*             | Application data such as maps, cities.xml and others.              |
+---------------------+--------------------------------------------------------------------+
| *share/templates*   | Template files used by e.g. SeedLink to create its native          |
|                     | configuration.                                                     |
+---------------------+--------------------------------------------------------------------+
| *etc*               | Configuration directory.                                           |
+---------------------+--------------------------------------------------------------------+
| *etc/descriptions*  | Contains all XML module descriptions.                              |
+---------------------+--------------------------------------------------------------------+
| *etc/defaults*      | The default configuration files. This directory is read as first   |
|                     | when a module starts.                                              |
+---------------------+--------------------------------------------------------------------+
| *etc/init*          | Module init scripts called by :program:`seiscomp`.                 |
+---------------------+--------------------------------------------------------------------+
| *etc/key*           | Station configurations and module bindings.                        |
+---------------------+--------------------------------------------------------------------+
