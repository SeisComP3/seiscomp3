
=======================================================
Reqlogstats - collate request statistics from webreqlog
=======================================================


**Motivation**:

As part of the development of EIDA, agencies need a common,
consistent set of usage statistics for reporting to their funding
providers.
The distributed nature of EIDA creates opportunities for under-counting or double counting.
An existing tool, webreqlog, is able to process and summarise logs produced by a single Arclink server, but a system is needed for combining these.


--------------
 Design notes
--------------

* Nodes send their statistics summary to a central node,
  or it fetches them.

* Central node stores everything in a database

  - these will be long-lived, so use a stable standard

  - for generating reports, it's not clear what the best format will be.

  So: use an SQL-based (relational) database. Stable - has been, probably will be, SQL databases around for a long time.
  Provides dump and restore. Many things can read from it.
  (Even Excel??)

  This implies there's a schema, but there's already an existing schema for the
  underlying Arclink database entities to derive this from.
  

* Components:

    1. collector (for each node, accept or fetch a file, and feed it to digester)
    
    2. digester (parse a file to add its contents to the database).

    3. report generator(s).

    4. database, needs to be accessible from 2. and 3.


Types of reports
~~~~~~~~~~~~~~~~

* Table-based: as plain text, as CSV, as HTML.

    - daily table::

        =========  ======  ======  ======  ======  ======
        NAME       NODE 1  NODE 2  NODE 3  NODE 4  Total
        =========  ======  ======  ======  ======  ======        
        qty 1                                        
        qty 1.1                                      X1
        qty 1.2                                      X2
        qty 1.3
        qty 1.4                                      X4
        Total 1      Y1       Y2      Y3      Y4    SX=SY
        =========  ======  ======  ======  ======  ======        


    - history table::

        ====  ======  ======  ======
        DAY   NODE 1  NODE 2   Total
        ====  ======  ======  ======        
        d1
        d2
        d3
        ====  ======  ======  ======

      or aggregrated over weeks, months, years.
      
 * Images: time history of X, over days.


Performance aspects
~~~~~~~~~~~~~~~~~~~

Don't worry too much.
One update per node per day; between 0 and a few dozen views per day.
So updates can take a few seconds; it would be nice for viewing to be fast.

(March 2014): The script to make monthly graphs takes a while.
Tried:

  CREATE INDEX start_idx ON ArcStatsNetwork ( start_day, networkCode );

but this didn't help. Maybe the 2014 DB file is too small - only 2MB.
The `make_month_graphs.sh` script takes 10 seconds just to run through
all codes! Times on sec24c106 for 167 networks:
 - 2.5 sec just running through
 - maybe 3 sec for SQLite look-up and days3.dat generation.
 - 8 sec for gnuplotting.
So no changes for now.


Implementation notes
~~~~~~~~~~~~~~~~~~~~

What DB:

    - for testing, SQLite runs on my Python/Windows
    - for real, MySQL running on:
        + st14, geofon-proc*, st30dmz, st13, st8 have a DB
        + st96 would be a natural place for monitoring
        + mail accumulates on eida.gfz-potsdam.de [geofon-open2/1] (via st1?)
        
      so collect files, transfer by rsync, and process the entire set?
      DB can be on a different host than collector, report generator.

    - for now, SQLite on geofon-open*.
      DB should *not* be under DocumentRoot for Apache. It contains
      usernames of Arclink requestors, and more detail than is presented
      on the report generator web pages. So try
        ~/reqlogstats/var/{name}.db  

Images:

    - SVG looks sharp.
    - Need to be under DocumentRoot.
    - Put under /srv/www/webdc/eida/data/ analogous to /srv/www/geofon/data/*

Key tasks
~~~~~~~~~

 1. A collector based on e-mailed HTML.
    Later we can consider XML or CSV or JSON data transfer,
    and ways to pull it.

 2. A database to store the results

 3. A lightweight report generator to demonstrate that it works.
	- this can be PHP to start off: reqlogdisplay.php
	- later mod_python/mod_wsgi to match webreqlog.py?

 4. Extend different components for:
     a. network code summary
     b. graphs of monthly usage.
     c. aggregating visitors by category (location, region)
     
Schema design
~~~~~~~~~~~~~

Tables are based on those produced by webreqlog today.

  #. Totals

  #. User

  #. Request Type

  #. Volume

  #. UserIP

  #. ClientIP

  #. Grouped by network

Keys would be (server, day). Remember these reports are per-Arclink server, not per DCID.
Disallow intervals other than one day.
Could handle non-day intervals with a key like
 (Server, start_day, days)
 (server, start_time, end_time)
This wouldn't gain too much, so forget about it.
