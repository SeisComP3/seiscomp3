slarchive connects to a SeedLink server, requests data streams and writes received
packets into directory/file structures. The precise layout of the directories and
files is defined in a format string.

The implemented file layouts are:

- the SeisComP Data Structure (SDS)
- Buffer of Uniform Data structure (BUD)
- the old SeisComP/datalog structure (DLOG) for backwards compatibility

To write more than one archive simply specify multiple format definitions (or presets).

SDS definition
==============

The basic directory and file layout is defined as:

:file:`<SDSdir>/Year/NET/STA/CHAN.TYPE/NET.STA.LOC.CHAN.TYPE.YEAR.DAY`.

+-----------+-----------------------------------------------+
| Field     | Description                                   |
+===========+===============================================+
| SDSdir    | Arbitrary base directory                      |
+-----------+-----------------------------------------------+
| YEAR      | 4 digit YEAR                                  |
+-----------+-----------------------------------------------+
| NET       | Network code/identifier, 1-8 characters,      |
|           | no spaces                                     |
+-----------+-----------------------------------------------+
| STA       | Station code/identifier, 1-8 characters,      |
|           | no spaces                                     |
+-----------+-----------------------------------------------+
| CHAN      | Channel code/identifier, 1-8 characters,      |
|           | no spaces                                     |
+-----------+-----------------------------------------------+
| TYPE      | 1 character, indicating the data type,        |
|           | provided types are:                           |
|           |                                               |
|           | | **D** Waveform data                         |
|           | | **E** Detection data                        |
|           | | **L** Log data                              |
|           | | **T** Timing data                           |
|           | | **C** Calibration data                      |
|           | | **R** Response data                         |
|           | | **O** Opaque data                           |
|           |                                               |
+-----------+-----------------------------------------------+
| LOC       | Location identifier, 1-8 characters,          |
|           | no spaces                                     |
+-----------+-----------------------------------------------+
| DAY       | 3 digit day of year, padded with zeros        |
+-----------+-----------------------------------------------+
