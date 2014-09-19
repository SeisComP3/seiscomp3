.. _system-management:

*****************
System management
*****************

The installation contains various modules for data acquisition, data
archiving, processing, distribution and much more. To control all these
module and to update their configuration the central program :program:`seiscomp`
is used. This is a Python script and it is installed in :file:`bin/seiscomp`.

The entire management framework is built upon Python which is portable to different
platforms. To make :program:`seiscomp` work, ensure that Python is installed on
your system.

:program:`seiscomp` can be called from anywhere in the file system and will source the environment
of its installation automatically. So it is possible to control different
installations.

To get an overview of all available commands, issue

.. code-block:: sh

   /path/to/seiscomp3/bin/seiscomp help

This will print all commands. To get help for a particular command, append
it to the ``help`` command.

.. code-block:: sh

   /path/to/seiscomp3/bin/seiscomp help [command]

Commands
********

**setup**

  .. warning::

     Setup might overwrite previous settings with default values.

  Initializes the configuration of all available modules. Each module implements
  its own setup handler which is called at this point. The initialization takes
  the installation directory into account and should be repeated when copying
  the system to another directory.


**install-deps** [packages]

  Installs 3rd party packages on which SeisComP3 depends such as MySQL. This is
  currently only supported for major Linux distributions. A list of packages
  needs to be given.

  Packages: base, mysql-server, postgresql-server

  #. Install only base system dependencies

     .. code-block:: sh

        seiscomp install-deps base

  #. Install base system dependencies and MYSQL server

     .. code-block:: sh

        seiscomp install-deps mysql-server

  #. Install base system dependencies and PostgreSQL server

     .. code-block:: sh

        seiscomp install-deps postgresql-server

**update-config** [module list]

  Updates the configuration. Modules should be able to read the configuration
  files in :file:`etc` directly, but some modules such as Seedlink need an additional
  step to convert the configuration to their native format. Futhermore all
  trunk station bindings and the inventory need to be sychronized with the
  database. If no module list is given, update-config is called for all available
  modules. Otherwise only the modules passed are updated.

**shell**

  Starts the interactive :ref:`SeisComP shell <system-management-shell>`, an
  approach to make configuration and manipulation of bindings more easy.

**enable** [module list]

  Enables a module to be started and checked automatically when either :command:`start`
  or :command:`check` is called without arguments. This creates a file :file:`etc/init/[module].auto`
  for each module passed.

**disable** [module list]

  The opposite of enable. Removes the file :file:`etc/init/[module].auto` for
  each module passed.

**start** [module-list]

  Starts all modules in [module-list]. If no module is named, all enabled modules are
  started.


**stop** [module-list]

  Stops all modules in [module-list]. If no module name is given, all running modules are
  stopped.


**restart** [module-list]

  Restarts all the given modules. If no module is passed, all running and enabled modules
  are first stopped and then restarted.


**check** [module-list]

  Checks if all passed modules are still running if they have been started.
  If no modules are listed, all modules are checked.

**status** [module-list]

  Prints the status of some or all modules.


**list** modules|aliases|enabled|disabled


**exec** [cmd]


**alias** create|remove new-name name


**print** crontab|env


**help** [command]



.. _system-management-shell:

Shell
*****

The SeisComP shell can be started with

.. code-block:: sh

   user@host:~$ seiscomp3/bin/seiscomp shell

which will open a command prompt. The shell is a helper to manage module station
bindings. Instead of manipulating hundreds of files using difficult commands
such as :command:`sed` in Bash scripts, shell can be used. It supports:

- list available stations
- list available profiles of a module
- list modules to which a station is bound
- bind stations to modules
- delete bindings
- track configuration of a station

.. code-block:: sh

   ================================================================================
   SeisComP shell
   ================================================================================

   Welcome to the SeisComP interactive shell. You can get help about
   available commands with 'help'. 'exit' leaves the shell.

   $

Enter :command:`help` to get a list of supported commands. The results of all
commands issued are written to disk immediately and **not buffered**.

Examples
========

#. Assigning the scautopick global profile to all GE stations

   .. code-block:: sh

      $ set profile scautopick global GE.*

#. Replace all profiles with station configuration for scautopick from GE
   network

   .. code-block:: sh

      $ remove profile scautopick global GE.*

#. Show bindings for station GE.MORC

   .. code-block:: sh

      $ print station GE.MORC
      [global]
      /home/sysop/seiscomp3/etc/key/global/profile_BH
      --------------------------------------------------------------------------------
      detecStream = BH
      --------------------------------------------------------------------------------

      [seedlink]
      /home/sysop/seiscomp3/etc/key/seedlink/profile_geofon
      --------------------------------------------------------------------------------
      sources = chain
      sources.chain.address = geofon.gfz-potsdam.de
      sources.chain.port = 18000
      --------------------------------------------------------------------------------

      [scautopick]
      /home/sysop/seiscomp3/etc/key/scautopick/profile_default
      --------------------------------------------------------------------------------
      detecEnable = true
      detecFilter = "RMHP(10)>>ITAPER(30)>>BW(4,0.7,2)>>STALTA(2,80)"
      trigOn = 3
      trigOff = 1.5
      timeCorr = -0.8
      --------------------------------------------------------------------------------

      [slarchive]
      /home/sysop/seiscomp3/etc/key/slarchive/profile_1day
      --------------------------------------------------------------------------------
      selectors = BHZ.D
      keep = 1
      --------------------------------------------------------------------------------

   This helps to see immediately in which file a certain parameter is
   defined and what module the station is bound to.


Init scripts
************

All module init scripts are placed in :file:`etc/init`. :program:`seiscomp`
loads all .py files and tries to find a class called Module. This class is
then instantiated with the environment object passed as only parameter
to the constructor. If no error occurred then the module is registered.

The name of the init script is ignored and not used furthermore. Only the
name in the Module object is important. It is important to note that only
one module can be placed in one init script.

The Module class must implement the interface used by :program:`seiscomp`.
See :py:class:`seiscomp3.Kernel.Module` for more details.

A simple default implementation looks like this which is available as a
template and can be used directly by using the same name as the module's
name. The modules name in this template is derived from the filename, but this
isn't a general rule as stated before.

.. code-block:: py

   import seiscomp3.Kernel

   class Module(seiscomp3.Kernel.Module):
     def __init__(self, env):
       seiscomp3.Kernel.Module.__init__(self, env, env.moduleName(__file__))


SeisComP3 provides a Python module (:py:mod:`seiscomp3.Kernel`) that allows to
write init scripts in an easy way.


Python kernel module
====================

The SeisComP3 setup kernel module provides interfaces to write init handlers
for modules used by :program:`seiscomp` in Python.

.. py:module:: seiscomp3.Kernel

.. py:class:: Module(env, name)

   :param env: The passes environment from :program:`seiscomp` which is
               stored in self.env.
   :param name: The module name which must be passed by derived classes.
                It is stored in self.name.

   The module interface which implementes the basic default operations.
   Each script can define its own handlers to customize the behaviour.

   .. py:attribute: env

      The kernel environment.

   .. py:attribute: name

      The modules unique name. This name is used for run/pid and log files.
   
   .. py:attribute: order

      The modules start order. The default value is 100 and modules with
      the same value are ordered alphabetically.
   
   .. py:method:: isRunning()

      :rtype: Boolean

      Checks if a module is running. The default implementation returns True
      if the lockfile if not locked.

   .. py:method:: start()

      :rtype: Integer

      Starts a module and returns 0 if no error occured and 1 otherwise. This
      method is called from :program:`seiscomp start`.

   .. py:method:: stop()

      :rtype: Integer

      Stops a module and returns 0 if no error occured and 1 otherwise. This
      method is called from :program:`seiscomp stop`.

   .. py:method:: check()

      :rtype: Integer

      Check is the same as start. The decision whether to check a module
      or not is made :program:`seiscomp` which check the existence
      of the corresponding run file. Returns 1 is case of error, 0 otherwise.

   .. py:method:: status(shouldRun)

      :param shouldRun: Boolean parameter that indicates if the module should
                        run or not. This is evaluated by :program:`seiscomp`.

      Prints the status of the module to stdout. Either is CSV format or as free
      text. This depends on self.env._csv. The default implementations calls

      .. code-block:: py

         self.env.logStatus(self.name, self, self.isRunning(), shouldRun,\
                            self.env.isModuleEnabled(self.name) or \
                            isinstance(self, CoreModule))

   .. py:method:: updateConfig()

      Updates the configuration and bindings based on the module's .cfg files
      and :file:`etc/key/[modname]`. A :term:`trunk` module does not need to
      do anything here. Stand-alone modules need to implement this method to
      convert the configuration to their native format.

      This is called from :program:`seiscomp update-config`.

   .. py:method:: printCrontab()

      Prints crontab entries to stdout. The default implementation does not
      print anything.

      This is called from :program:`seiscomp print crontab`.

.. py:class:: CoreModule(seiscomp3.Kernel.Module)

   The core module interface. A core module is a normal module but is started
   before all modules and stopped afterwards. Core modules are always enabled
   and will be started with :program:`seiscomp start` unless a CoreModule
   implementation applies additional checks in :py:meth:`Module.start`.

   :ref:`scmaster` is a core module which is a requirement for all :term:`trunk`
   modules.

.. py:class:: Environment

   Access to the setup environment.
