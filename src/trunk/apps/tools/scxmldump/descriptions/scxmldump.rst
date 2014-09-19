To get event information from the database without using SQL is an important
task for the user. :ref:`scxmldump` queries the database and transforms that
information into XML. The result can be converted into a bulletin with
scbulletin or imported into another database with :ref:`scdb`.

An XSD schema of the XML output can be downloaded
at http://geofon.gfz-potsdam.de/schema/.

Examples
--------

Export inventory

.. code-block:: sh

   scxmldump -fI -o inventory.xml -d mysql://sysop:sysop@localhost/seiscomp3

Export configuration

.. code-block:: sh

   scxmldump -fC -o config.xml -d mysql://sysop:sysop@localhost/seiscomp3

Export full event data

.. code-block:: sh

   scxmldump -fPAMF -E test2012abcd -o test2012abcd.xml \
             -d mysql://sysop:sysop@localhost/seiscomp3


Export summary event data

.. code-block:: sh

   scxmldump -fap -E test2012abcd -o test2012abcd.xml \
             -d mysql://sysop:sysop@localhost/seiscomp3


Create bulletin

.. code-block:: sh

   scxmldump -fPAMF -E test2012abcd
             -d mysql://sysop:sysop@localhost/seiscomp3 | \
   scbulletin


Copy event

.. code-block:: sh

   scxmldump -fPAMF -E test2012abcd \
             -d mysql://sysop:sysop@localhost/seiscomp3 | \
   scdb -i - -d mysql://sysop:sysop@archive-db/seiscomp3
