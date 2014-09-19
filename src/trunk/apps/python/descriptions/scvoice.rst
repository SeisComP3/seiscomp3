This tool runs an external script whenever an event has been created or updated.
It can also run a script in case an amplitude of a particular type or a
preliminary origin (heads-up message) has been sent. The common purpose for
this tool is to play a sound or to convert a message to speech using external
tools like festival or espeak. There are three possible script calls:

- Event creation/update
- Amplitude creation
- Origin creation (with status = preliminary)

Although this tool was designed to alert the user acoustically it can also be
used to send e-mails, sms or to do any other kind of alert. scvoice can only
run one script per call type at a time! A template (:ref:`scalert`) has been
added to SeisComP3 to be used as source for custom notifications.


Examples
========

The following script is used as event script. It requires
`festival <http://www.cstr.ed.ac.uk/projects/festival/>`_ which should be
available in almost any Linux distribution.

.. important::
   When saving the scripts given below do not forget to set the executable
   bit otherwise scvoice cannot call the scripts. In Linux just run:

   .. code-block:: sh

      chmod +x /path/to/file


.. code-block:: sh

   #!/bin/sh
   if [ "$2" = "1" ]; then
   echo " $1" | sed 's/,/, ,/g'   | festival --tts;
   else
   echo "Event updated, $1" | sed 's/,/, ,/g'   | festival --tts;
   fi

Save this script e.g. under :file:`~/.seiscomp3/event.sh` and add it to the
configuration file :file:`~/.seiscomp3/scvoice.cfg`:

.. code-block:: sh

   scripts.event = /home/sysop/.seiscomp3/event.sh

Amplitude script:


.. code-block:: sh

   #!/bin/sh
   # Play a wav file with a particular volume
   # derived from the amplitude itself.
   playwave ~/.seiscomp3/beep.wav -v $3

Save this script e.g. under :file:`~/.seiscomp3/amplitude.sh` and add it to
the configuration file :file:`~/.seiscomp3/scvoice.cfg`.

.. code-block:: sh

   scripts.amplitude = /home/sysop/.seiscomp3/amplitude.sh

Alert script:

.. code-block:: sh

   #!/bin/sh
   playwave /home/sysop/.seiscomp3/siren.wav

Save this script e.g. under :file:`~/.seiscomp3/alert.sh` and add it to
the configuration file :file:`~/.seiscomp3/scvoice.cfg`.

.. code-block:: sh

   scripts.alert = /home/sysop/.seiscomp3/alert.sh
