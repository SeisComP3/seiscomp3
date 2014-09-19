The text plugin writes for each client a status text file to a
:confval:`configurable <mtextplugin.outputDir>` directory. Each text file
is named after the client with the extension ".txt".

Plugin
======

The text plugin is installed under :file:`share/plugins/monitor/mtextplugin.so`.

To add the plugin to :ref:`scm`, add it to the plugin list:

.. code-block:: sh

   plugins = ${plugins}, mtextplugin

