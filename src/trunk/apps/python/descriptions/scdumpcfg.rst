scdumpcfg reads and prints the module or the bindings configuration for a specific module or
for global. This commandline utility is useful for debugging of configuration parameters.

Examples
========

#. Dump the global bindings configuration for all stations which have global bindings.

   .. code-block:: sh

      scdumpcfg global -d mysql://sysop:sysop@seiscomp3/localhost -B

#. Dump the bindings configuration for all stations which have bindings to a
   scautopick profile. Additionally use *-G* as scautopick inherits global bindings.

   .. code-block:: sh

      scdumpcfg scautopick -d mysql://sysop:sysop@seiscomp3/localhost -GB

#. Dump the module global module configuration specifcally searching for the map
   zoom sensitivity and output the result in the format of the SeisComP3 module
   configuration.

   .. code-block:: sh

      scdumpcfg global -d mysql://sysop:sysop@seiscomp3/localhost --cfg -P map.zoom.sensitivity

#. Dump the module configuration of scautopick and output in the format of the
   SeisComP3 module configuration.

   .. code-block:: sh

      scdumpcfg scautopick -d mysql://sysop:sysop@seiscomp3/localhost --cfg
