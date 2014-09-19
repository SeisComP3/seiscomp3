The email plugin sends emails to configured receipients if a client status
messages passes the configured :confval:`filter <memailplugin.filter>`.

Plugin
======

The email plugin is installed under :file:`share/plugins/monitor/memailplugin.so`.

To add the plugin to :ref:`scm`, add it to the plugin list:

.. code-block:: sh

   plugins = ${plugins}, memailplugin


Examples
========

An example configuration looks like this:

.. code-block:: sh

   # Send notification is a clients CPU usage exceeds 100 percent
   memailplugin.filter = "cpuusage>100"

   # Send emails, yes
   memailplugin.sendEmail = true

   # Send emails to this address(es)
   memailplugin.recipients = operator@my-agency.org, operator2@my-agency.org

   memailplugin.reportSilentClients = false

   # Minutes before report missing clients
   memailplugin.reportRequiredClients = 1

   # Interval to calculate mean of the message values for (in minutes)
   memailplugin.filterMeanInterval = 1

   # List of clients we definitely require to be operative
   memailplugin.requiredClients = scautopick, scautoloc, scevent, scamp,\
                                  scmag, scqc, scevtlog
