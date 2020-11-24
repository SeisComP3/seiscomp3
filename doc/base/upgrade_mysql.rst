.. _upgrade-mysql:

*******************
MYSQL upgrade guide
*******************

Backup
======

Since this will take a lot of time it and can leave your database in a broken
state in case of an error it is always good practice to create a backup of your
events with :ref:`scxmldump`.

For a full database backup use :code:`mysqldump -u sysop -p seiscomp3 > seiscomp3-backup.sql`.
Depending on the amount of data stored this file can get huge.

Upgrading by conversion
=======================

In order to convert the :code:`_oid` and :code:`_parent_oid` columns to 64bit
the following script should be used. Make sure that your database is the latest
schema version 0.11.

.. code::

   ALTER TABLE Access DROP FOREIGN KEY Access_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Amplitude DROP FOREIGN KEY Amplitude_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE AmplitudeReference DROP FOREIGN KEY AmplitudeReference_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE ArclinkRequest DROP FOREIGN KEY ArclinkRequest_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE ArclinkRequestLine DROP FOREIGN KEY ArclinkRequestLine_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE ArclinkStatusLine DROP FOREIGN KEY ArclinkStatusLine_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE ArclinkUser DROP FOREIGN KEY ArclinkUser_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Arrival DROP FOREIGN KEY Arrival_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE AuxDevice DROP FOREIGN KEY AuxDevice_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE AuxSource DROP FOREIGN KEY AuxSource_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE AuxStream DROP FOREIGN KEY AuxStream_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Comment DROP FOREIGN KEY Comment_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE CompositeTime DROP FOREIGN KEY CompositeTime_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE ConfigModule DROP FOREIGN KEY ConfigModule_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE ConfigStation DROP FOREIGN KEY ConfigStation_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE DataAttributeExtent DROP FOREIGN KEY DataAttributeExtent_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE DataExtent DROP FOREIGN KEY DataExtent_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE DataSegment DROP FOREIGN KEY DataSegment_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE DataUsed DROP FOREIGN KEY DataUsed_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Datalogger DROP FOREIGN KEY Datalogger_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE DataloggerCalibration DROP FOREIGN KEY DataloggerCalibration_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Decimation DROP FOREIGN KEY Decimation_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Event DROP FOREIGN KEY Event_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE EventDescription DROP FOREIGN KEY EventDescription_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE FocalMechanism DROP FOREIGN KEY FocalMechanism_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE FocalMechanismReference DROP FOREIGN KEY FocalMechanismReference_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE JournalEntry DROP FOREIGN KEY JournalEntry_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Magnitude DROP FOREIGN KEY Magnitude_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE MomentTensor DROP FOREIGN KEY MomentTensor_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE MomentTensorComponentContribution DROP FOREIGN KEY MomentTensorComponentContribution_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE MomentTensorPhaseSetting DROP FOREIGN KEY MomentTensorPhaseSetting_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE MomentTensorStationContribution DROP FOREIGN KEY MomentTensorStationContribution_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Network DROP FOREIGN KEY Network_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Origin DROP FOREIGN KEY Origin_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE OriginReference DROP FOREIGN KEY OriginReference_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Outage DROP FOREIGN KEY Outage_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Parameter DROP FOREIGN KEY Parameter_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE ParameterSet DROP FOREIGN KEY ParameterSet_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Pick DROP FOREIGN KEY Pick_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE PickReference DROP FOREIGN KEY PickReference_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE QCLog DROP FOREIGN KEY QCLog_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Reading DROP FOREIGN KEY Reading_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE ResponseFAP DROP FOREIGN KEY ResponseFAP_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE ResponseFIR DROP FOREIGN KEY ResponseFIR_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE ResponseIIR DROP FOREIGN KEY ResponseIIR_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE ResponsePAZ DROP FOREIGN KEY ResponsePAZ_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE ResponsePolynomial DROP FOREIGN KEY ResponsePolynomial_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Route DROP FOREIGN KEY Route_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE RouteArclink DROP FOREIGN KEY RouteArclink_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE RouteSeedlink DROP FOREIGN KEY RouteSeedlink_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Sensor DROP FOREIGN KEY Sensor_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE SensorCalibration DROP FOREIGN KEY SensorCalibration_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE SensorLocation DROP FOREIGN KEY SensorLocation_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Setup DROP FOREIGN KEY Setup_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Station DROP FOREIGN KEY Station_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE StationGroup DROP FOREIGN KEY StationGroup_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE StationMagnitude DROP FOREIGN KEY StationMagnitude_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE StationMagnitudeContribution DROP FOREIGN KEY StationMagnitudeContribution_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE StationReference DROP FOREIGN KEY StationReference_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE Stream DROP FOREIGN KEY Stream_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE WaveformQuality DROP FOREIGN KEY WaveformQuality_ibfk_1, CHANGE _oid _oid BIGINT(20), CHANGE _parent_oid _parent_oid BIGINT(20);
   ALTER TABLE PublicObject DROP FOREIGN KEY PublicObject_ibfk_1, CHANGE _oid _oid BIGINT(20);
   ALTER TABLE Object CHANGE _oid _oid BIGINT(20) AUTO_INCREMENT;
   ALTER TABLE PublicObject ADD CONSTRAINT PublicObject_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Access ADD CONSTRAINT Access_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Amplitude ADD CONSTRAINT Amplitude_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE AmplitudeReference ADD CONSTRAINT AmplitudeReference_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE ArclinkRequest ADD CONSTRAINT ArclinkRequest_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE ArclinkRequestLine ADD CONSTRAINT ArclinkRequestLine_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE ArclinkStatusLine ADD CONSTRAINT ArclinkStatusLine_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE ArclinkUser ADD CONSTRAINT ArclinkUser_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Arrival ADD CONSTRAINT Arrival_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE AuxDevice ADD CONSTRAINT AuxDevice_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE AuxSource ADD CONSTRAINT AuxSource_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE AuxStream ADD CONSTRAINT AuxStream_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Comment ADD CONSTRAINT Comment_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE CompositeTime ADD CONSTRAINT CompositeTime_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE ConfigModule ADD CONSTRAINT ConfigModule_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE ConfigStation ADD CONSTRAINT ConfigStation_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE DataAttributeExtent ADD CONSTRAINT DataAttributeExtent_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE DataExtent ADD CONSTRAINT DataExtent_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE DataSegment ADD CONSTRAINT DataSegment_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE DataUsed ADD CONSTRAINT DataUsed_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Datalogger ADD CONSTRAINT Datalogger_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE DataloggerCalibration ADD CONSTRAINT DataloggerCalibration_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Decimation ADD CONSTRAINT Decimation_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Event ADD CONSTRAINT Event_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE EventDescription ADD CONSTRAINT EventDescription_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE FocalMechanism ADD CONSTRAINT FocalMechanism_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE FocalMechanismReference ADD CONSTRAINT FocalMechanismReference_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE JournalEntry ADD CONSTRAINT JournalEntry_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Magnitude ADD CONSTRAINT Magnitude_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE MomentTensor ADD CONSTRAINT MomentTensor_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE MomentTensorComponentContribution ADD CONSTRAINT MomentTensorComponentContribution_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE MomentTensorPhaseSetting ADD CONSTRAINT MomentTensorPhaseSetting_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE MomentTensorStationContribution ADD CONSTRAINT MomentTensorStationContribution_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Network ADD CONSTRAINT Network_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Origin ADD CONSTRAINT Origin_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE OriginReference ADD CONSTRAINT OriginReference_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Outage ADD CONSTRAINT Outage_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Parameter ADD CONSTRAINT Parameter_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE ParameterSet ADD CONSTRAINT ParameterSet_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Pick ADD CONSTRAINT Pick_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE PickReference ADD CONSTRAINT PickReference_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE QCLog ADD CONSTRAINT QCLog_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Reading ADD CONSTRAINT Reading_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE ResponseFAP ADD CONSTRAINT ResponseFAP_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE ResponseFIR ADD CONSTRAINT ResponseFIR_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE ResponseIIR ADD CONSTRAINT ResponseIIR_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE ResponsePAZ ADD CONSTRAINT ResponsePAZ_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE ResponsePolynomial ADD CONSTRAINT ResponsePolynomial_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Route ADD CONSTRAINT Route_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE RouteArclink ADD CONSTRAINT RouteArclink_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE RouteSeedlink ADD CONSTRAINT RouteSeedlink_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Sensor ADD CONSTRAINT Sensor_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE SensorCalibration ADD CONSTRAINT SensorCalibration_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE SensorLocation ADD CONSTRAINT SensorLocation_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Setup ADD CONSTRAINT Setup_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Station ADD CONSTRAINT Station_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE StationGroup ADD CONSTRAINT StationGroup_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE StationMagnitude ADD CONSTRAINT StationMagnitude_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE StationMagnitudeContribution ADD CONSTRAINT StationMagnitudeContribution_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE StationReference ADD CONSTRAINT StationReference_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE Stream ADD CONSTRAINT Stream_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;
   ALTER TABLE WaveformQuality ADD CONSTRAINT WaveformQuality_ibfk_1 FOREIGN KEY(_oid) REFERENCES Object(_oid) ON DELETE CASCADE;

Save the content of the above code box into a file, e.g. :file:`upgrade-bigint.sql`.
Then run the following command from a terminal:

.. warning::

   The conversion can take a long time depending on the amount of data stored.

.. code::

   # Stop SeisComP and shutdown the database server
   seiscomp stop
   sudo systemctl stop mysql

   # Convert the database
   mysql -u sysop -p seiscomp3 < upgrade-bigint.sql

   # Start the database server and SeisComP
   sudo systemctl start mysql
   seiscomp start

Enter your database password and wait until the script finished. Please do not
abort it otherwise the database will end up in a broken state.


Upgrading by dump, drop and restore
===================================

An alternative to the table conversion is to dump the full database, drop it
and restore it. The only challenge is to modify the database dump to change it
to use the new column types. Actually the type have changed from :code:`INTENGER(11)`
to :code:`BIGINT(20)`.


1. Dump the database

   .. code::

      mysqldump -u sysop -p seiscomp3 > seiscomp3-backup.sql

2. Change the types

   Open :file:`seiscomp3-backup.sql` with a text editor and
   replace **`_oid` int(11) NOT NULL** with **`_oid` bigint(20) NOT NULL**
   and **`_parent_oid` int(11) NOT NULL** with **`_parent_oid` bigint(20) NOT NULL**.

3. Recreate the existing database

   .. code::

      $ mysql -u root -p
      MariaDB [(none)]> drop database seiscomp3

4. Create the database again

   .. code::

      MariaDB [(none)]> create database seiscomp3 character set utf8 collate utf8_bin;

5. Restore the database

   .. code::

      MariaDB [(none)]> source seiscomp3-backup.sql;
