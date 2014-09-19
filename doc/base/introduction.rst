**********************
Introduction and Scope
**********************

SeisComP is likely the most widely distributed software package for
seismological data acquisition and real-time data exchange over internet. Its
data transmission protocol SeedLink became a de facto world standard. The first
version of SeisComP was developed for the
`GEOFON <http://geofon.gfz-potsdam.de/geofon/>`_ network and further extended
within the MEREDIAN project under the lead of
`GEOFON <http://geofon.gfz-potsdam.de/geofon/>`_/`GFZ <http://www.gfz-potsdam.de/>`_
Potsdam and `ORFEUS <http://www.orfeus-eu.org/>`_. Originally SeisComP was
designed as a high standard fully automatic data acquisition and (near-)real-time
data processing tool including quality control, event detection and location as
well as dissemination of event alerts. In the context of the
`GITEWS <http://www.gitews.de/>`_ project (German Indian Ocean Tsunami Early
Warning System) additional functionality were implemented to fulfil the
requirements of 24/7 early warning control centers. Major changes in the
architecture of SeisComP were necessary and many new features result in the
upgrade of SeisComP to version 3. Important SeisComP releases are shown below.
A first prototype of SeisComP3 developed by the GITEWS/GEOFON development group
was released in May 2007. SeisComP3 provides the following features:

* data acquisition
* data quality control
* data recording
* real-time data exchange
* network status monitoring
* real-time data processing
* issuing event alerts
* waveform archiving
* waveform data distribution
* automatic event detection and location
* interactive event detection and location
* event parameter archiving
* easy access to relevant information about stations, waveforms and recent
  earthquakes

The new requirements for early warning purposes made it necessary to adopt the
design and architecture of the previous SeisComP. The guidelines for the design
of SeisComP3 are:

* implementation of critical functions as standalone modules to guarantee the
  independence from other functions (e.g. picker, magnitude calculation,
  interactive analysis)
* easy implementation of custom modules
* independence of hard- and software
* ability of data exchange between different automatic real-time systems
* distribution of modules on several systems
* robust system for rapid and reliable earthquake solutions (especially during
  seismic crises)

+---------+--------------------------------+-----------------------------------------------------+
| Version | Time                           |                                                     |
+=========+================================+=====================================================+
| 1.0     | February 2001                  | SeedLink 2.0 (plugin interface) Plugins for         |
|         |                                | EarthData PS2400 and Lennartz M24                   |
+---------+--------------------------------+-----------------------------------------------------+
| 1.1     | August 2001                    | SeedLink 2.1 (streams.xml, improved buffer          |
|         |                                | structure); make conf/make key scripts LISS         |
|         |                                | plugin, SeedLink-Antelope connectivity              |
+---------+--------------------------------+-----------------------------------------------------+
| 1.1.5   | January 2002                   | SeedLink 2.5 (multi-station mode)                   |
+---------+--------------------------------+-----------------------------------------------------+
| 1.16    | March 2002                     | GIF live seismograms                                |
+---------+--------------------------------+-----------------------------------------------------+
| 2.0     | October 2003                   | SeedLink 3.0 (INFO request, time window extraction) |
|         |                                | libslink, chain plugin, Comserv-independence        |
+---------+--------------------------------+-----------------------------------------------------+
| 2.1     | June 2004                      | Python add-on package (SeisPy) incl. AutoLoc2 chain |
|         |                                | plugin extension interface, triggered streams       |
+---------+--------------------------------+-----------------------------------------------------+
| 2.5     | March 2006                     | Integration of add-on packages, modular config      |
|         |                                | script                                              |
+---------+--------------------------------+-----------------------------------------------------+
| 3.0     | alpha May 2007                 | new architecture, new magnitude types, GUI          |
+---------+--------------------------------+-----------------------------------------------------+
| 3.0     | Barcelona release May 2008     | Stability and performance improvements, improved    |
|         |                                | GUI functionality                                   |
+---------+--------------------------------+-----------------------------------------------------+
| 3.0     | Erice release May 2009         | New Earthquake Schema and performance improvements, |
|         |                                | improved GUI functionality                          |
+---------+--------------------------------+-----------------------------------------------------+
| 3.0     | Potsdam release September 2010 | New Inventory Schema and performance improvements,  |
|         |                                | improved GUI functionality                          |
+---------+--------------------------------+-----------------------------------------------------+
