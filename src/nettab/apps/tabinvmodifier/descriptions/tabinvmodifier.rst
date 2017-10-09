Sometimes small tweaks need to be made to inventory.
The tabinvmodifier program reads a *rules file* (a network tab file without any station lines) and applies network and station attributes to existing inventory. This method can be used to modify attributes in inventory that originate from dataless SEED or other sources. It can change inventory at the network, station, location, and channel level; it can also change sensor and datalogger attributes (Ia lines).
Currently (2016) changes to station groups (virtual networks) aren't supported.

For details of what can go in a tab file, see
`NETTAB File Format Description <http://www.seiscomp3.org/wiki/doc/special/nettabv2>`_.

tabinvmodifier can either write directly to the inventory in an SC3 database, or dump its output as an XML file.
If output is as an XML file, typically this would then be moved to ~/seiscomp3/etc/inventory, and then loaded into the database with `seiscomp update-config`.

Examples
========

1. Set network-level attributes. Suppose the file `ge.rules` contains

   .. code-block:: sh

      Nw: GE 1993/001
      Na: Description="GEOFON Program, GFZ Potsdam, Germany"
      Na: Remark="Access to Libyan stations and Spanish HH streams limited"
      Na: Type=VBB

   The first line (Nw:) specifies the network, including its start date, that these rules apply to.
   The following lines starting with Na: provide values for the description, remark, and type attributes to be written into the new inventory.
   Note the capital letter on the attributes Description, Remark, Type, etc.

   We can use this rules file to change attributes of the GE network:

   .. code-block:: sh

      # Apply changes to database directly
      $ tabinvmodifier -r ge.rules

      # Apply changes to XML file
      $ tabinvmodifier -r ge.rules --inventory-db ge.xml -o ge-mod.xml

   The resulting inventory now contains:

   .. code-block:: xml

    <network publicID="Network#20130513163612.389203.2" code="GE">
      <start>1993-01-01T00:00:00.0000Z</start>
      <description>GEOFON Program, GFZ Potsdam, Germany</description>
      <institutions>GFZ/partners</institutions>
      <region>euromed global</region>
      <type>VBB</type>
      <netClass>p</netClass>
      <archive>GFZ</archive>
      <restricted>false</restricted>
      <shared>true</shared>
      <remark>access to Libyan stations and Spanish HH streams limited</remark>
      <station publicID="Station#20130620185450.488952.190" code="MSBI" archiveNetworkCode="GE">
        <start>2013-06-16T00:00:00.0000Z</start>

   Other attributes present in inventory are left unchanged.


2. Changing location codes. (Thanks to Andres H. for this example.)
   To replace an empty location code for station "KP.UPNV" with location code "00", together with its description and place.
   The rules file is:

   .. code-block:: sh

    Nw: KP 1980/001
    Sa: Description="GLISN Station Upernavik, Greenland" UPNV
    Sa: Place="Upernavik, Greenland" UPNV
    Sa: Code="00" UPNV, 

   The resulting inventory now contains:

   .. code-block:: xml

    <network publicID="Network#20140603153203.17936.2" code="KP">
      <start>1980-01-01T00:00:00.0000Z</start>
      ...
      <station publicID="Station#20140603153203.179738.3" code="UPNV">
        <start>2013-08-01T00:00:00.0000Z</start>
        <description>GLISN Station Upernavik, Greenland</description>
        <latitude>72.7829</latitude>
        <longitude>-56.1395</longitude>
        <elevation>38</elevation>
        <place>Upernavik, Greenland</place>
        <affiliation>GLISN</affiliation>
        ...
        <sensorLocation publicID="SensorLocation#20140603153203.181119.4" code="00">
          <start>2013-08-01T00:00:00.0000Z</start>
        ...
       </station>
     </network>

