.. _contributing_documentation:

**************************
Contributing Documentation
**************************

This is the documentation for the core processing elements and utilities that make up the SeisComP3 system. 
It aims to document the configuration and command line options for SC3 in multiple formats (html, man, pdf,
ePub etc) in a consistent way.  The functionality of SC3 differs between versions so the documentation is
versioned along with SC3.  For more general topics and tutorials please refer to the
`SeisComp3 wiki <http://www.seiscomp3.org/>`_.

The documentation is written in `reStructuredText <http://docutils.sourceforge.net/rst.html>`_ (reST) a
simple text mark up format. The documentation is generated using `Sphinx <http://sphinx.pocoo.org/index.html>`_
which is used to create the `Python documentation <http://docs.python.org/>`_.  The Sphinx website has a very
good `introduction to reST <http://sphinx.pocoo.org/rest.html>`_ and also covers the Sphinx specific
`directives <http://sphinx.pocoo.org/markup/index.html>`_.

If you would like to add to this documentation or you find an error then please submit a patch to
`trac <http://www.seiscomp3.org/newticket>`_ or to the mailing list.  

If you are viewing the html version of the documentation in a browser then you can use the *Show Source*
link on each page to view the reST source.  This or the documentation files for executables (see below)
is a good starting point for a patch.

Documenting Executables
=======================

The documentation for executables is generated from two sources:

'executable'.xml
    An xml file that contains a brief description of the command, markup describing the command line parameters,
    and any configuration parameters for the executable.  Each parameter should have a brief description of
    the purpose of the parameter.

    The description should be plain text and not containt reST markup. Where parameters are common accross
    a number of executables they should be placed in the appropriate common file and referred to using
    their publicID.

    All xml files live in the :file:`doc/apps` directory of the source distribution or in :file:`etc/descriptions` of an
    installation.

'executable'.rst
    This is a text file in reST markup that gives any more detailed description and examples for the executable.
    It is combined with the corresponding .xml to create the full documentation.  The first entry in the file
    should be a paragraph giving a more detailed description of the executable.

These two files should be placed in a :file:`descriptions` sub directory of the
respective module, e.g. :file:`src/seedlink/apps/seedlink/descriptions/seedlink.rst`.
The intention is that the documentation is close to the code to make it easier for developers to keep the
documentation up to date with code changes.

For a new executable an entry should also be made in the man section of conf.py. The man page is a short
form of the documentation that is generated from only the .xml file.


Images 
======

Any images should be placed in a suitable sub directory of :file:`descriptions/media`.
They can the be referred to (in .rst) like::

    .. figure::  media/scolv/scolv-overview.png

The images will be moved to the correct location during the documentation build.


Understanding the XML
=====================

As mentioned before XML is used to generate brief descriptions of the configuration and command line parameters.
This section describes the XML format.

Any description XML uses the root element *seiscomp*:

.. code-block:: xml

   <?xml version="1.0" encoding="UTF-8"?>
   <seiscomp>
     ...
   </seiscomp>

3 elements are used inside the root element: :ref:`module<xml-module>`, :ref:`plugin<xml-plugin>` and :ref:`binding<xml-binding>`.
Modules, plugins and bindings can be described in one XML or split up into one file per description. It is better to
have things as close as possible. A module and its binding should go into one module.XML whereas plugins should
go into separate XML files.


.. _xml-module:

Module
------

A module template can be found in :file:`doc/templates/app.xml`.

It describes a binary file, an application that can be started by SeisComP.

Element: **module**

+-----------------------------+----------+-----------+-----------------------------------------------+
| Name                        | XML type | Mandatory | Description                                   |
+=============================+==========+===========+===============================================+
| **name**                    | attrib   |    yes    | The name of the module. This name must be     |
|                             |          |           | unique among all available modules.           |
+-----------------------------+----------+-----------+-----------------------------------------------+
| **category**                | attrib   |    no     | The category of the module. It is used by the |
|                             |          |           | configurator to group modules and by the      |
|                             |          |           | documentation generator to create the final   |
|                             |          |           | document structure. The category can contain  |
|                             |          |           | slashes to introduce hierarchies.             |
+-----------------------------+----------+-----------+-----------------------------------------------+
| **standalone**              | attrib   |    no     | The standalone attribute is also optional and |
|                             |          |           | by default false. Standalone means that the   |
|                             |          |           | module does not take the global configuration |
|                             |          |           | files (eg :file:`etc/global.cfg`) into        |
|                             |          |           | account.                                      |
+-----------------------------+----------+-----------+-----------------------------------------------+
| **inherit-global-bindings** | attrib   |    no     | If global bindings are inherited. The default |
|                             |          |           | is 'false'. If 'yes' then all parameters of   |
|                             |          |           | the global binding are also available in      |
|                             |          |           | the module binding to allow overwriting them. |
|                             |          |           | Standalone modules will never inherit global  |
|                             |          |           | bindings regardless the value of this         |
|                             |          |           | attribute.                                    |
+-----------------------------+----------+-----------+-----------------------------------------------+
| **description**             | element  |    no     | A short description of the module.            |
+-----------------------------+----------+-----------+-----------------------------------------------+
| **configuration**           | element  |    no     | The available configuration parameters. See   |
|                             |          |           | element                                       |
|                             |          |           | :ref:`configuration<xml-configuration>`.      |
+-----------------------------+----------+-----------+-----------------------------------------------+
| **command-line**            | element  |    no     | The available command-line options. See       |
|                             |          |           | element                                       |
|                             |          |           | :ref:`command-line<xml-command-line>`.        |
+-----------------------------+----------+-----------+-----------------------------------------------+

It follows a simple example of how a module definition looks like.

.. code-block:: xml

   <?xml version="1.0" encoding="UTF-8"?>
   <seiscomp>
     <module name="scevent" category="Modules/Processing">
       <description>
         Associates an Origin to an Event or forms a new Event if no match
         is found. Selects the preferred magnitude.
       </description>
       <configuration/>
       <command-line/>
     </module>
   </seiscomp>


.. _xml-plugin:

Plugin
------

A plugin template can be found in :file:`doc/templates/plugin.xml`.

It describes an extension of a modules configuration. This is most likely the
case when an application loads dynamically shared libraries also called plugins.

Element: **plugin**

+-------------------+----------+-----------+-----------------------------------------------+
| Name              | XML type | Mandatory | Description                                   |
+===================+==========+===========+===============================================+
| **name**          | attrib   |    yes    | The name of the plugin.                       |
+-------------------+----------+-----------+-----------------------------------------------+
| **extends**       | element  |    yes    | The list of module names separated by comma   |
|                   |          |           | the plugin extends.                           |
+-------------------+----------+-----------+-----------------------------------------------+
| **description**   | element  |    no     | A short description of the plugin.            |
+-------------------+----------+-----------+-----------------------------------------------+
| **configuration** | element  |    no     | The available configuration parameters. See   |
|                   |          |           | element                                       |
|                   |          |           | :ref:`configuration<xml-configuration>`.      |
+-------------------+----------+-----------+-----------------------------------------------+

It follows a simple example of how a plugin definition looks like.

.. code-block:: xml

   <?xml version="1.0" encoding="UTF-8"?>
   <seiscomp>
     <plugin name="NonLinLoc">
       <extends>global</extends>
       <description>
         NonLinLoc locator wrapper plugin for SeisComP.
         NonLinLoc was written by Anthony Lomax (http://alomax.free.fr/nlloc).
       </description>
       <configuration/>
       <command-line/>
     </plugin>
   </seiscomp>


.. _xml-binding:

Binding
-------

A binding template can be found in :file:`doc/templates/binding.xml`.

It describes the set of configuration parameters to configure a station for a module.

Element: **binding**

+-------------------+----------+-----------+-----------------------------------------------+
| Name              | XML type | Mandatory | Description                                   |
+===================+==========+===========+===============================================+
| **module**        | attrib   |    yes    | The name of the module this binding belongs   |
|                   |          |           | to.                                           |
+-------------------+----------+-----------+-----------------------------------------------+
| **description**   | element  |    no     | A short description of the binding.           |
+-------------------+----------+-----------+-----------------------------------------------+
| **configuration** | element  |    no     | The available configuration parameters. See   |
|                   |          |           | element                                       |
|                   |          |           | :ref:`configuration<xml-configuration>`.      |
+-------------------+----------+-----------+-----------------------------------------------+

It follows a simple example of how a plugin definition looks like.

.. code-block:: xml

   <?xml version="1.0" encoding="UTF-8"?>
   <seiscomp>
     <binding module="seedlink">
       <description>
         Configures sources and parameters of a SeedLink station.
       </description>
       <configuration/>
     </binding>
   </seiscomp>


.. _xml-configuration:

Configuration
-------------

This element is used to describe the configuration parameters (not command-line, just
configuration file) of a module, binding and plugin.

Element: **configuration**

+-------------------+----------+-----------+---------------------------------------------------+
| Name              | XML type | Mandatory | Description                                       |
+===================+==========+===========+===================================================+
| **parameter**     | element  |    no     | A top level parameter that does not contain       |
|                   |          |           | dots in the configuration file.                   |
|                   |          |           |                                                   |
|                   |          |           | .. code-block:: sh                                |
|                   |          |           |                                                   |
|                   |          |           |    param = value                                  |
|                   |          |           |    group.param = "another value"                  |
|                   |          |           |                                                   |
|                   |          |           | Here ``param`` is a top level parameter           |
|                   |          |           | wheras ``group.param`` is not. See                |
|                   |          |           | :ref:`parameter<xml-configuration-parameter>`.    |
+-------------------+----------+-----------+---------------------------------------------------+
| **struct**        | element  |    no     | A top level structure definition. Structures      |
|                   |          |           | are different from groups and parameters          |
|                   |          |           | as they can be instantiated by an arbitrary       |
|                   |          |           | name.                                             |
+-------------------+----------+-----------+---------------------------------------------------+
| **group**         | element  |    no     | A parameter group that describes a logical        |
|                   |          |           | grouping of parameters also called "scope" or     |
|                   |          |           | "namespace". If a parameter in the                |
|                   |          |           | configuration file contains dots then only        |
|                   |          |           | the last part is a parameter all others are       |
|                   |          |           | groups.                                           |
|                   |          |           |                                                   |
|                   |          |           | .. code-block:: sh                                |
|                   |          |           |                                                   |
|                   |          |           |    group1.group2.param = value                    |
|                   |          |           |                                                   |
+-------------------+----------+-----------+---------------------------------------------------+


.. _xml-configuration-parameter:

Element: **parameter**

+-------------------+----------+-----------+---------------------------------------------------+
| Name              | XML type | Mandatory | Description                                       |
+===================+==========+===========+===================================================+
| **name**          | attrib   |    yes    | The name of the parameter. This name must be      |
|                   |          |           | unique among all parameters of the same           |
|                   |          |           | level.                                            |
+-------------------+----------+-----------+---------------------------------------------------+
| **type**          | attrib   |    no     | An optional description of the parameter          |
|                   |          |           | type which can be interpreted by a                |
|                   |          |           | configurator to provide specialized input         |
|                   |          |           | widgets. It is also important for the user        |
|                   |          |           | how the parameter is read by the module.          |
+-------------------+----------+-----------+---------------------------------------------------+
| **unit**          | attrib   |    no     | An optional unit such as "s" or "km" or           |
|                   |          |           | "deg".                                            |
+-------------------+----------+-----------+---------------------------------------------------+
| **default**       | attrib   |    no     | The default value the module uses if this         |
|                   |          |           | parameter is not configured.                      |
+-------------------+----------+-----------+---------------------------------------------------+
| **description**   | element  |    no     | Gives a brief description of the parameter.       |
+-------------------+----------+-----------+---------------------------------------------------+

.. _xml-configuration-struct:

Element: **struct**

+-------------------+----------+-----------+---------------------------------------------------+
| Name              | XML type | Mandatory | Description                                       |
+===================+==========+===========+===================================================+
| **type**          | attrib   |    yes    | The name of the struct type. This name is         |
|                   |          |           | used in a configurator to give a selection        |
|                   |          |           | of available types to be instantiated.            |
+-------------------+----------+-----------+---------------------------------------------------+
| **link**          | attrib   |    no     | The absolute reference parameter as it would      |
|                   |          |           | appear in the configuration file which            |
|                   |          |           | holds all instantiated structures.                |
|                   |          |           |                                                   |
|                   |          |           | .. code-block:: sh                                |
|                   |          |           |                                                   |
|                   |          |           |    # 'link' parameter holding all available       |
|                   |          |           |    # structures. "local" and "teleseismic"        |
|                   |          |           |    # are instances of a structure defined         |
|                   |          |           |    # below.                                       |
|                   |          |           |    locator.profiles = local, teleseismic          |
|                   |          |           |                                                   |
|                   |          |           |    # The structure defined in locator.profile     |
|                   |          |           |    # would have "locator.profiles" as link        |
|                   |          |           |    # attribute.                                   |
|                   |          |           |    locator.profile.local.param = value            |
|                   |          |           |    locator.profile.teleseismic.param = value      |
|                   |          |           |                                                   |
+-------------------+----------+-----------+---------------------------------------------------+
| **description**   | element  |    no     | Gives a brief description of the parameter.       |
+-------------------+----------+-----------+---------------------------------------------------+
| **parameter**     | element  |    no     | Describes a parameter in the struct. See          |
|                   |          |           | :ref:`parameter<xml-configuration-parameter>`.    |
+-------------------+----------+-----------+---------------------------------------------------+
| **struct**        | element  |    no     | Describes a struct part of this struct.           |
+-------------------+----------+-----------+---------------------------------------------------+
| **group**         | element  |    no     | Describes a group part of this struct. See        |
|                   |          |           | :ref:`group<xml-configuration-group>`.            |
+-------------------+----------+-----------+---------------------------------------------------+



.. _xml-configuration-group:

Element: **group**

+-------------------+----------+-----------+---------------------------------------------------+
| Name              | XML type | Mandatory | Description                                       |
+===================+==========+===========+===================================================+
| **name**          | attrib   |    yes    | The name of the group. This name must be          |
|                   |          |           | unique among all groups of the same level.        |
+-------------------+----------+-----------+---------------------------------------------------+
| **description**   | element  |    no     | Gives a brief description of the parameter.       |
+-------------------+----------+-----------+---------------------------------------------------+
| **parameter**     | element  |    no     | Describes a parameter in the group. See           |
|                   |          |           | :ref:`parameter<xml-configuration-parameter>`.    |
+-------------------+----------+-----------+---------------------------------------------------+
| **struct**        | element  |    no     | Describes a struct part of this group. See        |
|                   |          |           | :ref:`struct<xml-configuration-struct>`.          |
+-------------------+----------+-----------+---------------------------------------------------+
| **group**         | element  |    no     | Describes a group part of this group.             |
+-------------------+----------+-----------+---------------------------------------------------+


It follows an example of the plugin definition for the NonLinLoc plugin. It contains
groups, parameters and structures.

.. code-block:: xml

   <?xml version="1.0" encoding="UTF-8"?>
   <seiscomp>
     <plugin name="NonLinLoc">
       <extends>global</extends>
       <description>...</description>
       <configuration>
         <group name="NonLinLoc">
           <parameter name="publicID" type="string"
                      default="NLL.@time/%Y%m%d%H%M%S.%f@.@id@">
             <description>
               PublicID creation pattern for an origin created by NonLinLoc.
             </description>
           </parameter>

           <parameter name="outputPath" type="path" default="/tmp/sc3.nll">
             <description>
               Defines the output path for all native NonLinLoc input and
               output files.
             </description>
           </parameter>

           <parameter name="profiles" type="list:string">
             <description>
               Defines a list of active profiles to be used by the plugin.
             </description>
           </parameter>

           <group name="profile">
             <struct type="NonLinLoc profile" link = "NonLinLoc.profiles">
               <description>
                 Defines a regional profile that is used if a prelocation falls
                 inside the configured region.
               </description>
               <parameter name="earthModelID" type="string">
                 <description>
                   earthModelID that is stored in the created origin.
                 </description>
               </parameter>
             </struct>
           </group>
         </group>
       </configuration>
     </plugin>
   </seiscomp>



.. _xml-command-line:

Command-line
------------

This element is used to describe the command-line options of a module. The element structure is
much simpler than the :ref:`configuration<xml-configuration>` element. The command-line only
contains group elements which in turn have either option or optionReference elements. Through
the optionReference element it is possible to refer to existing command-line options. This is
important for all modules that are using the SeisComP3 libraries because they share a set of
basic command-line options inherited from the Application class.

Element: **command-line**

+---------------------+----------+-----------+-----------------------------------------------+
| Name                | XML type | Mandatory | Description                                   |
+=====================+==========+===========+===============================================+
| **synopsis**        | element  |    no     | Optional description of how to start the      |
|                     |          |           | module.                                       |
+---------------------+----------+-----------+-----------------------------------------------+
| **description**     | element  |    no     | Optional description of the command-line      |
|                     |          |           | and non option parameters.                    |
+---------------------+----------+-----------+-----------------------------------------------+
| **group**           | element  |    no     | Describes an option group. See                |
|                     |          |           | :ref:`group<xml-command-line-group>`.         |
+---------------------+----------+-----------+-----------------------------------------------+


.. _xml-command-line-group:

Element: **group**

+---------------------+----------+-----------+-----------------------------------------------+
| Name                | XML type | Mandatory | Description                                   |
+=====================+==========+===========+===============================================+
| **name**            | attrib   |    yes    | The name of the group. This name must be      |
|                     |          |           | unique among all groups of the same level.    |
+---------------------+----------+-----------+-----------------------------------------------+
| **option**          | element  |    no     | An option part of this group. See             |
|                     |          |           | :ref:`option<xml-command-line-option>`.       |
+---------------------+----------+-----------+-----------------------------------------------+
| **optionReference** | element  |    no     | A reference to an existing option using its   |
|                     |          |           | publicID.                                     |
+---------------------+----------+-----------+-----------------------------------------------+


.. _xml-command-line-option:

Element: **option**

+---------------------+----------+-----------+-----------------------------------------------+
| Name                | XML type | Mandatory | Description                                   |
+=====================+==========+===========+===============================================+
| **flag**            | attrib   |    semi   | The short option flag. Either this attribute  |
|                     |          |           | or long-flag must be set.                     |
+---------------------+----------+-----------+-----------------------------------------------+
| **long-flag**       | attrib   |    semi   | The long option flag. Either this attribute   |
|                     |          |           | or flag must be set.                          |
+---------------------+----------+-----------+-----------------------------------------------+
| **param-ref**       | attrib   |    no     | Refers to a configuration parameter name that |
|                     |          |           | this parameter overrides. Name is the full    |
|                     |          |           | path, e.g. *connection.server* and not just   |
|                     |          |           | *server*.                                     |
+---------------------+----------+-----------+-----------------------------------------------+
| **argument**        | attrib   |    no     | The optional argument string. If argument is  |
|                     |          |           | not set the option is a switch.               |
+---------------------+----------+-----------+-----------------------------------------------+
| **default**         | attrib   |    no     | The option's default value used if the option |
|                     |          |           | is not given though it is hard in most cases  |
|                     |          |           | because command-line options very often       |
|                     |          |           | redefine configuration parameters which is    |
|                     |          |           | then used as a default value for the option.  |
+---------------------+----------+-----------+-----------------------------------------------+
| **publicID**        | attrib   |    no     | The optional publicID of the option to be     |
|                     |          |           | able to reference it from an optionReference  |
|                     |          |           | element. The publicID must be unique among    |
|                     |          |           | all defined options.                          |
+---------------------+----------+-----------+-----------------------------------------------+
| **description**     | element  |    no     | Gives a brief description of the option.      |
+---------------------+----------+-----------+-----------------------------------------------+


It follows an example of the module definition for scautoloc (extract).

.. code-block:: xml

   <?xml version="1.0" encoding="UTF-8"?>
   <seiscomp>
     <module name="scautoloc" category="Modules/Processing">
       <description>Locates seismic events.</description>
       <configuration>
         ...
       </configuration>
       <command-line>
         <group name="Generic">
           <optionReference>generic#help</optionReference>
           <optionReference>generic#version</optionReference>
           <optionReference>generic#config-file</optionReference>
           <optionReference>generic#plugins</optionReference>
           <optionReference>generic#daemon</optionReference>
           <optionReference>generic#auto-shutdown</optionReference>
           <optionReference>generic#shutdown-master-module</optionReference>
           <optionReference>generic#shutdown-master-username</optionReference>
         </group>

         <group name="Mode">
           <option flag="" long-flag="test" argument="" default="">
             <description>Do not send any object</description>
           </option>

           <option flag="" long-flag="offline" argument="" default="">
             <description>
               Do not connect to a messaging server. Instead a
               station-locations.conf file can be provided. This implies
               --test and --playback
             </description>
           </option>

           <option flag="" long-flag="playback" argument="" default="">
             <description>Flush origins immediately without delay</description>
           </option>
         </group>
       </command-line>
     </module>
   </seiscomp>
